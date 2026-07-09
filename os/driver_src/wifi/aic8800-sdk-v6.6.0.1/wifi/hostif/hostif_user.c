/**
 ****************************************************************************************
 *
 * @file hostif_user.c
 *
 * @brief Implements the hostif user functions
 *
 * Copyright (C) AICSemi 2018-2021
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "hostif.h"
#include "hostif_cfg.h"
#ifdef CONFIG_RWNX_LWIP
#include "fhost_tx.h"
#endif
#include "dbg.h"
#include "dbg_assert.h"
#include "rtos_al.h"
#include "reg_access.h"

/*
 * DEFINITIONS
 ****************************************************************************************
 */
#define CONFIG_HOSTIF_USER_DEBUG_EN     0
#define CONFIG_HOSTIF_SDIO_DATA1_MAP_EN 0

/*
 * VARIABLES
 ****************************************************************************************
 */

uint8_t host_if_inited = 0;
#if defined(CONFIG_SDIO)
const uint32_t sdio_ioen_timeout_us = 0; // time unit: us, value 0 for loop forever
#endif

/*
 * FUNCTIONS
 ****************************************************************************************
 */

#if defined(CONFIG_SDIO)
void sdio_ioen_loop_callback(void)
{
    #if (CONFIG_HOSTIF_USER_DEBUG_EN)
    dbg("sdio ioen loop callback\n");
    #endif
    rtos_task_suspend(1); // suspend 1ms
}
#endif

void process_host_if_ready(struct hostif_msg *msg)
{
    //HOST_TYPE_T type = (HOST_TYPE_T)msg->data;
    #if defined(CONFIG_SDIO)
    dbg("sdio ioen loop\n");
    host_if_sdio_ioen_loop(sdio_ioen_timeout_us, &sdio_ioen_loop_callback);
    #endif
}

void process_host_if_init_done(struct hostif_msg *msg)
{
    #if (CONFIG_HOSTIF_USER_DEBUG_EN)
    dbg("hostif init done\n");
    #endif
    #if PLF_M2D_BLE || PLF_M2D_OTA
    #if defined(CONFIG_USB)
    host_if_usb_init_done();
    #endif
    #endif
}

#if defined(CONFIG_SDIO)
void host_if_user_init_sdio(void)
{
    #if (CONFIG_HOSTIF_SDIO_DATA1_MAP_EN)
    const uint32_t (*ptr_tbl)[2];
    int idx, cnt;
    #if (PLF_AIC8800)
    // for 8800, map data1 isr to gpioa7
    const uint32_t gpio_cfg_tbl[][2] = {
        {0x40500028, 0x00000000},
        {0x4050301C, 0x00000007},
        {0x40100054, 0x00000001},
        {0x4024107C, 0x00000001},
        {0x402400F0, 0x00340022},
    };
    #elif (PLF_AIC8800MC)
    #if 1
    // for 8800mc, map data1 isr to gpioa7
    const uint32_t gpio_cfg_tbl[][2] = {
        {0x40500040, 0x00000000},
        {0x4050401C, 0x00000007},
        {0x40100030, 0x00000001},
        {0x40241020, 0x00000001},
        {0x402400F0, 0x00340022},
    };
    #else
    // for 8800mc, map data1 isr to gpiob1
    const uint32_t gpio_cfg_tbl[][2] = {
        {0x40504044, 0x00000002},
        {0x40500060, 0x03020700},
        {0x40500040, 0x00000000},
        {0x40100030, 0x00000001},
        {0x40241020, 0x00000001},
        {0x402400F0, 0x00340022},
    };
    #endif
    #elif (PLF_AIC8800M40)
    #if 1
    // for 8800m40, map data1 isr to gpioa7
    const uint32_t gpio_cfg_tbl[][2] = {
        {0x4050401C, 0x00000007},
        {0x40500040, 0x00000000},
        {0x40100030, 0x00000001},
        {0x40241020, 0x00000001},
        {0x40240030, 0x00000004},
    };
    #else
    // for 8800m40, map data1 isr to gpiob1
    const uint32_t gpio_cfg_tbl[][2] = {
        {0x40504084, 0x00000006},
        {0x40500040, 0x00000000},
        {0x40100030, 0x00000001},
        {0x40241020, 0x00000001},
        {0x40240030, 0x00000004},
        {0x40240020, 0x03020700},
    };
    #endif
    #endif
    ptr_tbl = gpio_cfg_tbl;
    cnt = sizeof(gpio_cfg_tbl) / sizeof(uint32_t) / 2;
    for (idx = 0; idx < cnt; idx++) {
        REG_PL_WR(ptr_tbl[idx][0], ptr_tbl[idx][1]);
    }
    #endif
}
#endif

