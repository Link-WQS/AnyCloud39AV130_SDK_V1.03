// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 *
 * Derived from drivers/mtd/nand/spi/micron.c
 *   Copyright (c) 2016-2017 Micron Technology, Inc.
 */

#ifndef __UBOOT__
#include <linux/device.h>
#include <linux/kernel.h>
#endif
#include <linux/mtd/spinand.h>

#ifdef CONFIG_ANYKA


#define SPINAND_MFR_FM_MICRO			0xa1
#define FM25SXXA_STATUS_ECC_1_7_BITFLIPS	(1 << 4)
#define FM25SXXA_STATUS_ECC_8_BITFLIPS	(3 << 4)

#define FM25SXXA_REG_STATUS2		0xf0

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

static int fm25sxxa_ooblayout_ecc(struct mtd_info *mtd, int section,
				       struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = 64;
	region->length = 64;

	return 0;
}

static int fm25sxxa_ooblayout_free(struct mtd_info *mtd, int section,
					struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 1 bytes for the BBM. */
	region->offset = 0;
	region->length = 64;
	region->oob_badoffset = 8;
	region->oob_seglen = 12;
	region->oob_skiplen = 4;

	return 0;
}

static int fm25sxxa_ecc_get_status(struct spinand_device *spinand,
					u8 status)
{
	u8 status2;
	struct spi_mem_op op = SPINAND_GET_FEATURE_OP(FM25SXXA_REG_STATUS2,
						      &status2);
	int ret;

	switch (status & STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case FM25SXXA_STATUS_ECC_1_7_BITFLIPS:
		/*
		 * Read status2 register to determine a more fine grained
		 * bit error status
		 */
		ret = spi_mem_exec_op(spinand->slave, &op);
		if (ret)
			return ret;

		/*
		 * 4 ... 7 bits are flipped (1..4 can't be detected, so
		 * report the maximum of 4 in this case
		 */
		/* bits sorted this way (3...0): ECCS1,ECCS0,ECCSE1,ECCSE0 */
		return ((status & STATUS_ECC_MASK) >> 2) |
			((status2 & STATUS_ECC_MASK) >> 4);

	case FM25SXXA_STATUS_ECC_8_BITFLIPS:
		return 8;

	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	default:
		break;
	}

	return -EINVAL;
}

static const struct mtd_ooblayout_ops fm25sxxa_ooblayout = {
	.ecc = fm25sxxa_ooblayout_ecc,
	.free = fm25sxxa_ooblayout_free,
};

static const struct spinand_info fm_micro_spinand_table[] = {
	SPINAND_INFO("FM25S01A", 0xe4,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&fm25sxxa_ooblayout,
				     fm25sxxa_ecc_get_status)),//OK
	SPINAND_INFO("FM25S02A", 0xe5,
		     NAND_MEMORG(1, 2048, 64, 64, 2048, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&fm25sxxa_ooblayout,
				     fm25sxxa_ecc_get_status)),//ok
	SPINAND_INFO("FM25S01BI3", 0xd4,
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&fm25sxxa_ooblayout,
				     fm25sxxa_ecc_get_status)),//OK
};

static int fm_micro_spinand_detect(struct spinand_device *spinand)
{
	u8 *id = spinand->id.data;
	int ret;

	/*
	 * For GD NANDs, There is an address byte needed to shift in before IDs
	 * are read out, so the first byte in raw_id is dummy.
	 */
	if (id[0] != SPINAND_MFR_FM_MICRO)
		return 0;

	ret = spinand_match_and_init(spinand, fm_micro_spinand_table,
				     ARRAY_SIZE(fm_micro_spinand_table),
				     id[1]);
	if (ret)
		return ret;

	return 1;
}

static const struct spinand_manufacturer_ops fm_micro_spinand_manuf_ops = {
	.detect = fm_micro_spinand_detect,
};

const struct spinand_manufacturer fm_micro_spinand_manufacturer = {
	.id = SPINAND_MFR_FM_MICRO,
	.name = "FM_micro",
	.ops = &fm_micro_spinand_manuf_ops,
};
#endif
