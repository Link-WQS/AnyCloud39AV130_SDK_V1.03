
/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: plat misc driver header file.
 * Author: CompanyName
 * Create: 2022-11-29
 */

#ifndef __PLAT_MISC__H__
#define __PLAT_MISC__H__

#include "td_base.h"
#include "plat_hcc_msg_type.h"

#define PLAT_TRNG_RANDOM_BYTES_MAX 88

typedef struct {
    td_u32 data_size;
    td_u8 data[PLAT_TRNG_RANDOM_BYTES_MAX];
    osal_bool wait_cond;
    osal_wait wait_r; /* Wait Condition: Random Data Ready */
} plat_msg_ctl_struct;
td_s32 plat_trng_get_random_bytes(td_u8 *randnum, td_u32 size, td_u32 *get_size);
td_u32 uapi_drv_cipher_trng_get_random_bytes(td_u8 *randnum, td_u32 size);

td_void plat_misc_init(td_void);
td_void plat_misc_exit(td_void);

td_u32 send_message_to_device(td_u8 *data, td_u8 data_len, plat_cfg_h2d_msg_type msg_type,
    plat_msg_ctl_struct *ctl_struct, td_u8 *output_data, td_u32 output_data_len);
td_u32 plat_d2h_msg_callback(plat_msg_ctl_struct *msg_ctl, td_u8 *data, td_u16 len);
#endif
