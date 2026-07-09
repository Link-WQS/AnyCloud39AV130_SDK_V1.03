/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: wal iwconfig & iwpriv api.
 * Create: 2021-07-16
 */

#include "wal_linux_iwhandler.h"
#include "hmac_vap.h"
#include "wal_linux_util.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_ccpriv.h"
#include "wlan_msg.h"
#ifdef _PRE_WLAN_WIRELESS_EXT
#include "oal_list.h"
#include "wal_linux_iwlist.h"
#include "wal_linux_scan.h"
#endif
#include "wal_linux_ccpriv.h"
#include "wal_linux_netdev.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_IW_C
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define DIEID_EXTRA_LEN 300

#ifdef _PRE_WLAN_WIRELESS_EXT
#define WAIT_HMAC_RSP_TIME 5000
static wal_hmac_wait_sync_stru g_wal_hmac_wait_sync[WAIT_SYNC_MODE_BUTT];
#endif

OAL_STATIC osal_s32 iwpriv_get_dscp_tid_map(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_void *p_param, osal_s8 *pc_extra);
OAL_STATIC osal_s32  iwpriv_get_dieid(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_void *param, osal_s8 *extra);

/*****************************************************************************
 功能描述  : 获得无线的名字, 用来确认无限扩展的存在.
 输入参数  : net_dev: 指向net_device的指针
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年2月21日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_iwconfig_get_iwname(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_s8 *pc_name, osal_s8 *pc_extra)
{
    osal_s8    ac_iwname[] = "IEEE 802.11";

    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);
    if ((OAL_PTR_NULL == net_dev) || (OAL_PTR_NULL == pc_name)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_iwconfig_get_iwname::param null, net_dev = %p, pc_name = %p.}",
            (uintptr_t)net_dev, (uintptr_t)pc_name);
        return -OAL_EINVAL;
    }

    if (memcpy_s(pc_name, OAL_SIZEOF(ac_iwname), ac_iwname, OAL_SIZEOF(ac_iwname)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_iwconfig_get_iwname::memcpy_s error}");
    }

    return OAL_SUCC;
}

/*****************************************************************************
  协议模式字符串定义
*****************************************************************************/
OAL_CONST wal_ioctl_mode_map_stru g_ast_mode_map_etc[] = {
    /* legacy */
    {"11a",                 WLAN_LEGACY_11A_MODE,       WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11b",                 WLAN_LEGACY_11B_MODE,       WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11bg",                WLAN_MIXED_ONE_11G_MODE,    WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},

    {"11g2g5",              WLAN_MIXED_TWO_11G_MODE,    WLAN_BAND_2G,   WLAN_BAND_WIDTH_5M},
    {"11g2g10",             WLAN_MIXED_TWO_11G_MODE,    WLAN_BAND_2G,   WLAN_BAND_WIDTH_10M},
    {"11g2g20",             WLAN_MIXED_TWO_11G_MODE,    WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},

    {"11g5g40plus",         WLAN_MIXED_TWO_11G_MODE,    WLAN_BAND_5G,   WLAN_BAND_WIDTH_40PLUS},
    {"11g5g80plusplus",     WLAN_MIXED_TWO_11G_MODE,    WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSPLUS},

    /* 11n */
    {"11n2g5",              WLAN_HT_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_5M},
    {"11n2g10",             WLAN_HT_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_10M},
    {"11n2g20",             WLAN_HT_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11n2g40",             WLAN_HT_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_40M},
    {"11n2g40plus",         WLAN_HT_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_40PLUS},
    {"11n2g40minus",        WLAN_HT_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_40MINUS},

    {"11n5g5",              WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_5M},
    {"11n5g10",             WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_10M},
    {"11n5g20",             WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11n5g40",             WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_40M},
    {"11n5g40plus",         WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_40PLUS},
    {"11n5g40minus",        WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_40MINUS},
    {"11n5g80plusplus",     WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSPLUS},
    {"11n5g80plusminus",    WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSMINUS},
    {"11n5g80minusplus",    WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSPLUS},
    {"11n5g80minusminus",   WLAN_HT_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSMINUS},

    /* 11ac */
    {"11ac5",               WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_5M},
    {"11ac10",              WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_10M},
    {"11ac20",              WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11ac40",              WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_40M},
    {"11ac40plus",          WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ac40minus",         WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_40MINUS},
    {"11ac80",              WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_80M},
    {"11ac80plusplus",      WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSPLUS},
    {"11ac80plusminus",     WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSMINUS},
    {"11ac80minusplus",     WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSPLUS},
    {"11ac80minusminus",    WLAN_VHT_MODE,              WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSMINUS},

    /* 协议上不支持11ac 2.4G */
    {"11ac2g5",             WLAN_VHT_MODE,              WLAN_BAND_2G,   WLAN_BAND_WIDTH_5M},
    {"11ac2g10",            WLAN_VHT_MODE,              WLAN_BAND_2G,   WLAN_BAND_WIDTH_10M},
    {"11ac2g20",            WLAN_VHT_MODE,              WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11ac2g40",            WLAN_VHT_MODE,              WLAN_BAND_2G,   WLAN_BAND_WIDTH_40M},
    {"11ac2g40plus",        WLAN_VHT_MODE,              WLAN_BAND_2G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ac2g40minus",       WLAN_VHT_MODE,              WLAN_BAND_2G,   WLAN_BAND_WIDTH_40MINUS},

    /* 11n only and 11ac only, 都是20M带宽 */
    {"11nonly2g",           WLAN_HT_ONLY_MODE,   WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11nonly5g",           WLAN_HT_ONLY_MODE,   WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11aconly",            WLAN_VHT_ONLY_MODE,  WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},

#ifdef _PRE_WLAN_FEATURE_11AX
    /* 11ax */
    {"11ax2g20",            WLAN_HE_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_20M},
    {"11ax2g40",            WLAN_HE_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_40M},
    {"11ax2g40plus",        WLAN_HE_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ax2g40minus",       WLAN_HE_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_40MINUS},

    {"11ax5g20",            WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_20M},
    {"11ax5g40",            WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_40M},
    {"11ax5g40plus",        WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_40PLUS},
    {"11ax5g40minus",       WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_40MINUS},
    {"11ax5g80",            WLAN_HE_MODE,               WLAN_BAND_2G,   WLAN_BAND_WIDTH_80M},
    {"11ax5g80plusplus",    WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSPLUS},
    {"11ax5g80plusminus",   WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80PLUSMINUS},
    {"11ax5g80minusplus",   WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSPLUS},
    {"11ax5g80minusminus",  WLAN_HE_MODE,               WLAN_BAND_5G,   WLAN_BAND_WIDTH_80MINUSMINUS},
#endif

    {OAL_PTR_NULL}
};


/*****************************************************************************
 功能描述  : 设置模式: 包括协议、频段、带宽
 输入参数  : net_dev: net device
             p_param    : 参数
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2012年12月17日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_iwpriv_set_mode(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    void *p_param, osal_s8 *pc_extra)
{
    oal_iw_point_stru          *param;
    osal_s32                   l_ret;
    osal_s8                    ac_mode_str[24] = {0};     /* 预留协议模式字符串空间 */

    wal_record_wifi_external_log(WLAN_WIFI_IW_PRIV_HANDLERS, __func__);
    if (OAL_UNLIKELY((OAL_PTR_NULL == net_dev) || (OAL_PTR_NULL == p_param))) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_iwpriv_set_mode::net_dev(%p) or param(%p) is null}",
            (uintptr_t)net_dev, (uintptr_t)p_param);
        return -OAL_EFAUL;
    }

    l_ret = wal_get_init_state_vap(net_dev, OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_iwpriv_set_mode::wal_get_init_state_vap err}");
        return l_ret;
    }

    param = (oal_iw_point_stru *)p_param;
    oam_info_log1(0, OAM_SF_ANY, "{uapi_iwpriv_set_mode::input str length is %d!}", param->length);

    /* 字符串切断, 长度包括\0 */
    param->length = (param->length > OAL_SIZEOF(ac_mode_str)) ? OAL_SIZEOF(ac_mode_str) : param->length;

    /* copy_from_user函数的目的是从用户空间拷贝数据到内核空间，失败返回没有被拷贝的字节数，成功返回0 */
    if (osal_copy_from_user(ac_mode_str, param->pointer, param->length) > 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_iwpriv_set_mode::osal_copy_from_user return error}");
        return -OAL_EFAUL;
    }

    ac_mode_str[OAL_SIZEOF(ac_mode_str) - 1] = '\0';    /* 确保以null结尾 */
    l_ret = wal_set_mode(net_dev, g_ast_mode_map_etc, (const osal_s8 *)ac_mode_str);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwpriv_set_mode::wal_set_mode return err code %d!}", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置频点/信道
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年1月17日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_iwconfig_set_freq(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iw_freq_stru *freq, osal_s8 *pc_extra)
{
    osal_s32                   l_ret;

    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);
    if ((OAL_PTR_NULL == net_dev) || (OAL_PTR_NULL == freq)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_iwconfig_set_freq::param null, net_dev = %p, freq =%p.}",
            (uintptr_t)net_dev, (uintptr_t)freq);
        return -OAL_EINVAL;
    }

    l_ret = wal_get_init_state_vap(net_dev, OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_iwconfig_set_freq::wal_get_init_state_vap err}");
        return l_ret;
    }

    oam_info_log4(0, OAM_SF_ANY, "{uapi_iwconfig_set_freq::freq: m = %u, e = %u, i = %u, flags = %u!}",
                  (osal_u32)freq->m, (osal_u16)freq->e, freq->i, freq->flags);

    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_CURRENT_CHANEL,
        (osal_u8 *)&freq->m, OAL_SIZEOF(freq->m));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwconfig_set_freq::return err code %d!}", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置传输功率,要求功率值按照扩大10倍来输入,例如最大功率要限制为20，输入200;限制18.5,输入185
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年1月17日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwconfig_set_txpower(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iw_param_stru *param, osal_s8 *pc_extra)
{
    osal_s32                   l_ret;

    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);
    if ((OAL_PTR_NULL == net_dev) || (OAL_PTR_NULL == param)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_iwconfig_set_txpower::param null, net_dev = %p, param = %p.}",
                       (uintptr_t)net_dev, (uintptr_t)param);
        return -OAL_EINVAL;
    }

    l_ret = wal_get_init_state_vap(net_dev, OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_iwconfig_set_txpower::wal_get_init_state_vap err}");
        return l_ret;
    }

    oam_info_log4(0, OAM_SF_ANY,
                  "{uapi_iwconfig_set_txpower::param: value= %d, fixed = %d, disabled = %d, flags = %d!}",
                  param->value, param->fixed, param->disabled, param->flags);

    if (param->flags != OAL_IW_TXPOW_DBM) {     /* 如果参数输入类型不是dBm，则返回错误 */
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_iwconfig_set_txpower::invalid argument!}");
        return -OAL_EINVAL;
    }
    /* 参数异常: 设置值是db的10倍，功率限制大于1W */
    if (param->value > WLAN_MAX_TXPOWER * 10 || param->value < 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_iwconfig_set_txpower::invalid argument!}");
        return -OAL_EINVAL;
    }
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_TX_POWER,
        (osal_u8 *)&param->value, OAL_SIZEOF(param->value));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwconfig_set_txpower::return err code %d!}", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}

