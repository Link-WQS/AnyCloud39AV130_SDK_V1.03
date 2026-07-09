/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: host platform main source file
 * Author: Huanghe
 * Create: 2021-06-08
 */

#include "soc_errno.h"
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
#include "zdiag_adapt_layer.h"
#endif
#include "hcc_service.h"
#include "plat_pm_board.h"
#include "plat_pm_board_tr5330.h"
#include "plat_main.h"
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/reboot.h>
#if defined(_PRE_PLAT_SLP_UART_FORWARD) && (_PRE_PLAT_SLP_UART_FORWARD == 1)
#include "slp_hcc_forward.h"
#endif
#include "plat_pm.h"
#include "plat_pm_wlan.h"
#include "oal_kernel_file.h"
#include "ini.h"
#include "efuse_opt.h"
#include "mac_addr.h"
#include "plat_misc.h"
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
#include "zdiag_adapt_os.h"
#include "zdiag_local_log.h"
#endif
#endif

#include "soc_module.h"
#include "hcc_if.h"
#include "hcc_bus.h"
#include "soc_errno.h"
#include "hcc_test.h"
#include "hcc_cfg.h"
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
#include "hcc_diag_log.h"
#endif
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
#ifdef CONFIG_HCC_SUPPORT_SDIO
#include "hcc_sdio_host.h"
#endif
#endif
#include "plat_hcc_srv.h"
#ifdef _PRE_BSLE_GATEWAY
#include "coex_msg_adapt.h"
#endif
#include "oal_debug.h"
#if defined(_PRE_PLAT_SHA256SUM_CHECK) && (_PRE_PLAT_SHA256SUM_CHECK == 1)
#include "plat_sha256_calc.h"
#endif

#ifdef CONFIG_HCC_SUPPORT_UART
#include "hcc_uart_host.h"
#include "plat_firmware_uart.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef PLAT_LOG_TAG
#define PLAT_LOG_TAG "[plat_init_etc]"

/*****************************************************************************
  1 头文件包含
*****************************************************************************/

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLAT_MAIN_C

osal_s32 plat_exception_init(osal_void);
osal_void plat_exception_exit(osal_void);

volatile td_s32 g_plat_loglevel_etc = PLAT_LOG_INFO;
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
EXPORT_SYMBOL(g_plat_loglevel_etc);
#endif

td_s32 g_bug_on_enable_etc = BUG_ON_DISABLE;

osal_s32 plat_kernel_loglevel_get(void)
{
    return g_plat_loglevel_etc;
}

osal_void plat_kernel_loglevel_set(osal_s32 log_level)
{
    if (log_level > PLAT_LOG_DEBUG) {
        log_level = PLAT_LOG_DEBUG;
    }
    g_plat_loglevel_etc = log_level;
    oal_print_emerg("0:emerg log.\r\n");
    oal_print_alert("1:alert log.\r\n");
    oal_print_crit("2:crit log.\r\n");
    oal_print_err("3:err log.\r\n");
    oal_print_warning("4:warning log.\r\n");
    oal_print_notice("5:notice log.\r\n");
    oal_print_info("6:info log.\r\n");
    oal_print_dbg("7:dbg log.\r\n");
}

/* device平台低功耗使能控制 */
osal_u32 plat_pm_func_enable_switch(osal_s32 pm_switch)
{
    return hcc_send_message(HCC_CHANNEL_AP, (pm_switch == 0) ? H2D_MSG_WCPU_PM_DISABLE : H2D_MSG_WCPU_PM_ENABLE,
        HCC_ACTION_TYPE_TEST);
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static td_u32 host_plat_heartbeat_reply_process(td_u8 *cb_data)
{
    oal_print_err("receive heart_beat ack\n");
    /* Todo */
    return EXT_ERR_SUCCESS;
}

#endif

static td_u32 plat_hcc_msg_dev_bsp_ready_cb(td_u8 *param)
{
    oal_print_err("plat handle hcc msg dev_bsp_ready_cb. \n");
    pm_bsp_init_ready(param);
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    diag_hcc_conn_state_set(TD_TRUE);
#if defined(CONFIG_DIAG_SUPPORT_LOCAL_LOG)
    zdiag_local_log_cb_for_dev_bsp_ready();
#endif
#endif
#endif
    return OAL_SUCC;
}

static osal_void plat_hcc_register(osal_void)
{
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == TD_NULL) {
        hcc_printf_err_log("pm_data not init\n");
        return;
    }
    hcc_message_register(HCC_CHANNEL_AP, HCC_ACTION_TYPE_TEST, D2H_MSG_WLAN_READY, pm_device_init_ready,
        (osal_u8 *)pm_data);
    hcc_message_register(HCC_CHANNEL_AP, HCC_ACTION_TYPE_TEST, D2H_MSG_BSP_READY, plat_hcc_msg_dev_bsp_ready_cb,
        (osal_u8 *)pm_data);
    hcc_message_register(HCC_CHANNEL_AP, HCC_ACTION_TYPE_TEST, D2H_MSG_HEART_BEAT_ACK,
        host_plat_heartbeat_reply_process, (osal_u8 *)(hcc_get_handler(HCC_CHANNEL_AP)->bus));
#endif
}

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

