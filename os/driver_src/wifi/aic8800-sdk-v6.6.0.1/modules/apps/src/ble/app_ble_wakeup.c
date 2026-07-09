#include "tgt_cfg_bt.h"
#if PLF_BT_STACK == 1
#if PLF_BLE_ONLY == 1 && PLF_BLE_WAKEUP == 1
#include <string.h>
#include "rtos.h"
#include "ble_task_msg.h"
#include "app_ble_only.h"
#include "co_main.h"
#include "app_ble_queue.h"
#include "gpio_api.h"
#include "pmic_api.h"
#include "ble_task_msg.h"
#include "app_ble_wakeup.h"

#define BLE_WAKEUP_GPIO                 2
#define CONFIG_DEFAULT_PULLED_UP        0


ble_wakeup_param_t * ble_wakeup_input_param = (ble_wakeup_param_t *)(BLE_WAKEUP_INPUT_PARAM_ADDR);
ble_wakeup_param_t ble_wakeup_user = {
    .delay_scan_to = 1000,
    .reboot_to = 1000,
};
static co_timer *ble_wakeup_timer = NULL;
uint8_t ble_combo_num[MAX_ROLE_COMBO_IDX_NUM] = {0,};
static co_timer *scan_start_timer = NULL;
static bool trigger_on = false;


bool app_ble_white_list_check(uint8_t idx, bd_addr_t bdaddr)
{
    bool ret = false;
    uint8_t null_addr[6] = {0x00,0x00,0x00,0x00,0x00,0x00};

    if(ble_wakeup_user.ad_filter[idx].wl_addr.addr[0] == null_addr[5]
        && ble_wakeup_user.ad_filter[idx].wl_addr.addr[1] == null_addr[4]
        && ble_wakeup_user.ad_filter[idx].wl_addr.addr[2] == null_addr[3]
        && ble_wakeup_user.ad_filter[idx].wl_addr.addr[3] == null_addr[2]
        && ble_wakeup_user.ad_filter[idx].wl_addr.addr[4] == null_addr[1]
        && ble_wakeup_user.ad_filter[idx].wl_addr.addr[5] == null_addr[0]){
        ret = true;
    }else{
        if(ble_wakeup_user.ad_filter[idx].wl_addr.addr[0] == bdaddr.addr[5]
            && ble_wakeup_user.ad_filter[idx].wl_addr.addr[1] == bdaddr.addr[4]
            && ble_wakeup_user.ad_filter[idx].wl_addr.addr[2] == bdaddr.addr[3]
            && ble_wakeup_user.ad_filter[idx].wl_addr.addr[3] == bdaddr.addr[2]
            && ble_wakeup_user.ad_filter[idx].wl_addr.addr[4] == bdaddr.addr[1]
            && ble_wakeup_user.ad_filter[idx].wl_addr.addr[5] == bdaddr.addr[0]){
            ret = true;
        }
    }

    return ret;
}

static void app_ble_scan_start_timer(void *cb_param)
{
    app_ble_scan_msg_start();
    scan_start_timer = NULL;
}

void app_ble_delay_scan_start(void)
{
    uint32_t delay_time = (ble_wakeup_user.delay_scan_to & 0x0000ffff);
    dbg("%s, time %d\n",__func__,delay_time);
    if(scan_start_timer == NULL){
        co_timer_start(&scan_start_timer,delay_time,NULL,app_ble_scan_start_timer,0);
    }
}

static void app_ble_re_trigger_timer(void *cb_param)
{
    trigger_on = false;
    scan_start_timer = NULL;
}

static void app_ble_delay_re_trigger_start(void)
{
    uint32_t delay_time = (ble_wakeup_user.delay_scan_to & 0xffff0000)>>16;
    dbg("%s, time %d\n",__func__,delay_time);
    if(scan_start_timer == NULL){
        co_timer_start(&scan_start_timer,delay_time,NULL,app_ble_re_trigger_timer,0);
    }
}

ble_wakeup_param_t *app_ble_wakeup_get_param(void)
{
    return &ble_wakeup_user;
}

void app_ble_wakeup_notify(uint8_t gpio_num_idx)
{
    CO_MODUAL_EVENT msg;
    int res = 0;
    msg.mod_id = CO_MODUAL_BLE_WAKEUP;
    msg.mod_evt.EvtId = gpio_num_idx;
    res = co_event_send(&msg, false);
    ASSERT_ERR(res == 0 || res == 1);
}

