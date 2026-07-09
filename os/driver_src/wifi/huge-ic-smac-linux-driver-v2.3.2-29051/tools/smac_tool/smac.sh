#! /bin/sh

mkdir -p /var/run/hostapd
mkdir -p /var/run/wpa_supplicant

#interface name
IFNAME="wlan0"

#driver param
DFLAG=

#ko file path
SMAC_KO_PATH="/ko/hgics.ko"

HGIC_2G=`nvram_get hgic_2g`
IF_USB=`nvram_get if_usb`
TXQ_SIZE=`nvram_get txq_size`

#read parameters from system.
AH_MODE=`nvram_get ah_mode`
AH_SSID=`nvram_get ah_ssid`
AH_BRIDGE=`nvram_get ah_bridge`
AH_BEACON=`nvram_get ah_beacon`
AH_PV1=`nvram_get ah_pv1`
AH_MAC=`nvram_get ah_mac`
AH_IP=`nvram_get ah_ip`
AH_FMAC=`nvram_get ah_fmac`
AH_PSK=`nvram_get ah_psk`
AH_KEY_MGMT=`nvram_get ah_key_mgmt`
AH_FREQ_START=`nvram_get ah_freq_start`
AH_FREQ_END=`nvram_get ah_freq_end`
AH_BSS_BW=`nvram_get ah_bss_bw`
AH_CHAN_LIST=`nvram_get ah_chan_list`
AH_TX_BW=`nvram_get ah_tx_bw`
AH_TX_MCS=`nvram_get ah_tx_mcs`
AH_PRIMARY_CHAN=`nvram_get ah_primary_chan`
AH_CHANNEL=`nvram_get ah_channel`
AH_DEBUG=`nvram_get ah_debug`
AH_ACS=`nvram_get ah_acs`
AH_ACS_TM=`nvram_get ah_acs_tm`
SDIO_FC=`nvram_get sdio_fc`
AH_BGRSSI=`nvram_get ah_bgrssi`
AH_TX_POWER=`nvram_get ah_tx_power`
RX_REORDER=`nvram_get ah_rx_reorder`
AP_MAX_INACTIVITY=`nvram_get ah_ap_max_inactivity`
HGICS_DACK=`nvram_get hgics_dack`
WMM_ONLY_PARAM=`nvram_get wmm_only_param`
WMM_ENABLED=`nvram_get ah_wmm_capable`
WMM_AC_BK_AIFS=`nvram_get wmm_ac_bk_aifs`
WMM_AC_BK_CWMIN=`nvram_get wmm_ac_bk_cwmin`
WMM_AC_BK_CWMAX=`nvram_get wmm_ac_bk_cwmax`
WMM_AC_BK_TXOP=`nvram_get wmm_ac_bk_txop_limit`
WMM_AC_BK_ACM=`nvram_get wmm_ac_bk_acm`
WMM_AC_BE_AIFS=`nvram_get wmm_ac_be_aifs`
WMM_AC_BE_CWMIN=`nvram_get wmm_ac_be_cwmin`
WMM_AC_BE_CWMAX=`nvram_get wmm_ac_be_cwmax`
WMM_AC_BE_TXOP=`nvram_get wmm_ac_be_txop_limit`
WMM_AC_BE_ACM=`nvram_get wmm_ac_be_acm`
WMM_AC_VI_AIFS=`nvram_get wmm_ac_vi_aifs`
WMM_AC_VI_CWMIN=`nvram_get wmm_ac_vi_cwmin`
WMM_AC_VI_CWMAX=`nvram_get wmm_ac_vi_cwmax`
WMM_AC_VI_TXOP=`nvram_get wmm_ac_vi_txop_limit`
WMM_AC_VI_ACM=`nvram_get wmm_ac_vi_acm`
WMM_AC_VO_AIFS=`nvram_get wmm_ac_vo_aifs`
WMM_AC_VO_CWMIN=`nvram_get wmm_ac_vo_cwmin`
WMM_AC_VO_CWMAX=`nvram_get wmm_ac_vo_cwmax`
WMM_AC_VO_TXOP=`nvram_get wmm_ac_vo_txop_limit`
WMM_AC_VO_ACM=`nvram_get wmm_ac_vo_acm`
WMM_AUTO=`nvram_get wmm_auto`
WMM_PARAM1=`nvram_get wmm_param1`
WMM_PARAM2=`nvram_get wmm_param2`
WMM_PARAM4=`nvram_get wmm_param4`
WMM_PARAM8=`nvram_get wmm_param8`
WMM_PARAM16=`nvram_get wmm_param16`
PAIRED_STAS=`nvram_get paired_stas`
IF_TEST=`nvram_get if_test`
NOBOOTDL=`nvram_get nobootdl`
PMF_EN=`nvram_get pmf_en`
IF_AGG=`nvram_get if_agg`
AP_11N=`nvram_get ap_11n`
AH_11W=`nvram_get ah_11w`

