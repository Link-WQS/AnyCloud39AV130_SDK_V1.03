#include <linux/module.h>     //支持动态添加和卸载模块
#include <linux/kernel.h>    //驱动要写入内核，与内核相关的头文件
#include <linux/init.h>      //初始化头文件

#include <linux/fs.h>        //包含了文件操作相关struct的定义
#include <linux/types.h>     //对一些特殊类型的定义
#include <linux/fcntl.h>     //定义了文件操作等所用到的相关宏
#include <linux/vmalloc.h>  //vmalloc()分配的内存虚拟地址上连续，物理地址不连续
#include <linux/blkdev.h>  //采用request方式 块设备驱动程序需要调用blk_init_queue 分配请求队列
#include <linux/hdreg.h> //硬盘参数头文件，定义访问硬盘寄存器端口、状态码和分区表等信息。
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>

#include <mach/map.h>

#define RAMHD_NAME              "reserved_ram"    //设备名称
#define RAMHD_MAX_DEVICE        1           //最大设备数
#define RAMHD_MAX_PARTITIONS    1           //最大分区数

#define RAMHD_SECTOR_SIZE       512        //扇区大小
//扇区数  http://www.embedu.org/Column/Column863.htm
#define RAMHD_SECTORS           16
#define RAMHD_HEADS             4         //磁头数
#define RAMHD_CYLINDERS         256      //磁道(柱面)数 
//总大小
#define RAMHD_SECTOR_TOTAL      (RAMHD_SECTORS * RAMHD_HEADS * RAMHD_CYLINDERS)
#define RAMHD_SIZE              (RAMHD_SECTOR_SIZE * RAMHD_SECTOR_TOTAL) //8MB

//#define max(a, b) ((a) > (b) ? (a) : (b))
//#define min(a, b) ((a) < (b) ? (a) : (b))

/**
 * @RAMHD_DEV print functions
 * 1. *data,设备数据空间首地址
 * 2. *queue;设备请求队列
 * 3.  lock; 互斥自旋锁
 * 4.  *gd;  通用磁盘结构体
 */
typedef struct {
    unsigned char   *data;
    struct request_queue *queue;
    spinlock_t      lock;
    struct gendisk  *gd;
} RAMHD_DEV;
/*分配内存的首地址*/
static char *sdisk[RAMHD_MAX_DEVICE];
/*分配内存的首地址*/
static RAMHD_DEV *rdev[RAMHD_MAX_DEVICE];
/*主设备号*/
static dev_t ramhd_major;

static unsigned long disk_addr = 0x81400000;
static unsigned long disk_size = 0x2000000;
module_param(disk_addr, ulong, S_IRUGO);
module_param(disk_size, ulong, S_IRUGO);

static ssize_t ramdisk_info_show(struct device *dev,
                                 struct device_attribute *attr,char *buf)
{
    return sprintf(buf, "disk_addr:0x%lx disk_size:0x%lx\n",
                   disk_addr, disk_size);
}

static DEVICE_ATTR(ramdisk_info, 0400, ramdisk_info_show, NULL);

static struct attribute *akramdisk_attributes[] = {
    &dev_attr_ramdisk_info.attr,
    NULL,
};

static const struct attribute_group akramdisk_attrs = {
    .attrs = akramdisk_attributes,
};

static int ramhd_space_init(void)
{
    int i;
    int err = 0;
    for(i = 0; i < RAMHD_MAX_DEVICE; i++) {
        sdisk[i] = (void *)CONFIG_PAGE_OFFSET + \
                   (disk_addr - CONFIG_PHYS_OFFSET);
        if(!sdisk[i]) {
            err = -ENOMEM;  //errno:12 内存不足
            return err;
        }
        //memset(sdisk[i], 0, disk_size);
        disk_addr += disk_size;
    }

    return err;
}

static void ramhd_space_clean(void)
{

}

static int alloc_ramdev(void)
{
    int i;
    for(i = 0; i < RAMHD_MAX_DEVICE; i++) {
        /*向内核申请存放RAMHD_DEV结构体的内存空间*/
        rdev[i] = kzalloc(sizeof(RAMHD_DEV),
                          GFP_KERNEL); 
        if(!rdev[i])
            return -ENOMEM;   //errno:12  内存不足
    }
    return 0;
}

