/*
 * Copyright (c) @CompanyNameMagicTag 2020-2022. All rights reserved.
 * Description: intrf_mode algorithm rom.
 */

#ifdef _PRE_WLAN_FEATURE_INTRF_MODE
#ifdef _PRE_WLAN_FEATURE_CCA_OPT

/*****************************************************************************
  1 头文件包含
 *****************************************************************************/
#include "hmac_alg_if.h"
#include "alg_cca_optimize.h"
#include "alg_intf_det.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_ALG_CCA_INTRF_MODE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

/*****************************************************************************
  1 全局变量定义
 *****************************************************************************/
osal_u8 g_cca_intrf_mode_switch = OSAL_FALSE;      /* 特殊场景CCA配置开关 */
osal_u8 g_cca_intrf_mode_hist_enable = OSAL_FALSE; /* 特殊场景CCA配置是否开启的历史状态 */

/*****************************************************************************
  2 函数实现
****************************************************************************/
#define ALG_CCA_OPT_ED_HIGH_20TH_LOW_INTRF_MODE_TH (-62)    /* CCA 20M检测门限在特殊干扰场景下的门限值 */
#define ALG_CCA_OPT_INTRF_MODE_USR_RSSI_LMT (-42)           /* CCA 20M检测门限在特殊干扰场景下的用户RSSI阈值 */
#define ALG_CCA_OPT_INTRF_MODE_USR_RSSI_MAX (0)             /* CCA 20M检测门限在特殊干扰场景下的用户RSSI最大有效值 */
#define ALG_CCA_OPT_INTRF_MODE_STEP_20 (20)                 /* CCA 20M检测门限在特殊干扰场景下的调整幅值 */

/*****************************************************************************
  3 宏定义
****************************************************************************/
/******************************************************************************
 功能描述  : 干扰场景优化配置
******************************************************************************/
osal_void alg_cca_set_intrf_mode_switch(osal_u8 cca_switch)
{
    g_cca_intrf_mode_switch = cca_switch;
}

/******************************************************************************
 功能描述  : 干扰场景优化配置
******************************************************************************/
osal_u32 alg_cca_intrf_mode_process(hmac_user_stru *hmac_user, mac_tx_ctl_stru *cb, hal_tx_txop_alg_stru *txop_alg)
{
    osal_s16 usr_rssi   = OAL_RSSI_INIT_MARKER;
    osal_s16 rssi_thrd  = OAL_RSSI_INIT_MARKER;
    osal_s8 rssi_limit = ALG_CCA_OPT_ED_HIGH_20TH_LOW_TH;
    alg_cca_opt_stru *cca_opt = OSAL_NULL;
    hal_to_dmac_device_stru *hal_device = hal_chip_get_hal_device();

    unref_param(hmac_user);
    unref_param(cb);
    unref_param(txop_alg);

    cca_opt = alg_cca_get_cca_opt();

    /* 优化配置关闭时不做处理 */
    if (g_cca_intrf_mode_switch == OSAL_FALSE) {
        if (g_cca_intrf_mode_hist_enable == OSAL_TRUE) {
            /* 恢复CCA参数 */
            g_cca_intrf_mode_hist_enable = OSAL_FALSE;
            alg_cca_opt_set_th_default(cca_opt, WLAN_BAND_2G);
            hal_set_ed_high_th(cca_opt->ed_high_20th_reg, cca_opt->ed_high_40th_reg, OSAL_TRUE);
            hal_set_cca_prot_th(cca_opt->sd_cca_20th_dsss, cca_opt->sd_cca_20th_ofdm);
        }
        return OAL_SUCC;
    }

    g_cca_intrf_mode_hist_enable = OSAL_TRUE;

    /* 无干扰时不做优化处理 */
    if ((alg_intf_det_get_curr_coch_intf_type(hal_device) == OSAL_FALSE) &&
        (alg_intf_det_get_curr_adjch_intf_type(hal_device) == HAL_ALG_INTF_DET_ADJINTF_NO)) {
        oam_warning_log0(0, OAM_SF_INTRF_MODE, "{alg_cca_intrf_mode_process::no coch adjch intrf, opt not needed.}");
        return OAL_SUCC;
    }

    /* 用户rssi 不在[-42, 0]之间时不做优化处理 */
    usr_rssi = oal_get_real_rssi(cca_opt->rx_mgmt_rssi);
    if (usr_rssi <= ALG_CCA_OPT_INTRF_MODE_USR_RSSI_LMT || usr_rssi > ALG_CCA_OPT_INTRF_MODE_USR_RSSI_MAX) {
        oam_warning_log1(0, OAM_SF_INTRF_MODE,
            "{alg_cca_intrf_mode_process::usr rssi[%d] smaller than -42, opt not needed.}", usr_rssi);
        return OAL_SUCC;
    }

    rssi_thrd = osal_max((usr_rssi - ALG_CCA_OPT_INTRF_MODE_STEP_20), rssi_limit);
    hal_set_ed_high_th(ALG_CCA_OPT_ED_HIGH_20TH_LOW_INTRF_MODE_TH, cca_opt->ed_high_40th_reg, OSAL_TRUE);
    hal_set_cca_prot_th((osal_s8)rssi_thrd, (osal_s8)rssi_thrd);
    oam_info_log2(0, OAM_SF_INTRF_MODE, "{alg_cca_intrf_mode_process::usr_rssi[%d] rssi_thrd[%d] set.}",
        usr_rssi, rssi_thrd);

    return OAL_SUCC;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* #ifdef _PRE_WLAN_FEATURE_CCA_OPT */
#endif /* #ifdef _PRE_WLAN_FEATURE_INTRF_MODE */