TFTP_SVR=`nvram_get tftp_svr`

tftp_download_fw()
{
	if [ -n "$TFTP_SVR" ]; then
		echo "start download $1 from $TFTP_SVR ..."
		tftp -g -l /lib/firmware/$1 -r $1 $TFTP_SVR
	fi
}

if [ -n "$TFTP_SVR" ]; then
	echo "start download hgics.ko from $TFTP_SVR ..."
	tftp -g -l $SMAC_KO_PATH -r hgics.ko $TFTP_SVR
	#tftp -g -l /ko/hgics.ko -r hgics.ko 10.10.10.3
fi

if [ x"$IF_USB" == "x1" ]; then
	tftp_download_fw hgics_usb.bin
	KOARGS="fw_file=hgics_usb.bin"
else
	tftp_download_fw hgics_usb.bin
	KOARGS="fw_file=hgics_sdio.bin"
fi

[ -n "$IF_TEST" ]  && KOARGS="$KOARGS if_test=$IF_TEST"
[ -n "$QC_MODE" ]  && KOARGS="$KOARGS qc_mode=$QC_MODE"
[ -n "$NOBOOTDL" ] && KOARGS="$KOARGS no_bootdl=$NOBOOTDL"
[ -n "$IF_AGG" ]   && KOARGS="$KOARGS if_agg=$IF_AGG"
[ -n "$TXQ_SIZE" ] && KOARGS="$KOARGS txq_size=$TXQ_SIZE"

#set default values
s1g_1M_rates="300 600 900 1200 1800 2400 2700 3000 3600 4000"
s1g_2M_rates="650 1300 1950 2600 3900 5200 5850 6500 7800"
s1g_4M_rates="1350 2700 4050 5400 8100 10800 12150 13500 16200 18000"
s1g_8M_rates="2925 5850 8775 11700 17550 23400 26325 29250 35100 39000"
[ -z "$AH_MODE" ] && AH_MODE="ap"
[ -z "$AH_BEACON" ] && AH_BEACON="100"
[ -z "$AH_PV1" ] && AH_PV1="1"
[ -z "$AH_KEY_MGMT" ] && AH_KEY_MGMT="NONE"
[ -z "$AH_FREQ_START" ] && AH_FREQ_START="7800"
[ -z "$AH_FREQ_END" ] && AH_FREQ_END="8000"
[ -z "$AH_BSS_BW" ] && AH_BSS_BW="8"
[ -z "$AH_TX_BW" ] && AH_TX_BW="8"
[ -z "$AH_TX_MCS" ] && AH_TX_MCS="7"
[ -z "$AH_PRIMARY_CHAN" ] && AH_PRIMARY_CHAN="3"
[ -z "$AH_CHANNEL" ] && AH_CHANNEL="1"
[ -z "$AH_ACS" ] && AH_ACS="0"
[ -z "$AH_BGRSSI" ] && AH_BGRSSI="0"
[ -z "$AH_ACS_TM" ] && AH_ACS_TM="10"
[ "$AH_MODE" == "sta" ] && AH_ACS="0"
[ "$AH_ACS" == "1" ] && AH_CHANNEL="1"
[ -z "$AP_MAX_INACTIVITY" ] && AP_MAX_INACTIVITY="300"
[ -z "$WMM_ONLY_PARAM" ] && WMM_ONLY_PARAM="1"
[ -z "$WMM_ENABLED" ] && WMM_ENABLED="0"
[ -z "$WMM_AC_VO_AIFS" ] && WMM_AC_VO_AIFS="2"
[ -z "$WMM_AC_VO_CWMIN" ] && WMM_AC_VO_CWMIN="5"
[ -z "$WMM_AC_VO_CWMAX" ] && WMM_AC_VO_CWMAX="6"
[ -z "$WMM_AC_VO_ACM" ] && WMM_AC_VO_ACM="0"
[ -z "$WMM_AC_VO_TXOP" ] && WMM_AC_VO_TXOP="21"
[ -z "$WMM_AC_VI_AIFS" ] && WMM_AC_VI_AIFS="2"
[ -z "$WMM_AC_VI_CWMIN" ] && WMM_AC_VI_CWMIN="6"
[ -z "$WMM_AC_VI_CWMAX" ] && WMM_AC_VI_CWMAX="7"
[ -z "$WMM_AC_VI_ACM" ] && WMM_AC_VI_ACM="0"
[ -z "$WMM_AC_VI_TXOP" ] && WMM_AC_VI_TXOP="21"
[ -z "$WMM_AC_BE_AIFS" ] && WMM_AC_BE_AIFS="3"
[ -z "$WMM_AC_BE_CWMIN" ] && WMM_AC_BE_CWMIN="7"
[ -z "$WMM_AC_BE_CWMAX" ] && WMM_AC_BE_CWMAX="7"
[ -z "$WMM_AC_BE_ACM" ] && WMM_AC_BE_ACM="0"
[ -z "$WMM_AC_BE_TXOP" ] && WMM_AC_BE_TXOP="0"
[ -z "$WMM_AC_BK_AIFS" ] && WMM_AC_BK_AIFS="7"
[ -z "$WMM_AC_BK_CWMIN" ] && WMM_AC_BK_CWMIN="7"
[ -z "$WMM_AC_BK_CWMAX" ] && WMM_AC_BK_CWMAX="7"
[ -z "$WMM_AC_BK_ACM" ] && WMM_AC_BK_ACM="0"
[ -z "$WMM_AC_BK_TXOP" ] && WMM_AC_BK_TXOP="0"
[ "$WMM_ENABLED" == "1" ] && WMM_ONLY_PARAM="0"
[ -z "$WMM_AUTO" ] && WMM_AUTO=1
[ -z "$WMM_PARAM1" ] && WMM_PARAM1="0,4,7,3;0,4,7,7;21,3,7,2;21,2,6,2"
[ -z "$WMM_PARAM2" ] && WMM_PARAM2="0,6,7,3;2,6,7,7;21,5,7,2;21,4,6,2"
[ -z "$WMM_PARAM4" ] && WMM_PARAM4="0,7,7,3;2,7,7,7;21,6,7,2;21,5,6,2"
[ -z "$WMM_PARAM8" ] && WMM_PARAM8="0,8,8,3;2,8,8,7;21,7,8,2;21,6,7,2"
[ -z "$WMM_PARAM16" ] && WMM_PARAM16="0,9,9,3;2,9,9,7;21,8,9,2;21,7,8,2"
[ -z "$PMF_EN" ] && PMF_EN=0
[ -z "$AP_11N" ] && AP_11N=1
[ -z "$AH_11W" ] && AH_11W=0

