/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 * Sukumar Ghorai <s-ghorai@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
//#include <asm/errno.h>

#ifdef CONFIG_39EV33X_CODE
#include <asm/arch-ak39ev33x/ak_cpu.h>
#include <asm/arch-ak39ev33x/ak_types.h> 
#include <asm/arch-ak39ev33x/ak_module_reset.h>
#include <asm/arch-ak39ev33x/ak_timer_wdt.h>
#endif

#ifdef CONFIG_37_D_CODE
#include <asm/arch-ak37d/ak_cpu.h>
#include <asm/arch-ak37d/ak_types.h> 
#include <asm/arch-ak37d/ak_module_reset.h>
#include <asm/arch-ak37d/ak_timer_wdt.h>
#endif

#ifdef CONFIG_37_E_CODE
#include <asm/arch-ak37e/ak_cpu.h>
#include <asm/arch-ak37e/ak_types.h> 
#include <asm/arch-ak37e/ak_module_reset.h>
#include <asm/arch-ak37e/ak_timer_wdt.h>
#endif

#if defined(CONFIG_3918AV100_CODE)
#include <asm/arch-ak3918av100/ak_cpu.h>
#include <asm/arch-ak3918av100/ak_types.h> 
#include <asm/arch-ak3918av100/ak_module_reset.h>
#include <asm/arch-ak3918av100/ak_timer_wdt.h>
#endif

#if defined(CONFIG_3918AV130_CODE)
#include <asm/arch-ak3918av130/ak_cpu.h>
#include <asm/arch-ak3918av130/ak_types.h> 
#include <asm/arch-ak3918av130/ak_module_reset.h>
#include <asm/arch-ak3918av130/ak_timer_wdt.h>
#endif

#if defined(CONFIG_KM01A_CODE)
#include <asm/arch-km01a/ak_cpu.h>
#include <asm/arch-km01a/ak_types.h> 
#include <asm/arch-km01a/ak_module_reset.h>
#include <asm/arch-km01a/ak_timer_wdt.h>
#endif

#ifdef CONFIG_3918EV300L_CODE
#include <asm/arch-ak3918ev300l/ak_cpu.h>
#include <asm/arch-ak3918ev300l/ak_types.h> 
#include <asm/arch-ak3918ev300l/ak_module_reset.h>
#include <asm/arch-ak3918ev300l/ak_timer_wdt.h>
#endif

#ifdef CONFIG_39EV200_CODE
#include <asm/arch-ak39ev200/ak_cpu.h>
#include <asm/arch-ak39ev200/ak_types.h> 
#include <asm/arch-ak39ev200/ak_module_reset.h>
#include <asm/arch-ak39ev200/ak_timer_wdt.h>
#endif




#define UNIT_NS  21333
#define WATCH_DOG_1_SECOND_SET  (1000000000 / UNIT_NS)   // 1000000*64/3

static unsigned int def_heartbeat = 8 * WATCH_DOG_1_SECOND_SET;

/**
 * @brief enable watch_dog timer for system reset function.
 *
 * enable watch_dog timer for system reset function.
 * @author CaoDonghua
 * @date 2016-12-29
 * @param[in] void none
 * @return void none
 * @retval  none
 */
void wdt_enable(void)
{
    unsigned int cfg_val = (unsigned int)def_heartbeat;

    /* set watchdog time*/
    REG32(MODULE_WDT_CFG1) = ((0x55 << 24) | cfg_val);
 #if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
    REG32(0x00000000);
 #endif

    /*enable watchdog */
    REG32(MODULE_WDT_CFG2) = ((0xaaUL << 24) | 0x1);
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
    REG32(0x00000000);
#endif


    /*start watchdog */
    REG32(MODULE_WDT_CFG2) = ((0xaaUL << 24) | 0x3);
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
       REG32(0x00000000);
#endif

}

/**
 * @brief set watch_dog alive time.
 *
 * set watch_dog alive time, after that make a reset to system
 * @author CaoDonghua
 * @date 2016-09-30
 * @param[in] unsigned int heartbeat: watch dog keep alive time.
 * @return void none
 * @retval  none
 */
void wdt_keepalive(unsigned int heartbeat)
{
    unsigned int cfg_val = (unsigned int)heartbeat;
    
    printf("heartbeat = %x\n", heartbeat);

    REG32(MODULE_WDT_CFG1) = ((0x55 << 24) | cfg_val);
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
       REG32(0x00000000);
#endif

    REG32(MODULE_WDT_CFG2) = ((0xaaUL << 24) | 0x1);
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
       REG32(0x00000000);
#endif

    REG32(MODULE_WDT_CFG2) = ((0xaaUL << 24) | 0x3);
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
       REG32(0x00000000);
#endif


}

void wdt_disable(void)
{
    REG32(MODULE_WDT_CFG2) = ((0xaaUL << 24) | 0x0);
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
       REG32(0x00000000);
#endif

}

