/*
 * Copyright (C) 2018-2022 AICSemi Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "dbg.h"
#include "uart1_api.h"
#include "uart2_api.h"
#include "ring_buffer.h"
#include "rtos.h"
#include "rtos_al.h"
#include "stdio_uart.h"
#include "lp_ticker_api.h"

#include "usbd.h"
#include "cdc_device.h"
#include "bsp/board_api.h"

#ifdef CFG_TUSB_DEMO_CDC_DUAL_PORTS

/*
 * MACROS
 ****************************************************************************************
 */
#define UART_PRINT              dbg
#define CONFIG_UART_ENABLE      1
#define CONFIG_UART_RX_BUF_SIZE 64
#define CONFIG_UART_TX_BUF_SIZE 64
#define CONFIG_RING_BUF_SIZE    1024

/*
 * FUNCTIONS
 ****************************************************************************************
 */

#if CONFIG_UART_ENABLE
static ring_buffer_t ring_buf_elem[2];
static rtos_task_handle task_handle_elem[2];

static void uart_task_suspend(void)
{
    rtos_task_wait_notification(-1);
}

static void uart_task_resume(uint8_t itf, bool isr)
{
    // Notify the task
    rtos_task_notify_setvalue(task_handle_elem[itf], 0, isr);
}

void uart_to_usb_task_1(void *param)
{
    uint8_t buf[CONFIG_UART_TX_BUF_SIZE];
    uint32_t buf_cnt;
    for (;;) {
        critical_section_start();
        buf_cnt = ring_buffer_bytes_used(&ring_buf_elem[0]);
        critical_section_end();
        while (buf_cnt) {
            uint32_t n_bytes = (buf_cnt < CONFIG_UART_TX_BUF_SIZE) ? buf_cnt : CONFIG_UART_TX_BUF_SIZE;
            critical_section_start();
            ring_buffer_read(&ring_buf_elem[0], buf, n_bytes);
            critical_section_end();
            tud_cdc_n_write(0, buf, n_bytes);
            tud_cdc_n_write_flush(0);
            critical_section_start();
            buf_cnt = ring_buffer_bytes_used(&ring_buf_elem[0]);
            critical_section_end();
        }
        uart_task_suspend();
    }
}

void uart_to_usb_task_2(void *param)
{
    uint8_t buf[CONFIG_UART_TX_BUF_SIZE];
    uint32_t buf_cnt;
    for (;;) {
        critical_section_start();
        buf_cnt = ring_buffer_bytes_used(&ring_buf_elem[1]);
        critical_section_end();
        while (buf_cnt) {
            uint32_t n_bytes = (buf_cnt < CONFIG_UART_TX_BUF_SIZE) ? buf_cnt : CONFIG_UART_TX_BUF_SIZE;
            critical_section_start();
            ring_buffer_read(&ring_buf_elem[1], buf, n_bytes);
            critical_section_end();
            tud_cdc_n_write(1, buf, n_bytes);
            tud_cdc_n_write_flush(1);
            critical_section_start();
            buf_cnt = ring_buffer_bytes_used(&ring_buf_elem[1]);
            critical_section_end();
        }
        uart_task_suspend();
    }
}

void uart1_rx_handler(void)
{
    uint8_t buf[CONFIG_UART_RX_BUF_SIZE];
    int rx_cnt = uart1_get_rx_count();
    int n_bytes = (rx_cnt < CONFIG_UART_RX_BUF_SIZE) ? rx_cnt : CONFIG_UART_RX_BUF_SIZE;
    int idx;
    for (idx = 0; idx < n_bytes; idx++) {
        buf[idx] = uart1_getc();
    }
    ring_buffer_write(&ring_buf_elem[0], buf, n_bytes);
    uart_task_resume(0, true);
}

void uart2_rx_handler(void)
{
    uint8_t buf[CONFIG_UART_RX_BUF_SIZE];
    int rx_cnt = uart2_get_rx_count();
    int n_bytes = (rx_cnt < CONFIG_UART_RX_BUF_SIZE) ? rx_cnt : CONFIG_UART_RX_BUF_SIZE;
    int idx;
    for (idx = 0; idx < n_bytes; idx++) {
        buf[idx] = uart2_getc();
    }
    ring_buffer_write(&ring_buf_elem[1], buf, n_bytes);
    uart_task_resume(1, true);
}

