/*
     ———————————————————————————————————————————————————————————————————————————— |
    | 参数  | Min | Max | unit  |                                                 |
    |NCR时间|  2  |  64 |       |cmd_end到cmd_resp的时间(CMD2和ACMD41除外)         |
    |NID时间|  5  |  5  |       |CMD2和ACMD41的NCR时间                            |
    |NAC时间|  2  |  -  |       |cmd_end到read_data的时间                         |
    |NRC时间|  8  |  -  |       |resp_end到cmd_start的时间(已知问题)               |
    |NCC时间|  8  |  -  | clock |cmd_end到cmd_start的时间(CMD0,CMD4和CMD7)        |
    |NWR时间|  2  |  -  | cycles|resp_end到write_data或者crc_end到write_data的时间 |
    |NSD时间|  2  |  2  |       |multi_read时，CMD12_end到DATA线回归高阻的时间     |
    |——————————————————-       |-------------------------------------------------|
    |      |  1  |  1  |       |multi_write时，CMD12_end到繁忙状态时Data0拉低的时间|
    |NSB时间|  2  |  2  |       |                                                |
    |      |  4  |  4  |       |                                                 |
     ————————————————————————————————————————————————————————————————————————————|
*/

#include <config.h>
#include <common.h>
#include <mmc.h>
#include <part.h>
#include <asm/io.h>
#include <errno.h>
#include <clk.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <dm/of_access.h>
#include <asm/gpio.h>
#include "ak_mci.h"

#ifdef CONFIG_39EV33X_CODE
#include <asm/arch-ak39ev33x/ak_cpu.h>
#include <asm/arch-ak39ev33x/ak_types.h>
#include <asm/arch-ak39ev33x/ak_l2buf.h>
#endif

#ifdef CONFIG_37_E_CODE
#include <asm/arch-ak37e/ak_cpu.h>
#include <asm/arch-ak37e/ak_types.h>
#include <asm/arch-ak37e/ak_l2buf.h>
#endif

#ifdef CONFIG_37_D_CODE
#include <asm/arch-ak37d/ak_cpu.h>
#include <asm/arch-ak37d/ak_types.h>
#include <asm/arch-ak37d/ak_l2buf.h>
#endif

#ifdef CONFIG_3918AV100_CODE
#include <asm/arch-ak3918av100/ak_cpu.h>
#include <asm/arch-ak3918av100/ak_types.h>
#include <asm/arch-ak3918av100/ak_l2buf.h>
#endif

#ifdef CONFIG_3918AV130_CODE
#include <asm/arch-ak3918av130/ak_cpu.h>
#include <asm/arch-ak3918av130/ak_types.h>
#include <asm/arch-ak3918av130/ak_l2buf.h>
#endif

#ifdef CONFIG_KM01A_CODE
#include <asm/arch-km01a/ak_cpu.h>
#include <asm/arch-km01a/ak_types.h>
#include <asm/arch-km01a/ak_l2buf.h>
#endif

#ifdef CONFIG_3918EV300L_CODE
#include <asm/arch-ak3918ev300l/ak_cpu.h>
#include <asm/arch-ak3918ev300l/ak_types.h>
#include <asm/arch-ak3918ev300l/ak_l2buf.h>
#endif

#ifdef CONFIG_39EV200_CODE
#include <asm/arch-ak39ev200/ak_cpu.h>
#include <asm/arch-ak39ev200/ak_types.h>
#include <asm/arch-ak39ev200/ak_l2buf.h>
#endif

#define AKMCI_MIN_FREQ  400000
#define SD_IDENTIFICATION_MODE_CLK  (100*1000)

struct power_gpio_attribute {
    int gpio;
    int value;
    int power_invert;
};

struct akmci_host {
    unsigned long        gclk;
    /*
     *  data bus width
     *  #define MMC_BUS_WIDTH_1     0
     *  #define MMC_BUS_WIDTH_4     2
     *  #define MMC_BUS_WIDTH_8     3
     */
    unsigned char       bus_width;
    unsigned int        bus_clock;  /* Current clock speed */
    unsigned char       mci_mode;
    int         id;
    int         gpio_cd;
    int         gpio_wp;
    struct power_gpio_attribute gpio_power;

    int         detect_mode;
    int         card_status;

    void __iomem        *base;
    u32         phys_addr;

    struct mmc_cmd  *cmd;           /* Current command */
    struct mmc_data     *data;      /* Current data request */
    struct udevice      *dev;
    struct mmc      *mmc;
    struct akmci_plat *plat;

};

#ifndef CONFIG_DM_MMC
int akmci_init(void *regbase, u32 id)
{
    return 0;
}
#else

DECLARE_GLOBAL_DATA_PTR;

struct akmci_plat {
    struct mmc_config       cfg;
    struct mmc          mmc;
};

