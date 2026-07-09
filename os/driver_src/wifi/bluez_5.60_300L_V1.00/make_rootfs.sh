#!/bin/bash
#从_install目录制作自己的rootfs：
#因为刚编译出来的_install目录是比较大的，全部打包到rootfs中去会很占用flash空间
#实测16M的nor flash都不够放。所以需要对_install目录做处理后再打包到rootfs中去。

check_error_printf()
{
    if test $1 -ne 0; then
        echo " $2 failed!"
        exit
    else
        echo "$2 success!"
    fi
}

get_build_cfg()
{
	CROSS_PREFIX=`awk -F= '/^CROSS_PREFIX/{gsub(" ","",$2); printf $2}' config.mk`
    if test $? -eq 0 ; then
        echo "CROSS_PREFIX"="$CROSS_PREFIX"
    else
        return 1
    fi
}

get_build_cfg
ROOT_DIR_NAME=rootfs_bluez_V1.00_300L

rm $ROOT_DIR_NAME -rf
mkdir -p $ROOT_DIR_NAME

cp _install/* $ROOT_DIR_NAME -rf

#1、将usr/local/bin下的所有可执行文件都strip一下
cd $ROOT_DIR_NAME/usr/local/bin
ls | xargs $CROSS_PREFIX-strip
cd -

set -e
#2、删除usr/local/下的整个include目录、share目录
rm -rf $ROOT_DIR_NAME/usr/local/include $ROOT_DIR_NAME/usr/local/share/*
cp _install/usr/local/share/dbus-1 _install/usr/local/share/zsh $ROOT_DIR_NAME/usr/local/share/ -rf




#3、删除lib目录下所有的.a静态库，只保留.so动态库即可
rm -f $ROOT_DIR_NAME/usr/local/lib/*.a $ROOT_DIR_NAME/usr/local/lib/*.la

#4、将lib目录下面gio和glib两个库strip一下，因为这两个库是最大的，加起来有10M

$CROSS_PREFIX-strip $ROOT_DIR_NAME/usr/local/lib/libgio-2.0.so.0.5600.1 $ROOT_DIR_NAME/usr/local/lib/libglib-2.0.so.0.5600.1

#5、/usr/local/libexec下面的bluez命令程序strip一下
cd $ROOT_DIR_NAME/usr/local/libexec/bluetooth
ls | xargs $CROSS_PREFIX-strip
cd -

#6、将rootfs_bluez_other_xx中的所有文件拷贝到ROOT_DIR_NAME目录下面
cp rootfs_bluez_other_300L/* $ROOT_DIR_NAME/ -rf



