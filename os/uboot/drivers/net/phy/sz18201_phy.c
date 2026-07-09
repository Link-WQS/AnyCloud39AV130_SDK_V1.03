// SPDX-License-Identifier: GPL-2.0+
/*
 * Micrel PHY drivers
 *
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * author Andy Fleming
 * (C) 2012 NetModule AG, David Andrey, added KSZ9031
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <phy.h>

#define SZ18201_ADDRESS_OFFSET      0x001e
#define SZ18201_DATA_REG            0x001f

#define SZ18201_EXT_LED0_CONTROL    0x40C0
#define SZ18201_EXT_LED1_CONTROL    0x40C3

static int sz18201_config(struct phy_device *phydev)
{
	int reg;

	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, 0x9140);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);

	phy_write(phydev, MDIO_DEVAD_NONE, SZ18201_ADDRESS_OFFSET, 0x50);
	reg = phy_read(phydev, MDIO_DEVAD_NONE, SZ18201_DATA_REG);
	reg |= 0x0040;
	phy_write(phydev, MDIO_DEVAD_NONE, SZ18201_ADDRESS_OFFSET, 0x50);
	phy_write(phydev, MDIO_DEVAD_NONE, SZ18201_DATA_REG, reg);
	phy_write(phydev, MDIO_DEVAD_NONE, SZ18201_DATA_REG, reg);
	phy_write(phydev, MDIO_DEVAD_NONE, SZ18201_ADDRESS_OFFSET, 0x00);

	phy_write(phydev, MDIO_DEVAD_NONE, SZ18201_ADDRESS_OFFSET, SZ18201_EXT_LED0_CONTROL);
	phy_write(phydev, MDIO_DEVAD_NONE, SZ18201_DATA_REG, 0x30);

	phy_write(phydev, MDIO_DEVAD_NONE, SZ18201_ADDRESS_OFFSET, SZ18201_EXT_LED1_CONTROL);
	phy_write(phydev, MDIO_DEVAD_NONE, SZ18201_DATA_REG, 0x1300);

	return genphy_config_aneg(phydev);
}

static int sz18201_parse_status(struct phy_device *phydev)
{
	u32 lpa = 0;

	lpa = phy_read(phydev, MDIO_DEVAD_NONE, MII_ADVERTISE);
	lpa &= phy_read(phydev, MDIO_DEVAD_NONE, MII_LPA);

	if (lpa & (LPA_100FULL | LPA_100HALF)) {
		phydev->speed = SPEED_100;

		if (lpa & LPA_100FULL)
			phydev->duplex = DUPLEX_FULL;

	} else if (lpa & LPA_10FULL) {
		phydev->duplex = DUPLEX_FULL;
	}

	return 0;
}

static int sz18201_startup(struct phy_device *phydev)
{
	int ret;
	ret = genphy_update_link(phydev);
	if (ret){
		return ret;
	}
	
	return sz18201_parse_status(phydev);
}

static struct phy_driver sz18201_driver = {
	.name = "SZ18201 Switch",
	.uid  = 0x00000128,
	.mask = 0x1fffffff,
	.features = PHY_BASIC_FEATURES,
	.config 	= &sz18201_config,
	.startup 	= &sz18201_startup,
	.shutdown 	= &genphy_shutdown,
};

int phy_sr_sz18201_init(void)
{
	phy_register(&sz18201_driver);

	return 0;
}
