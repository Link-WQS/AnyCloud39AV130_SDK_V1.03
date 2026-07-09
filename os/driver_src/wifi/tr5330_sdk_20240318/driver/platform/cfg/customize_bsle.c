/*
 * Copyright (c) CompanyNameMagicTag. 2021-2023. All rights reserved.
 * Description: host customize bsle adapt source file
 * Author: CompanyName
 * Create: 2021-07-28
 */
#include <linux/kernel.h>
#include <linux/time.h>
#include "customize_bsle_ext.h"
#include "ini.h"
#include "oam_ext_if.h"
#include "oal_ext_if.h"
#include "oal_main.h"
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#include "plat_pm_board.h"
#include "hcc_if.h"
#include "hcc_host_if.h"
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
#include "diag_sample_data.h"
#endif
#if defined(CONFIG_HCC_SUPPORT_UART)
#include "hcc_adapt_uart.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_EXT_CUSTOMIZE_BSLE_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_BSLE_HOST

#undef BSLE_DEVICE_MSG_MAX_WAIT_TIME
#define BSLE_DEVICE_MSG_MAX_WAIT_TIME 2000

#undef BSLE_DEVICE_MSG_WAIT_TIME
#define BSLE_DEVICE_MSG_WAIT_TIME 10
#define BSLE_DATA_COLLECTION_CLOSE_FLAG 0

OAL_STATIC bsle_customize_stru g_al_bsle_cust_params[BSLE_CFG_INI_BUTT]; /* 定制化参数数组 */

