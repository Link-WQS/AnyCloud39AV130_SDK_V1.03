/*
 * Copyright (C) 2018-2020 AICSemi Ltd.
 *
 *
 *  Bt task init and manager in this file .
 */

#include "rtos.h"
#include "app_bt.h"
#include "aic_adp_api.h"
#include "bt_task_msg.h"
#if PLF_BLE_STACK
#include "aicble.h"
#include "app_ble_only.h"
#endif
#include "sysctrl_api.h"
#include "bt_task.h"
#include "bt_common_config.h"
#include "bt_hci.h"

bool bt_task_need_pon = true;
/// Handle of the BT task
static rtos_task_handle bt_task_handle;
/// Indicates whether BT task is suspended or not
static bool bt_task_suspended;
static rtos_queue app_bt_task_queue;

#if PLF_BLE_STACK
#define AIC_BT_STACK_SIZE 1024*4
#else
#define AIC_BT_STACK_SIZE TASK_STACK_SIZE_BT_TASK
#endif
static aic_host_cfg_t cfg = {
#if APP_SUPPORT_OTA_BOX
    .support_sniff = false,
#else
    .support_sniff = true,
#endif
#if APP_SUPPORT_MULTIPLE_PHONE == 0
    .multiple_phone = false,
#else
    .multiple_phone = true,
#endif
#if (PLF_AIC8800)
    #if (AON_SUPPORT)
        .support_aon = true,
    #ifdef APP_SUPPORT_DEEPSLEEP
        .support_sleep = true,
    #else
        .support_sleep = false,
    #endif
    #else
        .support_aon = false,
        .support_sleep = false,
    #endif
#elif (PLF_AIC8800MC) || (PLF_AIC8800M40)
        .support_aon = false,
    #ifdef APP_SUPPORT_DEEPSLEEP
        .support_sleep = true,
    #else
        .support_sleep = false,
    #endif
#endif
#ifdef CFG_BTDM_RAM_VER
    .rom_version = 0,
#else
    .rom_version = CFG_ROM_VER,
#endif
#if APP_SUPPORT_TWS == 1
#if APP_SUPPORT_TWS_PLUS == 1
    .tws_plus = true,
#else
    .tws_plus = false,
#endif
#endif
    .ssp_mode = true,
};

void bt_task_need_pon_set(bool need_pon)
{
    bt_task_need_pon = need_pon;
    TRACE("%s,%d\n",__func__,bt_task_need_pon);
}

bool bt_task_need_pon_get(void)
{
    return bt_task_need_pon;
}

void bt_task_change_default_param(void)
{
    bt_common_change_fw_load_in_ram();
}

uint32_t bt_task_suspend(void)
{
    uint32_t retvalue = 0;
    GLOBAL_INT_DISABLE();
    bt_task_suspended = true;
    GLOBAL_INT_RESTORE();

    // wait for notification
    retvalue = rtos_task_wait_notification(-1);
    return retvalue;
}


void bt_task_resume(bool isr,uint32_t value)
{
/*
    if (!bt_task_suspended) {
        return;
    }*/
    bt_task_suspended = false;

    // Notify the bt task
    rtos_task_notify_setvalue(bt_task_handle, value, isr);
}


/**
 * @brief Main loop for the BT Task
 */
