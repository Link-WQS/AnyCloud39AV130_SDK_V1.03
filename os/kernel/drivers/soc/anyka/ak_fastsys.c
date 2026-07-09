#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/mm.h>
#include <linux/workqueue.h>
#include <linux/atomic.h>
#include <linux/miscdevice.h>
//#include <asm/sbi.h>
#include <linux/slab.h>
#include <asm-generic/io.h>
#include <soc/anyka/ak_ipi.h>
#include <linux/kthread.h>
#include <linux/uaccess.h>
#include <mach/map.h>
#include <soc/anyka/ipi_protocol.h>

#define AK_FASTSYS_NAME                     "anyka-fastsys"
#define AK_FASTSYS_TARGET_HART              1
#define SBI_EXT_IPI_SEND_FASTSYS_IPI        0x1
#define AK_USEING_SAVE_YUV                  0

//#define FASTSYS_IPI_MEM_SIZE                0x100000

#define IPI_GET_CHAN_TX_ZONE(chan)    (struct ipi_chan_zone *)(chan->cz_addr);
#define IPI_GET_CHAN_RX_ZONE(chan)    (struct ipi_chan_zone *)(chan->cz_addr + fs->chan_size / 2);

struct ipi_mem_info {
    char magic[20];
#define IPI_RX              (0x1 << 0)
    unsigned int rx_status;

#define IPI_TX              (0x1 << 0)
    unsigned int tx_status;
    unsigned int state[4];

    unsigned int chan_zone_base_offset;
    unsigned int num_chans;
    unsigned int chan_size;
    unsigned int chan_data_size;

    unsigned int log_mem_base;
    unsigned int log_mem_size;
    unsigned int log_current_end;
};

/* 0 ~ 19 20Byte magic */
/* 32bit hart0 fastsys irq status */
/* 32bit hart1 fastsys irq status */

struct ak_fastsys {
    unsigned int ipi_mem_size;//核间通信地址, 也在预留地址范围内
    phys_addr_t mem_paddr_base;//小核预留内存地址
    phys_addr_t mem_paddr_size;//预留内存大小
    struct work_struct ipi_work;

#define FASTSYS_STATUS_RUNNING     (0x1)
#define FASTSYS_STATUS_DONE        (0x2)
#define FASTSYS_STATUS_FREE        (0x3)
    atomic_t fastsys_status;

    struct ipi_chan *chans;
    struct ipi_mem_info *mem_info;
    unsigned int num_chans;
    unsigned int chan_size;
    unsigned int chan_data_size;
    void *ipi_mem_base;//虚拟地址
    void * zone_base;//虚拟地址
    struct ipi_chan *chan;
    struct ipi_client client;

    void *log_mem_base;//虚拟地址
    unsigned int log_mem_size;
    unsigned int log_mem_end;
    unsigned int log_current_index;

    /* misc device */
    struct miscdevice miscdev;
    int irq;
};

static char *ipi_mem_magic = "anyka@fastsys";
static struct ak_fastsys *fs;
static LIST_HEAD(common_clients_list);
static DEFINE_SPINLOCK(lock);
void ak_fastsys_soft_irq_handle(void);

//
#define AK_HOST_SEND         0x164
#define AK_HOST_RECV         0x168

#define AHBANT_B_BASE_ADDR      (AK_VA_INT_TIMER + 0x2000)
#define AHBANT_L_BASE_ADDR      (AK_VA_INT_TIMER + 0x3000)

#define AHBHANT_VERSION   0x0
#define AHBHANT_CTRL      0x4 //set clear
#define AHBHANT_INTR_EN   0x8
#define AHBHANT_INTR_STA  0xc
#define AHBHANT_MODE      0x10

/*cpu ahbhant crtl bit*/
#define AHBHANT_CTRL_WORK      0x0
#define AHBHANT_CTRL_PAUSE     0x1
#define AHBHANT_CTRL_STOP      0x2
#define AHBHANT_CTRL_RUN       0x3
#define AHBHANT_CTRL_TRIG      0x5


/*cpu ahbhant interrupt enable bit*/
#define AHBHANT_INTR_WORK      0x0
#define AHBHANT_INTR_STOP      0x1
#define AHBHANT_INTR_PAUSE     0x2
#define AHBHANT_INTR_TRIG      0x3

static irqreturn_t ak_ahbhant_irq_handle(int irq, void* args)
{
    struct ak_fastsys *fs = (struct ak_fastsys*)args;
    uint32_t status;

    status = __raw_readl(AHBANT_B_BASE_ADDR + AHBHANT_INTR_STA);
    __raw_writel(status, AHBANT_B_BASE_ADDR + AHBHANT_INTR_STA);

    ak_fastsys_soft_irq_handle();
    return IRQ_HANDLED;
}

static int ak_ahbhant_intr_init(struct ak_fastsys *fs)
{
    /*enable interrupt from cpul*/
    __raw_writel(0x1UL << AHBHANT_INTR_TRIG, AHBANT_B_BASE_ADDR + AHBHANT_INTR_EN);
    return 0;
}

static void ak_ahbhant_trigger_cpub_irq(void)
{
    __raw_writel(0x1UL << AHBHANT_CTRL_TRIG, AHBANT_L_BASE_ADDR + AHBHANT_CTRL);
}

