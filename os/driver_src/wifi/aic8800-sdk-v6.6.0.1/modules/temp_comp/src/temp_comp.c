/*
 * Copyright (C) 2018-2023 AICSemi Ltd.
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

/**
 * Includes
 */
#include "rtos.h"
#include "dbg.h"
#include "temp_comp.h"
#include "msadc_api.h"
#include "rf_config.h"
#if (PLF_AIC8800 && PLF_WIFI_STACK && defined(CONFIG_RWNX_LWIP))
#include "rwnx_msg_tx.h"
#endif
#include "co_math.h"
#include "flash_api.h"
#include "pmic_api.h"
#if (PLF_AIC8800M40)
#include "reg_sysctrl.h"
#endif
#if TEMP_COMP_DPD_RES_IN_FLASH
#include "fmacfw_api.h"
#include "wlan_if.h"
#include "rwnx_msg_tx.h"
#endif

/**
 * Macros
 */
// temperature compensation timer period in second
#ifndef TEMP_COMP_TIMER_PERIOD_SEC
#define TEMP_COMP_TIMER_PERIOD_SEC      15
#endif
#define TEMP_COMP_DBG_PRINT(fmt, ...)   dbg(D_TEMP fmt, ##__VA_ARGS__)//do {} while(0)//

#if (PLF_AIC8800)
#define TEMP_COMP_DEGREE_INTERVAL       16
#else
#define TEMP_COMP_DEGREE_INTERVAL       10
#endif
#define TEMP_COMP_DEGREE_INVALID        (-128)
#define TEMP_DEGREE_STANDARDIZE(degree)   ((degree < 0) ? \
        ((degree - ((TEMP_COMP_DEGREE_INTERVAL / 2) - 1)) / (TEMP_COMP_DEGREE_INTERVAL / 2) * (TEMP_COMP_DEGREE_INTERVAL / 2)) : \
        (degree / (TEMP_COMP_DEGREE_INTERVAL / 2) * (TEMP_COMP_DEGREE_INTERVAL / 2)))

#if TEMP_COMP_DPD_RES_IN_FLASH
#define TEMP_COMP_DPD_CAL_DEGREE_INTERVAL               20
#define TEMP_COMP_DPD_CAL_DEGREE2LEVEL(degree)          \
    (((degree) + 35) / TEMP_COMP_DPD_CAL_DEGREE_INTERVAL)
#define TEMP_COMP_DPD_CAL_DEGREE_STANDARDIZE(degree)    \
    (((degree) + 25 + 5) / (TEMP_COMP_DPD_CAL_DEGREE_INTERVAL / 2) * (TEMP_COMP_DPD_CAL_DEGREE_INTERVAL / 2) - 25)
#endif

/**
 * Constants
 */
const int8_t temp_comp_xtal_cap_ofst[16] = {
    //<-30, <-20,   <-10,   <0,     <10,    <20,    <30,    <40
    -4,      2,      4,      5,      4,      3,      2,      0,
    //<50,  <60,    <70,    <80,    <90,    <100,   <110,   >=110
    -1,     -2,     -3,     -3,      0,      3,      7,     15
};

/**
 * TypeDefs
 */

/**
 * Variables
 */
temp_comp_env_t temp_comp_env = {
    #if (PLF_AIC8800)
    .degree_calib = 35,
    #else
    NULL,
    #endif
};

/**
 * Functions
 */
static uint8_t temp_comp_xtal_cap_calculate(int level_new)
{
    #if (PLF_AIC8800)
    int8_t level_calibed = temp_degree_to_level(temp_comp_env.degree_calib);
    #else
    int8_t level_calibed = temp_comp_env.temp_level_calibed;
    #endif
    int8_t ofst_new = temp_comp_xtal_cap_ofst[level_new + 7] - temp_comp_xtal_cap_ofst[level_calibed + 7];
    int xtal_cap_new = (int)temp_comp_env.xtal_cap_calibed + (int)ofst_new;
    if (xtal_cap_new < 1) {
        xtal_cap_new = 1;
    }
    if (xtal_cap_new > 0x1F) {
        xtal_cap_new = 0x1F;
    }
    TEMP_COMP_DBG_PRINT(D_INF "lvl %d->%d, cap=0x%x->0x%x\n", level_calibed, level_new, temp_comp_env.xtal_cap_calibed, xtal_cap_new);
    return (uint8_t)xtal_cap_new;
}

