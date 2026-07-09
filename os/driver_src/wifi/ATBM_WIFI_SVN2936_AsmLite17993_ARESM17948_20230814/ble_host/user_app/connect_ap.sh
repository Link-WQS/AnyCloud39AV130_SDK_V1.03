#!/bin/sh




write_wpa_conf()
{

if [ "$2" == "" ];then
	echo -e "
ctrl_interface=/var/run/wpa_supplicant\n
ap_scan=1\n
network={\n
\tssid=\"$1\"\n
\tkey_mgmt=NONE\n
\tscan_ssid=1\n
	}\n
	" > /tmp/wpa_supplicant.conf
else
	echo -e "
ctrl_interface=/var/run/wpa_supplicant\n
ap_scan=1\n
network={\n
\tssid=\"$1\"\n
\tpsk=\"$2\"\n
\tscan_ssid=1\n
	}\n
	" > /tmp/wpa_supplicant.conf
fi
}






connect_ap()
{

	write_wpa_conf $1 $2
	
	wpa_supplicant -Dnl80211 -iwlan0 -c /tmp/wpa_supplicant.conf -B
	udhcpc -iwlan0 &
}



connect_ap $1 $2