#if defined (LINUX_VERSION_CODE) && defined (KERNEL_VERSION) && \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
/*****************************************************************************
 功能描述  : 设置ssid
 输入参数  : oal_net_device_stru *net_dev
             osal_s8 *pc_param
 输出参数  : 无
 返 回 值  : OAL_STATIC osal_u32

 修改历史      :
  1.日    期   : 2014年12月17日
    修改内容   : 新生成函数

*****************************************************************************/
#ifdef CONTROLLER_CUSTOMIZATION
OAL_STATIC osal_s32  uapi_iwconfig_set_essid(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iw_freq_stru *freq, osal_s8 *pc_param)
#else
osal_u32  uapi_iwconfig_set_essid(oal_net_device_stru *net_dev, osal_s8 *pc_param)
#endif
{
    osal_u8                       ssid_len;
    osal_s32                       l_ret;
    hmac_vap_stru                   *hmac_vap;
    osal_u32                      off_set, ul_ret;
    osal_s8                       *pc_ssid;
    osal_s8                        ac_ssid[WAL_CCPRIV_CMD_NAME_MAX_LEN] = {0};

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (NULL == hmac_vap) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_iwconfig_set_essid::hmac_vap is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (WLAN_VAP_MODE_BSS_AP == hmac_vap->vap_mode) {
        /* 设备在up状态且是AP时，不允许配置，必须先down */
        if (hmac_vap->vap_state != MAC_VAP_STATE_INIT) {
            oam_error_log2(0, OAM_SF_ANY,
                           "vap_id[%d] {uapi_iwconfig_set_essid::device is busy, please down it firste %d!}",
                           hmac_vap->vap_id, hmac_vap->vap_state);
            return OAL_EBUSY;
        }
    }

    /* pc_param指向传入模式参数, 将其取出存放到ac_mode_str中 */
    ul_ret = wal_get_cmd_one_arg(pc_param, ac_ssid, OAL_SIZEOF(ac_ssid), &off_set);
    if (OAL_SUCC != ul_ret) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{uapi_iwconfig_set_essid::wal_get_cmd_one_arg vap name return err_code %d!}", ul_ret);
        return ul_ret;
    }

    pc_ssid       = oal_strim(ac_ssid);                   /* 去掉字符串开始结尾的空格 */
    ssid_len = (osal_u8)osal_strlen((const osal_s8 *)pc_ssid);
    if (ssid_len > WLAN_SSID_MAX_LEN - 1) {      /* -1为\0预留空间 */
        ssid_len = WLAN_SSID_MAX_LEN - 1;
    }

    oam_info_log2(0, OAM_SF_ANY, "vap_id[%d] {uapi_iwconfig_set_essid:: ssid length is %d!}", hmac_vap->vap_id,
                  ssid_len);

    l_ret = wal_set_ssid(net_dev, pc_ssid, ssid_len);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log2(0, OAM_SF_ANY, "vap_id[%d] {uapi_iwconfig_set_essid:: send event return err code %d!}",
            hmac_vap->vap_id, l_ret);
        return (osal_u32)l_ret;
    }

    return OAL_SUCC;
}
#endif

#if defined (LINUX_VERSION_CODE) && defined (KERNEL_VERSION) && \
    (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
/* 不使用iwconfig iwpriv ，采用ccpriv 接口 */
#ifdef CONTROLLER_CUSTOMIZATION
OAL_STATIC osal_s32  uapi_iwpriv_get_mode(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iw_freq_stru *freq, osal_s8 *pc_extra)
#else
OAL_STATIC osal_u32  uapi_iwpriv_get_mode(oal_net_device_stru *net_dev, osal_s8 *pc_param)
#endif
{
    return OAL_FAIL;
}

#ifdef CONTROLLER_CUSTOMIZATION
OAL_STATIC osal_s32  uapi_iwconfig_get_essid(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iw_freq_stru *freq, osal_s8 *pc_extra)
#else
OAL_STATIC osal_u32  uapi_iwconfig_get_essid(oal_net_device_stru *net_dev, osal_s8 *pc_param)
#endif
{
    return OAL_FAIL;
}

#ifdef CONTROLLER_CUSTOMIZATION
OAL_STATIC osal_s32  uapi_iwconfig_get_bss_type(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iw_freq_stru *freq, osal_s8 *pc_extra)
#else
OAL_STATIC osal_u32  uapi_iwconfig_get_bss_type(oal_net_device_stru *net_dev, osal_s8 *pc_param)
#endif
{
    return OAL_FAIL;
}

#ifdef CONTROLLER_CUSTOMIZATION
OAL_STATIC osal_s32  uapi_iwconfig_set_bss_type(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iw_freq_stru *freq, osal_s8 *pc_extra)
#else
OAL_STATIC osal_u32  uapi_iwconfig_set_bss_type(oal_net_device_stru *net_dev, osal_s8 *pc_param)
#endif
{
    return OAL_FAIL;
}

#ifdef CONTROLLER_CUSTOMIZATION
OAL_STATIC osal_s32  uapi_iwconfig_get_freq(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iw_freq_stru *freq, osal_s8 *pc_extra)
#else
OAL_STATIC osal_u32  uapi_iwconfig_get_freq(oal_net_device_stru *net_dev, osal_s8 *pc_param)
#endif
{
    return OAL_FAIL;
}

#ifdef CONTROLLER_CUSTOMIZATION
OAL_STATIC osal_s32  uapi_iwconfig_get_txpower(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iw_freq_stru *freq, osal_s8 *pc_extra)
#else
OAL_STATIC osal_u32  uapi_iwconfig_get_txpower(oal_net_device_stru *net_dev, osal_s8 *pc_param)
#endif
{
    return OAL_FAIL;
}

#else
/*****************************************************************************
 功能描述  : 读取vap模式: 包括协议 频点 带宽
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年1月18日
    修改内容   : 新生成函数
  2.日    期   : 2013年6月9日
    修改内容   : 统一发送消息接口函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwpriv_get_mode(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_void *p_param, osal_s8 *pc_extra)
{
    osal_s32  ret;
    mac_cfg_mode_param_stru fill_msg;
    frw_msg cfg_info = {0};
    oal_iw_point_stru              *point;
    osal_u8                       prot_idx;
    mac_cfg_mode_param_stru        *rsp_mode_param;
    osal_s8                       *pc_err_str = "Error protocal";

    wal_record_wifi_external_log(WLAN_WIFI_IW_PRIV_HANDLERS, __func__);
    cfg_info.rsp_buf_len = sizeof(fill_msg);
    cfg_info.rsp = (osal_u8 *)&fill_msg;

    ret = send_sync_cfg_to_host(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_GET_MODE, &cfg_info);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwpriv_get_mode:: return err code %d!}", ret);
        return ret;
    }

    /* 业务处理 */
    point = (oal_iw_point_stru *)p_param;

    rsp_mode_param = (mac_cfg_mode_param_stru *)(cfg_info.rsp);

    if (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
        for (prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map_etc[prot_idx].pc_name; prot_idx++) {
            if ((g_ast_mode_map_etc[prot_idx].mode == rsp_mode_param->protocol) &&
                (g_ast_mode_map_etc[prot_idx].band == rsp_mode_param->band) &&
                (g_ast_mode_map_etc[prot_idx].en_bandwidth == rsp_mode_param->en_bandwidth)) {
                break;
            }
        }
    } else if (net_dev->ieee80211_ptr->iftype == NL80211_IFTYPE_STATION) {
        /* STA模式下频段和频段在和AP关联之后自适应，此处仅比较协议模式 */
        for (prot_idx = 0; OAL_PTR_NULL != g_ast_mode_map_etc[prot_idx].pc_name; prot_idx++) {
            if (g_ast_mode_map_etc[prot_idx].mode == rsp_mode_param->protocol) {
                break;
            }
        }
    } else {
        return OAL_SUCC;
    }

    if (OAL_PTR_NULL == g_ast_mode_map_etc[prot_idx].pc_name) {
        point->length = (osal_u16)osal_strlen((const osal_s8 *)pc_err_str);
        if (memcpy_s(pc_extra, point->length, pc_err_str, point->length) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{uapi_iwpriv_get_mode::memcpy_s err_str error}");
        }
        return OAL_SUCC;
    }

    point->length = (osal_u16)osal_strlen((const osal_s8 *)g_ast_mode_map_etc[prot_idx].pc_name);
    if (memcpy_s(pc_extra, point->length, g_ast_mode_map_etc[prot_idx].pc_name, point->length) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_iwpriv_get_mode::memcpy_s name error}");
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置ssid
 输入参数  : net_dev: net device
             info   : iw info
             data   : ioctl payload
             pc_ssid    : ssid
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2013年1月14日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwconfig_set_essid(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iw_point_stru *data, osal_s8 *pc_ssid)
{
    osal_u8                       ssid_len;
    osal_s32                       l_ret;
    hmac_vap_stru                   *hmac_vap;

    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);
    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (NULL == hmac_vap) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_iwconfig_set_essid::hmac_vap is null!}");
        return -OAL_EFAUL;
    }

    if (WLAN_VAP_MODE_BSS_AP == hmac_vap->vap_mode) {
        /* 设备在up状态且是AP时，不允许配置，必须先down */
        if (hmac_vap->vap_state != MAC_VAP_STATE_INIT) {
            oam_error_log2(0, OAM_SF_ANY,
                           "vap_id[%d] {uapi_iwconfig_set_essid::device is busy, please down it firste %d!}",
                           hmac_vap->vap_id, hmac_vap->vap_state);
            return -OAL_EBUSY;
        }
    }

    pc_ssid = oal_strim(pc_ssid);                   /* 去掉字符串开始结尾的空格 */

    oam_info_log3(0, OAM_SF_ANY, "vap_id[%d] {uapi_iwconfig_set_essid:: data->flags, data->lengt %d, %d!}",
                  hmac_vap->vap_id, data->flags, data->length);
    ssid_len = (osal_u8)osal_strlen((const osal_s8 *)pc_ssid);
    if (ssid_len > WLAN_SSID_MAX_LEN - 1) {      /* -1为\0预留空间 */
        ssid_len = WLAN_SSID_MAX_LEN - 1;
    }

    oam_info_log2(0, OAM_SF_ANY, "vap_id[%d] {uapi_iwconfig_set_essid:: ssid length is %d!}", hmac_vap->vap_id,
                  ssid_len);

    l_ret = wal_set_ssid(net_dev, pc_ssid, ssid_len);
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log2(0, OAM_SF_ANY,
                         "vap_id[%d] {uapi_iwconfig_set_essid:: wal_set_ssid return err code %d!}",
                         hmac_vap->vap_id, l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置ssid
 输入参数  : net_dev: net device
             info   : iw info
             data   : ioctl payload
             pc_ssid    : ssid
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2013年1月14日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwconfig_get_essid(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iw_point_stru *data, osal_s8 *pc_ssid)
{
    osal_s32  ret;
    mac_cfg_ssid_param_stru fill_msg;
    frw_msg cfg_info;

    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);
    memset_s(&cfg_info, sizeof(cfg_info), 0, sizeof(cfg_info));
    cfg_info.rsp_buf_len = sizeof(fill_msg);
    cfg_info.rsp = (osal_u8 *)&fill_msg;

    ret = send_sync_cfg_to_host(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_GET_SSID, &cfg_info);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwconfig_get_essid:: return err code %d!}", ret);
        return ret;
    }

    data->flags = 1;    /* 设置出参标志为有效 */
    data->length = fill_msg.ssid_len;

    if (memcpy_s(pc_ssid, fill_msg.ssid_len, fill_msg.ac_ssid, fill_msg.ssid_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_iwconfig_get_essid::memcpy_s error}");
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : get bss type
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年1月17日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwconfig_get_bss_type(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_u32 *pul_type, osal_s8 *pc_extra)
{
    osal_u32 bss_type;
    osal_s32  ret;
    osal_s32   fill_msg;
    frw_msg    cfg_info;

    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);
    memset_s(&cfg_info, sizeof(cfg_info), 0, sizeof(cfg_info));
    cfg_info.rsp_buf_len = sizeof(fill_msg);
    cfg_info.rsp = (osal_u8 *)&fill_msg;

    ret = send_sync_cfg_to_host(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_BSS_TYPE, &cfg_info);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_get_bss_type::uapi_iwconfig_get_bss_type err %d!}", ret);
        return ret;
    }

    /* 业务处理 */
    bss_type = *((osal_u32 *)cfg_info.rsp);

    *pul_type = OAL_IW_MODE_AUTO;

    if (WLAN_MIB_DESIRED_BSSTYPE_INFRA == bss_type) {
        *pul_type = OAL_IW_MODE_INFRA;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : set bss type
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年1月17日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_iwconfig_set_bss_type(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_u32 *pul_type, osal_s8 *pc_extra)
{
    osal_u32                      type;
    osal_s32                       l_ret;

    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);
    if ((OAL_PTR_NULL == net_dev) || (OAL_PTR_NULL == pul_type)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_iwconfig_set_bss_type::param null, net_dev = %p, pul_type = %p.}",
                       net_dev, pul_type);
        return -OAL_EINVAL;
    }

    l_ret = wal_get_init_state_vap(net_dev, OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_iwconfig_set_bss_type::wal_get_init_state_vap err}");
        return l_ret;
    }

    type = *pul_type;

    oam_info_log1(0, OAM_SF_ANY, "{uapi_iwconfig_set_bss_type::type value is %d!}", type);

    if (type < WLAN_MIB_DESIRED_BSSTYPE_INFRA || type >= WLAN_MIB_DESIRED_BSSTYPE_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_iwconfig_set_bss_type::input type is invalid!}");
        return -OAL_EINVAL;
    }
    l_ret = wal_sync_post2hmac_no_rsp(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_BSS_TYPE,
        (osal_u8 *)&type, OAL_SIZEOF(type));
    if (OAL_UNLIKELY(OAL_SUCC != l_ret)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwconfig_set_bss_type::return err code %d!}", l_ret);
        return l_ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 获取频点/信道
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年1月17日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_iwconfig_get_freq(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iw_freq_stru *freq, osal_s8 *pc_extra)
{
    osal_s32  ret;
    osal_s32  fill_msg;
    frw_msg    cfg_info;

    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);
    memset_s(&cfg_info, sizeof(cfg_info), 0, sizeof(cfg_info));
    cfg_info.rsp_buf_len = sizeof(fill_msg);
    cfg_info.rsp = (osal_u8 *)&fill_msg;

    ret = send_sync_cfg_to_host(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_GET_CURRENT_CHANEL, &cfg_info);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwconfig_get_freq:: return err code %d!}", ret);
        return ret;
    }

    /* 业务处理 */
    freq->m = *((osal_s32 *)(cfg_info.rsp));
    freq->e = 0;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 获得传输功率
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年1月17日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_iwconfig_get_txpower(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iw_param_stru *param, osal_s8 *pc_extra)
{
    osal_s32  ret;
    osal_s32  fill_msg;
    frw_msg cfg_info;

    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);
    memset_s(&cfg_info, sizeof(cfg_info), 0, sizeof(cfg_info));
    cfg_info.rsp_buf_len = sizeof(fill_msg);
    cfg_info.rsp = (osal_u8 *)&fill_msg;

    ret = send_sync_cfg_to_host(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_GET_TX_POWER, &cfg_info);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwconfig_get_txpower:: return err code %d!}", ret);
        return ret;
    }

    /* 业务处理 */
    param->value    = *((osal_s32 *)(cfg_info.rsp));
    param->fixed    = 1;
    param->disabled = 0;
    param->flags    = OAL_IW_TXPOW_DBM;

    return OAL_SUCC;
}