static void temp_comp_xtal_cap_update(void)
{
    // xtal_cap comp
    if (temp_comp_env.xtal_cap_current != temp_comp_env.xtal_cap_target) {
        if (temp_comp_env.xtal_cap_current < temp_comp_env.xtal_cap_target) {
            temp_comp_env.xtal_cap_current++;
        } else {
            temp_comp_env.xtal_cap_current--;
        }
        pmic_xtal_cap_set(temp_comp_env.xtal_cap_current);
        TEMP_COMP_DBG_PRINT(D_WRN "adjust xtal_cap: tgt=0x%x, cur=0x%x\n",
            temp_comp_env.xtal_cap_target, temp_comp_env.xtal_cap_current);
    }
}

static int temp_comp_check(int8_t degree)
{
    int ret = 0;
    temp_comp_env.degree_real = degree; // always update
    #if (PLF_AIC8800M40)
    aonsysctrl_chipinfo_temp_setf(degree); // record into hw reg
    #endif
    if (co_abs(degree - temp_comp_env.degree_last) >= (TEMP_COMP_DEGREE_INTERVAL / 2)) {
        temp_comp_env.degree_last = TEMP_DEGREE_STANDARDIZE(degree);
        ret = 1;
    }
    return ret;
}

static void temp_comp_process(int8_t degree)
{
    int level = temp_degree_to_level(degree);
    // xtal_cap comp
    temp_comp_env.xtal_cap_target  = temp_comp_xtal_cap_calculate(level);
    #if (PLF_WIFI_STACK && defined(CONFIG_RWNX_LWIP))
    TEMP_COMP_DBG_PRINT(D_INF "pll recalib\n");
    rf_pll_recalib();
    #endif
    #if (PLF_AIC8800 && PLF_WIFI_STACK && defined(CONFIG_RWNX_LWIP))
    {
        rwnx_txpwr_comp_t txpwr_comp;
        int8_t txpwr_idx_comp_5g;
        if (degree >= temp_comp_env.degree_calib) {
            txpwr_idx_comp_5g = (degree - temp_comp_env.degree_calib) >> 4;
            if (txpwr_idx_comp_5g > 1) {
                txpwr_idx_comp_5g = 1;
            }
        } else {
            txpwr_idx_comp_5g = ((temp_comp_env.degree_calib - degree) >> 4) * -1;
        }
        txpwr_comp.pwridx_comp_5g = txpwr_idx_comp_5g;
        rwnx_set_txpwr_comp_8800(&txpwr_comp);
    }
    #endif
}

#if TEMP_COMP_DPD_RES_IN_FLASH
static int temp_comp_dpd_cal_update_check(int8_t degree)
{
    int ret = 0;
    if (co_abs(degree - temp_comp_env.dpd_cal_temp_degree_last) >= (TEMP_COMP_DPD_CAL_DEGREE_INTERVAL / 2)) {
        if (degree < -35) degree = -35;
        if (degree > 124) degree = 124;
        TEMP_COMP_DBG_PRINT(D_INF "%s degree %d->%d(real=%d)\n", __func__, temp_comp_env.dpd_cal_temp_degree_last, TEMP_COMP_DPD_CAL_DEGREE_STANDARDIZE(degree), degree);
        temp_comp_env.dpd_cal_temp_level_last = TEMP_COMP_DPD_CAL_DEGREE2LEVEL(degree); // lvl: [0~5]
        temp_comp_env.dpd_cal_temp_degree_last = TEMP_COMP_DPD_CAL_DEGREE_STANDARDIZE(degree);
        ret = 1;
    }
    return ret;
}

