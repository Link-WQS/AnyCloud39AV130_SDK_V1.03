/**
 ****************************************************************************************
 *
 * @file fhostif_cmd.c
 *
 * @brief Defines the host command parsing and handling functions
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
#include "dbg.h"
#if defined(CFG_HOSTIF) && defined(CONFIG_RWNX_LWIP)
#include "net_al.h"
#include "wlan_if.h"
#include "gpio_api.h"
#include "sleep_api.h"
#include "fhost.h"
#include "fhost_tx.h"
#include "fhost_cntrl.h"
#include "fhost_config.h"
#include "fhostif_cmd.h"
#include "rwnx_rx.h"
#include "rwnx_utils.h"
#include "co_main.h"
#include "inet.h"
#include "hostif_cfg.h"
#include "temp_comp.h"
#include "ipc_host.h"
#if (PLF_OTA)
#include "aic_ota.h"
#include "flash_api.h"
#endif
#if PLF_BLE_ONLY
#include "ble_task.h"
#endif
#ifdef CFG_FHOSTIF_IPC_FC
#include "hostif.h"
#endif
#if PLF_AIC8800M40
#include "reg_anareg.h"
#endif

#include "user_app.h"
#include "user_hal_gpio.h"

#define HOSTIF_KEEP_ALIVE 1
#define HOST_CNTRL_DHCP   0
#define HOST_OTA_REBOOT   0

#if HOSTIF_KEEP_ALIVE
struct custom_msg_keep_alive
{
    bool    driver_load_check;
    bool    first_timer_check;
    int32_t local_timestamp;
    int32_t alive_timestamp;
    TimerHandle_t keep_alive_timer;
    co_timer *start_alive_delay_timer;
    // Situation: 1.use sdio; 2.use usb with HOSTIF_OPT
    #if defined(CONFIG_SDIO) || defined(CFG_HOSTIF_OPT)
    co_timer *hostif_repower_delay_timer;
    #endif
} keep_alive;
#endif /* HOSTIF_KEEP_ALIVE */

uint8_t sleep_irq_evt_handled = 0;
uint8_t sleep_irq_wakeup_soc = 0;
static rtos_queue sleep_irq_evt_queue = NULL;
host_driver_t driver_st = HOST_DRIVER_RMMOD;

static int enter_sleep = 0;
TimerHandle_t hostif_timer = NULL;

extern void tcpip_task_stop(void);
extern void tcpip_task_start(void);

/**
 * @Brief: Any wifi process is no-block.
 *         Modify from 2023.10.30
 */
static rtos_queue wifi_queue;
void wifi_msg_write(struct wifi_noblock_msg *msg)
{
    int ret;
    ret = rtos_queue_write(wifi_queue, msg, 0, true);
    if (ret) {
        dbg("ERR: queue is full, wait and resend msg\n");
        // Can not write to queue, free msg buffer. Drop the cmd.
        if (msg->id == WIFI_STA_CONNECT || msg->id == WIFI_AP_START) {
            rtos_free(msg->ssid);
            rtos_free(msg->pw);
        }
        HOST_TYPE_T host_type = hostif_type_get();
        struct custom_msg_common *ind = (struct custom_msg_common *)
            e2a_msg_alloc(host_type, CUSTOM_MSG_WIFI_FAIL_IND, 0, 0, sizeof(struct custom_msg_common));
        ind->status = (int8_t)msg->id;
        e2a_msg_send(host_type, ind);
    }
}

int custom_msg_connect_status_ind_handler(uint16_t const host_type,         void *param)
{
    struct custom_msg_connect_ind *p = (struct custom_msg_connect_ind *) param;
    // Allocate the message response structure
    struct custom_msg_connect_ind *ind = (struct custom_msg_connect_ind *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_CONNECT_IND, 0, 0, sizeof(struct custom_msg_connect_ind));
    if (ind == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }

    if (HOSTIF_ST_IDLE == get_hostif_wlan_status()) {
        set_hostif_wlan_status(HOSTIF_ST_AWAKE);
        dbg("HOSTIF_ST: AWAKE\n");
    }

    ind->ip = p->ip;
    ind->gw = p->gw;
    ind->mk = p->mk;
    ind->rssi = data_pkt_rssi_get();
    wlan_get_sta_connect_ssid(ind->ussid);

    // Post the confirm message to the host
    e2a_msg_send(host_type, ind);

    return KE_MSG_CONSUMED;
}

int custom_msg_disconnect_status_ind_handler(uint16_t const host_type)
{
    // Allocate the message response structure
    struct custom_msg_common *ind = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_DISCONNECT_IND, 0, 0, sizeof(struct custom_msg_common));
    if (ind == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }

    if (HOSTIF_ST_AWAKE == get_hostif_wlan_status()) {
        set_hostif_wlan_status(HOSTIF_ST_IDLE);
        dbg("HOSTIF_ST: IDLE\n");
    }

    ind->status = 1;

    // Post the confirm message to the host
    e2a_msg_send(host_type, ind);

    return KE_MSG_CONSUMED;
}

static uint8_t prev_connect_ssid[SSID_LEN] = {0};
static uint8_t prev_connect_pw[PASS_LEN] = {0};
static uint8_t prev_connect_status = WLAN_DISCONNECT;

static int wifi_process_sta_connect(struct wifi_noblock_msg *msg)
{
    int ret = 0;
    ret = wlan_start_sta(msg->ssid, msg->pw, msg->timeouts);

    if (!ret) {
        strcpy(prev_connect_ssid, msg->ssid);
        strcpy(prev_connect_pw, msg->pw);
        prev_connect_status = WLAN_CONNECTED;
        dbg("%s: prev_connect_ssid %s prev_connect_pw %s\n", __func__, prev_connect_ssid, prev_connect_pw);
        rtos_free(msg->ssid);
        rtos_free(msg->pw);
        return ret;
    }

    dbg("WLAN connect failed.\n");
    prev_connect_status = WLAN_DISCONNECT;
    rtos_free(msg->ssid);
    rtos_free(msg->pw);

    // If sta-connect fail, send failure reason to host.
    HOST_TYPE_T host_type = hostif_type_get();
    struct custom_msg_common *result_cfm = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_CONNECT_RESULT_CFM, 0, 0, sizeof(struct custom_msg_common));
    if (result_cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }
    if (ret == -12) {
        result_cfm->status = WLAN_PSWD_WRONG;
    } else {
        result_cfm->status = WLAN_CONN_FAIL;
    }
    e2a_msg_send(host_type, result_cfm);
    return ret;
}

#define STA_CONNECT_TIMEOUT  20000
static int custom_msg_connect_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    struct wifi_noblock_msg msg;

    // Set DTIM. When AIC8800x enter sleep, DTIM is 10.
    set_deepsleep_param(10, 1);

    dbg("custom_msg_connect_handler\r\n");
    struct custom_msg_common *cfm = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_CONNECT_CFM, 0, 0, sizeof(struct custom_msg_common));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }
    e2a_msg_send(host_type, cfm);

    // If sta is CONN or ap is START, close it first.
    if (WLAN_CONNECTED == wlan_get_connect_status()) {
        msg.id = WIFI_STA_DISCONNECT;
        wifi_msg_write(&msg);
    }
    #ifdef CFG_SOFTAP
    if (wlan_get_softap_status()) {
        msg.id = WIFI_AP_STOP;
        wifi_msg_write(&msg);
    }
    #endif

    uint8_t *pssid = rtos_malloc(SSID_LEN);
    if (pssid == NULL) {
        dbg("alloc buff fail.\n");
        return KE_MSG_CONSUMED;
    }
    uint8_t *ppass = rtos_malloc(PASS_LEN);
    if (ppass == NULL) {
        rtos_free(pssid);
        dbg("alloc buff fail.\n");
        return KE_MSG_CONSUMED;
    }

    struct custom_msg_connect_req *req = (struct custom_msg_connect_req *)param;
    // strlen(req->ssid) < 32
    memcpy(pssid, req->ssid, strlen((const char *)req->ssid));
    pssid[strlen((const char *)req->ssid)] = '\0';
    memcpy(ppass, req->pw, strlen((const char *)req->pw));
    ppass[strlen((const char *)req->pw)] = '\0';


    msg.ssid = pssid;
    msg.pw   = ppass;
    msg.timeouts = WLAN_CONNECT_TO_CFG_2_PARAM(WLAN_CONNECT_CFG_DISABLE_AUTO_RECONN, STA_CONNECT_TIMEOUT);
    msg.id = WIFI_STA_CONNECT;
    dbg("STA-ssid: %s\n", msg.ssid);
    //dbg("STA-pass: %s\n", msg.pw);
    {
        int i;
        char *p = (char*)msg.pw;

        dbg("STA-pass: ");
        for (i = 0; i < strlen(p); i++) {
            if ((p[i] >= 32) && (p[i] <= 126))
                dbg("%02x(%c) ", p[i], p[i]);
            else
                dbg("%02x ", p[i]);
        }
        dbg("\n");
    }
    wifi_msg_write(&msg);

    return KE_MSG_CONSUMED;
}

static void wifi_process_sta_disconnect(void)
{
    wlan_disconnect_sta(0);
    prev_connect_status = WLAN_DISCONNECT;
}

static int custom_msg_disconnect_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    struct wifi_noblock_msg msg;
    struct custom_msg_common *cfm = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_DISCONNECT_CFM, 0, 0, sizeof(struct custom_msg_common));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }
    // Send msg confirm, it means AIC8800x start to execute command.
    e2a_msg_send(host_type, cfm);

    if (WLAN_CONNECTED == wlan_get_connect_status()) {
        msg.id = WIFI_STA_DISCONNECT;
        wifi_msg_write(&msg);
    } else {
        dbg("WLAN isn`t CONNECT.\n");
    }

    return KE_MSG_CONSUMED;
}

/**
 * @Brief: custom_msg_open_ble_handler
 */
static int custom_msg_open_ble_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    #if PLF_BLE_ONLY
    if (ble_status != BLE_STATUS_INIT) {
        if(ble_task_need_pon_get()==false){
            ble_task_need_pon_set(true);
            ble_task_init(INIT_CONSOLE);
        }
    } else {
        dbg("ble is opened!\n");
    }

    struct custom_msg_common *cfm = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_OPEN_BLE_CFM, 0, 0, sizeof(struct custom_msg_common));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }
    e2a_msg_send(host_type, cfm);
    #endif /* PLF_BLE_ONLY */

    return KE_MSG_CONSUMED;
}

/**
 * @Brief: custom_msg_close_ble_handler
 *   - If open ble function, need to close ble first, and then wifi can enter sleep
 *   - successfully.
 */
static int custom_msg_close_ble_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    #if PLF_BLE_ONLY
    if (ble_status != BLE_STATUS_DEINIT) {
        ble_task_deinit();
    } else {
        dbg("ble is closed!\n");
    }

    struct custom_msg_common *cfm = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_CLOSE_BLE_CFM, 0, 0, sizeof(struct custom_msg_common));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }
    e2a_msg_send(host_type, cfm);
    #endif /* PLF_BLE_ONLY */

    return KE_MSG_CONSUMED;
}

/**
 * @Brief: custom_msg_ble_status_ind_handler
 *   - When ble task status is changed, opened or closed, send msg to host.
 *   - Host can send enter-sleep cmd until getting confirm about ble-close cmd.
 * @param:
 *   - st: 1 -> ble open successfully
 *         0 -> ble close succcessfully
 */
void custom_msg_ble_status_ind_handler(uint16_t const host_type, uint32_t st)
{
    #if PLF_BLE_ONLY
    uint32_t msg_type = 0;
    hostif_status_e hostif_st = get_hostif_wlan_status();

    if (HOSTIF_ST_INIT == hostif_st || HOSTIF_ST_DEEPSLEEP == hostif_st) {
        dbg("HOSTIF_ST: not valid\n");
        return;
    }

    if (st == 1) {
        msg_type = CUSTOM_MSG_OPEN_BLE_IND;
    } else {
        msg_type = CUSTOM_MSG_CLOSE_BLE_IND;
    }

    struct custom_msg_common *ind = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, msg_type, 0, 0, sizeof(struct custom_msg_common));
    if (ind == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return;
    }
    e2a_msg_send(host_type, ind);
    #endif /* PLF_BLE_ONLY */
}