#define ak_reg_dump     pr_debug
static void akmci_dumpregs(struct akmci_host *host)
{
    ak_reg_dump("=========== REGISTER DUMP ===========\n");
    ak_reg_dump("SDIO_INTRCTR_REG       0x%08x\n",
        readl(host->base + SDIO_INTRCTR_REG));
    ak_reg_dump("MCI_CLK_REG            0x%08x\n",
        readl(host->base + MCI_CLK_REG));
    ak_reg_dump("MCI_ARGUMENT_REG       0x%08x\n",
        readl(host->base + MCI_ARGUMENT_REG));
    ak_reg_dump("MCI_COMMAND_REG        0x%08x\n",
        readl(host->base + MCI_COMMAND_REG));
    ak_reg_dump("MCI_RESPCMD_REG        0x%08x\n",
        readl(host->base + MCI_RESPCMD_REG));
    ak_reg_dump("MCI_RESPONSE0_REG      0x%08x\n",
        readl(host->base + MCI_RESPONSE0_REG));
    ak_reg_dump("MCI_RESPONSE1_REG      0x%08x\n",
        readl(host->base + MCI_RESPONSE1_REG));
    ak_reg_dump("MCI_RESPONSE2_REG      0x%08x\n",
        readl(host->base + MCI_RESPONSE2_REG));
    ak_reg_dump("MCI_RESPONSE3_REG      0x%08x\n",
        readl(host->base + MCI_RESPONSE3_REG));
    ak_reg_dump("MCI_DATATIMER_REG      0x%08x\n",
        readl(host->base + MCI_DATATIMER_REG));
    ak_reg_dump("MCI_DATALENGTH_REG     0x%08x\n",
        readl(host->base + MCI_DATALENGTH_REG));
    ak_reg_dump("MCI_DATACTRL_REG       0%08x\n",
        readl(host->base + MCI_DATACTRL_REG));
    ak_reg_dump("MCI_DATACNT_REG        0x%08x\n",
        readl(host->base + MCI_DATACNT_REG));
#if 0 /* do not read status(Bit byte RC) */
    ak_reg_dump("MCI_STATUS_REG         0x%08x\n",
        readl(host->base + MCI_STATUS_REG));
#endif
    ak_reg_dump("MCI_MASK_REG           0x%08x\n",
        readl(host->base + MCI_MASK_REG));
    ak_reg_dump("MCI_DMACTRL_REG        0x%08x\n",
        readl(host->base + MCI_DMACTRL_REG));
    ak_reg_dump("MCI_FIFO_REG           0x%08x\n",
        readl(host->base + MCI_FIFO_REG));

    return;
}

/**
* @brief Check sd controller ready status
*
* Check if sd controller is transferring now
*/
static int sd_trans_busy(struct akmci_host *host)
{
    unsigned int status;

    status = readl(host->base + MCI_STATUS_REG);
    if ((status & MCI_TXACTIVE) || (status & MCI_RXACTIVE)) {
        return  1;
    } else {
        return 0;
    }
}

/**
* @brief sd controller send data
*
* sd controller send data ,use cpu mode
*/

static int sd_tx_data_cpu(struct akmci_host *host, char *buf,
    unsigned int len)
{
    unsigned int status;
    unsigned int offset = 0;

    while (1) {
        status = readl(host->base + MCI_STATUS_REG);
        if ((offset < len) && ( status & MCI_FIFOEMPTY)) {
            writel( (buf[offset])|(buf[offset+1]<<8)|(buf[offset+2]<<16)|
                (buf[offset+3]<<24),  host->base + MCI_FIFO_REG);
            offset += 4;
        }

        if (status & MCI_DATATIMEOUT) {
            pr_err("%s, timeout, status is %x\n",__func__, status);
            return -ETIMEDOUT;
        }
    
        if (status & MCI_DATACRCFAIL) {
           pr_err("%s,crc fail, status is %x\n", __func__, status);
            return -ETIMEDOUT;
        }

        if (!(status & MCI_TXACTIVE)) {
            break;
        }
    }
    return 0;
}

/**
* @brief sd controller receive data
*
* sd controller receive data ,use cpu mode
*/
static int sd_rx_data_cpu(struct akmci_host *host, char *buf,
    unsigned int len)
{
    unsigned int status;
    unsigned int buf_tmp;
    unsigned int i;
    unsigned int offset, size;

    offset = 0;
    size = len;

    while (1) {
        status = readl(host->base + MCI_STATUS_REG);
        if ((status & MCI_FIFOFULL)) {
            buf_tmp  = readl(host->base + MCI_FIFO_REG);
            for (i=0; i<4; i++) {
                buf[offset+i] = (unsigned char)((buf_tmp >> (i*8)) & 0xff);
            }

            offset += 4;
            size -= 4;
        }

        if ((size > 0) && (size < 4) && (status & MCI_DATAEND)) {
            buf_tmp = readl(host->base + MCI_FIFO_REG);
            for (i=0; i<size; i++) {
                buf[offset+i] = (unsigned char)((buf_tmp >> (i*8)) & 0xff);
            }

            size = 0;
        }

        if ((status & MCI_DATATIMEOUT)) {
            pr_err("%s, timeout, status is %x, xferd:%d\n",__func__, status, offset);
            return -ETIMEDOUT;
        }

        if ((status & MCI_DATACRCFAIL)) {
            pr_err("%s,crc fail, status is %x, xferd:%d\n", __func__, status, offset);
            return -EIO;
        }

        if (!(status& MCI_RXACTIVE)) {
            //pr_err("%s, rx active fail, status is %x, xferd:%d\n",__func__, status, offset);
            break;
        }
    }

    return 0;
}

/**
 * @brief SD read or write data use cpu mode
 *
 * read or write data transfer with use cpu mode
 */
static int sd_trans_data_cpu(struct akmci_host *host,
    struct mmc_data *data)
{
    unsigned int size = data->blocksize * data->blocks;
    unsigned int datactrl = 0;