//extern void host_aon_sram_restore_all(void);
static RTOS_TASK_FCT(bt_task)
{
    BOOL ret = TRUE;
    #if APP_SUPPORT_AES == 1
    uint32_t pwrmd = pwrctrl_pwrmd_cpusys_sw_record_getf();
    if(pwrmd < CPU_SYS_POWER_DOWN) {
        // clk en
        cpusysctrl_hclkme_set(CSC_HCLKME_DMA_EN_BIT);
        bt_drv_generate_secret_key();
        bt_drv_start_generate_key();
        app_bt_handler_register(HANDLER_REG_2, bt_drv_pool_key_complete);
    }
    #endif
    #if (PLF_AIC8800M40)
    pwrctrl_bt_set(PWRCTRL_POWERUP);
    #endif
    #ifdef CFG_BTDM_RAM_VER
    bt_task_change_default_param();
    #endif
    #if PLF_BTDM
    app_ble_init();
    #endif
    #if (CFG_ROM_VER == 255)
    uint8_t chip_id = ChipIdGet(0);
    if (chip_id == 0x03) {
        cfg.rom_version = 2;
    } else if (chip_id == 0x07) {
        cfg.rom_version = 3;
    }
    #endif
    aic_stack_init(cfg);
    app_bt_init();
    ret = aic_adp_stack_config();
    if(ret == TRUE){
        GLOBAL_INT_DISABLE();
        aic_bt_start();
        GLOBAL_INT_RESTORE();
        bt_task_set_default_priority();
    }
#if (PLF_BT_STACK || PLF_BLE_STACK) && (PLF_WIFI_STACK) && defined(CONFIG_RWNX_LWIP)
    wb_coex_bt_on_set(1);
    wb_coex_check_status();
#endif
    bt_task_queue_notify(false);
    for (;;)
    {
        uint32_t msg;
        // Suspend the BT task while waiting for new events
        rtos_queue_read(app_bt_task_queue,(void *)&msg, -1, 0);
        //host_aon_sram_restore_all();//temp add for test ,it will be deleted in real deelsleep mode.
        // schedule all pending events
        aic_stack_loop();
        #if PLF_BLE_STACK
        aic_ble_schedule();
        #endif
        handler_reg_list_poll();
    }
}


void bt_task_queue_notify(bool isr)
{
    bool ret = 0;
    uint32_t msg = 0xff;
    if(rtos_queue_cnt(app_bt_task_queue) < 5){
        ret = rtos_queue_write(app_bt_task_queue,&msg , 0, isr);
        if(isr){
            ret += 0;
            //TRACE(" notify %d ",ret);
        }
    }
}


int bt_task_init(void)
{
    if(!bt_task_need_pon){
        return -1;
    }

    if (rtos_queue_create(sizeof(uint32_t),10,&app_bt_task_queue)){
        TRACE("BT_TASK: Failed to Create app_bt_queue\n");
        return -1;
    }

    bt_task_suspended = false;
    // Create the BT task
    if (rtos_task_create(bt_task, "BT_TASK", BT_TASK,
                         AIC_BT_STACK_SIZE, NULL, TASK_PRIORITY_MAX, &bt_task_handle)) {
        rtos_queue_delete(app_bt_task_queue);
        return -1;
    }
#if APP_SUPPORT_HFG == 1 && PLF_USB_BT == 1
    int res = 0;
    if (0 != usb_bt_init()) {
        TRACE("usb_bt_init fail\n");
        res = -1;
        goto exit;
    }

    if (0 != usb_bt_dongle_init()) {
        TRACE("usb_bt_dongle_init fail\n");
        res = -2;
        goto exit;
    }
exit:
    if (res == 0) {
        TRACE("usb_bt_test success\n");
    } else {
        TRACE("usb_bt_test fail\n");
        usb_bt_dongle_deinit();
        usb_bt_deinit();
    }
#endif
    return 0;
}

int bt_task_deinit(void)
{
    app_bt_deint_start_msg_send();
    #if PLF_BTDM
    app_ble_deint_start_msg_send();
    #endif
    return 0;
}

int bt_task_delete(void)
{
    app_bt_deinit();
    aic_adp_free_stack_memory();
    TRACE("bt_task_deinit ble_task_handle = 0x%x\n",bt_task_handle);
    app_bt_queue_deinit();
    #if PLF_BTDM
    app_ble_only_msg_deinit();
    #endif
    aic_bt_close();
    #if defined(CONFIG_RWNX_LWIP) && defined(CFG_HOSTIF)
    host_if_ble_deinit_done_indicate();
    #endif
    bt_task_need_pon_set(false);
    if(app_bt_task_queue){
        rtos_queue_delete(app_bt_task_queue);
        app_bt_task_queue = NULL;
    }
    if(bt_task_handle){
        rtos_task_delete(bt_task_handle);
        bt_task_handle = NULL;
    }
#if APP_SUPPORT_HFG == 1 && PLF_USB_BT == 1
    usb_bt_dongle_deinit();
    usb_bt_deinit();
#endif
    return 0;
}

void bt_task_set_max_priority(void)
{
    rtos_task_set_priority(bt_task_handle, TASK_PRIORITY_MAX);
}

void bt_task_set_default_priority(void)
{
    rtos_task_set_priority(bt_task_handle, TASK_PRIORITY_BT_TASK);
}

