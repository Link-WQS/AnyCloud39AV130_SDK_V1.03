#!/bin/sh
FILE_CONF=`pwd`/secureboot.conf
DIR_BURNTOOL=`pwd`/../burntool
SIGN_SECURE=sign_secure.bin                               #用于制作启动信息区的验证签名文件，用于烧录工具烧写
PRODUCER_SECURE=producer_secure.bin                                 #用于制作单个镜像的烧录文件，比如下载produce和升级内核等

if [ -f "$FILE_CONF" ]; then
    BURN_LEVEL=`cat $FILE_CONF | grep -E "BURN_LEVEL" | grep -Eo "[1-4]"`       #安全启动等级1-4
    BURN_TYPE=`cat $FILE_CONF | grep -E "BURN_TYPE" | grep -Eo "[0|1]"`         #USB模式-0 UART模式-1
                                                                                #烧写的波特率
    BAUD=`cat $FILE_CONF | grep -E "BAUD" | grep -Eo "(0)|(115200)|(921600)|(1000000)|(1500000)|(2000000)|(3000000)|(4000000)|(6000000)"`
    ID=`cat $FILE_CONF | grep -E "ID" | grep -Eo "[0-9a-fA-F]{12}"`             #唯一ID
                                                                                #ddr配置文件里面使用的配置小节
    DDR_PARAM=`cat $FILE_CONF | grep -E "DDR_PARAM" | grep -Eo "[0-9]+M_Ram_Param[0-9]+"`
    AES_KEY=`cat $FILE_CONF | grep -E "AES_KEY" | grep -Eo "[0-9a-fA-F]{64}"`   #AES加密密钥
    AES_IV=`cat $FILE_CONF | grep -E "AES_IV" | grep -Eo "[0-9a-fA-F]{32}"`     #AES加密IV值
                                                                                #RSA私钥文件名称
    RSA_FILE_PRIVATEKEY=`cat $FILE_CONF | grep -E "RSA_FILE_PRIVATEKEY" | grep -Eo "=.*$" | grep -Eo "[^= ]+$"`
    SECOND_SIGN=`cat $FILE_CONF | grep -E "SECOND_SIGN" | grep -Eo "[0|1]"`     #是否进行二级loader 是-1 否-0
                                                                                #OS目录路径
    DIR_OS=`cat $FILE_CONF | grep -E "DIR_OS" | grep -Eo "=.*$" | grep -Eo "[^= ]+$"`
    BURNTOOL_CONF=`cat $FILE_CONF | grep -E "BURNTOOL_CONF" | grep -Eo "config.*txt"`           #burntool配置文件
                                                                                #二级loader重新编译uboot的所使用的设备树
    UBOOT_DEV_TREE=`cat $FILE_CONF | grep -E "UBOOT_DEV_TREE" | grep -Eo "=.*$" | grep -Eo "[^= ]+$"`
fi

if [ "$BURN_LEVEL" = "" ]; then
    echo $BURN_LEVEL
    BURN_LEVEL=1
fi

if [ "$BURN_TYPE" = "" ]; then
    BURN_TYPE=0
fi

if [ "$BAUD" = "" ]; then
    BAUD=115200
fi

if [ "$ID" = "" ]; then
    ID=""
fi

if [ "$DDR_PARAM" = "" ]; then
    DDR_PARAM=64M_Ram_Param1
fi

if [ "$AES_KEY" = "" ]; then
    AES_KEY=20AE35D584E52A4CAFABE8463C2A9BF84151B00F18AC130A212384491A48F625
fi

if [ "$AES_IV" = "" ]; then
    AES_IV=DE632074A5B8C31E289347711490A91F
fi

if [ "$RSA_FILE_PRIVATEKEY" = "" ]; then
    RSA_FILE_PRIVATEKEY=./private.pem
fi

if [ "$SECOND_SIGN" = "" ]; then
    SECOND_SIGN=0
fi

if [ "$DIR_OS" = "" ]; then
    if [ -d "../../../../os" ]; then
        DIR_OS=../../../../os
    elif [ -d "../../../os" ]; then
        DIR_OS=../../../os
    elif [ -d "../../os" ]; then
        DIR_OS=../../os
    fi
