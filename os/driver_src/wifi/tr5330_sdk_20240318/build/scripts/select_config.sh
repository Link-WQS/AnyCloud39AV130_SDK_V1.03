#!/bin/bash
# Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
# Building script.

set -e

# 1: config dir
CONFIG_DIR=$(cd $1;pwd)

if [ ! "$(ls ${CONFIG_DIR}/*.config)" ];then
    echo "no config file in '${CONFIG_DIR}'!"
    exit 1
fi

sum=0
for config in ${CONFIG_DIR}/*.config;do
    echo "${sum}. $(basename ${config})" >&2
    let sum+=1
done

read -p "Type number to select a configuration (default: 0).Other input will exit! " select_number >&2

sum=0
for config in ${CONFIG_DIR}/*.config;do
    if [ "$select_number" = "" ] || [ "$select_number" -eq "$sum" ] 2>/dev/null;then
        echo "${config}"
        exit 0
    fi
    let sum+=1
done

echo "Nothing to do"