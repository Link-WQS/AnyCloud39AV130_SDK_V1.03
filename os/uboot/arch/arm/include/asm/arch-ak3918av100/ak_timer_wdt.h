/* 
 *  mach/reset.h
 */
#ifndef _AK_TIMER_WDT_H_
#define _AK_TIMER_WDT_H_

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch-ak3918av100/ak_cpu.h>
#include <asm/arch-ak3918av100/ak_types.h> 


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
void wdt_enable(void);

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
void wdt_keepalive(unsigned int heartbeat);

/**
 * @brief disable watch_dog timer for system reset function.
 *
 * disable watch_dog timer for system reset function.
 * @author CaoDonghua
 * @date 2016-12-29
 * @param[in] void none
 * @return void none
 * @retval  none
 */
void wdt_disable(void);


#endif
