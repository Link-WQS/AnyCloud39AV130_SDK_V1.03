/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准涉及到的phy功能接口
 */
#include "fe_hal_phy_if_host.h"
#include "fe_hal_phy_reg_if_host.h"
#include "hal_phy.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
osal_void fe_hal_phy_set_rf_temp_bank_sel(const hal_device_stru *device, osal_u8 rf_id, osal_u8 bank_sel)
{
    const osal_u32 rf_temp_bank_sel_addr_offset = 0x7F;
    unref_param(device);
    unref_param(rf_id);

    hal_set_cfg_reserv_1_reg_2_reserv_1_wr_value_0ch(bank_sel);
    hal_set_cfg_reserv_1_reg_1_reserv_1_wr_addr(rf_temp_bank_sel_addr_offset);
    hal_set_cfg_reserv_1_reg_1_cfg_reserv_1_wr_en(1);
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
