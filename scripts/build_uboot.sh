#!/bin/bash

workspace_dir=$(dirname $0)/..
if [ -a $workspace_dir/config.mk ] ; then
	source $workspace_dir/config.mk
else
	echo "cannot found config.mk, aborting ... "
	exit 1
fi

build_uboot()
{
    chip_series=$(echo $CONFIG_CHIP_SERIES | tr '[A-Z]' '[a-z]')
    arch_flag=""
    # fixme; temporary solution
    if test "$CONFIG_CHIP_SERIES" = "AK3918AV200" ; then
    	chip_series="ak3918av200"
        arch_flag="ARCH=riscv"
    fi

    if test "$CONFIG_CHIP_SERIES" = "AK39EV200" ; then
    	chip_series="ak3918ev200"
    fi

    # 根据芯片类型配置uboot
    if test $CONFIG_FLASH_TYPE = "NAND" ; then
        if test "$CONFIG_SYS_BOOT_MODE" = "FAST" ; then
            make O=../ubd ${arch_flag} ${chip_series}_fast_nand_spl_defconfig CROSS_COMPILE=$CONFIG_CROSS_COMPILE
        else
            make O=../ubd ${arch_flag} ${chip_series}_nand_defconfig CROSS_COMPILE=$CONFIG_CROSS_COMPILE
        fi
    elif test $CONFIG_FLASH_TYPE = "NOR" ; then
        if test "$CONFIG_SYS_BOOT_MODE" = "FAST" ; then
            make O=../ubd ${arch_flag} ${chip_series}_fast_nor_spl_defconfig CROSS_COMPILE=$CONFIG_CROSS_COMPILE
        else
            make O=../ubd ${arch_flag} ${chip_series}_nor_defconfig  CROSS_COMPILE=$CONFIG_CROSS_COMPILE
        fi
    elif test $CONFIG_FLASH_TYPE = "MMC" ; then
        make O=../ubd ${arch_flag} ${chip_series}_mmc_defconfig  CROSS_COMPILE=$CONFIG_CROSS_COMPILE
    fi

	# 检测配置过程是否存在错误
    if test $? -ne 0 ; then
        exit 1
    fi

	# 编译uboot
    if test "$CONFIG_CHIP_SERIES" = "AK3918AV200" ; then
        if test "$CONFIG_FLASH_TYPE" = "MMC" ; then
            make O=../ubd ${arch_flag} -j$(nproc) DEVICE_TREE=${CONFIG_DTB_NAME}_mmc CROSS_COMPILE=$CONFIG_CROSS_COMPILE
        else
            make O=../ubd ${arch_flag} -j$(nproc) DEVICE_TREE=${CONFIG_DTB_NAME} CROSS_COMPILE=$CONFIG_CROSS_COMPILE 
        fi
    else
        make O=../ubd all -s -j$(nproc) DEVICE_TREE=${CONFIG_DTB_NAME} CROSS_COMPILE=$CONFIG_CROSS_COMPILE
    fi

	if test "$CONFIG_FLASH_TYPE" = "NAND" ; then
		spi_boot=$CONFIG_OS_DIR/opensbi/spi_nand_boot.bin
	else
		spi_boot=$CONFIG_OS_DIR/opensbi/spi_nor_boot.bin
	fi
    if test "$CONFIG_CHIP_TYPE" = "KM02X" ; then
		ddr_bin=$CONFIG_OS_DIR/opensbi/DDR3_256MB_PHY0_PHY1_900MHZ_P.dat
    else
		ddr_bin=$CONFIG_OS_DIR/opensbi/DDR3_128MB_PHY1_900MHZ_P.dat
    fi
    if test "$CONFIG_SYS_BOOT_MODE" = "FAST" ; then
        cat $spi_boot $ddr_bin $CONFIG_OS_DIR/ubd/spl/u-boot-spl.bin  > $CONFIG_OS_DIR/ubd/spl/u-boot-spl_spi_boot.bin
    fi

	# 检测编译过程是否存在错误
    if test $? -ne 0 ; then
        exit 1
    fi
}

