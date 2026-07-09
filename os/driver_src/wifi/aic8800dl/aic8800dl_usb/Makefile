#!/bin/sh

# WIFI
CONFIG_AIC_LOADFW_SUPPORT=m
CONFIG_AIC8800_WLAN_SUPPORT=m

export CONFIG_AIC_LOADFW_SUPPORT
export CONFIG_AIC8800_WLAN_SUPPORT

# BT
CONFIG_BT_AICBTUSB=m
export CONFIG_BT_AICBTUSB

# AV100
KDIR=$(KSRC)
ARCH?=arm
CROSS_COMPILE?=arm-anycloud-linux-uclibcgnueabi-

all:
	# aic_load_fw.ko and aic8800_fdv.ko
	make -C ${KDIR} M=${PWD}/drivers/aic8800/ ARCH=arm CROSS_COMPILE=${CROSS_COMPILE}
	# aic_btusb.ko
	make -C ${KDIR} M=${PWD}/drivers/aic_btusb/ ARCH=arm CROSS_COMPILE=${CROSS_COMPILE}

clean:
	make -C ${KDIR} M=${PWD}/drivers/aic8800/ clean
	make -C ${KDIR} M=${PWD}/drivers/aic_btusb/ clean
