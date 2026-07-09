# U-Boot 2019.10.0 #

## 版本说明 ##

* 支持AK37E芯片
* 支持spi norflash, spi nandflash, spi norfalsh_SPL, spi nandfalsh_SPL启动
* 支持从TF卡、以太网tftp、串口下载镜像和升级镜像
* 支持uboot自升级
* 支持NFS调试
* 支持DTS，UBOOT中自带dtb，编译的时候和uboot合并到一个镜像中；
* 支持标准分区

## 编译使用说明 ##
一般在编译Uboot源码的时候，在同级目录下创建ubd编译输出目录：
    make O=/tmp/build distclean
    make O=/tmp/build NAME_defconfig
    make O=/tmp/build all
如果需要修改uboot编译配置选项，可以这样操作：
		make O=/tmp/build menuconfig  
		
或者

    make distclean
    make NAME_defconfig 
    make all DEVICE_TREE=DTB_NAME -j4 CROSS_COMPILE=工具链名
    如下：
    LINUX平台用的工具链名：
    make all DEVICE_TREE=EVB_CBDR_AK3760E_V1.0.1 -j4 CROSS_COMPILE=arm-anykav500-linux-uclibcgnueabi-
    ALIOS系统上用的工具链名：
    make O=../ubd all DEVICE_TREE=EVB_CBDR_AK3760E_V1.0.1 -j4 CROSS_COMPILE=arm-none-eabi-
    RTT上用的工具链名：
    make O=../ubd all DEVICE_TREE=EVB_CBDR_AK3760E_V1.0.1 -j4 CROSS_COMPILE=arm-anykav500-eabi-
如果需要修改uboot编译配置选项，可以这样操作：
		make menuconfig 

## 平台编译使用说明 ##
平台ak37e编译使用说明（基于spi norflash配置）：
    make O=../ubd distclean
    make O=../ubd anycloud_ak37e_nor_defconfig
如果要包含rgb的dtb文件则为：
    make O=../ubd all DEVICE_TREE="EVB_CBDR_AK3760E_V1.0.1"  
如果要包含mipi的dtb文件则为：
	  make O=../ubd  all DEVICE_TREE="EVB_CBDM_AK3760E_V1.0.0"
 
平台ak37e编译使用说明（基于spi nandflash配置）：
    make O=../ubd distclean
    make O=../ubd anycloud_ak37e_nand_defconfig
如果要包含rgb的dtb文件则为：
    make O=../ubd all DEVICE_TREE="EVB_CBDR_AK3760E_V1.0.1"  
如果要包含mipi的dtb文件则为：
	  make O=../ubd  all DEVICE_TREE="EVB_CBDM_AK3760E_V1.0.0"

## 编译Uboot配置项使用说明 ##
1.ak37e平台spi nandflash的uboot配置项文件：
anycloud_ak37e_nand_defconfig 

2.ak37e平台spi nandflash的uboot+spl配置项文件：
anycloud_ak37e_nand_spl_defconfig 

3.ak37e平台spi norflash的uboot配置项文件：
anycloud_ak37e_nor_defconfig 

4.ak37e平台spi norflash的uboot+spl配置项文件：
anycloud_ak37e_nor_spl_defconfig

## 双网卡TFTP下载设备节点切换使用说明，命令行执行下面的环境变量配置命令 ##
env set ethact eth1@20500000
切换到MAC1
然后再tftp

env set ethact eth0@20300000
切换到MAC0
然后再tftp

