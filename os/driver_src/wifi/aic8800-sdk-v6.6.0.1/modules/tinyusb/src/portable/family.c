/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This file is part of the TinyUSB stack.
 */


#include "bsp/board_api.h"
#include "board.h"
#include "dbg.h"
#include "sysctrl_api.h"
#include "reg_anareg.h"
#if TU_CHECK_MCU(OPT_MCU_AIC8800)
#if PLF_PMIC
#include "pmic_api.h"
#endif
#endif

//--------------------------------------------------------------------+
// Forward USB interrupt events to TinyUSB IRQ Handler
//--------------------------------------------------------------------+

void USBDMA_IRQHandler(void)
{
    #if CFG_TUD_ENABLED
    tud_int_handler(0);
    #endif
    #if CFG_TUH_ENABLED
    tuh_int_handler(0);
    #endif
}

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM
//--------------------------------------------------------------------+

#define USB_NO_VBUS_PIN

void usb_powerup(void)
{
    uint32_t reg_val;
    #if TU_CHECK_MCU(OPT_MCU_AIC8800)
    (void)reg_val;
    #if PLF_PMIC && PLF_PMIC_VER_LITE
    // power up usb & usb_pll
    PMIC_MEM_MASK_WRITE((unsigned int)(&aic1000liteAnalogReg->por_ctrl),
        (AIC1000LITE_ANALOG_REG_CFG_RTC_USB_PLL_PU | AIC1000LITE_ANALOG_REG_CFG_RTC_USB_PU |
        AIC1000LITE_ANALOG_REG_CFG_RTC_USBPLL_CLK_EN),
        (AIC1000LITE_ANALOG_REG_CFG_RTC_USB_PLL_PU | AIC1000LITE_ANALOG_REG_CFG_RTC_USB_PU |
        AIC1000LITE_ANALOG_REG_CFG_RTC_USBPLL_CLK_EN));
    #endif /* PLF_PMIC && PLF_PMIC_VER_LITE */
    // power up mmsys
    pwrctrl_mmsys_set(PWRCTRL_POWERUP);
    // clk en
    cpusysctrl_hclkme_set(CSC_HCLKME_USBC_EN_BIT);
    cpusysctrl_oclkme_set(CSC_OCLKME_ULPI_EN_BIT);
    if (cpusysctrl_ulpics_get()) {
        /* ULPI clock is stable, release ULPI/USBC RESETn */
        dbg("ULPI clk detected\r\n");
        cpusysctrl_oclkrs_ulpiset_setb(); // RESETn set
        cpusysctrl_hclkrs_usbcset_setb();
        cpusysctrl_oclkrc_ulpiclr_setb(); // RESETn clr
        cpusysctrl_hclkrc_usbcclr_setb();
    } else {
        dbg("err!! no usb ulpi clk\r\n");
        return;
    }

    #elif TU_CHECK_MCU(OPT_MCU_AIC8800MC, OPT_MCU_AIC8800M40)
    // power up mmsys
    pwrctrl_mmsys_set(PWRCTRL_POWERUP);

    #if TU_CHECK_MCU(OPT_MCU_AIC8800MC)
    //clear bit14 in usb_cfg0
    reg_val = AIC_MMSYSCTRL->usb_cfg0;
    AIC_MMSYSCTRL->usb_cfg0 = reg_val & ~(1<<14);

    #elif TU_CHECK_MCU(OPT_MCU_AIC8800M40)
    // for eye diagram test
    reg_val = AIC_MMSYSCTRL->usb_ana_cfg0 & ~(MM_SYS_CTRL_RG_USB_HS_IDAC_BIT(0x0F));
    AIC_MMSYSCTRL->usb_ana_cfg0 = reg_val | MM_SYS_CTRL_RG_USB_HS_IDAC_BIT(0x0C);

    //for usb host disconnect/connect detect
    reg_val = AIC_MMSYSCTRL->usb_ana_cfg0 & ~(MM_SYS_CTRL_RG_USB_ISET_HS_DISCONNECT(0x07));
    AIC_MMSYSCTRL->usb_ana_cfg0 = reg_val | MM_SYS_CTRL_RG_USB_ISET_HS_DISCONNECT(0x05);
    #endif

    cpusysctrl_oclkrs_usbcset_setb(); // RESETn set
    cpusysctrl_oclkrc_usbcclr_setb(); // RESETn clr
    //dbg("hw anareg\r\n");
    //1 hw anareg
    AIC_ANAREG1->ana_port_ctrl |= ANALOG_REG1_RF_CFG_ANA_PU_IREF_IO;
    dbg("pu usb\r\n");
    //2 pu usb
    AIC_MMSYSCTRL->cfg_pu_usb |=
        ((MM_SYS_CTRL_RG_PU_USB | MM_SYS_CTRL_RG_PU_USB_DR) |
        (MM_SYS_CTRL_RG_PU_USB_ANA | MM_SYS_CTRL_RG_PU_USB_ANA_DR));
    //dbg("wait\r\n");
    //3 wait
    while ((AIC_MMSYSCTRL->usb_status & MM_SYS_CTRL_RO_USB_READY) == 0);
    dbg("%s  wait done\n", __func__);
    #else
    #error "not support, TBD"
    #endif
}

void board_init(void) {
  /* Disable interrupts during init */
  //__disable_irq();

  usb_powerup();

  /* Enable interrupts globally */
  //__enable_irq();
}

//--------------------------------------------------------------------+
// Board porting API
//--------------------------------------------------------------------+

void board_led_write(bool state) {
  (void)state;
}

uint32_t board_button_read(void) {
  return 0;//BUTTON_STATE_ACTIVE == gpio_input_bit_get(BUTTON_PORT, BUTTON_PIN);
}

int board_uart_read(uint8_t* buf, int len) {
#if defined(UART_DEV)
  int rxsize = len;
  while (rxsize--) {
    *(uint8_t*)buf = usart_read(UART_DEV);
    buf++;
  }
  return len;
#else
  (void)buf;
  (void)len;
  return 0;
#endif
}

int board_uart_write(void const* buf, int len) {
#if defined(UART_DEV)
  int txsize = len;
  while (txsize--) {
    usart_write(UART_DEV, *(uint8_t const*)buf);
    buf++;
  }
  return len;
#else
  (void)buf;
  (void)len;
  return 0;
#endif
}

#if CFG_TUSB_OS == OPT_OS_NONE
volatile uint32_t system_ticks = 0;
void eclic_mtip_handler(void) {
  system_ticks++;
  SysTick_Reload(TIMER_TICKS);
}
uint32_t board_millis(void) { return system_ticks; }
#endif

#if 0//def USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(char* file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line)
   */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