int custom_msg_sleep_status_cfm_handler(uint16_t const host_type, uint32_t st)
{
    uint32_t msg_type = 0;
    if(st == 1)
        msg_type = CUSTOM_MSG_ENTER_SLEEP_CFM;
    else
        msg_type = CUSTOM_MSG_EXIT_SLEEP_CFM;

    struct custom_msg_common *cfm = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, msg_type, 0, 0, sizeof(struct custom_msg_common));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }

    e2a_msg_send(host_type, cfm);
    return KE_MSG_CONSUMED;
}

#if HOSTIF_KEEP_ALIVE
#define PERIOD_TIMES       3
#define KEEP_ALIVE_PERIOD  200
#define DELAY_START_TIMER  1000
#define LOCAL_ABS(x)       ((x) < 0 ? (-(x)) : (x))
static void keep_alive_init(void)
{
    keep_alive.first_timer_check = true;
    keep_alive.driver_load_check = false;
}
#if defined(CONFIG_SDIO) || defined(CFG_HOSTIF_OPT)
static void hostif_repower_handler(void *cb_param)
{
    host_if_repower();
}
#endif
static void keep_alive_timer_handler(TimerHandle_t xTimer)
{
    // Check whether hostif is alive. If not close hostif.
    //dbg("keep_alive_timer_handler %d, %d\r\n", keep_alive.local_timestamp, keep_alive.alive_timestamp);

    if (keep_alive.first_timer_check == true) {
        keep_alive.first_timer_check = false;
        keep_alive.local_timestamp = keep_alive.alive_timestamp;
    } else {
        if (LOCAL_ABS(keep_alive.local_timestamp - keep_alive.alive_timestamp) > KEEP_ALIVE_PERIOD * PERIOD_TIMES) {
            // After setting HOSTIF_ST_INIT, wifi data-pkg will not send to host.
            set_hostif_wlan_status(HOSTIF_ST_INIT);
            dbg("HOSTIF_ST: INIT\n");
            #if defined(CONFIG_SDIO) || defined(CFG_HOSTIF_OPT)
            // poweroff SDIO/USB interface
            host_if_poweroff();
            #endif

            // close keep_alive_timer
            int ret = rtos_timer_stop(keep_alive.keep_alive_timer, 0);
            if (ret) {
                dbg("stop keep alive timer failed: %d\n", ret);
            }
            ret = rtos_timer_delete(keep_alive.keep_alive_timer, 0);
            if (ret) {
                dbg("delete keep alive timer failed: %d\n", ret);
            }
            keep_alive.keep_alive_timer = NULL;
            keep_alive.first_timer_check = true;
            keep_alive.driver_load_check = false;

            #if defined(CONFIG_SDIO) || defined(CFG_HOSTIF_OPT)
            // delay some time to repower host interface
            co_timer_start(&keep_alive.hostif_repower_delay_timer, 10, NULL, hostif_repower_handler, 0);
            #endif
            dbg("Linux-host can`t keep alive, close host interface.\n");
        } else {
            keep_alive.local_timestamp = rtos_now(false);
        }
    }
}
static void start_keep_alive_timer_handler(void *cb_param)
{
    if ((keep_alive.keep_alive_timer == NULL) && (keep_alive.driver_load_check == true)) {
        //keep_alive.driver_load_check = true;
        keep_alive.keep_alive_timer = rtos_timer_create("hostif_keep_alive_timer",
            (uint32_t)KEEP_ALIVE_PERIOD, pdTRUE, NULL, keep_alive_timer_handler);
        if (keep_alive.keep_alive_timer == NULL) {
            dbg("create keep_alive_timer failed\n");
        }
        int ret = rtos_timer_start(keep_alive.keep_alive_timer, 1000, false);
        if (ret) {
            dbg("start keep alive timer failed: %d\n", ret);
        }
    }
}
#endif /* HOSTIF_KEEP_ALIVE */

//#define WAKEUP_IO_B0
//#define WAKEUP_IO_B1
#define WAKEUP_IO_B2
#define WAKEUP_IO_A

#define CONFIG_GPIOB0_1_RISE_EDGE_EN 1

#define CONSOLE_DEEPSLEEP_IO0_INDEX  0
#define CONSOLE_DEEPSLEEP_IO1_INDEX  1
#define CONSOLE_DEEPSLEEP_IO2_INDEX  2
#define CONSOLE_DEEPSLEEP_IOA_INDEX  2

static void fhostif_gpio_irq_handler(int event)
{
    if (0 == sleep_irq_evt_handled) {
        uint32_t io_index = GPIOIRQ_CB_INDEX(event);
        uint32_t event_edge = GPIOIRQ_CB_EVENT(event);
        dbg("Exit sleep: idx=%d, evt=%d\n", io_index, event_edge);
        rtos_queue_write(sleep_irq_evt_queue, &event, 0, true);
    }
    user_sleep_allow(0);
}

void hostif_reset_handler(TimerHandle_t xTimer)
{
    int ret;

    if (enter_sleep) {
        dbg("poweroff hostif\n");
        host_if_poweroff();
        dbg("repower hostif\n");
        host_if_repower();
    } else {
        ret = rtos_timer_stop(hostif_timer, 0);
        if (ret) {
            dbg("stop timer failed: %d\n", ret);
        }
        rtos_timer_delete(hostif_timer, 0);
    }
}

/**
 * @Brief: gpio wakeup setting example
 *         more configuration please refer do_enter_hibernate in fhost_console.c.
 */
static RTOS_TASK_FCT(sleep_irq_task)
{
    uint32_t event = 0;
    for (;;) {
        if (!rtos_queue_read(sleep_irq_evt_queue, &event, -1, false)) {
            if (sleep_irq_evt_handled) {
                continue;
            }
            dbg("event = 0x%x\r\n", event);
#if 0
            #if (PLF_AIC8800)
            #ifdef WAKEUP_IO_A
            gpioa_force_pull_none_enable(CONSOLE_DEEPSLEEP_IOA_INDEX);
            gpioa_irq_deinit(CONSOLE_DEEPSLEEP_IOA_INDEX);
            user_sleep_wakesrc_set(WAKESRC_GPIOA, 0, 0);
            #endif /* WAKEUP_IO_A */

            #ifdef WAKEUP_IO_B2
            gpiob_force_pull_none_enable(CONSOLE_DEEPSLEEP_IO2_INDEX);
            gpiob_irq_deinit(CONSOLE_DEEPSLEEP_IO2_INDEX);
            user_sleep_wakesrc_set(WAKESRC_GPIOB, 0, 0);
            #endif /* WAKEUP_IO_B2 */
            #endif /* PLF_AIC8800 */


            #if (PLF_AIC8800MC)
            #ifdef WAKEUP_IO_A
            gpioa_force_pull_none_enable(CONSOLE_DEEPSLEEP_IOA_INDEX);
            gpioa_irq_deinit(CONSOLE_DEEPSLEEP_IOA_INDEX);
            user_sleep_wakesrc_set(WAKESRC_GPIOA, 0, WAKEGPIO_ARG(WAKEGPIO_MUX_0, CONSOLE_DEEPSLEEP_IOA_INDEX));
            #endif /* WAKEUP_IO_A */

            #ifdef WAKEUP_IO_B2
            gpiob_force_pull_none_enable(CONSOLE_DEEPSLEEP_IO2_INDEX);
            gpiob_irq_deinit(CONSOLE_DEEPSLEEP_IO2_INDEX);
            user_sleep_wakesrc_set(WAKESRC_GPIOB, 0, WAKEGPIO_ARG(WAKEGPIO_MUX_1, CONSOLE_DEEPSLEEP_IO2_INDEX));
            #endif /* WAKEUP_IO_B2 */
            #endif /* PLF_AIC8800MC */


            #if (PLF_AIC8800M40)
            #ifdef WAKEUP_IO_B0
            gpiob_force_pull_none_enable(CONSOLE_DEEPSLEEP_IO0_INDEX);
            gpiob_irq_deinit(CONSOLE_DEEPSLEEP_IO0_INDEX);
            user_sleep_wakesrc_set(WAKESRC_GPIOB, 0, WAKEGPIO_ARG(WAKEGPIO_MUX_NUM_MAX, CONSOLE_DEEPSLEEP_IO0_INDEX));
            #endif /* WAKEUP_IO_B0 */

            #ifdef WAKEUP_IO_B1
            gpiob_force_pull_none_enable(CONSOLE_DEEPSLEEP_IO1_INDEX);
            gpiob_irq_deinit(CONSOLE_DEEPSLEEP_IO1_INDEX);
            user_sleep_wakesrc_set(WAKESRC_GPIOB, 0, WAKEGPIO_ARG(WAKEGPIO_MUX_NUM_MAX, CONSOLE_DEEPSLEEP_IO1_INDEX));
            #endif /* WAKEUP_IO_B1 */

            #ifdef WAKEUP_IO_B2
            gpiob_force_pull_none_enable(CONSOLE_DEEPSLEEP_IO2_INDEX);
            gpiob_irq_deinit(CONSOLE_DEEPSLEEP_IO2_INDEX);
            user_sleep_wakesrc_set(WAKESRC_GPIOB, 0, WAKEGPIO_ARG(WAKEGPIO_MUX_NUM_MAX, CONSOLE_DEEPSLEEP_IO2_INDEX));
            #endif /* WAKEUP_IO_B2 */

            #ifdef WAKEUP_IO_A
            gpioa_force_pull_none_enable(CONSOLE_DEEPSLEEP_IOA_INDEX);
            gpioa_irq_deinit(CONSOLE_DEEPSLEEP_IOA_INDEX);
            user_sleep_wakesrc_set(WAKESRC_GPIOA, 0, WAKEGPIO_ARG(WAKEGPIO_MUX_0, CONSOLE_DEEPSLEEP_IOA_INDEX));
            #endif /* WAKEUP_IO_A */
            #endif /* PLF_AIC8800M40 */
#endif

            sleep_irq_evt_handled = 1;
            user_clear_wakeup_src();

            dbg("Set PM_LEVEL_ACTIVE\n");
            sleep_level_set(PM_LEVEL_ACTIVE); // this api can't be called in isr
            user_sleep_allow(0);

            if (GPIO_WAKEUP_MCU_BUTTON > 0) {
                if ((sleep_irq_wakeup_soc == 0) && (GPIOIRQ_CB_INDEX(event) == PIN_IDX(GPIO_WAKEUP_MCU_BUTTON))) {
                    dbg("wakeup soc\n");
                    sleep_irq_wakeup_soc = 1;
                    gpio_init(GPIO_WAKEUP_AK);
                    gpio_dir_out(GPIO_WAKEUP_AK);
                    gpio_set(GPIO_WAKEUP_AK);

                    gpio_mask(GPIO_KM01A_PWR_EN);
                    gpio_dir_out(GPIO_KM01A_PWR_EN);
                    gpio_set(GPIO_KM01A_PWR_EN);
                    gpio_init(GPIO_KM01A_PWR_EN);

                    if (GPIO_WIFI_ST > 0) {
                        rtos_task_suspend(100);

                        gpio_dir_out(GPIO_WIFI_ST);
                        gpio_set(GPIO_WIFI_ST);
                    }
                    rtos_task_suspend(1000);
                    if (GPIO_WIFI_ST > 0) {
                        gpio_clr(GPIO_WIFI_ST);
                    }
                    gpio_deinit(GPIO_WAKEUP_AK);
                }
            }

            // if deepsleep before wifi connected, powerup wifi after wakeup
            if ((0 == wlan_connected) && (PM_LEVEL_DEEP_SLEEP == sleep_level_get())) {
                int wifi_state = wifi_fw_is_active();
                if (wifi_state == 0)
                {
                    tcpip_task_start();
#if PLF_MODULE_TEMP_COMP
                    temp_comp_init();
                    temp_comp_start();
#endif /* PLF_MODULE_TEMP_COMP */
                    wifi_fw_repower(); // this api can't be called in isr
                }
                else {
                    dbg("wifi_st=%d\n", wifi_state);
                }
            }

            // repower host interface
            host_if_repower();
            // User GPIO to wakeup linux-host
            // TODO:
            //
        }
    }
}

/**
 * @Brief: 8800MC SLEEP mode and wakeup-src
 *   -Support sleep mode:  LIGHT_SLEEP and DEEP_SLEEP
 *   -Support wakeup-src:  All PA0-15 and PB0-3 are OK in two mode.
 *   -Support wkp-src num: Three at the same time. Use GPIO_MUX_0-2.
 */