#define KERNEL_LOGLEVEL_INI_STRING "kernel_log_level"

/*******************************************************************************
  函 数 名		: plat_custom_get_kern_loglevel
  功能描述		: 定制化参数中获取kernel loglevel配置
  输入参数		:  None
  输出参数		:  None
  返 回 值		:  None
*******************************************************************************/
td_u32 plat_custom_get_kern_loglevel(td_void)
{
    td_u32 ret = EXT_ERR_SUCCESS;
    td_char ini_string[DOWNLOAD_CHANNEL_LEN] = {0};
    td_s32 plat_kernel_log_level;

    ret = get_cust_conf_string_etc(INI_MODU_PLAT, KERNEL_LOGLEVEL_INI_STRING, ini_string, DOWNLOAD_CHANNEL_LEN);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("get ini_string[%s] custom configuration failed!\n", KERNEL_LOGLEVEL_INI_STRING);
        return EXT_ERR_FAILURE;
    }

    plat_kernel_log_level = oal_strtol(ini_string, NULL, 10); /* 10 进制 */
    if (plat_kernel_log_level < PLAT_LOG_EMERG || plat_kernel_log_level > PLAT_LOG_DEBUG) {
        oal_print_err("custom ini[plat kern log level]: %d, invalid, set default level!\n", plat_kernel_log_level);
        plat_kernel_loglevel_set(PLAT_LOG_DEBUG);
        return EXT_ERR_FAILURE;
    }

    plat_kernel_loglevel_set(plat_kernel_log_level);
    oal_print_err("custom ini[plat kern log level]: %d.\n", plat_kernel_loglevel_get());

    return ret;
}
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define PLAT_REBOOT_TYPE_INI_STRING "plat_reboot_type"

#define PLAT_REBOOT_TYPE_DEFAULT    0
#define PLAT_REBOOT_TYPE_BLE_WKUP   1

td_s32 g_plat_reboot_type = PLAT_REBOOT_TYPE_DEFAULT;

/* reboot处理回调 */
static td_s32 plat_reboot_cb(struct notifier_block *nb, td_ulong action, td_void *data)
{
    oal_print_warning("plat_reboot_cb: action[%lu]. \r\n", action);
    if (action == SYS_POWER_OFF) {
        // 关机操作时, 判断处理类型
        switch (g_plat_reboot_type) {
            case PLAT_REBOOT_TYPE_BLE_WKUP:
                pm_shutdown_process_host();
                break;
            case PLAT_REBOOT_TYPE_DEFAULT:
            default:
                tr5330_board_power_reset(TD_TRUE);
                tr5330_board_power_off();
                break;
        }
    } else {
        // 其他复位场景默认进行复位处理
        tr5330_board_power_reset(TD_TRUE);
        tr5330_board_power_off();
    }

    return 0;
}

static struct notifier_block nb = {
    .notifier_call = plat_reboot_cb,
};

static td_s32 plat_reboot_cb_register(td_void)
{
    td_s32 ret;
    ret = register_reboot_notifier(&nb);
    if (ret != 0) {
        oal_print_err("plat_reboot_cb_register::register notifier_call fail! \n");
    }
    return ret;
}

static td_void plat_reboot_cb_unregister(td_void)
{
    unregister_reboot_notifier(&nb);
}

