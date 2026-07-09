/*
 * Copyright (c) CompanyNameMagicTag. 2021-2023. All rights reserved.
 * Description: host customize wlan adapt source file
 * Author: CompanyName
 * Create: 2021-07-28
 */

#include "customize_wifi.h"
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/etherdevice.h>
#include "ini.h"
#include "oam_ext_if.h"
#include "oal_ext_if.h"
#include "oal_types.h"
#include "hcc_sdio_host.h"
#include "oal_main.h"
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#include "plat_pm_board.h"
#include "hcc_if.h"
#include "plat_hcc_srv.h"
#include "reg/c_union_define_cmu_ctl_rb.h"
#include "reg/cmu_ctl_rb_reg_offset.h"
#include "efuse_opt.h"
#include "osal_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(_PRE_PRODUCT_ID_MP0X_HOST)

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE

#undef  THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_COMMON_CUSTOMIZE_WIFI_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST

#define DEFAULT_HOST2DEV_SCATT_MAX       64
#define WAL_EXTERMAL_RECORD_FILE_PATH "external_record_file_path"
#define XO_TRIM_TEMP_COMP_NUM 8   // 频偏粗调温补值 20℃一档 ~-20,0,20,40,60,80,100,~
#define XO_TRIM_TEMP_COMP_TIMER_PERIOD 2000   // 频偏粗调温补定时器间隔 ms
#define XO_TRIM_COARSE_VAL_MAX 15   // 频偏粗调补偿最大值
osal_s32 g_plat_host_init_params_etc[WLAN_PLAT_CFG_INIT_BUTT] = {0};      /* ini定制化参数数组 */
osal_timer g_xo_trim_temp_timer = {0};
osal_u8 g_xo_trim_temp_comp_sw = 0;      /* 频偏温度补偿开关 */
OAL_STATIC plat_cfg_cmd g_plat_config_cmds[] = {
    /* 性能 */
    {"bus_d2h_sched_count",             WLAN_CFG_INIT_BUS_D2H_SCHED_COUNT},
    {"bus_h2d_sched_count",             WLAN_CFG_INIT_BUS_H2D_SCHED_COUNT},
    /* 平台低功耗 */
    {"device_plt_pm_enable",            WLAN_CFG_INIT_DEVICE_PLT_PM_ENABLE},
    {"device_awake_host_gpio_idx",      WLAN_CFG_INIT_DEVICE_AWAKE_HOST_GPIO_IDX},
    {"device_awake_host_gpio_level",    WLAN_CFG_INIT_DEVICE_AWAKE_HOST_GPIO_LEVEL},
    {OAL_PTR_NULL, 0}
};

OAL_STATIC osal_void host_plat_params_init_first(osal_void)
{
    /* 性能 */
    g_plat_host_init_params_etc[WLAN_CFG_INIT_BUS_D2H_SCHED_COUNT]           = HCC_SDIO_DEV2HOST_SCATT_MAX;
    g_plat_host_init_params_etc[WLAN_CFG_INIT_BUS_H2D_SCHED_COUNT]           = HCC_SDIO_HOST2DEV_SCATT_MAX;
    /* 平台低功耗 */
    g_plat_host_init_params_etc[WLAN_CFG_INIT_DEVICE_PLT_PM_ENABLE] = 1;
    g_plat_host_init_params_etc[WLAN_CFG_INIT_DEVICE_AWAKE_HOST_GPIO_IDX] = 0;
    g_plat_host_init_params_etc[WLAN_CFG_INIT_DEVICE_AWAKE_HOST_GPIO_LEVEL] = 0;
}

osal_s32 hwifi_get_plat_init_value_etc(osal_s32 cus_tag, osal_s32 cfg_id)
{
    osal_s32              *pgal_params = OAL_PTR_NULL;
    osal_s32               plat_cfg_butt;

    if (cus_tag == CUS_TAG_INI) {
        pgal_params = &g_plat_host_init_params_etc[0];
        plat_cfg_butt = WLAN_PLAT_CFG_INIT_BUTT;
    } else {
        oam_error_log1(0, OAM_SF_ANY, "hwifi_get_plat_init_value_etc tag number[0x%2x] not correct!", cus_tag);
        return INI_FAILED;
    }

    if (cfg_id < 0 || cfg_id >= plat_cfg_butt) {
        oam_error_log2(0, OAM_SF_ANY,
            "hwifi_get_plat_init_value_etc cfg id[%d] out of range, max cfg id is:%d", cfg_id, (plat_cfg_butt - 1));
        return INI_FAILED;
    }

    return pgal_params[cfg_id];
}

