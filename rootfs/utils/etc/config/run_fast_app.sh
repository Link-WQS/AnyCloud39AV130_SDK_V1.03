#!/bin/sh

#ak_fast_aov_sample
run_fast_aov_sample_sig()
{
	#单目，快启 , 内核态slice模式
	#ak_fast_aov_sample -v 1 -d 0 -b 1  &
	ak_fast_aov_sample &
}

run_fast_aov_sample_dual()
{
	#双目，快启 , 内核态slice模式
	#ak_fast_aov_sample -v 2 -d 0 -b 1 &
	ak_fast_aov_sample &
}


         
#TF卡的demo
mount_tf_card_and_wifi()
{
	mkdir /tmp/sd
	tf_card.sh                        
	sleep 1
	if [ -e /dev/mmcblk0p1 ]; then                           
		mount -t vfat /dev/mmcblk0p1 /tmp/sd
	else
		mount -t vfat /dev/mmcblk0 /tmp/sd
	fi
	
    cp /tmp/sd/ak_font_16.bin /usr/sbin/
	cp /tmp/sd/ak_fast_main /usr/sbin/
	cp /tmp/sd/fast_test_mode*.ini /etc/config/
	cp /tmp/sd/model_0x0A000000_V1.1.01.bin  /usr/sbin/
	
    is_wifi_exist=`cat /sys/bus/mmc/devices/mmc*/type | grep SDIO`
    if [ -n "$is_wifi_exist" ]; then
        insmod /usr/modules/aic8800_netdrv.ko
    fi
}

#print kernel error default
echo 0 > /proc/sys/kernel/printk
#设置默认系统时间
date -s "2025-1-1 00:00:00"

case "$1" in
	1)
		#跑sample
		run_fast_aov_sample_sig
		
		#跑demo
		#mount_tf_card_and_wifi
		#/usr/sbin/ak_fast_main -Z /etc/config/fast_test_mode_sig.ini &
		;;
	2)
		run_fast_aov_sample_dual
		#mount_tf_card_and_wifi
		#/usr/sbin/ak_fast_main -Z /etc/config/fast_test_mode_dual.ini &
		;;
	*)
		echo "Error: Invalid argument for run_fast_aov_sample!"
		exit 1
		;;
esac

sleep 1
echo 4 > /proc/sys/kernel/printk