    pr_debug("%s blocksize=%x, blocks=%x, size=%x, data->flags=%d\n",__func__,
        data->blocksize,data->blocks, size,data->flags);

    writel(TRANS_DATA_TIMEOUT, host->base + MCI_DATATIMER_REG);
    writel(size, host->base + MCI_DATALENGTH_REG);

    datactrl |= MCI_DPSM_ENABLE;
    datactrl |= MCI_DPSM_BLOCKSIZE(data->blocksize);
    datactrl &= ~MCI_DPSM_STREAM;
#ifdef SDIO_INTR_AUTO_DETECT
    if(data->blocks > 1)
        datactrl |= MCI_DPSM_ABORT_EN_MULTIBLOCK;
#endif

    switch (host->bus_width) {
        case MMC_BUS_WIDTH_8:
            datactrl |= MCI_DPSM_BUSMODE(2);
            break;
        case MMC_BUS_WIDTH_4:
            datactrl |= MCI_DPSM_BUSMODE(1);
            break;
        case MMC_BUS_WIDTH_1:
        default:
            datactrl |= MCI_DPSM_BUSMODE(0);
            break;
    }

    if (data->flags & MMC_DATA_READ)
        datactrl |= MCI_DPSM_DIRECTION;
    else if (data->flags & MMC_DATA_WRITE)
        datactrl &= ~MCI_DPSM_DIRECTION;

    /* configurate data controller register */
    writel(datactrl, host->base + MCI_DATACTRL_REG);
    return 0;
}

/**
 * @brief  config sd controller, start sending command.
 * @param [in] *host information of sd controller.
 * @param [in] *cmd information of  cmd sended.
 * @param [in] *data information of cmd sended.
 * @return void.
 */
static void akmci_start_command(struct akmci_host *host,
    struct mmc_cmd *cmd, struct mmc_data *data)
{
    unsigned int ccon;

    dev_dbg(host->dev, "mci send cmd: op %i arg 0x%08x  res_type 0x%08x. %s \
        data.\n", cmd->cmdidx, cmd->cmdarg, cmd->resp_type,
        data ? "contain":"no");

    writel(cmd->cmdarg, host->base + MCI_ARGUMENT_REG);

    ccon = MCI_CPSM_CMD(cmd->cmdidx) | MCI_CPSM_ENABLE;
    if(cmd->resp_type & MMC_RSP_NONE)
        ccon &= ~(MCI_CPSM_RESPONSE | MCI_CPSM_LONGRSP);
    else if (cmd->resp_type & MMC_RSP_PRESENT) {
        ccon |= MCI_CPSM_RESPONSE;
        if (cmd->resp_type & MMC_RSP_136)
            ccon |= MCI_CPSM_LONGRSP;
    }

#ifdef SDIO_INTR_AUTO_DETECT
    #define GET_SDIO_FUNID(arg)  (((arg)>>28) & 0x7)
    #define GET_SDIO_REGID(arg)  (((arg)>>9) & 0x1FFFF)
    if( GET_SDIO_FUNID(cmd->cmdarg) == 0 && GET_SDIO_REGID(cmd->cmdarg)==0x6)
    {
        ccon |= MCI_CPSM_CMD12_IOABORT;
    }
#endif
    host->cmd = cmd;

    ccon &= ~(0xffffU << MCI_CPSM_RSPWAITCFG);
    ccon |= 0xffffU << MCI_CPSM_RSPWAITCFG;	// note: don't cfg 0 to bit 16-31, or will timeout error;

    if(cmd->cmdidx == MMC_CMD_SEND_OP_COND || cmd->cmdidx == SD_CMD_APP_SEND_OP_COND)
        ccon |= MCI_CPSM_RSPCRC_NOCHK;
    else
         ccon &= ~MCI_CPSM_RSPCRC_NOCHK;

    /* configurate cmd controller register */
    writel(ccon, host->base + MCI_COMMAND_REG);
}

/*
* @BRIEF       akmci_wait_cmd_complete
* @PARAM[in]: struct akmci_host *host
* @PARAM[in]: struct mmc_cmd *cmd
* @RETURN: int
* @RETVAL:
*/
/*
     ———————————————————————————————————------------------------------------------
    | 参数  | Min | Max | unit  |                                                 |
    |NCR时间|  2  |  64 |       |cmd_end到cmd_resp的时间(CMD2和ACMD41除外)         |
    |NID时间|  5  |  5  |       |CMD2和ACMD41的NCR时间                            |
    |NAC时间|  2  |  -  |       |cmd_end到read_data的时间                         |
    |NRC时间|  8  |  -  |       |resp_end到cmd_start的时间(已知问题)               |
    |NCC时间|  8  |  -  | clock |cmd_end到cmd_start的时间(CMD0,CMD4和CMD7)        |
    |NWR时间|  2  |  -  | cycles|resp_end到write_data或者crc_end到write_data的时间 |
    |NSD时间|  2  |  2  |       |multi_read时，CMD12_end到DATA线回归高阻的时间     |
    |——————————————————-       |-------------------------------------------------|
    |      |  1  |  1  |       |multi_write时，CMD12_end到繁忙状态时Data0拉低的时间|
    |NSB时间|  2  |  2  |       |                                                |
    |      |  4  |  4  |       |                                                 |
     ——————————————————————————————————————————————————————————————————————————--
*/
static int akmci_wait_cmd_complete(struct akmci_host *host,
    struct mmc_cmd *cmd)
{
    #define MAX_POLL_TIMES  0xfffff
    unsigned int status;
    unsigned int status_mask;
    int timeout_us = MAX_POLL_TIMES;

