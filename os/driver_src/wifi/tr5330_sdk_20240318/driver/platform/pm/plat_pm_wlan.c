/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: host platform wlan source file
 * Author: Huanghe
 * Create: 2021-06-08
 */

/*****************************************************************************
  1 Header File Including
*****************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include <linux/irq.h>

#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/host.h>
#include <linux/gpio.h>
#include <linux/pm_wakeup.h>

#include "hcc_sdio_host.h"
#include "hcc.h"
#include "hcc_bus.h"

#include "plat_debug.h"
#include "plat_cali.h"
#include "plat_pm_board.h"
#include "plat_pm_board_tr5330.h"
#include "plat_pm.h"
#include "plat_pm_dfr.h"
#include "plat_hcc_srv.h"
#include "oam_ext_if.h"
#include "hcc_if.h"
#include "hcc_cfg_comm.h"
#include "customize_bsle_ext.h"
#include "plat_pm_wlan.h"
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
#include "diag_sample_data.h"
#endif
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "customize_wifi.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLAT_PM_WLAN_C

struct wlan_pm_s* g_wlan_pm_info = OAL_PTR_NULL;

static pm_wlan_suspend_cb g_pm_wifi_suspend_cb_host = NULL;
static pm_wlan_resume_cb g_pm_wifi_resume_cb_host = NULL;

static pm_ble_suspend_cb g_pm_ble_suspend_cb_host = NULL;
static pm_ble_resume_cb g_pm_ble_resume_cb_host = NULL;

static pm_wow_wkup_cb g_pm_wow_wkup_cb_host = NULL;

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static RAW_NOTIFIER_HEAD(wifi_pm_chain);
#endif

#if defined(_PRE_MULTI_CORE_MODE) && defined(_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC)
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_uint8 g_uc_custom_cali_done_etc = OAL_FALSE;
EXPORT_SYMBOL_GPL(g_uc_custom_cali_done_etc);
#endif
#endif
#endif
oal_bool_enum g_wlan_pm_switch_etc = OSAL_TRUE;
EXPORT_SYMBOL_GPL(g_wlan_pm_switch_etc);
oal_uint8 g_wlan_fast_check_cnt = 0x6;
EXPORT_SYMBOL_GPL(g_wlan_fast_check_cnt);

void pm_resume_process_host_work_etc(struct work_struct *work);
osal_s32 pm_svc_open(osal_u8 service, osal_bool is_insmod);
osal_s32 pm_svc_close(osal_u8 service, osal_u8 is_rmmod);

/* 获取全局wlan结构 */
struct wlan_pm_s*  wlan_pm_get_drv_etc(oal_void)
{
    return g_wlan_pm_info;
}
EXPORT_SYMBOL_GPL(wlan_pm_get_drv_etc);

void wlan_pm_para_init(struct wlan_pm_s *pst_wlan_pm)
{
    if (pst_wlan_pm == NULL) {
        return;
    }

    /* 初始化芯片自检work */
    pst_wlan_pm->pm_enable = OAL_FALSE;
    pst_wlan_pm->pm_power_state = POWER_STATE_SHUTDOWN;
    pst_wlan_pm->pm_svc_state[PM_SVC_WLAN] = PM_SVC_STATE_SHUTDOWN;
    pst_wlan_pm->pm_svc_state[PM_SVC_BLE] = PM_SVC_STATE_SHUTDOWN;
    pst_wlan_pm->pm_svc_state[PM_SVC_SLE] = PM_SVC_STATE_SHUTDOWN;
    pst_wlan_pm->wlan_state = WLAN_STATE_SHUTDOWN;
}

