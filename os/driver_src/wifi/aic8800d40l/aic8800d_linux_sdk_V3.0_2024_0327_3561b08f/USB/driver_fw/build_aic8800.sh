#!/bin/sh

# WIFI
export CONFIG_AIC_LOADFW_SUPPORT=m
export CONFIG_AIC8800_WLAN_SUPPORT=m

# BT
export CONFIG_BT_AICBTUSB=m

# V500
export KDIR=/home/$(whoami)/anyka_sdk/anycloud/gerrit/repo_AnyCloud39EV330/V1.06/os/bd
export ARCH=arm
export CROSS_COMPILE=arm-anykav500-linux-uclibcgnueabi-
 
# aic_load_fw.ko and aic8800_fdv.ko
make -C $KDIR M=$PWD/drivers/aic8800/ ARCH=arm CROSS_COMPILE=${CROSS_COMPILE}

# aic_btusb.ko
make -C $KDIR M=$PWD/drivers/aic_btusb/ ARCH=arm CROSS_COMPILE=${CROSS_COMPILE}
 
# install
mkdir -p out/V500/
rm -rf out/V500/*
find drivers -name '*.ko' -exec cp {} out/V500/ \;