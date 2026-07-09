/* create @20200301 by aic*/

#ifndef WB_COEX_H
#define WB_COEX_H

#include <stdint.h>

#define WB_COEX_COANT_ON_FLAG   (0x01UL << 0)

struct coex_controller {
    uint8_t wifi_on;
    uint8_t bt_on;
    uint8_t wifi_connected;
    uint8_t bt_connected;
    uint8_t wifi_dhcp;
    uint8_t a2dp_on;//ftp,opp/hid
    uint8_t sco_on;
    uint8_t wifi_deepsleep;
    uint32_t wb_flags;
};

void wb_coex_check_status(void);

int wb_coex_wifi_on_get(void);

void wb_coex_wifi_on_set(int on);

int wb_coex_bt_on_get(void);

void wb_coex_bt_on_set(int on);

int wb_coex_wifi_connected_get(void);

void wb_coex_wifi_connected_set(int connected);

int wb_coex_bt_connected_get(void);

void wb_coex_bt_connected_set(int connected);

int wb_coex_bt_a2dp_on_get(void);

void wb_coex_bt_a2dp_on_set(int on);

int wb_coex_bt_sco_on_get(void);

void wb_coex_bt_sco_on_set(int on);

int wb_coex_coant_on_get(void);

void wb_coex_coant_on_set(int on);

#endif
