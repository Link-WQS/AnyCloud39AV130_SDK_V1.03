#!/bin/sh
### BEGIN INIT INFO
# File:             wifi_bt_driver_6012bx.sh
# Provides:         wifi ble driver install/uninstall
# Required-Start:   
# Required-Stop:
# Default-Start:     
# Default-Stop:
# Short-Description: atbm wifi ble script
# Author:            
# Email:             
# Date:
### END INIT INFO

PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
MODE=$1
WIFI_EN_GPIO=98
usage()
{
    echo "Usage: $0 MODE"
    echo "       MODE:   | stop"
    echo "               start      Start BLE"
    echo "               stop       Stop  BLE"
}

driver_uninstall()
{
    killall wpa_supplicant
    killall udhcpc
    killall udhcpd
    killall hostapd
    killall nimble_linux
    sleep 1
    rmmod ATBM606x_wifi_sdio.ko
    rmmod cfg80211
}

driver_install()
{
    driver_uninstall
    modprobe mmc_core
    insmod /usr/modules/cfg80211.ko
    insmod /usr/modules/ak_mci.ko
    sleep 2
    insmod /usr/modules/ATBM606x_wifi_sdio.ko wifi_bt_comb=1
    sleep 2
    /usr/sbin/nimble_linux &
}

wifi_bt_connect()
{
    driver_install

    cp /usr/sbin/connect_ap.sh /tmp
    #cd /tmp
    #./connect_ap.sh anyka_2.4Gtest Anyk@.gdgz

    sleep 1
    cd /usr/sbin
    ./atbm_cli AT+SMT_START
}


####### main
case "$MODE" in
    start)
        wifi_bt_connect
        ;;
    stop)
        driver_uninstall
        ;;
    *)
        usage
        ;;
esac