static void temp_comp_dpd_cal_update_process(int8_t degree)
{
    TEMP_COMP_DBG_PRINT(D_INF "%s t=%d\n", __func__, degree);
    if ((wlan_get_connect_status() == WLAN_CONNECTED)
        #ifdef CFG_SOFTAP
        || wlan_get_softap_status()
        #endif
        ) {
        int ch_grp_idx = temp_comp_env.dpd_cal_ch_idx_curr;
        int dpd_temp_lvl = temp_comp_env.dpd_cal_temp_level_last;
        // update misc ram if dpd_cal_res valid in flash
        int dpd_degree_flash, degree_ok = 0;
        int res_valid = flash_wifi_dpd_cal_res_valid(ch_grp_idx, dpd_temp_lvl, &dpd_degree_flash);
        if (res_valid) {
            int degree_mid = dpd_temp_lvl * TEMP_COMP_DPD_CAL_DEGREE_INTERVAL - 25;
            if (co_abs(dpd_degree_flash - degree_mid) < co_abs(degree - degree_mid)) {
                degree_ok = 1;
            }
        }
        TEMP_COMP_DBG_PRINT(D_INF "res_valid=%d(df=%d), ch=%d, lvl=%d, degree_ok=%d\n", res_valid, res_valid ? dpd_degree_flash : 0, ch_grp_idx, dpd_temp_lvl, degree_ok);
        if (res_valid && degree_ok) {
            int ret;
            uint32_t cfg_base;
            uint32_t rf_misc_ram_addr;
            // extract symbol table
            cfg_base = FMACFW_CFG_BASE_ADDR;
            rf_misc_ram_addr = REG_PL_RD(cfg_base + 0x10);
            ret = flash_wifi_dpd_cal_res_read(ch_grp_idx, dpd_temp_lvl, (void *)rf_misc_ram_addr);
            if (ret == INFO_READ_DONE) {
                struct mm_set_vendor_swconfig_req req = {0,};
                struct mm_set_vendor_swconfig_cfm cfm = {0,};
                // send vendor swconfig req
                req.swconfig_id = DYN_DPD_CAL_RES_SET_REQ;
                req.dyn_dpd_cal_res_set_req.dpd_temp_lvl = (uint8_t)dpd_temp_lvl;
                req.dyn_dpd_cal_res_set_req.set_val  = (uint8_t)(0x01U << ch_grp_idx);
                req.dyn_dpd_cal_res_set_req.set_mask = (uint8_t)(0x01U << ch_grp_idx);
                TEMP_COMP_DBG_PRINT("send DYN_DPD_CAL_RES_SET_REQ, ch=%d, lvl=%d\n", ch_grp_idx, dpd_temp_lvl);
                ret = rwnx_send_vendor_swconfig_req(&req, &cfm);
                if (!ret) {
                    TEMP_COMP_DBG_PRINT("dpd_res[%d] cur_val=0x%x\n", cfm.dyn_dpd_cal_res_set_cfm.dpd_temp_lvl, cfm.dyn_dpd_cal_res_set_cfm.cur_val);
                }
            } else {
                TEMP_COMP_DBG_PRINT("read dpd cal res from fls fail, ret=%d\n", ret);
            }
        }
    }
}

void temp_comp_dpd_cal_curr_ch_idx_set(int ch_grp_idx)
{
    temp_comp_env.dpd_cal_ch_idx_curr = ch_grp_idx;
}
#endif

void temp_comp_timer_handler(void const *param)
{
    int degree_curr;
    degree_curr = msadc_temp_measure(MSADC_TEMP_CHIP);
    TEMP_COMP_DBG_PRINT(D_INF "t=%d C\n", degree_curr);
    if (temp_comp_check(degree_curr)) {
        temp_comp_process(degree_curr);
    }
    temp_comp_xtal_cap_update();
    #if TEMP_COMP_DPD_RES_IN_FLASH
    if (temp_comp_dpd_cal_update_check(degree_curr)) {
        temp_comp_dpd_cal_update_process(degree_curr);
    }
    #endif
}

