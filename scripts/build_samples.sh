#!/bin/bash

pwd_dir=$(dirname $0)

source $pwd_dir/../config.mk

#
# Preparing for building
#
get_config_info()
{
	sample_install_dir=$CONFIG_TOP_DIR/rootfs/samples
	
	export CONFIG_FAST_MODE
}

#
# Build driver samples
#
build_driver_samples()
{
	cd $CONFIG_TOP_DIR/os/driver/external/sample
	make CROSS_COMPILE=$CONFIG_CROSS_COMPILE
	if [ $? -ne 0 ]; then
		echo -e "\033[31m driver samples built failed! Aborting\033[0m"
		exit 1
	fi

	make install INSTALL=$sample_install_dir
	cd -
}

#
# Build platform samples
#
build_platform_samples()
{
	cd $CONFIG_TOP_DIR/platform/sample
	make CROSS_COMPILE=$CONFIG_CROSS_COMPILE CONFIG_FAST_MODE=$CONFIG_FAST_MODE
	if [ $? -ne 0 ]; then
		echo -e "\033[31m platform samples built failed! Aborting\033[0m"
		exit 1
	fi

	make install SAMPLE_INSTALL_DIR=$sample_install_dir
	cd -
}

#
#init output samples
#
init_sample_folder()
{
    if [ -d $sample_install_dir ]; then
        rm -rf $sample_install_dir/* 
    else
        mkdir $sample_install_dir
    fi
}

#
# Clean all modules
#
clean_all()
{
    # Clean platform samples
	if [ -d $CONFIG_TOP_DIR/platform/sample ]; then
		cd $CONFIG_TOP_DIR/platform/sample
		make clean
		cd -
	fi

	# Clean driver samples
	if [ -d $CONFIG_TOP_DIR/os/driver/external/sample ]; then
		cd $CONFIG_TOP_DIR/os/driver/external/sample
		make clean
		cd -
	fi
}

#
# Main entry
#
get_config_info

init_sample_folder
case "$1" in
	all)
		build_driver_samples
		build_platform_samples
		;;
	drv)
		build_driver_samples
		;;
	plat)
		build_platform_samples
		;;
	clean)
		clean_all
		;;
	*)
		echo "Usage:"
		echo " $0 all  : build all samples"
		echo " $0 drv  : build driver samples"
		echo " $0 plat : build platform samples"
		echo " $0 clean: clean all"
		;;
esac
