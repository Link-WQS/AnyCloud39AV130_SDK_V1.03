// SPDX-License-Identifier: GPL-2.0+
/*
 * Anyka37D clock driver's resource headfile
 *
 */

#ifndef __CLK_AK37D_H__
#define __CLK_AK37D_H__

#include <dt-bindings/clock/ak37d-clock.h>

#define CHIP_CONF_BASE_ADDR      0x08000000
#define LCD_CONTROLLER_BASE_ADDR 0x20010000

#define CLOCK_CPU_PLL_CTRL        (CHIP_CONF_BASE_ADDR + 0x04)
#define CLOCK_CORE_PLL_CTRL       (CHIP_CONF_BASE_ADDR + 0x08)
#define CLOCK_ADC2_DAC_CTRL       (CHIP_CONF_BASE_ADDR + 0x0C)
#define CLOCK_ADC2_DAC_HS_CTRL    (CHIP_CONF_BASE_ADDR + 0x10)
#define CLOCK_PERI_PLL_CTRL1      (CHIP_CONF_BASE_ADDR + 0x14)
#define CLOCK_PERI_PLL_CTRL2      (CHIP_CONF_BASE_ADDR + 0x18)
#define CLOCK_GATE_CTRL1          (CHIP_CONF_BASE_ADDR + 0x1C)
#define CLOCK_SOFT_RESET          (CHIP_CONF_BASE_ADDR + 0x20)
#define CLOCK_GATE_RESET_CTRL2    (CHIP_CONF_BASE_ADDR + 0xFC)
#define CLOCK_USB_I2S_CTRL        (CHIP_CONF_BASE_ADDR + 0x58)
#define CLOCK_DAC_FADEOUT_CTRL    (CHIP_CONF_BASE_ADDR + 0x70)
#define CLOCK_SPI_HIGH_SPEED_CTRL (CHIP_CONF_BASE_ADDR + 0x100)
#define CLOCK_AUDIO_PLL_CTRL      (CHIP_CONF_BASE_ADDR + 0x1B8)
#define CLOCK_SD_ADC_DAC_CTRL     (CHIP_CONF_BASE_ADDR + 0x1BC)

/*
 * CLOCK_CORE_PLL_CTRL
 */
#define CLOCK_CORE_PLL_OD_CFG_MASK  (0x3)
#define CLOCK_CORE_PLL_OD_CFG_SHIFT (12)
#define CLOCK_CORE_PLL_N_CFG_MASK   (0xF)
#define CLOCK_CORE_PLL_N_CFG_SHIFT  (8)
#define CLOCK_CORE_PLL_M_CFG_MASK   (0xFF)
#define CLOCK_CORE_PLL_M_CFG_SHIFT  (0)

/*
 * CLOCK_PERI_PLL_CTRL1
 */
#define CLOCK_PERI_PLL_LCD_PCLK_SEL_SHIFT   (30)
#define CLOCK_PERI_PLL_MAC_RESERVED2_SHIFT  (28)
#define CLOCK_PERI_PLL_MAC_SPEED_SHIFT      (23)
#define CLOCK_PERI_PLL_MAC_RELATE_CLK_SHIFT (22)
#define CLOCK_PERI_PLL_CLK50M_EN_SHIFT      (21)
#define CLOCK_PERI_PLL_USB_PHY_SEL_SHIFT    (19)
#define CLOCK_PERI_PLL_MAC_RESERVED1_SHIFT  (18)
#define CLOCK_PERI_PLL_CLK12M_SHIFT         (17)
#define CLOCK_PERI_PLL_OD_CFG_MASK          (0x3)
#define CLOCK_PERI_PLL_OD_CFG_SHIFT         (12)
#define CLOCK_PERI_PLL_N_CFG_MASK           (0xF)
#define CLOCK_PERI_PLL_N_CFG_SHIFT          (8)
#define CLOCK_PERI_PLL_M_CFG_MASK           (0xFF)
#define CLOCK_PERI_PLL_M_CFG_SHIFT          (0)

/*
 * CLOCK_GATE_CTRL1
 * Clock Gate Control Register @0x0800,001C
 */
