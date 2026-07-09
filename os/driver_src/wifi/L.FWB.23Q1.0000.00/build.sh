#!/bin/bash
CUR_DIR=$(dirname $(readlink -f "$0"))
echo "CUR_DIR=${CUR_DIR}"
SSV_DRV_PATH=${CUR_DIR}
 
# 300L
KERNEL_OBJ_PATH=../../../kbd/
PLATFORM_TOOLCHAIN=/opt/arm-anycloud-linux-uclibcgnueabi/bin/arm-anycloud-linux-uclibcgnueabi-
INSTALL_PATH=${SSV_DRV_PATH}/_av100_install
PLATFORM_ARCH=arm
# 获取CPU核数，后续编译时用于提高编译效率
get_num_of_cpu_cores() {
    num_of_cpu_cores=$(cat /proc/cpuinfo | grep "processor" | wc -l)
}
get_num_of_cpu_cores
echo "########### num_of_cpu_cores:${num_of_cpu_cores} ###########"
echo "########### step 1: make clean ###########"
make -j${num_of_cpu_cores} -C $SSV_DRV_PATH KERNEL_OBJ_PATH=$KERNEL_OBJ_PATH SSV_DRV_PATH=$SSV_DRV_PATH ARCH=$PLATFORM_ARCH CROSS_COMPILE=$PLATFORM_TOOLCHAIN clean
echo "########### step 2: make modules ###########"
make -j${num_of_cpu_cores} -C $SSV_DRV_PATH KERNEL_OBJ_PATH=$KERNEL_OBJ_PATH SSV_DRV_PATH=$SSV_DRV_PATH ARCH=$PLATFORM_ARCH CROSS_COMPILE=$PLATFORM_TOOLCHAIN modules
echo "########### step 3: make strip ###########"
make -j${num_of_cpu_cores} -C $SSV_DRV_PATH SSV_DRV_PATH=$SSV_DRV_PATH CROSS_COMPILE=$PLATFORM_TOOLCHAIN strip
echo "########### step 4: make install ###########"
make -j${num_of_cpu_cores} -C $SSV_DRV_PATH KERNEL_OBJ_PATH=$KERNEL_OBJ_PATH SSV_DRV_PATH=$SSV_DRV_PATH ARCH=$PLATFORM_ARCH CROSS_COMPILE=$PLATFORM_TOOLCHAIN INSTALL_PATH=$INSTALL_PATH install
echo "########### step 5: make over  ###########"