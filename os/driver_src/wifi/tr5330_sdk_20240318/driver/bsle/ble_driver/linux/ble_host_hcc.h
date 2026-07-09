/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef __BLE_HOST_HCC_H
#define __BLE_HOST_HCC_H

#include "customize_bsle_ext.h"
#include "hcc_if.h"
#include "hcc_adapt.h"
#include "oal_schedule.h"
#include "securec.h"

#define IFTYPE_BLE 0xF1
#define IFTYPE_ORG 0x03

#define osal_array_size(_array)  (sizeof(_array) / sizeof((_array)[0]))

extern osal_s32 pm_ble_open(void);
extern osal_s32 pm_ble_close(void);
extern osal_s32 plat_bt_exception_rst_register_etc(osal_void *data);
extern unsigned int get_dev_addr(unsigned char *pc_addr, unsigned char addr_len, unsigned char type);

#endif