/* device侧平台低功耗定制化参数下发 */

typedef struct {
    osal_u32 device_plt_pm_enable : 1;
    osal_u32 device_awake_host_gpio_idx : 8;
    osal_u32 device_awake_host_gpio_level : 1;
    osal_u32 rsv : 22;
} device_plt_pm_cfg_stru;

osal_u32 plat_msg_hcc_send(osal_u8 *buf, osal_u16 len, osal_u16 hcc_plat_msg_id);
osal_void hiwifi_custom_adapt_plt_pm_device_ini_param(osal_void)
{
    device_plt_pm_cfg_stru cfg = {0};

    // 平台低功耗使能配置
    cfg.device_plt_pm_enable = hwifi_get_plat_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DEVICE_PLT_PM_ENABLE);
    if (cfg.device_plt_pm_enable != OAL_FALSE && cfg.device_plt_pm_enable != OAL_TRUE) {
        oam_error_log1(0, OAM_SF_CFG,
            "hiwifi_custom_adapt_plt_pm_device_ini_param::plt_pm_enable [%u] is invalid, return! \n",
            cfg.device_plt_pm_enable);
        return;
    }

    // device唤醒host GPIO管脚配置
    cfg.device_awake_host_gpio_idx =
        hwifi_get_plat_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DEVICE_AWAKE_HOST_GPIO_IDX);
    cfg.device_awake_host_gpio_level =
        hwifi_get_plat_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_DEVICE_AWAKE_HOST_GPIO_LEVEL);
    if (cfg.device_awake_host_gpio_level != OAL_FALSE && cfg.device_awake_host_gpio_level != OAL_TRUE) {
        oam_error_log1(0, OAM_SF_CFG,
            "hiwifi_custom_adapt_plt_pm_device_ini_param::device_awake_host_gpio_level [%u] is invalid, return! \n",
            cfg.device_awake_host_gpio_level);
        return;
    }

    oam_warning_log3(0, OAM_SF_CFG, "hiwifi_custom_adapt_plt_pm_device_ini_param::"
        "get ini plt_pm_enable[%u], gpio_idx[%u], gpio_lvl[%u] \n",
        cfg.device_plt_pm_enable, cfg.device_awake_host_gpio_idx, cfg.device_awake_host_gpio_level);
    plat_msg_hcc_send((osal_u8 *)&cfg, sizeof(cfg), H2D_PLAT_CFG_MSG_PM_INI);
}

typedef struct {
    td_u32 patch_not_writed;
    td_u32 panic_heap_choose;
} dfr_cfg_ini;

osal_s32 hwifi_config_plat_init_etc(osal_s32 cus_tag)
{
    osal_s32               cfg_id;
    osal_s32               ret = INI_FAILED;
    osal_s32               ori_val;
    plat_cfg_cmd       *pgast_wifi_config;
    osal_s32              *pgal_params;
    osal_s32               cfg_value = 0;
    osal_s32               plat_cfg_butt;

    switch (cus_tag) {
        case CUS_TAG_INI:
            host_plat_params_init_first();
            pgast_wifi_config = g_plat_config_cmds;
            pgal_params = g_plat_host_init_params_etc;
            plat_cfg_butt = WLAN_PLAT_CFG_INIT_BUTT;
            break;
        default:
            oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_init_etc tag number[0x%x] not correct!", cus_tag);
            return INI_FAILED;
    }

    for (cfg_id = 0; cfg_id < plat_cfg_butt; cfg_id++) {
        /* 获取ini的配置值 */
        ret = get_cust_conf_int32_etc(INI_MODU_PLAT, pgast_wifi_config[cfg_id].name, &cfg_value);
        if (ret == INI_FAILED) {
            oam_warning_log2(0, OAM_SF_CUSTOM,
                "hwifi_config_plat_init_etc read ini file cfg_id[%d]tag[%d] not exist!", cfg_id, cus_tag);
            continue;
        }
        /* no need to consider the sequence of ini cmd */
        ori_val = pgal_params[pgast_wifi_config[cfg_id].case_entry];
        pgal_params[pgast_wifi_config[cfg_id].case_entry] = cfg_value;
    }

    return INI_SUCC;
}