td_u32 plat_custom_get_reboot_type(td_void)
{
    td_u32 ret = EXT_ERR_SUCCESS;
    td_char ini_string[DOWNLOAD_CHANNEL_LEN] = {0};

    // 获取
    ret = get_cust_conf_string_etc(INI_MODU_PLAT, PLAT_REBOOT_TYPE_INI_STRING, ini_string, DOWNLOAD_CHANNEL_LEN);
    if (ret != EXT_ERR_SUCCESS) {
        oal_print_err("get ini_string[%s] custom configuration failed!\n", PLAT_REBOOT_TYPE_INI_STRING);
        return EXT_ERR_FAILURE;
    }
    g_plat_reboot_type = oal_strtol(ini_string, NULL, 10); /* 10 进制 */

    return EXT_ERR_SUCCESS;
}

/* PM处理回调 */
static td_s32 plat_pm_cb(struct notifier_block *nb, td_ulong action, td_void *data)
{
    static td_u32 resume_cnt = 0;
    static td_u32 total_cnt = 0;

    oal_print_warning("plat_pm_cb: action[%lu]. \r\n", action);

    total_cnt++;
    switch (action) {
        case PM_SUSPEND_PREPARE:
            pm_suspend_process_host();
            break;
        case PM_POST_SUSPEND:
            resume_cnt++;
            // resume on 2nd trigger
            if (resume_cnt == 2) {
                resume_cnt = 0;
                oal_print_warning("plat_pm_cb: Resume! \r\n");
                pm_resume_process_host();
            }
            break;
        default:
            oal_print_warning("Unexcept action [%lu]. \r\n", action);
    }
    oal_print_warning("resume_cnt[%u], total_cnt[%u]. \r\n", resume_cnt, total_cnt);
    return 0;
}

static struct notifier_block pm_nb = {
    .notifier_call = plat_pm_cb,
};

static td_s32 plat_pm_cb_register(void)
{
    td_s32 ret;
    ret = register_pm_notifier(&pm_nb);
    if (ret != 0) {
        oal_print_err("plat_pm_cb_register: register notifier_call fail!\r\n");
    }
    return ret;
}

static void plat_pm_cb_unregister(void)
{
    unregister_pm_notifier(&pm_nb);
}
#endif

#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
oal_int32 plat_reset_reinit_etc(void)
{
    oal_int32 ret;

#ifdef CONFIG_HCC_SUPPORT_SDIO
    ret = sdio_rescan_device(hcc_get_channel_bus(HCC_CHANNEL_AP));
    if (ret < 0) {
        oal_print_err("plat_reset_reinit_etc::sdio_rescan_device fail, ret=[%d]. \n", ret);
        return ret;
    }
#endif

    ret = hcc_bus_reinit(hcc_get_channel_bus(HCC_CHANNEL_AP));
    if (ret < 0) {
        oal_print_err("plat_reset_reinit_etc::hcc_bus_reinit fail, ret=[%d]. \n", ret);
        return ret;
    }
    cali_dyn_mem_cfg();
    ret = firmware_download_etc();
    if (ret != OAL_SUCC) {
        oal_print_err("plat_reset_reinit_etc::firmware_download_etc fail, ret=[%d]. \n", ret);
        return ret;
    }
#ifdef CONFIG_HCC_SUPPORT_USB
    ret = hcc_usb_reload(hcc_get_channel_bus(HCC_CHANNEL_AP));
    if (ret < 0) {
        oal_print_err("plat_reset_reinit_etc::hcc_usb_reload fail, ret=[%d]. \n", ret);
        return ret;
    }
#endif
    hcc_switch_status(HCC_ON);
    hcc_force_update_queue_id(hcc_get_channel_bus(HCC_CHANNEL_AP), OSAL_TRUE);
    return OAL_SUCC;
}
#endif

td_s32 hcc_init_etc(td_void)
{
    hcc_channel_init channel_init;

#ifdef CONFIG_HCC_SUPPORT_SDIO
    channel_init.bus_type = HCC_BUS_SDIO;
#endif
#ifdef CONFIG_HCC_SUPPORT_USB
    channel_init.bus_type = HCC_BUS_USB;
#endif
#ifdef CONFIG_HCC_SUPPORT_UART
    channel_init.bus_type = HCC_BUS_UART;
#endif
    channel_init.queue_cfg = hcc_get_queue_cfg(&channel_init.queue_len);
    channel_init.channel_name = HCC_CHANNEL_AP;
    channel_init.unc_pool_size = 0;
    if (hcc_init(&channel_init) != TR_ERR_SUCCESS) {
        oal_print_err("hcc init: fail\r\n");
        return EXT_ERR_FAILURE;
    }
#ifdef CONFIG_HCC_SUPPORT_TEST
    hcc_test_init();
#endif
    return EXT_ERR_SUCCESS;
}

