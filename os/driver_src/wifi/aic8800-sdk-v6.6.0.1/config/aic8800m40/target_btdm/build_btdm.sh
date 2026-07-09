#!/bin/sh

# check toolchain version & libary path
ver=`arm-none-eabi-gcc --version | grep release | cut -d ')' -f2 | awk '{print $1}'`
if [ "$ver" != "9.2.1" ]; then
  echo "WRONG toolchain: $ver"
  exit 1
fi
ver=`echo $GNUARM_4_8_LIB | awk -F"/" '{print $NF}'`
if [ "$ver" != "9.2.1" ]; then
  echo "WRONG libary path: $GNUARM_4_8_LIB"
  exit 1
fi

# build rom files
opt=""
if [ -f ./build_tgt.sh ]; then
  arg=$@" -libuse"
else
  arg=$@
fi
for var in $arg
do
  if [ $var = "-libuse" ]; then
    opt=$opt" USE_LIB_DRV=on USE_LIB_BT=on USE_LIB_AUDIO=on"
  elif [ $var = "-libgen" ]; then
    opt=$opt" GEN_LIB_DRV=on GEN_LIB_BT=on GEN_LIB_AUDIO=on"
  else
    opt=$opt" "$var
  fi
done
# BLE_DFT_STATE       : adv/scan/init/none
# BLE_APP_AUDTRANSMIT : client/server/none
# BLE_APP_SMARTCONFIG : off/on
# BLE_APP_HID         : off/on
# BLE_APP_UDF         : client/server/all_role/none
python ../../../tools/scons.py -u . PRODUCT=host-bt PLF=aic8800m40 BT=armgcc_4_8 BUILD_CMD="$0 $*" AON=off\
        TGT_MODE=btdm \
        BT_BLE=on BTDM=on APPS=on \
        BLE_DFT_STATE=none BLE_APP_AUDTRANSMIT=none BLE_APP_SMARTCONFIG=off BLE_APP_HID=off BLE_APP_UDF=all_role \
        A2DP=on A2DP_SOURCE=off HFP_AG=off LINK_ONE=on TGTNAME=btdm CONSOLE=on AUD_USED=off \
        ROM_VER=auto HEAP_SIZE=0x20000 $opt