static void ak_little_cpu_stop(void)
{
#if 0
    int count_down = 10000000;
    /*ahbhant stop*/
    __raw_writel(0x1UL << AHBHANT_CTRL_STOP, AHBANT_L_BASE_ADDR + AHBHANT_CTRL);

    //fanxin建议停止后查询状态确认小核已停止
    while ((count_down-- > 0) && (__raw_readl(AHBANT_L_BASE_ADDR + AHBHANT_CTRL) & (0x1UL << AHBHANT_CTRL_STOP)))
    {}

    if (count_down == 0) {
        pr_warn("little cpu stop 0x%.8x time out %d\n", __raw_readl(AHBANT_L_BASE_ADDR + AHBHANT_CTRL), count_down);
    }

    /*clk and reset*/
    __raw_writel((0x1UL << 31) | (0x1UL << 30), AK_VA_SYSCTRL + 0x28);
#else
    uint32_t regval;

    /*ahbhant stop*/
    __raw_writel(0x1UL << AHBHANT_CTRL_STOP, AHBANT_L_BASE_ADDR + AHBHANT_CTRL);
    udelay(10);

    /*reset assert*/
    regval = __raw_readl(AK_VA_SYSCTRL + 0x28);
    regval |= (0x1UL << 30);
    __raw_writel(regval, AK_VA_SYSCTRL + 0x28);
    udelay(10);
    /*clk close*/
    regval |= (0x1UL << 31);
    __raw_writel(regval, AK_VA_SYSCTRL + 0x28);

    /*清除小核中断以及关闭使能，否者小核会一直有中断*/
    __raw_writel(0x3FFF, AHBANT_L_BASE_ADDR + AHBHANT_INTR_STA);
    __raw_writel(0x0, AHBANT_L_BASE_ADDR + AHBHANT_INTR_EN);

    // pr_info("%s stop \n", ak_rproc_data->core_name);
#endif
}

static int ak_fastsys_stop(void)
{
    int fastsys_tick_timer = 4;

    /* ahb l cpu stop */
    ak_little_cpu_stop();

    /* disable RTOS tick -> timer4 */
    __raw_writel(0, AK_VA_INT_TIMER + 0x10000 + 0x1000 * fastsys_tick_timer + 0x0008);
    __raw_writel(0, AK_VA_INT_TIMER + 0x10000 + 0x1000 * fastsys_tick_timer + 0x0010);
    __raw_writel(1, AK_VA_INT_TIMER + 0x10000 + 0x1000 * fastsys_tick_timer + 0x000C);

    printk("%s\n", __func__);
    return 0;
}

/**
 * @brief ak_get_fastsys_state
 *
 * @param state
 * @param param
 * @return int
 */
int ak_get_fastsys_state(unsigned int *state, unsigned int *param)
{
    if (!fs)
        return -1;

    if (state)
        *state = fs->mem_info->state[0];
    if (param)
        *param = fs->mem_info->state[1];

    return 0;
}
EXPORT_SYMBOL(ak_get_fastsys_state);

/**
 * @brief ak_ipi_send_msg
 *
 * @param chan
 * @param msg
 * @return int
 */
int ak_ipi_send_msg(struct ipi_chan *chan, struct ipi_msg *msg)
{
    unsigned long flags;
    int ret = 0;
    struct ipi_chan_zone *tx_cz;

    // irq store
    spin_lock_irqsave(&chan->lock, flags);
    if (!chan || msg->len > fs->chan_data_size ||
        atomic_read(&fs->fastsys_status) != FASTSYS_STATUS_RUNNING) {
        spin_unlock_irqrestore(&chan->lock, flags);
        return -1;
    }

    tx_cz = IPI_GET_CHAN_TX_ZONE(chan);
    tx_cz->cmd = msg->cmd;
    tx_cz->len = msg->len;
    tx_cz->status = IPI_CHAN_TX;

    if (msg->len)
        memcpy((void *)tx_cz->data, msg->data, msg->len);

//    printk("ak_ipi_send_msg %d %d %d\n", tx_cz->status, tx_cz->cmd, tx_cz->len);

    fs->mem_info->tx_status = IPI_TX;

    ak_ahbhant_trigger_cpub_irq();
    spin_unlock_irqrestore(&chan->lock, flags);
    // irq restore
    if (chan->cl->tx_block) {
        if (!wait_for_completion_timeout(&chan->tx_complete, msecs_to_jiffies(3600000))) {
            pr_err("wait ipi tx complete timeout\n");
            ret = -ETIME;
        }
    }

    return ret;
}
EXPORT_SYMBOL(ak_ipi_send_msg);

/**
 * @brief ak_ipi_request_channel
 *
 * @param cl
 * @param index
 * @return ipi_chan
 */
struct ipi_chan *ak_ipi_request_channel(struct ipi_client *cl, int index)
{
    struct ipi_chan *chan = NULL;
    unsigned long flags;

    if (!fs || !cl || index > fs->num_chans)
        return NULL;
    spin_lock_irqsave(&lock, flags);
    // irq store
    chan = &fs->chans[index];
    if (!chan->cl) {
        chan->cl = cl;
        chan->cz_addr = fs->zone_base + fs->chan_size * index;
    } else
        chan = NULL;
    spin_unlock_irqrestore(&chan->lock, flags);
    // irq restore
    return chan;
}
EXPORT_SYMBOL(ak_ipi_request_channel);

//static int fast_ae_param[32] = {0};
DECLARE_COMPLETION(wait_fast_vi_close);
//void *ak_wait_fast_vi_close(void)
//{
//    printk("wait_fast_vi_close fast_ae_param:%d\n",sizeof(fast_ae_param));
//    wait_for_completion_timeout(&wait_fast_vi_close, msecs_to_jiffies(100));
//    return (void *) &fast_ae_param[0];
//}
//EXPORT_SYMBOL(ak_wait_fast_vi_close);