/* WLAN PM初始化接口 */
struct wlan_pm_s* wlan_pm_init_etc(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm;
    oal_int32 ret;
    pm_board_info *pst_board = get_pm_board_info();

    if (pst_board == OAL_PTR_NULL) {
        oal_print_err("wlan_pm_init_etc, get board info failed");
        return OAL_PTR_NULL;
    }
    pst_wlan_pm = kzalloc(sizeof(struct wlan_pm_s), GFP_KERNEL);
    if (pst_wlan_pm == NULL) {
        oal_print_err("no mem to allocate wlan_pm_data");
        return OAL_PTR_NULL;
    }
    pst_wlan_pm->pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);
    if (pst_wlan_pm->pst_bus == NULL) {
        oal_print_err("hcc bus is NULL, failed to create wlan_pm_wq!");
        kfree(pst_wlan_pm);
        return OAL_PTR_NULL;
    }

    ret = osal_spin_lock_init(&pst_wlan_pm->st_irq_lock);
    if (ret != OSAL_SUCCESS) {
        oal_print_err("osal_spin_lock_init failed");
        kfree(pst_wlan_pm);
        return OAL_PTR_NULL;
    }

    g_wlan_pm_info = pst_wlan_pm;

    // 注册GPIO唤醒回调
    pm_board_wkup_gpio_register(pm_wkup_process_host);

    wlan_pm_para_init(pst_wlan_pm);

    return  pst_wlan_pm;
}

/* WLAN pm退出接口 */
oal_uint wlan_pm_exit_etc(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();
    if (pst_wlan_pm == OAL_PTR_NULL) {
        return OAL_SUCC;
    }
    osal_spin_lock_destroy(&pst_wlan_pm->st_irq_lock);
    kfree(pst_wlan_pm);
    g_wlan_pm_info = OAL_PTR_NULL;
    return OAL_SUCC;
}

/* wifi是否上电 */
oal_uint32 wlan_pm_is_poweron_etc(oal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();
    if (pst_wlan_pm == OAL_PTR_NULL) {
        return OAL_FALSE;
    }

    return (pst_wlan_pm->pm_power_state == POWER_STATE_OPEN);
}
EXPORT_SYMBOL_GPL(wlan_pm_is_poweron_etc);

/* 获取回调handler指针 */
struct wifi_srv_callback_handler* wlan_pm_get_wifi_srv_handler_etc(oal_void)
{
    return OSAL_NULL;
}
EXPORT_SYMBOL_GPL(wlan_pm_get_wifi_srv_handler_etc);

wlan_customize_init_cb g_wlan_cfg_init_cb = NULL;
void wlan_cfg_init_cb_reg(wlan_customize_init_cb cb_cfg)
{
    g_wlan_cfg_init_cb = cb_cfg;
}

void wlan_cfg_init_cb_unreg(void)
{
    g_wlan_cfg_init_cb = NULL;
}

/*
  wlan业务开启:
  1、如果WCPU没有开启则开启WCPU
  2、进行业务侧的配置等
*/
oal_int32 wlan_open_data_cfg(oal_void)
{
    custom_cali_func cali_func = NULL;
    hcc_bus *pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);
    struct pm_drv_data *pm_data = pm_get_drvdata();
    struct wlan_pm_s    *pst_wlan_pm = wlan_pm_get_drv_etc();
    if ((pm_data == NULL) || (pst_bus == NULL) || (pst_wlan_pm == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_open_data_cfg::get hcc bus failed!");
        return OAL_FAIL;
    }
    /* 初始化配置定制化参数 */
    if (g_wlan_cfg_init_cb != NULL) {
        if (g_wlan_cfg_init_cb() != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_CFG, "wlan_open_data_cfg::hwifi_hcc_customize_h2d_data_cfg fail");
        }
    } else {
        oam_error_log0(0, OAM_SF_CFG, "g_wlan_cfg_init_cb is NULL");
        return OAL_FAIL;
    }
    init_completion(&pm_data->dev_wlan_ready);
    cali_func = get_custom_cali_func();
    if (cali_func == NULL) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_open_data_cfg::NO g_custom_cali_func registered");
        return OAL_FAIL;
    }

    /* 如果校准下发成功则等待device ready；否则继续打开wifi */
    if (cali_func() == OAL_SUCC) {
        if (oal_wait_for_completion_timeout((&pm_data->dev_wlan_ready),
            (td_s32)msecs_to_jiffies(HOST_WAIT_BOTTOM_WIFI_TIMEOUT)) == 0) {
            oam_error_log1(0, OAM_SF_PWR, "wlan_open_data_cfg: timeout retry %d !!!!!!", HOST_WAIT_BOTTOM_WIFI_TIMEOUT);
            hcc_sched_transfer(hcc_get_handler(HCC_CHANNEL_AP));
            if (oal_wait_for_completion_timeout((&pm_data->dev_wlan_ready),
                (td_s32)msecs_to_jiffies(HOST_WAIT_BOTTOM_WIFI_TIMEOUT_QUARTERS)) == 0) {
                oam_error_log0(0, OAM_SF_PWR, "wlan_open_data_cfg::cali timeout again\n");
                pst_wlan_pm->pm_power_state = POWER_STATE_SHUTDOWN;
                return OAL_FAIL;
            }
        }
    }
    return OAL_SUCC;
}

