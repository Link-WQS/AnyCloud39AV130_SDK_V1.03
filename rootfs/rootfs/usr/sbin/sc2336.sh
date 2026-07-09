#!/bin/sh

install()
{
    echo "install sc2336 driver"
#    v4l2_install
    /sbin/insmod /usr/modules/ak_isp.ko
    /sbin/insmod /usr/modules/sensor_sc2336.ko
    if test ! -f /etc/config/isp_sc2336_mipi_2lane_av130.conf ; then
        rm /etc/config/isp_*.conf
        cp /etc/isp_sc2336_mipi_2lane_av130.conf /etc/config/
    fi
}

uninstall()
{
    echo "uninstall sc2336 driver"
    /sbin/rmmod ak_isp.ko
    /sbin/rmmod sensor_sc2336.ko
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

