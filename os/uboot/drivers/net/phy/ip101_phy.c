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

#define ET1011C_CONFIG_REG			(0x16)
#define ET1011C_TX_FIFO_MASK		(0x3 << 12)
#define ET1011C_TX_FIFO_DEPTH_8		(0x0 << 12)
#define ET1011C_TX_FIFO_DEPTH_16	(0x1 << 12)
#define ET1011C_INTERFACE_MASK		(0x7 << 0)
#define ET1011C_GMII_INTERFACE		(0x2 << 0)
#define ET1011C_SYS_CLK_EN			(0x1 << 4)
#define ET1011C_TX_CLK_EN			(0x1 << 5)

#define ET1011C_STATUS_REG			(0x1A)
#define ET1011C_DUPLEX_STATUS		((0x1 << 14)|(0x1 << 12))
#define ET1011C_SPEED_MASK			((0x1 << 14)|(0x1 << 13))
#define ET1011C_SPEED_1000			(0x2 << 8)
#define ET1011C_SPEED_100			((0x1 << 14)|(0x1 << 13))
#define ET1011C_SPEED_10			((0x1 << 12)|(0x1 << 11))

static int ip101_config(struct phy_device *phydev)
{
	int ctl = 0;

	ctl = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
	if (ctl < 0)
		return ctl;
	ctl &= ~(BMCR_FULLDPLX | BMCR_SPEED100 | BMCR_SPEED1000 |
		 BMCR_ANENABLE);
	
	/* First clear the PHY */
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, ctl | BMCR_RESET);

	return genphy_config_aneg(phydev);
}

static int ip101_parse_status(struct phy_device *phydev)
{
	int mii_reg;
	int speed;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);
	if (mii_reg & ET1011C_DUPLEX_STATUS){
		phydev->duplex = DUPLEX_FULL;
	}
	else{
		phydev->duplex = DUPLEX_HALF;
	}

	speed = mii_reg & ET1011C_SPEED_MASK;
	switch (speed) {
	case ET1011C_SPEED_100:
		phydev->speed = SPEED_100;
		break;
	case ET1011C_SPEED_10:
		phydev->speed = SPEED_10;
		break;
	}
	
	printf("%s, duplex:%s, speed:%s\n", __func__, (phydev->duplex == 0x01)? "DUPLEX_FULL":"DUPLEX_HALF", 
		(phydev->speed == 100)? "SPEED_100":"SPEED_10");
	return 0;
}


static int ip101_startup(struct phy_device *phydev)
{
	int ret;
	ret = genphy_update_link(phydev);
	if (ret){
		return ret;
	}
	
	return ip101_parse_status(phydev);
}


static struct phy_driver ip101_driver = {
	.name = "IP101 Switch",
	.uid  = 0x02430C54,
	.mask = 0x1fffffff,
	.features = PHY_BASIC_FEATURES,
	.config 	= &ip101_config,
	.startup 	= &ip101_startup,
	.shutdown 	= &genphy_shutdown,
};

int phy_sr_ip101_init(void)
{
	phy_register(&ip101_driver);

	return 0;
}