int8_t temp_comp_calibed_temp_level_get(void)
{
    #if (PLF_AIC8800)
    int8_t level_calibed = temp_degree_to_level(temp_comp_env.degree_calib);
    #else
    int8_t level_calibed = temp_comp_env.temp_level_calibed;
    #endif
    return level_calibed;
}

uint32_t temp_comp_timer_period_ms_get(void)
{
    return TEMP_COMP_TIMER_PERIOD_SEC * 1000;
}

void temp_comp_init(void)
{
    if (temp_comp_env.tmr_hdl == NULL) {
        TimerHandle_t temp_timer = rtos_timer_create("temp_timer", TEMP_COMP_TIMER_PERIOD_SEC*1000, pdTRUE, NULL,
                                                    (TimerCallbackFunction_t)temp_comp_timer_handler);
        if (!temp_timer) {
            TEMP_COMP_DBG_PRINT("temp_timer create fail\n");
            return;
        }
        temp_comp_env.tmr_hdl = temp_timer;
        temp_comp_env.degree_boot = msadc_temp_measure(MSADC_TEMP_CHIP);
        temp_comp_env.degree_last = TEMP_COMP_DEGREE_INVALID;
        #if TEMP_COMP_DPD_RES_IN_FLASH
        temp_comp_env.dpd_cal_temp_degree_last = TEMP_COMP_DEGREE_INVALID;
        #endif
        #if (PLF_AIC8800M40)
        aonsysctrl_chipinfo_temp_setf(temp_comp_env.degree_boot); // record into hw reg
        #endif
        TEMP_COMP_DBG_PRINT("degree_boot=%d\n", temp_comp_env.degree_boot);
        // xtal_cap comp
        {
            uint8_t xtal_cap = 0;
            xtal_cap_info_t xtal = {0,};
            #if (PLF_AIC8800M40)
            int temp_level = 0;
            calib_temp_level_info_t temp_info = {0,};
            #endif
            int ret = flash_calib_xtal_cap_read(&xtal);
            if (ret == INFO_READ_DONE) {
                xtal_cap = xtal.cap;
            }
            if (xtal_cap) {
                if (xtal_cap > 0x1F) {
                    xtal_cap = 0x1F;
                }
                temp_comp_env.xtal_cap_calibed = xtal_cap;
            } else {
                temp_comp_env.xtal_cap_calibed = 0x0F; // default val from lima
            }
            temp_comp_env.xtal_cap_current = (uint8_t)pmic_xtal_cap_get();
            #if (PLF_AIC8800M40)
            ret = flash_calib_temp_level_read(&temp_info);
            if (ret == INFO_READ_DONE) {
                temp_level = temp_info.temp_level;
            }
            if (temp_level) {
                if (temp_level < -7) {
                    temp_level = -7;
                }
                if (temp_level > 8) {
                    temp_level = 8;
                }
                temp_comp_env.temp_level_calibed = (int8_t)temp_level;
                TEMP_COMP_DBG_PRINT("temp_lvl_calibed=%d\n", temp_level);
            }
            #endif
        }
    } else {
        TEMP_COMP_DBG_PRINT(D_WRN "temp_comp already inited\n");
    }
}

void temp_comp_deinit(void)
{
    if (temp_comp_env.tmr_hdl) {
        int ret = rtos_timer_delete(temp_comp_env.tmr_hdl, 0);
        if (ret) {
            TEMP_COMP_DBG_PRINT("temp_timer delete fail, ret=%d\n", ret);
        } else {
            temp_comp_env.tmr_hdl = NULL;
        }
    }
}

void temp_comp_start(void)
{
    if (temp_comp_env.tmr_hdl) {
        int ret = rtos_timer_start(temp_comp_env.tmr_hdl, 0, false);
        if (ret) {
            TEMP_COMP_DBG_PRINT("temp_timer start fail, ret=%d\n", ret);
        }
    }
}

void temp_comp_stop(void)
{
    if (temp_comp_env.tmr_hdl) {
        int ret = rtos_timer_stop(temp_comp_env.tmr_hdl, 0);
        if (ret) {
            TEMP_COMP_DBG_PRINT("temp_timer start fail, ret=%d\n", ret);
        }
    }
}