static void fhostif_gpio_irq_wakeup_handler(POWER_MODE_LEVEL_T level)
{
    dbg("%s\n", __func__);
#if 0
    #if (PLF_AIC8800)
    #ifdef WAKEUP_IO_A
    gpioa_force_pull_dn_enable(CONSOLE_DEEPSLEEP_IOA_INDEX);
    gpioa_irq_init(CONSOLE_DEEPSLEEP_IOA_INDEX, GPIOIRQ_TYPE_EDGE_BOTH, fhostif_gpio_irq_handler);
    user_sleep_wakesrc_set(WAKESRC_GPIOA, 1, 0);
    #endif

    #ifdef WAKEUP_IO_B2
    gpiob_force_pull_dn_enable(CONSOLE_DEEPSLEEP_IO2_INDEX);
    gpiob_irq_init(CONSOLE_DEEPSLEEP_IO2_INDEX, GPIOIRQ_TYPE_EDGE_BOTH, fhostif_gpio_irq_handler, 0);
    user_sleep_wakesrc_set(WAKESRC_GPIOB, 1, 0);
    #endif
    #endif /* PLF_AIC8800 */


    #if (PLF_AIC8800MC)
    #ifdef WAKEUP_IO_A
    gpioa_force_pull_dn_enable(CONSOLE_DEEPSLEEP_IOA_INDEX);
    gpioa_irq_init(CONSOLE_DEEPSLEEP_IOA_INDEX, GPIOIRQ_TYPE_EDGE_BOTH, fhostif_gpio_irq_handler);
    user_sleep_wakesrc_set(WAKESRC_GPIOA, 1, WAKEGPIO_ARG(WAKEGPIO_MUX_0, CONSOLE_DEEPSLEEP_IOA_INDEX));
    #endif /* WAKEUP_IO_A */

    #ifdef WAKEUP_IO_B2
    gpiob_force_pull_dn_enable(CONSOLE_DEEPSLEEP_IO2_INDEX);
    gpiob_irq_init(CONSOLE_DEEPSLEEP_IO2_INDEX, GPIOIRQ_TYPE_EDGE_BOTH, fhostif_gpio_irq_handler);
    user_sleep_wakesrc_set(WAKESRC_GPIOB, 1, WAKEGPIO_ARG(WAKEGPIO_MUX_1, CONSOLE_DEEPSLEEP_IO2_INDEX));
    #endif /* WAKEUP_IO_B2 */
    #endif /* PLF_AIC8800MC */


    #if (PLF_AIC8800M40)
    #ifdef WAKEUP_IO_B0
    // gpiob0 as wake src
    #if CONFIG_GPIOB0_1_RISE_EDGE_EN
    // EDGE_RISE => analog_pull_down
    anareg1_gpiob_pull_up_int_clrb(CONSOLE_DEEPSLEEP_IO0_INDEX);
    anareg1_gpiob_pull_dn_int_setb(CONSOLE_DEEPSLEEP_IO0_INDEX);
    gpiob_irq_init(CONSOLE_DEEPSLEEP_IO0_INDEX, GPIOIRQ_TYPE_EDGE_RISE, fhostif_gpio_irq_handler);
    #else
    // EDGE_FALL => analog_pull_up
    anareg1_gpiob_pull_up_int_setb(CONSOLE_DEEPSLEEP_IO0_INDEX);
    anareg1_gpiob_pull_dn_int_clrb(CONSOLE_DEEPSLEEP_IO0_INDEX);
    gpiob_irq_init(CONSOLE_DEEPSLEEP_IO0_INDEX, GPIOIRQ_TYPE_EDGE_FALL, fhostif_gpio_irq_handler);
    #endif
    user_sleep_wakesrc_set(WAKESRC_GPIOB, 1, WAKEGPIO_ARG(WAKEGPIO_MUX_NUM_MAX, CONSOLE_DEEPSLEEP_IO0_INDEX));
    #endif /* WAKEUP_IO_B0 */

    #ifdef WAKEUP_IO_B1
    // gpiob1 as wake src
    #if CONFIG_GPIOB0_1_RISE_EDGE_EN
    // EDGE_RISE => analog_pull_down
    anareg1_gpiob_pull_up_int_clrb(CONSOLE_DEEPSLEEP_IO1_INDEX);
    anareg1_gpiob_pull_dn_int_setb(CONSOLE_DEEPSLEEP_IO1_INDEX);
    gpiob_irq_init(CONSOLE_DEEPSLEEP_IO1_INDEX, GPIOIRQ_TYPE_EDGE_RISE, fhostif_gpio_irq_handler);
    #else
    // EDGE_FALL => analog_pull_up
    anareg1_gpiob_pull_up_int_setb(CONSOLE_DEEPSLEEP_IO1_INDEX);
    anareg1_gpiob_pull_dn_int_clrb(CONSOLE_DEEPSLEEP_IO1_INDEX);
    gpiob_irq_init(CONSOLE_DEEPSLEEP_IO1_INDEX, GPIOIRQ_TYPE_EDGE_FALL, fhostif_gpio_irq_handler);
    #endif
    user_sleep_wakesrc_set(WAKESRC_GPIOB, 1, WAKEGPIO_ARG(WAKEGPIO_MUX_NUM_MAX, CONSOLE_DEEPSLEEP_IO1_INDEX));
    #endif /* WAKEUP_IO_B1 */

    #ifdef WAKEUP_IO_B2
    // gpiob2 as wake src
    gpiob_force_pull_dn_enable(CONSOLE_DEEPSLEEP_IO2_INDEX);
    gpiob_irq_init(CONSOLE_DEEPSLEEP_IO2_INDEX, GPIOIRQ_TYPE_EDGE_BOTH, fhostif_gpio_irq_handler);
    user_sleep_wakesrc_set(WAKESRC_GPIOB, 1, WAKEGPIO_ARG(WAKEGPIO_MUX_NUM_MAX, CONSOLE_DEEPSLEEP_IO2_INDEX));
    #endif /* WAKEUP_IO_B2 */

    #ifdef WAKEUP_IO_A
    // gpioa0 as wake src
    gpioa_force_pull_dn_enable(CONSOLE_DEEPSLEEP_IOA_INDEX);
    gpioa_irq_init(CONSOLE_DEEPSLEEP_IOA_INDEX, GPIOIRQ_TYPE_EDGE_BOTH, fhostif_gpio_irq_handler);
    user_sleep_wakesrc_set(WAKESRC_GPIOA, 1, WAKEGPIO_ARG(WAKEGPIO_MUX_0, CONSOLE_DEEPSLEEP_IOA_INDEX));
    #endif /* WAKEUP_IO_A */
    #endif /* PLF_AIC8800M40 */
#endif
    user_setup_wakeup_src(fhostif_gpio_irq_handler);

    // set wakeup gpio finish
    sleep_irq_evt_handled = 0;
    sleep_irq_wakeup_soc = 0;

    if ((0 == wlan_connected) && (PM_LEVEL_DEEP_SLEEP == level)) {
        tcpip_task_stop();
        #if PLF_MODULE_TEMP_COMP
        temp_comp_deinit();
        #endif /* PLF_MODULE_TEMP_COMP */
        wifi_disconnected_sleep_allow(1);
    }

    // Move user_sleep_allow to timer-handler. Make sure that it executes host_if_poweroff
    // to disenable SDIO/USB interface before entering sleep.
    //user_sleep_allow(1);
}

#define POWEROFF_DELAY_TIME  800
static co_timer *co_hostif_poweroff_timer = NULL;
static void co_hostif_poweroff_timer_handler(void *cb_param)
{
    dbg("co_hostif_poweroff_timer_handler\r\n");
    host_if_poweroff();
    fhostif_gpio_irq_wakeup_handler(PM_LEVEL_DEEP_SLEEP);

    gpio_init(GPIO_WAKEUP_AK);
    gpio_dir_out(GPIO_WAKEUP_AK);
    gpio_clr(GPIO_WAKEUP_AK);

    // User`s GPIO to poweroff linux-host
    // TODO:
    if (wlan_connected) {
        user_do_tcp_client_wakeup();
    }

    // poweroff host interface
    user_sleep_allow(1);
}

static int custom_msg_enter_sleep_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    dbg("Enter sleep\r\n");
    enter_sleep++;

    //if (wlan_connected) {
    //    user_do_tcp_client_wakeup();
    //}

    #if HOSTIF_KEEP_ALIVE
    // close keep_alive timer
    if (keep_alive.keep_alive_timer != NULL) {
        int ret = rtos_timer_stop(keep_alive.keep_alive_timer, 0);
        if (ret) {
            dbg("stop keep alive timer failed: %d\n", ret);
        }
        ret = rtos_timer_delete(keep_alive.keep_alive_timer, 0);
        if (ret) {
            dbg("delete keep alive timer failed: %d\n", ret);
        }
        keep_alive.keep_alive_timer = NULL;
        keep_alive.first_timer_check = true;
        keep_alive.driver_load_check = false;
    }
    #endif /* HOSTIF_KEEP_ALIVE */

    #if PLF_BLE_ONLY
    if(ble_status == BLE_STATUS_INIT) {
        dbg("Please close ble task first.\n");
        return KE_MSG_CONSUMED;
    }
    #endif /* PLF_BLE_ONLY */

    // If Linux-host send sleep-cmd, it means that sdio/usb will be disconnected,
    // AIC8800x must set HOSTIF_ST_DEEPSLEEP. Then AIC8800x will not send pkt to
    // linux-host.
    set_hostif_wlan_status(HOSTIF_ST_DEEPSLEEP);
    dbg("HOSTIF_ST: DEEPSLEEP\n");

    #if (PLF_AIC8800)
    POWER_MODE_LEVEL_T sleep_mode = PM_LEVEL_LIGHT_SLEEP;
    #elif (PLF_AIC8800MC) || (PLF_AIC8800M40)
    POWER_MODE_LEVEL_T sleep_mode = PM_LEVEL_DEEP_SLEEP;
    #endif

    sleep_level_set(sleep_mode);
    //fhostif_gpio_irq_wakeup_handler(sleep_mode);
    dbg("Set sleep mode: %d\n", sleep_mode);

    // If use this cfm_handler, make sure Linux-host can receive msg and give a cfm return.
    custom_msg_sleep_status_cfm_handler(host_type, 1);

    // Delay some time to call host_if_poweroff in order to free buffer safety.
    co_timer_start(&co_hostif_poweroff_timer, POWEROFF_DELAY_TIME, NULL, co_hostif_poweroff_timer_handler, 0);

    // If get cmd to sleep, we believe that host has rmmoded driver.
    // Don`t use sdio/usb to send pkt any more.
    driver_st = HOST_DRIVER_RMMOD;
    dbg("Host driver: RMMOD\n");

    return KE_MSG_CONSUMED;
}

/**
 * @Brief: custom_msg_exit_sleep_handler
 *   - Linux-host can`t wake up AIC8800x! If AIC8800x enter sleep, it will disconnect
 *   - with linux-host in USB/SDIO interface. This handler is only for host-cmd test.
 */
static int custom_msg_exit_sleep_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    dbg("Exit sleep\r\n");
    enter_sleep--;

    dbg("Set PM_LEVEL_ACTIVE\n");
    sleep_level_set(PM_LEVEL_ACTIVE);

    if ((uint8_t)wlan_get_connect_status() == WLAN_CONNECTED) {
        set_hostif_wlan_status(HOSTIF_ST_AWAKE);
        dbg("HOSTIF_ST: AWAKE\n");
    } else {
        set_hostif_wlan_status(HOSTIF_ST_IDLE);
        dbg("HOSTIF_ST: IDLE\n");
    }

    custom_msg_sleep_status_cfm_handler(host_type, 0);
    return KE_MSG_CONSUMED;
}

static int custom_msg_set_mac_addr_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    uint8_t *mac_addr = (uint8_t *)param;
    dbg("Set mac addr\r\n");
    set_mac_address(mac_addr);

    return KE_MSG_CONSUMED;
}

