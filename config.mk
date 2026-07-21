#@help: AnyCloud39AV100项目的版本配置，支持AK3918AV100N/P/X
################################################################################
#  (1)核心配置区
################################################################################
# 芯片系列定义 (AK37E, AK37D, AK39EV33X, AK3918AV100, AK3918EV300L, KM01A, AK3918AV130)
CONFIG_CHIP_SERIES=AK3918AV130

# WIFI类型配置(NO_WIFI, atbm6031, atbm6032, atbm6031x, atbm6012bx, atbm6062u, atbm6062s, rtl8188ftv, rtl8189ftv, ssv6x5x, aic8800)
CONFIG_WIFI_TYPE=rtl8188ftv

# SENSOR类型配置
CONFIG_SENSOR_TYPE=0

# Flash类型配置(NOR, NAND)
CONFIG_FLASH_TYPE=NOR

# U-boot板级配置选择
#EVB_CBD_AK3918AV130N_V1.0.0
#EVB_CBD_AK3918AV130_V1.0.0
#EVB_CBD_AK3918AV130A_V1.0.0
CONFIG_DTB_NAME=EVB_CBD_AK3918AV130_V1.0.0

###############################################################################
# 全局路径配置
###############################################################################
# Toolchain Top Path
CONFIG_CROSS_COMPILE=arm-anycloud-linux-uclibcgnueabi-

# Kernel/Driver/Uboot Source Top Path
CONFIG_OS_DIR=`pwd`//os

# Kernel/Driver/Uboot Source Top Path
CONFIG_PLATFORM_DIR=`pwd`//platform

# Installing Path, usually is 
CONFIG_TOP_DIR=`pwd`/

# Image output path
CONFIG_IMAGE_OUTPUT_DIR=`pwd`//image

###############################################################################
# 启动模式配置 no_fast;  fast; fast_aov: fast+aov  aov: aov fast_uvc: fast+uvc no_fast_amp: no_fast+amp
###############################################################################
CONFIG_FAST_MODE=no_fast

#快启sensor数量，1：单目， 2：双目
CONFIG_FAST_SENSOR_NUM=1

#快启sensor名称( sc3332, sc2337p, gc20c3 )
CONFIG_SENSOR_NAME=sc3332

CONFIG_FAST_DEBUG=0

CONFIG_FAST_FS=romfs

#快启编译uboot且支持upgrade
CONFIG_UBOOT_AUTOUPGRADE=n

#fastsys配置文件(ak3918av130_config, ak3918av130_fastae_config)
CONFIG_FASTSYS_DEFCONFIG=ak3918av130_fastae_config

#配置为all的时候表示release 的sdk支持常电和快启，配置为single的时候表示只支持其中一种。
CONFIG_RELEASE_MODE=all
