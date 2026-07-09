#!/bin/bash

# 包含编译配置选项
pwd_dir=$(dirname $0)
if [ -a $pwd_dir/../config.mk ] ; then
	source $pwd_dir/../config.mk
else
	echo "cannot found config.mk, aborting ... "
	exit 1
fi

# 定义全局开关变量（true/false）
RTC_SAMPLE_TEST=false

get_config_info()
{
	FLASH_TYPE=$CONFIG_FLASH_TYPE
	SCREEN_TYPE=$SCREEN_TYPE
	WIFI_TYPE=$CONFIG_WIFI_TYPE
	CHIP_SERIES=$CONFIG_CHIP_SERIES
	SYS_BOOT_MODE=$CONFIG_SYS_BOOT_MODE

    rootfs_dir=$CONFIG_TOP_DIR/rootfs
    driver_install_dir=$CONFIG_TOP_DIR/os/driver
    image_install_dir=$CONFIG_IMAGE_OUTPUT_DIR
    sample_install_dir=$rootfs_dir/samples
    uboot_tools_dir=$CONFIG_TOP_DIR/tools/uboot_tools
	dss_tools_dir=$CONFIG_TOP_DIR/tools/dss_tool
	isp_ptool_dir=$CONFIG_TOP_DIR/tools/isp_partition_tool

  if [ $CHIP_SERIES = "AK3918AV200" ];then
  	if [ "$SYS_BOOT_MODE" = "FAST" ];then
      driver_ko_dir=$CONFIG_TOP_DIR/os/driver/external/fast
  	else
			driver_ko_dir=$CONFIG_TOP_DIR/os/driver/external/normal
  	fi     
  else
  	driver_ko_dir=$CONFIG_TOP_DIR/os/driver/external/
  fi

    echo "FLASH_TYPE=$FLASH_TYPE, WIFI_TYPE=$WIFI_TYPE, CHIP_SERIES=$CHIP_SERIES"
}

extract_fs()
{
    # 解压缩根文件系统
#    if [ -d "resource" ]; then
#        echo "delete the resource folder !"
#        rm "resource" -rf
#    fi


    if 	test $CHIP_SERIES = "AK3918AV200" ; then
        if [ -d "resource" ]; then
            echo "delete the resource folder !"
            rm "resource" -rf
        fi
        rm -rf rootfs.tar.gz 
        if test "$CONFIG_CROSS_COMPILE" = "riscv64-anyka-linux-musl-" ; then
            echo "musl rootfs!"
            tar zxvf rootfs_av200_musl.tar.gz  -C $rootfs_dir
            ln -s busybox_av200_musl.tar.gz rootfs.tar.gz
        elif test "$CONFIG_CROSS_COMPILE" = "riscv64-anyka-linux-gnu-"; then
            echo "glibc rootfs!"
            tar zxvf rootfs_av200_glibc.tar.gz -C $rootfs_dir
            ln -s busybox_av200_glibc.tar.gz rootfs.tar.gz
        else
            echo "The toochain $CONFIG_CROSS_COMPILE did not match the file system!"
            exit 1
        fi
    else
        echo "extract basic resource"
 #       tar zxvf resouce.tar.gz -C $rootfs_dir
    fi

    ./extract.sh
}