OAL_STATIC bsle_cfg_cmd g_ast_bsle_config[] = {
    /* bt_coex_mode bit0 = 0 is chip out coex, bt_coex_mode bit0 = 1 is chip in coex */
    {"bt_coex_mode",                   BSLE_CFG_INI_BT_COEX_MODE},
    {"ble_disable_ll_privacy",         BSLE_CFG_INI_BLE_DISABLE_LL_PRIVACY},
    {"ble_tv_rc_scan_interval",        BSLE_CFG_INI_BLE_TV_RC_SCAN_INTERVAL},
    {"ble_tv_rc_scan_window",          BSLE_CFG_INI_BLE_TV_RC_SCAN_WINDOW},
    {"bsle_front_switch",              BSLE_CFG_INI_BSLE_FRONT_SWITCH},
    /* BT BLE tx max power level, range 1-7, about 4dB per level */
    {"bt_maxpower",                    BSLE_CFG_INI_BT_MAXPOWER},
    /* TX power reference value, range 0-10000, higher the value, higher the power, band 1-N(N is the value of
       bt_cali_txpwr_pa_ref_num) should configured */
    {"bt_cali_txpwr_pa_ref_num",       BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_NUM},
    {"bt_cali_txpwr_pa_ref_band1",     BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND1},
    {"bt_cali_txpwr_pa_ref_band2",     BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND2},
    {"bt_cali_txpwr_pa_ref_band3",     BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND3},
    {"bt_cali_txpwr_pa_ref_band4",     BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND4},
    {"bt_cali_txpwr_pa_ref_band5",     BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND5},
    {"bt_cali_txpwr_pa_ref_band6",     BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND6},
    {"bt_cali_txpwr_pa_ref_band7",     BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND7},
    {"bt_cali_txpwr_pa_ref_band8",     BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND8},
    /* TX power frequency, range 0-78, one-to-one with the power reference configuration above */
    {"bt_cali_txpwr_pa_fre1",          BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE1},
    {"bt_cali_txpwr_pa_fre2",          BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE2},
    {"bt_cali_txpwr_pa_fre3",          BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE3},
    {"bt_cali_txpwr_pa_fre4",          BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE4},
    {"bt_cali_txpwr_pa_fre5",          BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE5},
    {"bt_cali_txpwr_pa_fre6",          BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE6},
    {"bt_cali_txpwr_pa_fre7",          BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE7},
    {"bt_cali_txpwr_pa_fre8",          BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE8},
    /* TX power frequency, the channel of apply, one-to-one with the cali channel */
    {"bt_cali_txpwr_pa_fre_block1",    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK1},
    {"bt_cali_txpwr_pa_fre_block2",    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK2},
    {"bt_cali_txpwr_pa_fre_block3",    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK3},
    {"bt_cali_txpwr_pa_fre_block4",    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK4},
    {"bt_cali_txpwr_pa_fre_block5",    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK5},
    {"bt_cali_txpwr_pa_fre_block6",    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK6},
    {"bt_cali_txpwr_pa_fre_block7",    BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK7},
    /*  SRRC switch val = 1 ON,val = 0 OFF */
    {"bt_srrc_switch",                 BSLE_CFG_INI_BT_SRRC_SWITCH},
    /* TX power reference value, range 0-10000, higher the value, higher the power, band 1-N(N is the value of
       bt_cali_txpwr_pa_ref_num) should configured */
    {"bt_srrc_pa_ref_val1",            BSLE_CFG_INI_BT_SRRC_PA_REF_VAL1},
    {"bt_srrc_pa_ref_val2",            BSLE_CFG_INI_BT_SRRC_PA_REF_VAL2},
    {"bt_srrc_pa_ref_val3",            BSLE_CFG_INI_BT_SRRC_PA_REF_VAL3},
    {"bt_srrc_pa_ref_val4",            BSLE_CFG_INI_BT_SRRC_PA_REF_VAL4},
    {"bt_srrc_pa_ref_val5",            BSLE_CFG_INI_BT_SRRC_PA_REF_VAL5},
    {"bt_srrc_pa_ref_val6",            BSLE_CFG_INI_BT_SRRC_PA_REF_VAL6},
    {"bt_srrc_pa_ref_val7",            BSLE_CFG_INI_BT_SRRC_PA_REF_VAL7},
    {"bt_srrc_pa_ref_val8",            BSLE_CFG_INI_BT_SRRC_PA_REF_VAL8},
    /* TX power frequency, range 0-78, one-to-one with the power reference configuration above */
    {"bt_srrc_pa_fre1",                BSLE_CFG_INI_BT_SRRC_PA_FRE1},
    {"bt_srrc_pa_fre2",                BSLE_CFG_INI_BT_SRRC_PA_FRE2},
    {"bt_srrc_pa_fre3",                BSLE_CFG_INI_BT_SRRC_PA_FRE3},
    {"bt_srrc_pa_fre4",                BSLE_CFG_INI_BT_SRRC_PA_FRE4},
    {"bt_srrc_pa_fre5",                BSLE_CFG_INI_BT_SRRC_PA_FRE5},
    {"bt_srrc_pa_fre6",                BSLE_CFG_INI_BT_SRRC_PA_FRE6},
    {"bt_srrc_pa_fre7",                BSLE_CFG_INI_BT_SRRC_PA_FRE7},
    {"bt_srrc_pa_fre8",                BSLE_CFG_INI_BT_SRRC_PA_FRE8},
    {OAL_PTR_NULL, 0}
};

OAL_STATIC bool bsle_device_msg[BSLE_DEVICE_MSG_BUTT] = { 0 };

bool hbsle_hcc_customize_get_device_status(bsle_hcc_msg_c2h msg_to_host)
{
    oal_uint64 start = oal_get_time_stamp_from_timeval();
    oal_uint64 end;
    bool device_status;
    while ((device_status = bsle_device_msg[msg_to_host]) == false) {
        osal_msleep(BSLE_DEVICE_MSG_WAIT_TIME);
        end = oal_get_time_stamp_from_timeval();
        if (end >= start + BSLE_DEVICE_MSG_MAX_WAIT_TIME) {
            hcc_debug("get device status:%d,time out \n", msg_to_host);
            break;
        }
    }
    end = oal_get_time_stamp_from_timeval();
    hcc_debug("get device status:%d,result:%d,time:%lld \n", msg_to_host, device_status, end - start);
    return device_status;
}

