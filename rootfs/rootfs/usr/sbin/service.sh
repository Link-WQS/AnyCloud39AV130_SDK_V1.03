#!/bin/sh
# phy_onboard=`cat /proc/eth0 | grep phy_onboard | awk -F" " '{print $2}'`
# if [ "$phy_onboard" = "mounted" ]; then
# udhcpc&
# fi
if [ -f "/data/app.sh" ]
then
	echo "/data/app.sh"
	/data/app.sh
elif [ -f "/etc/config/app.sh" ]
then
	echo "/etc/config/app.sh"
	/etc/config/app.sh
fi