#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44)) */ /* mp12 不使用iwconfig，采用ccpriv 接口 */

/*****************************************************************************
 功能描述  : 获取BSSID
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年4月21日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwconfig_get_apaddr(
#ifdef CONTROLLER_CUSTOMIZATION
    struct net_device           *net_dev,
    struct iw_request_info      *info,
    union iwreq_data            *wrqu,
    char                        *pc_extra)
#else
    oal_net_device_stru         *net_dev,
    oal_iw_request_info_stru    *info,
    oal_sockaddr_stru           *addr,
    osal_s8                    *pc_extra)
#endif
{
    hmac_vap_stru   *hmac_vap;
    osal_u8       zero_addr[WLAN_MAC_ADDR_LEN] = {0};

#ifdef CONTROLLER_CUSTOMIZATION
    oal_sockaddr_stru *addr = OAL_PTR_NULL;
    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);
    if ((OAL_PTR_NULL == net_dev) || (OAL_PTR_NULL == wrqu)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_iwconfig_get_apaddr::param null, net_dev = %p, wrqu = %p.}",
                       (uintptr_t)net_dev, (uintptr_t)wrqu);
        return -OAL_EINVAL;
    }
    addr = (oal_sockaddr_stru *)wrqu;
#else
    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);
    if ((OAL_PTR_NULL == net_dev) || (OAL_PTR_NULL == addr)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_iwconfig_get_apaddr::param null, net_dev = %p, addr = %p.}",
                       (uintptr_t)net_dev, (uintptr_t)addr);
        return -OAL_EINVAL;
    }
#endif

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (NULL == hmac_vap) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_iwconfig_get_apaddr::hmac_vap is null!}");
        return -OAL_EFAUL;
    }

    if (MAC_VAP_STATE_UP == hmac_vap->vap_state) {
        oal_set_mac_addr((osal_u8 *)addr->sa_data, (const osal_u8 *)hmac_vap->bssid);
    } else {
        oal_set_mac_addr((osal_u8 *)addr->sa_data, (const osal_u8 *)zero_addr);
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : iwconfig获取rate，不支持，返回-1
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年4月21日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwconfig_get_iwrate(
    oal_net_device_stru         *net_dev,
    oal_iw_request_info_stru    *info,
    oal_iw_param_stru           *param,
    osal_s8                    *pc_extra)
{
    /* iwconfig获取rate，不支持此命令，则返回-1 */

    return -OAL_EFAIL;
}

/*****************************************************************************
 功能描述  : iwconfig获取sense，不支持，返回-1
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年4月21日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwconfig_get_iwsense(
    oal_net_device_stru         *net_dev,
    oal_iw_request_info_stru    *info,
    oal_iw_param_stru           *param,
    osal_s8                    *pc_extra)
{
    /* iwconfig获取sense，不支持此命令，则返回-1 */
    return -OAL_EFAIL;
}

/*****************************************************************************
 功能描述  : iwconfig获取rtsthres
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年4月21日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwconfig_get_rtsthres(
    oal_net_device_stru         *net_dev,
    oal_iw_request_info_stru    *info,
    oal_iw_param_stru           *param,
    osal_s8                    *pc_extra)
{
    hmac_vap_stru   *hmac_vap;

    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);
    if ((OAL_PTR_NULL == net_dev) || (OAL_PTR_NULL == param)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_iwconfig_get_rtsthres::param null, net_dev = %p, param = %p.}",
                       (uintptr_t)net_dev, (uintptr_t)param);
        return -OAL_EINVAL;
    }

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (NULL == hmac_vap) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_iwconfig_get_rtsthres::hmac_vap is null!}");
        return -OAL_EFAUL;
    }

    param->value    = (osal_s32)mac_mib_get_rts_threshold(hmac_vap);
    param->disabled = (WLAN_RTS_MAX == param->value);
    param->fixed    = 1;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : iwconfig获取fragthres
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年4月21日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwconfig_get_fragthres(
    oal_net_device_stru         *net_dev,
    oal_iw_request_info_stru    *info,
    oal_iw_param_stru           *param,
    osal_s8                    *pc_extra)
{
    hmac_vap_stru   *hmac_vap;

    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);
    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (NULL == hmac_vap) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_iwconfig_get_fragthres::hmac_vap is null!}");
        return -OAL_EFAUL;
    }

    param->value    = (osal_s32)mac_mib_get_FragmentationThreshold(hmac_vap);
    param->disabled = (WLAN_FRAG_THRESHOLD_MAX == param->value);
    param->fixed    = 1;

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : iwconfig获取encode, 不支持
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年4月22日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwconfig_get_iwencode(
    oal_net_device_stru         *net_dev,
    oal_iw_request_info_stru    *info,
    oal_iw_point_stru           *param,
    osal_s8                    *pc_extra)
{
    /* 不支持iwconfig获取encode，直接返回-1 */
    return -OAL_EFAIL;
}

