/**
 ****************************************************************************************
 *
 * @file dbg_task.c
 *
 * @brief Defines the different debug states and messages handlers used by the debug task
 *
 * Copyright (C) RivieraWaves 2011-2019
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup ASSERT
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "dbg_assert.h"
#include "dbg.h"
#include "plf.h"
#include "arch.h"
#if PLF_CONSOLE
#include "console.h"
#endif
#if (PLF_PMIC)
#include "pmic_api.h"
#endif
#include "wdt_api.h"
#ifdef CFG_RTOS
#include "rtos_al.h"
#endif

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
int dbg_assert_block = 1;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
#if 0
void dbg_assert_rec(const char *condition, const char * file, int line)
{
    // Disable the interrupts
    GLOBAL_INT_DISABLE();

    // Display a trace message showing the error
    TRACE("ASSERT recovery: %F:%d", (line >> 24) , line);
    line = (line & 0xffffff);
    dbg(D_ERR "ASSERT (%s) at %s:%d\n", condition, file, line);

    // Restore the interrupts
    GLOBAL_INT_RESTORE();
}
#endif

void dbg_assert_err(const char *condition, const char * file, int line)
{
    unsigned int delay_ticks;
    TRACE("ASSERT error: %F:%d", (line >> 24) , line);
    line = (line & 0xffffff);

    // Stop the interrupts
    GLOBAL_INT_STOP();

    // Display a trace message showing the error
    dbg(D_ERR "ASSERT (%s) at %s:%d\n", condition, file, line);
    #if defined(CFG_RTOS) && (configUSE_TRACE_FACILITY == 1)
    rtos_task_info();
    #endif
    #if PLF_M2D_BLE || PLF_M2D_OTA
    delay_ticks = 0xF;
    #else
    delay_ticks = wdt_get_pmic_reboot_delay();
    #endif
    if (delay_ticks) {
        #if (PLF_PMIC)
        dbg("pmic_reboot\n");
        pmic_chip_reboot(delay_ticks);
        #endif
    }
    #if PLF_CONSOLE
    while (1) {
        if (stdio_uart_tstc()) {
            char ch = stdio_uart_getc();
            console_handle_char(ch);
            console_schedule();
        }
    }
    #else
    while(dbg_assert_block);
    #endif
}

void dbg_assert_warn(const char *condition, const char * file, int line)
{
    TRACE("ASSERT warning: %F:%d", (line >> 24) , line);
    line = (line & 0xffffff);
    dbg(D_ERR "WARNING (%s) at %s:%d\n", condition, file, line);
}


/// @}  // end of group ASSERT
