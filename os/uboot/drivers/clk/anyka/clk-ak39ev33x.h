// SPDX-License-Identifier: GPL-2.0+
/*
 * Anyka39EV33X clock driver's resource headfile
 *
 */

#ifndef __CLK_AK39EV33X_H__
#define __CLK_AK39EV33X_H__

#include <dt-bindings/clock/ak39ev33x-clock.h>

#define CHIP_CONF_BASE_ADDR                         0x08000000
#define LCD_CONTROLLER_BASE_ADDR                    0x20010000

#define CLOCK_CPU_PLL_CTRL                          (CHIP_CONF_BASE_ADDR + 0x04)
#define CLOCK_CORE_PLL_CTRL                         (CHIP_CONF_BASE_ADDR + 0x08)
#define CLOCK_ADC2_DAC_CTRL                         (CHIP_CONF_BASE_ADDR + 0x0C)
#define CLOCK_ADC2_DAC_HS_CTRL                      (CHIP_CONF_BASE_ADDR + 0x10)
#define CLOCK_PERI_PLL_CTRL1                        (CHIP_CONF_BASE_ADDR + 0x14)
#define CLOCK_PERI_PLL_CTRL2                        (CHIP_CONF_BASE_ADDR + 0x18)
#define CLOCK_GATE_CTRL1                            (CHIP_CONF_BASE_ADDR + 0x1C)
#define CLOCK_SOFT_RESET                            (CHIP_CONF_BASE_ADDR + 0x20)
#define CLOCK_USB_I2S_CTRL                          (CHIP_CONF_BASE_ADDR + 0x58)

/*
* CLOCK_CORE_PLL_CTRL @0x0800,0008
*/
#define CLOCK_CORE_PLL_OD_CFG_MASK                  (0x3)
#define CLOCK_CORE_PLL_OD_CFG_SHIFT                 (12)
#define CLOCK_CORE_PLL_N_CFG_MASK                   (0xF)
#define CLOCK_CORE_PLL_N_CFG_SHIFT                  (8)
#define CLOCK_CORE_PLL_M_CFG_MASK                   (0xFF)
#define CLOCK_CORE_PLL_M_CFG_SHIFT                  (0)

/*
* CLOCK_PERI_PLL_CTRL1
*/
#define CLOCK_PERI_PLL_OD_CFG_MASK                  (0x3)
#define CLOCK_PERI_PLL_OD_CFG_SHIFT                 (12)
#define CLOCK_PERI_PLL_N_CFG_MASK                   (0xF)
#define CLOCK_PERI_PLL_N_CFG_SHIFT                  (8)
#define CLOCK_PERI_PLL_M_CFG_MASK                   (0xFF)
#define CLOCK_PERI_PLL_M_CFG_SHIFT                  (0)

/*
* CLOCK_GATE_CTRL1
* Clock Gate Control Register @0x0800,001C
*/
#define GCLK_TWI2_CFG_SHIFT                         (31)
#define GCLK_DPHY_CLK_GATE_CFG_SHIFT                (25)
#define GCLK_DRAM_GATE_CFG_SHIFT                    (24)
#define GCLK_VENCODER_CFG_SHIFT                     (20)
#define GCLK_ISP_RELEATED_CFG_SHIFT                 (19)
#define GCLK_TWI1_CFG_SHIFT                         (17)
#define GCLK_MIPI_CFG_SHIFT                         (16)
#define GCLK_USB_CONTROLLER_CFG_SHIFT               (15)
#define GCLK_ENCRYPTION_CFG_SHIFT                   (14)
#define GCLK_MAC_CFG_SHIFT                          (13)
#define GCLK_GPIO_CFG_SHIFT                         (12)
#define GCLK_TWI0_CFG_SHIFT                         (10)
#define GCLK_L2BUF_CFG_SHIFT                        (9)
#define GCLK_UART1_CFG_SHIFT                        (8)
#define GCLK_UART0_CFG_SHIFT                        (7)
#define GCLK_SPI1_CFG_SHIFT                         (6)
#define GCLK_SPI0_CFG_SHIFT                         (5)
#define GCLK_SD_DAC_CFG_SHIFT                       (4)
#define GCLK_SD_ADC_CFG_SHIFT                       (3)
#define GCLK_MMC1_CFG_SHIFT                         (2)
#define GCLK_MMC0_CFG_SHIFT                         (1)

#endif //__CLK_AK39EV33X_H__