static int custom_msg_get_mac_addr_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    // Allocate the message response structure
    struct custom_msg_mac_addr_cfm *mac_addr = (struct custom_msg_mac_addr_cfm *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_GET_MAC_ADDR_CFM, 0, 0, sizeof(struct custom_msg_mac_addr_cfm));
    if (mac_addr == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }

    dbg("[Send msg] mac addr: ");
    uint8_t *addr = get_mac_address();
    for(int i=0; i<6; i++){
        mac_addr->chip_mac_addr[i] = addr[i];
        dbg("%x ", mac_addr->chip_mac_addr[i]);
    }
    dbg("\n");

    // Post the confirm message to the host
    e2a_msg_send(host_type, mac_addr);
    return KE_MSG_CONSUMED;
}

/**
 * @Brief: custom_msg_get_wlan_status_handler
 *   - When this handler is called, it means host driver has loaded and SDIO/USB interface
 *   - TX/RX is OK.
 */
static int custom_msg_get_wlan_status_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    // If linux-host get wlan status first, start keep_alive_timer.
    #if HOSTIF_KEEP_ALIVE
    if (keep_alive.driver_load_check == false) {
        keep_alive.driver_load_check = true;
        co_timer_start(&keep_alive.start_alive_delay_timer,
                       DELAY_START_TIMER, NULL, start_keep_alive_timer_handler, 0);
    }
    #endif

    // Allocate the message response structure
    struct custom_msg_wlan_status_cfm *cfm = (struct custom_msg_wlan_status_cfm *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_GET_WLAN_STATUS_CFM, 0, 0, sizeof(struct custom_msg_wlan_status_cfm));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }

    cfm->status = (uint8_t)wlan_get_connect_status();
    if (cfm->status == WLAN_DISCONNECT) {
        cfm->ip = 0;
        cfm->gw = 0;
        cfm->ussid[0] = '\0';
        dbg("WLAN_STATUS: DISCONNECT\r\n");

        // Set HOSTIF_ST_IDLE, it means that Linux-host has insmoded driver, and only msg is connected.
        set_hostif_wlan_status(HOSTIF_ST_IDLE);
        dbg("HOSTIF_ST: IDLE\n");

        #if HOST_CNTRL_DHCP
        is_fixed_ip = 1;
        fixed_ip = inet_addr("1.1.1.1");
        fixed_gw = inet_addr("1.1.1.1");
        fixed_mask = inet_addr("255.255.255.0");
        #endif

        // scan or connect status
        {
            uint32_t fvif_idx = 0;
            struct fhost_vif_tag *fhost_vif = &fhost_env.vif[fvif_idx];
            if (fhost_vif->connect_in_progress) {
                cfm->status = WLAN_CONNECTING;
                dbg("vif CONNECTING\n", cfm->status);
            } else if (fhost_vif->scan_in_progress) {
                cfm->status = WLAN_SCANNING;
                dbg("vif SCANNING\n", cfm->status);
            }
        }
    } else if (cfm->status == WLAN_CONNECTED) {
        cfm->rssi = data_pkt_rssi_get();
        wlan_get_sta_connect_ssid(cfm->ussid);
        net_if_get_ip(net_if_find_from_wifi_idx(0), &cfm->ip, &cfm->mk, &cfm->gw);
        dbg("WLAN_STATUS: CONNECT to \'%s\' (%3d dBm)\r\n", cfm->ussid, cfm->rssi);
        dbg("[Send msg] ip: %d.%d.%d.%d,  gw: %d.%d.%d.%d,  mk: %d.%d.%d.%d\n",
            (cfm->ip >> 0) & 0xff, (cfm->ip >> 8) & 0xff, (cfm->ip >> 16) & 0xff, (cfm->ip >> 24) & 0xff,
            (cfm->gw >> 0) & 0xff, (cfm->gw >> 8) & 0xff, (cfm->gw >> 16) & 0xff, (cfm->gw >> 24) & 0xff,
            (cfm->mk >> 0) & 0xff, (cfm->mk >> 8) & 0xff, (cfm->mk >> 16) & 0xff, (cfm->mk >> 24) & 0xff);

        // Set HOSTIF_ST_AWAKE, it means that Linux-host has insmoded driver, msg is connected, and
        // AIC8800x can send data-pkt to Linux-host.
        set_hostif_wlan_status(HOSTIF_ST_AWAKE);
        dbg("HOSTIF_ST: AWAKE\n");

    } else {
        // Never go to here.
        dbg("Get wlan status ERROR\n");
    }
    e2a_msg_send(host_type, cfm);

    // If get here, we believe that host has insmoded sdio/usb driver.
    // Hostif is ready to send and recv.
    if (driver_st != HOST_DRIVER_INSMOD) {
        driver_st = HOST_DRIVER_INSMOD;
        dbg("Host driver: INSMOD\n");
    }

    // update fhostif ipc fc status
    #ifdef CFG_FHOSTIF_IPC_FC
    if (rwnx_hostif_ipc_fc_status_get()) {
        host_if_sdio_ipc_fc_set(0x0);
        rwnx_hostif_ipc_fc_status_set(0);
    }
    #endif

    #ifdef CFG_SOFTAP
    if (!wlan_get_softap_status()) {
        return KE_MSG_CONSUMED;
    }
    struct custom_msg_ap_status_cfm *cfm2 = (struct custom_msg_ap_status_cfm *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_START_AP_IND, 0, 0, sizeof(struct custom_msg_ap_status_cfm));
    if (cfm2 == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }
    // In SOFTAP mode, ip == gw generally.
    cfm2->status = AIC_AP_START;
    cfm2->ip = get_ap_ip_addr();
    cfm2->gw = cfm2->ip;
    cfm2->mask = get_ap_subnet_mask();

    e2a_msg_send(host_type, cfm2);
    #endif /* CFG_SOFTAP */

    #if PLF_BLE_ONLY
    if (ble_status == BLE_STATUS_DEFAULT) {
        return KE_MSG_CONSUMED;
    }

    if (ble_status == BLE_STATUS_INIT) {
        custom_msg_ble_status_ind_handler(host_type, 1);
    } else if (ble_status == BLE_STATUS_DEINIT) {
        custom_msg_ble_status_ind_handler(host_type, 0);
    } else {
        dbg("ble_status default\n");
    }
    #endif /* PLF_BLE_ONLY */

    return KE_MSG_CONSUMED;
}

static int custom_msg_get_vnet_setting_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    // Allocate the message response structure
    struct custom_msg_aic8800x_vnet_setting_cfm *cfm = (struct custom_msg_aic8800x_vnet_setting_cfm *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_GET_VNET_SETTING_CFM, 0, 0, sizeof(struct custom_msg_aic8800x_vnet_setting_cfm));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }

    #if HOSTIF_KEEP_ALIVE
    cfm->keep_alive = KEEP_ALIVE_PERIOD;
    #else
    cfm->keep_alive = 0;
    #endif

    #if PLF_BLE_ONLY
    cfm->ble_enable = 1;
    #else
    cfm->ble_enable = 0;
    #endif

    #ifdef CFG_LZMA_APP
    cfm->lzma_enable = 1;
    #else
    cfm->lzma_enable = 0;
    #endif

    if (hostif_filter_type_get() == VNET_FILTER_DIRECT)
        cfm->filter_mode = VNET_FILTER_DIRECT;
    else
        cfm->filter_mode = VNET_FILTER_SHARED;

    #if PLF_AIC8800
    cfm->aicplatform = AIC8800M;
    #elif PLF_AIC8800MC
    cfm->aicplatform = AIC8800MC;
    #elif PLF_AIC8800M40
    cfm->aicplatform = AIC8800M40;
    #endif

    #if PLF_OTA
    cfm->upgrade_addr = OTA_ADDR;
    #else
    cfm->upgrade_addr = 0x0;
    #endif

    dbg("[vnet-mode]\n"
        " - keep_alive:   %d\n"
        " - ble_enable:   %d\n"
        " - lzma_enable:  %d\n"
        " - filter_mode:  %d\n"
        " - aicplatform:  %d\n"
        " - upgrade_addr: %08x\n\n",
        cfm->keep_alive, cfm->ble_enable, cfm->lzma_enable, cfm->filter_mode, cfm->aicplatform, cfm->upgrade_addr);

    e2a_msg_send(host_type, cfm);
    return KE_MSG_CONSUMED;
}

#ifdef CFG_SOFTAP
static void wlan_softap_event_handler(wlan_softap_event_e event, uint8_t *device_mac)
{
    uint8_t *sta_mac = (uint8_t *)device_mac;
    hostif_status_e hostif_st = get_hostif_wlan_status();
    if(HOSTIF_ST_INIT == hostif_st || HOSTIF_ST_DEEPSLEEP == hostif_st) {
        dbg("HOSTIF_ST: not valid\n");
        return ;
    }

    if (event == ASSOC_IND_EVENT) {
        dbg("%02X:%02X:%02X:%02X:%02X:%02X Joined\n",
            sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);
        // Allocate the message response structure
        struct custom_msg_ap_assoc_sta_ind *ind = (struct custom_msg_ap_assoc_sta_ind *)e2a_msg_alloc(
            hostif_type_get(), CUSTOM_MSG_ASSOC_AP_IND, 0, 0, sizeof(struct custom_msg_ap_assoc_sta_ind));
        if (ind == NULL) {
            dbg(D_ERR "msg alloc err\r\n");
            return;
        }
        memcpy(ind->sta_addr, sta_mac, sizeof(ind->sta_addr));
        e2a_msg_send(hostif_type_get(), ind);

    } else if (event == DISASSOC_IND_EVENT) {
        dbg("%02X:%02X:%02X:%02X:%02X:%02X Leaved\n",
            sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);
        // Allocate the message response structure
        struct custom_msg_ap_assoc_sta_ind *ind = (struct custom_msg_ap_assoc_sta_ind *)e2a_msg_alloc(
            hostif_type_get(), CUSTOM_MSG_DISASSOC_AP_IND, 0, 0, sizeof(struct custom_msg_ap_assoc_sta_ind));
        if (ind == NULL) {
            dbg(D_ERR "msg alloc err\r\n");
            return;
        }
        memcpy(ind->sta_addr, sta_mac, sizeof(ind->sta_addr));
        e2a_msg_send(hostif_type_get(), ind);
    } else {
        // Never go to here.
    }
}

#define STRCPY_AND_FORWARD(dst, src) do { \
    strcpy((char*)(dst), (const char *)(src)); \
    (dst) = (char*)(dst) + strlen((src)); \
} while (0)

static char *int_to_str(char *buf, unsigned int a)
{
    unsigned int i;
    int j;

    for (i = 10; a >= i; )
        i = i * 10;
    for (i = i / 10, j = 0; i > 0; ) {
        buf[j] = a / i + '0';
        a = a % i;
        j++;
        i = i / 10;
    }
    return buf + j;
}