    status_mask = MCI_RESPCRCFAIL|MCI_RESPTIMEOUT|MCI_CMDSENT|MCI_RESPEND;
    do {
        status = __raw_readl(host->base + MCI_STATUS_REG);
    } while (!(status & status_mask) && --timeout_us);

    if (timeout_us == 0){ //ETIMEDOUT
        pr_err("%s: timeout (%d us)\n", __func__, timeout_us);
        return -ETIMEDOUT;
    }

    if (cmd->resp_type == MMC_RSP_NONE) {
            if (status & MCI_CMDSENT) {
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
                timeout_us = MAX_POLL_TIMES;
                do {
                    timeout_us--;
                } while(!(__raw_readl(host->base + MCI_STATUS_REG) & MCI_CMDSENT_DLY) && timeout_us);

                if(timeout_us <= 0) {
                    pr_err("%s:cmdSent_dly timeout\n", __func__);
                    return -ETIMEDOUT;
                }
#endif
                return 0;
            }
            else {
                pr_err("send cmd %d error, %s\n", cmd->cmdidx, "cmd not send");
                return  -EIO;
            }
    }else if ((cmd->resp_type & MMC_RSP_PRESENT) ||
        (cmd->resp_type & MMC_RSP_136)) {
            if (status & MCI_RESPTIMEOUT) {
                pr_err("send cmd %d error, cmdtimeout\n", cmd->cmdidx);
                return  -ETIMEDOUT;
            }
            else if (status & MCI_RESPCRCFAIL) {
                pr_err("send cmd %d error, cmdcrcfail\n", cmd->cmdidx);
                return -EILSEQ;
            }
            else if (status & MCI_RESPEND) {
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
                timeout_us = MAX_POLL_TIMES;
                do {
                    timeout_us--;
                } while(!(__raw_readl(host->base + MCI_STATUS_REG) & MCI_CMDRSPEND_DLY) && timeout_us);

                if(timeout_us <= 0) {
                    pr_err("%s:cmdRespEnd_dly timeout\n", __func__);
                    return -ETIMEDOUT;
                }
#endif
                return 0;
            }
    }
    return  -EIO;
}

static void akmci_set_clock(struct akmci_host *host, unsigned int clock);

 /*
 * @BRIEF       akmci_request
 * @PARAM[in]: struct udevice *dev
 * @PARAM[in]: struct mmc_cmd *cmd
 * @PARAM[in]: struct mmc_data *data
 * @RETURN: int
 * @RETVAL:
 */
int akmci_request(struct udevice *dev, struct mmc_cmd *cmd,
        struct mmc_data *data)
{
    struct akmci_host *host = dev_get_priv(dev);
#if !defined(CONFIG_KM01A_CODE) && !defined(CONFIG_3918AV130_CODE)
    struct mmc *mmc = mmc_get_mmc_dev(dev);
#endif
    int ret = 0;
    unsigned int size;

    pr_debug("enter %s\n", __func__);

    /* first check sd controller bus busy state */
    if (sd_trans_busy(host)) {
        pr_err("%s check busy fail\n", __func__);
        return -EBUSY;
    }

#if !defined(CONFIG_KM01A_CODE) && !defined(CONFIG_3918AV130_CODE)
    /* notes:add delay while get card scr command, wait 50 ms */
    if (cmd->cmdidx == SD_CMD_APP_SEND_SCR){
        udelay(50000);
    }
#endif
 
#if 0 // Need it??
    /*
     * ACMD23 set the number of write blocks to be pre-erased
     * before writing for multi-blocks write
     */
    if(cmd->cmdidx == SD_CMD(25)){
        if (send_acmd(mmc, 23, SD_SHORT_RESPONSE, data->blocks) == AK_FALSE ){
            printf("set the number of write blocks to be pre-erased failed!\n");
            return -EIO;
        }
    }
#endif

    akmci_start_command(host, cmd, data);
    if (data) {
        ret = sd_trans_data_cpu(host, data);
    }

    ret = akmci_wait_cmd_complete(host, cmd);
    if (ret) {
        printf("block rw command %d is failed!\n", cmd->cmdidx);
        return ret;
    }

    /* get send sd cmd  response*/
    if (cmd->resp_type & MMC_RSP_PRESENT) {
        if (cmd->resp_type & MMC_RSP_136) {
            /* get long cmd  response*/
            cmd->response[0] = readl(host->base + MCI_RESPONSE0_REG);
            cmd->response[1] = readl(host->base + MCI_RESPONSE1_REG);
            cmd->response[2] = readl(host->base + MCI_RESPONSE2_REG);
            cmd->response[3] = readl(host->base + MCI_RESPONSE3_REG);
        } else {
            /* get short cmd  response*/
            cmd->response[0] = readl(host->base + MCI_RESPONSE0_REG);
        }
    }

