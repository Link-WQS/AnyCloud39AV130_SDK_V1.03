#!/bin/bash

pwd_dir=$(dirname $0)

source $pwd_dir/../config.mk

get_config_info()
{
    CHIP_SERIES=$CONFIG_CHIP_SERIES
	CROSS_COMPILE=$CONFIG_CROSS_COMPILE
    SYS_BOOT_MODE=$CONFIG_SYS_BOOT_MODE
	FLASH_TYPE=$CONFIG_FLASH_TYPE

	chip_series=$(echo $CHIP_SERIES | tr '[A-Z]' '[a-z]')
    kernel_dir=$CONFIG_OS_DIR/kernel
	kernel_build_dir=$CONFIG_OS_DIR/kbd
    kernel_install_dir=$CONFIG_TOP_DIR/image
	driver_install_dir=$CONFIG_TOP_DIR/os/driver
    uboot_tools_dir=$CONFIG_TOP_DIR/tools/uboot_tools
    num_of_cpu_cores=$(nproc)

	echo "CHIP_SERIES: $CHIP_SERIES, chip_series:$chip_series"
	echo "CROSS_COMPILE : $CROSS_COMPILE"
	echo "driver_install_dir : $driver_install_dir"
}

install_kernel_image()
{
	cd $kernel_dir

	# 安装镜像
	echo "Install kernel image!"
	#如果是快启模式，将内核镜像改名为uImage_fast
	if test $CONFIG_FAST_MODE = "fast"  || test $CONFIG_FAST_MODE = "fast_aov" || test $CONFIG_FAST_MODE = "fast_uvc"; then 
		find $kernel_build_dir -name uImage | xargs -i cp {} $kernel_install_dir/uImage_fast
	else
		find $kernel_build_dir -name uImage | xargs -i cp {} $kernel_install_dir
	fi

	# 安装dtb文件
	echo "Install dtb file!"
	# find $kernel_build_dir -name *.dtb | xargs -i cp {} $kernel_install_dir
	find $kernel_build_dir -name EVB_CBDM_AK3918AV130L_V1.0.0.dtb | xargs -i cp {} $kernel_install_dir

    # 安装内核模块
    make modules_install O=$kernel_build_dir INSTALL_MOD_PATH=$driver_install_dir/internal

	find $driver_install_dir/internal -name "build" | xargs rm
	find $driver_install_dir/internal -name "source" | xargs rm

	cd -
}

pack_uImage()
{
    kernel_image=$kernel_build_dir/arch/riscv/boot/Image
    if test ! -e $kernel_image ; then
        echo "The Kernel Image $kernel_image is not existed!"
    exit 1
    fi
    echo "kernel_image="$kernel_image

    if [ "$SYS_BOOT_MODE" = "FAST" ];then
        # lzma -f -6 $kernel_image > $kernel_build_dir/arch/riscv/boot/Image.lzma
        lz4 -3 -f $kernel_image $kernel_build_dir/arch/riscv/boot/Image.lzma
    else
        lzma -f -6 $kernel_image > $kernel_build_dir/arch/riscv/boot/Image.lzma
    fi
    kernel_image_lzma=$kernel_build_dir/arch/riscv/boot/Image.lzma
    if test ! -e $kernel_image_lzma ; then
        echo "The Kernel Image $kernel_image_lzma is not existed!\n"
    exit 1
    fi
    echo "kernel_image_lzma="$kernel_image_lzma

    if [ "$SYS_BOOT_MODE" = "FAST" ];then
	    $uboot_tools_dir/mkimage -A riscv -T kernel -C lz4 -O linux -a 0x40100000 -e 0x40100000 -n "AnyCloud39AV200 kernel" -d $kernel_image_lzma $kernel_build_dir/arch/riscv/boot/uImage
    else
	    $uboot_tools_dir/mkimage -A riscv -T kernel -C lzma -O linux -a 0x40100000 -e 0x40100000 -n "AnyCloud39AV200 kernel" -d $kernel_image_lzma $kernel_build_dir/arch/riscv/boot/uImage
    fi
}