static int wifi_process_ap_start(struct wifi_noblock_msg *msg)
{
    int ret = 0;
    uint8_t *pw = msg->pw; //anyka: for null password

    if (msg->ap.akm[0]) {
        static char buf[256];
        char *p = buf;
        char *argv[2] = {"do_ap", buf};
        int freq;
        memset(buf, 0, sizeof(buf));
        if (msg->band) {
            freq = 5000 + 5 * msg->channel;
        } else {
            if ((msg->channel >= 1) && (msg->channel <= 13))
                freq = 2412 + 5 * (msg->channel - 1);
            else
                freq = 2412;
        }
        STRCPY_AND_FORWARD(p, "-s ");
        STRCPY_AND_FORWARD(p, msg->ssid);
        STRCPY_AND_FORWARD(p, " -f ");
        p = int_to_str(p, freq);
        STRCPY_AND_FORWARD(p, " -a ");
        STRCPY_AND_FORWARD(p, msg->ap.akm);
        STRCPY_AND_FORWARD(p, " -k ");
        STRCPY_AND_FORWARD(p, msg->pw);
        if (msg->ap.unicast_cipher[0]) {
            STRCPY_AND_FORWARD(p, " -u ");
            STRCPY_AND_FORWARD(p, msg->ap.unicast_cipher);
        }
        if (msg->ap.group_cipher[0]) {
            STRCPY_AND_FORWARD(p, " -g ");
            STRCPY_AND_FORWARD(p, msg->ap.group_cipher);
        }
        STRCPY_AND_FORWARD(p, " -m 2");
        *p = '\0';
        dbg("do_ap %s\n", buf);
        ret = do_ap(2, argv);
        goto start_ap_end;
    }

    // set softap event callback
    wlan_softap_event_register(wlan_softap_event_handler);

    if (!msg->pw[0])
        pw = NULL;

    #if (PLF_AIC8800) || (PLF_AIC8800M40)
    if (msg->band) {
        if (msg->channel == 0) {
            set_ap_channel_num(36);  // 5G
        } else {
            set_ap_channel_num(msg->channel);  // 5G
        }
    } else {
        if (msg->channel == 0) {
            set_ap_channel_num(6);   // 2.4G
        } else {
            set_ap_channel_num(msg->channel);   // 2.4G
        }
    }
    ret = wlan_start_ap(msg->band, msg->ssid, pw);
    #else // 8800MC only suppport 2.4G
    // set 2.4G AP channal at 6.
    set_ap_channel_num(6);
    ret = wlan_start_ap(0, msg->ssid, pw);
    #endif

start_ap_end:
    rtos_free(msg->ssid);
    rtos_free(msg->pw);

    if(ret)
        dbg("Start AP fail!\n");
    else
        dbg("Start AP success.\n");

    #ifdef CFG_SOFTAP
    HOST_TYPE_T host_type = hostif_type_get();
    struct custom_msg_ap_status_cfm *cfm2 = (struct custom_msg_ap_status_cfm *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_START_AP_IND, 0, 0, sizeof(struct custom_msg_ap_status_cfm));
    if (cfm2 == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }
    // In SOFTAP mode, ip == gw generally.
    if (ret == 0) {
        cfm2->status = AIC_AP_START;
        cfm2->ip = get_ap_ip_addr();
        cfm2->gw = cfm2->ip;
        cfm2->mask = get_ap_subnet_mask();
    } else {
        cfm2->status = AIC_AP_CLOSE;
    }

    e2a_msg_send(host_type, cfm2);
    #endif /* CFG_SOFTAP */

    return ret;
}
#endif

static int custom_msg_start_ap_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    #ifdef CFG_SOFTAP
    struct wifi_noblock_msg msg;
    struct custom_msg_common *cfm = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_START_AP_CFM, 0, 0, sizeof(struct custom_msg_common));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }
    // Send msg confirm, it means AIC8800x start to execute command.
    e2a_msg_send(host_type, cfm);

    set_hostif_wlan_status(HOSTIF_ST_IDLE);
    dbg("HOSTIF_ST: IDLE\n");

    // If sta is CONN or ap is START, close it first.
    if (WLAN_CONNECTED == wlan_get_connect_status()) {
        msg.id = WIFI_STA_DISCONNECT;
        wifi_msg_write(&msg);
    }
    if (wlan_get_softap_status()) {
        msg.id = WIFI_AP_STOP;
        wifi_msg_write(&msg);
    }

    uint8_t *pssid = rtos_malloc(SSID_LEN);
    if (pssid == NULL) {
        dbg("alloc buff fail.\n");
        return KE_MSG_CONSUMED;
    }
    uint8_t *ppass = rtos_malloc(PASS_LEN);
    if (ppass == NULL) {
        rtos_free(pssid);
        dbg("alloc buff fail.\n");
        return KE_MSG_CONSUMED;
    }

    struct custom_msg_start_ap_req *req = (struct custom_msg_start_ap_req *)param;
    memcpy(pssid, req->ssid, strlen((const char *)req->ssid));
    pssid[strlen((const char *)req->ssid)] = '\0';
    memcpy(ppass, req->pw, strlen((const char *)req->pw));
    ppass[strlen((const char *)req->pw)] = '\0';

    msg.ssid = pssid;
    msg.pw   = ppass;
    msg.band = (uint32_t)req->band;
    msg.id = WIFI_AP_START;
    msg.channel = req->channel;
    strcpy(msg.ap.akm, req->akm);
    strcpy(msg.ap.unicast_cipher, req->unicast_cipher);
    strcpy(msg.ap.group_cipher, req->group_cipher);
    dbg("AP-ssid: %s\n", msg.ssid);
    dbg("AP-pass: %s\n", msg.pw);
    dbg("AP-channel: %d\n", msg.channel);
    dbg("AP-akm: %s\n", msg.ap.akm);
    dbg("AP-unicast_cipher: %s\n", msg.ap.unicast_cipher);
    dbg("AP-group_cipher: %s\n", msg.ap.group_cipher);
    wifi_msg_write(&msg);

    #endif /* CFG_SOFTAP */
    return KE_MSG_CONSUMED;
}

static int custom_msg_change_ap_mode_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    #ifdef CFG_SOFTAP
    if(!wlan_get_softap_status()) {
        dbg("AP is not START, please start AP first\n");
        return KE_MSG_CONSUMED;
    }

    // Allocate the message response structure
    struct custom_msg_common *cfm = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_CHANGE_AP_MODE_CFM, 0, 0, sizeof(struct custom_msg_common));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }

    if (HOSTIF_ST_IDLE == get_hostif_wlan_status()) {
        set_hostif_wlan_status(HOSTIF_ST_AWAKE);
        cfm->status = AIC_AP_MODE_DIRECT;
        dbg("AP_STATUS: AIC_AP_MODE_DIRECT, HOSTIF_ST=AWAKE\n");
    } else if (HOSTIF_ST_AWAKE == get_hostif_wlan_status()) {
        set_hostif_wlan_status(HOSTIF_ST_IDLE);
        cfm->status = AIC_AP_MODE_CONFIG;
        dbg("AP_STATUS: AIC_AP_MODE_CONFIG, HOSTIF_ST=IDLE\n");
    } else {
        // do nothing
    }

    e2a_msg_send(host_type, cfm);
    #endif /* CFG_SOFTAP */
    return KE_MSG_CONSUMED;
}


#ifdef CFG_SOFTAP
static int wifi_process_ap_stop(void)
{
    int ret = 0;
    wlan_stop_ap();
    dbg("AP is STOP.\n");

    HOST_TYPE_T host_type = hostif_type_get();
    struct custom_msg_common *ind = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_STOP_AP_IND, 0, 0, sizeof(struct custom_msg_common));
    if (ind == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }
    // Send msg indication, it means AIC8800x finish the command.
    e2a_msg_send(host_type, ind);

    return ret;
}
#endif

static int custom_msg_stop_ap_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    #ifdef CFG_SOFTAP
    struct wifi_noblock_msg msg;
    struct custom_msg_common *cfm = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_STOP_AP_CFM, 0, 0, sizeof(struct custom_msg_common));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }
    // Send msg confirm, it means AIC8800x start to execute command.
    e2a_msg_send(host_type, cfm);

    if (!wlan_get_softap_status()) {
        dbg("AP isn`t START.\n");
        return KE_MSG_CONSUMED;

    } else {
        msg.id = WIFI_AP_STOP;
        wifi_msg_write(&msg);
    }
    #endif /* CFG_SOFTAP */
    return KE_MSG_CONSUMED;
}

static int wifi_process_scanning(void)
{
    int nb_res;
    uint32_t fvif_idx = 0; // note: wlan_start_sta uses idx 0
    HOST_TYPE_T host_type = hostif_type_get();
    struct mac_scan_result result;
    static struct fhost_cntrl_link *fhostif_cntrl_link;

    ipc_host_cntrl_start();
    fhostif_cntrl_link = fhost_cntrl_cfgrwnx_link_open();
    if (fhostif_cntrl_link == NULL) {
        dbg(D_ERR "Failed to open link with control task\n");
        goto exit;
    }
    if (((fvif_idx != 0) || (wlan_get_connect_status() == WLAN_DISCONNECT))
        #ifdef CFG_SOFTAP
        && !wlan_get_softap_status()
        #endif
        )
    {
        // Reset STA interface (this will end previous wpa_supplicant task)
        if (fhost_set_vif_type(fhostif_cntrl_link, fvif_idx, VIF_UNKNOWN, false) ||
            fhost_set_vif_type(fhostif_cntrl_link, fvif_idx, VIF_STA, false)) {
            fhost_cntrl_cfgrwnx_link_close(fhostif_cntrl_link);
            goto exit;
        }
    }

    nb_res = fhost_scan(fhostif_cntrl_link, fvif_idx, NULL);
    dbg("Got %d scan results\n", nb_res);

    struct custom_msg_scan_wifi_result_cfm *cfmp1 = NULL;
    struct custom_msg_scan_wifi_result_cfm *cfm1 =
        (struct custom_msg_scan_wifi_result_cfm *)e2a_msg_alloc(host_type,
        CUSTOM_MSG_SCAN_WIFI_IND, 0, 0, sizeof(struct custom_msg_scan_wifi_result_cfm[32]));
    if (cfm1 == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        fhost_cntrl_cfgrwnx_link_close(fhostif_cntrl_link);
        goto exit;
    }
    cfmp1 = cfm1;

    bool buff_flag = 0;
    struct custom_msg_scan_wifi_result_cfm *cfmp2 = NULL;
    struct custom_msg_scan_wifi_result_cfm *cfm2 = NULL;
    if (nb_res > 32) {
        cfm2 = (struct custom_msg_scan_wifi_result_cfm *)e2a_msg_alloc(host_type,
            CUSTOM_MSG_SCAN_WIFI_IND, 0, 0, sizeof(struct custom_msg_scan_wifi_result_cfm[32]));
        if (cfm2 == NULL) {
            dbg(D_ERR "Secound msg alloc err\r\n");
            buff_flag = 1;
        } else {
            cfmp2 = cfm2;
        }
    } else {
        // nb_res <= 32
        buff_flag = 2;
    }

    nb_res = 0;
    while(fhost_get_scan_results(fhostif_cntrl_link, nb_res++, 1, &result)) {

        if (nb_res <= 32) {
            result.ssid.array[result.ssid.length] = '\0'; // set ssid string ending
            cfmp1->rssi = (int8_t)result.rssi;
            cfmp1->channal = phy_freq_to_channel(result.chan->band, result.chan->freq);
            cfmp1->akm = result.akm;
            cfmp1->scan_num = nb_res;

            memcpy(cfmp1->bssid, (uint8_t *)result.bssid.array, 6);
            if(result.ssid.array[0] != '\0') {
                memcpy(cfmp1->ssid, result.ssid.array, result.ssid.length);
                cfmp1->ssid[result.ssid.length] = '\0';
            }
            else {
                memcpy(cfmp1->ssid, "[Hidden ssid]", sizeof("[Hidden ssid]"));
                cfmp1->ssid[sizeof("[Hidden ssid]")] = '\0';
            }

            dbg("%2d:(%3d dBm) CH=%3d AKM=%3d BSSID=%02x:%02x:%02x:%02x:%02x:%02x SSID=%s\n",
                cfmp1->scan_num, cfmp1->rssi, cfmp1->channal, cfmp1->akm, cfmp1->bssid[0], cfmp1->bssid[1],
                cfmp1->bssid[2], cfmp1->bssid[3], cfmp1->bssid[4], cfmp1->bssid[5], cfmp1->ssid);
            cfmp1++;
        } else {
            // nb_res > 32
            if (!buff_flag) {
                result.ssid.array[result.ssid.length] = '\0'; // set ssid string ending
                cfmp2->rssi = (int8_t)result.rssi;
                cfmp2->channal = phy_freq_to_channel(result.chan->band, result.chan->freq);
                cfmp2->akm = result.akm;
                cfmp2->scan_num = nb_res;
                memcpy(cfmp2->bssid, (uint8_t *)result.bssid.array, 6);
                if(result.ssid.array[0] != '\0') {
                    memcpy(cfmp2->ssid, result.ssid.array, result.ssid.length);
                    cfmp2->ssid[result.ssid.length] = '\0';
                }
                else {
                    memcpy(cfmp2->ssid, "[Hidden ssid]", sizeof("[Hidden ssid]"));
                    cfmp2->ssid[sizeof("[Hidden ssid]")] = '\0';
                }

                dbg("%2d:(%3d dBm) CH=%3d AKM=%3d BSSID=%02x:%02x:%02x:%02x:%02x:%02x SSID=%s\n",
                    cfmp2->scan_num, cfmp2->rssi, cfmp2->channal, cfmp2->akm, cfmp2->bssid[0], cfmp2->bssid[1],
                    cfmp2->bssid[2], cfmp2->bssid[3], cfmp2->bssid[4], cfmp2->bssid[5], cfmp2->ssid);
                cfmp2++;
            } else {
                break;
            }
        }
    }

    if (((fvif_idx != 0) || (wlan_get_connect_status() == WLAN_DISCONNECT))
        #ifdef CFG_SOFTAP
        && !wlan_get_softap_status()
        #endif
    ) {
        fhost_set_vif_type(fhostif_cntrl_link, fvif_idx, VIF_UNKNOWN, false);
    }

    fhost_cntrl_cfgrwnx_link_close(fhostif_cntrl_link);

    e2a_msg_send(host_type, cfm1);
    if (!buff_flag)
        e2a_msg_send(host_type, cfm2);

exit:
    return 0;
}



