#!/bin/bash
workspace_dir=$(dirname $0)/
echo $workspace_dir
if [ -a $workspace_dir/config.mk ] ; then
    source $workspace_dir/config.mk
else
    echo "cannot found config.mk, aborting ... "
    exit 1
fi

# 编译模块
SelectCompiledModules()
{
    echo -n "Compiled Uboot $CompiledUboot? [y/n]"
    read -n 2 CompiledUboot
    if [ "$CompiledUboot" = "y" ]; then
        if test ! -d os/uboot ; then
            tar xvfz os/uboot.tar.gz -C os
        fi
        
        ./scripts/build_uboot.sh normal
        # 检查是否存在编译错误
        if test $? -ne 0 ; then
            echo "Uboot build failed!"
            exit
        fi
    fi

    echo -n "Compiled Kernel $CompiledKernel? [y/n]"
    read -n 2 CompiledKernel
    if [ "$CompiledKernel" = "y" ]; then
        if test ! -d os/kernel ; then
            tar xvfz os/linux.tar.gz -C os
        fi

        ./scripts/build_kernel.sh all
        # 检查是否存在编译错误
        if test $? -ne 0 ; then
            echo "Kernel build failed!"
            exit
        fi
    fi
}

SelectCompiledModules

# 将驱动KO安装到文件系统中
mkdir -p rootfs/ko
# 检查os/driver_src/是否存在，如果不存在就解压对应的配置的驱动
pwd
if test ! -d os/driver_src ; then
	tar -zxvf os/driver_normal.tar.gz -C os/
fi
# 直接拷贝驱动文件
cp -rf os/driver/* rootfs/ko/

./scripts/build_samples.sh all
# 检查是否存在编译错误
if test $? -ne 0 ; then
	echo "sample build failed!"
    exit 1
fi

# 制作文件系统
./scripts/build_rootfs.sh all

# 制作分区表
cd tools/envtool/
./build_env.sh
cd -

# 添加md5校验信息
cd image
md5sum * > burnfile.md5
cd -

# 安装镜像
cp -rf image/* tools/burntool

# 添加env*文件的md5校验信息
cd tools/burntool
md5sum env* >> burnfile.md5

# 选择是否生成映像升级文件
echo "What is your Compile Image File? [y/n]"
# 使用time命令和kill信号处理实现超时
time read -t 10 -n 2 Image
# 检查是否超时，如果超时则$Image将为空
if [ -z "$Image" ]; then
    echo "No input after 10 seconds. Exiting."
    exit 1
elif [ "$Image" = "y" ]; then
    ./partition_image.sh
else
    echo "Skipping image compile."
fi