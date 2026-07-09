#!/bin/sh

install()
{
    echo "install sc1346 driver"
#    v4l2_install
    insmod /usr/modules/ak_pwm_char.ko
    insmod /usr/modules/ak_isp.ko multi_cis_switch=2  gen_pwm_freq_divi=12
    insmod /usr/modules/sensor_sc1346.ko   MAX_FPS=45 addr0=0x31 addr1=0x32 addr2=0x34
    if test ! -f /etc/config/isp_sc1346_mipi_1lane_av130_tri.conf ; then
        rm /etc/config/isp_*.conf
        cp /etc/isp_sc1346_mipi_1lane_av130_tri.conf /etc/config/
    fi
}

uninstall()
{
    echo "uninstall sc1346 driver"
    /sbin/rmmod ak_isp.ko
    /sbin/rmmod sensor_sc1346.ko
    /sbin/rmmod ak_pwm_char.ko
#    v4l2_uninstall
}

usage()
{
    echo "usage :" 
    echo "    ./sensor.sh <install/uninstall>"
    echo "    default : install"

}

if test $# -gt 1 ; then
    usage
elif test $# -eq 0 ; then
    install
elif test $# -eq 1 ; then
    if test $1 = "install" ; then
        install
    elif test $1 = "uninstall" ; then
        uninstall
    else
        usage
    fi
fi