td_void hbsle_hcc_customize_reset_device_status(void)
{
    memset_s(bsle_device_msg, sizeof(bool) * BSLE_DEVICE_MSG_BUTT, 0, sizeof(bool) * BSLE_DEVICE_MSG_BUTT);
}

// 释放给hcc发送消息的内核内存，回调由hcc来调用
OAL_STATIC td_void hcc_adapt_bsle_msg_free(hcc_queue_type queue_id, td_u8 *buf, td_u8 *user_param)
{
    oal_uint8 *tx_buf = (oal_uint8 *)buf;

    hcc_debug("enter:%s \n", __func__);
    if (user_param != TD_NULL) {
        osal_kfree(user_param);
    }
    if (buf != TD_NULL) {
        kfree(tx_buf);
    }
}

/* device向host发送的消息,hcc接收到原始消息需要申请32字节对齐的内存空间,*buf即是内存申请指向的空间 */
OAL_STATIC td_u32 hcc_adapt_bsle_msg_alloc(hcc_queue_type queue_id, td_u32 len, td_u8 **buf, td_u8 **user_param)
{
    *buf = (td_u8 *)osal_kmalloc(len, OSAL_GFP_ATOMIC);
    if (*buf == TD_NULL) {
        hcc_debug("failed\r\n");
        return EXT_ERR_FAILURE;
    }
    return EXT_ERR_SUCCESS;
}

