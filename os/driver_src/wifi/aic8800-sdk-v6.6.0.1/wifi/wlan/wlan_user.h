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
#ifndef _WLAN_USER_H_
#define _WLAN_USER_H_

/**
 * Includes
 */
#include "plf.h"

/**
 * Macros
 */

/**
 * TypeDefs
 */

/**
 * Functions
 */

/**
 * @brief: process wifi ipc corruption
 */
void wlan_process_ipc_corruption(void);

/**
 * @brief: initialize user wifi setting
 */
void wlan_initialize_user_setting(void);

#if (PLF_AIC8800M40)
/**
 * @brief: process wifi dynamic dpd cal res indication
 */
void wlan_dynamic_dpd_cal_res_indication(int ch_grp_idx, int dpd_temp_lvl, int dpd_degree_val, unsigned int rf_misc_ram_addr);
#endif

#endif /* _WLAN_USER_H_ */