static oal_int32 wlan_open_etc(osal_void)
{
    hcc_bus* pst_bus;
    struct wlan_pm_s    *pst_wlan_pm = wlan_pm_get_drv_etc();
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if ((pm_data == NULL)||(pst_wlan_pm == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_open::pm_data or pst_wlan_pm is NULL!");
        return OAL_FAIL;
    }

    osal_mutex_lock(&pm_data->host_mutex);
    if (pst_wlan_pm->wlan_state == WLAN_STATE_OPEN) {
        osal_mutex_unlock(&pm_data->host_mutex);
        oal_print_err("%s, already opend\n", __func__);
        return OAL_ERR_CODE_ALREADY_OPEN;
    }

    pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);
    if (pst_bus == NULL) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_open::get hcc bus failed!");
        osal_mutex_unlock(&pm_data->host_mutex);
        return OAL_FAIL;
    }
    init_completion(&pm_data->dev_wlan_ready);
    wlan_power_open_cmd();

    // 在dfr流程中走到此处时hcc为关闭状态, 需要打开(默认流程中此处为开)
    hcc_switch_status(HCC_ON);

    if (oal_wait_for_completion_timeout((&pm_data->dev_wlan_ready),
        (oal_uint32)msecs_to_jiffies(HOST_WAIT_BOTTOM_WIFI_TIMEOUT)) == 0) {
        oam_error_log1(0, OAM_SF_PWR, "wait device ready timeout... %d ms ", HOST_WAIT_BOTTOM_WIFI_TIMEOUT);
        if (oal_wait_for_completion_timeout((&pm_data->dev_wlan_ready),
            (oal_uint32)msecs_to_jiffies(HOST_WAIT_BOTTOM_WIFI_TIMEOUT_QUARTERS)) == 0) {
            oam_error_log0(0, OAM_SF_PWR, "retry 5 second hold, still timeout");
            osal_mutex_unlock(&pm_data->host_mutex);
            return -OAL_ETIMEDOUT;
        } else {
            /* 强制调度成功，说明有可能是GPIO中断未响应 */
            oam_error_log0(0, OAM_SF_PWR, KERN_WARNING"[E]retry succ, maybe gpio interrupt issue");
        }
    }

    // dfr自愈流程, 此处强制下发hcc控制消息(默认关),确保后续数据消息顺利下发
    hcc_force_update_queue_id(pst_bus, 1);
    if (wlan_open_data_cfg() == OAL_FAIL) {
        osal_mutex_unlock(&pm_data->host_mutex);
        hcc_force_update_queue_id(pst_bus, 0);
        return OAL_FAIL;
    }
    hcc_force_update_queue_id(pst_bus, 0);

    oam_warning_log0(0, OAM_SF_PWR, "wlan_open::wlan_open SUCC!!");
    raw_notifier_call_chain(&wifi_pm_chain, WIFI_PM_POWERUP_EVENT, (oal_void*)pst_wlan_pm); /* powerup chain */

    pst_wlan_pm->wlan_state = WLAN_STATE_OPEN;
    osal_mutex_unlock(&pm_data->host_mutex);

    return OAL_SUCC;
}

