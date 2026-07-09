/*
 * Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.
 */
#include "sle_host_register.h"
#include "sle_chan_kernel.h"
#include "sle_tm_chba_proc.h"
#include "sle_hcc_proc.h"

#include <asm/unaligned.h>

MODULE_LICENSE("GPL");

static int32_t sle_dfr_recovery(void);

static int32_t sle_host_init(void)
{
    int32_t ret;
    bool boot_finish;

    ret = pm_sle_open();
    if (ret != OAL_SUCC) {
        oal_print_err("pm_sle_open failed\n");
    } else {
        oal_print_err("pm_sle_open success\n");
    }

    boot_finish = hbsle_hcc_customize_get_device_status(BSLE_STATUS_BOOT_FINISH);
    if (boot_finish == false) {
        hcc_debug("device boot not finish \n");
    }

    oal_print_err("sle device open success\n");

    ret = sle_hcc_init();
    if (ret != OAL_SUCC) {
        hcc_debug("initial hcc sle service failed\n");
        return OAL_FAIL;
    }

    ret = sle_chan_init();
    if (ret == SLE_CHAN_FAILED) {
        oal_print_err("sle_chan_init failed\r\n");
        return OAL_FAIL;
    }
    sle_hci_register_send_frame_cb((sle_send_frame_cb_t)sle_hci_send_frame);
    sle_tm_register_send_frame_cb((sle_send_frame_cb_t)sle_tm_send_to_chba);
    hci_kernel_init();
    // 注册dfr回调
    plat_bt_exception_rst_register_etc(sle_dfr_recovery);
    oal_print_info("sle host init finished\n");
    return OAL_SUCC;
}

static osal_void sle_host_deinit(void)
{
    int32_t ret;
    oal_print_err("enter:%s\n", __func__);
    hcc_service_deinit(HCC_CHANNEL_AP, HCC_ACTION_TYPE_SLE);

    ret = pm_sle_close();
    if (ret != OAL_SUCC) {
        oal_print_err("pm_sle_close failed\n");
    } else {
        oal_print_err("finish: pm_sle_close\n");
    }

    hbsle_hcc_customize_reset_device_status();
    sle_chan_exit();
    oal_print_err("finish: H2D_MSG_SLE_CLOSE\n");
}

static int32_t sle_dfr_recovery(void)
{
    int32_t ret;
    bool boot_finish;
    bool customize_received;
    oal_print_err("start sle dfr recovery \n");
    /* 恢复HCC状态 */
    hcc_switch_status(HCC_ON);
    hbsle_hcc_customize_reset_device_status();
    /* 重新下发定制化配置 */
    ret = hbsle_hcc_custom_ini_data_buf();
    if (ret != OAL_SUCC) {
        hcc_debug("sle_dfr_recovery::hcc ini data,fail ret=%d. \n", ret);
        return ret;
    }
    customize_received = hbsle_hcc_customize_get_device_status(BSLE_STATUS_CUSTOMIZE_RECEIVED);
    if (customize_received == false) {
        hcc_debug("customize data not received \n");
    }
    ret = hcc_send_message(HCC_CHANNEL_AP, H2D_MSG_BT_OPEN, HCC_ACTION_TYPE_TEST);
    if (ret != OAL_SUCC) {
        hcc_debug("sle_dfr_recovery::ble/sle open,fail, ret=%d. \n", ret);
        return ret;
    }
    boot_finish = hbsle_hcc_customize_get_device_status(BSLE_STATUS_BOOT_FINISH);
    if (boot_finish == false) {
        hcc_debug("device boot not finish \n");
    }
    return ret;
}

module_init(sle_host_init);
module_exit(sle_host_deinit);
EXPORT_SYMBOL(sle_tm_register_chba_recv_interface);
EXPORT_SYMBOL(sle_hci_register_chba_recv_interface);
EXPORT_SYMBOL(sle_hci_recv_from_chba);
