#!/bin/bash

pwd_dir=$(dirname $0)

source $pwd_dir/../config.mk

fastsys_dir=$CONFIG_OS_DIR/fastsys/project/applications/
install_dir=$CONFIG_TOP_DIR/image
uboot_tools_dir=$CONFIG_TOP_DIR/tools/uboot_tools
chip_series=$(echo $CONFIG_CHIP_SERIES | tr '[A-Z]' '[a-z]')
csi_sensor=$CONFIG_SENSOR_TYPE
fastsys_defconfig=$CONFIG_FASTSYS_DEFCONFIG

install_fastsys_image()
{
    local ext_flag=
    if [ "$fastsys_defconfig" != "" ];then
        ext_flag="$ext_flag -c $fastsys_defconfig "
    fi

    echo "Install fastsys uImage!"
    cd $fastsys_dir
    ./build_tool.sh install  $chip_series $uboot_tools_dir  $install_dir $ext_flag
    # 检查是否编译出错
    if test $? -ne 0 ; then
        exit 1
    fi

    cd -
}

build_fastsys() 
{
    local ext_flag=
    if [ "$fastsys_defconfig" != "" ];then
        ext_flag="$ext_flag -c $fastsys_defconfig "
    fi
    echo "ext_flag: " "$ext_flag"

    cd $fastsys_dir

    # sed -i 's/^CONFIG_SENSOR_TYPE=.*/CONFIG_SENSOR_TYPE=0/' config
    # if test $CONFIG_SENSOR_TYPE = "sc450ai"; then
    #     sed -i 's/^CONFIG_SENSOR_TYPE=.*/CONFIG_SENSOR_TYPE=sc450ai/' config
    # fi
    ./build_tool.sh build $chip_series $csi_sensor $ext_flag

    # 检查是否编译出错
    if test $? -ne 0 ; then
        exit 1
    fi

    cd -
}

clean_fastsys()
{
    local ext_flag=
    if [ "$fastsys_defconfig" != "" ];then
        ext_flag="$ext_flag -c $fastsys_defconfig "
    fi

    cd $fastsys_dir
    ./build_tool.sh clean $chip_series $ext_flag

    # 检查是否编译出错
    if test $? -ne 0 ; then
        exit 1
    fi

    cd -
}

echo "Fastsys project path $fastsys_dir"

if test ! -d $fastsys_dir ; then
    echo "Build fastsys failed, can't find fastsys dir!"
    exit 1
fi

case "$1" in
    all)
        build_fastsys
        install_fastsys_image
        ;;
    install)
        install_fastsys_image
        ;;
    clean)
        clean_fastsys
        ;;
    *)
        echo "Usage:"
        echo " $0 all  : build fastsys and install"
        echo " $0 install : install fastsys uImage/args"
        echo " $0 clean : clean fastsys"
        ;;
esac
