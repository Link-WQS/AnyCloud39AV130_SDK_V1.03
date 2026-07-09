#!/bin/bash

pwd_dir=$(dirname $0)

if [ -d `pwd`/../../workspace ]; then
  source $pwd_dir/../../config.mk
else
  source $pwd_dir/../config.mk
fi

get_config_info()
{
  if [ -d `pwd`/../../workspace ]; then
	  config_top=`pwd`/../../workspace
  else
    config_top=`pwd`/../
  fi
    uboot_tools_dir=$pwd_dir/../tools/uboot_tools
}

prepare_image()
{
    if test -d ramfs ; then
    	rm -rf ramfs
    fi
    # if test ! -d ramfs ; then
    #     mkdir ramfs
    #     tar xvfz rootfs.tar.gz -C ramfs
    # fi

    # # 拷贝必要文件到ramfs
    # mkdir -p ./ramfs/etc
    # cp ./ramfs_res/etc/* ./ramfs/etc/ -rf
    # cp -rf $config_top/os/driver/internal/lib ./ramfs/

    mkdir -p ramfs
    mkdir -p ramfs/fast
    cp -af rootfs/* ramfs
    rm -rf ramfs/etc/config/*
    rm -rf ramfs/usr/*
    rm -rf ramfs/lib/*
    rm -rf ramfs/etc/isp_*
    cp -af rootfs/usr/bin/ak_fast_sample ramfs/etc/
}

# 制作烧录镜像
make_image()
{
    cd ./ramfs
    find . | cpio -o -H newc > ../ramfs.cpio
    cd ..
    lz4 -l -3 -f ramfs.cpio ramfs.cpio.lz4

    $uboot_tools_dir/mkimage -A riscv -T ramdisk -C lz4 -O linux -a 0x0 -e 0x0 -n "AnyCloud39AV200 ramfs" -d ./ramfs.cpio.lz4 ./ramfs.img
    cp ramfs.img $config_top/tools/burntool/
    cp ramfs.img $config_top/image/
}

clean_image()
{
    echo "clean fs image..."
    rm ramfs ramfs.cpio ramfs.cpio.lzma -rf
}

get_config_info

case "$1" in
	all)
		prepare_image
		make_image
		;;
	clean)
		clean_image
		;;
	*)
		echo "Usage:"
		echo " $0 all  : build all"
		echo " $0 clean: clean all"
		;;
esac