fi

if [ "$BURNTOOL_CONF" = "" ]; then
    BURNTOOL_CONF=config_km01a_km01w_64MB_fast_ramfs_8MB_flash_config.txt
fi

if [ "$DIR_UBOOT" = "" ]; then
    if [ -d "$DIR_OS/uboot2019" ]; then
        DIR_UBOOT=$DIR_OS/uboot2019
    elif [ -d "$DIR_OS/uboot" ]; then
        DIR_UBOOT=$DIR_OS/uboot
    fi
fi

#if [ "$UBOOT_DEV_TREE" = "" ]; then                                             #根据config.mk获取uboot的设备树
#    if [ -f "../../../config.mk" ]; then
#        FILE_CONFIG_MK=../../../config.mk
#    elif [ -f "../../config.mk" ]; then
#        FILE_CONFIG_MK=../../config.mk
#    fi
#                                                                                #获取设备类型
#    CHIP_TYPE=`cat $FILE_CONFIG_MK | grep -E "CONFIG_DTB_NAME" | grep -Eo "=.*$" | grep -Eo "[^= ]+$"`
#                                                                                #获取设备树配置文件
#    UBOOT_DEV_TREE=`find $DIR_UBOOT/arch/arm/dts -name "*$CHIP_TYPE*.dts" | grep -Ev "sky" | grep -Eo "[^/]+$" | sed "s/.dts//g"`
#fi

if [ "$FLASH_TYPE" = "" ]; then
    FLASH_TYPE=0
fi

function menu()
{
cat <<EOF
####################
    安全启动配置
####################
`echo -e " 0) 安全启动等级 : [$BURN_LEVEL]"`
`echo -e " 1) 烧写模式 : [$BURN_TYPE]"`
`echo -e " 2) 烧写波特率 : [$BAUD]"`
`echo -e " 3) 设备ID : [$ID]"`
`echo -e " 4) DDR参数配置项 : [$DDR_PARAM]"`
`echo -e " 5) AES加密密钥 : [$AES_KEY]"`
`echo -e " 6) AES加密IV值 : [$AES_IV]"`
`echo -e " 7) RSA私钥文件 : [$RSA_FILE_PRIVATEKEY]"`
`echo -e " 8) 二级loader : [$SECOND_SIGN]"`
`echo -e " 9) OS目录(二级loader需要) : [$DIR_OS]"`
`echo -e " B) burntool配置文件 : [$BURNTOOL_CONF]"`
`echo -e " U) UBOOT设备树配置 : [$UBOOT_DEV_TREE]"`
`echo -e " F) 闪存类型 : [$FLASH_TYPE]"`
`echo -e " C) 创建安全启动烧写文件"`
`echo -e " S) 保存当前配置信息"`
`echo -e " X) 退出"`
EOF
read -p "请输入选项：" num
case $num in
    0)
    set_burn_level
    ;;
    1)
    set_burn_type
    ;;
    2)
    set_baud
    ;;
    3)
    set_id
    ;;
    4)
    set_ddr
    ;;
    5)
    set_aes_key
    ;;
    6)
    set_aes_iv
    ;;
    7)
    set_rsa_file_privatekey
    ;;
    8)
    set_second_sign
    ;;
    9)
    set_dir_uboot
    ;;
    b|B)
    set_burntool_conf
    ;;
    u|U)
    set_uboot_dev_tree
    ;;
    f|F)
    set_flash_type
    ;;
    c|C)
    build_secureboot_file
    ;;
    s|S)
    save_secureboot_conf
    ;;
    x|X)
    exit 0
    ;;
    *)
    echo "请输入正确的编号."
    menu
esac
}

