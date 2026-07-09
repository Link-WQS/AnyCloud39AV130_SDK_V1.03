/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef SLE_HOST_REGISTER_H
#define SLE_HOST_REGISTER_H

#include "customize_bsle_ext.h"
#include "oal_schedule.h"
#include "hcc_comm.h"
#include "hcc_if.h"
#include "hcc_adapt.h"

extern osal_s32 pm_sle_open(void);
extern osal_s32 pm_sle_close(void);
extern osal_s32 plat_bt_exception_rst_register_etc(osal_void *data);

#endif