/**
 * @brief ak_fastsys_soft_irq_handle
 *   if msg is ipc normal module req, parse and invoke rx_callback process
 *   if msg is glass ,here re-process and store messgae packet to glass_packet
 *   ipi message recv & parse, and relay to ipicmd_thread,
 *   if sigle cmd parse and store to glass_packet, send SIGIO signal to user
 * @param
 * @param
 * @return
 */
void ak_fastsys_soft_irq_handle(void)
{
    struct ipi_msg msg;
    struct ipi_chan *chan;
    struct ipi_chan_zone *rx_cz, *tx_cz;
    unsigned int i;
    unsigned long flags;

    irq_enter();
    if (!fs || atomic_read(&fs->fastsys_status) != FASTSYS_STATUS_RUNNING)
        goto exit;

    if (fs->mem_info->rx_status & IPI_RX) {
        fs->mem_info->rx_status = 0;
        for (i = 0; i < fs->num_chans; i++) {
            chan = &fs->chans[i];
            if (!chan->cl)
                continue;
            rx_cz = IPI_GET_CHAN_RX_ZONE(chan);
            tx_cz = IPI_GET_CHAN_TX_ZONE(chan);
            if (rx_cz->status & IPI_CHAN_RX) {
                msg.cmd = rx_cz->cmd;
                msg.len = rx_cz->len;
                msg.data = (void *)rx_cz->data;
                if(i == IPI_CHANNEL_ISP && msg.cmd  == 0x1) {
//                    printk("fast_ae_param-len:%d\n", msg.len);
//                    memcpy(&fast_ae_param[0], msg.data, msg.len);
                    complete(&wait_fast_vi_close);
//                    pr_info("exp=%d again=%d dgain=%d\n", fast_ae_param[0], fast_ae_param[1], fast_ae_param[2]);
                }
//                else {
                if (chan->cl->rx_callback)
                    chan->cl->rx_callback(chan->cl, &msg);
//                }

                rx_cz->cmd = 0;
                rx_cz->len = 0;
                rx_cz->status = IPI_CHAN_RX_COMPLETE;
            }
            if (tx_cz->status & IPI_CHAN_TX_COMPLETE) {
                if (chan->cl->tx_block)
                    complete(&chan->tx_complete);
                tx_cz->status = 0;
            }
        }
    }

exit:
    //ZGP
//    csr_clear(CSR_IP, IE_SIE);
    irq_exit();
} /* end of func */
EXPORT_SYMBOL(ak_fastsys_soft_irq_handle);

#define IPI_FASTSYS_CMD_RUNNING     0x1
#define IPI_FASTSYS_CMD_DONE        0x2
#define IPI_FASTSYS_CMD_ION         0x3

#define IPI_MASTER_CMD_RUNNING      0x1
#define IPI_MASTER_CMD_FSDONE       0x2

struct ion_used_mem {
    void *base;
    unsigned int  size;
};

static struct ion_used_mem ion_mem;

/**
 * @brief fastsys_get_used_ion_mem
 *
 * @param phys_addr_t *base
 * @param size
 * @return int
 */
int fastsys_get_used_ion_mem(phys_addr_t *base, unsigned int *size)
{
    if (!ion_mem.base || ion_mem.size)
        return -1;

    if (base)
        *base = (phys_addr_t)(ion_mem.base - 0x40000000);
    if (size)
        *size = ion_mem.size;

    return 0;
}

/**
 * @brief fastsys_rx_callback
 *
 * @param client
 * @param msg
 * @return
 */
static void fastsys_rx_callback(struct ipi_client *client, struct ipi_msg *msg)
{
    switch (msg->cmd) {
        case IPI_FASTSYS_CMD_RUNNING:
            pr_info("IPI_FASTSYS_CMD_RUNNING\n");
            atomic_set(&fs->fastsys_status, FASTSYS_STATUS_RUNNING);
            break;
        case IPI_FASTSYS_CMD_DONE:
            pr_info("FASTSYS_STATUS_DONE\n");
            atomic_set(&fs->fastsys_status, FASTSYS_STATUS_DONE);
            schedule_work(&fs->ipi_work);
            break;
        case IPI_FASTSYS_CMD_ION:
            memcpy(&ion_mem, msg->data, sizeof(struct ion_used_mem));
            pr_info("fastsys_rx_callback: fastsys used ion mem base 0x%x size 0x%x\n", ion_mem.base, ion_mem.size);
            break;
        default:
            break;
    }
}

/**
 * @brief ak_fastsys_ipi_work
 *
 * @param work
 * @param
 * @return
 */
static void ak_fastsys_ipi_work(struct work_struct *work)
{
    struct ipi_common_client *client;
    unsigned long flags;
    char *log_buffer;

    // free fastsys resource
    if ((atomic_read(&fs->fastsys_status) == FASTSYS_STATUS_DONE)) {
        ak_fastsys_stop();
        atomic_set(&fs->fastsys_status, FASTSYS_STATUS_FREE);

        log_buffer = kzalloc(fs->log_mem_size, GFP_KERNEL);
        memcpy(log_buffer, (void *)fs->log_mem_base, fs->log_mem_size);
        spin_lock_irqsave(&fs->chan->lock, flags);
        pr_info("Fastsys is done !!! free fastsys resource\n");
        fs->log_mem_base = (phys_addr_t)log_buffer;
        fs->log_mem_end = fs->mem_info->log_current_end;
#if 0
        memunmap((void *)fs->ipi_mem_base);
        memunmap((void *)fs->log_mem_base);

        free_reserved_area(phys_to_virt(fs->mem_paddr_base),
                 phys_to_virt(fs->mem_paddr_base + fs->mem_paddr_size), 0, "fastsys");
        while (!list_empty(&common_clients_list)) {
            client = list_first_entry(&common_clients_list, struct ipi_common_client, node);
            list_del(&client->node);
            spin_unlock_irqrestore(&fs->chan->lock, flags);
            if (client->cb)
                client->cb(client->data);
            spin_lock_irqsave(&fs->chan->lock, flags);
        }
#endif
        spin_unlock_irqrestore(&fs->chan->lock, flags);
    }
}