static void clean_ramdev(void)
{
    int i;
    for(i = 0; i < RAMHD_MAX_DEVICE; i++) {
        if(rdev[i])
            /*释放分配的内存*/
            kfree(rdev[i]);
    }
}
/*设备打开用到*/
int ramhd_open(struct block_device *bdev, fmode_t mode)
{
    return 0;
}
void ramhd_release(struct gendisk *gd, fmode_t mode)
{
    return;
}

//IO控制
static int ramhd_ioctl(struct block_device *bdev, fmode_t mode,
                       unsigned int cmd, unsigned long arg)
{
    int err; 
    //hd_geometry结构体包含磁头，扇区，柱面等信息
    struct hd_geometry geo; 

    switch(cmd) {
    case HDIO_GETGEO:  //获取块设备的物理参数
         //检查指针所指向的存储块是否可写
        err = !access_ok(VERIFY_WRITE, arg, sizeof(geo));
        if(err) return -EFAULT;     //errno:14   地址错

        geo.cylinders = RAMHD_CYLINDERS;     //柱面数
        geo.heads = RAMHD_HEADS;            //磁头数
        geo.sectors = RAMHD_SECTORS;        //扇区数
        geo.start = get_start_sect(bdev);   //起始地址
        if(copy_to_user((void *)arg, &geo, sizeof(geo)))
            //把内核地址&geo指示的数据复制到arg指代的用户空间的地址上
            return -EFAULT;   //errno:14   地址错
        return 0;
    }
    //errno:25     不适当的IO控制操作
    return -ENOTTY;      
}
                       
/*用来描述一个块设备的操作函数集*/
static struct block_device_operations ramhd_fops
    = { 

    .owner = THIS_MODULE,
    .open = ramhd_open,
    .release = ramhd_release,
    .ioctl = ramhd_ioctl,
};

/*处理传递给这个设备的请求*/
void ramhd_req_func (struct request_queue *q)
{
    //用来提取req
    struct request *req;  
    RAMHD_DEV *pdev;
    char *pData;
    unsigned long addr, size, start;
    req = blk_fetch_request(q); //从块设备队列提取存储的req;
    
   /*判断当前request是否合法  循环从请求队列中获取下一个要处理的请求*/
    while (req) {  
        // 获取当前request结构的起始扇区
        start = blk_rq_pos(req);
        //获得设备结构体指针
        pdev = (RAMHD_DEV *)req->rq_disk->private_data; 
        //设备地址
        pData = pdev->data;
        addr = (unsigned long)pData + start * RAMHD_SECTOR_SIZE;//计算地址

        size = blk_rq_cur_bytes(req); //访问 req 的下一段数据
        //获得数据传送方向.返回0表示从设备读取,否则表示写向设备.
          if (rq_data_dir(req) == READ)
            memcpy(bio_data(req->bio), (char *)addr, size); //读
        else
            memcpy((char *)addr, bio_data(req->bio), size); //写
        //这个函数处理完返回false
        if(!__blk_end_request_cur(req, 0)) 
            //继续取出请求队列中的请求 
            req = blk_fetch_request(q); 
    }
}

