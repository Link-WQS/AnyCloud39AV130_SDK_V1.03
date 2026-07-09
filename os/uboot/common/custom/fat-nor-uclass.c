/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-06-15 14:18:44
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-06-28 19:13:27
 * @FilePath: /uboot/drivers/demo/nor_fat_uclass.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <spi.h>
#include <dm.h>
#include <asm/io.h>
#include <linux/list.h>
#include "spi_flash.h"
#include <common.h>
#include <jffs2/load_kernel.h>
#include <dm/device-internal.h>

static struct spi_flash *flash = NULL;
static struct part_info *part = NULL;

static int nor_fat_blk_probe(struct udevice *dev)
{
    int ret = 0;
    struct mtd_device *mtd_dev;
    u8 pnum;
    flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
                            CONFIG_SF_DEFAULT_CS,
                            CONFIG_SF_DEFAULT_SPEED,
                            CONFIG_SF_DEFAULT_MODE);

    if (!flash)
    {
        puts("Nor probe failed.\n");
        return -ENODEV;
    }

    mtdparts_init();
    if (find_dev_and_part("FAT", &mtd_dev, &pnum, &part) != 0)
    {
        puts("Find part FAT failed.\n");
        return -ENODEV;
    }
    printf("Part:%s,Start:0x%llx,Size:%lluKB!!!\n", part->name, part->offset, part->size / 1024);

    struct blk_desc *bdesc = dev_get_uclass_platdata(dev);
    if (!bdesc)
    {
        return -ENODEV;
    }
    bdesc->blksz = 4096;
    bdesc->log2blksz = LOG2(bdesc->blksz);
    bdesc->lba = (part->size) / bdesc->blksz;
    return ret;
}

ulong nor_fat_bread(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
                    void *dst)
{
    if (flash)
    {
        struct blk_desc *bdesc = dev_get_uclass_platdata(dev);
        if (bdesc)
        {
            // printf("[%s]start:0x%lx,len:0x%lx\n", __func__, part->offset + start * bdesc->blksz, blkcnt * bdesc->blksz);
            spi_flash_read(flash, part->offset + start * bdesc->blksz, blkcnt * bdesc->blksz, dst);
        }
    }
    return blkcnt;
}

ulong nor_fat_bwrite(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
                     const void *src)
{
    if (flash)
    {
        struct blk_desc *bdesc = dev_get_uclass_platdata(dev);
        if (bdesc)
        {
            // printf("[%s]start:0x%llx,len:%lu\n", __func__, part->offset + start * bdesc->blksz, blkcnt * bdesc->blksz);
            spi_flash_erase(flash, part->offset + start * bdesc->blksz, blkcnt * bdesc->blksz);
            spi_flash_write(flash, part->offset + start * bdesc->blksz, blkcnt * bdesc->blksz, src);
        }
    }
    return blkcnt;
}

static const struct blk_ops nor_fat_blk_ops = {
    .read = nor_fat_bread,
    .write = nor_fat_bwrite,
};

static int nor_fat_bind(struct udevice *dev)
{
    int ret, devnum = -1;
    struct udevice *bdev;
    ret = blk_create_devicef(dev, "nor_fat_blk", "blk", IF_TYPE_NOR,
                             devnum, 4096, 0, &bdev);
    return ret;
}

UCLASS_DRIVER(nor_fat) = {
    .name = "nor_fat",
    .id = UCLASS_NOR,
};

U_BOOT_DRIVER(nor_fat_drv) = {
    .name = "nor_fat_drv",
    .id = UCLASS_NOR,
    .bind = nor_fat_bind,
};

U_BOOT_DEVICE(nor_fat) = {
    .name = "nor_fat_drv",
};

U_BOOT_DRIVER(nor_fat_blk) = {
    .name = "nor_fat_blk",
    .id = UCLASS_BLK,
    .probe = nor_fat_blk_probe,
    .ops = &nor_fat_blk_ops,
};