/*****************************************************************************
 功能描述  : iwconfig获取iwrange, 不支持
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2014年4月21日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwconfig_get_iwrange(
    oal_net_device_stru         *net_dev,
    oal_iw_request_info_stru    *info,
    oal_iw_point_stru           *param,
    osal_s8                    *pc_extra)
{
#ifdef _PRE_WLAN_WIRELESS_EXT
    struct iw_range *range = OSAL_NULL;
    if (net_dev == OSAL_NULL || info == OSAL_NULL || param == OSAL_NULL || pc_extra == OSAL_NULL) {
        return -OAL_EFAUL;
    }

    range = (struct iw_range *)pc_extra;
    param->length = (osal_u16)sizeof(struct iw_range);
    memset_s(range, sizeof(struct iw_range), 0, sizeof(struct iw_range));
    range->we_version_compiled = WIRELESS_EXT;
    return OAL_SUCC;
#else
    return -OAL_EFAIL;
#endif
}

/*****************************************************************************
 功能描述  : iwpriv私有获取参数命令入口
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年1月17日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_iwpriv_get_param(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_void *p_iw, osal_s8 *extra)
{
    osal_s32 l_ret;
    osal_s32 *pl_param = OSAL_NULL;
    osal_u16 subid;
    frw_msg cfg_info;

    if (extra == OSAL_NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_iwpriv_get_param::extra param null!}");
        return OAL_FAIL;
    }

    pl_param = (osal_s32 *)extra;
    subid = (osal_u16)pl_param[0];
    oam_info_log1(0, OAM_SF_ANY, "{uapi_iwpriv_get_param::subid %d!}", subid);

    memset_s(&cfg_info, sizeof(cfg_info), 0, sizeof(cfg_info));
    cfg_info.rsp_buf_len = (osal_u16)sizeof(osal_s32);
    cfg_info.rsp = extra;

    wal_record_external_log_by_id(WLAN_WIFI_RECORD_IWCHAR, subid);
    l_ret = send_sync_cfg_to_host(wal_util_get_vap_id(net_dev), subid, &cfg_info);
    if ((l_ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL) || (cfg_info.rsp_len > OAL_SIZEOF(osal_s32))) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwpriv_get_param:: return err code %d!}", l_ret);
        return l_ret;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : iwpriv私有设置参数命令入口
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年1月17日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwpriv_set_param(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_void *p_iw, osal_s8 *pc_extra)
{
    return OAL_SUCC;
}

STATIC osal_s32 iwpriv_check_wme_params(osal_s32 subioctl_id, osal_s32 l_value)
{
    switch (subioctl_id) {                          /* 根据sub-ioctl id填写WID */
        case WLAN_MSG_W2H_CFG_EDCA_TABLE_CWMIN:
        case WLAN_MSG_W2H_CFG_QEDCA_TABLE_CWMIN:
            if ((l_value > WLAN_QEDCA_TABLE_CWMIN_MAX) || (l_value < WLAN_QEDCA_TABLE_CWMIN_MIN)) {
                return -OAL_EINVAL;
            }
            break;

        case WLAN_MSG_W2H_CFG_EDCA_TABLE_CWMAX:
        case WLAN_MSG_W2H_CFG_QEDCA_TABLE_CWMAX:
            if ((l_value > WLAN_QEDCA_TABLE_CWMAX_MAX) || (l_value < WLAN_QEDCA_TABLE_CWMAX_MIN)) {
                return -OAL_EINVAL;
            }
            break;

        case WLAN_MSG_W2H_CFG_EDCA_TABLE_AIFSN:
        case WLAN_MSG_W2H_CFG_QEDCA_TABLE_AIFSN:
            if ((l_value < WLAN_QEDCA_TABLE_AIFSN_MIN) || (l_value > WLAN_QEDCA_TABLE_AIFSN_MAX)) {
                return -OAL_EINVAL;
            }
            break;

        case WLAN_MSG_W2H_CFG_EDCA_TABLE_TXOP_LIMIT:
        case WLAN_MSG_W2H_CFG_QEDCA_TABLE_TXOP_LIMIT:
            if (l_value > WLAN_QEDCA_TABLE_TXOP_LIMIT_MAX) {
                return -OAL_EINVAL;
            }
            break;

        case WLAN_MSG_W2H_CFG_EDCA_TABLE_MSDU_LIFETIME:
        case WLAN_MSG_W2H_CFG_QEDCA_TABLE_MSDU_LIFETIME:
            if (l_value > WLAN_QEDCA_TABLE_MSDU_LIFETIME_MAX) {
                return -OAL_EINVAL;
            }
            break;

        case WLAN_MSG_W2H_CFG_EDCA_TABLE_MANDATORY:
        case WLAN_MSG_W2H_CFG_QEDCA_TABLE_MANDATORY:
            if ((OAL_TRUE != l_value) && (OAL_FALSE != l_value)) {
                return -OAL_EINVAL;
            }
            break;

        default:
            break;
    }
    return 0;
}

/*****************************************************************************
 功能描述  : iwpriv私有设置参数命令入口
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年5月9日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwpriv_set_wme_params(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_void *p_iw, osal_s8 *pc_extra)
{
    osal_s32                       l_error = 0;
    osal_s32                       l_subioctl_id;
    osal_s32                       l_ac;
    osal_s32                       l_value;
    osal_s32                        ret;
    osal_s32                      *pl_param;
    hmac_vap_stru                  *hmac_vap;
    wal_msg_wmm_stru                wmm_params;

    wal_record_wifi_external_log(WLAN_WIFI_IW_PRIV_HANDLERS, __func__);
    (osal_void)memset_s(&wmm_params, OAL_SIZEOF(wmm_params), 0, OAL_SIZEOF(wmm_params));

    if ((OAL_PTR_NULL == net_dev) || (OAL_PTR_NULL == pc_extra)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_iwpriv_set_wme_params::param null, net_dev = %p, pc_extra = %p.}",
                       (uintptr_t)net_dev, (uintptr_t)pc_extra);
        return -OAL_EINVAL;
    }

    ret = wal_get_init_state_vap(net_dev, &hmac_vap);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_iwpriv_set_wme_params::wal_get_init_state_vap err}");
        return (osal_s32)ret;
    }

    pl_param      = (osal_s32 *)pc_extra;
    l_subioctl_id = pl_param[0];    /* 获取sub-ioctl的ID */
    l_ac          = pl_param[1];
    l_value       = pl_param[2];    /* 第2位，获取要设置的值 */

    oam_info_log3(0, OAM_SF_ANY, "{uapi_iwpriv_set_wme_params::the subioctl_id,l_ac,value is %d, %d, %d!}",
        l_subioctl_id, l_ac, l_value);

    /* ac取值0~3, value不能为负值 */
    if ((l_value < 0) || (l_ac < 0) || (l_ac >= WLAN_WME_AC_BUTT)) {
        oam_warning_log2(0, OAM_SF_ANY, "{uapi_iwpriv_set_wme_params::input value is negative %d, %d!}", l_value, l_ac);
        return -OAL_EINVAL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 填写消息 */
    l_error = iwpriv_check_wme_params(l_subioctl_id, l_value);
    if (0 != l_error) {         /* 参数异常 */
        return l_error;
    }

    wmm_params.cfg_id    = (osal_u16)l_subioctl_id;
    wmm_params.ac        = (osal_u32)l_ac;
    wmm_params.value     = (osal_u32)l_value;

    ret = wal_sync_post2hmac_no_rsp(hmac_vap->vap_id, (osal_u16)l_subioctl_id,
        (osal_u8 *)&wmm_params, OAL_SIZEOF(wmm_params));
    if (OAL_UNLIKELY(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwpriv_set_wme_params::return err code %d!}", ret);
        return (osal_s32)ret;
    }

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : iwpriv私有获取参数命令入口
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年5月9日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwpriv_get_wme_params(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_void *p_iw, osal_s8 *pc_extra)
{
    osal_slong *param;

    wal_record_wifi_external_log(WLAN_WIFI_IW_PRIV_HANDLERS, __func__);
    if ((OAL_PTR_NULL == net_dev) || (OAL_PTR_NULL == pc_extra)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_iwpriv_get_wme_params::param null, net_dev = %p, pc_extra = %p.}",
                       (uintptr_t)net_dev, (uintptr_t)pc_extra);
        return -OAL_EINVAL;
    }

    param = (osal_slong *)pc_extra;

    param[0] = (osal_slong)wal_config_get_wmm_params_etc(net_dev, (osal_u8 *)pc_extra);

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 字符串参数设置总入口
 输入参数  : net_dev: net device
 输出参数  : p_param    : 输入字符串
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2017年03月27日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_iwpriv_set_param_char(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_void *p_param, osal_s8 *pc_extra)
{
    return TR_SUCCESS;
}

/*****************************************************************************
 功能描述  : iwpriv字符串参数总入口
 输入参数  : 无
 输出参数  : 无

 修改历史      :
  1.日    期   : 2013年3月30日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_iwpriv_get_param_char(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_void *p_w, osal_s8 *pc_extra)
{
    oal_iw_point_stru              *w = (oal_iw_point_stru *)p_w;
    osal_u32                      subioctl_id;
    osal_s32                       l_ret;
    mac_cfg_param_char_stru param;
    hmac_vap_stru                   *hmac_vap;
    frw_msg cfg_info;

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (NULL == hmac_vap) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_iwpriv_get_param_char::hmac_vap is null!}");
        return -OAL_EINVAL;
    }

    subioctl_id = w->flags;
    wal_record_external_log_by_id(WLAN_WIFI_RECORD_IWINT, subioctl_id);
    if (WLAN_MSG_W2H_CFG_GET_DSCP_TID_MAP == subioctl_id) {
        l_ret = iwpriv_get_dscp_tid_map(net_dev, info, p_w, pc_extra);
        return l_ret;
    } else if (WLAN_MSG_W2H_CFG_GET_DIEID == subioctl_id) {
        l_ret = iwpriv_get_dieid(net_dev, info, p_w, pc_extra);
        return l_ret;
    } else {
        memset_s(&param, sizeof(param), 0, sizeof(param));
        memset_s(&cfg_info, sizeof(cfg_info), 0, sizeof(cfg_info));
        cfg_info.rsp_buf_len = (osal_u16)sizeof(param);
        cfg_info.rsp = (osal_u8 *)&param;
        l_ret = send_sync_cfg_to_host(hmac_vap->vap_id, (osal_u16)subioctl_id, &cfg_info);
        if ((l_ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
            oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwpriv_get_param:: return err code %d!}", l_ret);
            return l_ret;
        }
        if (cfg_info.rsp_len != 0) {
            w->length = (osal_u16)param.buff_len;
            if (memcpy_s(pc_extra, (osal_u32)param.buff_len, (osal_s8 *)param.buff,
                (osal_u32)param.buff_len) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{uapi_iwpriv_get_param_char::memcpy_s error}");
            }
        } else {
            w->length = 0;
        }
        w->flags = 1;
    }
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置管制域国家码
 输入参数  : net_dev: net device
             info   : 请求的信息
             p_w        : 请求的信息
             pc_extra   : 国家字符串
 输出参数  : 无
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2013年10月16日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwpriv_set_country(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_void *p_w, osal_s8 *pc_extra)
{
#ifdef _PRE_WLAN_FEATURE_11D
    osal_s32  l_ret;

    wal_record_wifi_external_log(WLAN_WIFI_IW_PRIV_HANDLERS, __func__);
    if ((OAL_PTR_NULL == net_dev) || (OAL_PTR_NULL == pc_extra)) {
        oam_error_log2(0, OAM_SF_ANY, "{uapi_iwpriv_set_country::param null, net_dev = %p, pc_extra= %p.}",
                       (uintptr_t)net_dev, (uintptr_t)pc_extra);
        return -OAL_EINVAL;
    }

    l_ret = wal_get_init_state_vap(net_dev, OAL_PTR_NULL);
    if (l_ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_iwpriv_set_country::wal_get_init_state_vap err}");
        return l_ret;
    }

    l_ret = wal_util_set_country_code(net_dev, pc_extra);
    if (OAL_SUCC != l_ret) {
        oam_warning_log1(0, OAM_SF_DFS, "{uapi_iwpriv_set_country::util_set_country_code err code [%d]!}", l_ret);
        return l_ret;
    }
#else
    oam_info_log0(0, OAM_SF_ANY, "{uapi_iwpriv_set_country:_PRE_WLAN_FEATURE_11D is not define!}");
#endif

    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置netd下发APUT config参数 - 解析MAX_SCB
 被调函数wal_iwpriv_set_ap_confignfig(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
                                    oal_iwreq_data_union *wrqu, osal_s8 *pc_extra)
*****************************************************************************/
OAL_STATIC osal_s32 wal_iwpriv_parse_max_scb(oal_net_device_stru *net_dev, osal_s8 *pc_parse_command,
    osal_u32 off_set)
{
    osal_s32 ret = OAL_SUCC;
    osal_u32 ret_code;
    osal_s8 ac_parse_command[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN];
    wal_ap_config_stru *ap_config_info = wal_get_ap_config_info();

    /* 解析MAX_SCB */
    pc_parse_command += off_set;
    ret_code = wal_get_parameter_from_cmd(pc_parse_command, ac_parse_command, "MAX_SCB=", &off_set,
        WAL_IOCTL_PRIV_SUBCMD_MAX_LEN);
    if (ret_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_iwpriv_parse_max_scb::wal_get_parameter_from_cmd MAX_SCB return err_code [%u]!}", ret_code);
        return -OAL_EINVAL;
    }

    ap_config_info->ap_max_user = (osal_u32)oal_atoi((const osal_s8 *)ac_parse_command);

    if (net_dev->ml_priv != OSAL_NULL) {
        ret = wal_set_ap_max_user(net_dev, (osal_u32)oal_atoi((const osal_s8 *)ac_parse_command));
    }
    return ret;
}

