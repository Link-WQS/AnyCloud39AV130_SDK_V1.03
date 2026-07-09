/*
 * Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.
 * Description: host platform dyn mem source file
 * Author: Huanghe
 * Create: 2021-06-08
 */

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_firmware.h"
#include "plat_debug.h"
#include "plat_pm_board.h"
#include "plat_pm.h"
#include "ini.h"
#include "plat_cali.h"

#include <linux/version.h>
#endif
#include "soc_osal.h"
#include "hal_soc_reg.h"
#include "hcc_bus.h"
#include "hcc_if.h"

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*****************************************************************************
  3 全局变量定义
*****************************************************************************/
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
custom_cali_func g_custom_cali_func = NULL;
EXPORT_SYMBOL_GPL(g_custom_cali_func);
custom_cali_func g_custom_hmac_exit_later_func = NULL;
EXPORT_SYMBOL_GPL(g_custom_hmac_exit_later_func);

void set_custom_exit_later_func(custom_cali_func func)
{
    g_custom_hmac_exit_later_func = func;
}

custom_cali_func get_custom_exit_later_func(void)
{
    return g_custom_hmac_exit_later_func;
}

void set_custom_cali_func(custom_cali_func func)
{
    g_custom_cali_func = func;
}

custom_cali_func get_custom_cali_func(void)
{
    return g_custom_cali_func;
}

EXPORT_SYMBOL(set_custom_exit_later_func);
EXPORT_SYMBOL(set_custom_cali_func);
#endif

#endif

td_void cali_dyn_mem_cfg(td_void)
{
    hcc_switch_status(HCC_OFF);
    // 0x33: 0011_0011b close RAM2&RAM3's clock
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, 0x33);
    // 0x0: bank ram2 use as CPURAM
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_BANK_RAM2_USE_CFG_REG, 0x0);
    // 0x0: bank ram3 use as CPURAM
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_BANK_RAM3_USE_CFG_REG, 0x0);
    // 0x430000: cfg bank ram2 START_ADDR
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_BANK_RAM2_START_ADDR_REG, 0x430000);
    // 0x438000: cfg bank ram3 START_ADDR
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_BANK_RAM3_START_ADDR_REG, 0x438000);
    // 0x3F: 0011_1111b restore RAM2&RAM3's clock
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, 0x3F);
    hcc_switch_status(HCC_BUS_FORBID);
}

td_void bsle_cali_dyn_mem_cfg(td_void)
{
    hcc_switch_status(HCC_OFF);
    // 0x2F: 0010_1111b close RAM4's clock
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, 0x2F);
    // 0x0: bank ram4 use as CPURAM
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_BANK_RAM4_USE_CFG_REG, 0x0);
    // 0x440000: cfg bank ram4 START_ADDR
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_BANK_RAM4_START_ADDR_REG, 0x440000);
    // 0x3F: 0011_1111b restore RAM4's clock
    hcc_write_reg(HCC_CHANNEL_AP, CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, 0x3F);
    hcc_switch_status(HCC_BUS_FORBID);
}