osal_void hwifi_custom_set_xo_trim_temp_comp_sw(osal_u8 sw)
{
    if (sw == OSAL_TRUE) {
        g_xo_trim_temp_comp_sw = sw;
    } else {
        g_xo_trim_temp_comp_sw = 0;
    }
}
osal_u8 hwifi_custom_get_xo_trim_temp_comp_sw(osal_void)
{
    return g_xo_trim_temp_comp_sw;
}
osal_void hwifi_config_get_cmu_xo_trim_temp_comp_sw(osal_void)
{
    osal_s32 ret;
    osal_u8 ini_str[INI_READ_VALUE_LEN] = {0};
    osal_u8 comp_sw = 0;
    osal_u8 param_len = 0;

    ret = get_cust_conf_string_etc(INI_MODU_PLAT, "cmu_xo_trim_temp_sw", ini_str, INI_READ_VALUE_LEN);
    if (ret != INI_SUCC) {
        oam_warning_log0(0, OAM_SF_CUSTOM, "{get_cmu_xo_trim_temp_comp:find cmu_xo_trim fail.}");
        return;
    }
    ret = ini_get_cust_item_list(ini_str, &comp_sw, INI_PARAM_BYTE_ONE, &param_len, INI_READ_VALUE_COUNT);
    if (ret != INI_SUCC || param_len != 1) {
        oam_error_log1(0, OAM_SF_CUSTOM, "get_cmu_xo_trim_temp_comp fail ret[%d]!", ret);
        return;
    }
    hwifi_custom_set_xo_trim_temp_comp_sw(comp_sw);
}
static osal_s32 hwifi_config_get_efuse_temp_gear(osal_u8 *temp_gear)
{
    osal_u64 state = 0;
    if (uapi_efuse_get_lockstat(&state) != OAL_SUCC) {
        oam_error_log0(0, 0, "{get_cmu_xo_trim_temp_comp, get efuse stat fail}");
        return INI_FAILED;
    }
    if ((((osal_u64)1 << (EXT_EFUSE_LOCK_XO_PPM_1_ID - EXT_EFUSE_LOCK_CHIP_ID)) & state) != 0) {
        if (uapi_efuse_read(EXT_EFUSE_XO_PPM_TEMPERATURE_2_ID, temp_gear, sizeof(osal_u8)) != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_CUSTOM, "get_cmu_xo_trim_temp_comp:uapi_efuse_read failed.");
            return INI_FAILED;
        }
    } else if ((((osal_u64)1 << (EXT_EFUSE_LOCK_CMU_XO_TRIM_COARSE_ID - EXT_EFUSE_LOCK_CHIP_ID)) & state) != 0) {
        if (uapi_efuse_read(EXT_EFUSE_XO_PPM_TEMPERATURE_1_ID, temp_gear, sizeof(osal_u8)) != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_CUSTOM, "get_cmu_xo_trim_temp_comp:uapi_efuse_read failed.");
            return INI_FAILED;
        }
    }
    return INI_SUCC;
}
osal_void hwifi_config_get_cmu_xo_trim_temp_comp(osal_void)
{
    osal_s32 ret;
    osal_u8 ini_str[INI_READ_VALUE_LEN] = {0};
    osal_u8 cmu_xo_trim_temp_param[XO_TRIM_TEMP_COMP_NUM] = {0};
    osal_u8 param_len = 0;
    osal_s16 xo_trim_coarse;
    osal_s16 xo_trim_diff;
    osal_s16 xo_trim_temp;
    osal_u8 temp_gear = 7;  // efuse保存温度档位 10℃一档 7档表示30~40
    osal_u8 i;
    u_cmu_xo_sig val;

    ret = get_cust_conf_string_etc(INI_MODU_PLAT, "cmu_xo_trim_temp", ini_str, INI_READ_VALUE_LEN);
    if (ret != INI_SUCC) {
        oam_warning_log0(0, OAM_SF_CUSTOM, "{get_cmu_xo_trim_temp_comp:find cmu_xo_trim fail.}");
        return;
    }
    ret = ini_get_cust_item_list(ini_str, cmu_xo_trim_temp_param, INI_PARAM_BYTE_ONE, &param_len, INI_READ_VALUE_COUNT);
    if (ret != INI_SUCC || param_len != XO_TRIM_TEMP_COMP_NUM) {
        oam_error_log1(0, OAM_SF_CUSTOM, "get_cmu_xo_trim_temp_comp fail ret[%d]!", ret);
        return;
    }

    if (hcc_read_reg(HCC_CHANNEL_AP, CMU_CTL_RB_CMU_XO_SIG_REG, &val.u32) != EXT_ERR_SUCCESS) {
        oam_warning_log0(0, OAM_SF_CUSTOM, "get_cmu_xo_trim_temp_comp read_reg fail");
        return;
    }

    if (hwifi_config_get_efuse_temp_gear(&temp_gear) != INI_SUCC) {
        return;
    }

    xo_trim_coarse = (osal_u8)val.bits.rg_cmu_xo_trim_coarse;
    // 更新为补偿值索引 20℃一档
    temp_gear = ((temp_gear >> 1) >= XO_TRIM_TEMP_COMP_NUM) ? XO_TRIM_TEMP_COMP_NUM - 1 : (temp_gear >> 1);
    xo_trim_diff = (osal_s16)cmu_xo_trim_temp_param[temp_gear] - xo_trim_coarse;
    for (i = 0; i < XO_TRIM_TEMP_COMP_NUM; i++) {
        xo_trim_temp = (osal_s16)cmu_xo_trim_temp_param[i] - xo_trim_diff; // 取值范围0-15
        xo_trim_temp = (xo_trim_temp < 0) ? 0 : (xo_trim_temp > XO_TRIM_COARSE_VAL_MAX) ?
            XO_TRIM_COARSE_VAL_MAX : xo_trim_temp;
        cmu_xo_trim_temp_param[i] = xo_trim_temp;
    }
    plat_msg_hcc_send(cmu_xo_trim_temp_param, sizeof(cmu_xo_trim_temp_param), H2D_PLAT_CFG_MSG_XO_TRIM_TEMP_INI);
}