/**
 * @brief ak_fastsys_free_resource
 *
 * @param
 * @param
 * @return
 */
void ak_fastsys_free_resource(void)
{
    schedule_work(&fs->ipi_work);
}
EXPORT_SYMBOL(ak_fastsys_free_resource);

/**
 * @brief ak_fastsys_is_done
 *
 * @param
 * @param
 * @return bool
 */
static inline bool ak_fastsys_is_done(void)
{
    return atomic_read(&fs->fastsys_status) >= FASTSYS_STATUS_DONE;
}

/**
 * @brief ak_register_common_client
 *
 * @param client
 * @param
 * @return int
 */
int ak_register_common_client(struct ipi_common_client *client)
{
    unsigned long flags;

    if (!client || !client->cb)
        return -EINVAL;

    spin_lock_irqsave(&fs->chan->lock, flags);
    if (ak_fastsys_is_done()) {
        spin_unlock_irqrestore(&fs->chan->lock, flags);
        client->cb(client->data);
        return 0;
    }

    list_add_tail(&client->node, &common_clients_list);
    spin_unlock_irqrestore(&fs->chan->lock, flags);

    return 0;
}
EXPORT_SYMBOL(ak_register_common_client);

/**
 * @brief ak_unregister_common_client
 *
 * @param client
 * @param
 * @return int
 */
void ak_unregister_common_client(struct ipi_common_client *client)
{
    unsigned long flags;

    if (!client)
        return;

    spin_lock_irqsave(&fs->chan->lock, flags);
    list_del(&client->node);
    spin_unlock_irqrestore(&fs->chan->lock, flags);
}
EXPORT_SYMBOL(ak_unregister_common_client);

/**
 * @brief ak_fastsys_wait_state_change
 *
 * @param fs
 * @param target
 * @param delaycnt_unit5ms
 * @return int
 */
static int ak_fastsys_wait_state_change(struct ak_fastsys *fs,
    unsigned int target, unsigned int delaycnt_unit5ms)
{
    unsigned int cur_state, cur_param;

    /* wait */
    delaycnt_unit5ms = (delaycnt_unit5ms)? delaycnt_unit5ms : 1;
    do{
        if(ak_get_fastsys_state(&cur_state, &cur_param))
            break;
        if (cur_state >= MACHINE_VICAP_CLOSE_END)
            break;
        if(--delaycnt_unit5ms) mdelay(5);
    }while(delaycnt_unit5ms && cur_state != target);
    return (cur_state != target)? -1 : 0;
}

/**
 * @brief ak_fastsys_preprocess
 *
 * @param fs
 * @param
 * @param
 * @return int
 */
static int ak_fastsys_preprocess(struct ak_fastsys *fs)
{
    unsigned int state, param;
    struct ipi_msg msg;
    int ret = 0;

    ret = ak_get_fastsys_state(&state, &param);
    if(ret){
        pr_err("get small-core state failed(%d)\n", ret);
        return -1;
    }
    pr_info("%s %d, state %d param %d\n", __func__, __LINE__, state, param);
    switch(state){
        case MACHINE_COMMUNICATION:
            pr_crit("small-core state MACHINE_COMMUNICATION param%d\n", param);
            break;
        default:
            pr_info("small-core state %d param%d\n", state, param);
            /* shutdown fastsys */
            memset(&msg, 0, sizeof(struct ipi_msg));
            msg.cmd = IPI_MASTER_CMD_FSDONE;
            ak_ipi_send_msg(fs->chan, &msg);
            pr_info("wait small-core state change\n");
            /* wait */
            ret = ak_fastsys_wait_state_change(fs, MACHINE_COMMUNICATION, 1000/5);
            if(ret)
                pr_err("wait small-core state change to %d failed. %d\n", MACHINE_COMMUNICATION, fs->mem_info->state[0]);
            break;
    }
    return ret;
}

/**
 * @brief log_size_align
 *
 * @param size
 * @param
 * @param
 * @return int
 */
static inline unsigned int log_size_align(unsigned int size)
{
#define align_size          4
    unsigned int _size =  (((size)+((align_size)-1))&(typeof(size))(~((align_size)-1)));

    if (!(_size % align_size))
        return _size + align_size;
    return size;
}

/**
 * @brief fastsys_log_open
 *
 * @param inode
 * @param file
 * @param
 * @return int
 */
static int fastsys_log_open(struct inode* inode, struct file* file)
{
    fs->log_current_index = 0;
    return 0;
}

/**
 * @brief fastsys_log_release
 *
 * @param inode
 * @param file
 * @param
 * @return int
 */
static int fastsys_log_release(struct inode* inode, struct file* file)
{
    return 0;
}

/**
 * @brief fastsys_log_read
 *
 * @param file
 * @param buffer
 * @param count
 * @param ptr
 * @return int
 */
static ssize_t fastsys_log_read(
    struct file* file, char __user* buffer, size_t count, loff_t* ptr)
{
#if defined(CONFIG_MACH_AK3918AV130)
typedef u32 _log_meta_t;
#else
typedef u64 _log_meta_t;
#endif
    _log_meta_t line_len, len, current_time;
    unsigned int log_count = 0;
    char *current_buffer_base;
#define FASTSYS_LOG_BUFFER_SIZE      512
    char log_copy_buffer[FASTSYS_LOG_BUFFER_SIZE];

