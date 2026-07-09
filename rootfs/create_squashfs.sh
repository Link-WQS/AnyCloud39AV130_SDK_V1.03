#!/bin/bash

pwd_dir=$(dirname $0)

if [ -d `pwd`/../../workspace ]; then
  source $pwd_dir/../../config.mk
else
  source $pwd_dir/../config.mk
fi

#	第一个参数指定要打包目录
#	第二个参数指定输出文件名
#	第三个参数指定打包方式
mkdir -p tmpfs
cp -af rootfs/* tmpfs
rm -rf tmpfs/usr/* tmpfs/etc/config/* tmpfs/app/* tmpfs/data/*
rm -rf tmpfs/lib/modules/4.4.302-cip94/kernel/drivers/video

./mksquashfs tmpfs/ root.sqsh4 -noappend -comp xz
if [ "$CONFIG_SYS_BOOT_MODE" = "FAST" ]; then
    mkdir -p tmpfs_usr
    cp -af rootfs/usr/* tmpfs_usr
    rm -rf tmpfs_usr/modules/*.ko
    rm -rf tmpfs_usr/bin/*_sample
    rm -rf tmpfs_usr/lib/*.so
    rm -rf tmpfs_usr/sbin/*.sh
    rm -rf tmpfs_usr/sbin/yolov5n_v1_640
    rm -rf tmpfs_usr/sbin/test_random_mobilenetsoftmaxcrydet_opt
    rm -rf tmpfs_usr/sbin/upperbody640_opt

    cp scripts/others/main.sh tmpfs_usr/sbin/
    cp rootfs/usr/sbin/service.sh tmpfs_usr/sbin/

    cp rootfs/usr/modules/ak_mci.ko tmpfs_usr/modules/
    cp rootfs/lib/modules/5.10.111/kernel/drivers/mmc/core/* tmpfs_usr/modules/
    cp rootfs/lib/modules/5.10.111/kernel/fs/exfat/* tmpfs_usr/modules/
    cp rootfs/usr/modules/ak_rtc.ko tmpfs_usr/modules/
    cp rootfs/usr/modules/ak_efuse.ko tmpfs_usr/modules/
    cp rootfs/usr/modules/ak_eth.ko tmpfs_usr/modules/
    ./mksquashfs tmpfs_usr usr.sqsh4 -noappend -comp xz
    rm -rf tmpfs_usr
else
    ./mksquashfs rootfs/usr usr.sqsh4 -noappend -comp xz
    ./mksquashfs rootfs/app/ app.sqsh4 -noappend -comp xz
fi

rm -rf tmpfs