oal_int32 wlan_open(oal_void)
{
    oal_int32 ret;

    ret = pm_svc_open(PM_SVC_WLAN, OSAL_FALSE);
    if (ret != OAL_SUCC && ret != OAL_ERR_CODE_ALREADY_OPEN) {
        oam_error_log1(0, OAM_SF_PWR, "wlan_pm_open_etc fail %d!", ret);
        return OAL_FAIL;
    }

    return wlan_open_etc();
}

oal_int32 wlan_open_ex(oal_void)
{
    oal_int32 ret;

    ret = pm_svc_open(PM_SVC_WLAN, OSAL_TRUE);
    if (ret != OAL_SUCC && ret != OAL_ERR_CODE_ALREADY_OPEN) {
        oam_error_log1(0, OAM_SF_PWR, "wlan_pm_open_etc fail %d!", ret);
        return OAL_FAIL;
    }

    return wlan_open_etc();
}
EXPORT_SYMBOL_GPL(wlan_cfg_init_cb_reg);

EXPORT_SYMBOL_GPL(wlan_open);
EXPORT_SYMBOL_GPL(wlan_open_ex);

static osal_u32 wlan_close_etc(osal_void)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();
    struct pm_drv_data *pm_data = pm_get_drvdata();
    custom_cali_func exit_later_func = get_custom_exit_later_func();

    oam_warning_log0(0, OAM_SF_PWR, "wlan_pm_close_etc enter");
    if ((pm_data == NULL) || (pst_wlan_pm == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_pm_close_etc,pm_data or pst_wlan_pm is NULL!");
        return OAL_FAIL;
    }
    osal_mutex_lock(&pm_data->host_mutex);

    if ((pst_wlan_pm->pm_power_state == POWER_STATE_SHUTDOWN) ||
        (pst_wlan_pm->wlan_state == WLAN_STATE_SHUTDOWN)) {
        osal_mutex_unlock(&pm_data->host_mutex);
        oal_print_err("%s, already closed\n", __func__);
        return OAL_ERR_CODE_ALREADY_CLOSE;
    }

    if (wlan_power_close_cmd() != OAL_SUCC) {
        osal_mutex_unlock(&pm_data->host_mutex);
        oal_print_err("%s, close cmd fail\n", __func__);
        return OAL_FAIL;
    }

    if (exit_later_func != OSAL_NULL) {
        oal_print_err("%s, exit_later_func\n", __func__);
        exit_later_func();
    } else {
        oal_print_err("%s, miss exit_later_func\n", __func__);
    }

    pst_wlan_pm->wlan_state = WLAN_STATE_SHUTDOWN;
    osal_mutex_unlock(&pm_data->host_mutex);

    return OAL_SUCC;
}

/* close the Wi-Fi service and try to close the WCPU. */
oal_uint32 wlan_close(oal_void)
{
    osal_u32 ret;
    ret = wlan_close_etc();
    if (ret != OAL_SUCC) {
        return ret;
    }
    /* try close wcpu */
    pm_svc_close(PM_SVC_WLAN, OSAL_FALSE);
    return OAL_SUCC;
}

oal_uint32 wlan_close_ex(oal_void)
{
    osal_u32 ret;
    ret = wlan_close_etc();
    if (ret != OAL_SUCC && ret != OAL_ERR_CODE_ALREADY_CLOSE) {
        return ret;
    }
    /* try close wcpu */
    pm_svc_close(PM_SVC_WLAN, OSAL_TRUE);
    return OAL_SUCC;
}
EXPORT_SYMBOL_GPL(wlan_close);
EXPORT_SYMBOL_GPL(wlan_close_ex);

oal_int32 wlan_power_open_cmd(oal_void)
{
    oal_int32 ret;
    oal_print_err("wlan_power_open_cmd");
    oam_warning_log0(0, OAM_SF_PWR, "Send H2D_MSG_WLAN_OPEN cmd");
    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_WLAN_OPEN, HCC_ACTION_TYPE_TEST); /* can wakeup dev */
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }
    oam_warning_log0(0, OAM_SF_PWR, "Send H2D_MSG_WLAN_OPEN cmd succ ");
    return OAL_SUCC;
}