    if (data) {
        size = data->blocksize * data->blocks;

        /* tx or rx data transfer use cpu mode  */
        if (data->flags & MMC_DATA_READ)  {
            /* receive data with cpu mode */
            pr_debug("receive data with cpu mode\n");
            ret = sd_rx_data_cpu(host, data->dest, size);
            if(ret)
                return ret;
        }
        else if (data->flags & MMC_DATA_WRITE){
            /* send data with cpu mode */
            pr_debug("send data with cpu mode\n");
            ret = sd_tx_data_cpu(host, data->dest, size);
            if(ret)
                return ret;
        }
    }

#if !defined(CONFIG_KM01A_CODE) && !defined(CONFIG_3918AV130_CODE)
    if(mmc->clock <= 400000){
        dev_dbg(host->dev, "delay 50us for NRC\n");
        udelay(50);     //NRC,need 8 mclk cycles at least;
    }
#endif
    return ret;
}

/*
 * @BRIEF       akmci_set_clock
 * @PARAM[in]: struct akmci_host *host
 * @PARAM[in]: unsigned int clock
 * @RETURN: void
 * @RETVAL:
 */
static void akmci_set_clock(struct akmci_host *host, unsigned int clock)
{
    unsigned int clk, div;
    unsigned int clk_div_h, clk_div_l;

    pr_debug("%s, line:%d, clock=%u\n", __func__, __LINE__,clock);
    if (clock == 0) {
        clk = readl(host->base + MCI_CLK_REG);
        clk &= ~MCI_CLK_ENABLE;
        writel(clk, host->base + MCI_CLK_REG);

        host->bus_clock = 0;
    } else {
        clk = readl(host->base + MCI_CLK_REG);

        clk |= MCI_CLK_ENABLE | MCI_CLK_PWRSAVE ;

        clk &= ~0xffff; /* clear clk div */

        div = host->gclk/clock;

        if (host->gclk % clock)
            div += 1;

        div -= 2;

        /* better to 50% duty */
        clk_div_h = div/2;
        clk_div_l = div - clk_div_h;

        clk |= MMC_CLK_DIVL(clk_div_l) | MMC_CLK_DIVH(clk_div_h);
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
        clk |= MCI_ENABLE|MCI_HS_MODE; //|MCI_MULREAD_STOP_EN; 
#else
        clk |= MCI_ENABLE|MCI_FALL_TRIGGER;
#endif
        writel(clk, host->base + MCI_CLK_REG);

        host->bus_clock = host->gclk / ((clk_div_h+1)+(clk_div_l + 1));

        debug("gclk = %lu Hz. div_l=%u, div_h=%u,host->bus_clock = %u Hz\n",
            host->gclk, clk_div_l, clk_div_h,host->bus_clock);
    }
}

//TODO:
static void powersave_enable(struct akmci_host *host)
{
    unsigned int pwrsave;
    unsigned long flags;

    spin_lock_irqsave(&host->lock, flags);

    pwrsave = readl(host->base + MCI_CLK_REG);
    pwrsave |= MCI_CLK_PWRSAVE;
    writel(pwrsave, host->base + MCI_CLK_REG);

    spin_unlock_irqrestore(&host->lock, flags);

    return;
}

static void powersave_disable(struct akmci_host *host)
{
    unsigned int pwrsave;
    unsigned long flags;

    spin_lock_irqsave(&host->lock, flags);

    pwrsave = readl(host->base + MCI_CLK_REG);
    pwrsave &= ~MCI_CLK_PWRSAVE;
    writel(pwrsave, host->base + MCI_CLK_REG);

    spin_unlock_irqrestore(&host->lock, flags);

    return;
}

/*
 * @BRIEF       akmci_set_ios
 * @PARAM[in]: struct udevice *dev
 * @RETURN: int
 * @RETVAL:
 */
static int akmci_set_ios(struct udevice *dev)
{
    struct akmci_host *host = dev_get_priv(dev);
    struct mmc *mmc = mmc_get_mmc_dev(dev);
    unsigned int val;
    unsigned int initclk = 0;

    if(mmc->clock <= 400000 && mmc->clock > 0)
        initclk = 1;

    if(mmc->selected_mode == SD_LEGACY || mmc->selected_mode ==  MMC_LEGACY) {
        pr_debug("Set SD/MMC LEGACY mode.\n");
        val = readl(host->base + MCI_CLK_REG);
        val |= MCI_HS_MODE;
        writel(val, host->base + MCI_CLK_REG);
    } else if(mmc->selected_mode == SD_HS || mmc->selected_mode == MMC_HS) {
        pr_debug("Set SD/MMC High speed mode.\n");
        val = readl(host->base + MCI_CLK_REG);
        val &= (~MCI_HS_MODE);
        writel(val, host->base + MCI_CLK_REG);
    } else {
        pr_err("Set SD/MMC unknown mode, %d\n", mmc->selected_mode);
    }

    if (!mmc->clock || mmc->clock != host->bus_clock) {
        akmci_set_clock(host, mmc->clock);
        host->bus_clock = mmc->clock;
    }

    /* configure the bus width (host side) later in data transfer */
    pr_debug("host->bus_width=%d,mmc->bus_width=%d\n", 
        host->bus_width, mmc->bus_width);

    host->bus_width = mmc->bus_width;

    if(initclk) {
        powersave_disable(host);
        mdelay(1); //output at least 74clks before card init.
        powersave_enable(host);
    }

    return 0;
}

/*
 * #define CARD_UNPLUGED        0
 * #define CARD_PLUGED          1
 * prenset = 1
 */