osal_void hwifi_update_cmu_xo_trim_temp_comp_timer_handler(uintptr_t data)
{
    osal_u8 temp_gear = 7;  // efuse保存温度档位 10℃一档 7档表示30~40

    plat_msg_hcc_send((osal_u8 *)&temp_gear, sizeof(temp_gear), H2D_PLAT_CFG_MSG_UPDATE_XO_TRIM);
    osal_timer_mod(&g_xo_trim_temp_timer, XO_TRIM_TEMP_COMP_TIMER_PERIOD);
}

osal_void hwifi_update_cmu_xo_trim_temp_comp_timer(osal_void)
{
    // 定时更新
    g_xo_trim_temp_timer.handler = hwifi_update_cmu_xo_trim_temp_comp_timer_handler;
    g_xo_trim_temp_timer.interval = XO_TRIM_TEMP_COMP_TIMER_PERIOD;
    osal_timer_init(&g_xo_trim_temp_timer);
    osal_timer_start(&g_xo_trim_temp_timer);
}

osal_u32 ini_get_cmu_xo_trim(osal_void)
{
    osal_s32 ret;
    osal_u8 ini_str[INI_READ_VALUE_LEN] = {0};
    osal_u8 cmu_xo_trim_param[WLAN_CMU_XO_TRIM_NUM] = {0};
    osal_u8 param_len = 0;

    ret = get_cust_conf_string_etc(INI_MODU_PLAT, "cmu_xo_trim", ini_str, INI_READ_VALUE_LEN);
    if (ret != INI_SUCC) {
        oam_warning_log0(0, OAM_SF_CUSTOM, "{hwifi_config_get_cmu_xo_trim:find cmu_xo_trim fail.}");
        return INI_FAILED;
    }
    ret = ini_get_cust_item_list(ini_str, cmu_xo_trim_param, INI_PARAM_BYTE_ONE, &param_len, INI_READ_VALUE_COUNT);
    if (ret != INI_SUCC || param_len != WLAN_CMU_XO_TRIM_NUM) {
        oam_error_log1(0, OAM_SF_CUSTOM, "hwifi_config_get_cmu_xo_trim fail ret[%d]!", ret);
        return INI_FAILED;
    }

    /*
     * 频率校正码值 ini eg: cmu_xo_trim=8,60;
     * bit0-bit7 = 8; bit8-bit15 = 60(0x3C);
     */
    return (osal_u32)(cmu_xo_trim_param[0] + (cmu_xo_trim_param[WLAN_CMU_XO_TRIM_NUM - 1] << 8));
}