/**
 * @Brief: support wifi-scan-result maximum -> 64
 */
static int custom_msg_scan_wifi_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    struct wifi_noblock_msg msg;
    struct custom_msg_common *cfm =
        (struct custom_msg_common *)e2a_msg_alloc(host_type,
        CUSTOM_MSG_SCAN_WIFI_CFM, 0, 0, sizeof(struct custom_msg_common));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return KE_MSG_CONSUMED;
    }
    e2a_msg_send(host_type, cfm);

    msg.id = WIFI_SCANNING;
    wifi_msg_write(&msg);

    return KE_MSG_CONSUMED;
}

#if HOST_OTA_REBOOT
#include "pmic_api.h"
TimerHandle_t reboot_timer;
static void reboot_timer_handler(TimerHandle_t xTimer)
{
    int ret = rtos_timer_stop(reboot_timer, 0);
    if (ret) {
        dbg("stop reboot_timer failed: %d\n", ret);
    }
    rtos_timer_delete(reboot_timer, 0);
    pmic_chip_reboot(0xF);
}
#endif

static int custom_msg_host_ota_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    #if (PLF_OTA)
    // Allocate the message response structure
    struct custom_msg_common *cfm = (struct custom_msg_common *)e2a_msg_alloc(host_type,
        CUSTOM_MSG_HOST_OTA_CFM, 0, 0, sizeof(struct custom_msg_common));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }
    struct custom_msg_host_ota_req *req = (struct custom_msg_host_ota_req *)param;

    // recv the file size and erase flash
    if(OTA_STEP_FLASH_ERASE == req->ota_step) {
        uint32_t a4k = 0;
        dbg("OTA-File: file-size = %d Byte\n", req->ota_msg.ota_file_info.file_size);
        a4k = req->ota_msg.ota_file_info.file_size + 0xFFF;
        a4k = a4k & ~0xFFF;
        dbg("OTA-File: erase-len = %d Byte\n", a4k);

        if(flash_erase((void *)UPGRADE_START_ADDR, 0x1000 + a4k)) {
            dbg("OTA-Step0: flash erase fail. Please restart OTA\n");
            cfm->status = OTA_STEP_FLASH_ERASE_ERR;
        } else {
            flash_cache_invalid_range((void *)UPGRADE_START_ADDR, 0x1000 + a4k);
            cfm->status = OTA_STEP_FLASH_ERASE_OK;
        }

    } else if(OTA_STEP_FR_PKG_WRITE == req->ota_step) {
        // recv pkg and write into flash
        if(flash_write((void *)req->ota_msg.ota_pkg_info.flash_addr, req->ota_msg.ota_pkg_info.flash_size, (char *)req->ota_msg.ota_pkg_info.pkg_buf)) {
            dbg("OTA_Step1: write pkg at 0x%x fail. Please restart OTA.\r\n", req->ota_msg.ota_pkg_info.flash_addr);
            cfm->status = OTA_STEP_FR_PKG_WRITE_ERR;
        } else {
            cfm->status = OTA_STEP_FR_PKG_WRITE_OK;
        }

    } else if(OTA_STEP_LT_PKG_WRITE == req->ota_step) {
        // recv last pkg and write into flash
        uint8_t buf[1024] = {0xFF};
        uint32_t num = req->ota_msg.ota_pkg_info.flash_size / 0x100;
        memcpy(buf, req->ota_msg.ota_pkg_info.pkg_buf, req->ota_msg.ota_pkg_info.flash_size);
        if(flash_write((void *)req->ota_msg.ota_pkg_info.flash_addr, 0x100*(num+1), (char *)buf)) {
            dbg("OTA_Step2: write pkg at 0x%x fail. Please restart OTA.\r\n", req->ota_msg.ota_pkg_info.flash_addr);
            cfm->status = OTA_STEP_LT_PKG_WRITE_ERR;
        } else {
            flash_cache_invalid_range((void *)UPGRADE_IMAGE_ADDR,
                            (req->ota_msg.ota_pkg_info.flash_addr - UPGRADE_IMAGE_ADDR) + req->ota_msg.ota_pkg_info.flash_size);
            cfm->status = OTA_STEP_LT_PKG_WRITE_OK;
        }

    }
    #ifndef CFG_LZMA_APP
    else if(OTA_STEP_HEADER_WRITE == req->ota_step) {
        // recv bin-file crc value and write image-header
        critical_section_start();
        uint32_t crc_result = aic_crc32((void *)UPGRADE_IMAGE_ADDR, req->ota_msg.ota_file_info.file_size, ~0UL);
        critical_section_end();
        dbg("OTA-Step3: cal-crc addr  = 0x%x, len = %d\n", UPGRADE_IMAGE_ADDR, req->ota_msg.ota_file_info.file_size);
        dbg("OTA-Step3: recv host-crc = %x, cal-crc = %x\n", req->ota_msg.ota_file_info.file_crc, crc_result);
        if(req->ota_msg.ota_file_info.file_crc != crc_result) {
            dbg("OTA_Step3: bin-file CRC result fail. Please restart OTA.\n");
            cfm->status = OTA_STEP_BIN_FILE_CRC_ERR;
        } else {
            struct image_header hdr;
            hdr.magic = IMAGE_HEADER_MAGIC;
            hdr.size  = req->ota_msg.ota_file_info.file_size;
            hdr.crc32 = crc_result;
            hdr.addr  = UPGRADE_IMAGE_ADDR;

            // Use OTA-Tool to build OTA.bin, its version is "v0.1.0".
            // Updated version is necessary and newer (than "v0.1.0") when build newer bin!
            char *version = "v0.1.1";  // strlen(version) <= VER_BYTE_CNT
            struct image_info *curr_info = (void *)CURRENT_INFO_ADDR;
            dbg("OTA_Step3: new-version is %s, curr-version is %s\n", version, curr_info->version);
            if (strcmp((const char *)curr_info->version, version) >= 0) {
                dbg("OTA_Step3: upgrade-image is not newer.\n");
                cfm->status = OTA_STEP_BIN_VERSION_ERR;
            } else {
                memset(hdr.version, 0, VER_BYTE_CNT);
                memcpy(hdr.version, version, strlen(version));
                hdr.version[strlen(version)] = '\0';
                if(flash_write((void *)UPGRADE_START_ADDR, 0x100, (char *)(&hdr))) {
                    dbg("OTA_Step3: image-header flash write fail. Please restart OTA.\n");
                    cfm->status = OTA_STEP_HEADER_WRITE_ERR;
                } else {
                    flash_cache_invalid_range((void *)UPGRADE_START_ADDR, 0x100);
                    dbg("OTA finish...\n");
                    cfm->status = OTA_STEP_HEADER_WRITE_OK;
                }
            }
        }
    }
    #else
    else if(OTA_STEP_LZMA_CHECK == req->ota_step) {
        // recv bin-file crc value and check
        critical_section_start();
        uint32_t crc_result = aic_crc32((void *)UPGRADE_START_ADDR, req->ota_msg.ota_file_info.file_size, ~0UL);
        critical_section_end();
        dbg("OTA-Step3: cal-crc addr  = 0x%x, len = %d\n", UPGRADE_START_ADDR, req->ota_msg.ota_file_info.file_size);
        dbg("OTA-Step3: recv host-crc = %x, cal-crc = %x\n", req->ota_msg.ota_file_info.file_crc, crc_result);
        if(req->ota_msg.ota_file_info.file_crc != crc_result) {
            dbg("OTA_Step3: bin-file CRC result fail. Please restart OTA.\n");
            cfm->status = OTA_STEP_LZMA_CRC_CHECK_ERR;
        } else {

            // Check whether upg-version is newer than curr-version.
            struct image_info *curr_info = (void *)CURRENT_INFO_ADDR;
            struct image_header *upg_header = (void *)UPGRADE_START_ADDR;
            if (strcmp((const char *)curr_info->version, (const char *)upg_header->version) >= 0) {
                dbg("OTA_Step3: upgrade-image is not newer.\n");
                cfm->status = OTA_STEP_BIN_VERSION_ERR;
                // erease upg_image header
                if (flash_erase((void *)UPGRADE_START_ADDR, 0x1000)) {
                    dbg("OTA_Step3: erease upg_image header fail.\n");
                }
            } else {
                dbg("OTA_Step3: LZMA OTA finish...\n");
                cfm->status = OTA_STEP_LZMA_CRC_CHECK_OK;
            }
        }
    }
    #endif /* CFG_LZMA_APP */
    else {
        // do nothing
    }

    e2a_msg_send(host_type, cfm);
    #endif /* PLF_OTA */

    #if HOST_OTA_REBOOT
    int ret = rtos_timer_start(reboot_timer, 1000, false);
    if (ret)
        dbg("start reboot_timer failed: %d\n", ret);
    #endif

    return KE_MSG_CONSUMED;
}

/**
 * @Brief: custom_msg_host_keep_alive_handler
 *   - If enable, linux-host will send keep-alive msg periodically after loading netdrv.
 *   - If keep-alive msg interrupts, it means that linux-host may be close or SDIO/USB
 *   - bus may be cut accidentally.
 */
static int custom_msg_host_keep_alive_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    #if HOSTIF_KEEP_ALIVE
    keep_alive.alive_timestamp = rtos_now(false);
    //dbg("alive_timestamp: %d\n", keep_alive.alive_timestamp);

    // Allocate the message response structure
    struct custom_msg_common *cfm = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_HOST_KEEP_ALIVE_CFM, 0, 0, sizeof(struct custom_msg_common));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }
    e2a_msg_send(host_type, cfm);
    #endif /* HOSTIF_KEEP_ALIVE */
    return KE_MSG_CONSUMED;
}

/**
 * @Brief: custom_msg_host_stop_keep_alive_handler
 */
static int custom_msg_host_stop_keep_alive_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    #if HOSTIF_KEEP_ALIVE
    dbg("%s\n", __func__);
    // close keep_alive timer
    if (keep_alive.keep_alive_timer != NULL) {
        int ret = rtos_timer_stop(keep_alive.keep_alive_timer, 0);
        if (ret) {
            dbg("stop keep alive timer failed: %d\n", ret);
        }
        ret = rtos_timer_delete(keep_alive.keep_alive_timer, 0);
        if (ret) {
            dbg("delete keep alive timer failed: %d\n", ret);
        }
        keep_alive.keep_alive_timer = NULL;
    }
    keep_alive.first_timer_check = true;
    keep_alive.driver_load_check = false;
    #endif /* HOSTIF_KEEP_ALIVE */
    return KE_MSG_CONSUMED;
}

/**
 * @Brief: custom_msg_host_set_wlan_ip_handler
 *   - If enable, aic8800x will not process DHCP, replace by linux-host. First, aic8800x
 *   - will send fake ip and gw to linux-host after connecting to AP. DHCP pkt will transfer
 *   - to linux-host. Then, if linux-host get ip and gw from DHCPs, linux-host should send
 *   - a msg about CUSTOM_MSG_HOST_SET_WLAN_IP_REQ to aic8800x for reset its ip and gw.
 */
static int custom_msg_host_set_wlan_ip_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    #if HOST_CNTRL_DHCP
    struct custom_msg_host_set_wlan_ip *req = (struct custom_msg_host_set_wlan_ip *)param;
    fixed_ip = inet_addr((const char *)req->ip);
    fixed_gw = inet_addr((const char *)req->gw);
    net_if_t *net_if = net_if_find_from_wifi_idx(0);
    net_if_set_ip(net_if, fixed_ip, fixed_mask, fixed_gw);
    dbg("[Get wlan] ip: %s(%08x), gw: %s(%08x)\n", req->ip, fixed_ip, req->gw, fixed_gw);

    // Allocate the message response structure
    struct custom_msg_common *cfm = (struct custom_msg_common *)e2a_msg_alloc(
        host_type, CUSTOM_MSG_HOST_SET_WLAN_IP_CFM, 0, 0, sizeof(struct custom_msg_common));
    if (cfm == NULL) {
        dbg(D_ERR "msg alloc err\r\n");
        return -1;
    }
    e2a_msg_send(host_type, cfm);
    #endif /* HOST_CNTRL_DHCP */
    return KE_MSG_CONSUMED;
}