// echo to serail port
static void echo_serial_port(uint8_t itf, uint8_t buf[], uint32_t count)
{
    if (itf == 0) {
        uart1_nputs((const char *)buf, count);
    } else {
        uart2_nputs((const char *)buf, count);
    }
}

void config_serial_port(uint8_t itf, cdc_line_coding_t const *p_coding)
{
    // 5 data bits support 1/1.5 stop bits, 6/7/8 data bits support 1/2 stop bits
    int stop_bits = p_coding->stop_bits + 1;
    if (stop_bits > 2) {
        stop_bits = 2;
    }
    if (itf == 0) {
        uart1_format_set(p_coding->data_bits, p_coding->parity, stop_bits);
        uart1_baud_set(p_coding->bit_rate);
    } else {
        uart2_format_set(p_coding->data_bits, p_coding->parity, stop_bits);
        uart2_baud_set(p_coding->bit_rate);
    }
}
#endif

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
static void cdc_task(void)
{
  uint8_t itf;

  for (itf = 0; itf < CFG_TUD_CDC; itf++) {
    // connected() check for DTR bit
    // Most but not all terminal client set this when making connection
    // if ( tud_cdc_n_connected(itf) )
    {
      if (tud_cdc_n_available(itf)) {
        uint8_t buf[256];

        uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));

        // echo back to serial port
        echo_serial_port(itf, buf, count);
      }
    }
  }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void)rts;

    // DTR = false is counted as disconnected
    if (!dtr) {
        UART_PRINT("itf %d disconnect\n", itf);
    }
    else {
        cdc_line_coding_t coding;
        UART_PRINT("itf %d connect\n", itf);
        tud_cdc_n_get_line_coding(itf, &coding);
        UART_PRINT("line_state: baud=%d\n, data_bits=%d, parity=%d, stop_bits=%d\n",
            coding.bit_rate, coding.data_bits, coding.parity, coding.stop_bits);
        #if CONFIG_UART_ENABLE
        config_serial_port(itf, &coding);
        #endif
    }
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *p_line_coding)
{
    UART_PRINT("itf %d line_coding: baud=%d\n, data_bits=%d, parity=%d, stop_bits=%d\n",
        itf, p_line_coding->bit_rate, p_line_coding->data_bits, p_line_coding->parity, p_line_coding->stop_bits);
    #if CONFIG_UART_ENABLE
    config_serial_port(itf, p_line_coding);
    #endif
}

/**
 ****************************************************************************************
 * @brief test task implementation.
 ****************************************************************************************
 */
void tusb_cdc_dual_ports_demo(void)
{
    #if CONFIG_UART_ENABLE
    static uint8_t ring_buf_pool_1[CONFIG_RING_BUF_SIZE];
    static uint8_t ring_buf_pool_2[CONFIG_RING_BUF_SIZE];
    #endif

    UART_PRINT("\nTINYUSB cdc dual ports demo start\n");

    #if CONFIG_UART_ENABLE
    ring_buffer_init(&ring_buf_elem[0], ring_buf_pool_1, CONFIG_RING_BUF_SIZE);
    ring_buffer_init(&ring_buf_elem[1], ring_buf_pool_2, CONFIG_RING_BUF_SIZE);
    if (rtos_task_create(uart_to_usb_task_1, "u1", CONSOLE_TASK,
                         512, NULL, TASK_PRIORITY_TEST, &task_handle_elem[0])) {
        UART_PRINT("u1 task create fail\n");
        return;
    }
    if (rtos_task_create(uart_to_usb_task_2, "u2", CONSOLE_TASK,
                         512, NULL, TASK_PRIORITY_TEST, &task_handle_elem[1])) {
        UART_PRINT("u2 task create fail\n");
        return;
    }
    uart1_init();
    register_uart1_rx_function(uart1_rx_handler);
    uart2_init();
    register_uart2_rx_function(uart2_rx_handler);
    #endif

    board_init();

    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);
    UART_PRINT("tud_init done\n");

    while (1) {
      tud_task(); // tinyusb device task
      cdc_task();
    }

    UART_PRINT("\nTINYUSB cdc dual ports demo done\n");
}

#endif /* CFG_TUSB_DEMO_CDC_DUAL_PORTS */
