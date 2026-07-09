/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: tsensor driver implementatioin.
 * Author: CompanyName
 * Create: 2023-02-15
 */

#if defined(CONFIG_SUPPORT_TSENSOR)

#include "tsensor.h"
#include <soc_tsensor.h>
#include "soc_resource_id.h"
#include "soc_osal.h"

#define LOW_CODE 114
#define HIGH_CODE 896
#define LOW_TEMP (-40)
#define HIGH_TEMP 125
#define TMP_BASE_ADDR 0x40018800

td_s16 g_temperature_record;

td_u32 uapi_tsensor_read_temperature(td_s16 *temperature)
{
    td_u16 temperature_code;
    td_u32 tmp_val = 0;

    if (temperature == TD_NULL) {
        return EXT_ERR_TSENSOR_INVALID_PARAMETER;
    }

    if (hcc_read_reg(HCC_CHANNEL_AP, TMP_BASE_ADDR + TSENSOR_AUTO_STS, &tmp_val) != EXT_ERR_SUCCESS) {
        return EXT_ERR_FAILURE;
    }
    if (tmp_val & (1 << TSENSOR_READY_POS)) {
        temperature_code = (td_u16)(tmp_val >> TSENSOR_READY_DATA_OFFSET_2BITS);
        *temperature = ((temperature_code - LOW_CODE) * (HIGH_TEMP - LOW_TEMP) / (HIGH_CODE - LOW_CODE) + LOW_TEMP);
        g_temperature_record = *temperature;
        return EXT_ERR_SUCCESS;
    }
    *temperature = g_temperature_record;
    return EXT_ERR_SUCCESS;
}

td_u32 uapi_tsensor_get_current_temp(int8_t *temp)
{
    td_s16 temperature;
    td_u32 ret;

    ret = uapi_tsensor_read_temperature(&temperature);
    if (ret == EXT_ERR_SUCCESS) {
        *temp = temperature & 0xFFFF; // 0xFFFF 取低16bit，该数字只低16bit有效
    }
    return ret;

}

osal_module_export(uapi_tsensor_read_temperature);

#endif