static int akram_probe(struct platform_device *pdev)
{
    int i;
    int ret;
    u32 rm_size;
    u32 rm_addr;
    u32 addr_low;
    u32 addr_high;

    struct device_node *memory;
    struct device_node* np = pdev->dev.of_node;

    memory = of_parse_phandle(np, "memory-region", 0);
    if (!memory)
        return -ENODEV;
    rm_addr = of_translate_address(memory, of_get_address(memory, 0,
                                   (u64 *)&rm_size, NULL));
    dev_info(&pdev->dev, "reserved_addr = %x, reserved_size =%x\n", rm_addr,
             rm_size);

    addr_low =  rm_addr;
    addr_high = rm_addr + rm_size;

    if((disk_addr < addr_low) || ((disk_addr + disk_size)  > addr_high)) {
        disk_addr = addr_low;
        disk_size = addr_high - addr_low;
    }
    dev_info(&pdev->dev,"using %x - %x as ramdisk\n", addr_low, addr_high);

    ret = ramhd_space_init();
    if(ret < 0) {
        dev_err(&pdev->dev,"ramhd_space_init fail!\n");
        return -1;
    }

    ret = alloc_ramdev();
    if(ret < 0) {
        dev_err(&pdev->dev,"alloc_ramdev fail!\n");
        return -1;
    }

    //块设备驱动注册到内核中
    ramhd_major = register_blkdev(0, RAMHD_NAME);
    //major为0，内核会自动分配一个新的主设备号 ramhd_major
    for(i = 0; i < RAMHD_MAX_DEVICE; i++) {
        rdev[i]->data = sdisk[i];
        rdev[i]->gd = alloc_disk(RAMHD_MAX_PARTITIONS);
        spin_lock_init(&rdev[i]->lock);  //初始化自旋锁
        //初始化将ramhd_req_func函数与队列绑定
        rdev[i]->queue = blk_init_queue(ramhd_req_func,&rdev[i]->lock);
        rdev[i]->gd->major = ramhd_major;
        rdev[i]->gd->first_minor = i * RAMHD_MAX_PARTITIONS;
        //关联到这个设备的方法集合
        rdev[i]->gd->fops = &ramhd_fops;  
        rdev[i]->gd->queue = rdev[i]->queue;
        //使用这个成员来指向分配的数据
        rdev[i]->gd->private_data = rdev[i];
        sprintf(rdev[i]->gd->disk_name, "reserved_ram%d", i);
        //set_capacity(rdev[i]->gd, RAMHD_SECTOR_TOTAL);
        set_capacity(rdev[i]->gd, disk_size/RAMHD_SECTOR_SIZE);
        //向系统中添加这个块设备
        add_disk(rdev[i]->gd); 
    }

    sysfs_create_group(&pdev->dev.kobj, &akramdisk_attrs);

    return 0;
}

static int akram_remove(struct platform_device *pdev)
{
    int i;

    sysfs_remove_group(&pdev->dev.kobj, &akramdisk_attrs);

    for(i = 0; i < RAMHD_MAX_DEVICE; i++) {
        //删除gendisk结构体
        del_gendisk(rdev[i]->gd); 
        //减少gendisk结构体的引用计数
        put_disk(rdev[i]->gd); 
    //清除请求对列
        blk_cleanup_queue(rdev[i]->queue); 
    } 
    //注销块设备
    unregister_blkdev(ramhd_major,RAMHD_NAME);
    clean_ramdev();
    ramhd_space_clean();

    return 0;

}

#if defined(CONFIG_OF)
static const struct of_device_id akram_match[] = {
    { .compatible = "anyka,ak37d-ramdisk" },
    { .compatible = "anyka,ak37e-ramdisk" },
    { .compatible = "anyka,ak39ev330-ramdisk"},
    { .compatible = "anyka,ak3918av100-ramdisk"},
    { .compatible = "anyka,ak3918av130-ramdisk"},
    { .compatible = "anyka,ak3918ev300l-ramdisk"},
    { .compatible = "anyka,km01a-ramdisk"},
    { }
};
MODULE_DEVICE_TABLE(of, akram_match);
#endif

static struct platform_driver akram_driver = {
    .probe  = akram_probe,
    .remove = akram_remove,
    .driver     = {
        .name   = "ak-ramdisk",
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(akram_match),
    },
};

/**
 * @brief registe rakram_driver to platform
 * @return register status..
 */
  //模块加载函数
static int __init ramhd_init(void)
{
    return platform_driver_register(&akram_driver);
}
/**
 * @release akram_driver from platform
 * @return void;
 */  
 //模块卸载函数
void ramhd_exit(void)
{
    return platform_driver_unregister(&akram_driver);
}

module_init(ramhd_init);
module_exit(ramhd_exit);


MODULE_AUTHOR("Anyka Microelectronic Ltd.");
MODULE_DESCRIPTION("Anyka Ramdisk Driver.");
MODULE_LICENSE("GPL");
MODULE_VERSION("2.0.00");
