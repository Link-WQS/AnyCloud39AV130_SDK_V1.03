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
#ifndef _TEMP_COMP_H_
#define _TEMP_COMP_H_

/**
 * Includes
 */
#include "co_int.h"
#include "rtos_al.h"
#if (PLF_AIC8800M40 && PLF_WIFI_STACK)
#include "flash_api_wifi.h"
#endif

/**
 * Macros
 */
/* convert calib temp degree to level stored in flash */
#define TEMP_DEGREE2LEVEL(degree)   (((degree) < 30) ? ((int)(((int)(degree) - 40) / 10)) : ((int)(((int)(degree) - 30) / 10)))
#define TEMP_LEVEL2DEGREE_H(level)  ((int)(level) * 10 + 40)

#define TEMP_COMP_DPD_RES_IN_FLASH  (PLF_AIC8800M40 && PLF_WIFI_STACK && defined(CONFIG_RWNX_LWIP) && defined(CFG_DPD))

/**
 * TypeDefs
 */
typedef struct {
    TimerHandle_t tmr_hdl;
    #if (PLF_AIC8800)
    int8_t degree_calib;
    #else
    int8_t temp_level_calibed;
    #endif
    int8_t degree_boot;
    int8_t degree_last; // last compensation temperature
    int8_t degree_real; // last realtime read temperature
    uint8_t xtal_cap_calibed;
    uint8_t xtal_cap_target;
    uint8_t xtal_cap_current;
    #if TEMP_COMP_DPD_RES_IN_FLASH
    uint8_t dpd_cal_ch_idx_curr;
    int8_t dpd_cal_temp_level_last;
    int8_t dpd_cal_temp_degree_last; // last dpd cal temperature, standardlized
    #endif
} temp_comp_env_t;

/**
 * Functions
 */

/**
 * @brief       Convert temp degree to level
 */
__STATIC_INLINE int temp_degree_to_level(int degree)
{
    int level = TEMP_DEGREE2LEVEL(degree);
    if (level < -7) {
        level = -7;
    }
    if (level > 8) {
        level = 8;
    }
    return level;
}

/**
 * @brief       Get calibed temp level
 */
int8_t temp_comp_calibed_temp_level_get(void);

/**
 * @brief       Get timer period in ms
 */
uint32_t temp_comp_timer_period_ms_get(void);

#if TEMP_COMP_DPD_RES_IN_FLASH
/**
 * @brief       Set dpd cal curr ch_idx
 */
void temp_comp_dpd_cal_curr_ch_idx_set(int ch_grp_idx);
#endif

/**
 * @brief       Init temp comp process
 */
void temp_comp_init(void);

/**
 * @brief       Deinit temp comp process
 */
void temp_comp_deinit(void);

/**
 * @brief       Start temp comp timer
 */
void temp_comp_start(void);

/**
 * @brief       Stop temp comp timer
 */
void temp_comp_stop(void);

#endif /* _TEMP_COMP_H_ */
