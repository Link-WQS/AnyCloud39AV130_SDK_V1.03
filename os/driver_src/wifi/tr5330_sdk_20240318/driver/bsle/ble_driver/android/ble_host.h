/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef BT_UAER_H_
#define BT_UAER_H_

#include "osal_list.h"

typedef void (*pm_ble_suspend_cb)(void);
typedef osal_s32 (*pm_ble_resume_cb)(void);

extern osal_s32 pm_ble_open(void);
extern osal_s32 pm_ble_close(void);

extern osal_void pm_ble_suspend_cb_host_register(pm_ble_suspend_cb cb);
extern osal_void pm_ble_resume_cb_host_register(pm_ble_resume_cb cb);
extern osal_s32 plat_bt_exception_rst_register_etc(osal_void *data);

#endif