static int akmci_card_present(struct udevice *dev)
{
    struct akmci_host *host = dev_get_priv(dev);
    unsigned int card_pwr_en = CARD_POWER_ENABLE;

    if(host->gpio_power.gpio >=0)
        card_pwr_en = (host->gpio_power.value)^(host->gpio_power.power_invert);

    pr_debug("akmci_card_present, card_pwr_en=%d, \
        card_status=%d(1=CARD_PLUGED, 0=CARD_UNPLUGED)\n",
        card_pwr_en,host->card_status );
    return (host->card_status && card_pwr_en);
}

/*
 * return 1 CARD_PLUGED.
 * return 0 CARD_UNPLUGED.
 */
int _get_card_status(struct akmci_host *host)
{
    unsigned int i, status[3], detect_retry_count = 0;
    unsigned int mci_status = CARD_UNPLUGED;

    while (1) {
        for (i = 0; i < 3; i++) {
            if(host->detect_mode == AKMCI_DETECT_MODE_CLK)
            {
#if (defined(CONFIG_3918AV100_CODE) || defined(CONFIG_3918EV300L_CODE))
                status[i] = readl(MCI_PLUGINOFF_REG1);
                mci_status = status[i] & (1 <<
                    (MCI0_PLUG_STATUS_OFFSET - host->id));
#elif defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
                status[i] = readl(host->base + MCI_STATUS_REG);
                mci_status = status[i] & (MCI_PIOSTA);
#else
                unsigned int mci_active;
                status[i] = readl(MCI_PLUGINOFF_REG0);
                pr_debug("#status[%d](0x08000080)=%x\n", i, status[i]);
                mci_active = status[i] & (1 << (MCI_PLUG_ACTIVE_OFFSET +
                    host->id));
                mci_status = status[i] & (1 << (MCI_PLUG_STATUS_OFFSET +
                    host->id));
                if(mci_active)
                {
                    printf( "Data transmission is in progress. MCI_CLK will be \
                        pulled down, and can not function as the \
                        detection pin.!");
                    //"Data transmission is in progress" treat as CARD_PLUGED(1)
                    return CARD_PLUGED;
                }
#endif
                status[i] = mci_status;
            }
            else if(host->detect_mode == AKMCI_DETECT_MODE_GPIO)
            {
                status[i] = gpio_get_value(host->gpio_cd);
            }
            else if(host->detect_mode == AKMCI_DETECT_MODE_D3)
            {
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
                status[i] = readl(host->base + MCI_STATUS_REG);
                mci_status = status[i] & (MCI_PIOSTA);
#else
                status[i] = readl(MCI_PLUGINOFF_REG1);
                mci_status = status[i] & (1 << (MCI0_PLUG_STATUS_OFFSET -
                    host->id));
#endif
                pr_debug("#status[%d](0x08000080)=%x\n", i, status[i]);

                status[i] = mci_status;
            }
            udelay(10);
        }

        if ((status[0] == status[1]) && (status[0] == status[2]))
        break;

        detect_retry_count++;
        if (detect_retry_count >= 5) {
            pr_err("this is a dithering,card detect error!\n");
            return CARD_UNPLUGED;  // err condition,treat as CARD_UNPLUGED(0)
        }
    }

#if (defined(CONFIG_3918AV100_CODE) || defined(CONFIG_3918EV300L_CODE) || \
        defined(CONFIG_KM01A_CODE)) || defined(CONFIG_3918AV130_CODE)
    if((host->detect_mode == AKMCI_DETECT_MODE_CLK) ||
        (host->detect_mode == AKMCI_DETECT_MODE_D3)){
        /*
         * status register val
         * 0: pluged off;
         * 1: pluged in.
         *
         * _get_card_status return 1 CARD_PLUGED, return 0   CARD_UNPLUGED
         */
        return !!status[0];
    }
    else // host->detect_mode == AKMCI_DETECT_MODE_GPIO
#endif
    return !status[0];
    //TODO:polarity definition:low level when plugin
}

/*
 * @BRIEF       akmci_setup_cfg
 * @PARAM[in]: struct akmci_host *host
 * @RETURN: void
 * @RETVAL:
 */
static void akmci_setup_cfg(struct akmci_host *host)
{
    struct mmc_config *cfg = &host->plat->cfg;

    cfg->name = host->dev->name;
    /* set mmc/sd controller parameters
    *  host_caps:support high speed and high capacity
    *  f_min:mini speed 400kHz
    *  f_max:max speed 52MHz
    */
    //cfg->host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS;

    if(host->bus_width ==8)
        cfg->host_caps |= MMC_MODE_8BIT;
    if(host->bus_width ==4)
        cfg->host_caps |= MMC_MODE_4BIT;
    if(host->bus_width ==1)
        cfg->host_caps |= MMC_MODE_1BIT;

    cfg->f_min = AKMCI_MIN_FREQ;

    if (dev_read_u32(host->dev, "max-frequency", &cfg->f_max) != 0) {
        if (cfg->host_caps & MMC_MODE_HS) {
            if (cfg->host_caps & MMC_MODE_HS_52MHz)
                cfg->f_max = 52000000;
            else
                cfg->f_max = 26000000;
        } else
            cfg->f_max = 20000000;
    }
    /*
     * set mmc/sd controller parameters
     */
    cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;

    cfg->b_max  = CONFIG_SYS_MMC_MAX_BLK_COUNT;

    mmc_of_parse(host->dev, cfg);
}

/*
 * @BRIEF       akmci_init_host_cfg
 * @PARAM[in]: struct akmci_host *host
 * @RETURN: int
 * @RETVAL:
 */
