#ifndef __UBOOT__
#include <linux/device.h>
#include <linux/kernel.h>
#endif
#include <linux/mtd/spinand.h>

#ifdef CONFIG_ANYKA

#define SPINAND_MFR_GSTO 0x01

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

static int GSS0xGSAM0_ecc_get_status(struct spinand_device *spinand,
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

static int GSS01GSAM0_ooblayout_ecc(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 64;
	region->length = 64;

	return 0;
}

static int GSS01GSAM0_ooblayout_free(struct mtd_info *mtd, int section,
					struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 1 bytes for the BBM. */
	region->offset = 0;
	region->length = 64;

#ifdef CONFIG_ANYKA	
	region->oob_badoffset = 4;
	region->oob_seglen = 12;
	region->oob_skiplen = 4;
#endif
	return 0;
}

static const struct mtd_ooblayout_ops GSS01GSAM0_ooblayout = {
	.ecc = GSS01GSAM0_ooblayout_ecc,
	.free = GSS01GSAM0_ooblayout_free,
};

static const struct spinand_info gsto_spinand_table[] = {

    SPINAND_INFO("GSS01GSAM0", 0x15,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&GSS01GSAM0_ooblayout,
				     GSS0xGSAM0_ecc_get_status)),

};

/**
 * gsto_spinand_detect - initialize device related part in spinand_device
 * struct if it is a gsto device.
 * @spinand: SPI NAND device structure
 */
static int gsto_spinand_detect(struct spinand_device *spinand)
{
	u8 *id = spinand->id.data;
	int ret;

	/*
	 * gsto SPI NAND read ID need a dummy byte,
	 * so the first byte in raw_id is dummy.
	 */
	if (id[0] != SPINAND_MFR_GSTO)
		return 0;

	ret = spinand_match_and_init(spinand, gsto_spinand_table,
				     ARRAY_SIZE(gsto_spinand_table), id[1]);
	if (ret)
		return ret;

	return 1;
}

static const struct spinand_manufacturer_ops gsto_spinand_manuf_ops = {
	.detect = gsto_spinand_detect,
	// .init = gsto_spinand_init,
};

const struct spinand_manufacturer gsto_spinand_manufacturer = {
	.id = SPINAND_MFR_GSTO,
	.name = "GSTO",
	.ops = &gsto_spinand_manuf_ops,
};       

#endif