/*****************************************************************************
 功能描述  : 读取国家码
 输入参数  : net_dev: net device
             info   : 请求的信息
             p_w        : 请求的信息
 输出参数  : pc_extra   : 读取到的国家码
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2013年10月16日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwpriv_get_country(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_void *p_w, osal_s8 *pc_extra)
{
#ifdef _PRE_WLAN_FEATURE_11D
    osal_s32  ret;
    frw_msg    cfg_info;
    mac_cfg_get_country_stru  fill_msg;
    mac_cfg_get_country_stru *rsp_info;
    oal_iw_point_stru *w = (oal_iw_point_stru *)p_w;

    wal_record_wifi_external_log(WLAN_WIFI_IW_PRIV_HANDLERS, __func__);
    memset_s(&cfg_info, sizeof(cfg_info), 0, sizeof(cfg_info));
    cfg_info.rsp_buf_len = (osal_u16)sizeof(fill_msg);
    cfg_info.rsp = (osal_u8 *)&fill_msg;

    ret = send_sync_cfg_to_host(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_GET_COUNTRY, &cfg_info);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwpriv_get_country:: return err code %d!}", ret);
        return ret;
    }

    /* 业务处理 */
    rsp_info = (mac_cfg_get_country_stru *)(cfg_info.rsp);

    if (memcpy_s(pc_extra, WLAN_COUNTRY_STR_LEN, rsp_info->ac_country, WLAN_COUNTRY_STR_LEN) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_iwpriv_get_country::memcpy_s error}");
    }
    w->length = WLAN_COUNTRY_STR_LEN;

#endif
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 设置netd下发APUT config参数 (最大用户数)
 输入参数  : oal_iw_request_info_stru *info
             oal_iwreq_data_union *wrqu
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年5月21日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 uapi_iwpriv_set_ap_config(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iwreq_data_union *wrqu, osal_s8 *pc_extra)
{
    osal_s8 *command = OSAL_NULL;
    osal_s8 *pc_parse_command = OSAL_NULL;
    osal_s8 ac_parse_command[WAL_IOCTL_PRIV_SUBCMD_MAX_LEN];
    osal_s32 ret;
    osal_u32 ret_code, off_set;

    wal_record_wifi_external_log(WLAN_WIFI_IW_PRIV_HANDLERS, __func__);
    if (OAL_UNLIKELY(net_dev == OSAL_NULL || wrqu == OSAL_NULL)) {
        oam_warning_log2(0, OAM_SF_ANY, "iwpriv_set_ap_config:dev[%p],wrqu[%p]", (uintptr_t)net_dev, (uintptr_t)wrqu);
        return -OAL_EFAIL;
    }

    /* 1. 申请内存保存netd 下发的命令和数据 */
    command = oal_memalloc((osal_s32)(wrqu->data.length + 1));
    if (command == OSAL_NULL) {
        return -OAL_ENOMEM;
    }
    /* 2. 拷贝netd 命令到内核态中 */
    (osal_void)memset_s(command, (osal_u32)(wrqu->data.length + 1), 0, (osal_u32)(wrqu->data.length + 1));
    if (osal_copy_from_user(command, wrqu->data.pointer, (osal_u32)(wrqu->data.length)) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{uapi_iwpriv_set_ap_config::osal_copy_from_user: -OAL_EFAIL }");
        oal_free(command);
        return -OAL_EFAIL;
    }
    command[wrqu->data.length] = '\0';

    oam_warning_log1(0, OAM_SF_ANY, "uapi_iwpriv_set_ap_config,data len:%u", (osal_u32)wrqu->data.length);

    pc_parse_command = command;
    /* 3.   解析参数 */
    /* 3.1  解析ASCII_CMD */
    /* WAL_IOCTL_PRIV_SUBCMD_MAX_LEN为20，代表子命令的最大长度 */
    ret_code = wal_get_parameter_from_cmd(pc_parse_command, ac_parse_command, "ASCII_CMD=", &off_set, 20);
    if (ret_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwpriv_set_ap_config::wal_get_parameter_from_cmd err[%u]}", ret_code);
        oal_free(command);
        return -OAL_EINVAL;
    }

    if (osal_strcmp("AP_CFG", (const osal_s8 *)ac_parse_command) != 0) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_iwpriv_set_ap_config::sub_command != 'AP_CFG' }");
        oal_free(command);
        return -OAL_EINVAL;
    }

    /* 3.2  解析CHANNEL，目前不处理netd下发的channel信息 */
    pc_parse_command += off_set;
    /* WAL_IOCTL_PRIV_SUBCMD_MAX_LEN为20，代表子命令的最大长度 */
    ret_code = wal_get_parameter_from_cmd(pc_parse_command, ac_parse_command, "CHANNEL=", &off_set, 20);
    if (ret_code != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwpriv_set_ap_config::wal_get_parameter_from_cmd err[%u]}", ret_code);
        oal_free(command);
        return -OAL_EINVAL;
    }

    /* 3.3  解析MAX_SCB */
    ret = wal_iwpriv_parse_max_scb(net_dev, pc_parse_command, off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwpriv_set_ap_config::wal_iwpriv_parse_max_scb err_code [%u]!}", ret);
    }

    /* 5. 结束释放内存 */
    oal_free(command);
    return ret;
}

/*****************************************************************************
 功能描述  : netd下发命令取得关联设备列表
 输入参数  : oal_iw_request_info_stru *info
             oal_iwreq_data_union *wrqu
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年5月22日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwpriv_get_assoc_list(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iwreq_data_union *wrqu, osal_s8 *pc_extra)
{
    osal_s32  ret;
    frw_msg    cfg_info;
    osal_u8  fill_msg[WAL_MSG_WRITE_MAX_LEN];
    osal_s8         *pc_sta_list;
    oal_netbuf_stru  *response_netbuf;

    wal_record_wifi_external_log(WLAN_WIFI_IW_PRIV_HANDLERS, __func__);
    if (OAL_UNLIKELY(net_dev == OSAL_NULL || info == OSAL_NULL || wrqu == OSAL_NULL || pc_extra == OSAL_NULL)) {
        oam_warning_log4(0, OAM_SF_ANY,
            "{uapi_iwpriv_get_assoc_list:: param is NULL ,psy_dev=%p, info = %p, wrqu = %p , pc_extra = %p}",
            (uintptr_t)net_dev, (uintptr_t)info, (uintptr_t)wrqu, (uintptr_t)pc_extra);
        return -OAL_EFAIL;
    }

    memset_s(&cfg_info, sizeof(cfg_info), 0, sizeof(cfg_info));
    cfg_info.rsp_buf_len = (osal_u16)sizeof(fill_msg);
    cfg_info.rsp = (osal_u8 *)&fill_msg;

    ret = send_sync_cfg_to_host(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_GET_STA_LIST, &cfg_info);
    if ((ret != OAL_SUCC) || (cfg_info.rsp == OAL_PTR_NULL)) {
        oam_warning_log1(0, OAM_SF_ANY, "{uapi_iwpriv_get_assoc_list:: return err code %d!}", ret);
        return ret;
    }

    if (cfg_info.rsp_len >= OAL_SIZEOF((uintptr_t)response_netbuf)) {
        if (memcpy_s(&response_netbuf, OAL_SIZEOF((uintptr_t)response_netbuf),
            cfg_info.rsp, OAL_SIZEOF((uintptr_t)response_netbuf)) != EOK) {
            oam_error_log0(0, OAM_SF_ANY, "{uapi_iwpriv_get_assoc_list::memcpy_s rsp error}");
        }
        if (NULL != response_netbuf) {
            pc_sta_list = (osal_s8 *)OAL_NETBUF_DATA(response_netbuf);
            wrqu->data.length = (osal_u16)(OAL_NETBUF_LEN(response_netbuf) + 1);
            if (memcpy_s(pc_extra, wrqu->data.length, pc_sta_list, wrqu->data.length) != EOK) {
                oam_error_log0(0, OAM_SF_ANY, "{uapi_iwpriv_get_assoc_list::memcpy_s sta_list error}");
            }
            pc_extra[OAL_NETBUF_LEN(response_netbuf)] = '\0';
            oal_netbuf_free(response_netbuf);
        } else {
            ret = -OAL_ENOMEM;
        }
    } else {
        /* dump的groupsize为32 */
        oal_print_hex_dump(cfg_info.rsp, cfg_info.rsp_len, 32, "query msg: ");
        ret = -OAL_EINVAL;
    }

    if (OAL_SUCC != ret) {
        oam_error_log1(0, OAM_SF_ANY, "{uapi_iwpriv_get_assoc_list::process failed,ret=%d}", ret);
    } else {
        oam_warning_log1(0, OAM_SF_ANY, "uapi_iwpriv_get_assoc_list,len:%d", wrqu->data.length);
    }

    return ret;
}