    memset(log_copy_buffer, 0, FASTSYS_LOG_BUFFER_SIZE);
//    if (!ak_fastsys_is_done()) {
        fs->log_mem_end = fs->mem_info->log_current_end;
//    }

    if (fs->log_current_index != fs->log_mem_end) {
        current_buffer_base = (char *)(fs->log_mem_base + fs->log_current_index);
        line_len = *((_log_meta_t *)current_buffer_base);
        if (line_len > FASTSYS_LOG_BUFFER_SIZE)
            len = FASTSYS_LOG_BUFFER_SIZE;
        else
            len = line_len;
        current_buffer_base +=  sizeof(_log_meta_t);
        current_time = *((_log_meta_t *)current_buffer_base);
        current_buffer_base +=  sizeof(_log_meta_t);
        fs->log_current_index += log_size_align(line_len + 2 * sizeof(_log_meta_t));
#if defined(CONFIG_MACH_AK3918AV130)
        sprintf(log_copy_buffer, "[%5dms] %s", current_time, current_buffer_base);
#else
        sprintf(log_copy_buffer, "[%8lldms] %s", current_time, current_buffer_base);
#endif
        log_count = strlen(log_copy_buffer);
        copy_to_user(buffer, log_copy_buffer, log_count);
    }

    return log_count;
}

/* file ops */
static const struct file_operations fastsys_log_fops = {
    .owner   = THIS_MODULE,
    .open    = fastsys_log_open,
    .read    = fastsys_log_read,
    .release = fastsys_log_release,
};

#if 1
//测试代码
static int ak_ipimsg_test_ipi_thread(void *arg)
{
    char buf[512] = {0xAA};
    struct ipi_msg msg = {0};
    msg.cmd = IPI_MASTER_CMD_RUNNING;
    msg.len = 512;
    msg.data = buf;

    while (1) {
        msleep(200);
        ak_ipi_send_msg(fs->chan, &msg);
    }

    return 0;
}

#if 0
typedef struct ak_isp_wb_gain {
    unsigned short r_gain;
    unsigned short g_gain;
    unsigned short b_gain;
    signed short r_offset;
    signed short g_offset;
    signed short b_offset;
} AK_ISP_WB_GAIN;

typedef struct ae_fast_struct {
    int sensor_exp_time;
    int sensor_a_gain;
    int sensor_d_gain;
    int isp_d_gain;
    AK_ISP_WB_GAIN wb;
}AK_ISP_AE_FAST;

struct ipi_isp_frame{
    unsigned int attr;
#define IPI_ISP_FRAME_ATTR_ERR              (0x1 << 0)

    unsigned int seq;
    unsigned long timestramp;
    unsigned long length;
    unsigned long phy_addr;
};

/**
 * ipi
 */
/**
 * cmd
 */
/* cmd list */
/* rtos to linux */
/* post fastae message */
#define IPI_FASTSYS_CMD_AE_FAST_PARAM       0x1
/* query frame information */
#define IPI_FASTSYS_CMD_POST_FRAMESINFO     0x2
/* get one frame */
#define IPI_FASTSYS_CMD_POST_ONEFRAME       0x3
/* post rtos parameter for display*/
#define IPI_FASTSYS_CMD_UMAP_PARAM          0x4

/* linux to rtos */
/* request fastae command from linux */
#define IPI_MASTER_CMD_GET_AE_FAST_PARAM    0x1
/* query frame information */
#define IPI_MASTER_CMD_QUERY_FRAMESINFO     0x2
/* get one frame */
#define IPI_MASTER_CMD_GET_ONEFRAME         0x3
/* release one frame */
#define IPI_MASTER_CMD_RELEASE_ONEFRAME     0x4
/* all down */
#define IPI_MASTER_CMD_ALL_DONE             0x5
/* get rtos parameter for display */
#define IPI_MASTER_CMD_GET_UMAP_PARAM       0x6
#endif

struct fastsys_isp {
    struct ipi_client isp_ipi_client;
    struct ipi_chan *ipi_chan;
    struct ipip_isp_frame frame[2];
    int frame_count;
    struct ipip_fast_ae_info fast_ae_info;
    int fast_ae_num;

    struct ipip_vi_dev_status ir_led;
};

static struct fastsys_isp *_s_fastsys_isp;

void *ak_wait_fast_vi_close(void)
{
    if (!_s_fastsys_isp) {
        return NULL;
    }
    pr_info("fast_ae_num %d\n", _s_fastsys_isp->fast_ae_num);
    _s_fastsys_isp->fast_ae_info.fast_valid_ae_num = _s_fastsys_isp->fast_ae_num;

    if (_s_fastsys_isp->fast_ae_num == 0) {
        wait_for_completion_timeout(&wait_fast_vi_close, msecs_to_jiffies(100));

//        if (_s_fastsys_isp->fast_ae_num == 0) {
//            return NULL;
//        }
    }
    return (void *) &_s_fastsys_isp->fast_ae_info;
}
EXPORT_SYMBOL(ak_wait_fast_vi_close);

void *ak_get_vi_dev_status(void)
{
    if (!_s_fastsys_isp) {
        return NULL;
    }

    return (void *) &_s_fastsys_isp->ir_led;
}
EXPORT_SYMBOL(ak_get_vi_dev_status);