oal_int32 wlan_power_close_cmd(oal_void)
{
    oal_int32 ret;

    oam_warning_log0(0, OAM_SF_PWR, "Send H2D_MSG_WLAN_CLOSE cmd");
    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_WLAN_CLOSE, HCC_ACTION_TYPE_TEST); /* can wakeup dev */
    if (ret != OAL_SUCC) {
        return OAL_FAIL;
    }
    oam_warning_log0(0, OAM_SF_PWR, "Send H2D_MSG_WLAN_CLOSE cmd succ ");
    return OAL_SUCC;
}

oal_uint8 pm_svc_open_state_get(oal_uint8 svc_type)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();
    if (pst_wlan_pm == OSAL_NULL) {
        return PM_SVC_STATE_SHUTDOWN;
    }
    return pst_wlan_pm->pm_svc_state[svc_type];
}

oal_int32 pm_bsle_open(struct pm_drv_data *pm_data, osal_bool is_first_svc)
{
    oal_int32 ret;
    // ble或sle业务, 只通知一次device启动业务
    /* 初始化配置定制化参数 */
    if (hbsle_hcc_customize_h2d_data_cfg() != OAL_SUCC) {
        hcc_debug("bsle_open_data_cfg::hbsle_hcc_customize_h2d_data_cfg fail\n");
    } else {
        hcc_debug("get bsle custom para data\n");
    }
    if (hbsle_hcc_customize_get_device_status(BSLE_STATUS_CUSTOMIZE_RECEIVED) == false) {
        hcc_debug("customize data not received \n");
    }
    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_BT_OPEN, HCC_ACTION_TYPE_TEST);
    if (ret != OAL_SUCC) {
        oal_print_err("pm_svc_open::ble/sle open, send message fail, ret=%d. \n", ret);
        if (is_first_svc) {
            oal_print_err("pm_svc_open::first svc fail, power off \n");
            pm_svc_power_off();
            oal_wake_unlock(&pm_data->st_wakelock);
        }
        return ret;
    }
#if defined(WSCFG_PLAT_DIAG_LOG_OUT)
    // 注册bt数采回调
    diag_sample_data_register(ZDIAG_BSLE_SAMPLE_CB, hcc_adapt_bsle_msg_data_col_cmd_tx_proc);
#endif
    return OAL_SUCC;
}

oal_int32 pm_init_n_firmware_download(struct pm_drv_data *pm_data, osal_bool *is_first_svc)
{
    oal_int32 ret;
    oal_print_err("pm_svc_open::first svc open, pm_svc_power_on! \n");
    if (!pm_data->st_wakelock.lock_count) {
        /* make sure open only lock once */
        oal_wake_lock(&pm_data->st_wakelock);
    }

    ret = pm_svc_power_on();
    if (ret != OAL_SUCC) {
        // 初始化失败, 返回
        oal_wake_unlock(&pm_data->st_wakelock);
        oal_print_err("pm_svc_open::first svc open, pm_svc_power_on fail, ret=%d \n", ret);
        return ret;
    }
    *is_first_svc = OSAL_TRUE;
    oal_print_warning("pm_svc_open::first svc open, pm_svc_power_on succ! \n");
    init_completion(&pm_data->dev_wlan_ready);
    return OAL_SUCC;
}