/*****************************************************************************
 功能描述  : netd下发命令设置黑名单或白名单
 输入参数  : oal_iw_request_info_stru *info
             oal_iwreq_data_union *wrqu
 输出参数  : 无
 返 回 值  : osal_u32
 修改历史      :
  1.日    期   : 2015年5月22日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwpriv_set_mac_filters(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    oal_iwreq_data_union *wrqu, osal_s8 *pc_extra)
{
    return TR_SUCCESS;
}

/*****************************************************************************
 功能描述  : netd下发命令去关联STA
 输入参数  : oal_iw_request_info_stru *info
             oal_iwreq_data_union *wrqu
 输出参数  : 无
 返 回 值  : osal_u32

 修改历史      :
  1.日    期   : 2015年5月22日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  uapi_iwpriv_set_ap_sta_disassoc(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *info, oal_iwreq_data_union *wrqu, osal_s8 *pc_extra)
{
    return TR_SUCCESS;
}

#ifdef _PRE_WLAN_WIRELESS_EXT
static iwlist_scan_essid_enable_stru g_scan_essid_info;
osal_s32 wal_ioctl_wait_hmac_rsp_wait_condition(const void *param)
{
    wait_sync_mode_enum mode = *(wait_sync_mode_enum *)param;
    return (g_wal_hmac_wait_sync[mode].wal_hmac_sync_flag == OSAL_TRUE);
}

static osal_s32 wal_ioctl_wait_hmac_rsp_event(hmac_vap_stru *hmac_vap, osal_u32 time, wait_sync_mode_enum mode)
{
    osal_slong leftime;
    osal_s32 ret;

    if (hmac_vap == OSAL_NULL) {
        return -OAL_EFAIL;
    }

    if (mode >= WAIT_SYNC_MODE_BUTT) {
        oam_error_log1(0, OAM_SF_ANY, "wal_ioctl_wait_hmac_rsp_event:: mode[%d] error", mode);
        return -OAL_EFAIL;
    }

    oam_warning_log2(0, OAM_SF_ANY, "wal_ioctl_wait_hmac_rsp_event::mode[%d], lock[%d]",
                     mode, g_wal_hmac_wait_sync[mode].lock);

    /* 当前已经有相同mode在使用, 正忙 */
    if (g_wal_hmac_wait_sync[mode].lock == 1) {
        return -OAL_EBUSY;
    }

    memset_s(&g_wal_hmac_wait_sync[mode], sizeof(wal_hmac_wait_sync_stru), 0, sizeof(wal_hmac_wait_sync_stru));
    g_wal_hmac_wait_sync[mode].lock = 1;
    g_wal_hmac_wait_sync[mode].wal_hmac_sync_flag = OSAL_FALSE;

    leftime = osal_wait_timeout_interruptible(&hmac_vap->query_wait_q, wal_ioctl_wait_hmac_rsp_wait_condition,
        (osal_u8 *)&mode, time);
    if (leftime <= 0) {
        /* 超时或者定时器错误 */
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_wait_hmac_rsp_event::query wait for 5s error[%d]!}", leftime);
        ret = -OAL_EFAIL;
    } else {
        /* 正常结束  */
        oam_warning_log0(0, OAM_SF_ANY, "{wal_ioctl_wait_hmac_rsp_event::Waiting event coming!}");
        ret = OAL_SUCC;
        if (g_wal_hmac_wait_sync[mode].status == OSAL_FALSE) {
            ret = -OAL_EFAIL;
        }
    }

    /* 无论成功还是失败，lock均置位0，不阻碍下一次 */
    g_wal_hmac_wait_sync[mode].lock = 0;
    return ret;
}

osal_s32 wal_wait_hmac_sync_data(hmac_vap_stru *hmac_vap, frw_msg *msg)
{
    hmac_to_wal_sync_data_stru *sync_param = OSAL_NULL;
    osal_u8 mode_id;

    if ((msg->data == OSAL_NULL) || (hmac_vap == OSAL_NULL)) {
        oam_error_log0(0, OAM_SF_RX, "{wal_wait_hmac_sync_data::msg or hmac_vap is null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    sync_param = (hmac_to_wal_sync_data_stru *)msg->data;
    if (sync_param == OSAL_NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    mode_id = sync_param->mode_id;
    if (mode_id >= WAIT_SYNC_MODE_BUTT) {
        oam_error_log1(0, OAM_SF_SDP, "wal_wait_hmac_sync_data:: mode[%d] error", mode_id);
        return OAL_FAIL;
    }

    oam_warning_log2(0, OAM_SF_ANY, "wal_wait_hmac_sync_data::mode_id[%d], lock[%d]",
                     mode_id, g_wal_hmac_wait_sync[mode_id].lock);

    /* 为0 无需处理 预防有的接口ccpriv命令也调用, 导致此函数功能异常 */
    if (g_wal_hmac_wait_sync[mode_id].lock == 0) {
        wifi_printf("wal_wait_hmac_sync_data:: no handle\r\n");
        return OAL_SUCC;
    }

    oam_warning_log0(0, OAM_SF_SDP, "enter wal_wait_hmac_sync_data:: osal_wait_queue_wake_up_interrupt");
    g_wal_hmac_wait_sync[mode_id].status = sync_param->status;
    memcpy_s(g_wal_hmac_wait_sync[mode_id].data, SYNC_DATA_MAX, sync_param->data, SYNC_DATA_MAX);
    g_wal_hmac_wait_sync[mode_id].wal_hmac_sync_flag = OSAL_TRUE;
    osal_wait_wakeup(&hmac_vap->query_wait_q);
    return OAL_SUCC;
}

static osal_s32 uapi_iwconfig_set_scan(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *info, oal_iwreq_data_union *wrqu, osal_s8 *extra)
{
    osal_u16 len;
    osal_s32 ret;
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    struct iw_scan_req *req = OSAL_NULL;
    frw_msg msg_info = {0};

    if (net_dev == OSAL_NULL || info == OSAL_NULL || wrqu == OSAL_NULL ||
        extra == OSAL_NULL || net_dev->ml_priv == OSAL_NULL) {
        return -OAL_EFAUL;
    }

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{uapi_iwconfig_set_scan::hmac_vap is null!}");
        return -OAL_EFAUL;
    }
    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);

    if (wrqu->data.length == sizeof(struct iw_scan_req)) {
        req = (struct iw_scan_req *)extra;

        if (wrqu->data.flags & IW_SCAN_THIS_ESSID) { /* 证明填充了essid */
            memset_s(&g_scan_essid_info, sizeof(g_scan_essid_info), 0, sizeof(g_scan_essid_info));

            len = (req->essid_len > IW_ESSID_MAX_SIZE) ? IW_ESSID_MAX_SIZE : req->essid_len;
            g_scan_essid_info.essid_len = len;
            g_scan_essid_info.essid_enable = 1;
            memcpy_s(g_scan_essid_info.essid, WLAN_SSID_MAX_LEN, req->essid, len);
        } else if (wrqu->data.flags & IW_SCAN_HACK) { /* last参数, 取上一次的扫描结果 */
            return OAL_SUCC;
        }
    } else {
        memset_s(&g_scan_essid_info, sizeof(g_scan_essid_info), 0, sizeof(g_scan_essid_info));
    }

    msg_info.data = (osal_u8 *)&g_scan_essid_info;
    msg_info.data_len = (osal_u16)sizeof(g_scan_essid_info);
    ret = send_sync_cfg_to_host(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_IWLIST_SCAN, &msg_info);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_ioctl_iw_set_scan::send_sync_cfg_to_host err %d!}", ret);
        return ret;
    }

    /* 由于全频段全信道扫描, 给与扫描时间，等待扫描结束，防止get下发无数据 */
    ret = wal_ioctl_wait_hmac_rsp_event(hmac_vap, WAIT_HMAC_RSP_TIME, IWLIST_SET_SCAN_MODE);
    if (ret != OAL_SUCC) {
        wifi_printf("wal_ioctl_iw_set_scan::wal_ioctl_wait_hmac_rsp_event\r\n");
        return ret;
    }

    return OAL_SUCC;
}

static hmac_bss_mgmt_stru *wal_ioctl_scan_get_bss_mgmt(oal_net_device_stru *net_dev)
{
    hmac_vap_stru *hmac_vap = OSAL_NULL;
    hmac_device_stru *hmac_device = OSAL_NULL;
    mac_chip_stru *mac_chip = OSAL_NULL;
    hmac_scan_record_stru *record = OSAL_NULL;

    if (net_dev == OSAL_NULL) {
        return OSAL_NULL;
    }

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap == OSAL_NULL) {
        return OSAL_NULL;
    }

    mac_chip = mac_res_get_mac_chip(hmac_vap->chip_id);
    if (mac_chip == OSAL_NULL) {
        return OSAL_NULL;
    }

    /* 获取hmac device和扫描运行记录 */
    hmac_device = hmac_res_get_mac_dev_etc(hmac_vap->device_id);
    if (hmac_device == OSAL_NULL) {
        return OSAL_NULL;
    }

    /* 判断device是否正在进行测量 */
    if (hmac_device->scan_mgmt.is_scanning == OSAL_TRUE) {
        wifi_printf("--------------------------------------waiting\r\n");
        return OSAL_NULL;
    }

    record = &(hmac_device->scan_mgmt.scan_record_mgmt);
    return &(record->bss_mgmt);
}

static osal_s32 uapi_iwconfig_get_scan(oal_net_device_stru *net_dev,
    oal_iw_request_info_stru *info, oal_iwreq_data_union *wrqu, osal_s8 *extra)
{
    osal_s32 ret = OAL_SUCC;
    osal_s8 *ev = OSAL_NULL;
    osal_s8 *stop = OSAL_NULL;
    struct osal_list_head *entry = OSAL_NULL;
    hmac_bss_mgmt_stru *bss_mgmt = OSAL_NULL;
    hmac_scanned_bss_info *scanned_bss = OSAL_NULL;
    mac_bss_dscr_stru *bss_dscr = OSAL_NULL;
    osal_u16 len;

    if (net_dev == OSAL_NULL || info == OSAL_NULL || wrqu == OSAL_NULL || extra == OSAL_NULL) {
        return -OAL_EFAUL;
    }

    wal_record_wifi_external_log(WLAN_WIFI_IW_HANDLERS, __func__);

    ev = extra;
    stop = ev + wrqu->data.length;

    /* 获取扫描结果的管理结构地址 */
    bss_mgmt = wal_ioctl_scan_get_bss_mgmt(net_dev);
    if (bss_mgmt == OSAL_NULL) {
        return -OAL_EBUSY;
    }

    /* 遍历扫描到的bss信息并打印至proc文件 */
    osal_list_for_each(entry, &(bss_mgmt->bss_list_head)) {
        if ((stop - ev) < SCAN_ITEM_SIZE) {
            ret = -E2BIG; /* iwlist 源码返回错误码，会申请更多得空间 */
            break;
        }

        scanned_bss = osal_list_entry(entry, hmac_scanned_bss_info, dlist_head);
        if (scanned_bss == OSAL_NULL) {
            return OAL_SUCC;
        }

        bss_dscr = &(scanned_bss->bss_dscr_info);
        if (bss_dscr == OSAL_NULL) {
            return OAL_SUCC;
        }

        /* bss结构体数据在一段时间内是不断累加的, 因此若有过滤需要过滤 */
        len = (WLAN_SSID_MAX_LEN - 1 > g_scan_essid_info.essid_len) ? g_scan_essid_info.essid_len :
               WLAN_SSID_MAX_LEN - 1;
        if (g_scan_essid_info.essid_enable == 1 &&
            osal_strncmp(bss_dscr->ac_ssid, len, g_scan_essid_info.essid, len) != 0) {
            continue;
        }

        ev = iwlist_translate_scan(bss_dscr, (struct iw_request_info *)&info, ev, stop);
    }

    /* 预防驱动超出iwlist大小限制, 不再返回-E2BIG申请空间 */
    if ((ret == -E2BIG) && wrqu->data.length > IW_SCAN_DER_LIMIT_BUFF_MAX) {
        wifi_printf("lwlist scan info size limit 4096 * 8\r\n");
        ret = OAL_SUCC;
    }

    wrqu->data.length = ev - extra;
    wrqu->data.flags = 0;
    return ret;
}
#endif

