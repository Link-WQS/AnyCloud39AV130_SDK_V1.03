#!/bin/sh

install()
{
    echo "install sc2331x driver"
#    v4l2_install
    /sbin/insmod /usr/modules/ak_isp.ko
    /sbin/insmod /usr/modules/sensor_sc2331x.ko addr0=0x30 addr1=0x30 MAX_FPS=46
    if test ! -f /etc/config/isp_sc2331x_mipi_2lane_av130_tri_fsync.conf ; then
        rm /etc/config/isp_*.conf
        cp /etc/isp_sc2331x_mipi_2lane_av130_tri_fsync.conf /etc/config/
    fi
}

uninstall()
{
    echo "uninstall sc2331x driver"
    /sbin/rmmod ak_isp.ko
    /sbin/rmmod sensor_sc2331x.ko
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