static void ak_fastsys_isp_rx_callback(struct ipi_client *client, struct ipi_msg *msg)
{
    struct ipi_msg askmsg = {0};
    int i;

    switch (msg->cmd) {
        case IPI_FASTSYS_CMD_AE_FAST_PARAM: {
                struct ipip_fast_ae_status *fast_ae;
                _s_fastsys_isp->fast_ae_num = msg->len / sizeof(struct ipip_fast_ae_status);
                if (_s_fastsys_isp->fast_ae_num <= 0 || _s_fastsys_isp->fast_ae_num > 4) {
                    pr_err("fast_ae_num %d\n", _s_fastsys_isp->fast_ae_num);
                }
                else {
                    memcpy(_s_fastsys_isp->fast_ae_info.fast_ae, msg->data, msg->len);
                    for (i = 0; i < _s_fastsys_isp->fast_ae_num; ++i) {
                        fast_ae = &_s_fastsys_isp->fast_ae_info.fast_ae[i];
                        pr_info("dev%d AE_FAST_PARAM ae(%d,%d,%d,%d), wb offset(%d %d %d) gain(%d %d %d)\n", i,
                                                            fast_ae->ae.sensor_exp_time,
                                                            fast_ae->ae.sensor_a_gain,
                                                            fast_ae->ae.sensor_d_gain,
                                                            fast_ae->ae.isp_d_gain,
                                                            fast_ae->ae.wb.r_offset,
                                                            fast_ae->ae.wb.g_offset,
                                                            fast_ae->ae.wb.b_offset,
                                                            fast_ae->ae.wb.r_gain,
                                                            fast_ae->ae.wb.g_gain,
                                                            fast_ae->ae.wb.b_gain);
                    }
                }

                /* query fast param2 */
                askmsg.cmd = IPI_MASTER_CMD_GET_FAST_PARAM2;
                ak_ipi_send_msg(_s_fastsys_isp->ipi_chan, &askmsg);
            }
            break;

        case IPI_FASTSYS_CMD_POST_FAST_PARAM2: {
                memcpy(&(_s_fastsys_isp->ir_led), msg->data + 4, sizeof(struct ipip_vi_dev_status));
                for (i = 0; i < 4; ++i) {
                    if (_s_fastsys_isp->ir_led.ircut[i].ircut_a_gpio.gpio >= 0) {
                        pr_info("ircut_id[%d] a(%d,%d) b(%d,%d), irled(%d %d %d), whiteled(%d %d %d)\n", i,
                                            _s_fastsys_isp->ir_led.ircut[i].ircut_a_gpio.gpio,
                                            _s_fastsys_isp->ir_led.ircut_a_curstate[i],
                                            _s_fastsys_isp->ir_led.ircut[i].ircut_b_gpio.gpio,
                                            _s_fastsys_isp->ir_led.ircut_b_curstate[i],
                                            _s_fastsys_isp->ir_led.led[i].irled_gpio.gpio,
                                            _s_fastsys_isp->ir_led.led[i].irled_gpio.value,
                                            _s_fastsys_isp->ir_led.irled_curstate[i],
                                            _s_fastsys_isp->ir_led.led[i].whiteled_gpio.gpio,
                                            _s_fastsys_isp->ir_led.led[i].whiteled_gpio.value,
                                            _s_fastsys_isp->ir_led.whiteled_curstate[i]);
                    }
                }

                /* query framesinfo */
                askmsg.cmd = IPI_MASTER_CMD_QUERY_FRAMESINFO;
                ak_ipi_send_msg(_s_fastsys_isp->ipi_chan, &askmsg);
            }
            break;

        case IPI_FASTSYS_CMD_POST_FRAMESINFO: {
#if AK_USEING_SAVE_YUV
                askmsg.cmd = IPI_MASTER_CMD_GET_ONEFRAME;
                ak_ipi_send_msg(_s_fastsys_isp->ipi_chan, &askmsg);
#else
                askmsg.cmd = IPI_MASTER_CMD_ALL_DONE;
                ak_ipi_send_msg(_s_fastsys_isp->ipi_chan, &askmsg);
#endif
            }
            break;

        case IPI_FASTSYS_CMD_POST_ONEFRAME: {
                int i = _s_fastsys_isp->frame_count++;
                if (i < 2) {
                    memcpy(&_s_fastsys_isp->frame[i], msg->data, sizeof(struct ipip_isp_frame));
                    pr_err("frame[%d]:\n",      _s_fastsys_isp->frame_count);
                    pr_err("attr: %x\n",        _s_fastsys_isp->frame[i].attr);
                    pr_err("seq: %d\n",         _s_fastsys_isp->frame[i].seq);
                    pr_err("timestramp: %lu\n",  _s_fastsys_isp->frame[i].timestramp);
                    pr_err("length: %lu\n",      _s_fastsys_isp->frame[i].length);
                    pr_err("phy_addr: %lx\n",    _s_fastsys_isp->frame[i].phy_addr);
                }
                if (_s_fastsys_isp->frame_count < 2) {
                    askmsg.cmd = IPI_MASTER_CMD_GET_ONEFRAME;
                    ak_ipi_send_msg(_s_fastsys_isp->ipi_chan, &askmsg);
                } else {
                    askmsg.cmd = IPI_MASTER_CMD_ALL_DONE;
                    ak_ipi_send_msg(_s_fastsys_isp->ipi_chan, &askmsg);
                }
            }
            break;

        default:
            break;
    }
}/* end of func */

