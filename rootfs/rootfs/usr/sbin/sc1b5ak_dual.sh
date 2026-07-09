#!/bin/sh

install()
{    
    insmod /usr/modules/ak_isp.ko
    insmod /usr/modules/sensor_sc3336.ko SENSOR_I2C_ADDR=0x30 addr0=0x32 addr1=0x34 SENSOR_ID=0x1b5a

    if test ! -f /etc/config/isp_sc1b5ak_mipi_2lane_av130_slave.conf ; then
        rm /etc/config/isp_*.conf
        cp /etc/isp_sc1b5ak_mipi_2lane_av130_slave.conf /etc/config/
    fi
}

uninstall()
{
    rmmod /usr/modules/ak_isp.ko
    rmmod /usr/modules/sensor_sc3336.ko 
}

usage()
{
    echo "usage :" 
    echo "    ./sc1B5AK.sh <install/uninstall>"
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