static int custom_msg_soc_power_off_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    dbg("%s\r\n", __func__);
    user_soc_power_off();
    return KE_MSG_CONSUMED;
}

static int custom_msg_blewifi_cfg_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    dbg("%s\r\n", __func__);
    do_blewifi_config(NULL, NULL);
    return KE_MSG_CONSUMED;
}

//extern uint8_t *sock_target_host;
//extern uint16_t sock_target_port;
//extern uint8_t *sock_keepalive_buffer;
//extern unsigned int sock_keepalive_interval_ms;

void sock_target_host_set(char *s);
char *sock_target_host_get(void);
void sock_target_port_set(uint16_t p);
uint16_t sock_target_port_get(void);
void sock_keepalive_buffer_set(char *s);
char *sock_keepalive_buffer_get(void);
void sock_keepalive_interval_ms_set(uint32_t ms);
uint32_t sock_keepalive_interval_ms_get(void);

static int custom_msg_net_wakeup_cfg_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    struct custom_msg_net_wakeup_cfg *req = param;
    dbg("%s ip %s port %d keepalive_interval_ms %d keepalive_msg %s\r\n",
            __func__, req->server_ip, req->server_port, req->keepalive_interval_ms, req->keepalive_msg);
    sock_target_host_set(req->server_ip);
    sock_target_port_set(req->server_port);
    sock_keepalive_buffer_set(req->keepalive_msg);
    sock_keepalive_interval_ms_set(req->keepalive_interval_ms);
    return KE_MSG_CONSUMED;
}

static co_timer *co_hostif_reset_timer = NULL;

static void hostif_reset_timer_handler(void *cb_param)
{
    host_if_poweroff();
    host_if_repower();
}

static int custom_msg_hostif_reset_handler(uint16_t const host_type,
                         void *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    dbg("%s\r\n", __func__);
    co_timer_start(&co_hostif_reset_timer, 800, NULL, hostif_reset_timer_handler, 0);
    return KE_MSG_CONSUMED;
}

/*
 * @Brief: fhostif_get_driver_status
 *   - get host driver status for internal callback.
 */
host_driver_t fhostif_get_driver_status(void)
{
    return driver_st;
}

/*
 * Entry point of wifi noblock task
 */
static RTOS_TASK_FCT(wifi_noblock_task)
{
    struct wifi_noblock_msg msg;

    for (;;) {
        rtos_queue_read(wifi_queue, &msg, -1, false);

        switch (msg.id) {
        case WIFI_STA_CONNECT:
            wifi_process_sta_connect(&msg);
            break;
        case WIFI_STA_DISCONNECT:
            wifi_process_sta_disconnect();
            break;
        #ifdef CFG_SOFTAP
        case WIFI_AP_START:
            wifi_process_ap_start(&msg);
            break;
        case WIFI_AP_STOP:
            wifi_process_ap_stop();
            break;
        #endif
        case WIFI_SCANNING:
            wifi_process_scanning();
            break;
        default:
            break;
        }
    }

    rtos_task_delete(NULL);
}

static RTOS_TASK_FCT(reconnect_wifi_task)
{
    int ret;
    for (;;) {
        if (get_hostif_wlan_status() != HOSTIF_ST_DEEPSLEEP) {
            if (prev_connect_status == WLAN_CONNECTED) {
                ret = wlan_get_connect_status();
                if (ret == WLAN_DISCONNECT) {
                    // reconnect it
                    ret = wlan_start_sta(prev_connect_ssid, prev_connect_pw,
                            WLAN_CONNECT_TO_CFG_2_PARAM(WLAN_CONNECT_CFG_DISABLE_AUTO_RECONN, STA_CONNECT_TIMEOUT));
                    if (!ret) {
                        dbg("reconnect ok!\n");
                    } else {
                        dbg("reconnect failed %d\n", ret);
                    }
                }
            }
        }
        rtos_task_suspend(1 * 1000);
    }
    rtos_task_delete(NULL);
}

#if NX_BEACONING
static struct fhost_cntrl_link *ap_link;
#include "dhcps.h"
#endif

static void fhost_nw_upper(char *str, char *stop)
{
    char *ptr = str;
    char c;

    if (stop)
    {
        c = *stop;
        *stop = 0;
    }
    while (*ptr)
    {
        if ((*ptr >= 'a') && (*ptr <= 'z'))
            *ptr -= 'a' - 'A';
        ptr++;
    }

    if (stop)
        *stop = c;
}

/**
 ****************************************************************************************
 * @brief Process function for 'ap' command
 *
 * Start an AP
 * @verbatim
   ap [-i <itf>] -s <SSID> -f <freq>[+-@] [-a <akm>[,<akm 2>]] [-k <key>]
      [-b bcn_int[,dtim_period]] [-u <unicast cipher>[,<unicast cipher 2>]]
      [-g <group cipher>] [-m <mfp: 0|1|2>]
   @endverbatim
 * The '+/-' after the frequency allow to configure a 40MHz channel with the secondary
 * channel being the upper/lower one. The '@' allow to configure a 80 MHz channel, this
 * is only allowed for valid primary channel and center freq is automatically computed.
 *
 * @param[in] params  Connection parameters
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
int do_ap(int argc, char * const argv[])
{
#if NX_BEACONING
    static char ap_params[256] = {0};
    unsigned int idx = 0, j = 0;
    struct fhost_vif_ap_cfg cfg;
    int fhost_vif_idx = 0;//fhost_search_first_valid_itf();

    if ((argc < 1)) {
        dbg("Usage:\n  ap \n");
        return -1;
    }

    memset(ap_params, 0, sizeof(ap_params));
    j = 2;
    while(j <= argc) {
        memcpy(&(ap_params[idx]), argv[j - 1], strlen(argv[j - 1]));
        idx += strlen(argv[j - 1]);
        ap_params[idx] = ' ';
        idx ++;
        j++;
    }
    dbg("ap_params: %s\n", ap_params);

    char *token, *next = ap_params;
    memset(&cfg, 0, sizeof(cfg));

    while ((token = fhost_nw_next_token(&next)))
    {
        char option;

        if ((token[0] != '-') | (token[2] != '\0'))
            return -1;

        option = token[1];
        token = fhost_nw_next_token(&next);
        if (!token)
            return -1;

        switch(option)
        {
            #if 0
            case 'i':
            {
                fhost_vif_idx = fhost_search_itf(token);
                if (fhost_vif_idx < 0)
                    return ERR_CMD_FAIL;
                break;
            }
            #endif
            case 's':
            {
                size_t ssid_len = strlen(token);
                if (ssid_len > sizeof(cfg.ssid.array))
                {
                    dbg("Invalid SSID\r\n");
                    return -1;
                }

                memcpy(cfg.ssid.array, token, ssid_len);
                cfg.ssid.length = ssid_len;
                break;
            }
            case 'k':
            {
                size_t key_len = strlen(token);
                if ((key_len + 1) > sizeof(cfg.key))
                {
                    dbg("Invalid Key\r\n");
                    return -1;
                }
                strcpy(cfg.key, token);
                break;
            }
            case 'f':
            {
                int len = strlen(token) - 1;
                struct mac_chan_def *chan = NULL;
                int offset = 0;
                if (token[len] == '+')
                {
                    token[len] = 0;
                    offset = 10;
                    cfg.chan.type = PHY_CHNL_BW_40;
                }
                else if (token[len] == '-')
                {
                    token[len] = 0;
                    offset = -10;
                    cfg.chan.type = PHY_CHNL_BW_40;
                }
                else if (token[len] == '@')
                {
                    token[len] = 0;
                    cfg.chan.type = PHY_CHNL_BW_80;
                }
                else
                {
                    cfg.chan.type = PHY_CHNL_BW_20;
                }

                cfg.chan.prim20_freq = atoi(token);
                chan = fhost_chan_get(cfg.chan.prim20_freq);
                if (!chan)
                {
                    dbg("Invalid channel\n");
                    return -1;
                }

                if (cfg.chan.prim20_freq >= PHY_FREQ_5G)
                    cfg.chan.band = PHY_BAND_5G;
                else
                    cfg.chan.band = PHY_BAND_2G4;

                if (cfg.chan.type == PHY_CHNL_BW_80)
                {
                    if ((cfg.chan.prim20_freq < 5180) ||
                        (cfg.chan.prim20_freq > 5805))
                    {
                        dbg("Invalid primary for 80MHz channel\n");
                        return -1;
                    }
                    offset = (cfg.chan.prim20_freq - 5180) % 80;
                    if (offset < 20)
                        offset = 30;
                    else if (offset < 40)
                        offset = 10;
                    else if (offset < 60)
                        offset = -10;
                    else
                        offset = -30;
                }
                cfg.chan.center1_freq = cfg.chan.prim20_freq + offset;
                break;
            }
            case 'a':
            {
                char *next_akm;
                fhost_nw_upper(token, NULL);
                next_akm = strchr(token, ',');
                while (token)
                {
                    if (strncmp(token, "OPEN", 4) == 0)
                    {
                        cfg.akm |= CO_BIT(MAC_AKM_NONE);
                    }
                    else if (strncmp(token, "WEP", 4) == 0)
                    {
                        cfg.akm |= CO_BIT(MAC_AKM_PRE_RSN);
                    }
                    else if (strncmp(token, "WPA", 3) == 0)
                    {
                        cfg.akm |= CO_BIT(MAC_AKM_PRE_RSN) | CO_BIT(MAC_AKM_PSK);
                    }
                    else if (strncmp(token, "RSN", 3) == 0)
                    {
                        cfg.akm |= CO_BIT(MAC_AKM_PSK);
                    }
                    else if (strncmp(token, "SAE", 3) == 0)
                    {
                        cfg.akm |= CO_BIT(MAC_AKM_SAE);
                    }
                    else
                    {
                        dbg("The following AKM are supported [%s]:\n"
                                    "OPEN: For open AP\n"
                                    "WEP: For AP with WEP security\n"
                                    "WPA: For AP with WPA/PSK security (pre WPA2)\n"
                                    "RSN: For AP with WPA2/PSK security\n"
                                    "SAE: For AP with WPA3/PSK security\n", token);
                        if (strncmp(token, "HELP", 4) == 0)
                            return 0;
                        else
                            return -1;
                    }

                    token = next_akm;
                    if (token)
                    {
                        token++;
                        next_akm = strchr(token, ',');
                    }
                }
                break;
            }
            case 'u':
            case 'g':
            {
                char *next_cipher;
                uint32_t cipher = 0;

                fhost_nw_upper(token, NULL);
                next_cipher = strchr(token, ',');
                while (token)
                {
                    if (strncmp(token, "CCMP", 4) == 0)
                    {
                        cipher |= CO_BIT(MAC_CIPHER_CCMP);
                    }
                    else if (strncmp(token, "TKIP", 4) == 0)
                    {
                        cipher |= CO_BIT(MAC_CIPHER_TKIP);
                    }
                    else if (strncmp(token, "WEP40", 5) == 0)
                    {
                        cipher |= CO_BIT(MAC_CIPHER_WEP40);
                    }
                    else if (strncmp(token, "WEP104", 6) == 0)
                    {
                        cipher |= CO_BIT(MAC_CIPHER_WEP104);
                    }
                    else if (strncmp(token, "SMS4", 4) == 0)
                    {
                        cipher |= CO_BIT(MAC_CIPHER_WPI_SMS4);
                    }
                    else
                    {
                        dbg("The following cipher are supported [%s]:\n"
                                    "CCMP, TKIP, WEP40, WEP104, SMS4", token);
                        if (strncmp(token, "HELP", 4) == 0)
                            return 0;
                        else
                            return -1;
                    }

                    token = next_cipher;
                    if (token)
                    {
                        token++;
                        next_cipher = strchr(token, ',');
                    }
                }

                if (option == 'u')
                    cfg.unicast_cipher = cipher;
                else
                    cfg.group_cipher = cipher;

                break;
            }
            case 'b':
            {
                char *dtim = strchr(token, ',');
                if (dtim)
                {
                    *dtim++ = 0;
                    cfg.dtim_period = atoi(dtim);
                }
                cfg.bcn_interval = atoi(token);

                break;
            }
            case 'm':
            {
                cfg.mfp = atoi(token);
                break;
            }
            case 'h':
            {
                cfg.hidden_ssid = atoi(token);
                break;
            }
            default:
            {
                dbg("Invalid option %c\n", option);
                return -1;
            }
        }
    }

    if (fhost_vif_idx < 0)
        return -1;

    if ((cfg.ssid.length == 0) || (cfg.chan.prim20_freq == 0))
        return -1;

    // try to select the best AKM if not set
    if (cfg.akm == 0)
    {
        if (strlen(cfg.key) == 0)
            cfg.akm = CO_BIT(MAC_AKM_NONE);
        else if (strlen(cfg.key) == 5)
            cfg.akm = CO_BIT(MAC_AKM_PRE_RSN);
        else
            cfg.akm = CO_BIT(MAC_AKM_PSK);
    }
    cfg.enable_he = (uint8_t)fhost_config_value_get(FHOST_CFG_HE);
    ipc_host_cntrl_start();

#if 0
    {
        dbg("ssid: len %d array %s\n", cfg.ssid.length, cfg.ssid.array);
        dbg("chan: band %d type %d prim20_freq %d center1_freq %d center2_freq %d tx_power %d flags 0x%x\n",
                cfg.chan.band, cfg.chan.type, cfg.chan.prim20_freq, cfg.chan.center1_freq,
                cfg.chan.center2_freq, cfg.chan.tx_power, cfg.chan.flags);
        dbg("bcn %d dtim %d\n", cfg.bcn_interval, cfg.dtim_period);
        dbg("akm 0x%x unicast_cipher 0x%x group_cipher 0x%x\n", cfg.akm, cfg.unicast_cipher, cfg.group_cipher);
        dbg("key %s\n", cfg.key);
        dbg("mfp %d hidden_ssid %d enable_he %d  enable_acs %d\n", cfg.mfp, cfg.hidden_ssid, cfg.enable_he, cfg.enable_acs);
    }
#endif

    struct fhost_vif_tag *fhost_vif;

    ap_link = fhost_cntrl_cfgrwnx_link_open();
    if (ap_link == NULL) {
        dbg(D_ERR "Failed to open link with control task\n");
        ASSERT_ERR(0);
    }

    // (Re)Set interface type to AP
    if (fhost_set_vif_type(ap_link, fhost_vif_idx, VIF_UNKNOWN, false) ||
        fhost_set_vif_type(ap_link, fhost_vif_idx, VIF_AP, false))
        return -1;

    fhost_cntrl_cfgrwnx_link_close(ap_link);

    fhost_vif = &fhost_env.vif[fhost_vif_idx];
    MAC_ADDR_CPY(&(vif_info_tab[fhost_vif_idx].mac_addr), &(fhost_vif->mac_addr));

    if (fhost_ap_cfg(fhost_vif_idx, &cfg))
    {
        dbg("Failed to start AP, check your configuration");
        return -1;
    }

    net_if_t *net_if = fhost_to_net_if(fhost_vif_idx);
    if (net_if == NULL) {
        dbg("[AIC] net_if_find_from_wifi_idx fail\r\n");
        return 1;
    }
    uint32_t ip_mask = 0x00FFFFFF;
    uint32_t ip_addr = get_ap_ip_addr();
    net_if_set_ip(net_if, ip_addr, ip_mask, 0);

    //set up DHCP server
    dhcpServerStart(net_if);

    // Now that we got an IP address use this interface as default
    net_if_set_default(net_if);

    dbg("DHCPS init: ip=%d.%d.%d.%d\r\n",
          (ip_addr)&0xFF, (ip_addr>>8)&0xFF, (ip_addr>>16)&0xFF, (ip_addr>>24)&0xFF);

#endif // NX_BEACONING


    return 0;
}

/*
 * TASK DESCRIPTOR DEFINITIONS
 ****************************************************************************************
 */

