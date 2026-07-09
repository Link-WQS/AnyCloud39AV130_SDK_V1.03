/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: host platform pm dfr header file
 * Create: 2023-02
 */

#ifndef __PLAT_PM_DFR_H__
#define __PLAT_PM_DFR_H__

#include "soc_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PLAT_DFR_CMD_WAIT_MS 100
#define PLAT_DEVICE_RESET_REG 0x4001F118

#define DFR_BUS_STATE_CLOSE 0
#define DFR_BUS_STATE_OPEN 1
#define DFR_BUS_STATE_CLOSE_REG 2

typedef void (*recovery_complete)(void);

osal_s32 plat_exception_init(osal_void);
osal_void plat_exception_exit(osal_void);
td_u8 plat_is_device_in_recovery(td_void);
td_void plat_update_device_recovery_flag(td_u8 flag);
osal_void wlan_set_dfr_recovery_flag(osal_u8 dfr_flag);
extern osal_s32 plat_wifi_exception_rst_register_etc(osal_void *data);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __PLAT_PM_DFR_H__ */

