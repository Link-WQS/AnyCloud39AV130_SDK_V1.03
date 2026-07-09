/*
 * Copyright (C) 2018-2022 AICSemi Ltd.
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
#include "dbg.h"

#ifdef CFG_TEST_TINYUSB

/*
 * MACROS
 ****************************************************************************************
 */
#define UART_PRINT              dbg


/*
 * FUNCTIONS
 ****************************************************************************************
 */

#ifdef CFG_TUSB_DEMO_CDC_DUAL_PORTS
extern void tusb_cdc_dual_ports_demo(void);
#define TUSB_DEMO_FUNC()    tusb_cdc_dual_ports_demo()
#endif

/**
 ****************************************************************************************
 * @brief test task implementation.
 ****************************************************************************************
 */
void tinyusb_test(void)
{
    UART_PRINT("\nTINYUSB test start\n");

    TUSB_DEMO_FUNC();

    UART_PRINT("\nTINYUSB test done\n");
}

#endif /* CFG_TEST_TINYUSB */
