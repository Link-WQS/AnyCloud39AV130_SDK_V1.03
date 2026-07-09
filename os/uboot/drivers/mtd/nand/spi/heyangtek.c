#ifndef __UBOOT__
#include <linux/device.h>
#include <linux/kernel.h>
#endif
#include <linux/mtd/spinand.h>

#ifdef CONFIG_ANYKA

#define SPINAND_MFR_HEYANGTEK 0xC9

static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_QUADIO_OP(0, 2, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_DUALIO_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
		SPINAND_PROG_LOAD(false, 0, NULL, 0));

static int HYFxGQxUAAxE_ecc_get_status(struct spinand_device *spinand,
					u8 status)
{
    int ret = -EINVAL;
    if (!spinand) 
    {
        return -EINVAL;
    }

    if (STATUS_ECC_UNCOR_ERROR == (status & STATUS_ECC_MASK))
    {
        ret = -EBADMSG;
    } else {
        ret = 0;
    }
	return ret;
}

static int HYFxGQxUAAxE_ooblayout_ecc(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 64;
	region->length = 64;

	return 0;
}

static int HYFxGQxUAAxE_ooblayout_free(struct mtd_info *mtd, int section,
					struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 1 bytes for the BBM. */
	region->offset = 0;
	region->length = 128;

#ifdef CONFIG_ANYKA	
	region->oob_badoffset = 4;
	region->oob_seglen = 0;
	region->oob_skiplen = 24;
#endif
	return 0;
}

static const struct mtd_ooblayout_ops HYFxGQxUAAxE_ooblayout = {
	.ecc = HYFxGQxUAAxE_ooblayout_ecc,
	.free = HYFxGQxUAAxE_ooblayout_free,
};

static const struct spinand_info heyangtek_spinand_table[] = {

    SPINAND_INFO("HYF4GQ4UAACAE", 0x54,
		     NAND_MEMORG(1, 2048, 128, 64, 4096, 1, 1, 1),
		     NAND_ECCREQ(14, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&HYFxGQxUAAxE_ooblayout,
				     HYFxGQxUAAxE_ecc_get_status)),

};

/**
 * heyangtek_spinand_detect - initialize device related part in spinand_device
 * struct if it is a heyangtek device.
 * @spinand: SPI NAND device structure
 */
static int heyangtek_spinand_detect(struct spinand_device *spinand)
{
	u8 *id = spinand->id.data;
	int ret;

	/*
	 * heyangtek SPI NAND read ID need a dummy byte,
	 * so the first byte in raw_id is dummy.
	 */
	if (id[0] != SPINAND_MFR_HEYANGTEK)
		return 0;

	ret = spinand_match_and_init(spinand, heyangtek_spinand_table,
				     ARRAY_SIZE(heyangtek_spinand_table), id[1]);
	if (ret)
		return ret;

	return 1;
}

static const struct spinand_manufacturer_ops heyangtek_spinand_manuf_ops = {
	.detect = heyangtek_spinand_detect,
	// .init = heyangtek_spinand_init,
};

const struct spinand_manufacturer heyangtek_spinand_manufacturer = {
	.id = SPINAND_MFR_HEYANGTEK,
	.name = "heyangtek",
	.ops = &heyangtek_spinand_manuf_ops,
};       

#endif