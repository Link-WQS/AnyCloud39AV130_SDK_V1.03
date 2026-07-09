#!/bin/sh -x

opt=$@

sh build_btdm_wifi_8800m40.sh $opt
if [ $? -eq 0 ]; then
    echo '************************************succeed'
else
    echo '************************************failed'
    exit 1
fi