#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
td_u32 hcc_adapt_bsle_msg_data_col_cmd_tx_proc(td_void *cmd, td_u32 len)
{
    oal_uint32 ret;
    oal_uint8 *tx_buf = NULL;
    oal_uint8 *data_buf = NULL;
    oal_uint8 *msg_tag_buf = NULL;
    hcc_transfer_param hcc_transfer_param = { 0 };
    bsle_msg_tag bsle_msg_tag = { 0 };
    diag_btsoc_sample_cmd *sample_cmd = (diag_btsoc_sample_cmd *)cmd;

    if (sample_cmd->flag == BSLE_DATA_COLLECTION_CLOSE_FLAG) {
        return OAL_FAIL;
    }

    if ((!pm_svc_open_state_get(PM_SVC_BLE)) && (!pm_svc_open_state_get(PM_SVC_SLE))) {
        return EXT_ERR_FAILURE;
    }

    tx_buf = kzalloc(BSLE_LARGE_NETBUF_SIZE, GFP_KERNEL);
    if (tx_buf == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hcc_adapt_bsle_msg_data_col_cmd_tx_proc: alloc tx_buf fail\n");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    // 发送给hcc的数据进行地址偏移，预留hcc消息头
    data_buf = tx_buf + hcc_get_head_len();
    msg_tag_buf = data_buf;
    // 发送给device侧的数据进行地址偏移，预留device侧识别消息类型的信元内存
    data_buf += sizeof(bsle_msg_tag);

    if ((len > BSLE_LARGE_NETBUF_SIZE - hcc_get_head_len() - sizeof(bsle_msg_tag)) || (len == 0)) {
        oam_error_log1(0, OAM_SF_CFG, "hcc_adapt_bsle_msg_data_col_cmd_tx_proc: got wrong ul_data_len[%d]\n", len);
        kfree(tx_buf);
        return OAL_FAIL;
    }
    oal_print_err("hcc_adapt_bsle_msg_data_col_cmd_tx_proc:%d\n", len);

    // 填充device侧识别消息类型的信元
    bsle_msg_tag.len = len;
    bsle_msg_tag.type = BSLE_MSG_HCC_TYPE_DATA_COLLECTION;

    if (memcpy_s(msg_tag_buf, sizeof(bsle_msg_tag), &bsle_msg_tag, sizeof(bsle_msg_tag)) != EOK) {
        oal_print_err("hcc_adapt_bsle_msg_data_col_cmd_tx_proc memcpy_s failed\n");
    }

    if (memcpy_s(data_buf, len, cmd, len) != EOK) {
        oal_print_err("hcc_adapt_bsle_msg_data_col_cmd_tx_proc memcpy_s failed\n");
    }

    hcc_transfer_param.service_type = HCC_ACTION_TYPE_BSLE_MSG;
    hcc_transfer_param.sub_type = 0;
    hcc_transfer_param.queue_id = BSLE_MSG_QUEUE;
    hcc_transfer_param.fc_flag = 0;
    hcc_transfer_param.user_param = NULL;

    ret = (oal_uint32)hcc_tx_data(HCC_CHANNEL_AP, tx_buf, BSLE_LARGE_NETBUF_SIZE, &hcc_transfer_param);
    if (unlikely(ret != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_CFG,
            "hcc_adapt_bsle_msg_data_col_cmd_tx_proc fail ret[%d] buf[%p]\n", ret, (uintptr_t)data_buf);
        kfree(tx_buf);
    }
    return ret;
}
#endif
OAL_STATIC td_u32 hcc_adapt_bsle_msg_rx_proc(hcc_queue_type queue_id, td_u8 sub_type, td_u8 *buf, td_u32 len,
    td_u8 *user_param)
{
    bsle_msg_tag *tag = NULL;
    uint8_t *data = NULL;
    bsle_hcc_msg_c2h *device_msg;
    uint8_t *res;
    if (buf == NULL) {
        return EXT_ERR_FAILURE;
    }

    data = buf + hcc_get_head_len();
    tag = (bsle_msg_tag *)data;
    data += sizeof(bsle_msg_tag);
    device_msg = (bsle_hcc_msg_c2h *)data;

    hcc_debug("start hcc_adapt_bsle_msg_rx_proc,type:%d,device_msg:%d \n", tag->type, *device_msg);

    if (tag->type == BSLE_MSG_HCC_TYPE_DEVICE_STATUS) {
        bsle_device_msg[*device_msg] = true;
    } else if (tag->type == BSLE_MSG_HCC_TYPE_DATA_COLLECTION) {
        res = (uint8_t *)data;
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
        diag_cmd_report_sample_data(res, tag->len);
#endif
    }
    osal_kfree(buf);
    if (user_param != NULL) {
        osal_kfree(user_param);
    }
    return EXT_ERR_SUCCESS;
}

OAL_STATIC hcc_adapt_ops g_hcc_bsle_msg_adapt = {
    .free = hcc_adapt_bsle_msg_free,
    .alloc = hcc_adapt_bsle_msg_alloc,
    .start_subq = TD_NULL,
    .stop_subq = TD_NULL,
    .rx_proc = hcc_adapt_bsle_msg_rx_proc,
};

OAL_STATIC osal_s32 hbsle_config_init_custom(void)
{
    td_s32 bsle_cfg_id;
    td_s32 ret = INI_FAILED;
    (osal_void)memset_s(g_al_bsle_cust_params, sizeof(g_al_bsle_cust_params), 0, sizeof(g_al_bsle_cust_params));
    for (bsle_cfg_id = 0; bsle_cfg_id < BSLE_CFG_INI_BUTT; bsle_cfg_id++) {
        /* 获取bsle的配置值 */
        ret = get_cust_conf_int32_etc(INI_MODU_DEV_BT, g_ast_bsle_config[bsle_cfg_id].name,
            &(g_al_bsle_cust_params[bsle_cfg_id].val));
        if (INI_FAILED == ret) {
            g_al_bsle_cust_params[bsle_cfg_id].value_state = OAL_FALSE;
            continue;
        }
        g_al_bsle_cust_params[bsle_cfg_id].value_state = OAL_TRUE;
    }

    oam_warning_log0(0, OAM_SF_CFG, "hbsle_config_init_private_custom read from ini success!\n");
    return INI_SUCC;
}

// 此处先开发对应的接口，后续定制化字段需要和device侧及前段同事进行确认
OAL_STATIC oal_void hbsle_custom_fill_ini_param(bfgn_bt_customization_stru *bsle_custom_data, td_s32 bsle_cfg_id,
    oal_int32 val)
{
    #define BSLE_CFG_PA_HIGH_16BITS 16 // for PA, high 16bits = freq ,low 16 bits = ref
    switch (bsle_cfg_id) {
        case BSLE_CFG_INI_BT_COEX_MODE:
            bsle_custom_data->bt_coex_mode = val;
            break;
        case BSLE_CFG_INI_BLE_DISABLE_LL_PRIVACY:
            bsle_custom_data->ble_disable_ll_privacy = val;
            break;
        case BSLE_CFG_INI_BLE_TV_RC_SCAN_INTERVAL:
            bsle_custom_data->ble_tv_rc_scan_interval = val;
            break;
        case BSLE_CFG_INI_BLE_TV_RC_SCAN_WINDOW:
            bsle_custom_data->ble_tv_rc_scan_window = val;
            break;
        case BSLE_CFG_INI_BSLE_FRONT_SWITCH:
            bsle_custom_data->bsle_front_switch = val;
            break;
        case BSLE_CFG_INI_BT_MAXPOWER:
            bsle_custom_data->bt_maxpower = val;
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_NUM:
            bsle_custom_data->bt_20dbm_txpwr_cali_num = val;
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND1:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref1 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND2:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref2 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND3:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref3 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND4:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref4 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND5:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref5 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND6:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref6 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND7:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref7 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_REF_BAND8:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref8 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE1:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref1 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE2:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref2 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE3:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref3 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE4:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref4 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE5:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref5 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE6:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref6 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE7:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref7 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE8:
            bsle_custom_data->bt_20dbm_txpwr_cali_freq_ref8 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK1:
            bsle_custom_data->bt_20dbm_txpwr_cali_chl1_chl2 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK2:
            bsle_custom_data->bt_20dbm_txpwr_cali_chl1_chl2 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK3:
            bsle_custom_data->bt_20dbm_txpwr_cali_chl3_chl4 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK4:
            bsle_custom_data->bt_20dbm_txpwr_cali_chl3_chl4 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK5:
            bsle_custom_data->bt_20dbm_txpwr_cali_chl5_chl6 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK6:
            bsle_custom_data->bt_20dbm_txpwr_cali_chl5_chl6 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_CALI_TXPWR_PA_FRE_BLOCK7:
            bsle_custom_data->bt_20dbm_txpwr_cali_chl7 = val;
            break;
        case BSLE_CFG_INI_BT_SRRC_SWITCH:
            bsle_custom_data->bt_srrc_switch = val;
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_REF_VAL1:
            bsle_custom_data->bt_srrc_freq_ref1 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_REF_VAL2:
            bsle_custom_data->bt_srrc_freq_ref2 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_REF_VAL3:
            bsle_custom_data->bt_srrc_freq_ref3 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_REF_VAL4:
            bsle_custom_data->bt_srrc_freq_ref4 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_REF_VAL5:
            bsle_custom_data->bt_srrc_freq_ref5 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_REF_VAL6:
            bsle_custom_data->bt_srrc_freq_ref6 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_REF_VAL7:
            bsle_custom_data->bt_srrc_freq_ref7 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_REF_VAL8:
            bsle_custom_data->bt_srrc_freq_ref8 |= (val & 0xffff);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_FRE1:
            bsle_custom_data->bt_srrc_freq_ref1 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_FRE2:
            bsle_custom_data->bt_srrc_freq_ref2 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_FRE3:
            bsle_custom_data->bt_srrc_freq_ref3 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_FRE4:
            bsle_custom_data->bt_srrc_freq_ref4 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_FRE5:
            bsle_custom_data->bt_srrc_freq_ref5 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_FRE6:
            bsle_custom_data->bt_srrc_freq_ref6 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_FRE7:
            bsle_custom_data->bt_srrc_freq_ref7 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        case BSLE_CFG_INI_BT_SRRC_PA_FRE8:
            bsle_custom_data->bt_srrc_freq_ref8 |= (val << BSLE_CFG_PA_HIGH_16BITS);
            break;
        default:
            oam_error_log1(0, OAM_SF_CFG, "unknown bsle cfg id:[%d]\n", bsle_cfg_id);
    }
}

OAL_STATIC oal_void hbsle_custom_adapt_device_ini_param(bfgn_bt_customization_stru *bsle_custom_data)
{
    oal_int32 bsle_cfg_id;
    bsle_customize_stru temp_para;
    if (NULL == bsle_custom_data) {
        oam_error_log0(0, OAM_SF_CFG, "{hbsle_custom_adapt_device_ini_param::bsle_custom_data is NULL.}\n");
        return;
    }

    // 填充给bsle的device侧发送协商好的结构体
    for (bsle_cfg_id = 0; bsle_cfg_id < BSLE_CFG_INI_BUTT; bsle_cfg_id++) {
        temp_para = g_al_bsle_cust_params[bsle_cfg_id];
        if (OAL_FALSE == temp_para.value_state) {
            continue;
        }
        oam_error_log2(0, OAM_SF_CFG, "bsle cfg id is %d, val is %d\n", bsle_cfg_id, temp_para.val);
        hbsle_custom_fill_ini_param(bsle_custom_data, bsle_cfg_id, temp_para.val);
    }
}

static td_void hbsle_hcc_custom_param_init(hcc_transfer_param *param)
{
    param->service_type = HCC_ACTION_TYPE_BSLE_MSG;
    param->sub_type = 0;
    param->queue_id = BSLE_MSG_QUEUE;
    param->fc_flag = 0;
    param->user_param = NULL;
}

static td_s32 hbsle_hcc_custom_txbuf_alloc(td_u32 buf_len, td_u8 **pst_buf, td_u8 **data_buf)
{
#if defined(CONFIG_HCC_SUPPORT_UART)
    hcc_handler *hcc = hcc_get_handler(HCC_CHANNEL_AP);
#endif
    td_u8 *tx_buf;
    tx_buf = kzalloc(buf_len, GFP_KERNEL);
    if (tx_buf == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "hbsle_hcc_custom_ini_data_buf::alloc tx_buf fail\n");
        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }

    // 发送给hcc的数据进行地址偏移，预留hcc消息头
    *data_buf = tx_buf + hcc_get_head_len();
#if defined(CONFIG_HCC_SUPPORT_UART)
    if (hcc->bus->bus_type == HCC_BUS_UART) {
        *data_buf = *data_buf + sizeof(uart_packet_head);
    }
#endif
    *pst_buf = tx_buf;
    return OAL_SUCC;
}