static void app_ble_trigger_gpio_by_idx(uint8_t gpio_num_idx)
{
    if(gpio_num_idx & TG_IDX_0){
        switch(ble_wakeup_input_param->gpio_dft_lvl[0]){
            case 0:
                gpiob_set(ble_wakeup_user.gpio_num[0]); // high after trigger
                dbg("wakeup matched, set gpio %d\r\n",ble_wakeup_user.gpio_num[0]);
                break;
            case 1:
                gpiob_clr(ble_wakeup_user.gpio_num[0]); // high after trigger
                dbg("wakeup matched, clr gpio %d\r\n",ble_wakeup_user.gpio_num[0]);
                break;
            default:
                dbg("gpio_dft_lvl error\n");
                break;
        }
    }
    if(gpio_num_idx & TG_IDX_1){
        switch(ble_wakeup_input_param->gpio_dft_lvl[1]){
            case 0:
                gpiob_set(ble_wakeup_user.gpio_num[1]); // high after trigger
                dbg("wakeup matched, set gpio %d\r\n",ble_wakeup_user.gpio_num[1]);
                break;
            case 1:
                gpiob_clr(ble_wakeup_user.gpio_num[1]); // high after trigger
                dbg("wakeup matched, clr gpio %d\r\n",ble_wakeup_user.gpio_num[1]);
                break;
            default:
                dbg("gpio_dft_lvl error\n");
                break;
        }
    }
}

static void app_ble_clean_gpio_by_idx(uint8_t gpio_num_idx)
{
    if(gpio_num_idx & TG_IDX_0){
        switch(ble_wakeup_user.gpio_dft_lvl[0]){
            case 1:
                gpiob_set(ble_wakeup_user.gpio_num[0]); // high after trigger time down
                dbg("clean matched, set gpio %d\r\n",ble_wakeup_user.gpio_num[0]);
                break;
            case 0:
                gpiob_clr(ble_wakeup_user.gpio_num[0]); // high after trigger time down
                dbg("clean matched, clr gpio %d\r\n",ble_wakeup_user.gpio_num[0]);
                break;
            default:
                dbg("gpio_dft_lvl error\n");
                break;
        }
    }
    if(gpio_num_idx & TG_IDX_1){
        switch(ble_wakeup_user.gpio_dft_lvl[1]){
            case 1:
                gpiob_set(ble_wakeup_user.gpio_num[1]); // high after trigger time down
                dbg("clean matched, set gpio %d\r\n",ble_wakeup_user.gpio_num[1]);
                break;
            case 0:
                gpiob_clr(ble_wakeup_user.gpio_num[1]); // high after trigger time down
                dbg("clean matched, clr gpio %d\r\n",ble_wakeup_user.gpio_num[1]);
                break;
            default:
                dbg("gpio_dft_lvl error\n");
                break;
        }
    }
}

static void app_ble_reboot_timer_handler(void *env)
{
    bool need_reboot = false;
    ble_wakeup_timer = NULL;
    dbg("%s\n", __func__);
    if(ble_wakeup_user.reboot_to&0xC0000000){
        need_reboot = true;
        dbg("need_reboot %d\n", need_reboot);
    }
    if(need_reboot)
        pmic_chip_reboot(100); // reboot
}

static void app_ble_delay_reboot_start(void)
{
    uint32_t delay_time = (ble_wakeup_user.reboot_to&0x3fffffff);
    dbg("%s, time 0x%x\n",__func__,delay_time);
    if (ble_wakeup_timer == NULL) {
        co_timer_start(&ble_wakeup_timer, delay_time, (void *)NULL, app_ble_reboot_timer_handler, 0);
    }
}

static void app_ble_trigger_timer(void *cb_param)
{
    uint32_t gpio = (uint32_t)cb_param;
    uint32_t delay_time = (ble_wakeup_user.delay_scan_to & 0xffff0000)>>16;
    ble_wakeup_timer = NULL;
    dbg("%s, gpio %d\n",__func__,gpio);
    app_ble_clean_gpio_by_idx(gpio);
    if(delay_time){
        app_ble_delay_re_trigger_start();
    }else{
        trigger_on = false;
    }
}

static void app_ble_trigger_timer_start(uint32_t gpio)
{
    uint32_t delay_time = (ble_wakeup_user.reboot_to&0x3fffffff);
    if(delay_time == 0){
        delay_time = 100;
    }
    trigger_on = true;
    if (ble_wakeup_timer == NULL) {
        co_timer_start(&ble_wakeup_timer, delay_time, (void *)gpio, app_ble_trigger_timer, 0);
    }
}

