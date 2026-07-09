#!/bin/sh

# WIFI
export CONFIG_AIC_LOADFW_SUPPORT=m
export CONFIG_AIC8800_WLAN_SUPPORT=m

# BT
export CONFIG_BT_AICBTUSB=m

# AV100
export KDIR=../../../kbd/
export ARCH=arm
export CROSS_COMPILE=arm-anycloud-linux-uclibcgnueabi-
 
# aic_load_fw.ko and aic8800_fdv.ko
make -C $KDIR M=$PWD/drivers/aic8800/ ARCH=arm CROSS_COMPILE=${CROSS_COMPILE}

# aic_btusb.ko
make -C $KDIR M=$PWD/drivers/aic_btusb/ ARCH=arm CROSS_COMPILE=${CROSS_COMPILE}
 
# install
mkdir -p out/av100/
rm -rf out/av100/*
find drivers -name '*.ko' -exec cp {} out/av100/ \;