function set_burn_level()
{
cat<<EOF
########################
    设置安全启动等级
########################
                                               BOND AES  RSA
1) 使用RSA2048进行签名和验签                             y
2) 使用AES256进行加密和解密                         y
3) 使用AES256+RSA2048                               y    y
X) 返回主菜单
------------------------
EOF
read -p "请输入编号:" num
if [ -z "$num" ]; then
menu
fi
case $num in
    1)
    BURN_LEVEL=1
    menu
    ;;
    2)
    BURN_LEVEL=2
    menu
    ;;
    3)
    BURN_LEVEL=3
    menu
    ;;
    x|X)
    menu
    ;;
    *)
    echo -e "请输入正确编号"
    set_burn_level
esac
}

function set_burn_type()
{
cat<<EOF
####################
    设置烧写模式
####################
0) USB烧写模式
1) UART烧写模式
X) 返回主菜单
------------------------
EOF
read -p "请输入编号:" num
if [ -z "$num" ]; then
menu
fi
case $num in
    0)
    BURN_TYPE=0
    menu
    ;;
    1)
    BURN_TYPE=1
    menu
    ;;
    x|X)
    menu
    ;;
    *)
    echo -e "请输入正确编号"
    set_burn_type
esac
}

function set_baud()
{
cat<<EOF
######################
    设置烧写波特率
######################
0) 0
1) 115200
2) 921600
3) 1000000
4) 1500000
5) 2000000
6) 3000000
7) 4000000
8) 6000000
X) 返回主菜单
------------------------
EOF
read -p "请输入编号:" num
if [ -z "$num" ]; then
menu
fi
case $num in
    0)
    BAUD=0
    menu
    ;;
    1)
    BAUD=115200
    menu
    ;;
    2)
    BAUD=921600
    menu
    ;;
    3)
    BAUD=1000000
    menu
    ;;
    4)
    BAUD=1500000
    menu
    ;;
    5)
    BAUD=2000000
    menu
    ;;
    6)
    BAUD=3000000
    menu
    ;;
    7)
    BAUD=4000000
    menu
    ;;
    8)
    BAUD=6000000
    menu
    ;;
    x|X)
    menu
    ;;
    *)
    echo -e "请输入正确编号"
    set_baud
esac
}

function set_id()
{
cat<<EOF
######################
    设置设备ID
######################
<ENTER>)  返回主菜单
------------------------
EOF
read -p "请输入ID(12字节长度HEX):" num
if [ -z "$num" ]; then
menu
fi
ID_TMP=`echo $num | grep -Eo "^[0-9a-fA-F]{12}"`
if [ -z "$ID_TMP" ]; then
echo "ID格式不合法."
set_id
else
ID=$ID_TMP
menu
fi
}

function set_ddr()
{
cat<<EOF
######################
    DDR参数配置项
######################
<ENTER>)  返回主菜单
------------------------
可输入的选项:
`cat $DIR_BURNTOOL/SDK_CPU_DDR2.ini | grep -Eo "\\[[0-9]+.*\\]" | grep -Eo "[0-9a-zA-Z_]+"`
EOF
read -p "请输入ddr参数项:" ddr
if [ -z "$ddr" ]; then
menu
fi
DDRTMP=`echo $ddr | grep -Eo "^[0-9]+M_Ram_Param[0-9]+"`
if [ -z "$DDRTMP" ]; then
echo "ddr参数项不合法."
set_ddr
else
DDR_PARAM=$DDRTMP
menu
fi
}

function set_aes_key()
{
cat<<EOF
########################
    设置AES KEY
########################
<ENTER>)  返回主菜单
------------------------
EOF
read -p "请输入AES KEY(64字节长度HEX):" num
if [ -z "$num" ]; then
menu
fi
AES_KEY_TMP=`echo $num | grep -Eo "^[0-9a-fA-F]{64}"`
if [ -z "$AES_KEY_TMP" ]; then
echo "AES KEY格式不合法."
set_aes_key
else
AES_KEY=$AES_KEY_TMP
menu
fi
}

function set_aes_iv()
{
cat<<EOF
##################
    设置AES IV
##################
<ENTER>)  返回主菜单
------------------------
EOF
read -p "请输入AES IV(32字节长度HEX):" num
if [ -z "$num" ]; then
menu
fi
AES_IV_TMP=`echo $num | grep -Eo "^[0-9a-fA-F]{32}"`
if [ -z "$AES_IV_TMP" ]; then
echo "AES IV格式不合法."
set_aes_key
else
AES_IV=$AES_IV_TMP
menu
fi
}