/* 平台管理业务启停 begin */
/* 平台新增, 业务开启统一处理接口 */
osal_s32 pm_svc_open(osal_u8 service, osal_bool is_insmod)
{
    osal_bool is_first_svc = OSAL_FALSE;
    oal_int32 ret;
    hcc_bus* pst_bus;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();
    struct pm_drv_data *pm_data = pm_get_drvdata();

    // 用于区分wlan的insmod与业务启停
    if (!is_insmod) {
        oal_print_err("pm_svc_open::not insmod service[%u], return already opened. \n", service);
        return OAL_ERR_CODE_ALREADY_OPEN;
    }

    // 判断业务合法性
    if (service >= PM_SVC_NUM) {
        oal_print_err("pm_svc_open::invalid svc [%u] \n", service);
        return OAL_FAIL;
    }

    if ((pm_data == NULL) || (pst_wlan_pm == NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_pm_open_etc:pm_data or pst_wlan_pm is NULL!");
        return OAL_FAIL;
    }

    osal_mutex_lock(&pm_data->host_mutex);

    // 原有流程判断, 暂时保留
    pst_bus = hcc_get_channel_bus(HCC_CHANNEL_AP);
    if (pst_bus == NULL) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_pm_open_etc::get hcc bus failed!");
        osal_mutex_unlock(&pm_data->host_mutex);
        return OAL_FAIL;
    }

    // 判断该业务是否已经启动
    if (pst_wlan_pm->pm_svc_state[service] == PM_SVC_STATE_OPEN) {
        oal_print_err("pm_svc_open::svc [%u] already opened \n", service);
        osal_mutex_unlock(&pm_data->host_mutex);
        return OAL_ERR_CODE_ALREADY_OPEN;
    }

    // 判断是否已有业务启动
    // 若为首个业务开启, 先进行初始化和firmware download
    if (!(pst_wlan_pm->pm_svc_state[PM_SVC_WLAN] || pst_wlan_pm->pm_svc_state[PM_SVC_BLE] ||
          pst_wlan_pm->pm_svc_state[PM_SVC_SLE])) {
        ret = pm_init_n_firmware_download(pm_data, &is_first_svc);
        if (ret != OAL_SUCC) {
            osal_mutex_unlock(&pm_data->host_mutex);
            return ret;
        }
    }

    // 对BSLE进行初始化之后的处理
    if (service == PM_SVC_WLAN) {
        // wlan业务启动, 通过hcc通知device侧启动业务
        oal_print_err("pm_svc_open::wlan open. \n");
    } else if (!(pst_wlan_pm->pm_svc_state[PM_SVC_BLE] || pst_wlan_pm->pm_svc_state[PM_SVC_SLE])) {
        ret = pm_bsle_open(pm_data, is_first_svc);
        if (ret != OAL_SUCC) {
            osal_mutex_unlock(&pm_data->host_mutex);
            return ret;
        }
    }
    
    // 业务初始化成功, 更新标志位
    pst_wlan_pm->pm_svc_state[service] = OAL_TRUE;
    oal_wake_unlock(&pm_data->st_wakelock);
    osal_mutex_unlock(&pm_data->host_mutex);
    oal_print_err("pm_svc_open::SUCC! \n");
    return OAL_SUCC;
}

static osal_void pm_svc_close_etc(struct wlan_pm_s *pst_wlan_pm, struct pm_drv_data *pm_data)
{
    osal_s32 ret;

    ret = pm_svc_power_off();
    if (ret != OAL_SUCC) {
        oal_print_err("pm_svc_close::pm_svc_power_off fail, ret=%d. \n", ret);
    }

    pst_wlan_pm->pm_power_state = POWER_STATE_SHUTDOWN;
    oal_wake_unlock(&pm_data->st_wakelock);
    pm_wakelocks_release_detect(pm_data);
}