td_s32 hbsle_hcc_custom_ini_data_buf(void)
{
    size_t data_len = sizeof(bfgn_bt_customization_stru);
    oal_int32 ret;
    oal_uint8 *tx_buf, *data_buf, *msg_tag_buf;
    hcc_transfer_param hcc_transfer_param = { 0 };
    bsle_msg_tag bsle_msg_tag = { 0 };
    bfgn_bt_customization_stru bsle_custom_data = { 0 };
    td_u32 buf_len = BSLE_LARGE_NETBUF_SIZE;
#if defined(CONFIG_HCC_SUPPORT_UART)
    hcc_handler *hcc = hcc_get_handler(HCC_CHANNEL_AP);
    if (hcc->bus->bus_type == HCC_BUS_UART) {
        buf_len += sizeof(uart_packet_head) + 1;
    }
#endif
    ret = hbsle_hcc_custom_txbuf_alloc(buf_len, &tx_buf, &data_buf);
    if (ret != OAL_SUCC) {
        return ret;
    }
    msg_tag_buf = data_buf;
    // 发送给device侧的数据进行地址偏移，预留device侧识别消息类型的信元内存
    data_buf += sizeof(bsle_msg_tag);
    hbsle_custom_adapt_device_ini_param(&bsle_custom_data);

    if ((data_len > BSLE_LARGE_NETBUF_SIZE - hcc_get_head_len() - sizeof(bsle_msg_tag)) || (data_len == 0)) {
        oam_error_log1(0, OAM_SF_CFG, "hbsle_hcc_custom_ini_data_buf::got wrong ul_data_len[%d]\n", data_len);
        kfree(tx_buf);
        return OAL_FAIL;
    }
    oal_print_err("***hbsle_hcc_custom_ini_data_buf:%zu ***********\n", data_len);

    // 填充device侧识别消息类型的信元
    bsle_msg_tag.len = data_len;
    bsle_msg_tag.type = BSLE_MSG_HCC_TYPE_CUSTOMIZE_DATA;
    ret = memcpy_s(msg_tag_buf, sizeof(bsle_msg_tag), &bsle_msg_tag, sizeof(bsle_msg_tag));
    if (ret != EOK) {
        oam_warning_log0(0, "memcpy_s failed ret[%d]. \r\n", ret);
        kfree(tx_buf);
        return ret;
    }

    ret = memcpy_s(data_buf, data_len, &bsle_custom_data, data_len);
    if (ret != EOK) {
        oam_warning_log0(0, "memcpy_s failed ret[%d]. \r\n", ret);
        kfree(tx_buf);
        return ret;
    }

    hbsle_hcc_custom_param_init(&hcc_transfer_param);
    ret = (oal_uint32)hcc_tx_data(HCC_CHANNEL_AP, tx_buf, buf_len, &hcc_transfer_param);
    if (unlikely(ret != OAL_SUCC)) {
        oam_error_log2(0, OAM_SF_CFG, "hbsle_hcc_custom_ini_data_buf fail ret[%d] buf[%p]\n", ret, (uintptr_t)data_buf);
        kfree(tx_buf);
    }
    return ret;
}