process_wifi_driver()
{
    rm -rf rootfs/usr/modules/rtl*
    rm -rf rootfs/usr/modules/atbm*
    rm -rf rootfs/usr/modules/ssv*
    rm -rf rootfs/usr/modules/cfg*
    rm -rf rootfs/usr/modules/ATBM*
    rm -rf rootfs/usr/modules/fb*.ko
    rm -fr rootfs/usr/modules/aic*.ko

    # 根据选择的WIFI类型，清理掉无用的驱动
    if test $WIFI_TYPE = "NO_WIFI" ; then
		true
    elif test $WIFI_TYPE = "rtl8188ftv" ; then
      if 	test $CHIP_SERIES = "AK3918AV200" ; then
        cp -rf $driver_ko_dir/rtl8188fu.ko rootfs/usr/modules/
      else
        cp -rf $driver_ko_dir/rtl8188ftv.ko rootfs/usr/modules/
      fi
    elif test $WIFI_TYPE = "rtl8189ftv" ; then
        cp -rf $driver_ko_dir/rtl8189ftv.ko rootfs/usr/modules/
    elif test $WIFI_TYPE = "atbm6031" ; then
		cp -rf $driver_ko_dir/atbm6031.ko rootfs/usr/modules/
  elif test $WIFI_TYPE = "atbm6032" ; then
		cp -rf $driver_ko_dir/atbm6032.ko rootfs/usr/modules/
  elif test $WIFI_TYPE = "atbm6012b" ; then
		cp -rf $driver_ko_dir/atbm6032.ko rootfs/usr/modules/
  elif test $WIFI_TYPE = "atbm6132" ; then
    cp -rf $driver_ko_dir/atbm6132_wifi_usb.ko rootfs/usr/modules/
	elif test $WIFI_TYPE = "atbm6031x" ; then
		cp -rf $driver_ko_dir/atbm6031x.ko rootfs/usr/modules/
	elif test $WIFI_TYPE = "atbm6012bx" ; then
		cp -rf $driver_ko_dir/atbm6012bx.ko rootfs/usr/modules/
	elif test $WIFI_TYPE = "atbm6062u" ; then
		cp -rf $driver_ko_dir/ATBM606x_wifi_usb.ko rootfs/usr/modules/
		cp -rf $driver_ko_dir/cfg80211.ko rootfs/usr/modules/
	elif test $WIFI_TYPE = "atbm6062s" ; then
		cp -rf $driver_ko_dir/ATBM606x_wifi_sdio.ko rootfs/usr/modules/
		cp -rf $driver_ko_dir/cfg80211.ko rootfs/usr/modules/
        elif test $WIFI_TYPE = "atbm6162" ; then
                cp -rf $driver_ko_dir/ATBM6162_wifi_sdio.ko rootfs/usr/modules/
                cp -rf $driver_ko_dir/cfg80211.ko rootfs/usr/modules/	
    elif test $WIFI_TYPE = "ssv6x5x" ; then
        cp -rf $driver_ko_dir/ssv6x5x.ko rootfs/usr/modules/
    elif test $WIFI_TYPE = "aic8800" ; then
        cp -rf $driver_ko_dir/aic8800_netdrv.ko rootfs/usr/modules/
    elif test $WIFI_TYPE = "aic8800d40l" ; then
        cp -rf $driver_ko_dir/aic8800_bsp.ko rootfs/usr/modules/
        cp -rf $driver_ko_dir/aic8800_fdrv.ko rootfs/usr/modules/
        mkdir -p rootfs/usr/lib/firmware/
        cp -rf $driver_install_dir/external/firmware/* rootfs/usr/lib/firmware/
    fi
}

process_screen_driver()
{
    rm -rf rootfs/usr/modules/ts_*

    if test -z $SCREEN_TYPE ; then
        return
    fi

    if test $SCREEN_TYPE = "NO_SCREEN" ; then
        true
    elif test $SCREEN_TYPE = "RGB" ; then
        cp -rf $driver_ko_dir/ts_icn85xx.ko rootfs/usr/modules/
        true
    elif test $SCREEN_TYPE = "MIPI" ; then
        cp -rf $driver_ko_dir/ts_icn85xx.ko rootfs/usr/modules/
        true
    else
        echo "Screen Type Error, please check!"
        exit 1
    fi
}

process_sensor_driver()
{
    rm -rf rootfs/usr/modules/sensor_*

    #cp -rf ko/external/sensor_*.ko rootfs/usr/modules/
    if test $CHIP_SERIES = "AK37E" ; then
    	cp -rf $driver_install_dir/external/sensor_gc0308.ko rootfs/usr/modules/
    	cp -rf $driver_install_dir/external/sensor_tp9950.ko rootfs/usr/modules/
    elif 	test $CHIP_SERIES = "AK37D" ; then
        cp -rf $driver_install_dir/external/sensor_ar0230.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_pr2000.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_ps5268.ko rootfs/usr/modules/
    elif 	test $CHIP_SERIES = "AK39EV33X" ; then
        cp -rf $driver_install_dir/external/sensor_f22_f23_f28_f35_f37.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc3335.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_gc4653.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_gc2063.ko rootfs/usr/modules/
	elif 	test $CHIP_SERIES = "AK3918AV130" ; then
		if test "$CONFIG_FAST_MODE" = "fast" || test "$CONFIG_FAST_MODE" = "fast_aov"; then
			cp -rf $driver_install_dir/external/sensor_sc2337p.ko rootfs/usr/modules/
			cp -rf $driver_install_dir/external/sensor_sc3332.ko rootfs/usr/modules/
			cp -rf $driver_install_dir/external/sensor_gc20c3.ko rootfs/usr/modules/
			cp -rf $driver_install_dir/external/sensor_gc1084.ko rootfs/usr/modules/
		elif test "$CONFIG_FAST_MODE" = "no_fast"; then
			cp -rf $driver_install_dir/external/sensor_gc1084.ko rootfs/usr/modules/
			cp -rf $driver_install_dir/external/sensor_gc2063.ko rootfs/usr/modules/
			cp -rf $driver_install_dir/external/sensor_gc3003.ko rootfs/usr/modules/
			cp -rf $driver_install_dir/external/sensor_sc2331x.ko rootfs/usr/modules/
			cp -rf $driver_install_dir/external/sensor_sc2337p.ko rootfs/usr/modules/
			cp -rf $driver_install_dir/external/sensor_gc20c3.ko rootfs/usr/modules/
			cp -rf $driver_install_dir/external/sensor_sc3332.ko rootfs/usr/modules/
			cp -rf $driver_install_dir/external/sensor_cv2005.ko rootfs/usr/modules/
		else
			echo "错误的CONFIG_FAST_MODE!"
		fi
	elif 	test $CHIP_SERIES = "KM01A" ; then
        cp -rf $driver_install_dir/external/sensor_sc431ai.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc2337p.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc2336p.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc200ai.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc2336.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_cv2005.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_virtual.ko rootfs/usr/modules/
    elif 	test $CHIP_SERIES = "AK3918AV130" ; then
        cp -rf $driver_install_dir/external/sensor_gc2063.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc2337p.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc3332.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc2331x.ko rootfs/usr/modules/
    elif 	test $CHIP_SERIES = "AK3918AV100" ; then
        cp -rf $driver_install_dir/external/sensor_sc3336.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_gc4653.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_gc2063.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc2336.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc2336x.ko rootfs/usr/modules/
		cp -rf $driver_install_dir/external/sensor_sc1346.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_mis2008.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc223a.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc401ai.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_gc3003.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc5238.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_h63_h63p.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_gc2083.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/ak_tsensor.ko rootfs/usr/modules/
    elif 	test $CHIP_SERIES = "AK3918EV300L" ; then
        cp -rf $driver_install_dir/external/sensor_gc2063.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_gc2083.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc2336.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc223a.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc3336.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_gc3003.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_h63_h63p.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_sc1346.ko rootfs/usr/modules/
        cp -rf $driver_install_dir/external/sensor_mis2006.ko rootfs/usr/modules/
    elif 	test $CHIP_SERIES = "AK3918AV200" ; then
        cp $driver_ko_dir/sensor*.ko rootfs/usr/modules/
    else
    	exit 1
    fi
}

# 删除不对外发布的AI模型文件
rm_nne_model() {
	# 以下代码决定 utils/usr/sbin和rootfs/usr/sbin目录需要保留的AI模型文件
    local dirs=("utils/usr/sbin" "rootfs/usr/sbin")

    if test $CHIP_SERIES = "AK3918AV130"; then
		# utils_keep_patterns 这个是utils/usr/sbin目录需要保留的AI模型文件
        local utils_keep_patterns=("model_0x0A000000_*.bin" "model_0x0A00000D_*.bin")
		# rootfs_keep_patterns 这个是rootfs/usr/sbin目录需要保留的AI模型文件
        local rootfs_keep_patterns=("model_0x0A000000_*.bin")
    elif test $CHIP_SERIES = "KM01A"; then
        local utils_keep_patterns=("model_0x0A000000_*.bin" "model_0x0A00000D_*.bin" "model_0x0A000002_*.bin" "model_0x0A000008_*.bin")
        local rootfs_keep_patterns=("model_0x0A000000_*.bin" "model_0x0A000008_*.bin")
    elif test $CHIP_SERIES = "AK3918AV100"; then
        local utils_keep_patterns=("model_0x0A000000_*.bin" "model_0x0A00000D_*.bin" "model_0x0A000002_*.bin" "model_0x0A000008_*.bin")
        local rootfs_keep_patterns=("model_0x0A000008_*.bin")
    elif test $CHIP_SERIES = "AK3918AV200"; then
        local utils_keep_patterns=("yolov5n_v1_640")
        local rootfs_keep_patterns=("yolov5n_v1_640")
    else
        local utils_keep_patterns=("model_0x0A000000_*.bin" "model_0x0A00000D_*.bin")
        local rootfs_keep_patterns=("model_0x0A000000_*.bin")
    fi

    for dir in "${dirs[@]}"; do
        echo "Processing: $dir"

        # 根据目录决定保留模式
        local keep_patterns=()
        if [ "$dir" = "utils/usr/sbin" ]; then
            keep_patterns=("${utils_keep_patterns[@]}")
        elif [ "$dir" = "rootfs/usr/sbin" ]; then
            keep_patterns=("${rootfs_keep_patterns[@]}")
        else
            echo "Skipping unknown directory: $dir"
            continue
        fi

        # 初始化数组
        declare -a model_files
		model_files=()

        # 读取 find 输出，按 null 分隔
        while IFS= read -r -d '' file; do
            model_files+=("$file")
        done < <(find "$dir" -type f \( -name "model_*.bin" -o -name "model_checksum.md5" -o -name "model_aicrydet_*.bin" -o -name "yolov*" \) -print0)

        #echo "数组长度: ${#model_files[@]}"

        # 处理文件
        for file in "${model_files[@]}"; do
            local keep=0
            local filename=$(basename "$file")

            for pattern in "${keep_patterns[@]}"; do
                case "$filename" in
                    $pattern) keep=1 ;;
                esac
                if [ "$keep" -eq 1 ]; then
                    break
                fi
            done

            if [ "$keep" -eq 0 ]; then
                echo "Removing: $file"
                rm -f "$file"
            else
                echo "Keeping: $file"
            fi
        done
    done
}

rm_isp_conf() 
{
    # 清理不对外发布的isp config文件
    # 不同配置需要保留的isp配置文件是不一样的

    local dirs=( "utils/etc" "rootfs/etc" )
	if 	test $CHIP_SERIES = "AK3918AV130"; then
		# utils_keep_patterns 这个是utils/etc目录需要保留的isp配置文件
		# rootfs_keep_patterns 这个是rootfs/etc目录需要保留的isp配置文件
		if test "$CONFIG_FAST_MODE" = "fast" || test "$CONFIG_FAST_MODE" = "fast_aov"; then
			#快启模式
			local utils_keep_patterns=( "isp_sc2337p_mipi_2lane_av130.conf" "isp_sc2337p_mipi_2lane_av130_dual.conf" "isp_sc3332_mipi_2lane_av130.conf" "isp_gc20c3_mipi_2lane_av130.conf" "isp_gc20c3_mipi_2lane_av130_dual.conf" )
			local rootfs_keep_patterns=( "isp_sc2337p_mipi_2lane_av130.conf" "isp_sc2337p_mipi_2lane_av130_dual.conf" "isp_sc3332_mipi_2lane_av130.conf" "isp_gc20c3_mipi_2lane_av130.conf" "isp_gc20c3_mipi_2lane_av130_dual.conf" )
		else
			#常电模式
			local utils_keep_patterns=( "isp_gc3003_mipi_2lane_av130_dual_hp.conf" "isp_gc3003_mipi_2lane_av130_hp.conf" "isp_sc2331x_mipi_2lane_av130_tri_fsync.conf" "isp_sc2337p_mipi_2lane_av130_dual.conf" )
			local rootfs_keep_patterns=( "isp_gc3003_mipi_2lane_av130_dual_hp.conf" "isp_gc3003_mipi_2lane_av130_hp.conf" "isp_sc2331x_mipi_2lane_av130_tri_fsync.conf" "isp_sc2337p_mipi_2lane_av130_dual.conf" )
		fi
	elif 	test $CHIP_SERIES = "KM01A" ; then
		local utils_keep_patterns=( "isp_cv2005_mipi_2lane_km01a.conf" "isp_sc200ai_mipi_2lane_km01a.conf" "isp_sc431hai_mipi_2lane_km01a.conf" "isp_sc2337p_mipi_2lane_km01a.conf" "isp_sc2337p_mipi_2lane_km01a_dual.conf" "isp_sc2337p_mipi_2lane_km01a_triple.conf" )
		local rootfs_keep_patterns=( "isp_cv2005_mipi_2lane_km01a.conf" "isp_sc200ai_mipi_2lane_km01a.conf" "isp_sc431hai_mipi_2lane_km01a.conf" "isp_sc2337p_mipi_2lane_km01a.conf" "isp_sc2337p_mipi_2lane_km01a_dual.conf" "isp_sc2337p_mipi_2lane_km01a_triple.conf" )
	elif 	test $CHIP_SERIES = "AK3918AV100" ; then
		local utils_keep_patterns=( "isp_cv2005_mipi_2lane_km01a.conf" "isp_sc200ai_mipi_2lane_km01a.conf" "isp_sc431hai_mipi_2lane_km01a.conf" "isp_sc2337p_mipi_2lane_km01a.conf" "isp_sc2337p_mipi_2lane_km01a_dual.conf" "isp_sc2337p_mipi_2lane_km01a_triple.conf" )
		local rootfs_keep_patterns=( "isp_cv2005_mipi_2lane_km01a.conf" "isp_sc200ai_mipi_2lane_km01a.conf" "isp_sc431hai_mipi_2lane_km01a.conf" "isp_sc2337p_mipi_2lane_km01a.conf" "isp_sc2337p_mipi_2lane_km01a_dual.conf" "isp_sc2337p_mipi_2lane_km01a_triple.conf" )
	elif 	test $CHIP_SERIES = "AK3918AV200" ; then
		local utils_keep_patterns=( "isp_cv2003_mipi_2lane_av200.conf" )
		local rootfs_keep_patterns=( "isp_cv2003_mipi_2lane_av200.conf" )
	else
		local utils_keep_patterns=( "" )
		local rootfs_keep_patterns=( "" )
	fi

    for dir in "${dirs[@]}"; do
        echo "Processing: $dir"

        # 根据目录决定保留模式
        local keep_patterns=()
        if [ "$dir" = "utils/etc" ]; then
            keep_patterns=("${utils_keep_patterns[@]}")
        elif [ "$dir" = "rootfs/etc" ]; then
            keep_patterns=("${rootfs_keep_patterns[@]}")
        else
            echo "Skipping unknown directory: $dir"
            continue
        fi

        # 初始化数组
        declare -a model_files
		model_files=()

        # 读取 find 输出，按 null 分隔
        while IFS= read -r -d '' file; do
            model_files+=("$file")
        done < <(find "$dir" -type f \( -name "isp_*.conf" \) -print0)

        echo "数组长度: ${#model_files[@]}"

        # 处理文件
        for file in "${model_files[@]}"; do
            local keep=0
            local filename=$(basename "$file")

            for pattern in "${keep_patterns[@]}"; do
                case "$filename" in
                    $pattern) keep=1 ;;
                esac
                if [ "$keep" -eq 1 ]; then
                    break
                fi
            done

            if [ "$keep" -eq 0 ]; then
                echo "Removing: $file"
                rm -f "$file"
            else
                echo "Keeping: $file"
            fi
        done
    done
}


# 安装驱动到文件系统  
install_driver()
{
    # 删除文件统中遗留的文件
	if test -d rootfs/usr/modules ; then
        rm -rf rootfs/usr/modules/*
        rm -rf rootfs/lib/modules/*
    else
		mkdir rootfs/usr/modules
	fi

    #拷贝driver ko
    cp -rf $driver_ko_dir/*.ko rootfs/usr/modules

    #拷贝kernel ko
    cp -rf $driver_install_dir/internal/lib rootfs/
    rm -rf rootfs/lib/modules/4.4.*/kernel/crypto/
    rm -rf rootfs/lib/modules/4.4.*/kernel/drivers/staging/rtl8188eu
    rm -rf rootfs/lib/modules/4.4.*/kernel/net/wireless/mac80211.ko
    rm -rf rootfs/lib/modules/4.4.*/kernel/net/bridge/*.ko
    rm -rf rootfs/lib/modules/4.4.*/kernel/net/llc/llc.ko
    rm -rf rootfs/lib/modules/4.4.*/kernel/net/802/stp.ko

    process_wifi_driver

    process_screen_driver

    process_sensor_driver
}