/*****************************************************************************
  标准ioctl命令函数表.
*****************************************************************************/
OAL_STATIC OAL_CONST oal_iw_handler g_ast_iw_handlers[] = {
    OAL_PTR_NULL,                               /* SIOCSIWCOMMIT, */
    (oal_iw_handler)uapi_iwconfig_get_iwname,       /* SIOCGIWNAME, */
    OAL_PTR_NULL,                               /* SIOCSIWNWID, */
    OAL_PTR_NULL,                               /* SIOCGIWNWID, */
    (oal_iw_handler)uapi_iwconfig_set_freq,         /* SIOCSIWFREQ, 设置频点/信道 */
    (oal_iw_handler)uapi_iwconfig_get_freq,         /* SIOCGIWFREQ, 获取频点/信道 */
    (oal_iw_handler)uapi_iwconfig_set_bss_type,     /* SIOCSIWMODE, 设置bss type */
    (oal_iw_handler)uapi_iwconfig_get_bss_type,     /* SIOCGIWMODE, 获取bss type */
    OAL_PTR_NULL,                               /* SIOCSIWSENS, */
    (oal_iw_handler)uapi_iwconfig_get_iwsense,      /* SIOCGIWSENS, */
    OAL_PTR_NULL,                               /* SIOCSIWRANGE, */ /* not used */
    (oal_iw_handler)uapi_iwconfig_get_iwrange,      /* SIOCGIWRANGE, */
    OAL_PTR_NULL,                               /* SIOCSIWPRIV, */  /* not used */
    OAL_PTR_NULL,                               /* SIOCGIWPRIV, */  /* kernel code */
    OAL_PTR_NULL,                               /* SIOCSIWSTATS, */ /* not used */
    OAL_PTR_NULL,                               /* SIOCGIWSTATS, */
    OAL_PTR_NULL,                               /* SIOCSIWSPY, */
    OAL_PTR_NULL,                               /* SIOCGIWSPY, */
    OAL_PTR_NULL,                               /* -- hole -- */
    OAL_PTR_NULL,                               /* -- hole -- */
    OAL_PTR_NULL,                               /* SIOCSIWAP, */
    (oal_iw_handler)uapi_iwconfig_get_apaddr,       /* SIOCGIWAP, */
    OAL_PTR_NULL,                               /* SIOCSIWMLME, */
    OAL_PTR_NULL,                               /* SIOCGIWAPLIST, */
#ifdef _PRE_WLAN_WIRELESS_EXT
    (oal_iw_handler)uapi_iwconfig_set_scan,     /* SIOCSIWSCAN, */
    (oal_iw_handler)uapi_iwconfig_get_scan,     /* SIOCGIWSCAN, */
#else
    OAL_PTR_NULL,                               /* SIOCSIWSCAN, */
    OAL_PTR_NULL,                               /* SIOCGIWSCAN, */
#endif
    (oal_iw_handler)uapi_iwconfig_set_essid,    /* SIOCSIWESSID, 设置ssid */
    (oal_iw_handler)uapi_iwconfig_get_essid,        /* SIOCGIWESSID, 读取ssid */
    OAL_PTR_NULL,                               /* SIOCSIWNICKN */
    OAL_PTR_NULL,                               /* SIOCGIWNICKN */
    OAL_PTR_NULL,                               /* -- hole -- */
    OAL_PTR_NULL,                               /* -- hole -- */
    OAL_PTR_NULL,                               /* SIOCSIWRATE */
    (oal_iw_handler)uapi_iwconfig_get_iwrate,       /* SIOCGIWRATE */
    OAL_PTR_NULL,                               /* SIOCSIWRTS */
    (oal_iw_handler)uapi_iwconfig_get_rtsthres,     /* SIOCGIWRTS */
    OAL_PTR_NULL,                               /* SIOCSIWFRAG */
    (oal_iw_handler)uapi_iwconfig_get_fragthres,    /* SIOCGIWFRAG */
    (oal_iw_handler)uapi_iwconfig_set_txpower,      /* SIOCSIWTXPOW, 设置传输功率限制 */
    (oal_iw_handler)uapi_iwconfig_get_txpower,      /* SIOCGIWTXPOW, 设置传输功率限制 */
    OAL_PTR_NULL,                               /* SIOCSIWRETRY */
    OAL_PTR_NULL,                               /* SIOCGIWRETRY */
    OAL_PTR_NULL,                               /* SIOCSIWENCODE */
    (oal_iw_handler)uapi_iwconfig_get_iwencode,     /* SIOCGIWENCODE */
    OAL_PTR_NULL,                               /* SIOCSIWPOWER */
    OAL_PTR_NULL,                               /* SIOCGIWPOWER */
    OAL_PTR_NULL,                               /* -- hole -- */
    OAL_PTR_NULL,                               /* -- hole -- */
    OAL_PTR_NULL,                               /* SIOCSIWGENIE */
    OAL_PTR_NULL,                               /* SIOCGIWGENIE */
    OAL_PTR_NULL,                               /* SIOCSIWAUTH */
    OAL_PTR_NULL,                               /* SIOCGIWAUTH */
    OAL_PTR_NULL,                               /* SIOCSIWENCODEEXT */
    OAL_PTR_NULL                                /* SIOCGIWENCODEEXT */
};