/* 平台新增, 业务关闭统一接口 */
osal_s32 pm_svc_close(osal_u8 service, osal_u8 is_rmmod)
{
    oal_int32 ret;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv_etc();
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (!is_rmmod) {
        oal_print_err("pm_svc_close::not rmmod svc [%u], return. \n", service);
        return OAL_FAIL;
    }

    // 判断业务合法性
    if (service >= PM_SVC_NUM) {
        oal_print_err("pm_svc_close::invalid svc [%u] \n", service);
        return OAL_FAIL;
    }

    if ((pm_data == NULL) || (pst_wlan_pm == OAL_PTR_NULL)) {
        oam_error_log0(0, OAM_SF_PWR, "wlan_pm_close_etc,pm_data or pst_wlan_pm is NULL!");
        return OAL_FAIL;
    }

    // 关闭流程中上锁后, 除业务已经关闭情况外, 处理失败也继续往下执行完毕再解锁
    osal_mutex_lock(&pm_data->host_mutex);

    // 判断该业务是否已经关闭
    if (pst_wlan_pm->pm_svc_state[service] == PM_SVC_STATE_SHUTDOWN) {
        oal_print_err("pm_svc_close::svc [%u] already closed \n", service);
        osal_mutex_unlock(&pm_data->host_mutex);
        return OAL_ERR_CODE_ALREADY_CLOSE;
    }

    // 更新业务标志位
    pst_wlan_pm->pm_svc_state[service] = PM_SVC_STATE_SHUTDOWN;
    oal_print_err("pm_svc_close::cur_state: wlan[%u] ble[%u] sle[%u] \n",
        pst_wlan_pm->pm_svc_state[PM_SVC_WLAN],
        pst_wlan_pm->pm_svc_state[PM_SVC_BLE],
        pst_wlan_pm->pm_svc_state[PM_SVC_SLE]);

    // 分业务进行业务关闭处理, 处理失败时继续往下执行
    if (service == PM_SVC_WLAN) {
        oal_print_err("pm_svc_close::wlan close, send message. \n");
    } else {
        // BLE和SLE均关闭时通知device侧停止BT业务
        oal_print_err("pm_svc_close::ble/sle closed. \n");
        if (!(pst_wlan_pm->pm_svc_state[PM_SVC_BLE] || pst_wlan_pm->pm_svc_state[PM_SVC_SLE])) {
            oal_print_err("pm_svc_close::ble/sle all closed, send message. \n");
            ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_BT_CLOSE, HCC_ACTION_TYPE_TEST);
            if (ret != OAL_SUCC) {
                oal_print_err("pm_svc_close::ble/sle close, send message fail, ret=%d. \n", ret);
            }
        }
    }

    // 所有业务都关闭时对device进行复位
    if (!(pst_wlan_pm->pm_svc_state[PM_SVC_WLAN] ||
          pst_wlan_pm->pm_svc_state[PM_SVC_BLE] ||
          pst_wlan_pm->pm_svc_state[PM_SVC_SLE])) {
        oal_print_err("pm_svc_close::all svc closed, deinit! \n");
        pm_svc_close_etc(pst_wlan_pm, pm_data);
    }

    osal_mutex_unlock(&pm_data->host_mutex);
    return OAL_SUCC;
}

osal_s32 pm_ble_open(void)
{
    return pm_svc_open(PM_SVC_BLE, OSAL_TRUE);
}

osal_s32 pm_ble_close(void)
{
    return pm_svc_close(PM_SVC_BLE, OSAL_TRUE);
}

osal_s32 pm_sle_open(void)
{
    return pm_svc_open(PM_SVC_SLE, OSAL_TRUE);
}

osal_s32 pm_sle_close(void)
{
    return pm_svc_close(PM_SVC_SLE, OSAL_TRUE);
}

static void pm_plt_shutdown_cb_host(void)
{
    osal_s32 ret;
    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_HOST_SLP, HCC_ACTION_TYPE_TEST);
    if (ret != EOK) {
        oal_print_err("pm_plt_suspend_cb_host: hcc_send_message fail, ret[%d] \r\n", ret);
    }

    osal_mdelay(20); // 避免消息阻塞, 等待20ms

    // 关闭HCC
    hcc_deinit(HCC_CHANNEL_AP);
}

void pm_shutdown_process_host(void)
{
    oal_print_warning("pm_shutdown_process_host! \r\n");
    if (g_pm_wifi_suspend_cb_host != NULL) {
        oal_print_warning("pm_suspend_process_host: call g_pm_wifi_suspend_cb_host(). \r\n");
        g_pm_wifi_suspend_cb_host();
    }
    if (g_pm_ble_suspend_cb_host != NULL) {
        oal_print_warning("pm_suspend_process_host: call g_pm_ble_suspend_cb_host(). \r\n");
        g_pm_ble_suspend_cb_host();
    }

    // 平台自身业务处理
    pm_plt_shutdown_cb_host();
}