static int akmci_init_host_cfg(struct akmci_host *host)
{
    int ret = 0;
    /*1. set mci_pin to idle state: gpio, pupd disable, input disable.*/
    ret |= pinctrl_select_state(host->dev,"idle");

    /*2. disable card.*/
    host->gpio_power.value  = 
        host->gpio_power.power_invert ^ (CARD_POWER_ENABLE);
    ret |= gpio_direction_output(host->gpio_power.gpio,
        !host->gpio_power.value);

    /*3. sleep.*/
    mdelay(100);

    /*4. enable card.*/
    ret |= gpio_direction_output(host->gpio_power.gpio, host->gpio_power.value);

    /*5. default state: mci function.*/
    ret |= pinctrl_select_state(host->dev,"default");

    return ret;
}

/*
 * @BRIEF       akmci_probe
 * @PARAM[in]: struct udevice *dev
 * @RETURN: int
 * @RETVAL:
 */
static int  akmci_probe(struct udevice *dev)
{
    struct akmci_plat *plat = dev_get_platdata(dev);
    struct akmci_host *host = dev_get_priv(dev);
    struct mmc *mmc = mmc_get_mmc_dev(dev);
    struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
    int ret;
    unsigned int array[3] = {0};
    struct clk clk;

    host->dev = dev;
    host->mmc = mmc;
    host->mmc->priv = host;
    host->plat = plat;
    upriv->mmc = &plat->mmc;

    host->phys_addr = devfdt_get_addr(dev);
    if (host->phys_addr == FDT_ADDR_T_NONE){
        pr_err("return -EINVAL\n");
        return -EINVAL;
    }

    ret = clk_get_by_index(dev, 0, &clk);
    if (ret)
        return ret;
    ret = clk_enable(&clk);
    if (ret)
        return ret;
    host->gclk = clk_get_rate(&clk);
    debug("MCI: gclk %ld\n", host->gclk);
    if (!host->gclk)
        return -EINVAL;

    debug("host->gclk = %lu\n", host->gclk);
    host->base = devm_ioremap(dev, host->phys_addr, SZ_256);
    if (!host->base){
        pr_err("return -ENOMEM\n");
        return -ENOMEM;
    }

    /* host relate info*/
    host->bus_width = fdtdec_get_int(gd->fdt_blob,
        dev_of_offset(dev),"bus-width", 4);
    host->id= fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),"mci_id", 0);
    ret = fdtdec_get_int_array(gd->fdt_blob, dev_of_offset(dev), "power-pins",
        array, 3);
    if (ret < 0){
        pr_info("%s %d , power-pins not exist\n", __func__, __LINE__);
        host->gpio_power.gpio = -1;
        host->gpio_power.power_invert = -1;
    }
    else
    {
        host->gpio_power.gpio = array[1];
        host->gpio_power.power_invert = fdtdec_get_bool(gd->fdt_blob,
            dev_of_offset(dev), "power-inverted");

        if(host->gpio_power.gpio  >= 0){
            ret = gpio_request(host->gpio_power.gpio, "mci-power-pins");
            if (ret) {
            pr_err("%s %d, gpio request power-pins:%d fail\n",
                        __func__, __LINE__, host->gpio_power.gpio);
            return ret;
            }
        }

        host->gpio_power.value = 
            host->gpio_power.power_invert ^ (CARD_POWER_ENABLE);
        ret = gpio_direction_output(host->gpio_power.gpio, host->gpio_power.value);
    }

    if(fdtdec_get_bool(gd->fdt_blob, dev_of_offset(dev), "cd_clk")){
        host->detect_mode = AKMCI_DETECT_MODE_CLK;
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
        unsigned int val = readl(host->base + MCI_PLUGINOFF_REG);
        val &= ~(1 << 24); // dat3 enable bit
        val |= (1 << 25); // mclk enable bit
        val &= ~(0xffff);
        val |= 0xff;
        writel(val , host->base + MCI_PLUGINOFF_REG);
#else
        unsigned long val = readl(MCI_PLUGINOFF_REG0);      /*
        * config detect mode.
        * threshold and polarity use default value.
        *
        *  For MCLK:    A SD card is plugged in  when the level is low
        *  For D3:      A SD card is plugged in  when the level is high
        */
        val |= ( 1 <<  (MCI0_CLK_DETECT_ENABLE_OFFSET - host->id));
        val &= ~( 1 << (MCI0_D3_DETECT_ENABLE_OFFSET  - host->id));
        writel(val, MCI_PLUGINOFF_REG0);
#endif
    }else{
        ret = fdtdec_get_int_array(gd->fdt_blob, dev_of_offset(dev),
            "detect-gpio", array, 3);
        if(ret == 0){
                host->detect_mode = AKMCI_DETECT_MODE_GPIO;
                host->gpio_cd = array[1];
                ret = gpio_request(host->gpio_cd, "mci-detect-gpio");
                if (ret) {
                    pr_err("%s %d, gpio request detect-gpio:%d fail\n",
                            __func__, __LINE__, host->gpio_cd);
                    return ret;
                }
        }else if(fdtdec_get_bool(gd->fdt_blob, dev_of_offset(dev),
            "non-removable")){
                host->detect_mode = AKMCI_PLUGIN_ALWAY;
        }else if(fdtdec_get_bool(gd->fdt_blob, dev_of_offset(dev), "cd_d3")){
        host->detect_mode = AKMCI_DETECT_MODE_D3;
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
        unsigned int val = readl(host->base + MCI_PLUGINOFF_REG);
        val |= (1 << 24); // dat3 enable bit
        val &= ~(1 << 25); // mclk enable bit
        val &= ~(0xffff);
        val |= 0xff;
        writel(val, host->base + MCI_PLUGINOFF_REG);
#else
        unsigned long val = readl(MCI_PLUGINOFF_REG0);      /*
        * config detect mode.
        * threshold and polarity use default value.
        *
        *  For MCLK:    A SD card is plugged in  when the level is low
        *  For D3:      A SD card is plugged in  when the level is high
        */
        val |= ( 1 <<  (MCI0_D3_DETECT_ENABLE_OFFSET   - host->id));
        val &= ~( 1 << (MCI0_CLK_DETECT_ENABLE_OFFSET  - host->id));
        writel(val, MCI_PLUGINOFF_REG0);
#endif
        }else{
            host->detect_mode = AKMCI_PLUGIN_ALWAY;
        }
    }

    host->card_status = _get_card_status(host);
    pr_err("@get_card_status, card %s\n",
        (host->card_status?"PLUGED":"UNPLUGED"));

    if(host->gpio_power.gpio >= 0) {
        debug("id=%d, base=%p, bus_width=%d, gpio_power=%d(power_invert=%d), \
            detect_mode=%d, gpio_cd=%d, card_status=%d\n",
            host->id, host->base, host->bus_width, host->gpio_power.gpio,
            host->gpio_power.power_invert, host->detect_mode,
            (host->detect_mode == AKMCI_DETECT_MODE_GPIO ? host->gpio_cd:-1),
            host->card_status);
        /* No sdcard present */
        if(!host->card_status)
        {
           pinctrl_select_state(host->dev,"idle");

           host->gpio_power.value  =
               host->gpio_power.power_invert ^ (CARD_POWER_ENABLE);
           ret = gpio_direction_output(host->gpio_power.gpio,
               !host->gpio_power.value);
           return 0;
        }
    } else {
        debug("id=%d, base=%p, bus_width=%d,detect_mode=%d, gpio_cd=%d, \
            card_status=%d\n",
            host->id, host->base, host->bus_width,host->detect_mode,
            (host->detect_mode == AKMCI_DETECT_MODE_GPIO ? host->gpio_cd:-1),
            host->card_status);
    }

    akmci_setup_cfg(host);

    if(host->gpio_power.gpio >= 0) {
        ret = akmci_init_host_cfg(host);
        if (ret) {
            pr_err("%s: Err!\n", __func__);
            return ret;
        }
    } else {
        pinctrl_select_state(host->dev, "default");
    }

    /* initial sd interface default clock from get current asic clock
    *   and open this sd system module clock
    *   and enable power save
    */
    akmci_set_clock(host, SD_IDENTIFICATION_MODE_CLK);
    debug("%s -> OK\n", __func__);
    akmci_dumpregs(host);
    return 0;
}/* end of func */