# 安装平台中间件的库文件到文件系统
install_platform()
{
    cp -rf $CONFIG_TOP_DIR/platform/lib/*.so rootfs/usr/lib
}

process_flash_scripts()
{
    if test $FLASH_TYPE = "NOR" ; then
		if test "$CONFIG_FAST_MODE" = "fast" || test "$CONFIG_FAST_MODE" = "fast_aov"; then
			if test $CONFIG_FAST_SENSOR_NUM = "1"; then
				cp ./scripts/flash/rc.local.fast.sig.nor rootfs/etc/init.d/rc.local
			elif test $CONFIG_FAST_SENSOR_NUM = "2"; then
				cp ./scripts/flash/rc.local.fast.dual.nor rootfs/etc/init.d/rc.local
			elif test $CONFIG_FAST_SENSOR_NUM = "3"; then
				cp ./scripts/flash/rc.local.fast.triple.nor rootfs/etc/init.d/rc.local
			fi
			
			cp ./scripts/flash/update.fast.nor.sh rootfs/sbin/update.sh
		elif test "$CONFIG_FAST_MODE" = "fast_uvc" ; then
            		cp ./scripts/flash/rc.local.fast_uvc.nor rootfs/etc/init.d/rc.local
			cp ./scripts/flash/update.fast.nor.sh rootfs/sbin/update.sh
		else
			cp ./scripts/flash/rc.local.nor rootfs/etc/init.d/rc.local
			cp ./scripts/flash/update.nor.sh rootfs/sbin/update.sh
		fi

    elif test $FLASH_TYPE = "NAND" ; then
        cp ./scripts/flash/rc.local.nand rootfs/etc/init.d/rc.local
        cp ./scripts/flash/update.nand.sh rootfs/sbin/update.sh
		cp ./scripts/flash/nand_update rootfs/bin/nand_update
    elif test $FLASH_TYPE = "MMC" ; then
        echo "update.sh didn't support now!"
        cp ./scripts/flash/rc.local.mmc rootfs/etc/init.d/rc.local
    else
        echo "Flash type error, please check!"
        exit 1
    fi
    
    if test "$CONFIG_SYS_BOOT_MODE" = "FAST" ; then
    	cp ./scripts/flash/fast.rcS rootfs/etc/init.d/rcS
    fi
	if 	test $CHIP_SERIES = "AK3918AV200" ; then
		cp ./scripts/flash/update_ddr.sh rootfs/sbin/update_ddr.sh
	fi
}

process_sensor_scripts()
{
    cp scripts/sensor/*.sh rootfs/usr/sbin/
}

process_screen_scripts()
{
    if test -z $SCREEN_TYPE ; then
        return
    fi

    if test $SCREEN_TYPE = "NO_SCREEN" ; then
        true
    elif test $SCREEN_TYPE = "RGB" ; then
        sed -i 's/ts_icn86xx/ts_icn85xx/' scripts/screen/screen.sh
        cp scripts/screen/screen.sh rootfs/usr/sbin/
    elif test $SCREEN_TYPE = "MIPI" ; then
        sed -i 's/ts_icn86xx/ts_icn85xx/' scripts/screen/screen.sh
        cp scripts/screen/screen.sh rootfs/usr/sbin/
    else
        echo "Screen Type Error, please check!"
        exit 1
    fi
}

process_lcd_scripts()
{
    cp $driver_ko_dir/ak_lcd.ko rootfs/usr/modules/
    cp scripts/screen/lcd.sh rootfs/usr/sbin/
}

process_wifi_scripts()
{
    # 根据选择的WIFI类型，清理掉无用的驱动
    if test $WIFI_TYPE = "NO_WIFI" ; then
        true
    elif test $WIFI_TYPE = "rtl8188ftv" ; then
        cp scripts/wifi/rtl8188.sh rootfs/usr/sbin/wifi_driver.sh
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli_6062
        rm rootfs/usr/sbin/atbm_tool_6062
        rm rootfs/usr/sbin/nimble_linux_6062
		if 	test $CHIP_SERIES = "AK3918AV200" ; then
			rm rootfs/usr/sbin/atbm_cli_6132
			rm rootfs/usr/sbin/atbm_tool_6132
			rm rootfs/usr/sbin/nimble_linux_6132
		fi
        rm rootfs/usr/sbin/atbm_cli_6031x_300w
        rm rootfs/usr/sbin/atbm_tool_6031x_300w
        rm rootfs/usr/sbin/nimble_linux_6031x_300w
        #rm rootfs/usr/sbin/custom_msg
        #rm rootfs/usr/sbin/fasync_user
    elif test $WIFI_TYPE = "rtl8189ftv" ; then
        cp scripts/wifi/rtl8189.sh rootfs/usr/sbin/wifi_driver.sh
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli_6062
        rm rootfs/usr/sbin/atbm_tool_6062
        rm rootfs/usr/sbin/nimble_linux_6062
		if 	test $CHIP_SERIES = "AK3918AV200" ; then
			rm rootfs/usr/sbin/atbm_cli_6132
			rm rootfs/usr/sbin/atbm_tool_6132
			rm rootfs/usr/sbin/nimble_linux_6132
		fi
        rm rootfs/usr/sbin/atbm_cli_6031x_300w
        rm rootfs/usr/sbin/atbm_tool_6031x_300w
        rm rootfs/usr/sbin/nimble_linux_6031x_300w
        #rm rootfs/usr/sbin/custom_msg
        #rm rootfs/usr/sbin/fasync_user
    elif test $WIFI_TYPE = "atbm6031" ; then
        cp scripts/wifi/atbm6031.sh rootfs/usr/sbin/wifi_driver.sh
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli_6062
        rm rootfs/usr/sbin/atbm_tool_6062
        rm rootfs/usr/sbin/nimble_linux_6062
		if 	test $CHIP_SERIES = "AK3918AV200" ; then
			rm rootfs/usr/sbin/atbm_cli_6132
			rm rootfs/usr/sbin/atbm_tool_6132
			rm rootfs/usr/sbin/nimble_linux_6132
		fi
        rm rootfs/usr/sbin/atbm_cli_6031x_300w
        rm rootfs/usr/sbin/atbm_tool_6031x_300w
        rm rootfs/usr/sbin/nimble_linux_6031x_300w
        #rm rootfs/usr/sbin/custom_msg
        #rm rootfs/usr/sbin/fasync_user
    elif test $WIFI_TYPE = "atbm6032" ; then
        cp scripts/wifi/atbm6032.sh rootfs/usr/sbin/wifi_driver.sh
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli_6062
        rm rootfs/usr/sbin/atbm_tool_6062
        rm rootfs/usr/sbin/nimble_linux_6062
		if 	test $CHIP_SERIES = "AK3918AV200" ; then
			rm rootfs/usr/sbin/atbm_cli_6132
			rm rootfs/usr/sbin/atbm_tool_6132
			rm rootfs/usr/sbin/nimble_linux_6132
		fi
        rm rootfs/usr/sbin/atbm_cli_6031x_300w
        rm rootfs/usr/sbin/atbm_tool_6031x_300w
        rm rootfs/usr/sbin/nimble_linux_6031x_300w
        #rm rootfs/usr/sbin/custom_msg
        #rm rootfs/usr/sbin/fasync_user
    elif test $WIFI_TYPE = "atbm6012b" ; then
        cp scripts/wifi/atbm6032.sh rootfs/usr/sbin/wifi_driver.sh
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli_6062
        rm rootfs/usr/sbin/atbm_tool_6062
        rm rootfs/usr/sbin/nimble_linux_6062
        rm rootfs/usr/sbin/atbm_cli_6132
        rm rootfs/usr/sbin/atbm_tool_6132
        rm rootfs/usr/sbin/nimble_linux_6132
        rm rootfs/usr/sbin/atbm_cli_6031x_300w
        rm rootfs/usr/sbin/atbm_tool_6031x_300w
        rm rootfs/usr/sbin/nimble_linux_6031x_300w
        #rm rootfs/usr/sbin/custom_msg
        #rm rootfs/usr/sbin/fasync_user
    elif test $WIFI_TYPE = "atbm6132" ; then
        cp scripts/wifi/atbm6132.sh rootfs/usr/sbin/wifi_driver.sh
        cp scripts/wifi/wifi_bt_driver_6132.sh rootfs/usr/sbin/
        cp scripts/wifi/connect_ap.sh rootfs/usr/sbin/
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli
        rm rootfs/usr/sbin/atbm_tool
        rm rootfs/usr/sbin/nimble_linux
        rm rootfs/usr/sbin/atbm_cli_6062
        rm rootfs/usr/sbin/atbm_tool_6062
        rm rootfs/usr/sbin/nimble_linux_6062
        rm rootfs/usr/sbin/atbm_cli_6031x_300w
        rm rootfs/usr/sbin/atbm_tool_6031x_300w
        rm rootfs/usr/sbin/nimble_linux_6031x_300w
        mv rootfs/usr/sbin/atbm_cli_6132 rootfs/usr/sbin/atbm_cli
        mv rootfs/usr/sbin/atbm_tool_6132 rootfs/usr/sbin/atbm_tool
        mv rootfs/usr/sbin/nimble_linux_6132 rootfs/usr/sbin/nimble_linux
        #rm rootfs/usr/sbin/custom_msg
        #rm rootfs/usr/sbin/fasync_user
    elif test $WIFI_TYPE = "atbm6031x" ; then
        cp scripts/wifi/atbm6031x.sh rootfs/usr/sbin/wifi_driver.sh
        cp scripts/wifi/wifi_bt_driver_6031x.sh rootfs/usr/sbin/
        cp scripts/wifi/connect_ap.sh rootfs/usr/sbin/
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli_6062
        rm rootfs/usr/sbin/atbm_tool_6062
        rm rootfs/usr/sbin/nimble_linux_6062
        rm rootfs/usr/sbin/atbm_cli_6132
        rm rootfs/usr/sbin/atbm_tool_6132
        rm rootfs/usr/sbin/nimble_linux_6132
        #rm rootfs/usr/sbin/custom_msg
        #rm rootfs/usr/sbin/fasync_user
        if 	test $CONFIG_CHIP_TYPE = "AK3918EV300W" ; then
            echo "chip type is AK3918EV300W!"
            rm rootfs/usr/sbin/atbm_cli
            rm rootfs/usr/sbin/atbm_tool
            rm rootfs/usr/sbin/nimble_linux
            mv rootfs/usr/sbin/atbm_cli_6031x_300w rootfs/usr/sbin/atbm_cli
            mv rootfs/usr/sbin/atbm_tool_6031x_300w rootfs/usr/sbin/atbm_tool
            mv rootfs/usr/sbin/nimble_linux_6031x_300w rootfs/usr/sbin/nimble_linux
        else
        	  echo "chip type is not AK3918EV300W!"
            rm rootfs/usr/sbin/atbm_cli_6031x_300w
            rm rootfs/usr/sbin/atbm_tool_6031x_300w
            rm rootfs/usr/sbin/nimble_linux_6031x_300w
        fi
    elif test $WIFI_TYPE = "atbm6012bx" ; then
        cp scripts/wifi/atbm6012bx.sh rootfs/usr/sbin/wifi_driver.sh
        cp scripts/wifi/wifi_bt_driver_6012bx.sh rootfs/usr/sbin/
        cp scripts/wifi/connect_ap.sh rootfs/usr/sbin/
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli_6062
        rm rootfs/usr/sbin/atbm_tool_6062
        rm rootfs/usr/sbin/nimble_linux_6062
        rm rootfs/usr/sbin/atbm_cli_6132
        rm rootfs/usr/sbin/atbm_tool_6132
        rm rootfs/usr/sbin/nimble_linux_6132
        rm rootfs/usr/sbin/atbm_cli_6031x_300w
        rm rootfs/usr/sbin/atbm_tool_6031x_300w
        rm rootfs/usr/sbin/nimble_linux_6031x_300w
        #rm rootfs/usr/sbin/custom_msg
        #rm rootfs/usr/sbin/fasync_user
    elif test $WIFI_TYPE = "atbm6062u" ; then
        cp scripts/wifi/atbm6062u.sh rootfs/usr/sbin/wifi_driver.sh
        cp scripts/wifi/wifi_bt_driver_6062u.sh rootfs/usr/sbin/
        cp scripts/wifi/connect_ap.sh rootfs/usr/sbin/
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli
        rm rootfs/usr/sbin/atbm_tool
        rm rootfs/usr/sbin/nimble_linux
        rm rootfs/usr/sbin/atbm_cli_6132
        rm rootfs/usr/sbin/atbm_tool_6132
        rm rootfs/usr/sbin/nimble_linux_6132
        rm rootfs/usr/sbin/atbm_cli_6031x_300w
        rm rootfs/usr/sbin/atbm_tool_6031x_300w
        rm rootfs/usr/sbin/nimble_linux_6031x_300w
        mv rootfs/usr/sbin/atbm_cli_6062 rootfs/usr/sbin/atbm_cli
        mv rootfs/usr/sbin/atbm_tool_6062 rootfs/usr/sbin/atbm_tool
        mv rootfs/usr/sbin/nimble_linux_6062 rootfs/usr/sbin/nimble_linux
        #rm rootfs/usr/sbin/custom_msg
        #rm rootfs/usr/sbin/fasync_user
    elif test $WIFI_TYPE = "atbm6062s" ; then
        cp scripts/wifi/atbm6062s.sh rootfs/usr/sbin/wifi_driver.sh
        cp scripts/wifi/wifi_bt_driver_6062s.sh rootfs/usr/sbin/
        cp scripts/wifi/connect_ap.sh rootfs/usr/sbin/
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli
        rm rootfs/usr/sbin/atbm_tool
        rm rootfs/usr/sbin/nimble_linux
        rm rootfs/usr/sbin/atbm_cli_6132
        rm rootfs/usr/sbin/atbm_tool_6132
        rm rootfs/usr/sbin/nimble_linux_6132
        rm rootfs/usr/sbin/atbm_cli_6031x_300w
        rm rootfs/usr/sbin/atbm_tool_6031x_300w
        rm rootfs/usr/sbin/nimble_linux_6031x_300w
        mv rootfs/usr/sbin/atbm_cli_6062 rootfs/usr/sbin/atbm_cli
        mv rootfs/usr/sbin/atbm_tool_6062 rootfs/usr/sbin/atbm_tool
        mv rootfs/usr/sbin/nimble_linux_6062 rootfs/usr/sbin/nimble_linux
        #rm rootfs/usr/sbin/custom_msg
        #rm rootfs/usr/sbin/fasync_user
    elif test $WIFI_TYPE = "atbm6162" ; then
        cp scripts/wifi/atbm6162.sh rootfs/usr/sbin/wifi_driver.sh
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli_6062
        rm rootfs/usr/sbin/atbm_tool_6062
        rm rootfs/usr/sbin/nimble_linux_6062
        rm rootfs/usr/sbin/atbm_cli_6132
        rm rootfs/usr/sbin/atbm_tool_6132
        rm rootfs/usr/sbin/nimble_linux_6132
        rm rootfs/usr/sbin/atbm_cli_6031x_300w
        rm rootfs/usr/sbin/atbm_tool_6031x_300w
        rm rootfs/usr/sbin/nimble_linux_6031x_300w
        #rm rootfs/usr/sbin/custom_msg
        #rm rootfs/usr/sbin/fasync_user
	rm rootfs/usr/sbin/nimble_linux
        rm rootfs/usr/sbin/atbm_cli
        rm rootfs/usr/sbin/atbm_tool
	
    elif test $WIFI_TYPE = "ssv6x5x" ; then
        cp scripts/wifi/ssv6x5x.sh rootfs/usr/sbin/wifi_driver.sh
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli_6062
        rm rootfs/usr/sbin/atbm_tool_6062
        rm rootfs/usr/sbin/nimble_linux_6062
        rm rootfs/usr/sbin/atbm_cli_6132
        rm rootfs/usr/sbin/atbm_tool_6132
        rm rootfs/usr/sbin/nimble_linux_6132
        rm rootfs/usr/sbin/atbm_cli_6031x_300w
        rm rootfs/usr/sbin/atbm_tool_6031x_300w
        rm rootfs/usr/sbin/nimble_linux_6031x_300w
        #rm rootfs/usr/sbin/custom_msg
        #rm rootfs/usr/sbin/fasync_user
    elif test $WIFI_TYPE = "aic8800" ; then
        cp scripts/wifi/aic8800.sh rootfs/usr/sbin/wifi_driver.sh
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli_6062
        rm rootfs/usr/sbin/atbm_tool_6062
        rm rootfs/usr/sbin/nimble_linux_6062
        rm rootfs/usr/sbin/atbm_cli_6132
        rm rootfs/usr/sbin/atbm_tool_6132
        rm rootfs/usr/sbin/nimble_linux_6132
        rm rootfs/usr/sbin/atbm_cli_6031x_300w
        rm rootfs/usr/sbin/atbm_tool_6031x_300w
        rm rootfs/usr/sbin/nimble_linux_6031x_300w
    elif test $WIFI_TYPE = "aic8800d40l" ; then
        cp scripts/wifi/aic8800.sh rootfs/usr/sbin/wifi_driver.sh
        cp -rf ./wifi/* rootfs/
        rm rootfs/usr/sbin/atbm_cli_6062
        rm rootfs/usr/sbin/atbm_tool_6062
        rm rootfs/usr/sbin/nimble_linux_6062
        rm rootfs/usr/sbin/atbm_cli_6132
        rm rootfs/usr/sbin/atbm_tool_6132
        rm rootfs/usr/sbin/nimble_linux_6132
        rm rootfs/usr/sbin/atbm_cli_6031x_300w
        rm rootfs/usr/sbin/atbm_tool_6031x_300w
        rm rootfs/usr/sbin/nimble_linux_6031x_300w
    else
        echo "Wifi Type Error, please check!"
        exit 1
    fi
    
    rm -rf rootfs/usr/sbin/wpa3
    if [ -z "$CONFIG_SYS_GLASS_MODE" ] || [ "$CONFIG_SYS_GLASS_MODE" != "y" ] ; then 
        rm -rf rootfs/usr/sbin/p2p_wpa_cli
        rm -rf rootfs/usr/sbin/p2p_wpa_supplicant
        rm -rf rootfs/usr/sbin/dnsmasq
    fi
}

process_fs_scripts()
{
    cp -rf scripts/fs/* rootfs/usr/sbin/
}

process_other_scripts()
{
    cp -rf scripts/others/* rootfs/usr/sbin/
    rm -rf rootfs/usr/sbin/key_gpio.sh
}

process_utils()
{
	UTILS_SUPPORT=1

    if [ "$UTILS_SUPPORT" -eq 1 ]; then
        # 复制 utils 目录到 rootfs
        echo "Copying utils/* to rootfs/"
        cp -rf utils/* rootfs/ || { echo "Error: Failed to copy utils to rootfs"; return 1; }

		rm_nne_model
        rm_isp_conf
    else
        echo "UTILS_SUPPORT is disabled. Skipping utils processing."
    fi
}

# 快启制作dss.bin
build_dss ()
{
    if test $CHIP_SERIES = "AK3918AV130"; then
		rm -rf $dss_tools_dir/dss_config_sc3332_single.bin $dss_tools_dir/dss_config_sc2337p_dual.bin $dss_tools_dir/dss_config_gc20c3_dual.bin
		$dss_tools_dir/dss_config_tool -j $dss_tools_dir/dss_config_sc3332_single.json  $dss_tools_dir/dss_config_sc3332_single.bin 
        $dss_tools_dir/dss_config_tool -j $dss_tools_dir/dss_config_sc2337p_single.json  $dss_tools_dir/dss_config_sc2337p_single.bin
        $dss_tools_dir/dss_config_tool -j $dss_tools_dir/dss_config_gc20c3_single.json  $dss_tools_dir/dss_config_gc20c3_single.bin
		$dss_tools_dir/dss_config_tool -j $dss_tools_dir/dss_config_gc1084_single.json  $dss_tools_dir/dss_config_gc1084_single.bin 
		$dss_tools_dir/dss_config_tool -j $dss_tools_dir/dss_config_sc2337p_dual.json  $dss_tools_dir/dss_config_sc2337p_dual.bin
		$dss_tools_dir/dss_config_tool -j $dss_tools_dir/dss_config_gc20c3_dual.json  $dss_tools_dir/dss_config_gc20c3_dual.bin 
		if test "$CONFIG_FAST_SENSOR_NUM" = "1"; then
			# 单目
			if test $CONFIG_SENSOR_NAME = "sc3332"; then
				# 生成sc3332单目配置
				${uboot_tools_dir}/mkimage -A arm -T standalone -C none -O linux -a 0 -e 0 -n "dss_config_sc3332_single.bin" -d $dss_tools_dir/dss_config_sc3332_single.bin $dss_tools_dir/dss.bin
			elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
            	# 生成sc2337p单目配置
				${uboot_tools_dir}/mkimage -A arm -T standalone -C none -O linux -a 0 -e 0 -n "dss_config_sc2337p_single.bin" -d $dss_tools_dir/dss_config_sc2337p_single.bin $dss_tools_dir/dss.bin
			elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
            	# 生成gc20c3单目配置
				${uboot_tools_dir}/mkimage -A arm -T standalone -C none -O linux -a 0 -e 0 -n "dss_config_gc20c3_single.bin" -d $dss_tools_dir/dss_config_gc20c3_single.bin $dss_tools_dir/dss.bin
			elif test $CONFIG_SENSOR_NAME = "gc1084"; then
            	# 生成gc1084单目配置
				${uboot_tools_dir}/mkimage -A arm -T standalone -C none -O linux -a 0 -e 0 -n "dss_config_gc1084_single.bin" -d $dss_tools_dir/dss_config_gc1084_single.bin $dss_tools_dir/dss.bin
            fi
		elif test "$CONFIG_FAST_SENSOR_NUM" = "2"; then
			# 双目
			if test $CONFIG_SENSOR_NAME = "sc2337p"; then
				# 生成sc2337p双目配置
				${uboot_tools_dir}/mkimage -A arm -T standalone -C none -O linux -a 0 -e 0 -n "dss_config_sc2337p_dual.bin" -d $dss_tools_dir/dss_config_sc2337p_dual.bin $dss_tools_dir/dss.bin
			elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
				# 生成gc20c3双目配置
				${uboot_tools_dir}/mkimage -A arm -T standalone -C none -O linux -a 0 -e 0 -n "dss_config_gc20c3_dual.bin" -d $dss_tools_dir/dss_config_gc20c3_dual.bin $dss_tools_dir/dss.bin
			fi
		elif test "$CONFIG_FAST_SENSOR_NUM" = "3"; then
			# 三目
			echo "sensor num is 3"
		fi

		python3 $isp_ptool_dir/fix_one_isp_crc.py $dss_tools_dir/dss.bin

		mv $dss_tools_dir/dss.bin rootfs/etc/config/dss.bin
		cp -rf rootfs/etc/config/dss.bin ${image_install_dir}/dss.img
	fi
}

#安装samples
install_samples()
{
    if [ -d $sample_install_dir ];then
        cp -rf $sample_install_dir/* rootfs/usr/bin
    fi
}

# 安装各类脚本
install_scripts()
{
    process_flash_scripts
    
    if 	test $CHIP_SERIES = "AK3918AV200" ; then
		process_lcd_scripts
    fi

    process_screen_scripts

    process_sensor_scripts

    process_wifi_scripts

    process_fs_scripts

    process_other_scripts

    process_utils
	
	if test "$CONFIG_FAST_MODE" = "fast" || test "$CONFIG_FAST_MODE" = "fast_aov" || test "$CONFIG_FAST_MODE" = "fast_uvc"; then
		build_dss
	fi
}

make_isp_cfg_cpio()
{
	#删除掉就的isp.conf文件

    if 	test $CHIP_SERIES = "AK3918AV130"; then
		rm -rf $isp_ptool_dir/isp.conf $isp_ptool_dir/total_isp.conf.hdr.pthdr
        if test "$CONFIG_FAST_SENSOR_NUM" = "1"; then
            if test $CONFIG_SENSOR_NAME = "sc2337p"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc2337p_mipi_2lane_av130.conf  ${isp_ptool_dir}/isp.conf
            elif test $CONFIG_SENSOR_NAME = "sc3332"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc3332_mipi_2lane_av130.conf  ${isp_ptool_dir}/isp.conf
            elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc20c3_mipi_2lane_av130.conf  ${isp_ptool_dir}/isp.conf
            elif test $CONFIG_SENSOR_NAME = "gc1084"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc1084_mipi_2lane_av130.conf  ${isp_ptool_dir}/isp.conf
            fi
        elif test "$CONFIG_FAST_SENSOR_NUM" = "2"; then
            if test $CONFIG_SENSOR_NAME = "sc2337p"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc2337p_mipi_2lane_av130_dual.conf  ${isp_ptool_dir}/isp.conf
            elif test $CONFIG_SENSOR_NAME = "sc3332"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc3332_mipi_2lane_av130.conf  ${isp_ptool__dir}/isp.conf
            elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc20c3_mipi_2lane_av130_dual.conf  ${isp_ptool_dir}/isp.conf
            fi
        fi
		#因为mk_isp_part.sh需要在工作目录才能正确引用其目录内的py脚本，因此需要进入目录操作
		cd $isp_ptool_dir
        ./mk_isp_part.sh none $isp_ptool_dir/isp.conf
        cp -rf $isp_ptool_dir/total_isp.conf.hdr.pthdr ${CONFIG_IMAGE_OUTPUT_DIR}/isp.part
		cd -
    else
        #制作单目ispcfg.cpio
        if test "$CONFIG_FAST_SENSOR_NUM" = "1"; then
            mkdir isp_cfg

            if test $CONFIG_SENSOR_NAME = "sc2336"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc2336_mipi_2lane_km01a.conf ./isp_cfg
            elif test $CONFIG_SENSOR_NAME = "sc2336p"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc2336p_mipi_2lane_km01a.conf ./isp_cfg
            elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
                #cp -rf $rootfs_dir/utils/etc/isp_sc2337p_mipi_2lane_km01a.conf ./isp_cfg
                cp -rf $rootfs_dir/utils/etc/isp_sc2337p_mipi_2lane_av130.conf ./isp_cfg
            elif test $CONFIG_SENSOR_NAME = "sc3332"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc3332_mipi_2lane_av130.conf ./isp_cfg
            elif test $CONFIG_SENSOR_NAME = "gc2053"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc2053_mipi_2lane_km01a.conf ./isp_cfg
            elif test $CONFIG_SENSOR_NAME = "gc4653"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc4653_mipi_2lane_km01a.conf ./isp_cfg
            elif test $CONFIG_SENSOR_NAME = "sc200ai"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc200ai_mipi_2lane_km01a.conf ./isp_cfg
            elif test $CONFIG_SENSOR_NAME = "cv2005"; then
                cp -rf $rootfs_dir/utils/etc/isp_cv2005_mipi_2lane_km01a.conf ./isp_cfg
            elif test $CONFIG_SENSOR_NAME = "sc431ai"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc431hai_mipi_2lane_km01a.conf ./isp_cfg
            elif test $CONFIG_SENSOR_NAME = "gc3003"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc3003_mipi_2lane_av130_hp.conf ./isp_cfg
            elif test $CONFIG_SENSOR_NAME = "sc3332"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc3332_mipi_2lane_av130.conf ./isp_cfg
            elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc20c3_mipi_2lane_av130.conf ./isp_cfg
            elif test $CONFIG_SENSOR_NAME = "gc1084"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc1084_mipi_2lane_av130.conf ./isp_cfg
            fi

            #构建ispcfg.cpio.lz4压缩文件
			cd isp_cfg
			find . | cpio -o -H newc > ../ispcfg.cpio
			cd ..
            lz4c -l -c0 -f ispcfg.cpio ispcfg.cpio.lz4
        elif test "$CONFIG_FAST_SENSOR_NUM" = "2"; then
            #制作双目ispcfg_dual.cpio
            mkdir ispcfg_dual

            if test $CONFIG_SENSOR_NAME = "sc2336"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc2336_mipi_2lane_km01a_dual.conf ./ispcfg_dual
            elif test $CONFIG_SENSOR_NAME = "gc2053"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc2053_mipi_2lane_km01a_dual.conf ./ispcfg_dual
            elif test $CONFIG_SENSOR_NAME = "gc4653"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc4653_mipi_2lane_km01a_dual.conf ./ispcfg_dual
            elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc2337p_mipi_2lane_av130_dual.conf ./ispcfg_dual
            elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc20c3_mipi_2lane_av130_dual.conf ./ispcfg_dual
            fi

            #构建ispcfg_dual.cpio.lz4压缩文件
			cd ispcfg_dual
            find . | cpio -o -H newc > ../ispcfg_dual.cpio
			cd ..
            lz4c -l -c0 -f ispcfg_dual.cpio ispcfg_dual.cpio.lz4
        elif test "$CONFIG_FAST_SENSOR_NUM" = "3"; then
            #制作双目ispcfg_triple.cpio
            mkdir ispcfg_triple

            if test $CONFIG_SENSOR_NAME = "sc2336"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc2336_mipi_2lane_km01a_dual.conf ./ispcfg_triple
            elif test $CONFIG_SENSOR_NAME = "gc2053"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc2053_mipi_2lane_km01a_dual.conf ./ispcfg_triple
            elif test $CONFIG_SENSOR_NAME = "gc4653"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc4653_mipi_2lane_km01a_dual.conf ./ispcfg_triple
            elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
                cp -rf $rootfs_dir/utils/etc/isp_sc2337p_mipi_2lane_km01a_three.conf ./ispcfg_triple
            elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
                cp -rf $rootfs_dir/utils/etc/isp_gc20c3_mipi_2lane_av130_dual.conf ./ispcfg_dual
            fi

            #构建ispcfg_triple.cpio.lz4压缩文件
			cd ispcfg_triple
            find . | cpio -o -H newc > ispcfg_triple.cpio
			cd ..
            lz4c -l -c0 -f ispcfg_triple.cpio ispcfg_triple.cpio.lz4
        fi
    fi

}

prepare_initfs()
{
	#拷贝initfs的必要文件
	echo "prepare initfs "
	rm -rf initfs/lib/*
	
	cp -rf $driver_install_dir/external/ak_i2c.ko initfs/lib/
	cp -rf $driver_install_dir/external/ak_ion.ko initfs/lib/

    if test $CHIP_SERIES = "KM01A"; then
		cp -rf $driver_install_dir/external/ak_vi.ko initfs/lib/
		cp -rf $driver_install_dir/external/ak_vicap.ko initfs/lib/
		cp -rf $driver_install_dir/external/ak_pp.ko initfs/lib/
	fi

	cp -rf $driver_install_dir/external/ak_isp.ko initfs/lib/
	cp -rf $driver_install_dir/external/ak_saradc.ko initfs/lib/
	cp -rf $driver_install_dir/external/ak_ircut.ko initfs/lib/
	cp -rf $driver_install_dir/external/ak_npu.ko initfs/lib/
	cp -rf $driver_install_dir/external/ak_venc_adapter.ko initfs/lib/
	cp -rf $driver_install_dir/external/ak_venc_bridge.ko initfs/lib/
	cp -rf $driver_install_dir/external/ak_quick_start.ko initfs/lib/
	
	#根据config.mk设定的CIS型号，拷贝对应的CIS驱动到initfs/lib分区
	if test $CONFIG_SENSOR_NAME = "sc2336"; then
		cp -rf $driver_install_dir/external/sensor_sc2336.ko initfs/lib/
	elif test $CONFIG_SENSOR_NAME = "sc2336p"; then
		cp -rf $driver_install_dir/external/sensor_sc2336p.ko initfs/lib/
	elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
		cp -rf $driver_install_dir/external/sensor_sc2337p.ko initfs/lib/
	elif test $CONFIG_SENSOR_NAME = "gc2053"; then
		cp -rf $driver_install_dir/external/sensor_gc2053.ko initfs/lib/
	elif test $CONFIG_SENSOR_NAME = "gc4653"; then
		cp -rf $driver_install_dir/external/sensor_gc4653.ko initfs/lib/
	elif test $CONFIG_SENSOR_NAME = "sc200ai"; then
		cp -rf $driver_install_dir/external/sensor_sc200ai.ko initfs/lib/
	elif test $CONFIG_SENSOR_NAME = "cv2005"; then
		cp -rf $driver_install_dir/external/sensor_cv2005.ko initfs/lib/
	elif test $CONFIG_SENSOR_NAME = "sc431ai"; then
		cp -rf $driver_install_dir/external/sensor_sc431ai.ko initfs/lib/
	elif test $CONFIG_SENSOR_NAME = "gc3003"; then
		cp -rf $driver_install_dir/external/sensor_gc3003.ko initfs/lib/
	elif test $CONFIG_SENSOR_NAME = "sc3332"; then
		cp -rf $driver_install_dir/external/sensor_sc3332.ko initfs/lib/
	elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
		cp -rf $driver_install_dir/external/sensor_gc20c3.ko initfs/lib/
	elif test $CONFIG_SENSOR_NAME = "sc2331x"; then
		cp -rf $driver_install_dir/external/sensor_sc2331x.ko initfs/lib/
	elif test $CONFIG_SENSOR_NAME = "gc1084"; then
		cp -rf $driver_install_dir/external/sensor_gc1084.ko initfs/lib/
	fi

}

prepare_ramfs4()
{
	echo "prepare  ramfs4"
	#拷贝第二阶段应用程序到rootfs/bin
	#cp -rf ak_fast_main rootfs/bin/
}

prepare_config()
{
	echo "prepare  config"
	#制作ispcfg.cpio
	make_isp_cfg_cpio
	
}

prepare_image()
{
    extract_fs
    install_platform

    install_driver

    install_scripts

    install_samples
	
	#安装快启需要的一些文件
	if test "$CONFIG_FAST_MODE" = "fast" || test "$CONFIG_FAST_MODE" = "fast_aov" || test "$CONFIG_FAST_MODE" = "fast_uvc"; then
		#准备initfs所需文件
		prepare_initfs   
		
		#准备ramfs4所需文件
		prepare_ramfs4
		
		#准备/etc/config所需文件
		prepare_config
	fi
	
}

make_ramfs4()
{
	#删除已有的ramfs4
	rm -rf ramfs4.cpio ramfs4.cpio.lz4 ramfs
	
	mkdir -p ramfs
	cp -af rootfs/* ramfs
	
	#删除ramfs里不必要的文件 
	rm -rf ramfs/usr/lib/*
	
	
	rm -rf ramfs/usr/sbin/model*
	rm -rf ramfs/usr/sbin/host*
	rm -rf ramfs/usr/sbin/ubi*
	rm -rf ramfs/usr/modules/fb*.ko 
	rm -rf ramfs/usr/modules/sensor*.ko
	rm -rf ramfs/usr/sbin/wpa_supplicant ramfs/usr/sbin/wpa_cli ramfs/usr/sbin/iw*
	rm -rf ramfs/usr/modules/ak_venc_adapter.ko ramfs/usr/modules/ak_venc_bridge.ko ramfs/usr/modules/rtl8188ftv.ko
	rm -rf ramfs/usr/bin/*sample
	rm -rf ramfs/etc/config/*
	rm -rf ramfs/etc/isp*
	rm -rf ramfs/data ramfs/dev ramfs/lib ramfs/mnt ramfs/proc ramfs/sys ramfs/tmp ramfs/var

	rm -rf ramfs/usr/modules/*.ko 

    if [ "$RTC_SAMPLE_TEST" = "false" ]; then
        cp rootfs/usr/bin/ak_fast_aov_sample ramfs/usr/bin/
    fi
	
	#cp utils/usr/sbin/model_0x0A000000_V1.1.0.bin ramfs/usr/sbin/
    
	mkdir -p ramfs/lib/modules/4.4.302-cip94/kernel/net/wireless/
	mkdir -p ramfs/lib/modules/4.4.302-cip94/kernel/net/sunrpc/
	mkdir -p ramfs/lib/modules/4.4.302-cip94/kernel/fs/nfs/
	mkdir -p ramfs/lib/modules/4.4.302-cip94/kernel/fs/lockd/
	mkdir -p ramfs/lib/modules/4.4.302-cip94/kernel/fs/nfs_common/
	mkdir -p ramfs/lib/modules/4.4.302-cip94/kernel/drivers/mmc/core/
	mkdir -p ramfs/lib/modules/4.4.302-cip94/kernel/drivers/mmc/card/
	
	
	if test $CONFIG_FAST_DEBUG = "1"; then
		#拷贝C库，支持mtd_debug gdb等
		mkdir -p ./debug/libc/
		cp resource/lib/ld-uClibc.so.0 debug/libc/
		cp resource/lib/libc.so.0      debug/libc/
		cp resource/lib/libiconv.so.2  debug/libc/
		cp resource/lib/libstdc++.so.6 debug/libc/
		cp resource/lib/libgcc_s.so.1  debug/libc/
		
		#支持usb转以太网
		mkdir -p ./debug/net_ko/
		cp $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/drivers/usb/common/usb-common.ko debug/net_ko/
		cp $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/drivers/usb/core/usbcore.ko      debug/net_ko/
		cp $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/drivers/net/usb/usbnet.ko        debug/net_ko/
		cp $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/drivers/net/usb/cdc_ether.ko     debug/net_ko/
		cp $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/drivers/net/mii.ko               debug/net_ko/
		cp $driver_install_dir/external/ak_hcd.ko                                                         debug/net_ko/
		cp $rootfs_dir/scripts/others/usb_net.sh                                                          debug/net_ko/
	fi
	
	
	#拷贝必要的ko
	cp -rf $driver_install_dir/external/ak_efuse.ko ramfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_pwm_char.ko ramfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_saradc.ko ramfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_gpio_keys.ko ramfs/usr/modules/
	#cp -rf $driver_install_dir/external/ak_npu.ko ramfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_uio.ko ramfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_rtc.ko ramfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_leds.ko ramfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_pcm.ko ramfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_mci.ko ramfs/usr/modules/
	cp -rf $driver_install_dir/external/aic8800_netdrv.ko ramfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_eth.ko ramfs/usr/modules/
	cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/modules* ramfs/lib/modules/4.4.302-cip94/
	cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/net/sunrpc/sunrpc.ko ramfs/lib/modules/4.4.302-cip94/kernel/net/sunrpc/
	cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/fs/nfs/* ramfs/lib/modules/4.4.302-cip94/kernel/fs/nfs/
	cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/fs/lockd/* ramfs/lib/modules/4.4.302-cip94/kernel/fs/lockd/
	cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/fs/nfs_common/* ramfs/lib/modules/4.4.302-cip94/kernel/fs/nfs_common/
	cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/drivers/mmc/core/mmc_core.ko ramfs/lib/modules/4.4.302-cip94/kernel/drivers/mmc/core/
	cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/drivers/mmc/card/mmc_block.ko ramfs/lib/modules/4.4.302-cip94/kernel/drivers/mmc/card/
	cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/net/wireless/cfg80211.ko ramfs/lib/modules/4.4.302-cip94/kernel/net/wireless/
	
	cp -rf $rootfs_dir/wifi/usr/sbin/custom_msg ramfs/usr/sbin/
    if [ "$RTC_SAMPLE_TEST" = "true" ]; then
        cp -rf $driver_install_dir/external/sample/ak_drv_rtc_sample/ak_drv_rtc_sample ramfs/usr/sbin/
    fi
    
    #uvc需要的ko
    if test $CONFIG_FAST_MODE = "fast_uvc"; then
        cp -rf $driver_install_dir/external/ak_udc.ko ramfs/usr/modules/
        cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/fs/configfs ramfs/lib/modules/4.4.302-cip94/kernel/fs/
        mkdir ramfs/lib/modules/4.4.302-cip94/kernel/drivers/
        cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/drivers/usb ramfs/lib/modules/4.4.302-cip94/kernel/drivers
        cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/drivers/media ramfs/lib/modules/4.4.302-cip94/kernel/drivers
        rm -rf ramfs/lib/modules/4.4.302-cip94/kernel/drivers/usb/storage
        rm -rf ramfs/lib/modules/4.4.302-cip94/kernel/drivers/usb/serial
        rm -rf ramfs/lib/modules/4.4.302-cip94/kernel/drivers/usb/mon
        rm -rf ramfs/lib/modules/4.4.302-cip94/kernel/drivers/usb/class
    fi

	#构建ramfs4文件系统
	if test ! -f ramfs4.cpio.lz4; then
		if test ! -f ramfs4.cpio; then
			cd ramfs
			find . | cpio -o -H newc > ../ramfs4.cpio
			cd ..
		fi
	lz4c -l -c0 -f ramfs4.cpio ramfs4.cpio.lz4
	fi
	
	#rm -rf ramfs
}

gen_ko_bin_sig()
{
	#ko装载顺序
	echo '[ak_i2c.ko]&' >> len.bin
	echo '0' >> len.bin

	echo '[ak_ion.ko]&' >> len.bin
	echo '0' >> len.bin

	#echo '[ak_vi.ko]&' >> len.bin
	#echo '0' >> len.bin
	
	#echo '[ak_vicap.ko]&' >> len.bin
	#echo '0' >> len.bin
	
	echo '[ak_isp.ko]&' >> len.bin
	echo '0' >> len.bin
	
	#echo '[ak_pp.ko]&' >> len.bin
	#echo '0' >> len.bin

	if test $CONFIG_SENSOR_NAME = "sc2336"; then
		echo '[sensor_sc2336.ko]&' >> len.bin
		echo '0' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "sc2336p"; then
		echo '[sensor_sc2336p.ko]&' >> len.bin
		echo '0' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
		echo '[sensor_sc2337p.ko]&' >> len.bin
		echo '0' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "sc200ai"; then
		echo '[sensor_sc200ai.ko]&' >> len.bin
		echo '0' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "cv2005"; then
		echo '[sensor_cv2005.ko SENSOR_I2C_ADDR=0x36]&' >> len.bin
		#echo '[sensor_cv2005.ko ]&' >> len.bin
		echo '0' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "sc431ai"; then
		echo '[sensor_sc431ai.ko]&' >> len.bin
		echo '0' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "gc3003"; then
		echo '[sensor_gc3003.ko]&' >> len.bin
		echo '0' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "sc3332"; then
		echo '[sensor_sc3332.ko]&' >> len.bin
		echo '0' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
		echo '[sensor_gc20c3.ko]&' >> len.bin
		echo '0' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "gc1084"; then
		echo '[sensor_gc1084.ko]&' >> len.bin
		echo '0' >> len.bin
	fi

	echo '[ak_saradc.ko]&' >> len.bin
	echo '0' >> len.bin
	
	#echo '[ak_ircut.ko]&' >> len.bin
	#echo '0' >> len.bin

	echo '[ak_venc_adapter.ko UseExtStreamBuf=1]&' >> len.bin
	echo '0' >> len.bin

	echo '[ak_venc_bridge.ko]&' >> len.bin
	echo '0' >> len.bin

	echo '[ak_npu.ko]&' >> len.bin
    echo '0' >> len.bin

	echo '[ak_quick_start.ko]&' >> len.bin
	echo '0' >> len.bin
}

#制作len.bin 单目
make_rootfs_ram_sig()
{
	##head info file list Start mark
	rm -f len.bin
	echo 'S' > len.bin
	##head initfs size
	#wc -c initfs.cpio | awk '{print $1}' >> len.bin

	#isp.conf
	# echo '[isp.conf]' >> len.bin
	# if test $CONFIG_SENSOR_NAME = "sc200ai"; then
	# 	wc -c $rootfs_dir/utils/etc/isp_sc200ai_mipi_2lane_km01a.conf | awk '{print $1}' >> len.bin
	# elif test $CONFIG_SENSOR_NAME = "sc2336"; then
	# 	wc -c $rootfs_dir/utils/etc/isp_sc2336_mipi_2lane_km01a.conf | awk '{print $1}' >> len.bin
	# elif test $CONFIG_SENSOR_NAME = "sc2336p"; then
	# 	wc -c $rootfs_dir/utils/etc/isp_sc2336p_mipi_2lane_km01a.conf | awk '{print $1}' >> len.bin
	# elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
	# 	#wc -c $rootfs_dir/utils/etc/isp_sc2337p_mipi_2lane_km01a.conf | awk '{print $1}' >> len.bin
	# 	wc -c $rootfs_dir/utils/etc/isp_sc2337p_mipi_2lane_av130.conf | awk '{print $1}' >> len.bin
	# elif test $CONFIG_SENSOR_NAME = "cv2005"; then
	# 	wc -c $rootfs_dir/utils/etc/isp_cv2005_mipi_2lane_km01a.conf | awk '{print $1}' >> len.bin
	# elif test $CONFIG_SENSOR_NAME = "sc431ai"; then
	# 	wc -c $rootfs_dir/utils/etc/isp_sc431hai_mipi_2lane_km01a.conf | awk '{print $1}' >> len.bin
	# elif test $CONFIG_SENSOR_NAME = "gc3003"; then
	# 	wc -c $rootfs_dir/utils/etc/isp_gc3003_mipi_2lane_av130_hp.conf | awk '{print $1}' >> len.bin
	# elif test $CONFIG_SENSOR_NAME = "sc3332"; then
	# 	wc -c $rootfs_dir/utils/etc/isp_sc3332_mipi_2lane_av130.conf | awk '{print $1}' >> len.bin
	# elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
	# 	wc -c $rootfs_dir/utils/etc/isp_gc20c3_mipi_2lane_av130.conf | awk '{print $1}' >> len.bin
	# fi
    
    if [ "$RTC_SAMPLE_TEST" = "false" ]; then
        gen_ko_bin_sig
    fi

	##rootfs file package
	echo '[ramfs4.cpio.lz4]#' >> len.bin
	#check info TAG
	echo '?' >> len.bin
	#check file info,option
	head -c 4 ramfs4.cpio.lz4 >> len.bin
	wc -c ramfs4.cpio.lz4 | awk '{print $1}' >> len.bin


	##ispconf backup cpio
	# echo '[ispcfg.cpio.lz4]!' >> len.bin
	# wc -c ispcfg.cpio.lz4 | awk '{print $1}' >> len.bin 

	##file list end mark
	echo 'E' >> len.bin
	##fast-run prog param
	echo 'v' >> len.bin

	echo -e -n "\x05\x02\x02\x04\x00\x1e\x00\x01" >> len.bin
	echo 'e' >> len.bin

	##file data add begin
	cat len.bin > rootfs.ram

	# if test $CONFIG_SENSOR_NAME = "sc200ai"; then
	# 	cat $rootfs_dir/utils/etc/isp_sc200ai_mipi_2lane_km01a.conf >> rootfs.ram
	# elif test $CONFIG_SENSOR_NAME = "sc2336"; then
	# 	cat $rootfs_dir/utils/etc/isp_sc2336_mipi_2lane_km01a.conf >> rootfs.ram
	# elif test $CONFIG_SENSOR_NAME = "sc2336p"; then
	# 	cat $rootfs_dir/utils/etc/isp_sc2336p_mipi_2lane_km01a.conf >> rootfs.ram
	# elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
	# 	cat $rootfs_dir/utils/etc/isp_sc2337p_mipi_2lane_av130.conf >> rootfs.ram
	# elif test $CONFIG_SENSOR_NAME = "cv2005"; then
	# 	cat $rootfs_dir/utils/etc/isp_cv2005_mipi_2lane_km01a.conf >> rootfs.ram
	# elif test $CONFIG_SENSOR_NAME = "sc431ai"; then
	# 	cat $rootfs_dir/utils/etc/isp_sc431hai_mipi_2lane_km01a.conf >> rootfs.ram
	# elif test $CONFIG_SENSOR_NAME = "gc3003"; then
	# 	cat $rootfs_dir/utils/etc/isp_gc3003_mipi_2lane_av130_hp.conf >> rootfs.ram
	# elif test $CONFIG_SENSOR_NAME = "sc3332"; then
	# 	cat $rootfs_dir/utils/etc/isp_sc3332_mipi_2lane_av130.conf >> rootfs.ram
	# elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
	# 	cat $rootfs_dir/utils/etc/isp_gc20c3_mipi_2lane_av130.conf >> rootfs.ram
	# fi
	

	cat ramfs4.cpio.lz4 >> rootfs.ram

	#cat ispcfg.cpio.lz4 >> rootfs.ram


	rm -rf ./isp_cfg

	ls -tl ./rootfs.ram
	cp rootfs.ram $image_install_dir/

}

gen_ko_bin_dual()
{
	#ko装载顺序
	echo '[ak_i2c.ko]&' >> len.bin
	echo '0' >> len.bin

	echo '[ak_ion.ko]&' >> len.bin
	echo '0' >> len.bin

	#echo '[ak_vi.ko mp_logic_open_num=2]&' >> len.bin
	#echo '0' >> len.bin
	#
	#echo '[ak_vicap.ko]&' >> len.bin
	#echo '0' >> len.bin
	
	echo '[ak_isp.ko]&' >> len.bin
	echo '0' >> len.bin
	
	#echo '[ak_pp.ko]&' >> len.bin
	#echo '0' >> len.bin

	if test $CONFIG_SENSOR_NAME = "sc2336"; then
		echo '[sensor_sc2336.ko addr0=0x32 addr1=0x30]&' >> len.bin
		echo '0' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
		echo '[sensor_sc2337p.ko addr0=0x30 addr1=0x32]&' >> len.bin
		echo '0' >> len.bin
    elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
		echo '[sensor_gc20c3.ko addr0=0x31 addr1=0x10]&' >> len.bin
		echo '0' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "gc2053"; then
		echo '[sensor_gc2053.ko]&' >> len.bin
		echo '0' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "gc4653"; then
		echo '[sensor_gc4653.ko]&' >> len.bin
		echo '0' >> len.bin
	fi

	echo '[ak_saradc.ko]&' >> len.bin
	echo '0' >> len.bin

	#echo '[ak_ircut.ko]&' >> len.bin
	#echo '0' >> len.bin
	
	echo '[ak_venc_adapter.ko UseExtStreamBuf=1]&' >> len.bin
	echo '0' >> len.bin

	echo '[ak_venc_bridge.ko]&' >> len.bin
	echo '0' >> len.bin

	echo '[ak_npu.ko]&' >> len.bin
    echo '0' >> len.bin

	echo '[ak_quick_start.ko]&' >> len.bin
	echo '0' >> len.bin

}

gen_ko_bin_triple()
{
	#ko装载顺序
	echo '[ak_i2c.ko]&' >> len.bin
	echo '0' >> len.bin

	echo '[ak_ion.ko]&' >> len.bin
	echo '0' >> len.bin

	echo '[ak_vi.ko mp_logic_open_num=3]&' >> len.bin
	echo '0' >> len.bin
	
	echo '[ak_vicap.ko]&' >> len.bin
	echo '0' >> len.bin
	
	echo '[ak_isp.ko]&' >> len.bin
	echo '0' >> len.bin
	
	echo '[ak_pp.ko]&' >> len.bin
	echo '0' >> len.bin

	if test $CONFIG_SENSOR_NAME = "sc2336"; then
		echo '[sensor_sc2336.ko addr0=0x32 addr1=0x30]&' >> len.bin
		echo '0' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
		echo '[sensor_sc2337p.ko  addr0=0x30 addr1=0x32 addr2=0x30 MIPI_LANES_SWAP=0x31623 common_power=1]&' >> len.bin
		echo '0' >> len.bin
	fi

	echo '[ak_saradc.ko]&' >> len.bin
	echo '0' >> len.bin

	echo '[ak_ircut.ko]&' >> len.bin
	echo '0' >> len.bin
	
	echo '[ak_venc_adapter.ko UseExtStreamBuf=1]&' >> len.bin
	echo '0' >> len.bin

	echo '[ak_venc_bridge.ko]&' >> len.bin
	echo '0' >> len.bin

	echo '[ak_npu.ko]&' >> len.bin
    echo '0' >> len.bin

	echo '[ak_quick_start.ko]&' >> len.bin
	echo '0' >> len.bin

}

#制作len.bin 双目
make_rootfs_ram_dual()
{
	##head info file list Start mark
	rm -f len.bin
	echo 'S' > len.bin
	##head initfs size
	#wc -c initfs.cpio | awk '{print $1}' >> len.bin
	##dual_mode config
	echo 'dual:1' >> len.bin
	##############################

	# echo '[isp.conf]' >> len.bin
	# if test $CONFIG_SENSOR_NAME = "sc2336"; then
	# 	wc -c $rootfs_dir/utils/etc/isp_sc2336_mipi_2lane_km01a_dual.conf | awk '{print $1}' >> len.bin
	# elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
	# 	wc -c $rootfs_dir/utils/etc/isp_sc2337p_mipi_2lane_av130_dual.conf | awk '{print $1}' >> len.bin
	# fi

    if [ "$RTC_SAMPLE_TEST" = "false" ]; then
        gen_ko_bin_dual
	fi

	##rootfs file package
	echo '[ramfs4.cpio.lz4]#' >> len.bin
	#check info TAG
	echo '?' >> len.bin
	#check file info,option
	head -c 4 ramfs4.cpio.lz4 >> len.bin
	wc -c ramfs4.cpio.lz4 | awk '{print $1}' >> len.bin

	##support usr partition cpio
	#echo '[usr.cpio.lz4]@' >> len.bin
	#wc -c usr.cpio.lz4 | awk '{print $1}' >> len.bin 

	##ispconf backup cpio
	# echo '[ispcfg_dual.cpio.lz4]!' >> len.bin
	# wc -c ispcfg_dual.cpio.lz4 | awk '{print $1}' >> len.bin 

	##file list end mark
	echo 'E' >> len.bin
	##fast-run prog param
	echo 'v' >> len.bin
	# main_res, frame_fmt,  venc_mode,  thd[en],sub_res, venc_fps, drop_frame, fast_ae_enable
	# main[2~8] h265[0~3] , mode[1 or 2], 0/1, sub[0~2], fps[0~30], drop[0~30], fast_ae[0,1]
	#    720p yuv
	#echo -e -n "\x02\x03\x01\x01" >> len.bin
	#echo -e -n "\x01\x03\x01\x01" >> len.bin
	#     1080p, jpg, kernel_frame , drop 1, sub:720p, 30fps
	#echo -e -n "\x04\x01\x01\x01\x01\x2\x1e" >> len.bin
	#     main[2~8] h265[0~3] , mode[1 or 2], thd[en], sub[0~2], fps[0~30], drop[0~30], fast_ae[0,1]
	## run param fill in file 
	#echo "-Z /mnt/anyka_fast_app_run_param.ini &" >> len.bin
	#echo -e -n "\x05\x02\x02\x04\x00\x1e\x00\x01" >> len.bin
	echo -e -n "\x04\x02\x02\x02\x00\x0f\x00\x01" >> len.bin
	#echo -e -n "\x04\x02\x02\x02" >> len.bin
	#     1296p h265
	#echo -e -n "\x05\x02\x02\x04" >> len.bin
	##tail mark
	echo 'e' >> len.bin

	##file data add begin
	cat len.bin  > rootfs.ram


	# if test $CONFIG_SENSOR_NAME = "sc2336"; then
	# 	cat $rootfs_dir/utils/etc/isp_sc2336_mipi_2lane_km01a_dual.conf >> rootfs.ram
	# elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
	# 	cat $rootfs_dir/utils/etc/isp_sc2337p_mipi_2lane_av130_dual.conf >> rootfs.ram
	# fi


	cat ramfs4.cpio.lz4 >> rootfs.ram

	#cat ispcfg_dual.cpio.lz4 >> rootfs.ram

	##APP1 config file
	#md5sum fast_config.ini.dual | awk '{print $1}'   >> config.fs
	#cat fast_config.ini.dual                         >> config.fs
	rm -rf ./ispcfg_dual

	ls -tl ./rootfs.ram
	cp rootfs.ram $image_install_dir/
	#cp config.fs ../tools/burntool/config_d.fs

	#mkdir -p ./rootfs/etc/jffs2
	#cp fast_config.ini.dual ./rootfs/etc/jffs2/fast_config.ini
	#cp fast_config.ini.dual ./rootfs/etc/config/fast_config.ini
	
}

#制作len.bin 三目
make_rootfs_ram_triple()
{
	##head info file list Start mark
	rm -f len.bin
	echo 'S' > len.bin
	##head initfs size
	#wc -c initfs.cpio | awk '{print $1}' >> len.bin
	##dual_mode config
	echo 'dual:1' >> len.bin
	##############################

	echo '[isp.conf]' >> len.bin
	if test $CONFIG_SENSOR_NAME = "sc2336"; then
		wc -c $rootfs_dir/utils/etc/isp_sc2336_mipi_2lane_km01a_dual.conf | awk '{print $1}' >> len.bin
	elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
		wc -c $rootfs_dir/utils/etc/isp_sc2337p_mipi_2lane_km01a_triple.conf | awk '{print $1}' >> len.bin
	fi

    if [ "$RTC_SAMPLE_TEST" = "false" ]; then
        gen_ko_bin_triple
	fi

	##rootfs file package
	echo '[ramfs4.cpio.lz4]#' >> len.bin
	#check info TAG
	echo '?' >> len.bin
	#check file info,option
	head -c 4 ramfs4.cpio.lz4 >> len.bin
	wc -c ramfs4.cpio.lz4 | awk '{print $1}' >> len.bin


	##ispconf backup cpio
	echo '[ispcfg_triple.cpio.lz4]!' >> len.bin
	wc -c ispcfg_triple.cpio.lz4 | awk '{print $1}' >> len.bin 

	##file list end mark
	echo 'E' >> len.bin
	##fast-run prog param
	echo 'v' >> len.bin
	echo -e -n "\x04\x02\x02\x02\x00\x0f\x00\x01" >> len.bin

	##tail mark
	echo 'e' >> len.bin

	##file data add begin
	cat len.bin  > rootfs.ram


	if test $CONFIG_SENSOR_NAME = "sc2336"; then
		cat $rootfs_dir/utils/etc/isp_sc2336_mipi_2lane_km01a_dual.conf >> rootfs.ram
	elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
		cat $rootfs_dir/utils/etc/isp_sc2337p_mipi_2lane_km01a_triple.conf >> rootfs.ram
	fi


	cat ramfs4.cpio.lz4 >> rootfs.ram

	cat ispcfg_triple.cpio.lz4 >> rootfs.ram

	rm -rf ./ispcfg_triple

	ls -tl ./rootfs.ram
	cp rootfs.ram $image_install_dir/

}

make_rootfs_ram()
{
	#删除上一次的rootfs.ram
	if [ -f "rootfs.ram" ]; then
		rm -rf rootfs.ram
	fi
	#删除上一次的len.bin
	if [ -f "len.bin" ]; then
		rm -rf len.bin
	fi

	if [ "$1" = sig ]; then
		echo "make sig sensor file system"
		make_rootfs_ram_sig
	elif [ "$1" = dual ]; then
		echo "make dual sensor file system"
		make_rootfs_ram_dual
	elif [ "$1" = triple ]; then
		echo "make triple sensor file system"
		make_rootfs_ram_triple
	fi
}

make_initfs()
{
	cd ./initfs
	find . | cpio -o -H newc > ../initfs.cpio
	cd -

    # initfs.cpio 添加为lz4压缩
    #find . | cpio -o -H newc > ../initfs_raw.cpio
	#lz4c -l -c0 -f ../initfs_raw.cpio initfs.cpio

    ${uboot_tools_dir}/mkimage -A arm -T ramdisk -C none -O linux -a 0 -e 0 -n "initfs" -d ./initfs.cpio ./initfs.img
    cp -rf ./initfs.img $image_install_dir/
}

make_fast4()
{
	#制作ramfs文件系统
	make_ramfs4

	#制作len.bin文件, 生成rootfs.ram文件系统
	if test $CONFIG_FAST_SENSOR_NUM = "1"; then
		make_rootfs_ram "sig"
	elif test $CONFIG_FAST_SENSOR_NUM = "2"; then
		make_rootfs_ram "dual"
	elif test $CONFIG_FAST_SENSOR_NUM = "3"; then
		make_rootfs_ram "triple"
	fi
	
}

make_romfs()
{
	#删除已有的romfs4
	rm -rf romfs root_fast.sqsh4

	mkdir -p romfs
	cp -af rootfs/* romfs

	mkdir romfs/init
	cp -af initfs/lib/* romfs/init

	if test $CONFIG_SENSOR_NAME = "sc200ai"; then
		cp utils/etc/isp_sc200ai_mipi_2lane_km01a.conf romfs/init/isp.conf
	elif test $CONFIG_SENSOR_NAME = "sc2337p"; then
		if test $CONFIG_FAST_SENSOR_NUM = "1"; then
			#cp utils/etc/isp_sc2337p_mipi_2lane_km01a.conf romfs/init/isp.conf
            cp utils/etc/isp_sc2337p_mipi_2lane_av130.conf romfs/init/isp.conf
		elif test $CONFIG_FAST_SENSOR_NUM = "2"; then
			cp utils/etc/isp_sc2337p_mipi_2lane_av130_dual.conf romfs/init/isp.conf
		elif test $CONFIG_FAST_SENSOR_NUM = "3"; then
			cp utils/etc/isp_sc2337p_mipi_2lane_km01a_triple.conf romfs/init/isp.conf
		fi
    elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
		if test $CONFIG_FAST_SENSOR_NUM = "1"; then
            cp utils/etc/isp_gc20c3_mipi_2lane_av130.conf romfs/init/isp.conf
		elif test $CONFIG_FAST_SENSOR_NUM = "2"; then
			cp utils/etc/isp_gc20c3_mipi_2lane_av130_dual.conf romfs/init/isp.conf
		fi
    elif test $CONFIG_SENSOR_NAME = "gc1084"; then
		if test $CONFIG_FAST_SENSOR_NUM = "1"; then
            cp utils/etc/isp_gc1084_mipi_2lane_av130.conf romfs/init/isp.conf
		fi
    elif test $CONFIG_SENSOR_NAME = "cv2005"; then
		cp utils/etc/isp_cv2005_mipi_2lane_km01a.conf romfs/init/isp.conf
    elif test $CONFIG_SENSOR_NAME = "gc3003"; then
		cp utils/etc/isp_gc3003_mipi_2lane_av130_hp.conf romfs/init/isp.conf
    elif test $CONFIG_SENSOR_NAME = "sc3332"; then
		cp utils/etc/isp_sc3332_mipi_2lane_av130.conf romfs/init/isp.conf
    elif test $CONFIG_SENSOR_NAME = "gc20c3"; then
		cp utils/etc/isp_gc20c3_mipi_2lane_av130.conf romfs/init/isp.conf
	
	fi

	##生成len.bin
	rm -f len.bin
	echo 'S' > len.bin
	if test $CONFIG_FAST_SENSOR_NUM = "1"; then
		gen_ko_bin_sig
	elif test $CONFIG_FAST_SENSOR_NUM = "2"; then
		gen_ko_bin_dual
	elif test $CONFIG_FAST_SENSOR_NUM = "3"; then
		gen_ko_bin_triple
	fi
	
	
	echo 'e' >> len.bin
	cp len.bin romfs/init

	#删除romfs里不必要的文件 
	rm -rf romfs/usr/lib/*
	
	rm -rf romfs/usr/sbin/model*
	rm -rf romfs/usr/sbin/host*
	rm -rf romfs/usr/sbin/ubi*
	rm -rf romfs/usr/modules/fb*.ko 
	rm -rf romfs/usr/modules/sensor*.ko
	rm -rf romfs/usr/sbin/wpa_supplicant romfs/usr/sbin/wpa_cli romfs/usr/sbin/iw*
	rm -rf romfs/usr/modules/ak_venc_adapter.ko romfs/usr/modules/ak_venc_bridge.ko romfs/usr/modules/rtl8188ftv.ko
	rm -rf romfs/usr/bin/*sample
	rm -rf romfs/etc/config/*
	rm -rf romfs/etc/isp*
	#rm -rf romfs/data romfs/dev romfs/lib romfs/mnt romfs/proc romfs/sys romfs/tmp romfs/var
	rm -rf romfs/lib 

	rm -rf romfs/usr/modules/*.ko 

    if [ "$RTC_SAMPLE_TEST" = "false" ]; then
        cp rootfs/usr/bin/ak_fast_aov_sample romfs/usr/bin/
    fi

	mkdir -p romfs/lib/modules/4.4.302-cip94/kernel/net/wireless/
	mkdir -p romfs/lib/modules/4.4.302-cip94/kernel/net/sunrpc/
	mkdir -p romfs/lib/modules/4.4.302-cip94/kernel/fs/nfs/
	mkdir -p romfs/lib/modules/4.4.302-cip94/kernel/fs/lockd/
	mkdir -p romfs/lib/modules/4.4.302-cip94/kernel/fs/nfs_common/
	mkdir -p romfs/lib/modules/4.4.302-cip94/kernel/drivers/mmc/core/
	mkdir -p romfs/lib/modules/4.4.302-cip94/kernel/drivers/mmc/card/
	
	#拷贝C库，支持mtd_debug
	#cp resource/lib/ld-uClibc.so.0 romfs/lib/
	#cp resource/lib/libc.so.0 romfs/lib/
	#cp resource/lib/libiconv.so.2 romfs/lib/
	
	cd $rootfs_dir
	
	#拷贝必要的ko
	cp -rf $driver_install_dir/external/ak_efuse.ko romfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_pwm_char.ko romfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_saradc.ko romfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_gpio_keys.ko romfs/usr/modules/
	#cp -rf $driver_install_dir/external/ak_npu.ko romfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_uio.ko romfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_rtc.ko romfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_leds.ko romfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_pcm.ko romfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_mci.ko romfs/usr/modules/
	cp -rf $driver_install_dir/external/aic8800_netdrv.ko romfs/usr/modules/
	cp -rf $driver_install_dir/external/ak_eth.ko romfs/usr/modules/
	cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/modules* romfs/lib/modules/4.4.302-cip94/
	#cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/net/sunrpc/sunrpc.ko romfs/lib/modules/4.4.302-cip94/kernel/net/sunrpc/
	#cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/fs/nfs/* romfs/lib/modules/4.4.302-cip94/kernel/fs/nfs/
	cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/fs/lockd/* romfs/lib/modules/4.4.302-cip94/kernel/fs/lockd/
	#cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/fs/nfs_common/* romfs/lib/modules/4.4.302-cip94/kernel/fs/nfs_common/
	cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/drivers/mmc/core/mmc_core.ko romfs/lib/modules/4.4.302-cip94/kernel/drivers/mmc/core/
	cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/drivers/mmc/card/mmc_block.ko romfs/lib/modules/4.4.302-cip94/kernel/drivers/mmc/card/
	cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/net/wireless/cfg80211.ko romfs/lib/modules/4.4.302-cip94/kernel/net/wireless/
	
	cp -rf $rootfs_dir/wifi/usr/sbin/custom_msg romfs/usr/sbin/
    if [ "$RTC_SAMPLE_TEST" = "true" ]; then
        cp -rf $driver_install_dir/external/sample/ak_drv_rtc_sample/ak_drv_rtc_sample romfs/usr/sbin/
    fi


    #uvc需要的ko
    if test $CONFIG_FAST_MODE = "fast_uvc"; then
        cp -rf $driver_install_dir/external/ak_udc.ko romfs/usr/modules/
        cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/fs/configfs romfs/lib/modules/4.4.302-cip94/kernel/fs/
        mkdir romfs/lib/modules/4.4.302-cip94/kernel/drivers/
        cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/drivers/usb romfs/lib/modules/4.4.302-cip94/kernel/drivers
        cp -rf $driver_install_dir/internal/lib/modules/4.4.302-cip94/kernel/drivers/media romfs/lib/modules/4.4.302-cip94/kernel/drivers
        rm -rf romfs/lib/modules/4.4.302-cip94/kernel/drivers/usb/storage
        rm -rf romfs/lib/modules/4.4.302-cip94/kernel/drivers/usb/serial
        rm -rf romfs/lib/modules/4.4.302-cip94/kernel/drivers/usb/mon
        rm -rf romfs/lib/modules/4.4.302-cip94/kernel/drivers/usb/class
    fi

	#flash空间不够时可以删掉一些文件
	#rm -rf romfs/usr/sbin/nimble_linux romfs/usr/sbin/atbm* romfs/usr/sbin/custom_msg romfs/usr/sbin/ak_secureboot_burn
	#rm -rf romfs/usr/bin/lrz romfs/usr/bin/lsz romfs/usr/bin/iperf* romfs/usr/bin/tree

	#./genromfs -d romfs -f rootfs.rom -V "romfs"
	names=$(find ./romfs -type f -size +100k | awk -F[/] '{print $NF}' | paste -sd/ )
	echo "names:$names"
	./genromfs -d romfs -f rootfs.rom -V "romfs" -t lz4 -b 4096 -n $names -c 1

	cp rootfs.rom ../tools/burntool/rootfs.rom
	cd $rootfs_dir
	#rm -rf romfs
}

# 制作烧录镜像
make_image()
{
    # 制作fast boot rootfs镜像
    if test "$SYS_BOOT_MODE" = "FAST" ; then
        if test "$FLASH_TYPE" = "NOR" ; then
            ./create_initfs.sh all
            ./create_ramfs.sh all
            ./create_jffs2fs.sh
            ./create_squashfs.sh
            # 将制作好的烧录镜像放到burntool下
            cp -rf usr.sqsh4 $image_install_dir
	    	cp -rf usr.sqsh4  ../tools/burntool/
            cp -rf usr.jffs2 $image_install_dir
	    	cp -rf usr.jffs2  ../tools/burntool/	
        elif test "$FLASH_TYPE" = "NAND" ; then
            ./create_initfs_nand.sh all
            ./create_ramfs_nand.sh all
            # ./create_yaffs2fs_fast_ZKTeco.sh
            ./create_ubifs_fast_ZKTeco.sh
            # cp -rf *.yaffs2 $image_install_dir
            cp -rf *.ubi $image_install_dir
        fi
    else
        # 根据选择的Flash类型制作烧录镜像
		if test $FLASH_TYPE = "NOR" ; then
			./create_jffs2fs.sh
			./create_squashfs.sh

			# 将制作好的烧录镜像放到burntool下
			cp -rf usr.sqsh4 $image_install_dir
			
			#如果是快启，需将usr.jffs2重命名为config.jffs2
			if test "$CONFIG_FAST_MODE" = "fast" || test "$CONFIG_FAST_MODE" = "fast_aov" || test "$CONFIG_FAST_MODE" = "fast_uvc"; then
				cp -rf usr.jffs2 $image_install_dir/config.jffs2
				
    			#if 	test $CHIP_SERIES = "AK3918AV130" ; then
				#	cp -rf fastsys_uImage $image_install_dir/
				#fi
			else	
				cp -rf usr.jffs2 $image_install_dir
				cp -rf root.sqsh4 $image_install_dir
				cp -rf data.jffs2 $image_install_dir  
				cp -rf app.sqsh4 $image_install_dir  
			fi
			
		elif test $FLASH_TYPE = "NAND" ; then
			./create_yaffs2fs.sh
			cp -rf *.yaffs2 $image_install_dir
		elif test $FLASH_TYPE = "MMC" ; then
			echo "mmc image didn't support now!"
		fi
		
		# 制作快启rootfs镜像
		if test $CONFIG_FAST_MODE = "fast" || test $CONFIG_FAST_MODE = "fast_aov" || test $CONFIG_FAST_MODE = "fast_uvc"; then
			make_initfs
			make_fast4
			#制作romfs镜像
			if test $CONFIG_FAST_FS = "romfs" ; then
				make_romfs
			fi
		fi
    fi

}

clean_image()
{
    echo "clean fs image..."
}

get_config_info

cd $rootfs_dir

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

cd -