/*****************************************************************************
平台初始化函数总入口
*****************************************************************************/
oal_int32  plat_init_etc(void)
{
    oal_int32 ret;

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    ret = ini_cfg_init_etc();
    if (ret != OAL_SUCC) {
        goto board_init_fail;
    }
    plat_custom_get_kern_loglevel();
#endif

    ret = pm_board_init();
    if (ret != OAL_SUCC) {
        oal_print_err("plat_init_etc: hh503_board_init fail\r\n");
        goto board_init_fail;
    }
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
    zdiag_adapt_init();
#endif
    if (hcc_init_etc() != EXT_ERR_SUCCESS) {
        goto hcc_init_fail;
    }

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    // 注册linux的reboot回调
    plat_custom_get_reboot_type();
    plat_reboot_cb_register();
    plat_pm_cb_register();
#endif

#ifdef _PRE_BSLE_GATEWAY
    coex_bsle_hcc_msg_plat_host_to_device_init();
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(_PRE_PLAT_SLP_UART_FORWARD) && (_PRE_PLAT_SLP_UART_FORWARD == 1)
    slp_hcc_init();
#endif
#endif
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
    ret = hcc_diag_log_init();
    if (ret != OAL_SUCC) {
        oal_print_err("hcc_diag_log_init return error code: %d\r\n", ret);
        goto hcc_diag_init_fail;
    }
#endif
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    ret = low_power_init_etc();
    if (ret != OAL_SUCC) {
        oal_print_err("plat_init_etc: low_power_init_etc return error code: %d\r\n", ret);
        goto low_power_init_fail;
    }
    // plat_hcc_register需要放在pm模块初始化之后
    plat_hcc_register();
#if defined(CONFIG_PLAT_SUPPORT_DFR)
    if (plat_exception_init() != OAL_SUCC) {
        oal_print_err("plat_exception_init falied!\r\n");
    }
#endif
#endif
    plat_misc_init();
#if defined(_PRE_OS_VERSION_LITEOS) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    cali_dyn_mem_cfg();
    ret = hcc_bus_reinit(hcc_get_channel_bus(HCC_CHANNEL_AP));

    firmware_download_etc();
#ifdef CONFIG_HCC_SUPPORT_USB
    ret = hcc_usb_reload(hcc_get_channel_bus(HCC_CHANNEL_AP));
    if (ret < 0) {
        goto board_init_fail;
    }
#endif
    hcc_switch_status(HCC_ON);
#endif

    // 平台初始化完成后, 拉低Device
    tr5330_board_power_off();

    /* 启动完成后，输出打印 */
    oal_print_warning("plat_init_etc::succ!\n\n");

    return OAL_SUCC;

board_init_fail:
    plat_misc_exit();

low_power_init_fail:
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hcc_diag_log_exit();
#endif
hcc_diag_init_fail:
#endif
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(_PRE_PLAT_SLP_UART_FORWARD) && (_PRE_PLAT_SLP_UART_FORWARD == 1)
    slp_hcc_exit();
#endif
#endif

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    plat_pm_cb_unregister();
    plat_reboot_cb_unregister();
#endif
#ifdef CONFIG_HCC_SUPPORT_TEST
    hcc_test_deinit();
#endif
    hcc_deinit(HCC_CHANNEL_AP);
hcc_init_fail:
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
    zdiag_adapt_exit();
#endif

    pm_board_exit();
    return ret;
}

oal_void plat_exit_etc(oal_void)
{
    plat_misc_exit();
#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#if defined(CONFIG_PLAT_SUPPORT_DFR)
    plat_exception_exit();
#endif

    low_power_exit_etc();
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
    hcc_diag_log_exit();
#endif
#if defined(_PRE_PLAT_SLP_UART_FORWARD) && (_PRE_PLAT_SLP_UART_FORWARD == 1)
    slp_hcc_exit();
#endif

    plat_pm_cb_unregister();
    plat_reboot_cb_unregister();
#endif

#ifdef CONFIG_HCC_SUPPORT_TEST
    hcc_test_deinit();
#endif

    hcc_deinit(HCC_CHANNEL_AP);
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
    zdiag_adapt_exit();
#endif
    pm_board_exit();

    ini_cfg_exit_etc();

    oal_print_err("plat_exit_etc::succ!\n");
    return;
}

module_init(plat_init_etc);
module_exit(plat_exit_etc);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