static bool app_ble_wakeup_handle_process(CO_EVENT *evt)
{
    uint32_t reboot_mode = (ble_wakeup_user.reboot_to&0xc0000000)>>30;
    uint8_t gpio_num_idx = (uint8_t)evt->EvtId;
    if(trigger_on){
        return false;
    }
    dbg("%s, gpio_num_idx %d\n",__func__,gpio_num_idx);
    app_ble_trigger_gpio_by_idx(gpio_num_idx);
    dbg("delay %x ms & reboot,reboot_mode %d\r\n", ble_wakeup_user.reboot_to,reboot_mode);

    if(ble_wakeup_user.reboot_to){
        switch(reboot_mode){
            case 0:
            case 1:
                {
                    app_ble_scan_msg_stop();
                    dbg("delay %dms & reboot\r\n", ble_wakeup_user.reboot_to);
                    app_ble_delay_reboot_start();
                }
                break;
            case 2:
                {
                    app_ble_trigger_timer_start(gpio_num_idx);
                }
                break;
            default:
                break;
        }
    }else{
        app_ble_trigger_timer_start(gpio_num_idx);
    }
    return true;
}

void app_ble_wakeup_init(void)
{
    dbg("%s ram_size = %d\n",__func__,sizeof(ble_wakeup_param_t));
    dbg("[%p]=%08X\n",&ble_wakeup_input_param->magic_num,ble_wakeup_input_param->magic_num);
    dbg("[%p]=%08X\n",&ble_wakeup_input_param->delay_scan_to,ble_wakeup_input_param->delay_scan_to);
    dbg("[%p]=%08X\n",&ble_wakeup_input_param->reboot_to,ble_wakeup_input_param->reboot_to);
    for(uint8_t i=0;i<MAX_GPIO_TRIGGER_NUM;i++){
        dbg("gpio_num idx[%d] [%p]=%x\n",i,&ble_wakeup_input_param->gpio_num[i],ble_wakeup_input_param->gpio_num[i]);
        dbg("gpio_dft_lvl idx[%d] [%p]=%x\n",i,&ble_wakeup_input_param->gpio_dft_lvl[i],ble_wakeup_input_param->gpio_dft_lvl[i]);
    }
    for(uint8_t i=0;i<BLE_AD_FILTER_NUM_MAX;i++){
        dbg("ad_data_mask [%p]=%08X\n",&ble_wakeup_input_param->ad_filter[i].ad_data_mask,ble_wakeup_input_param->ad_filter[i].ad_data_mask);
        dbg("gpio_trigger_idx [%p]=%x\n",&ble_wakeup_input_param->ad_filter[i].gpio_trigger_idx,ble_wakeup_input_param->ad_filter[i].gpio_trigger_idx);
        dbg("ad_role [%p]=%x\n",&ble_wakeup_input_param->ad_filter[i].ad_role,ble_wakeup_input_param->ad_filter[i].ad_role);
        dbg("ad_len [%p]=%x\n",&ble_wakeup_input_param->ad_filter[i].ad_len,ble_wakeup_input_param->ad_filter[i].ad_len);
        dbg("ad_type [%p]=%x\n",&ble_wakeup_input_param->ad_filter[i].ad_type,ble_wakeup_input_param->ad_filter[i].ad_type);
    }
    if (ble_wakeup_input_param->magic_num == BLE_WAKEUP_MAGIC_NUM) {
        ble_wakeup_user.magic_num = ble_wakeup_input_param->magic_num;
        ble_wakeup_user.delay_scan_to = ble_wakeup_input_param->delay_scan_to;
        ble_wakeup_user.reboot_to = ble_wakeup_input_param->reboot_to;
        for(uint8_t i=0;i<MAX_GPIO_TRIGGER_NUM;i++){
            if(ble_wakeup_input_param->gpio_num[i] != 0xff){
                ble_wakeup_user.gpio_num[i] = ble_wakeup_input_param->gpio_num[i];
            }
            if(ble_wakeup_input_param->gpio_dft_lvl[i] != 0xff){
                ble_wakeup_user.gpio_dft_lvl[i] = ble_wakeup_input_param->gpio_dft_lvl[i];
            }
        }

        for(uint8_t idx=0;idx<BLE_AD_FILTER_NUM_MAX;idx++){
            ble_wakeup_user.ad_filter[idx].ad_data_mask = ble_wakeup_input_param->ad_filter[idx].ad_data_mask;
            ble_wakeup_user.ad_filter[idx].gpio_trigger_idx = ble_wakeup_input_param->ad_filter[idx].gpio_trigger_idx;
            ble_wakeup_user.ad_filter[idx].ad_role = ble_wakeup_input_param->ad_filter[idx].ad_role;
            ble_wakeup_user.ad_filter[idx].ad_len = ble_wakeup_input_param->ad_filter[idx].ad_len;
            ble_wakeup_user.ad_filter[idx].ad_type = ble_wakeup_input_param->ad_filter[idx].ad_type;
            dbg("idx %d find valid ad_data:\n",idx);
            for (uint8_t i = 0; i < ble_wakeup_user.ad_filter[idx].ad_len-1; i++) {
                ble_wakeup_user.ad_filter[idx].ad_data[i] = ble_wakeup_input_param->ad_filter[idx].ad_data[i];
                dbg(" 0x%x ", ble_wakeup_user.ad_filter[idx].ad_data[i]);
            }
            dbg("\n");
            if(ble_wakeup_user.ad_filter[idx].ad_len >0 && ((ble_wakeup_user.ad_filter[idx].ad_role & AD_ROLE_FLAG) == ROLE_COMBO)){
                uint8_t trigger_idx = (ble_wakeup_user.ad_filter[idx].ad_role & ROLE_COMBO_IDX_FLAG)>>4;
                dbg("ad_role: %d, trigger_idx %d\n",ble_wakeup_user.ad_filter[idx].ad_role,trigger_idx);
                ble_combo_num[trigger_idx]++;
            }
            dbg("idx %d wl_addr:\n",idx);
            for (uint8_t i = 0; i < 6; i++) {
                ble_wakeup_user.ad_filter[idx].wl_addr.addr[i] = ble_wakeup_input_param->ad_filter[idx].wl_addr.addr[i];
                dbg(" 0x%x ", ble_wakeup_user.ad_filter[idx].wl_addr.addr[i]);
            }
            dbg("\n");
        }
        for(uint8_t i=0;i<MAX_ROLE_COMBO_IDX_NUM;i++){
            dbg("ble_combo_num idx %d: %d\n", i,ble_combo_num[i]);
        }
    } else {
        dbg("no filter ad_data set\n");
    }
    for(uint8_t i=0;i<MAX_GPIO_TRIGGER_NUM;i++){
        switch(ble_wakeup_input_param->gpio_dft_lvl[i]){
            case 0:
                gpiob_init(ble_wakeup_user.gpio_num[i]);
                gpiob_clr(ble_wakeup_user.gpio_num[i]); // low by default
                gpiob_dir_out(ble_wakeup_user.gpio_num[i]);
                gpiob_clr(ble_wakeup_user.gpio_num[i]); // low by default
                dbg("idx %d ,gpiob %d set\r\n",i,ble_wakeup_user.gpio_num[i]);
                break;
            case 1:
                gpiob_force_pull_up_enable(ble_wakeup_user.gpio_num[i]);
                gpiob_init(ble_wakeup_user.gpio_num[i]);
                gpiob_set(ble_wakeup_user.gpio_num[i]); // high by default
                gpiob_dir_out(ble_wakeup_user.gpio_num[i]);
                gpiob_set(ble_wakeup_user.gpio_num[i]); // high by default
                dbg("idx %d ,gpiob %d set\r\n",i,ble_wakeup_user.gpio_num[i]);
                break;
            default:
            #if 0
            #if CONFIG_DEFAULT_PULLED_UP == 1
                gpiob_force_pull_up_enable(BLE_WAKEUP_GPIO);
                gpiob_init(BLE_WAKEUP_GPIO);
                gpiob_set(BLE_WAKEUP_GPIO); // high by default
                gpiob_dir_out(BLE_WAKEUP_GPIO);
                gpiob_set(BLE_WAKEUP_GPIO); // high by default
            #else
                gpiob_init(BLE_WAKEUP_GPIO);
                gpiob_clr(BLE_WAKEUP_GPIO); // low by default
                gpiob_dir_out(BLE_WAKEUP_GPIO);
                gpiob_clr(BLE_WAKEUP_GPIO); // low by default
            #endif
                dbg("gpiob %d set\r\n",BLE_WAKEUP_GPIO);
            #endif
                dbg("idx %d no set\r\n",i);
                break;
        }
    }

    co_main_evt_handler_rigister(CO_MODUAL_BLE_WAKEUP,app_ble_wakeup_handle_process);
}

#endif//PLF_BLE_ONLY == 1 && PLF_BLE_WAKEUP == 1
#endif//PLF_BT_STACK == 1