void pm_suspend_process_host(void)
{
    osal_s32 ret;
    oal_print_warning("pm_suspend_process_host! \r\n");
    wlan_set_dfr_recovery_flag(OSAL_TRUE);
    plat_update_device_recovery_flag(TD_TRUE);
    if (g_pm_wifi_suspend_cb_host != NULL) {
        oal_print_warning("pm_suspend: wifi suspend. \r\n");
        g_pm_wifi_suspend_cb_host();
    }
    if (g_pm_ble_suspend_cb_host != NULL) {
        oal_print_warning("pm_suspend: ble suspend. \r\n");
        g_pm_ble_suspend_cb_host();
    }

    ret = pm_svc_power_off();
    if (ret != OAL_SUCC) {
        oal_print_err("pm_suspend: pm_svc_power_off fail!. \r\n");
    }

    hcc_switch_status(HCC_BUS_FORBID);

    oal_print_warning("pm_suspend: success!. \r\n");
}

void pm_resume_process_host(void)
{
    osal_s32 ret;
    oal_print_warning("pm_resume_process_host!\r\n");

    // 复用DRF流程, 需要提前reinit hcc
    tr5330_board_power_on();
    hcc_bus_reinit(hcc_get_channel_bus(HCC_CHANNEL_AP));
    // 下载bin
    pm_svc_power_on();
    // 重新禁止 hcc发数据
    hcc_switch_status(HCC_BUS_FORBID);
    if (g_pm_wifi_resume_cb_host != NULL) {
        ret = g_pm_wifi_resume_cb_host();
        oal_print_warning("pm_resume: wifi resume, ret=[%d].\r\n", ret);
    }
    if (g_pm_ble_resume_cb_host != NULL) {
        ret = g_pm_ble_resume_cb_host();
        oal_print_warning("pm_resume: ble resume, ret=[%d].\r\n", ret);
    }
    plat_update_device_recovery_flag(TD_FALSE);

    oal_print_warning("pm_resume: success!\r\n");
}

osal_void pm_wifi_suspend_cb_host_register(pm_wlan_suspend_cb cb)
{
    g_pm_wifi_suspend_cb_host = cb;
}

osal_void pm_wifi_resume_cb_host_register(pm_wlan_resume_cb cb)
{
    g_pm_wifi_resume_cb_host = cb;
}

osal_void pm_ble_suspend_cb_host_register(pm_ble_suspend_cb cb)
{
    g_pm_ble_suspend_cb_host = cb;
}

osal_void pm_ble_resume_cb_host_register(pm_ble_resume_cb cb)
{
    g_pm_ble_resume_cb_host = cb;
}

osal_s32 pm_wkup_process_host(osal_void)
{
    oal_print_warning("pm_wkup_process_host! \r\n");
    if (g_pm_wow_wkup_cb_host != NULL) {
        g_pm_wow_wkup_cb_host();
    }
    return OSAL_SUCCESS;
}

osal_void pm_wifi_wkup_cb_host_register(pm_wow_wkup_cb cb)
{
    g_pm_wow_wkup_cb_host = cb;
}

EXPORT_SYMBOL_GPL(pm_svc_open);
EXPORT_SYMBOL_GPL(pm_svc_close);
EXPORT_SYMBOL_GPL(pm_ble_open);
EXPORT_SYMBOL_GPL(pm_ble_close);
EXPORT_SYMBOL_GPL(pm_sle_open);
EXPORT_SYMBOL_GPL(pm_sle_close);
EXPORT_SYMBOL_GPL(pm_wifi_suspend_cb_host_register);
EXPORT_SYMBOL_GPL(pm_wifi_resume_cb_host_register);
EXPORT_SYMBOL_GPL(pm_ble_suspend_cb_host_register);
EXPORT_SYMBOL_GPL(pm_ble_resume_cb_host_register);
EXPORT_SYMBOL_GPL(pm_wifi_wkup_cb_host_register);
/* 平台管理业务启停 end */