function set_rsa_file_privatekey()
{
cat<<EOF
########################
    设置RSA私钥文件
########################
<ENTER>)  返回主菜单
------------------------
`ls -1 *.pem`
EOF
read -p "请输入rsa私钥文件名称:" file
if [ -z "$file" ]; then
menu
fi
if [ ! -f "$file" ]; then
echo "文件不存在"
set_rsa_file_privatekey
else
RSA_FILE_PRIVATEKEY=$file
menu
fi
}

function set_second_sign()
{
cat<<EOF
####################
    设置二级loader
####################
0) 不进行二级loader
1) 进行二级loader
X) 返回主菜单
------------------------
EOF
read -p "请输入编号:" num
if [ -z "$num" ]; then
menu
fi
case $num in
    0)
    SECOND_SIGN=0
    menu
    ;;
    1)
    SECOND_SIGN=1
    menu
    ;;
    x|X)
    menu
    ;;
    *)
    echo -e "请输入正确编号"
    set_second_sign
esac
}

function set_dir_uboot()
{
cat<<EOF
########################
    设置系统OS目录
########################
<ENTER>)  返回主菜单
------------------------
EOF
read -p "请输入系统OS目录路径:" dir
if [ -z "$dir" ]; then
menu
fi
if [ -d "$dir" ]; then
DIR_OS=$dir
menu
else
echo "目录不存在: $dir"
set_dir_uboot
fi
}

function set_burntool_conf()
{
cat<<EOF
####################
    设置burntool使用的配置文件模板
####################
<ENTER>)  返回主菜单
------------------------
`find $DIR_BURNTOOL/ -name "config*.txt" | grep -Eo "config.*txt$"`
EOF
read -p "文件名称:" file
if [ -z "$file" ]; then
menu
fi
if [ ! -f "$DIR_BURNTOOL/$file" ]; then
echo "文件不存在!"
set_burntool_conf
else
BURNTOOL_CONF=$file
menu
fi
}

function set_uboot_dev_tree()
{
cat<<EOF
##########################
    设置uboot设备树文件
##########################
<ENTER>)  返回主菜单
------------------------
`find $DIR_UBOOT/arch/arm/dts -name "*AK3918AV*.dts" | grep -Eo "[^/]+$" | sed "s/.dts//g"`
EOF
read -p "设置uboot设备树配置:" file
if [ -z "$file" ]; then
menu
fi
if [ ! -f "$DIR_UBOOT/arch/arm/dts/$file.dts" ]; then
echo "文件不存在!"
set_uboot_dev_tree
else
UBOOT_DEV_TREE=$file
menu
fi
}

function set_flash_type()
{
cat<<EOF
####################
    设置设备内存
####################
0) NOR FLASH
1) NAND FLASH
X) 返回主菜单
------------------------
EOF
read -p "请输入编号:" num
if [ -z "$num" ]; then
menu
fi
case $num in
    0)
    FLASH_TYPE=0
    menu
    ;;
    1)
    FLASH_TYPE=1
    menu
    ;;
    x|X)
    menu
    ;;
    *)
    echo -e "请输入正确编号"
    set_flash_type
esac
}

