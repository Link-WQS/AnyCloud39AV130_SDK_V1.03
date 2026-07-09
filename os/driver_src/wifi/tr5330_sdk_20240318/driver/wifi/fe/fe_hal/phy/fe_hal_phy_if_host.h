/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: 校准涉及到的phy功能接口
 * Date: 2022-10-19
 */

#ifndef __FE_HAL_PHY_IF_HOST_H__
#define __FE_HAL_PHY_IF_HOST_H__
#include "hal_device.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
// 刷新射频温度码字配置bank
osal_void fe_hal_phy_set_rf_temp_bank_sel(const hal_device_stru *device, osal_u8 rf_id, osal_u8 bank_sel);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif  // __CALI_PHY_IF_H__