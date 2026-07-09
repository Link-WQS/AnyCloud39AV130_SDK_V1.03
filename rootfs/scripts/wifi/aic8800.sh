#! /bin/sh
### BEGIN INIT INFO
# File:             aic8800.sh
# Provides:         driver install/uninstall, operate as station/ap
# Required-Start:   $
# Required-Stop:
# Default-Start:
# Default-Stop:
# Short-Description: aic8800 wifi script
# Author:
# Email:
# Date:                2020-02-28
### END INIT INFO

PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin
MODE=$1
IS_NET_WAKEUP_PIN=66
WAKEUP_PIN=69
usage()
{
    echo "Usage:"
    echo "               ap  <ssid> <pwd> [<2.4G/5G>] [channel] [akm unicast_cipher group_cipher] Start wifi as ap role"
    echo "                 -- channel = 1~14(2.4G),5G(depend on fhost_chan)"
    echo "                 -- akm = Authentication and Key Management suite, WPA/PSK, WPA2/PSK, WPA3/PSK"
    echo "                 -- unicast_cipher = unicast cipher suite, CCMP/TKIP"
    echo "                 -- group_cipher = group cipher suite, CCMP/TKIP"
    echo "               sta <ssid> [<pwd>]                     Start wifi as station role"
    echo "               blecfg                                 Start blewifi_cfg"
    echo "               stop                                   Stop  wifi "
}

wifi_stop()
{
    killall fasync_user
    rmmod aic8800_netdrv
}

driver_install()
{
    #wifi_stop
    modprobe mmc_core
    insmod /usr/modules/ak_mci.ko
    sleep 2
    insmod /usr/modules/aic8800_netdrv.ko
    ifconfig vnet0 up
}

####### main
case "$MODE" in
    ap)
        if [ $# -lt 3 ]; then
          usage
          exit 1
        fi
        if [ $# -gt 3 ]; then
          band=$4
        else
          band=2.4G
        fi
        if [ $# -gt 4 ]; then
          channel=$5
        else
          channel=
        fi
        if [ $# -gt 5 ]; then
          akm=$6
          if [ $# -gt 6 ]; then
            uc=$7
          else
            uc=          
          fi
          if [ $# -gt 7 ]; then
            gc=$8
          else
            gc=
          fi
        else
          akm=
          uc=
          gc=
        fi
        
        driver_install
        /usr/sbin/custom_msg vnet0 9 $2 $3 $band $channel $akm $uc $gc
        ;;
    sta)
        if [ $# -lt 2 ]; then
          usage
          exit 1
        fi
        driver_install
        /usr/sbin/fasync_user &
        /usr/sbin/custom_msg vnet0 1 $2 $3
        ;;
    blecfg)
        driver_install
        /usr/sbin/custom_msg vnet0 3
        /usr/sbin/custom_msg vnet0 16
        ;;
    stop)
        wifi_stop
        ;;
    sleep)
        is_wifi_connect=`/usr/sbin/custom_msg vnet0 8 | grep 'ip:'`
        if [ -n "$is_wifi_connect" ]; then
          pkill -9 fasync_user
          /usr/sbin/custom_msg vnet0 4
          /usr/sbin/custom_msg vnet0 5
          rmmod aic8800_netdrv
          echo 201b0000.mmc2 > /sys/module/ak_mci/drivers/platform\:akmci/unbind
          sleep 1
          echo ${IS_NET_WAKEUP_PIN} > /sys/class/gpio/export
          echo in > /sys/class/gpio/gpio${IS_NET_WAKEUP_PIN}/direction
          echo 1 > /sys/class/gpio/gpio${IS_NET_WAKEUP_PIN}/input_enable
        else
          echo connect to ap before going to sleep
        fi
        ;;                                                                                                              
    wakeup)
        echo ${WAKEUP_PIN} > /sys/class/gpio/export
        echo out > /sys/class/gpio/gpio${WAKEUP_PIN}/direction
        echo 1 > /sys/class/gpio/gpio${WAKEUP_PIN}/value
        sleep 0.01
        echo ${WAKEUP_PIN} > /sys/class/gpio/unexport
        echo ${IS_NET_WAKEUP_PIN} > /sys/class/gpio/unexport
        sleep 1.5
        echo 201b0000.mmc2 > /sys/module/ak_mci/drivers/platform\:akmci/bind
        insmod /usr/modules/aic8800_netdrv.ko
        ifconfig vnet0 up
        /usr/sbin/fasync_user &
        /usr/sbin/custom_msg vnet0 6
        /usr/sbin/custom_msg vnet0 8
        ;;
    *)
        usage
        ;;
esac
exit 0