osal_s32 hwifi_hcc_customize_h2d_plat_data_cfg(void)
{
    osal_s32 ret;

    ret = hwifi_config_plat_init_etc(CUS_TAG_INI);
    if (ret != INI_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG, "hwifi_hcc_customize_h2d_data_cfg data ret[%d]", ret);
    }

    // device侧平台低功耗定制化参数下发
    hiwifi_custom_adapt_plt_pm_device_ini_param();

    hwifi_config_get_cmu_xo_trim_temp_comp_sw();
    // 定制化开启频偏温度补偿时 刷新温补数据 并启动定时器 随温度刷新
    if (hwifi_custom_get_xo_trim_temp_comp_sw() == OSAL_TRUE) {
        hwifi_config_get_cmu_xo_trim_temp_comp();
        hwifi_update_cmu_xo_trim_temp_comp_timer();
    }
    // plat dfr定制化下发
    return INI_SUCC;
}

osal_void hwifi_custom_adapt_device_ini_perf_param(osal_u8 *data, osal_u32 *data_len)
{
    osal_u32 ret;
    hmac_to_dmac_cfg_custom_data_stru sync_msg = {0};
    bus_cus_config_stru cus_config = {0};

    if (data == OSAL_NULL) {
        return;
    }

    sync_msg.syn_id = CUSTOM_CFGID_INI_PERF_ID;

    cus_config.bus_d2h_sched_count = hwifi_get_plat_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_BUS_D2H_SCHED_COUNT);
    cus_config.bus_h2d_sched_count = hwifi_get_plat_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_BUS_H2D_SCHED_COUNT);

    if (cus_config.bus_h2d_sched_count < 1 || cus_config.bus_h2d_sched_count > HCC_SDIO_HOST2DEV_SCATT_MAX) {
        oam_error_log2(0, OAM_SF_ANY,
            "{hwifi_custom_adapt_device_ini_perf_param::bus_h2d_sched_count[%d] out of range(0,%d]}",
            cus_config.bus_h2d_sched_count, HCC_SDIO_HOST2DEV_SCATT_MAX);
    } else {
        hcc_set_tx_sched_count(HCC_CHANNEL_AP, cus_config.bus_h2d_sched_count);
    }

    if (cus_config.bus_d2h_sched_count < 1 || cus_config.bus_d2h_sched_count > HCC_SDIO_DEV2HOST_SCATT_MAX) {
        oam_error_log2(0, OAM_SF_ANY,
            "{hwifi_custom_adapt_device_ini_perf_param::sdio_assem_d2h[%d] out of range(0,%d]}",
            cus_config.bus_d2h_sched_count, HCC_SDIO_DEV2HOST_SCATT_MAX);
    }

    sync_msg.len = (osal_u32)sizeof(cus_config);

    ret = memcpy_s(data, CUSTOM_MSG_DATA_HDR_LEN, &sync_msg, CUSTOM_MSG_DATA_HDR_LEN);
    if (ret != EOK) {
        oam_warning_log0(0, "memcpy_s failed ret[%d]. \r\n", ret);
        return;
    }

    ret = memcpy_s(data + CUSTOM_MSG_DATA_HDR_LEN, sizeof(cus_config), &cus_config, sizeof(cus_config));
    if (ret != EOK) {
        oam_warning_log0(0, "memcpy_s failed ret[%d]. \r\n", ret);
        return;
    }

    *data_len += ((osal_u32)sizeof(cus_config) + CUSTOM_MSG_DATA_HDR_LEN);

    oam_warning_log1(0, OAM_SF_CFG, "{hwifi_custom_adapt_device_ini_perf_param::da_len[%d].}", *data_len);
}

EXPORT_SYMBOL_GPL(hwifi_custom_adapt_device_ini_perf_param);

#endif /* #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE */

#endif // #if defined(_PRE_PRODUCT_ID_MP0X_HOST)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