static const struct dm_mmc_ops akmci_ops = {
    .get_cd    = akmci_card_present,
    .send_cmd = akmci_request,
    .set_ios = akmci_set_ios,
};

/*
 * @BRIEF       akmci_bind
 * @PARAM[in]: struct udevice *dev
 * @RETURN: int
 * @RETVAL:
 */
static int akmci_bind(struct udevice *dev)
{
    struct akmci_plat *plat = dev_get_platdata(dev);

    return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id akmci_match[] = {
    { .compatible = "anyka,ak37d-mmc0" },
    { .compatible = "anyka,ak37d-mmc1" },
    { .compatible = "anyka,ak37d-mmc2" },
    { .compatible = "anyka,ak39ev330-mmc0" },
    { .compatible = "anyka,ak39ev330-mmc1" },
    { .compatible = "anyka,ak37e-mmc0" },
    { .compatible = "anyka,ak37e-mmc1" },
    { .compatible = "anyka,ak37e-mmc2" },
    { .compatible = "anyka,ak3918av100-mmc0" },
    { .compatible = "anyka,ak3918av100-mmc1" },
    { .compatible = "anyka,ak3918av100-mmc2" },
    { .compatible = "anyka,ak3918av100v2-mmc0" },
    { .compatible = "anyka,ak3918av100v2-mmc1" },
    { .compatible = "anyka,ak3918av100v2-mmc2" },
    { .compatible = "anyka,ak3918ev300l-mmc0" },
    { .compatible = "anyka,ak3918ev300l-mmc1" },
    { .compatible = "anyka,ak3918ev300l-mmc2" },
    { .compatible = "anyka,ak39ev200-mmc0" },
    { .compatible = "anyka,ak39ev200-mmc1" },
    { .compatible = "anyka,ak3918av130-mmc0" },
    { .compatible = "anyka,ak3918av130-mmc1" },
    { .compatible = "anyka,ak3918av130-mmc2" },
    { }
};

U_BOOT_DRIVER(akmci_drv) = {
    .name       = "akmci",
    .id     = UCLASS_MMC,
    .of_match   = akmci_match,
    .bind       = akmci_bind,
    .probe      = akmci_probe,
    .priv_auto_alloc_size = sizeof(struct akmci_host),
    .platdata_auto_alloc_size = sizeof(struct akmci_plat),
    .ops        = &akmci_ops,
};
#endif
