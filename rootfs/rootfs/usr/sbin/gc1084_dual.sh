#!/bin/sh

install()
{
    echo "install gc1084 driver"
#    v4l2_install
    /sbin/insmod /usr/modules/ak_isp.ko
    /sbin/insmod /usr/modules/sensor_gc1084.ko MAX_FPS=30 addr0=0x39 addr1=0x40
    if test ! -f /etc/config/isp_gc1084_mipi_1lane_av130_dual.conf ; then
        rm /etc/config/isp_*.conf
        cp /etc/isp_gc1084_mipi_1lane_av130_dual.conf /etc/config/
    fi
}

uninstall()
{
    echo "uninstall gc1084 driver"
    /sbin/rmmod ak_isp.ko
    /sbin/rmmod sensor_gc1084.ko
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

