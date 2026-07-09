#!/bin/bash
TARGET_DIR="$PWD"
BOOT_TOOLS=$TARGET_DIR/tools/burntool/boot_tool.bin
UPGRADE_IMAGE_DIR=$TARGET_DIR/upgrade/platform/
upgrade_bin_name=$TARGET_DIR/SAT_ANYKA_130L.IMG
upgrade_bin_version=$(date +"%Y%m%d%H%M%S")
UBOOT_NAME="u-boot.bin"
ENV_NAME="env_av130_64M_spinor.img"
ENVBK_NAME="env_av130_64M_spinor.img"
DTB_NAME="EVB_CBDM_AK3918AV130L_V1.0.0.dtb"
KERNEL_NAME="uImage"
ROOTFS_NAME="root.sqsh4"
USR_NAME="usr.sqsh4"
CONFIG_NAME="usr.jffs2"
APP_NAME="app.sqsh4"
DATA_NAME="data.jffs2"

UBOOT_PARTTION="UBOOT"
ENV_PARTTION="ENV"
ENVBK_PARTTION="ENVBK"
DTB_PARTTION="DTB"
KERNEL_PARTTION="KERNEL"
ROOTFS_PARTTION="ROOTFS"
USR_PARTTION="USR"
CONFIG_PARTTION="CONFIG"
APP_PARTTION="APP"
DATA_PARTTION="DATA"

first_arg=$1
#upgrade_bin_version
echo "$BOOT_TOOLS"

if [ "$first_arg" = "all" ]; then
    uboot_upgrade=y
    env_img_uprade=y
    dtb_upgrade=y
    kernel_upgrade=y
    rootfs_upgrade=y
    usr_upgrade=y
    config_upgrade=y
    app_upgrade=y
    data_upgrade=y
elif [ "$first_arg" = "app" ]; then
    app_upgrade=y
else
    echo -n "upgrade $UBOOT_PARTTION? [y/n]"
    read -n 2 uboot_upgrade

    echo -n "upgrade $ENV_PARTTION? [y/n]"
    read -n 2 env_img_uprade

    echo -n "upgrade $DTB_PARTTION? [y/n]"
    read -n 2 dtb_upgrade

    echo -n "upgrade  $KERNEL_PARTTION? [y/n]"
    read -n 2 kernel_upgrade

    echo -n "upgrade $ROOTFS_PARTTION? [y/n]"
    read -n 2 rootfs_upgrade

    echo -n "upgrade $USR_PARTTION?[y/n]"
    read -n 2 usr_upgrade

    echo -n "upgrade  $CONFIG_PARTTION? [y/n]"
    read -n 2 config_upgrade

    echo -n "upgrade $APP_PARTTION? [y/n]"
    read -n 2 app_upgrade

    echo -n "upgrade $DATA_PARTTION? [y/n]"
    read -n 2 data_upgrade
    
fi

if [ -e $upgrade_bin_name ]; then
    rm -f $upgrade_bin_name
fi

echo "#<upgrade_bin_version=$upgrade_bin_version>" >$upgrade_bin_name

parttion_start_postion=0
if [ "$uboot_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$UBOOT_NAME")
    echo "# File Parttion: $UBOOT_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$env_img_uprade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$ENV_NAME")
    echo "# File Parttion: $ENV_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))

    echo "# File Parttion: $ENVBK_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$dtb_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$DTB_NAME")
    echo "# File Parttion: $DTB_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$kernel_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$KERNEL_NAME")
    echo "# File Parttion: $KERNEL_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi


if [ "$rootfs_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$ROOTFS_NAME")
    echo "# File Parttion: $ROOTFS_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$usr_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$USR_NAME")
    echo "# File Parttion: $USR_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$config_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$CONFIG_NAME")
    echo "# File Parttion: $CONFIG_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$app_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$APP_NAME")
    echo "# File Parttion: $APP_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$data_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$DATA_NAME")
    echo "# File Parttion: $DATA_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

echo "# <- this is end of image parttion" >>$upgrade_bin_name
if [ "$uboot_upgrade" = "y" ]; then
    dd if=$BOOT_TOOLS bs=512 count=1 >>$upgrade_bin_name
    dd if=$UPGRADE_IMAGE_DIR$UBOOT_NAME bs=512 skip=1 >>$upgrade_bin_name
fi
if [ "$env_img_uprade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$ENV_NAME bs=512 conv=notrunc >>$upgrade_bin_name
    dd if=$UPGRADE_IMAGE_DIR$ENV_NAME bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$dtb_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$DTB_NAME bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$kernel_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$KERNEL_NAME bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$rootfs_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$ROOTFS_NAME bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$usr_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$USR_NAME bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$config_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$CONFIG_NAME bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$app_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$APP_NAME bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$data_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$DATA_NAME bs=512 conv=notrunc >>$upgrade_bin_name
fi

chmod 777 $upgrade_bin_name
sync