build_uboot_spl()
{
    chip_series=$(echo $CONFIG_CHIP_SERIES | tr '[A-Z]' '[a-z]')

    # fixme; temporary solution
    if test "$CONFIG_CHIP_SERIES" = "AK39EV200" ; then
        chip_series="ak3918ev200"
    fi

    if test $CONFIG_UBOOT_AUTOUPGRADE = "y"; then
        autoupgrade_flag="uboot_upgrade_"
    fi

    # 根据芯片类型配置uboot
    if test $CONFIG_FLASH_TYPE = "NAND" ; then
        if test "$CONFIG_SYS_BOOT_MODE" = "FAST" ; then
            make O=../ubd ${arch_flag} ${chip_series}_fast_nand_spl_defconfig CROSS_COMPILE=$CONFIG_CROSS_COMPILE
        else
            make O=../ubd ${arch_flag} ${chip_series}_nand_spl_defconfig CROSS_COMPILE=$CONFIG_CROSS_COMPILE
        fi
    elif test $CONFIG_FLASH_TYPE = "NOR" ; then
        if test "$CONFIG_SYS_BOOT_MODE" = "FAST" ; then
            make O=../ubd ${arch_flag} ${chip_series}_fast_nor_spl_defconfig CROSS_COMPILE=$CONFIG_CROSS_COMPILE
        elif test $CONFIG_FAST_MODE = "fast" ; then
            make O=../ubd ${chip_series}_nor_spl_fast_${autoupgrade_flag}defconfig CROSS_COMPILE=$CONFIG_CROSS_COMPILE
        elif test $CONFIG_FAST_MODE = "fast_aov" ; then
            make O=../ubd ${chip_series}_nor_spl_fast_${autoupgrade_flag}defconfig CROSS_COMPILE=$CONFIG_CROSS_COMPILE
        elif test $CONFIG_FAST_MODE = "fast_uvc" ; then
            make O=../ubd ${chip_series}_nor_spl_fast_${autoupgrade_flag}defconfig CROSS_COMPILE=$CONFIG_CROSS_COMPILE
        else
            make O=../ubd ${chip_series}_nor_spl_${autoupgrade_flag}defconfig CROSS_COMPILE=$CONFIG_CROSS_COMPILE
        fi
    fi

    # 检测配置过程是否存在错误
    if test $? -ne 0 ; then
        exit 1
    fi

    # 编译uboot
    if test "$CONFIG_CHIP_SERIES" = "AK3918AV200" ; then
        make O=../ubd ${arch_flag} -j$(nproc) DEVICE_TREE=${CONFIG_DTB_NAME} CROSS_COMPILE=$CONFIG_CROSS_COMPILE 
    else
        make O=../ubd all -s -j$(nproc) DEVICE_TREE=${CONFIG_DTB_NAME} CROSS_COMPILE=$CONFIG_CROSS_COMPILE
    fi

	if test "$CONFIG_FLASH_TYPE" = "NAND" ; then
		spi_boot=$CONFIG_OS_DIR/opensbi/spi_nand_boot.bin
	else
		spi_boot=$CONFIG_OS_DIR/opensbi/spi_nor_boot.bin
	fi
    if test "$CONFIG_CHIP_TYPE" = "KM02X" ; then
		ddr_bin=$CONFIG_OS_DIR/opensbi/DDR3_256MB_PHY0_PHY1_900MHZ_P.dat
    else
		ddr_bin=$CONFIG_OS_DIR/opensbi/DDR3_128MB_PHY1_900MHZ_P.dat
    fi
    if test "$CONFIG_SYS_BOOT_MODE" = "FAST" ; then
        cat $spi_boot $ddr_bin $CONFIG_OS_DIR/ubd/spl/u-boot-spl.bin  > $CONFIG_OS_DIR/ubd/spl/u-boot-spl_spi_boot.bin
    fi

	# 检测编译过程是否存在错误
	if test $? -ne 0 ; then
		exit 1
	fi
}

#
# main entry
#
cd $CONFIG_OS_DIR/uboot

# 检测uboot源码是否已导入
if test $? -ne 0 ; then
    echo "Build u-boot failed, can't find uboot src dir!"
	exit 1
fi

mkdir -p ../ubd
mkdir -p $CONFIG_IMAGE_OUTPUT_DIR

case "$1" in
	normal)
		build_uboot
        if test "$CONFIG_CHIP_SERIES" != "AK3918AV200" ; then
    		cp -f ../ubd/u-boot.bin $CONFIG_IMAGE_OUTPUT_DIR
        fi
		;;
	spl)
		build_uboot_spl
        if test "$CONFIG_CHIP_SERIES" = "AK3918AV200" ; then
            cp -f $CONFIG_OS_DIR/ubd/spl/u-boot-spl_spi_boot.bin $CONFIG_IMAGE_OUTPUT_DIR
        fi
		cp -f ../ubd/spl/u-boot-spl.bin $CONFIG_IMAGE_OUTPUT_DIR
        if test  "$CONFIG_CHIP_SERIES" != "AK3918AV200" ; then
    		cp -f ../ubd/u-boot.bin $CONFIG_IMAGE_OUTPUT_DIR
        fi
        ;;
	clean)
		make O=../ubd clean
		make O=../ubd distclean
		rm -rf ../ubd
		make mrproper
		;;
	*)
		echo "Usage:"
		echo " $0 normal: build normal uboot"
		echo " $0 spl   : build uboot-spl"
		echo " $0 clean : clean uboot"
		;;
esac