get_wpa_psk()
{
	psk_list=$(wpa_passphrase $1 $2|grep psk=)
	for item in $psk_list; do
		psk=$item
	done
	echo ${psk#*=}
}

create_wlan1()
{
	wlan1=$(ifconfig -a|grep wlan1)
	if [ -z "$wlan1" ]; then
		phy=$(iw phy|grep Wiphy)
		phy=${phy#* }
		iw phy $phy interface add wlan1 type station
	fi
}

gen_AH_config()
{

#driver config file
cat > /etc/hgics.conf <<EOF
freq_start=$AH_FREQ_START
freq_end=$AH_FREQ_END
bss_bw=$AH_BSS_BW
tx_bw=$AH_TX_BW
tx_mcs=$AH_TX_MCS
acs=$AH_ACS
acs_tm=$AH_ACS_TM
primary_chan=$AH_PRIMARY_CHAN
chan_list=$AH_CHAN_LIST
EOF

#####
cat >> /etc/hostapd.conf <<EOF
hw_mode=ah
ieee80211ah=1
s1g_compatibility=0
s1g_supported_chan_width=$AH_BSS_BW
s1g_oper_chwidth=$AH_BSS_BW
rx_s1g_mcs_map=0
s1g_rx_supported_long_gi_data_rate=0
tx_s1g_mcs_map=0
s1g_tx_supported_long_gi_data_rate=0
s1g_rx_mcs_1mhz=0
s1g_tx_mcs_1mhz=0
require_s1g=1
#compressed_ssid=test
s1g_operating_class=62
s1g_pri_chan=$AH_PRIMARY_CHAN
s1g_chan_center_freq=$AH_CHANNEL
s1g_use_sta_nsts=0
s1g_basic_mcs_nsts=0
s1g_full_beacon_int=500
dot11S1GOptionImplemented=1
s1g_unscaled_interval=100
s1g_unified_scaling_factor=0
s1g_aid_switch_count=2
eapol_key_index_workaround=0
s1g_freq_start=$AH_FREQ_START
s1g_freq_end=$AH_FREQ_END
s1g_chan_bw=$AH_BSS_BW
ap_max_inactivity=$AP_MAX_INACTIVITY
s1g_chan_list=$AH_CHAN_LIST
wmm_only_param=$WMM_ONLY_PARAM
edca_auto=$WMM_AUTO
edca_auto_1=$WMM_PARAM1
edca_auto_2=$WMM_PARAM2
edca_auto_4=$WMM_PARAM4
edca_auto_8=$WMM_PARAM8
edca_auto_16=$WMM_PARAM16
EOF

if [ x"$AH_PV1" == "x1" ]; then
	echo "s1g_capab=[MAX-MPDU-7991] [S1G-LONG] [SUPPORTED-BW-0] [Rx-LDPC] [TX-STBC] [RX-STBC] [PV1-FRAME-SUPPORT]" >> /etc/hostapd.conf
	echo "s1g_pv1_support=1" >> /etc/hostapd.conf
else
	echo "s1g_capab=[MAX-MPDU-7991] [S1G-LONG] [SUPPORTED-BW-0] [Rx-LDPC] [TX-STBC] [RX-STBC]" >> /etc/hostapd.conf
fi

if [ $AH_BSS_BW == "1" ]; then
	echo "supported_rates=$s1g_1M_rates" >> /etc/hostapd.conf
	echo "basic_rates=$s1g_1M_rates" >> /etc/hostapd.conf
elif [ $AH_BSS_BW == "2" ];then
	echo "supported_rates=$s1g_2M_rates" >> /etc/hostapd.conf
	echo "basic_rates=$s1g_2M_rates" >> /etc/hostapd.conf
elif [ $AH_BSS_BW == "4" ];then
	echo "supported_rates=$s1g_4M_rates" >> /etc/hostapd.conf
	echo "basic_rates=$s1g_4M_rates" >> /etc/hostapd.conf
elif [ $AH_BSS_BW == "8" ];then
	echo "supported_rates=$s1g_8M_rates" >> /etc/hostapd.conf
	echo "basic_rates=$s1g_8M_rates" >> /etc/hostapd.conf
fi

cat >> /etc/wpas.conf <<EOF
s1g_capab=[MAX-MPDU-7991] [S1G-LONG] [SUPPORTED-BW-0] [Rx-LDPC] [TX-STBC] [RX-STBC]
dot11S1GOptionImplemented=1
rx_s1g_mcs_map=0
s1g_rx_supported_long_gi_data_rate=0
tx_s1g_mcs_map=0
s1g_tx_supported_long_gi_data_rate=0
s1g_rx_mcs_1mhz=0
s1g_tx_mcs_1mhz=0
s1g_pv1_support=1
s1g_freq_start=$AH_FREQ_START
s1g_freq_end=$AH_FREQ_END
s1g_chan_bw=$AH_BSS_BW
s1g_chan_list=$AH_CHAN_LIST
EOF

if [ x"$AH_BRIDGE" == "x1" ]; then
	echo "bridge=br0" >> /etc/wpas.conf
fi

}

gen_2G_config()
{
cat >> /etc/hostapd.conf <<EOF
require_ht=0
ieee80211n=$AP_11N
hw_mode=g
EOF

}

###############################################
###############################################
#generate config files
rm -f /etc/hgics.conf
rm -f /etc/hostapd.conf
rm -f /etc/wpas.conf

#AP config file
cat > /etc/hostapd.conf <<EOF
interface=$IFNAME
driver=nl80211
logger_syslog=-1
logger_syslog_level=2
logger_stdout=-1
logger_stdout_level=2
ctrl_interface=/var/run/hostapd
ctrl_interface_group=0
country_code=CN
ssid=$AH_SSID
dtim_period=10
beacon_int=$AH_BEACON
channel=$AH_CHANNEL
max_num_sta=255
rts_threshold=-1
fragm_threshold=-1
macaddr_acl=0
auth_algs=3
ignore_broadcast_ssid=0
wmm_enabled=$WMM_ENABLED
wmm_ac_bk_cwmin=$WMM_AC_BK_CWMIN
wmm_ac_bk_cwmax=$WMM_AC_BK_CWMAX
wmm_ac_bk_aifs=$WMM_AC_BK_AIFS
wmm_ac_bk_txop_limit=$WMM_AC_BK_TXOP
wmm_ac_bk_acm=$WMM_AC_BK_ACM
wmm_ac_be_aifs=$WMM_AC_BE_AIFS
wmm_ac_be_cwmin=$WMM_AC_BE_CWMIN
wmm_ac_be_cwmax=$WMM_AC_BE_CWMAX
wmm_ac_be_txop_limit=$WMM_AC_BE_TXOP
wmm_ac_be_acm=$WMM_AC_BE_ACM
wmm_ac_vi_aifs=$WMM_AC_VI_AIFS
wmm_ac_vi_cwmin=$WMM_AC_VI_CWMIN
wmm_ac_vi_cwmax=$WMM_AC_VI_CWMAX
wmm_ac_vi_txop_limit=$WMM_AC_VI_TXOP
wmm_ac_vi_acm=$WMM_AC_VI_ACM
wmm_ac_vo_aifs=$WMM_AC_VO_AIFS
wmm_ac_vo_cwmin=$WMM_AC_VO_CWMIN
wmm_ac_vo_cwmax=$WMM_AC_VO_CWMAX
wmm_ac_vo_txop_limit=$WMM_AC_VO_TXOP
wmm_ac_vo_acm=$WMM_AC_VO_ACM
EOF

##wpa_supplicant config file
cat > /etc/wpas.conf <<EOF
ctrl_interface=/var/run/wpa_supplicant
pmf=$PMF_EN
eapol_version=1
update_config=1
ap_scan=1
EOF


#############################################
if [ -z "$HGIC_2G" ];then
	DFLAG="-H "
	gen_AH_config
else
	gen_2G_config
fi

if [ "x$AH_11W" == "x1" ];then
	echo "ieee80211w=$PMF_EN" >> /etc/hostapd.conf
fi

if [ x"$AH_BRIDGE" == "x1" ]; then
	echo "bridge=br0" >> /etc/hostapd.conf
fi

if [ x"$AH_KEY_MGMT" == "xWPA-PSK" ]; then
	echo "wpa=2" >> /etc/hostapd.conf
	echo "wpa_key_mgmt=WPA-PSK" >> /etc/hostapd.conf
	echo "wpa_pairwise=CCMP" >> /etc/hostapd.conf
	echo "rsn_pairwise=CCMP" >> /etc/hostapd.conf
	echo "wpa_psk=$(get_wpa_psk $AH_SSID $AH_PSK)" >> /etc/hostapd.conf	
fi

echo "network={" >> /etc/wpas.conf
echo "    ssid=\"$AH_SSID\"" >> /etc/wpas.conf
if [ x"$AH_KEY_MGMT" == "xWPA-PSK" ]; then
echo "    proto=WPA RSN" >> /etc/wpas.conf
echo "    key_mgmt=WPA-PSK WPA-PSK-SHA256" >> /etc/wpas.conf
echo "    pairwise=CCMP TKIP" >> /etc/wpas.conf
echo "    group=CCMP TKIP" >> /etc/wpas.conf
echo "    psk=\"$AH_PSK\"" >> /etc/wpas.conf
echo "    priority=5" >> /etc/wpas.conf
else
echo "    key_mgmt=NONE" >> /etc/wpas.conf
fi
echo "}" >> /etc/wpas.conf


##########################################################
##########################################################
##########################################################
killall hostapd
killall wpa_supplicant
killall hgics

#load driver
ko_exist=$(lsmod|grep hgics)
if [ -z "$ko_exist" ]; then
	rmmod hgicf
	insmod $SMAC_KO_PATH $KOARGS
	sleep 2
fi

[ x"$AH_BRIDGE" == "x1" ] && BRARG="-bbr0"

ifconfig $IFNAME down
brctl delif br0 $IFNAME

if [ -n "$AH_MAC" ]; then
	ifconfig $IFNAME down
	ifconfig $IFNAME hw ether $AH_MAC
	ifconfig $IFNAME up
fi

if [ x"$AH_MODE" == "xap" ]; then
	### start ap
	hostapd $DFLAG /etc/hostapd.conf &
else
	### start sta
	wpa_supplicant $DFLAG -Dnl80211 -i$IFNAME $BRARG -c /etc/wpas.conf &
	create_wlan1
fi

[ "$AH_BSS_BW" == "1" ] && ifconfig $IFNAME mtu 380

if [ x"$AH_BRIDGE" == "x1" ]; then
	ifconfig $IFNAME 0.0.0.0
	brctl addif br0 $IFNAME
else
	ifconfig $IFNAME $AH_IP netmask 255.255.255.0
fi

hgics $AH_MODE $AH_SSID &
