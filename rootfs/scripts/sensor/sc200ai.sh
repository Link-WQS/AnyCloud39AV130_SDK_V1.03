#!/bin/sh

install()
{
    echo "install sc200ai driver"
#    v4l2_install
    /sbin/insmod /usr/modules/ak_vi.ko
	/sbin/insmod /usr/modules/ak_vicap.ko
	/sbin/insmod /usr/modules/ak_isp.ko
	/sbin/insmod /usr/modules/ak_pp.ko
    /sbin/insmod /usr/modules/sensor_sc200ai.ko check_id=1
    if test ! -f /etc/config/isp_sc200ai_mipi_2lane_av130.conf ; then
        rm /etc/config/isp_*.conf
        cp /etc/isp_sc200ai_mipi_2lane_av130.conf /etc/config/
    fi
}

uninstall()
{
    echo "uninstall sc200ai driver"
    rmmod /usr/modules/ak_pp.ko
    rmmod /usr/modules/ak_isp.ko
    rmmod /usr/modules/ak_vicap.ko
    rmmod /usr/modules/ak_vi.ko
    rmmod /usr/modules/sensor_sc200ai.ko
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