build_kernel() 
{
    # 检查内核源码目录是否正常
    if test ! -d $kernel_dir ; then
        echo "Build Kernel failed, can't find kernel dir!"
		exit 1
    fi

	pwd
	cd $kernel_dir

    # 创建编译的目标目录
	if test ! -d $kernel_build_dir ; then
    	mkdir $kernel_build_dir
	fi

    # 根据芯片类型进行内核配置
    if [ "$CHIP_SERIES" = "AK3918AV200" ] && [ "$CONFIG_FLASH_TYPE" != "MMC" ];then
        if [ "$SYS_BOOT_MODE" = "FAST" ];then
            if [ "$FLASH_TYPE" = "NOR" ];then
                make O=$kernel_build_dir ARCH=riscv ${chip_series}_fastsys_mini_defconfig CROSS_COMPILE=$CROSS_COMPILE
                # make O=$kernel_build_dir ARCH=riscv ${chip_series}_fast_nor_mini_defconfig CROSS_COMPILE=$CROSS_COMPILE
            else
                make O=$kernel_build_dir ARCH=riscv ${chip_series}_fast_nand_mini_defconfig CROSS_COMPILE=$CROSS_COMPILE
            fi
        else
            make O=$kernel_build_dir ARCH=riscv ${chip_series}_mini_defconfig CROSS_COMPILE=$CROSS_COMPILE
        fi
    elif [ "$CHIP_SERIES" = "AK3918AV200" ] && [ "$CONFIG_FLASH_TYPE" = "MMC" ] ; then
        make O=$kernel_build_dir ARCH=riscv ${chip_series}_mini_mmc_defconfig CROSS_COMPILE=$CROSS_COMPILE
    else
        	#av100 快启
		if test $CONFIG_FAST_MODE = "fast" ; then 
            if test $CONFIG_FAST_FS = "romfs" ; then
                make O=$kernel_build_dir anycloud_${chip_series}_fast_romfs_mini_defconfig CROSS_COMPILE=$CROSS_COMPILE
            else
                make O=$kernel_build_dir anycloud_${chip_series}_fast_mini_defconfig CROSS_COMPILE=$CROSS_COMPILE
            fi
		#av100 快启 + aov
		elif test $CONFIG_FAST_MODE = "fast_aov" ; then
			make O=$kernel_build_dir anycloud_${chip_series}_fastaov_mini_defconfig CROSS_COMPILE=$CROSS_COMPILE
		elif test $CONFIG_FAST_MODE = "aov" ; then
			make O=$kernel_build_dir anycloud_${chip_series}_aov_mini_defconfig CROSS_COMPILE=$CROSS_COMPILE
		#av100 快启 + uvc
		elif test "$CONFIG_FAST_MODE" = "fast_uvc" ; then
			# make O=$kernel_build_dir anycloud_${chip_series}_fastuvc_mini_defconfig CROSS_COMPILE=$CROSS_COMPILE
			make O=$kernel_build_dir anycloud_${chip_series}_fastaov_uvc_mini_defconfig CROSS_COMPILE=$CROSS_COMPILE
		elif test "$CONFIG_FAST_MODE" = "no_fast_amp" ; then
			make O=$kernel_build_dir anycloud_${chip_series}_amp_mini_defconfig CROSS_COMPILE=$CROSS_COMPILE
		#av100 标准sdk配置
		else
			make O=$kernel_build_dir anycloud_${chip_series}_mini_defconfig CROSS_COMPILE=$CROSS_COMPILE
		fi
    fi

    # 检查配置是否正常
    if test $? -ne 0 ; then
        exit 1
    fi

    cd $kernel_build_dir
	if test $CONFIG_SENSOR_TYPE = "sc450ai"; then
        sed -i 's/^# CONFIG_AK_SENSOR_SC450AI is not set/CONFIG_AK_SENSOR_SC450AI=y/' .config
    fi
    cd $kernel_dir
    
    # 编译内核
    if [ "$CHIP_SERIES" = "AK3918AV200" ];then
        make O=$kernel_build_dir ARCH=riscv  -j$num_of_cpu_cores CROSS_COMPILE=$CROSS_COMPILE 
        make O=$kernel_build_dir ARCH=riscv dtbs modules -j$num_of_cpu_cores CROSS_COMPILE=$CROSS_COMPILE
    else
        make O=$kernel_build_dir dtbs modules uImage -j$num_of_cpu_cores CROSS_COMPILE=$CROSS_COMPILE
    fi

    # 检查是否编译出错
    if test $? -ne 0 ; then
        exit 1
    fi

	cd -
}

clean_kernel() 
{
    # 检查编译过程目录是否存在，如果存在先清除掉上一次的编译
    if test -d $kernel_build_dir ; then
		echo "Build kernel dir($kernel_build_dir) , clean it"
        rm -rf $kernel_build_dir
    fi
}

get_config_info

case "$1" in
	all)
        build_kernel
        if [ "$CHIP_SERIES" = "AK3918AV200" ];then
            pack_uImage
        fi
		install_kernel_image
		;;
	image)
        build_kernel
		;;
	install)
        install_kernel_image
		;;
	clean)
		clean_kernel
		;;
	*)
		echo "Usage:"
		echo " $0 all   : build kernel and install"
		echo " $0 image   : build kernel"
		echo " $0 install   : install kernel image/dtb/ko"
		echo " $0 clean : clean kernel"
		;;
esac