/**
 * @brief       This function init sw & hw host interface
 * @param[in]   is_repower: Repower or not
 */
void init_host(int is_repower)
{
    #if defined(CONFIG_SDIO)
    host_if_sw_init_sdio();
    host_if_hw_init_sdio();
    host_if_user_init_sdio();

    #elif defined(CONFIG_USB)
    host_if_sw_init_usb();
    host_if_hw_init_usb();
    #endif

    if (is_repower == 0) {
        #if defined(CFG_DEVICE_IPC)
        hostif_ipc_cntrl_init(__NVIC_PRIO_LOWEST);
        #endif

        // Main application task
        if (hostif_application_init())
        {
            dbg("app init err\r\n");
            ASSERT_ERR(0);
        }
    }
    host_if_inited = 1;
}

void host_if_repower(void)
{
    if (host_if_inited) {
        return;
    }
    init_host(1);
}

void host_if_poweroff(void)
{
    if (!host_if_inited) {
        return;
    }
    #if defined(CONFIG_SDIO)
    host_if_deinit_sdio();

    #elif defined(CONFIG_USB)
    host_if_deinit_usb();
    #endif
    host_if_inited = 0;
}

void host_if_stop(void)
{
    #if defined(CONFIG_SDIO)
    host_if_stop_sdio();

    #elif defined(CONFIG_USB)
    host_if_stop_usb();
    #endif
}

void host_if_info_dump(void)
{
    #if defined(CONFIG_SDIO)
    host_if_info_dump_sdio();

    #elif defined(CONFIG_USB)
    host_if_info_dump_usb();
    #endif
}

__WEAK void host_if_rawdata_rx_callback(unsigned char *rx_data, unsigned int rx_len)
{
    unsigned int dump_len;
    dbg("host data recv, len=%d\r\n", rx_len);
    if (rx_len > 16) {
        dump_len = 16;
    } else {
        dump_len = rx_len;
    }
    dump_mem((uint32_t)rx_data, dump_len, 1, false);
}

#ifdef CONFIG_RWNX_LWIP
uint32_t host_a2e_data_wait_ms = 0;

void host_if_rxc_data_handler(unsigned char *rx_data, unsigned int rx_len)
{
    HOST_MODE_T if_mode = hostif_mode_get();
    if (HOST_VNET_MODE == if_mode) {
        do {
            int ret = fhost_tx_direct((uint8_t *)rx_data, (uint32_t)rx_len);
            if (ret) {
                rtos_task_suspend(1);
                host_a2e_data_wait_ms++;
                if (host_a2e_data_wait_ms > fhost_config_value_get(FHOST_CFG_TX_LFT)) {
                    dbg("tx direct: %d, wait: %d\n", ret, host_a2e_data_wait_ms);
                    host_a2e_data_wait_ms = 0;
                    break; // drop current pkt
                }
            } else {
                host_a2e_data_wait_ms = 0;
                break;
            }
        } while (1);
    } else if (HOST_RAWDATA_MODE == if_mode) {
        host_if_rawdata_rx_callback(rx_data, rx_len);
    }
}
#endif

#if (CONFIG_HOSTIF_USER_DEBUG_EN)
#if defined(CONFIG_SDIO)
#if (PLF_AIC8800 || PLF_AIC8800MC)
void sdio_isr_sdio2ahb_llst_err_callback(uint32_t err_status)
{
    host_if_info_dump();
    if (err_status) {
        //panic();
    }
}
#else
void sdio_isr_func1_error_intr_callback(uint32_t err_status)
{
    host_if_info_dump();
    if (err_status & ~(0x01UL << 11)) {
        //panic();
    }
}
#endif
#endif
#endif
