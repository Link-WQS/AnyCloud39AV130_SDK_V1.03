#!/bin/bash

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
SDK_ROOT_DIR=$(cd "$SCRIPT_DIR/../.." && pwd)
ENV_CFG="env_av130_64M_spinor.cfg"

if [ -f "$SDK_ROOT_DIR/config.mk" ]; then
    # shellcheck disable=SC1090
    source "$SDK_ROOT_DIR/config.mk"
    if [ "$CONFIG_WIFI_TYPE" != "NO_WIFI" ]; then
        ENV_CFG="env_av130_64M_spinor_wifi.cfg"
    fi
fi

cd "$SCRIPT_DIR"
rm env.img
tr '\000' '\377' < /dev/zero | dd of=./env.img bs=1024 count=4
./fw_setenv -s "$ENV_CFG"
./fw_printenv
cp -rf env.img  ../burntool/env_av130_64M_spinor.img
cp -rf env.img  ../../image/env_av130_64M_spinor.img
