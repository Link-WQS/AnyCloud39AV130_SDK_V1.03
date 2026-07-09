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

	rootfs_dir=$config_top/rootfs
    driver_install_dir=$config_top/os/driver
    image_install_dir=$CONFIG_IMAGE_OUTPUT_DIR
    sample_install_dir=$rootfs_dir/samples

    driver_ko_dir=$config_top/os/driver/external/fast
     


	uboot_tools_dir=$pwd_dir/../tools/uboot_tools

    echo "FLASH_TYPE=$FLASH_TYPE, WIFI_TYPE=$WIFI_TYPE, CHIP_SERIES=$CHIP_SERIES"
}

prepare_image()
{
    if test -d initfs ; then
    	rm -rf initfs
    fi
    if test ! -d initfs ; then
        tar xvfz initfs.tar.gz -C .
    fi

	echo $config_top
    # 拷贝必要文件到initfs
    cp $driver_ko_dir/ak_i2c.ko ./initfs/fast/
    
	if test $CONFIG_SENSOR_TYPE = "sc450ai"; then
        cp $driver_ko_dir/sensor_sc450ai.ko ./initfs/fast/
	elif test $CONFIG_SENSOR_TYPE = "gc13b0"; then
        cp $driver_ko_dir/sensor_gc13b0.ko ./initfs/fast/
	elif test $CONFIG_SENSOR_TYPE = "sc200ai"; then
        cp $driver_ko_dir/sensor_sc200ai.ko ./initfs/fast/
    else
        cp $driver_ko_dir/sensor_sc450ai.ko ./initfs/fast/
    fi

    cp $driver_ko_dir/ak_vi.ko ./initfs/fast/
    cp $driver_ko_dir/ak_vicap.ko ./initfs/fast/
    cp $driver_ko_dir/ak_isp.ko ./initfs/fast/
    cp $driver_ko_dir/ak_pp.ko ./initfs/fast/
	cp $driver_ko_dir/ak_venc_adapter.ko ./initfs/fast/
	cp $driver_ko_dir/ak_venc_bridge.ko ./initfs/fast/
    cp $driver_ko_dir/ak_quick_start.ko ./initfs/fast/
    
    if test $CONFIG_SYS_GLASS_MODE = "y"; then
        cp $driver_ko_dir/ak_ipu.ko ./initfs/fast/
        #cp $driver_ko_dir/ak_mpu6050.ko ./initfs/fast/
        cp $driver_ko_dir/ak_pcm.ko ./initfs/fast/
        cp $driver_ko_dir/ak_mci.ko ./initfs/fast/
        cp $driver_ko_dir/ak_sc7i22.ko ./initfs/fast/
        cp $driver_ko_dir/ak_npu.ko ./initfs/fast/
        cp $driver_ko_dir/../../internal/lib/modules/5.10.111/kernel/drivers/base/firmware_loader/firmware_class.ko ./initfs/fast/	
    fi
    
if [ -d `pwd`/../../workspace ]; then
  ispconf_dir=$config_top/../chips/AK3918AV200/configs/isp
else
  ispconf_dir=$config_top/rootfs/utils/etc
fi

	if test $CONFIG_SENSOR_TYPE = "sc450ai"; then
        cp $ispconf_dir/isp_sc450ai_mipi_2lane_av200.conf ./initfs/fast/
        cp $ispconf_dir/isp_sc450ai_mipi_2lane_av200.conf $config_top/tools/burntool/isp.conf
	elif test $CONFIG_SENSOR_TYPE = "gc13b0"; then
	#for Linux use
        cp $ispconf_dir/isp_gc13b0_mipi_2lane_av200_30fps_10b.conf ./initfs/fast/isp_gc13b0_mipi_2lane_av200_30fps.conf
        cp $ispconf_dir/isp_gc13b0_mipi_2lane_av200_10fps_10b.conf ./initfs/fast/isp_gc13b0_mipi_2lane_av200.conf
        #for fastsys use
        cp $ispconf_dir/isp_gc13b0_mipi_2lane_av200_10b.conf $config_top/tools/burntool/isp.conf
	#for Linux use
        #cp $ispconf_dir/isp_gc13b0_mipi_2lane_av200_30fps.conf ./initfs/fast/isp_gc13b0_mipi_2lane_av200_30fps.conf
        #cp $ispconf_dir/isp_gc13b0_mipi_2lane_av200_10fps.conf ./initfs/fast/isp_gc13b0_mipi_2lane_av200.conf
        #for fastsys use
        #cp $ispconf_dir/isp_gc13b0_mipi_2lane_av200.conf $config_top/tools/burntool/isp.conf
	elif test $CONFIG_SENSOR_TYPE = "sc200ai"; then
        cp $ispconf_dir/isp_sc200ai_mipi_2lane_av200.conf ./initfs/fast/
        cp $ispconf_dir/isp_sc200ai_mipi_2lane_av200.conf $config_top/tools/burntool/isp.conf
    else
        cp $ispconf_dir/isp_sc450ai_mipi_2lane_av200.conf ./initfs/fast/
        cp $ispconf_dir/isp_sc450ai_mipi_2lane_av200.conf $config_top/tools/burntool/isp.conf
    fi
}

# 制作烧录镜像
make_image()
{
    cd ./initfs
    find . | cpio -o -H newc > ../initfs.cpio
    cd ..
    # lz4 -l -1 -f initfs.cpio initfs.cpio.lz4

    # $uboot_tools_dir/mkimage -A riscv -T ramdisk -C lz4 -O linux -a 0x4a408000 -e 0x4a408000 -n "AnyCloud39AV200 initfs" -d ./initfs.cpio.lz4 ./initfs.img
    $uboot_tools_dir/mkimage -A riscv -T ramdisk -C none -O linux -a 0x42A08000 -e 0x42A08000 -n "AnyCloud39AV200 initfs" -d ./initfs.cpio ./initfs.img
    cp initfs.img $config_top/tools/burntool/
    cp initfs.img $config_top/image/

    # cd  $config_top/tools/envtool
    # FILE_CONF=./env_av200_128MB_fast.cfg
    # # 制作uboot环境变量
    # cd  $config_top/tools/envtool
    # rm env.img -f
    # tr '\000' '\377' < /dev/zero | dd of=./env.img bs=1024 count=4
    # ./fw_setenv -s $FILE_CONF
    # ./fw_printenv

    # cp env.img  $config_top/tools/burntool/env_av200_128MB_fast.img
}

clean_image()
{
    echo "clean fs image..."
    rm initfs initfs.cpio initfs.cpio.lzma initfs.img -rf
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