///  custom msg handler
const struct ke_msg_handler custom_msg_state[] = {
    {CUSTOM_MSG_CONNECT_REQ,          (ke_msg_func_t)custom_msg_connect_handler},
    {CUSTOM_MSG_DISCONNECT_REQ,       (ke_msg_func_t)custom_msg_disconnect_handler},
    {CUSTOM_MSG_OPEN_BLE_REQ,         (ke_msg_func_t)custom_msg_open_ble_handler},
    {CUSTOM_MSG_CLOSE_BLE_REQ,        (ke_msg_func_t)custom_msg_close_ble_handler},
    {CUSTOM_MSG_ENTER_SLEEP_REQ,      (ke_msg_func_t)custom_msg_enter_sleep_handler},
    {CUSTOM_MSG_EXIT_SLEEP_REQ,       (ke_msg_func_t)custom_msg_exit_sleep_handler},
    {CUSTOM_MSG_SET_MAC_ADDR_REQ,     (ke_msg_func_t)custom_msg_set_mac_addr_handler},
    {CUSTOM_MSG_GET_MAC_ADDR_REQ,     (ke_msg_func_t)custom_msg_get_mac_addr_handler},
    {CUSTOM_MSG_GET_WLAN_STATUS_REQ,  (ke_msg_func_t)custom_msg_get_wlan_status_handler},
    {CUSTOM_MSG_GET_VNET_SETTING_REQ, (ke_msg_func_t)custom_msg_get_vnet_setting_handler},
    {CUSTOM_MSG_START_AP_REQ,         (ke_msg_func_t)custom_msg_start_ap_handler},
    {CUSTOM_MSG_CHANGE_AP_MODE_REQ,   (ke_msg_func_t)custom_msg_change_ap_mode_handler},
    {CUSTOM_MSG_STOP_AP_REQ,          (ke_msg_func_t)custom_msg_stop_ap_handler},
    {CUSTOM_MSG_SCAN_WIFI_REQ,        (ke_msg_func_t)custom_msg_scan_wifi_handler},
    {CUSTOM_MSG_HOST_OTA_REQ,         (ke_msg_func_t)custom_msg_host_ota_handler},
    {CUSTOM_MSG_HOST_KEEP_ALIVE_REQ,  (ke_msg_func_t)custom_msg_host_keep_alive_handler},
    {CUSTOM_MSG_HOST_SET_WLAN_IP_REQ, (ke_msg_func_t)custom_msg_host_set_wlan_ip_handler},
    {CUSTOM_MSG_SOC_POWER_OFF_REQ, (ke_msg_func_t)custom_msg_soc_power_off_handler},
    {CUSTOM_MSG_BLEWIFI_CFG_REQ, (ke_msg_func_t)custom_msg_blewifi_cfg_handler},
    {CUSTOM_MSG_NET_WAKEUP_CFG_REQ, (ke_msg_func_t)custom_msg_net_wakeup_cfg_handler},
    {CUSTOM_MSG_HOSTIF_RESET_REQ, (ke_msg_func_t)custom_msg_hostif_reset_handler},
    {CUSTOM_MSG_HOST_STOP_KEEP_ALIVE_REQ, (ke_msg_func_t)custom_msg_host_stop_keep_alive_handler},
};

/// Specifies the message handlers that are common to all states.
const struct ke_state_handler custom_msg_handler = {
    custom_msg_state, sizeof(custom_msg_state) / sizeof(struct ke_msg_handler)
};

/**
 * @brief: hostif application init, set recv pkg filter.
 *   - Filter Type:        VNET_FILTER_DIRECT and VNET_FILTER_SHARED
 *   - VNET_FILTER_DIRECT: filter protocol and dst_port only
 *   - VNET_FILTER_SHARED: support combination type as following,
 *                         1. src_ipaddr
 *                         2. src_ipaddr and src_port
 *                         3. protocol and src_port
 *                         4. protocol and dst_port
 *                         5. src_ipaddr, protocol and src_port
 *                         6. src_port(use PACKET_FILTER_MASK_SHARE_DNS_PORT)
 *                            This can share DNS pkt both HOST and MCU
 *   - VNET_FILTER_SHARED: Besides, both host and mcu can ping outside ip.
 *                         But only mcu will echo, when get ping request.
 */
int hostif_application_init(void)
{
    int ret = 0;
    ip_packet_filter_t filter;
    init_hostif_user_filter();

    if (hostif_filter_type_get() == VNET_FILTER_DIRECT) {
        // set ip pkt filter
        #if !(HOST_CNTRL_DHCP)
        filter.used = 1;
        filter.protocol = 17;  // UDP
        filter.dst_port = 68;  // DHCP client
        ret = set_hostif_user_filter(&filter);
        if (ret) {
            dbg("failed to set ip pkt filter\n");
        }
        #endif

        #ifdef CFG_SOFTAP
        filter.used = 1;
        filter.protocol = 17;  // UDP
        filter.dst_port = 67;  // DHCP server
        ret = set_hostif_user_filter(&filter);
        if (ret) {
            dbg("failed to set ip pkt filter\n");
        }
        #endif
    } else if (hostif_filter_type_get() == VNET_FILTER_SHARED) {
        // set ip pkt filter
        filter.used = 1;
        filter.protocol = 17;  // UDP
        filter.dst_port = 68;  // DHCP client
        filter.filter_mask = PACKET_FILTER_MASK_PROTOCOL | PACKET_FILTER_MASK_DST_PORT;
        ret = set_hostif_user_filter(&filter);
        if (ret) {
            dbg("failed to set ip pkt filter\n");
        }
        #ifdef CFG_SOFTAP
        filter.used = 1;
        filter.protocol = 17;  // UDP
        filter.dst_port = 67;  // DHCP server
        filter.filter_mask = PACKET_FILTER_MASK_PROTOCOL | PACKET_FILTER_MASK_DST_PORT;
        ret = set_hostif_user_filter(&filter);
        if (ret) {
            dbg("failed to set ip pkt filter\n");
        }
        #endif

        // If you need to filter src ipaddr, add this.
        #if 0
        uint32_t ipaddr = inet_addr("192.168.124.2");
        if (ipaddr == IPADDR_NONE) {
            dbg("Get ip addr fail\n");
            return 1;
        }
        dbg("u32 ip: %x\n", ipaddr);
        filter.used = 1;
        filter.src_ipaddr = ipaddr;
        filter.filter_mask = PACKET_FILTER_MASK_SRC_IP;
        ret = set_hostif_user_filter(&filter);
        if (ret) {
            dbg("failed to set ip pkt filter\n");
        }
        #endif
    }

    #if HOSTIF_KEEP_ALIVE
    keep_alive_init();
    #endif

    #ifdef CFG_FHOSTIF_IPC_FC
    // If need to change ipc flowctrl params, change them before timer start.
    // Change here! Todo...
    fhost_ipc_fc_timer_init();
    #endif

    #if HOST_OTA_REBOOT
    reboot_timer = rtos_timer_create("reboot_timer", (uint32_t)2000, pdFALSE, NULL, reboot_timer_handler);
    if (reboot_timer == NULL)
        dbg("create reboot_timer failed.\n");
    #endif

    // The wifi_queue length is 8 default.
    if (rtos_queue_create(sizeof(struct wifi_noblock_msg), 8, &wifi_queue)) {
        return 1;
    }
    if (rtos_task_create(wifi_noblock_task, "noblock wifi", NOBLOCK_WIFI_TASK,
                         512, NULL, RTOS_TASK_PRIORITY(1), NULL))
        return 2;

    if (rtos_queue_create(sizeof(uint32_t), 6, &sleep_irq_evt_queue)) {
        return 3;
    }
    if (rtos_task_create(sleep_irq_task, "SLEEP IRQ task", SLEEP_IRQ_TASK,
                         256, NULL, RTOS_TASK_PRIORITY(4), NULL)) {
        return 4;
    }
    if (rtos_task_create(reconnect_wifi_task, "reconnect wifi", RECONNECT_WIFI_TASK,
                512, NULL, RTOS_TASK_PRIORITY(4), NULL))
        return 5;

    return ret;
}

#endif /* CONFIG_RWNX_LWIP  && CFG_HOSTIF*/