int hbsle_hcc_customize_h2d_data_cfg(void)
{
    oal_int32 ret;

    ret = hcc_service_init(HCC_CHANNEL_AP, HCC_ACTION_TYPE_BSLE_MSG, &g_hcc_bsle_msg_adapt);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "initial hcc bsle message service failed, ret is %d\n", ret);
        return OAL_FAIL;
    }

    /* bsle上电时重读定制化配置，配置数值先保存在全局变量中，读取和发送分开 */
    ret = hbsle_config_init_custom();
    if (OAL_SUCC != ret) {
        oam_warning_log1(0, OAM_SF_CFG, "hbsle_hcc_customize_h2d_data_cfg data ret[%d]\n", ret);
    }

    /* 构建定制化数据的结构体，发送此数据给hcc */
    ret = hbsle_hcc_custom_ini_data_buf();
    if (unlikely(OAL_SUCC != ret)) {
        oam_error_log1(0, OAM_SF_CFG, "hbsle_hcc_customize_h2d_data_cfg priv ini data fail, ret[%d]\n", ret);
        return OAL_FAIL;
    }

    return INI_SUCC;
}

EXPORT_SYMBOL(hbsle_hcc_customize_h2d_data_cfg);
EXPORT_SYMBOL(hbsle_hcc_custom_ini_data_buf);
EXPORT_SYMBOL(hbsle_hcc_customize_get_device_status);
EXPORT_SYMBOL(hbsle_hcc_customize_reset_device_status);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