#if AK_USEING_SAVE_YUV
static int ak_fastsys_isp_thread(void *arg)
{
    int i;
    struct file *filp;
    int path[64];
    u32 data;

    msleep(10000);

    pr_info("fastsys-frame-count: %s\n", _s_fastsys_isp->frame_count);

    for (i = 0; i < _s_fastsys_isp->frame_count; ++i) {

        sprintf(path, "/tmp/%d-%d.yuv", _s_fastsys_isp->frame[i].seq, _s_fastsys_isp->frame[i].timestramp);

        pr_info("fastsys-yuv-file: %s\n", path);

        data = memremap(_s_fastsys_isp->frame[i].phy_addr, _s_fastsys_isp->frame[i].length, MEMREMAP_WT);

        filp = filp_open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        kernel_write(filp, (const char *)data, _s_fastsys_isp->frame[i].length, NULL);

        filp_close(filp, NULL);

        memunmap(data);
    }

    return 0;
}
#endif /*AK_USEING_SAVE_YUV*/


#endif

/**
 * @brief ak_fastsys_probe
 *
 * @param pdev
 * @return int
 */
static int ak_fastsys_probe(struct platform_device *pdev)
{
    struct resource res;
    struct device_node *memory;
    phys_addr_t ipi_mem_paddr;
    int err, i;
    struct ipi_msg msg;
    u32 pp_mem_size = 0;

    /*
    从dts获取小核预留地址空间
    */
    memory = of_parse_phandle(pdev->dev.of_node, "memory-region", 0);
    err = of_address_to_resource(memory, 0, &res);
    of_node_put(memory);
    if (err)
        return err;

    fs = devm_kzalloc(&pdev->dev, sizeof(struct ak_fastsys), GFP_KERNEL);
    if (!fs) {
        pr_err("%s alloc the fastsys fail.\n", __func__);
        return -ENOMEM;
    }

    /*

    */
    fs->mem_paddr_base = res.start;
    fs->mem_paddr_size = resource_size(&res);
    fs->ipi_mem_size = FASTSYS_IPI_MEM_SIZE;//ipi预留大小代码固定配置

    /*
    ZGP
    TODO:AV130没预留pp-mem-size, 去掉
    */
#if 0
    if (of_property_read_u32(pdev->dev.of_node, "pp-mem-size", &pp_mem_size)) {
        pr_err("fastsys get pp-buff-size error\n");
        return -1;
    }
    pr_info("pp-mem-size 0x%x\n", pp_mem_size);
#endif

    /* map as noncached */

    /*******************************************************
    map ipi 地址
    ********************************************************/
#if 0
    ipi_mem_paddr = res.start + CONFIG_RISCV_UNCACHED_OFFSET + resource_size(&res) - pp_mem_size - fs->ipi_mem_size;
#else
    ipi_mem_paddr = res.start + resource_size(&res) - pp_mem_size - fs->ipi_mem_size - SYS_VEC_MEM_SIZE;
#endif
    fs->ipi_mem_base = memremap(ipi_mem_paddr, fs->ipi_mem_size, MEMREMAP_WT);
//    fs->ipi_mem_base = devm_ioremap_nocache(&pdev->dev, ipi_mem_paddr, fs->ipi_mem_size);
    pr_info("fs->ipi_mem_base:0x%x \n",fs->ipi_mem_base);
    if (IS_ERR((void *)fs->ipi_mem_base))
        return PTR_ERR((void *)fs->ipi_mem_base);

    /*******************************************************
    读取ipi_mem_info信息
    ********************************************************/
    fs->mem_info = (struct ipi_mem_info *)(fs->ipi_mem_base);
    // check ipi memory magic
    if (strncmp(fs->mem_info->magic, ipi_mem_magic, strlen(ipi_mem_magic))) {
        pr_err("fastsys memory address error\n");
        memunmap((void *)fs->ipi_mem_base);
        fs = NULL;
        return PTR_ERR((void *)fs);
    }

    fs->chan_size = fs->mem_info->chan_size;
    fs->zone_base = fs->ipi_mem_base + fs->mem_info->chan_zone_base_offset;
    fs->num_chans = fs->mem_info->num_chans;
    fs->chan_data_size = fs->mem_info->chan_data_size;

    /*******************************************************
    map log 地址
    ********************************************************/
    pr_info("ipi log memmory info: base 0x%x size %d\n", fs->mem_info->log_mem_base, fs->mem_info->log_mem_size);
    fs->log_mem_base = memremap(fs->mem_info->log_mem_base, fs->mem_info->log_mem_size, MEMREMAP_WT);
    fs->log_mem_size = fs->mem_info->log_mem_size;

    atomic_set(&fs->fastsys_status, FASTSYS_STATUS_RUNNING);

    pr_info("ipi channel memmory info: zone base 0x%x size %d number %d data size %d\n",
                     (unsigned int)fs->zone_base, fs->chan_size, fs->num_chans, fs->chan_data_size);


    /*******************************************************
    irq
    ********************************************************/
    fs->irq = platform_get_irq(pdev, 0);
    if (fs->irq > 0) {
        err = devm_request_irq(&pdev->dev, fs->irq, ak_ahbhant_irq_handle, 0, "ahbhant", fs);
        if (err) {
            pr_err("failed to request npu interrupt.\n");
            err = -ENOENT;
            return err;
        }
        ak_ahbhant_intr_init(fs);
    }

    /* register log miscdev */
    // fs->miscdev = (struct miscdevice) {
    //     .minor = MISC_DYNAMIC_MINOR,
    //     .name  = "fastsys-log",
    //     .fops  = &fastsys_log_fops,
    // };
    // err = misc_register(&fs->miscdev);
    // if (err) {
    //     pr_err("fastsys misc register error\n");
    // }

    fs->chans = (struct ipi_chan *)devm_kzalloc(&pdev->dev, sizeof(struct ipi_chan) * fs->num_chans, GFP_KERNEL);
    for (i = 0; i < fs->num_chans; i++) {
        spin_lock_init(&fs->chans[i].lock);
        init_completion(&fs->chans[i].tx_complete);
    }

    /*******************************************************
    注册 ipi的 IPI_CHANNEL_FASTSYS
    ********************************************************/
    fs->client.rx_callback = fastsys_rx_callback;
    fs->chan = ak_ipi_request_channel(&fs->client, IPI_CHANNEL_FASTSYS);

    INIT_WORK(&fs->ipi_work, ak_fastsys_ipi_work);

    /*******************************************************
    核间通信初始化 end
    开始请求小核信息
    ********************************************************/


    /*******************************************************
    ZGP
    1. 大核第1条请求命令:
    ********************************************************/
    msg.cmd = IPI_MASTER_CMD_RUNNING;
    msg.len = 0;
    ak_ipi_send_msg(fs->chan, &msg);

    /* fastsys pre-process */
    /*******************************************************
    ZGP
    2. 预处理
        a. 查询小核状态
           -- state == MACHINE_COMMUNICATION ？
                -- y =》
                -- n =》请求关闭采集, 发送 IPI_MASTER_CMD_FSDONE请求
                        -- 等待状态改变. state >= MACHINE_BIG_WINDOWS_INIT ?
                                            -- y => exit
                                            --
    ********************************************************/
    ak_fastsys_preprocess(fs);

    /* pre-process need obtain ipi rx */
    /*******************************************************
    ZGP
    3. 获取AE参数
    ********************************************************/
    /*get fast ae regist need ahead of time avoid rtt send lost */
    if (1) {
        /* request fastae command from linux */
        _s_fastsys_isp = kzalloc(sizeof(struct fastsys_isp), GFP_KERNEL);
        struct ipi_msg msg = {0};

        //保存一个默认的参数
        for (i = 0; i < 4; ++i) {
            _s_fastsys_isp->fast_ae_info.fast_ae[i].ae.sensor_exp_time = 2152;
            _s_fastsys_isp->fast_ae_info.fast_ae[i].ae.sensor_a_gain = 256;
            _s_fastsys_isp->fast_ae_info.fast_ae[i].ae.sensor_d_gain = 256;
            _s_fastsys_isp->fast_ae_info.fast_ae[i].ae.isp_d_gain = 256;
            _s_fastsys_isp->fast_ae_info.fast_ae[i].ae.wb.r_gain = 445;
            _s_fastsys_isp->fast_ae_info.fast_ae[i].ae.wb.g_gain = 256;
            _s_fastsys_isp->fast_ae_info.fast_ae[i].ae.wb.b_gain = 396;
            _s_fastsys_isp->fast_ae_info.fast_ae[i].ae.wb.r_offset = 0;
            _s_fastsys_isp->fast_ae_info.fast_ae[i].ae.wb.g_offset = 0;
            _s_fastsys_isp->fast_ae_info.fast_ae[i].ae.wb.b_offset = 0;
        }

        pr_info("%s regist IPI_CHANNEL_ISP\n",__func__);
        _s_fastsys_isp->isp_ipi_client.rx_callback = ak_fastsys_isp_rx_callback;
        _s_fastsys_isp->ipi_chan = ak_ipi_request_channel(&_s_fastsys_isp->isp_ipi_client, IPI_CHANNEL_ISP);
        msg.cmd = IPI_MASTER_CMD_GET_AE_FAST_PARAM;
        msg.len = 0;
        msg.data = 0;
        ak_ipi_send_msg(_s_fastsys_isp->ipi_chan, &msg);
    }

    /*
    TODO: 循环发送测试
    */
//    kthread_run(ak_ipimsg_test_ipi_thread, NULL, "ak_ipimsg_test_ipi_thread");

    /*
    TODO: 保存文件小核yuv图片到/tmp目录
    */
#if AK_USEING_SAVE_YUV
    kthread_run(ak_fastsys_isp_thread, NULL, "ak_fastsys_isp_thread");
#endif
    return err;
}/* end of func */