#define GCLK_DPHY_CLK_GATE_CFG_SHIFT (25)
#define GCLK_DCLK_CFG_SHIFT          (24)
#define GCLK_GUI_CFG_SHIFT           (23)
#define GCLK_LCD_CFG_SHIFT           (22)
#define GCLK_VDECODER_CFG_SHIFT      (21)
#define GCLK_VENCODER_CFG_SHIFT      (20)
#define GCLK_ISP_CFG_SHIFT           (19)
#define GCLK_TWI1_CFG_SHIFT          (17)
#define GCLK_MIPI_CFG_SHIFT          (16)
#define GCLK_USB_CFG_SHIFT           (15)
#define GCLK_ENCRYPTION_CFG_SHIFT    (14)
#define GCLK_MAC_CFG_SHIFT           (13)
#define GCLK_GPIO_CFG_SHIFT          (12)
#define GCLK_TWI0_CFG_SHIFT          (10)
#define GCLK_L2BUF_CFG_SHIFT         (9)
#define GCLK_UART1_CFG_SHIFT         (8)
#define GCLK_UART0_CFG_SHIFT         (7)
#define GCLK_SPI1_CFG_SHIFT          (6)
#define GCLK_SPI0_CFG_SHIFT          (5)
#define GCLK_SD_DAC_CFG_SHIFT        (4)
#define GCLK_SD_ADC_CFG_SHIFT        (3)
#define GCLK_MMC1_CFG_SHIFT          (2)
#define GCLK_MMC0_CFG_SHIFT          (1)

/*
 * CLOCK_GATE_RESET_CTRL2
 * Clock Gate Control Register @0x0800,00FC
 */
#define GCLK_TWI3_RST_SHIFT  (21)
#define GCLK_TWI2_RST_SHIFT  (20)
#define GCLK_MMC2_RST_SHIFT  (19)
#define GCLK_DSI_RST_SHIFT   (18)
#define GCLK_UART3_RST_SHIFT (17)
#define GCLK_UART2_RST_SHIFT (16)
#define GCLK_TWI3_CFG_SHIFT  (5)
#define GCLK_TWI2_CFG_SHIFT  (4)
#define GCLK_MMC2_CFG_SHIFT  (3)
#define GCLK_DSI_CFG_SHIFT   (2)
#define GCLK_UART3_CFG_SHIFT (1)
#define GCLK_UART2_CFG_SHIFT (0)

/*
 * CLOCK_SOFT_RESET
 * Software Reset Control @0x0800,0020
 */
#define GCLK_GUI_RST_SHIFT        (23)
#define GCLK_LCD_RST_SHIFT        (22)
#define GCLK_VDECODER_RST_SHIFT   (21)
#define GCLK_VENCODER_RST_SHIFT   (20)
#define GCLK_ISP_RST_SHIFT        (19)
#define GCLK_TWI1_RST_SHIFT       (17)
#define GCLK_MIPI_RST_SHIFT       (16)
#define GCLK_USB_RST_SHIFT        (15)
#define GCLK_ENCRYPTION_RST_SHIFT (14)
#define GCLK_MAC_RST_SHIFT        (13)
#define GCLK_GPIO_RST_SHIFT       (12)
#define GCLK_TWI0_RST_SHIFT       (10)
#define GCLK_L2BUF_RST_SHIFT      (9)
#define GCLK_UART1_RST_SHIFT      (8)
#define GCLK_UART0_RST_SHIFT      (7)
#define GCLK_SPI1_RST_SHIFT       (6)
#define GCLK_SPI0_RST_SHIFT       (5)
#define GCLK_SD_DAC_RST_SHIFT     (4)
#define GCLK_SD_ADC_RST_SHIFT     (3)
#define GCLK_MMC1_RST_SHIFT       (2)
#define GCLK_MMC0_RST_SHIFT       (1)
#define GCLK_RST_CFG_SHIFT        (0)

/*
 * CLOCK_SPI_HIGH_SPEED_CTRL
 */
#define CLOCK_SPI0_HSCLK_RST_CFG_SHIFT    (31)
#define CLOCK_LCD_PCLK_RST_CFG_SHIFT      (30)
#define CLOCK_CSI1_SCLK_DIV_VLD_CFG_MASK  (0x01)
#define CLOCK_CSI1_SCLK_DIV_VLD_CFG_SHIFT (19)
#define CLOCK_CSI1_SCLK_EN_CFG_SHIFT      (18)
#define CLOCK_CSI1_SCLK_DIV_NUM_CFG_MASK  (0x3F)
#define CLOCK_CSI1_SCLK_DIV_NUM_CFG_SHIFT (10)
#define CLOCK_SPI_HSDIV_VLD_CFG_MASK      (0x01)
#define CLOCK_SPI_HSDIV_VLD_CFG_SHIFT     (9)
#define CLOCK_SPI_HSCLK_EN_CFG_SHIFT      (8)
#define CLOCK_SPI_HSCLK_DIV_NUM_CFG_MASK  (0xFF)
#define CLOCK_SPI_HSCLK_DIV_NUM_CFG_SHIFT (0)

#endif //__CLK_AK37D_H__