/*****************************************************************************
  私有ioctl命令参数定义定义
*****************************************************************************/
OAL_STATIC OAL_CONST oal_iw_priv_args_stru g_ast_iw_priv_args[] = {
    /* 参数类型是char, 个数为24 */
    {WAL_IOCTL_PRIV_SET_MODE,       OAL_IW_PRIV_TYPE_CHAR | 24, OAL_IW_PRIV_TYPE_CHAR | 24, "mode"},
    {WAL_IOCTL_PRIV_GET_MODE,       0, OAL_IW_PRIV_TYPE_CHAR | 24, "get_mode"}, /* 参数类型是char, 个数为24 */
    {WAL_IOCTL_PRIV_SET_COUNTRY,    OAL_IW_PRIV_TYPE_CHAR | 3, 0,  "setcountry"},
    {WAL_IOCTL_PRIV_GET_COUNTRY,    0, OAL_IW_PRIV_TYPE_CHAR | 3,  "getcountry"},
    {WAL_IOCTL_PRIV_SET_AP_CFG, OAL_IW_PRIV_TYPE_CHAR |  256, 0, "AP_SET_CFG"},
    {WAL_IOCTL_PRIV_AP_MAC_FLTR, OAL_IW_PRIV_TYPE_CHAR | 256, OAL_IW_PRIV_TYPE_CHAR | OAL_IW_PRIV_SIZE_FIXED | 0,
     "AP_SET_MAC_FLTR"},
    {WAL_IOCTL_PRIV_AP_GET_STA_LIST, 0, OAL_IW_PRIV_TYPE_CHAR | 1024, "AP_GET_STA_LIST"},
    {WAL_IOCTL_PRIV_AP_STA_DISASSOC, OAL_IW_PRIV_TYPE_CHAR | 256, OAL_IW_PRIV_TYPE_CHAR | 0, "AP_STA_DISASSOC"},

    /* sub-ioctl函数入口 */
    {WAL_IOCTL_PRIV_SETPARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "setparam"},
    {
        WAL_IOCTL_PRIV_GETPARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
        OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "getparam"
    },

    /* sub-ioctl标志，name为'\0', 数字1表示set命令后面跟1个参数, get命令得到1个值 */
    {WAL_IOCTL_PRIV_SETPARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, ""},
    {WAL_IOCTL_PRIV_SETPARAM,       OAL_IW_PRIV_TYPE_BYTE | OAL_IW_PRIV_SIZE_FIXED | OAL_IW_PRIV_TYPE_ADDR, 0, ""},
    {WAL_IOCTL_PRIV_GETPARAM,       0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "" },
    {WAL_IOCTL_PRIV_GETPARAM,       0, OAL_IW_PRIV_TYPE_BYTE | OAL_IW_PRIV_SIZE_FIXED | OAL_IW_PRIV_TYPE_ADDR, ""},
    {WLAN_MSG_W2H_CFG_SHORTGI,            OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "shortgi20"},
    {WLAN_MSG_W2H_CFG_GET_SHORTGI,            0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_shortgi20"},
    {WLAN_MSG_W2H_CFG_SHORTGI_FORTY,      OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "shortgi40"},
    {WLAN_MSG_W2H_CFG_GET_SHORTGI_FORTY,      0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_shortgi40"},
    {WLAN_MSG_W2H_CFG_SHORTGI_EIGHTY,     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "shortgi80"},
    {WLAN_MSG_W2H_CFG_GET_SHORTGI_EIGHTY,     0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_shortgi80"},

    {WLAN_MSG_W2H_CFG_SHORT_PREAMBLE,     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "shpreamble"},
    {WLAN_MSG_W2H_CFG_GET_SHORT_PREAMBLE,     0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_shpreamble"},
    {WLAN_MSG_W2H_CFG_GET_CURRENT_CHANEL,     0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "getchannel"},
    {WLAN_MSG_W2H_CFG_PROT_MODE,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "protmode"},
    {WLAN_MSG_W2H_CFG_GET_PROT_MODE,          0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_protmode"},
    {WLAN_MSG_W2H_CFG_AUTH_MODE,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "authmode"},
    {WLAN_MSG_W2H_CFG_GET_AUTH_MODE,          0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_authmode"},
    {WLAN_MSG_W2H_CFG_NO_BEACON,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "nobeacon"},
    {WLAN_MSG_W2H_CFG_GET_NO_BEACON,          0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_nobeacon"},
    {WLAN_MSG_W2H_CFG_DTIM_PERIOD,        OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "dtim_period"},
    {WLAN_MSG_W2H_CFG_GET_DTIM_PERIOD,        0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_dtim_period"},

    /* EDCA参数配置命令 sub-ioctl入口 */
    {WAL_IOCTL_PRIV_SET_WMM_PARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 3, 0, "setwmmparam"},
    {
        WAL_IOCTL_PRIV_GET_WMM_PARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2,
        OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, "getwmmparam"
    },

    /* sub-ioctl标志，name为'\0', 2表示set命令后跟两个参数 */
    {WAL_IOCTL_PRIV_SET_WMM_PARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, ""},
    {
        WAL_IOCTL_PRIV_GET_WMM_PARAM,       OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
        OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, ""
    },
    {WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_CWMIN,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "cwmin"},
    {WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_CWMIN,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_cwmin"},
    {WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_CWMAX,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "cwmax"},
    {WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_CWMAX,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_cwmax"},
    {WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_AIFSN,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "aifsn"},
    {WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_AIFSN,          OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_aifsn"},
    {WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_TXOP_LIMIT,     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "txoplimit"},
    {WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_TXOP_LIMIT,     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_txoplimit"},
    {WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_MSDU_LIFETIME,  OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "lifetime"},
    {WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_MSDU_LIFETIME,  OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_lifetime"},
    {WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_MANDATORY,      OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "mandatory"},
    {WLAN_MSG_W2H_CFG_GET_EDCA_TABLE_MANDATORY,      OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_mandatory"},
    {WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_CWMIN,         OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "qcwmin"},
    {WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_CWMIN,         OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_qcwmin"},
    {WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_CWMAX,         OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "qcwmax"},
    {WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_CWMAX,         OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_qcwmax"},
    {WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_AIFSN,         OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "qaifsn"},
    {WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_AIFSN,         OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_qaifsn"},
    {WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_TXOP_LIMIT,   OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "qtxoplimit"},
    {WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_TXOP_LIMIT,   OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_qtxoplimit"},
    {WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_MSDU_LIFETIME, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "qlifetime"},
    {WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_MSDU_LIFETIME, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_qlifetime"},
    {WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_MANDATORY,    OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 2, 0, "qmandatory"},
    {WLAN_MSG_W2H_CFG_GET_QEDCA_TABLE_MANDATORY,    OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1,
     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_qmandatory"},
#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
    /* 使能proxy arp */
    {WLAN_MSG_W2H_CFG_PROXYARP_EN,   OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "proxyarp_en"},
#endif

    {WLAN_MSG_W2H_CFG_ADD_BLACK_LIST,     OAL_IW_PRIV_TYPE_CHAR | 40, 0, "blkwhtlst_add"},
    {WLAN_MSG_W2H_CFG_DEL_BLACK_LIST,     OAL_IW_PRIV_TYPE_CHAR | 40, 0, "blkwhtlst_del"},
    {WLAN_MSG_W2H_CFG_CLR_BLACK_LIST,     OAL_IW_PRIV_TYPE_CHAR | 40, 0, "blkwhtlst_clr"},
    /* 配置黑名单模式 */
    {WLAN_MSG_W2H_CFG_BLACKLIST_MODE,     OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, 0, "blkwhtlst_mode"},
    /* 黑名单模式打印 */
    {WLAN_MSG_W2H_CFG_GET_BLACKLIST_MODE,     0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_blkwhtmode"},

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    {WLAN_MSG_W2H_CFG_GET_2040BSS_SW,         0, OAL_IW_PRIV_TYPE_INT | OAL_IW_PRIV_SIZE_FIXED | 1, "get_obss_sw"},
#endif
};

/*****************************************************************************
  私有ioctl命令函数表.
*****************************************************************************/
OAL_STATIC OAL_CONST oal_iw_handler g_ast_iw_priv_handlers[] = {
    (oal_iw_handler)uapi_iwpriv_set_param,                /* SIOCWFIRSTPRIV+0 */  /* sub-ioctl set 入口 */
    (oal_iw_handler)uapi_iwpriv_get_param,                /* SIOCWFIRSTPRIV+1 */  /* sub-ioctl get 入口 */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+2 */  /* setkey */
    (oal_iw_handler)uapi_iwpriv_set_wme_params,           /* SIOCWFIRSTPRIV+3 */  /* setwmmparams */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+4 */  /* delkey */
    (oal_iw_handler)uapi_iwpriv_get_wme_params,           /* SIOCWFIRSTPRIV+5 */  /* getwmmparams */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+6 */  /* setmlme */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+7 */  /* getchaninfo */
    (oal_iw_handler)uapi_iwpriv_set_country,               /* SIOCWFIRSTPRIV+8 */  /* setcountry */
    (oal_iw_handler)uapi_iwpriv_get_country,               /* SIOCWFIRSTPRIV+9 */  /* getcountry */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+10 */  /* addmac */
    (oal_iw_handler)uapi_iwpriv_get_param_char,           /* SIOCWFIRSTPRIV+11 */  /* getscanresults */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+12 */  /* delmac */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+13 */  /* getchanlist */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+14 */  /* setchanlist */
    (oal_iw_handler)uapi_iwpriv_set_param_char,           /* SIOCWFIRSTPRIV+15 */  /* setmac */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+16 */  /* chanswitch */
    (oal_iw_handler)uapi_iwpriv_get_mode,                 /* SIOCWFIRSTPRIV+17 */  /* 获取模式, 例: iwpriv vapN get_mode */
    (oal_iw_handler)uapi_iwpriv_set_mode,                 /* SIOCWFIRSTPRIV+18 */  /* 设置模式, 例: iwpriv vapN mode 11g */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+19 */  /* getappiebuf */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+20 */  /* null */
    (oal_iw_handler)uapi_iwpriv_get_assoc_list,           /* SIOCWFIRSTPRIV+21 */  /* APUT取得关联STA列表 */
    (oal_iw_handler)uapi_iwpriv_set_mac_filters,          /* SIOCWFIRSTPRIV+22 */  /* APUT设置STA过滤 */
    (oal_iw_handler)uapi_iwpriv_set_ap_config,            /* SIOCWFIRSTPRIV+23 */  /* 设置APUT参数 */
    (oal_iw_handler)uapi_iwpriv_set_ap_sta_disassoc,      /* SIOCWFIRSTPRIV+24 */  /* APUT去关联STA */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+25 */  /* getStatistics */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+26 */  /* sendmgmt */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+27 */  /* null */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+28 */  /* null */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+29 */  /* null */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+30 */  /* sethbrparams */
    OAL_PTR_NULL,                                       /* SIOCWFIRSTPRIV+31 */  /* setrxtimeout */
};

/*****************************************************************************
  无线配置iw_handler_def定义
*****************************************************************************/

oal_iw_handler_def_stru g_st_iw_handler_def_etc = {
    .standard           = g_ast_iw_handlers,
    .num_standard       = OAL_ARRAY_SIZE(g_ast_iw_handlers),
#ifdef CONFIG_WEXT_PRIV
    .private            = g_ast_iw_priv_handlers,
    .num_private        = OAL_ARRAY_SIZE(g_ast_iw_priv_handlers),
    .private_args       = g_ast_iw_priv_args,
    .num_private_args   = OAL_ARRAY_SIZE(g_ast_iw_priv_args),
#endif
    .get_wireless_stats = OAL_PTR_NULL
};

/*****************************************************************************
 功能描述  : 获取iwpriv命令get_args使用的buff长度
 输入参数  : cmd
 返 回 值  : buff长度

 修改历史      :
  1.日    期   : 2017年10月30日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  wal_iwpriv_get_args_buff_len(wlan_cfgid_enum_uint16 cmd)
{
    osal_u32    i;
    osal_s32     l_buff_len = 0;
    for (i = 0; i < (OAL_SIZEOF(g_ast_iw_priv_args) / OAL_SIZEOF(g_ast_iw_priv_args[0])); i++) {
        if (cmd == g_ast_iw_priv_args[i].cmd) {
            l_buff_len = g_ast_iw_priv_args[i].get_args & OAL_IW_PRIV_SIZE_MASK;
            break;
        }
    }

    return l_buff_len;
}

/*****************************************************************************
 功能描述  : 获得收包数
 输入参数  : net_dev: net device
 输出参数  : p_param    : 输入字符串
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2017年06月29日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32 iwpriv_get_dscp_tid_map(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_void *p_param, osal_s8 *pc_extra)
{
    oal_iw_point_stru               *w = (oal_iw_point_stru *)p_param;
    hmac_vap_stru                   *hmac_vap;
    osal_s32                        l_ret_len;
    osal_u8                        dscp;
    osal_s32                        l_str_left;

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (OAL_PTR_NULL == hmac_vap) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_ioctl_get_sta_info::dev->ml_priv, return null!}");
        return -OAL_EINVAL;
    }

    l_str_left = wal_iwpriv_get_args_buff_len(WLAN_MSG_W2H_CFG_GET_DSCP_TID_MAP);
    if (0 == l_str_left) {
        return -OAL_FAIL;
    }

    l_ret_len = snprintf_s(pc_extra, l_str_left, l_str_left, "\n-- DSCP TID map --\nDSCP:TID:\n");
    if ((l_ret_len < 0) || (l_ret_len > l_str_left - 1)) {
        oam_warning_log1(0, OAM_SF_ANY, "vap_id[%d] no memery to get tid_dscp_map", hmac_vap->vap_id);
        return -OAL_FAIL;
    } else {
        w->length = (osal_u16)l_ret_len;
        pc_extra      += l_ret_len;
        l_str_left    -= l_ret_len;
    }

    for (dscp = 0; dscp < HMAC_MAX_DSCP_VALUE_NUM; dscp++) {
        if (HMAC_DSCP_VALUE_INVALID != hmac_vap->dscp_tid_map[dscp]) {
            l_ret_len = snprintf_s(pc_extra, l_str_left, l_str_left, "%2u->%u\n",
                dscp, hmac_vap->dscp_tid_map[dscp]);
            if ((l_ret_len < 0) || (l_ret_len > l_str_left - 1)) {
                break;
            } else {
                w->length += (osal_u16)l_ret_len;
                l_str_left    -= l_ret_len;
                pc_extra      += l_ret_len;
            }
        }
    }
    *pc_extra = 0;
    return OAL_SUCC;
}

/*****************************************************************************
 功能描述  : 获得芯片dieid
 输入参数  : net_dev: net device
 输出参数  : param    : 输入字符串
 返 回 值  : 错误码

 修改历史      :
  1.日    期   : 2017年08月04日
    修改内容   : 新生成函数

*****************************************************************************/
OAL_STATIC osal_s32  iwpriv_get_dieid(oal_net_device_stru *net_dev, oal_iw_request_info_stru *info,
    osal_void *param, osal_s8 *extra)
{
    osal_s32                       ret;
    oal_iw_point_stru              *w = (oal_iw_point_stru *)param;
    hmac_vap_stru                  *hmac_vap = OSAL_NULL;
    osal_s32                        ret_len;
    frw_msg                         cfg_info;
    osal_u8                       value = 0;
    mac_cfg_show_dieid_stru         fill_msg = {0};

    hmac_vap = (hmac_vap_stru *)net_dev->ml_priv;
    if (hmac_vap == OSAL_NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{iwpriv_get_dieid::dev->ml_priv, return null!}");
        return -OAL_EINVAL;
    }

    /***************************************************************************
                            抛事件到wal层处理
    ***************************************************************************/
    memset_s(&cfg_info, OAL_SIZEOF(cfg_info), 0, OAL_SIZEOF(cfg_info));
    cfg_msg_init((osal_u8 *)&value, 0, (osal_u8 *)&fill_msg, OAL_SIZEOF(fill_msg), &cfg_info);

    ret = send_sync_cfg_to_host(wal_util_get_vap_id(net_dev), WLAN_MSG_W2H_CFG_GET_DIEID, &cfg_info);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{iwpriv_get_dieid::return err code %d!}", ret);
        return ret;
    }

    ret_len = snprintf_s(extra, DIEID_EXTRA_LEN, DIEID_EXTRA_LEN, "\n");
    w->length = (osal_u16)ret_len;
    extra += ret_len;

    ret_len = snprintf_s(extra, (DIEID_EXTRA_LEN - w->length), (DIEID_EXTRA_LEN - w->length), "\nsuccess\n");
    if (ret_len < 0) {
        oam_warning_log1(0, OAM_SF_ANY, "{iwpriv_get_dieid::return err_len %d}", ret_len);
    }
    w->length += (osal_u16)ret_len;

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