function build_secureboot_file()
{
cat<<EOF
########################
    创建安全启动文件
########################
EOF

set -x
if [ $SECOND_SIGN = "1" ];then                                                  #二级loader
    cd $DIR_UBOOT
    make clean
    make ak3918av100_nor_secure_defconfig
    make DEVICE_TREE=$UBOOT_DEV_TREE -j8 CROSS_COMPILE=arm-anycloud-linux-uclibcgnueabi-
    cd -
    export PATH=$DIR_UBOOT/tools:$DIR_UBOOT/scripts/dtc:$PATH
    which mkimage
    which dtc
    mkdir -p ./keys
    openssl req -batch -new -x509 -key $RSA_FILE_PRIVATEKEY -out ./keys/dev.crt
    cp $RSA_FILE_PRIVATEKEY ./keys/dev.key

    UBOOT_DTB=$DIR_UBOOT/u-boot.dtb
    UBOOT_DATA=$DIR_UBOOT/u-boot-nodtb.bin
    UBOOT_BIN=./u-boot-spl.bin
    UIMAGE_SECURE=$DIR_BURNTOOL/uImage-secure
    CLOUDOS_DTB_SECURE=$DIR_BURNTOOL/cloudOS-secure.dtb
    ROOT_SECURE=$DIR_BURNTOOL/root-secure.sqsh4

    rm -f $UIMAGE_SECURE $CLOUDOS_DTB_SECURE $ROOT_SECURE $UBOOT_BIN
    mkimage -D "-I dts -O dtb -p 2000" -f uImage.its  -K $UBOOT_DTB -k keys -r $UIMAGE_SECURE -E -p 0x1000
    cat $UBOOT_DATA $UBOOT_DTB > $UBOOT_BIN

    mkimage -D "-I dts -O dtb -p 2000" -f cloudOS.its -K $UBOOT_DTB -k keys -r $CLOUDOS_DTB_SECURE
    mkimage -D "-I dts -O dtb -p 2000" -f root.its    -K $UBOOT_DTB -k keys -r $ROOT_SECURE -E -p 0x1000

    #升级分区偏移值
    ./ak_env_key --dev $ENV_IMG_SECURE \
    --insert "mtdparts=mtdparts=spi0.0:256K@0x0(UBOOT),4K@0x40000(ENV),4K@0x41000(ENVBK),64K@0x42000(DTB),1600K@0x52000(KERNEL),2200K@0x1E2000(ROOTFS),400K@0x408000(CONFIG),3664K@0x46C000(APP),2196K@0x1E3000(ROOTFS_MOUNT)"
    #升级内核启动位置
    ./ak_env_key --dev $ENV_IMG_SECURE --insert "loadaddr=0x81000000"
    #升级根分区编号
    ./ak_env_key --dev $ENV_IMG_SECURE --insert "mtd_root=/dev/mtdblock8"
    #升级启动参数
    ./ak_env_key --browser --dev $ENV_IMG_SECURE \
    --insert "boot_normal=env set bootargs console=ttySAK0,115200n8 root=/dev/mtdblock8 rootfstype=squashfs init=/sbin/init \${mtdparts} \${mem} \${memsize}; run read_kernel; run read_dtb;sf read 81800000 1e2000 226000;bootm \${loadaddr} 81800000 \${fdtcontroladdr}"
fi

rm -f $DIR_BURNTOOL/config_*_secure.txt
BURNTOOL_CONF_SECURE=`echo $BURNTOOL_CONF | sed 's#\..*$#_secure.txt#g'` #创建burntool的安全烧写配置文件名称
echo "iconv -f UTF-16LE -t UTF-8 $DIR_BURNTOOL/$BURNTOOL_CONF > $DIR_BURNTOOL/$BURNTOOL_CONF_SECURE"
iconv -f UTF-16LE -t UTF-8 $DIR_BURNTOOL/$BURNTOOL_CONF > $DIR_BURNTOOL/$BURNTOOL_CONF_SECURE #转为文件格式
FILE_UBOOT_UPDATE=`cat $DIR_BURNTOOL/$BURNTOOL_CONF_SECURE | grep -E "download_to_nand1[ ]*=" | awk -F'[ ,]+' '{print $3}'`
FILE_UBOOT_SECURE=`echo $FILE_UBOOT_UPDATE | sed 's#\..*$#_secure.bin#g'`

sed -i "s/path producer = .*/path producer = $PRODUCER_SECURE/" $DIR_BURNTOOL/$BURNTOOL_CONF_SECURE
sed -i "s/path 1k data = .*/path 1k data = $SIGN_SECURE/" $DIR_BURNTOOL/$BURNTOOL_CONF_SECURE
sed -i "s/boot mode flag = .*/boot mode flag = 2/" $DIR_BURNTOOL/$BURNTOOL_CONF_SECURE
PARTITION_KEY=`cat $DIR_BURNTOOL/$BURNTOOL_CONF_SECURE | grep -E "download_to_nand1[ ]*="`
PARTITION_KEY_REPLACE=`echo $PARTITION_KEY | sed "s#$FILE_UBOOT_UPDATE#$FILE_UBOOT_SECURE#g"`
sed -i "s/$PARTITION_KEY/$PARTITION_KEY_REPLACE/" $DIR_BURNTOOL/$BURNTOOL_CONF_SECURE

iconv -f UTF-8 -t UTF-16LE $DIR_BURNTOOL/$BURNTOOL_CONF_SECURE > $DIR_BURNTOOL/$BURNTOOL_CONF_SECURE.tmp #转为文件格式
mv $DIR_BURNTOOL/$BURNTOOL_CONF_SECURE.tmp $DIR_BURNTOOL/$BURNTOOL_CONF_SECURE


CMD_SECUREBOOT="./ak_secureboot \
--hex-aes-key $AES_KEY \
--hex-aes-iv $AES_IV \
--burn-level $BURN_LEVEL \
--sector $DDR_PARAM \
--burn-type $BURN_TYPE \
--baud $BAUD \
--flash $FLASH_TYPE \
--chip-type 1 \
--file-private-key $RSA_FILE_PRIVATEKEY \
--file-ddr-ini $DIR_BURNTOOL/SDK_CPU_DDR2.ini \
--file-sign-secure $DIR_BURNTOOL/$SIGN_SECURE \
--file-uboot $DIR_BURNTOOL/$FILE_UBOOT_UPDATE \
--file-uboot-secure $DIR_BURNTOOL/$FILE_UBOOT_SECURE \
--file-producer $DIR_BURNTOOL/producer_39xx.bin \
--file-producer-secure $DIR_BURNTOOL/$PRODUCER_SECURE"

if [ -n "$ID" ];then
    CMD_SECUREBOOT=$CMD_SECUREBOOT" --hex-id $ID"
fi
$CMD_SECUREBOOT

DIR_UPDATE4SCRIPT=`pwd`/update4script
rm -rf $DIR_UPDATE4SCRIPT
mkdir -p $DIR_UPDATE4SCRIPT
cp $DIR_BURNTOOL/$FILE_UBOOT_SECURE       $DIR_UPDATE4SCRIPT/$FILE_UBOOT_UPDATE
if [ $SECOND_SIGN = "1" ];then
    cp $ENV_IMG_SECURE      $DIR_UPDATE4SCRIPT/env.img
    cp $CLOUDOS_DTB_SECURE  $DIR_UPDATE4SCRIPT/cloudOS.dtb
    cp $UIMAGE_SECURE       $DIR_UPDATE4SCRIPT/uImage
    cp $ROOT_SECURE         $DIR_UPDATE4SCRIPT/root.sqsh4
fi

set +x
menu
}

save_secureboot_conf()
{
    echo BURN_LEVEL=$BURN_LEVEL > $FILE_CONF
    echo BURN_TYPE=$BURN_TYPE >> $FILE_CONF
    echo BAUD=$BAUD >> $FILE_CONF
    echo ID=$ID >> $FILE_CONF
    echo DDR_PARAM=$DDR_PARAM >> $FILE_CONF
    echo AES_KEY=$AES_KEY >> $FILE_CONF
    echo AES_IV=$AES_IV >> $FILE_CONF
    echo RSA_FILE_PRIVATEKEY=$RSA_FILE_PRIVATEKEY >> $FILE_CONF
    echo SECOND_SIGN=$SECOND_SIGN >> $FILE_CONF
    echo DIR_OS=$DIR_OS >> $FILE_CONF
    echo BURNTOOL_CONF=$BURNTOOL_CONF >> $FILE_CONF
    echo UBOOT_DEV_TREE=$UBOOT_DEV_TREE >> $FILE_CONF
    echo FLASH_TYPE=$FLASH_TYPE >> $FILE_CONF
    echo -e "\033[32m当前配置已经保存到:$FILE_CONF\033[0m"
    menu
}
menu