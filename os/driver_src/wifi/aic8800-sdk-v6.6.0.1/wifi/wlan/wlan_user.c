/*
 * Copyright (C) 2018-2024 AICSemi Ltd.
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
#include "wlan_user.h"
#include "dbg.h"
#include "boot.h"

#if (PLF_AIC8800M40 && PLF_MODULE_TEMP_COMP)
#include "temp_comp.h"
#include "fhost.h"
#include "fhost_config.h"
#include "rwnx_msg_tx.h"
#include "flash_api_wifi.h"
#endif

/**
 * Macros
 */

/**
 * TypeDefs
 */

/**
 * Variables
 */

/**
 * Functions
 */

void wlan_process_ipc_corruption(void)
{
    dbg("wifi ipc corrupted!\n");

    /* USER CODE BEGIN */
    //panic();
    /* USER CODE END */
}

void wlan_initialize_user_setting(void)
{
    #if PLF_AIC8800M40 && PLF_MODULE_TEMP_COMP
    {
        int ret;
        struct mm_set_vendor_swconfig_req req = {0,};
        struct mm_set_vendor_swconfig_cfm cfm = {0,};
        // send vendor swconfig req
        req.swconfig_id = TEMP_COMP_SET_REQ;
        req.temp_comp_set_req.enable = 1;
        req.temp_comp_set_req.reserved[0] = (uint8_t)temp_comp_calibed_temp_level_get();
        req.temp_comp_set_req.tmr_period_ms  = temp_comp_timer_period_ms_get();
        ret = rwnx_send_vendor_swconfig_req(&req, &cfm);
        if (!ret) {
            dbg("temp_comp status=0x%x\n", cfm.temp_comp_set_cfm.status);
        } else {
            dbg("send TEMP_COMP_SET_REQ fail, ret=%d\n", ret);
            return;
        }
        #if TEMP_COMP_DPD_RES_IN_FLASH
        // enbale dynamic dpd based on config
        if (fhost_usr_cfg.wifi_ext_flags & WIFI_EXT_DYNAMIC_DPD_CALIB_EN_FLAG) {
            // send vendor swconfig req
            req.swconfig_id = EXT_FLAGS_MASK_SET_REQ;
            req.ext_flags_mask_set_req.user_flags_mask = USER_DYNAMIC_DPD_CALIB_FLAG;
            req.ext_flags_mask_set_req.user_flags_val  = USER_DYNAMIC_DPD_CALIB_FLAG;
            ret = rwnx_send_vendor_swconfig_req(&req, &cfm);
            if (!ret) {
                dbg("user_flags=0x%x\n", cfm.ext_flags_mask_set_cfm.user_flags);
            } else {
                dbg("send EXT_FLAGS_MASK_SET_REQ fail, ret=%d\n", ret);
                return;
            }
        }
        #endif
    }
    #endif

    /* USER CODE BEGIN */
    /* USER CODE END */
}

#if (PLF_AIC8800M40)
void wlan_dynamic_dpd_cal_res_indication(int ch_grp_idx, int dpd_temp_lvl, int dpd_degree_val, unsigned int rf_misc_ram_addr)
{
    #if PLF_MODULE_TEMP_COMP && TEMP_COMP_DPD_RES_IN_FLASH
    if (fhost_usr_cfg.wifi_ext_flags & WIFI_EXT_DYNAMIC_DPD_CALIB_EN_FLAG) {
        wifi_rf_cal_info_t *buf_tmp;
        temp_comp_dpd_cal_curr_ch_idx_set(ch_grp_idx);
        buf_tmp = rtos_malloc(sizeof(wifi_rf_cal_info_t));
        if (buf_tmp) {
            int ret;
            dbg("store dpd_res ch=%d lvl=%d t=%d into flash\n", ch_grp_idx, dpd_temp_lvl, dpd_degree_val);
            ret = flash_wifi_dpd_cal_res_write(ch_grp_idx, dpd_temp_lvl, dpd_degree_val, buf_tmp, (void *)rf_misc_ram_addr);
            rtos_free(buf_tmp);
            if (ret) {
                dbg("wifi_rf_cal_info fls wr fail %d, %d, %x\n", ch_grp_idx, dpd_temp_lvl, rf_misc_ram_addr);
            }
        } else {
            dbg("wifi_rf_cal_info ram alloc fail\n");
        }
    }
    #endif
}
#endif
