#!/bin/sh

BLUEZ_PATH=/usr/local
 
export LD_LIBRARY_PATH=${BLUEZ_PATH}/lib:$LD_LIBRARY_PATH
export PATH=${BLUEZ_PATH}/libexec/bluetooth:$PATH
export PATH=${BLUEZ_PATH}/bin:$PATH
export PATH=${BLUEZ_PATH}/../bin:$PATH

modprobe cfg80211
modprobe mac80211

modprobe mmc_core.ko
modprobe mmc_block.ko
#insmod /usr/modules/ak_mci.ko
insmod /usr/modules/ak_mci.ko sdio_clk_mode=1

insmod /usr/modules/crc16.ko
insmod /usr/modules/bluetooth.ko

#insmod wifi&bt module driver
insmod ./ssv6158.ko stacfgpath=./ssv6x5x-wifi.cfg


#prepare for bluez application running
sleep 1
hciconfig hci0 up
mkdir -p /var/run/dbus
dbus-daemon --system --print-pid --print-address
bluetoothd -n -d &





