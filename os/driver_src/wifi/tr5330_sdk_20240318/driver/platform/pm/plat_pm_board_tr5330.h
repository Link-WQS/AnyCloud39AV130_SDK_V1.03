/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: board adapter source file
 * Author: CompanyName
 */

#ifndef __PLAT_PM_BOARD_TR5330_H__
#define __PLAT_PM_BOARD_TR5330_H__

#include "soc_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

osal_s32 tr5330_board_power_on(osal_void);
osal_s32 tr5330_board_power_off(osal_void);
osal_s32 tr5330_board_power_reset_hcc(osal_void);
osal_s32 tr5330_board_power_reset_gpio(osal_void);
osal_s32 tr5330_board_power_reset(td_bool sys_boot);
osal_s32 tr5330_board_service_enter(osal_void);
osal_s32 tr5330_board_service_exit(osal_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of plat_pm_board_tr5330.h */