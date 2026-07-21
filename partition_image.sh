#!/bin/bash
TARGET_DIR="$PWD"
BOOT_TOOLS=$TARGET_DIR/tools/burntool/u-boot_tool_128.bin
UPGRADE_IMAGE_DIR=$TARGET_DIR/image/
upgrade_bin_name=$TARGET_DIR/ANYKA_130L.IMG
uboot_raw_upgrade_name=$TARGET_DIR/ANYKA_130L_UBOOT_RAW.IMG
PARTITION_CFG=$TARGET_DIR/tools/envtool/env_av130_64M_spinor.cfg
SDK_ROOT_DIR=$TARGET_DIR
upgrade_bin_version=$(date +"%Y%m%d%H%M%S")
HEADER_ALIGN=64
UPGRADE_IMAGE_END_MARKER="# <- this is end of image parttion"
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

if [ -f "$SDK_ROOT_DIR/config.mk" ]; then
    # shellcheck disable=SC1090
    source "$SDK_ROOT_DIR/config.mk"
    if [ "$CONFIG_WIFI_TYPE" != "NO_WIFI" ]; then
        PARTITION_CFG=$TARGET_DIR/tools/envtool/env_av130_64M_spinor_wifi.cfg
    fi
fi

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

TMP_IMAGE_DIR=$(mktemp -d /tmp/anyka_partition_image.XXXXXX)

cleanup() {
    rm -rf "$TMP_IMAGE_DIR"
}

trap cleanup EXIT

get_mtdparts() {
    awk '$1 == "mtdparts" { sub(/^mtdparts[[:space:]]+/, ""); print; exit }' "$PARTITION_CFG"
}

parse_size_spec() {
    local size_spec=$1
    local number suffix

    number=${size_spec%[KkMm]}
    suffix=${size_spec#$number}

    case "$suffix" in
        K|k)
            echo $((number * 1024))
            ;;
        M|m)
            echo $((number * 1024 * 1024))
            ;;
        *)
            echo "$number"
            ;;
    esac
}

get_partition_size() {
    local part_name=$1
    local mtdparts part_entry size_spec

    mtdparts=$(get_mtdparts)
    part_entry=$(printf '%s\n' "$mtdparts" | tr ',' '\n' | grep "(${part_name})" | head -n 1)
    if [ -z "$part_entry" ]; then
        echo "partition ${part_name} not found in $PARTITION_CFG" >&2
        exit 1
    fi

    size_spec=$(printf '%s\n' "$part_entry" | sed -E 's/^.*spi-nor://; s/@.*$//')
    parse_size_spec "$size_spec"
}

append_ff_padding() {
    local out_file=$1
    local current_size=$2
    local target_size=$3
    local padding_size

    padding_size=$((target_size - current_size))
    if [ "$padding_size" -lt 0 ]; then
        echo "image ${out_file} size ${current_size} exceeds partition size ${target_size}" >&2
        exit 1
    fi

    if [ "$padding_size" -gt 0 ]; then
        dd if=/dev/zero bs=1 count="$padding_size" 2>/dev/null | tr '\000' '\377' >>"$out_file"
    fi
}

build_padded_image() {
    local part_name=$1
    local src_name=$2
    local out_file=$3
    local partition_size actual_size

    partition_size=$(get_partition_size "$part_name")

    case "$part_name" in
        "$UBOOT_PARTTION")
            # UBOOT 仍沿用原来的打包方式：前 512B 取工具头，后续内容取 u-boot.bin 跳过首块后的数据。
            dd if="$BOOT_TOOLS" bs=512 count=1 of="$out_file" 2>/dev/null
            dd if="$UPGRADE_IMAGE_DIR$src_name" bs=512 skip=1 conv=notrunc oflag=append of="$out_file" 2>/dev/null
            ;;
        *)
            cat "$UPGRADE_IMAGE_DIR$src_name" >"$out_file"
            ;;
    esac

    actual_size=$(wc -c <"$out_file")
    # 将分区镜像补齐到 mtdparts 定义的分区大小，保证 custom_upgrade 按 file_size 写时也能写满整个分区。
    append_ff_padding "$out_file" "$actual_size" "$partition_size"
    echo "$partition_size"
}

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

if [ -e $uboot_raw_upgrade_name ]; then
    rm -f $uboot_raw_upgrade_name
fi

echo "#<upgrade_bin_version=$upgrade_bin_version>" >$upgrade_bin_name

parttion_start_postion=0
if [ "$uboot_upgrade" = "y" ]; then
    build_padded_image "$UBOOT_PARTTION" "$UBOOT_NAME" "$TMP_IMAGE_DIR/$UBOOT_NAME" >/dev/null
    value=$(wc -c <"$TMP_IMAGE_DIR/$UBOOT_NAME")
    echo "# File Parttion: $UBOOT_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$env_img_uprade" = "y" ]; then
    build_padded_image "$ENV_PARTTION" "$ENV_NAME" "$TMP_IMAGE_DIR/$ENV_NAME" >/dev/null
    build_padded_image "$ENVBK_PARTTION" "$ENVBK_NAME" "$TMP_IMAGE_DIR/$ENVBK_NAME" >/dev/null
    value=$(wc -c <"$TMP_IMAGE_DIR/$ENV_NAME")
    echo "# File Parttion: $ENV_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))

    echo "# File Parttion: $ENVBK_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$dtb_upgrade" = "y" ]; then
    build_padded_image "$DTB_PARTTION" "$DTB_NAME" "$TMP_IMAGE_DIR/$DTB_NAME" >/dev/null
    value=$(wc -c <"$TMP_IMAGE_DIR/$DTB_NAME")
    echo "# File Parttion: $DTB_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$kernel_upgrade" = "y" ]; then
    build_padded_image "$KERNEL_PARTTION" "$KERNEL_NAME" "$TMP_IMAGE_DIR/$KERNEL_NAME" >/dev/null
    value=$(wc -c <"$TMP_IMAGE_DIR/$KERNEL_NAME")
    echo "# File Parttion: $KERNEL_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi


if [ "$rootfs_upgrade" = "y" ]; then
    build_padded_image "$ROOTFS_PARTTION" "$ROOTFS_NAME" "$TMP_IMAGE_DIR/$ROOTFS_NAME" >/dev/null
    value=$(wc -c <"$TMP_IMAGE_DIR/$ROOTFS_NAME")
    echo "# File Parttion: $ROOTFS_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$usr_upgrade" = "y" ]; then
    build_padded_image "$USR_PARTTION" "$USR_NAME" "$TMP_IMAGE_DIR/$USR_NAME" >/dev/null
    value=$(wc -c <"$TMP_IMAGE_DIR/$USR_NAME")
    echo "# File Parttion: $USR_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$config_upgrade" = "y" ]; then
    build_padded_image "$CONFIG_PARTTION" "$CONFIG_NAME" "$TMP_IMAGE_DIR/$CONFIG_NAME" >/dev/null
    value=$(wc -c <"$TMP_IMAGE_DIR/$CONFIG_NAME")
    echo "# File Parttion: $CONFIG_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$app_upgrade" = "y" ]; then
    build_padded_image "$APP_PARTTION" "$APP_NAME" "$TMP_IMAGE_DIR/$APP_NAME" >/dev/null
    value=$(wc -c <"$TMP_IMAGE_DIR/$APP_NAME")
    echo "# File Parttion: $APP_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$data_upgrade" = "y" ]; then
    build_padded_image "$DATA_PARTTION" "$DATA_NAME" "$TMP_IMAGE_DIR/$DATA_NAME" >/dev/null
    value=$(wc -c <"$TMP_IMAGE_DIR/$DATA_NAME")
    echo "# File Parttion: $DATA_PARTTION, $parttion_start_postion, $value" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

header_size=$(wc -c <"$upgrade_bin_name")
end_marker_size=$(printf "%s\n" "$UPGRADE_IMAGE_END_MARKER" | wc -c)
padding_size=$(( (HEADER_ALIGN - ((header_size + end_marker_size) % HEADER_ALIGN)) % HEADER_ALIGN ))
if [ "$padding_size" -gt 0 ]; then
    dd if=/dev/zero bs=1 count=$padding_size 2>/dev/null | tr '\000' ' ' >>$upgrade_bin_name
fi
printf "%s\n" "$UPGRADE_IMAGE_END_MARKER" >>$upgrade_bin_name

if [ "$uboot_upgrade" = "y" ]; then
    cat "$TMP_IMAGE_DIR/$UBOOT_NAME" >>"$upgrade_bin_name"
    # dd if=$BOOT_TOOLS bs=512 count=1 >$uboot_raw_upgrade_name
    # dd if=$UPGRADE_IMAGE_DIR$UBOOT_NAME bs=512 skip=1 >>$uboot_raw_upgrade_name
fi
if [ "$env_img_uprade" = "y" ]; then
    cat "$TMP_IMAGE_DIR/$ENV_NAME" >>"$upgrade_bin_name"
    cat "$TMP_IMAGE_DIR/$ENVBK_NAME" >>"$upgrade_bin_name"
fi
if [ "$dtb_upgrade" = "y" ]; then
    cat "$TMP_IMAGE_DIR/$DTB_NAME" >>"$upgrade_bin_name"
fi
if [ "$kernel_upgrade" = "y" ]; then
    cat "$TMP_IMAGE_DIR/$KERNEL_NAME" >>"$upgrade_bin_name"
fi
if [ "$rootfs_upgrade" = "y" ]; then
    cat "$TMP_IMAGE_DIR/$ROOTFS_NAME" >>"$upgrade_bin_name"
fi
if [ "$usr_upgrade" = "y" ]; then
    cat "$TMP_IMAGE_DIR/$USR_NAME" >>"$upgrade_bin_name"
fi
if [ "$config_upgrade" = "y" ]; then
    cat "$TMP_IMAGE_DIR/$CONFIG_NAME" >>"$upgrade_bin_name"
fi
if [ "$app_upgrade" = "y" ]; then
    cat "$TMP_IMAGE_DIR/$APP_NAME" >>"$upgrade_bin_name"
fi
if [ "$data_upgrade" = "y" ]; then
    cat "$TMP_IMAGE_DIR/$DATA_NAME" >>"$upgrade_bin_name"
fi

chmod 777 $upgrade_bin_name
if [ -e $uboot_raw_upgrade_name ]; then
    chmod 777 $uboot_raw_upgrade_name
fi
sync
