#!/bin/sh

install()
{
    insmod /usr/modules/ak_isp.ko
    insmod /usr/modules/sensor_gc3003.ko addr0=0x37 addr1=0x37 sensor0_switch=1

    if test ! -f /etc/config/isp_gc3003_mipi_2lane_av130_dual.conf ; then
        rm /etc/config/isp_*.conf
        cp /etc/isp_gc3003_mipi_2lane_av130_dual.conf /etc/config/
    fi
}

uninstall()
{
    rmmod /usr/modules/ak_isp.ko
    rmmod /usr/modules/sensor_gc3003.ko 
}

usage()
{
    echo "usage :" 
    echo "    ./gc3003_third.sh <install/uninstall>"
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

