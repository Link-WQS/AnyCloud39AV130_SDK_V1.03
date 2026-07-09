/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: wal_linux_psd
 * Create: 2022-08-15
 */

#ifndef __WAL_LINUX_PSD_H__
#define __WAL_LINUX_PSD_H__

#ifdef _PRE_WLAN_FEATURE_PSD
/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "soc_zdiag.h"
#include "wal_linux_ccpriv.h"
#include "hmac_psd.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_PSD_H
#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

osal_void wal_psd_enable_callback(osal_u16 cmd_id, osal_void* cmd_param, osal_u16 cmd_param_size,
    DIAG_OPTION *option);
osal_s32 wal_psd_data_report(hmac_vap_stru *hmac_vap, frw_msg *msg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
#endif /* end of wal_psd.h */
