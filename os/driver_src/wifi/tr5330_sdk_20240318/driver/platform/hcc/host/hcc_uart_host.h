/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: hcc_uart_host header file
 * Author: @CompanyNameTag
 */

#ifndef __HCC_UART_HOST_H__
#define __HCC_UART_HOST_H__

#include "td_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* 函数声明 */
td_s32 ps_write_tty(const td_u8 *data, td_u32 count, td_bool hcc_flag);
td_s32 ps_read_tty(td_u8 *data, td_s32 len);
void hcc_uart_set_transfer_mode(td_u8 mode);
td_void hcc_uart_tx_thread_wakeup(td_void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // __HCC_UART_HOST_H__