static const struct of_device_id ak_fastsys_match[] = {
    { .compatible = "anyka,fastsys" },
    {},
};
MODULE_DEVICE_TABLE(of, ak_fastsys_match);

static struct platform_driver ak_fastsys_platform_driver = {
    .probe        = ak_fastsys_probe,
    .driver        = {
        .name    = AK_FASTSYS_NAME,
        .of_match_table = of_match_ptr(ak_fastsys_match),
    },
};

static int __init ak_fastsys_init(void)
{
    int ret;
    ret = platform_driver_register(&ak_fastsys_platform_driver);
    if(ret){
        pr_info("%s failed\n",__func__);
        return ret;
    }

    pr_info("%s success\n",__func__);
    return ret;
}

arch_initcall(ak_fastsys_init);


static int __init ak_fastsys_log_init(void)
{
    int err = 0;

    if (!fs) {
        pr_err("%s fs is null\n", __func__);
        return -1;
    }

    /* register log miscdev */
    fs->miscdev = (struct miscdevice) {
        .minor = MISC_DYNAMIC_MINOR,
        .name  = "fastsys-log",
        .fops  = &fastsys_log_fops,
    };

    err = misc_register(&fs->miscdev);
    if (err) {
        pr_err("fastsys misc register error\n");
    }
    return err;
}

module_init(ak_fastsys_log_init);

void *ak_get_dss_remap(void)
{
   return (void *) ioremap(PARAM_MEM_BASE, PARAM_MEM_SIZE);
}
EXPORT_SYMBOL(ak_get_dss_remap);

void *ak_get_ispconf_remap(void)
{
   return (void *) ioremap(ISPCONF_MEM_BASE, ISPCONF_MEM_SIZE);
}
EXPORT_SYMBOL(ak_get_ispconf_remap);
