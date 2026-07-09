// SPDX-License-Identifier: GPL-2.0+
/*
 * Anyka37E clock driver's resource headfile
 *
 */

#ifndef __CLK_AK37E_H__
#define __CLK_AK37E_H__

#include <asm/arch-ak37e/ak_cpu.h>
#include <dt-bindings/clock/ak37e-clock.h>

#define CHIP_CONF_BASE_ADDR      0x08000000
#define LCD_CONTROLLER_BASE_ADDR 0x20010000

#define CLOCK_CPU_PLL_CTRL          (CHIP_CONF_BASE_ADDR + 0x04)
#define CLOCK_ASIC_PLL_CTRL         (CHIP_CONF_BASE_ADDR + 0x08)
#define CLOCK_SAR_ADC_CTRL          (CHIP_CONF_BASE_ADDR + 0x0C)
#define CLOCK_SD_ADC_DAC_HS_CTRL    (CHIP_CONF_BASE_ADDR + 0x10)
#define CLOCK_IMAGE_CAPTURE_CTRL    (CHIP_CONF_BASE_ADDR + 0x14)
#define CLOCK_MAC_OPCLK_CTRL        (CHIP_CONF_BASE_ADDR + 0x18)
#define CLOCK_GATE_CTRL             (CHIP_CONF_BASE_ADDR + 0x1C)
#define CLOCK_SOFT_RESET            (CHIP_CONF_BASE_ADDR + 0x20)
#define CLOCK_GATE_SOFT_RESET_CTRL  (CHIP_CONF_BASE_ADDR + 0xFC)
#define CLOCK_USB_I2S_CTRL          (CHIP_CONF_BASE_ADDR + 0x58)
#define CLOCK_DAC_FADEOUT_CTRL      (CHIP_CONF_BASE_ADDR + 0x70)
#define CLOCK_SPI_HS_CTRL           (CHIP_CONF_BASE_ADDR + 0x100)
#define CLOCK_AUDIO_PLL_CTRL        (CHIP_CONF_BASE_ADDR + 0x1B8)
#define CLOCK_SD_ADC_DAC_AUDIO_CTRL (CHIP_CONF_BASE_ADDR + 0x1BC)
#define CLOCK_LCD_PCLK_CTRL         (LCD_CONTROLLER_BASE_ADDR + 0xBC)

/*
 * CLOCK_CPU_PLL_CTRL
 */
#define CPU_PLL_DPHYCLK_1X2X_CFG_MASK  (0x1)
#define CPU_PLL_DPHYCLK_1X2X_CFG_SHIFT (30)
#define ASIC_PLL_FREQ_ADJ_CFG_MASK     (0x1)
#define ASIC_PLL_FREQ_ADJ_CFG_SHIFT    (28)
#define CPU_PLL_ST_MODE_SHIFT          (27)
#define CPU_PLL_CPU_5X_SEL_CFG_MASK    (0x1)
#define CPU_PLL_CPU_5X_SEL_CFG_SHIFT   (26)
#define CPU_PLL_CLK_5X_EN_CFG_SHIFT    (25)
#define CPU_PLL_EVEN_DIV_VLD_CFG_SHIFT (17)
#define CPU_PLL_EVEN_DIV_NUM_CFG_MASK  (0x3)
#define CPU_PLL_EVEN_DIV_NUM_CFG_SHIFT (15)
#define CPU_PLL_FREQ_ADJ_CFG_SHIFT     (14)
#define CPU_PLL_OD_CFG_MASK            (0x3)
#define CPU_PLL_OD_CFG_SHIFT           (12)
#define CPU_PLL_N_CFG_MASK             (0xF)
#define CPU_PLL_N_CFG_SHIFT            (8)
#define CPU_PLL_M_CFG_MASK             (0xFF)
#define CPU_PLL_M_CFG_SHIFT            (0)

/*
 * CLK_ASIC_PLL_CTRL
 */
#define CLOCK_ASIC_GCLK_SEL_MASK          (0x1)
#define CLOCK_ASIC_GCLK_SEL_SHIFT         (24)
#define CLOCK_ASIC_VCLK_DIV_VLD_CFG_SHIFT (23)
#define CLOCK_ASIC_VCLK_DIV_NUM_CFG_MASK  (0x3)
#define CLOCK_ASIC_VCLK_DIV_NUM_CFG_SHIFT (17)
#define CLOCK_ASIC_PLL_OD_CFG_MASK        (0x3)
#define CLOCK_ASIC_PLL_OD_CFG_SHIFT       (12)
#define CLOCK_ASIC_PLL_N_CFG_MASK         (0xF)
#define CLOCK_ASIC_PLL_N_CFG_SHIFT        (8)
#define CLOCK_ASIC_PLL_M_CFG_MASK         (0xFF)
#define CLOCK_ASIC_PLL_M_CFG_SHIFT        (0)

/*
 * CLOCK_IMAGE_CAPTURE_CTRL
 * Image Capture Related Register @0x0800,0014
 */
#define CLOCK_CSI_DVP_SENSOR_SEL_CFG_SHIFT   (31)
#define CLOCK_CSI_DUAL_SENSOR_MODE_CFG_SHIFT (29)
#define CLOCK_CSI1_SCLK_SEL_CFG_SHIFT        (27)
#define CLOCK_CSI_PRST_CFG_SHIFT             (25)
#define CLOCK_CSI0_SCLK_SEL_CFG_SHIFT        (24)
#define CLOCK_MAC0_SPEED_CFG_SHIFT           (23)
#define CLOCK_MAC0_INTF_SEL_CFG_SHIFT        (22)
#define CLOCK_MAC1_SPEED_CFG_SHIFT           (21)
#define CLOCK_MAC1_INTF_SEL_CFG_SHIFT        (20)
#define CLOCK_IMAGE_CAPTURE_CTRL_CFG_MASK    (0x1)

/*
 * CLOCK_MAC_OPCLK_CTRL
 * MAC0_OPCLK/MAC1_OPCLK Register @0x0800,0018
 */
#define CLOCK_CSI1_PCLK_POL_SHIFT          (31)
#define CLOCK_CSI0_PCLK_POL_SHIFT          (30)
#define CLOCK_MAC1_OPCLK_DIV_VLD_CFG_MASK  (0x01)
#define CLOCK_MAC1_OPCLK_DIV_VLD_CFG_SHIFT (29)
#define CLOCK_MAC1_OPCLK_EN_CFG_SHIFT      (28)
#define CLOCK_MAC1_DIV_NUM_CFG_MASK        (0x3F)
#define CLOCK_MAC1_DIV_NUM_CFG_SHIFT       (20)
#define CLOCK_CSI0_SCLK_DIV_VLD_CFG_MASK   (0x01)
#define CLOCK_CSI0_SCLK_DIV_VLD_CFG_SHIFT  (19)
#define CLOCK_CSI0_SCLK_EN_CFG_SHIFT       (18)
#define CLOCK_CSI0_SCLK_DIV_NUM_CFG_MASK   (0x3F)
#define CLOCK_CSI0_SCLK_DIV_NUM_CFG_SHIFT  (10)
#define CLOCK_MAC0_OPCLK_DIV_VLD_CFG_MASK  (0x01)
#define CLOCK_MAC0_OPCLK_DIV_VLD_CFG_SHIFT (9)
#define CLOCK_MAC0_OPCLK_EN_CFG_SHIFT      (8)
#define CLOCK_MAC0_DIV_NUM_CFG_MASK        (0x3F)
#define CLOCK_MAC0_DIV_NUM_CFG_SHIFT       (0)

/*
 * CLOCK_CTRL_REG
 * Clock Gate Control Register @0x0800,001C
 */
#define GCLK_DSI_CONTROLLER_SHIFT     (31)
#define GCLK_DPHY_CLK_GATE_CFG_SHIFT  (25)
#define GCLK_DCLK_GATE_CFG_SHIFT      (24)
#define GCLK_GUI_CFG_SHIFT            (23)
#define GCLK_LCD_CONTROLLER_CFG_SHIFT (22)
#define GCLK_VIDEO_DECODER_CFG_SHIFT  (21)
#define GCLK_JPEG_CODEC_CFG_SHIFT     (20)
#define GCLK_IMAGE_CAPTURE_CFG_SHIFT  (19)
#define GCLK_VCLK_GATE_CFG_SHIFT      (18)
#define GCLK_TWI1_CFG_SHIFT           (17)
#define GCLK_MAC1_CFG_SHIFT           (16)
#define GCLK_USB_CONTROLLER_CFG_SHIFT (15)
#define GCLK_PDM_CFG_SHIFT            (14)
#define GCLK_MAC0_CFG_SHIFT           (13)
#define GCLK_GPIO_CFG_SHIFT           (12)
#define GCLK_L2BUF1_CFG_SHIFT         (11)
#define GCLK_TWI0_CFG_SHIFT           (10)
#define GCLK_L2BUF0_CFG_SHIFT         (9)
#define GCLK_UART1_CFG_SHIFT          (8)
#define GCLK_UART0_CFG_SHIFT          (7)
#define GCLK_SPI1_CFG_SHIFT           (6)
#define GCLK_SPI0_CFG_SHIFT           (5)
#define GCLK_SD_DAC_CFG_SHIFT         (4)
#define GCLK_SD_ADC_CFG_SHIFT         (3)
#define GCLK_MMC1_CFG_SHIFT           (2)
#define GCLK_MMC0_CFG_SHIFT           (1)

/*
 * CLOCK_GATE_SOFT_RESET_CTRL
 * Clock Gate and Software Reset Register @0x0800,00FC
 */
#define CLOCK_PDM_I2SM_CLK_RST_CFG (31)
#define CLOCK_PDM_HS_CLK_RST_CFG   (30)
#define GCLK_TWI3_RST_CFG_SHIFT    (21)
#define GCLK_TWI2_RST_CFG_SHIFT    (20)
#define GCLK_MMC2_RST_CFG_SHIFT    (19)
#define GCLK_SPI2_RST_CFG_SHIFT    (18)
#define GCLK_UART3_RST_CFG_SHIFT   (17)
#define GCLK_UART2_RST_CFG_SHIFT   (16)
#define GCLK_TWI3_CFG_SHIFT        (5)
#define GCLK_TWI2_CFG_SHIFT        (4)
#define GCLK_MMC2_CFG_SHIFT        (3)
#define GCLK_SPI2_CFG_SHIFT        (2)
#define GCLK_UART3_CFG_SHIFT       (1)
#define GCLK_UART2_CFG_SHIFT       (0)

/*
 * CLOCK_USB_I2S_CTRL
 */
#define CLOCK_I2S_MCLK_SEL_SHIFT          (30)
#define CLOCK_I2SM_ADC_MODE_ENABLED_SHIFT (29)
#define CLOCK_I2SM_MODE_ENABLED_SHIFT     (28)
#define CLOCK_I2SSR_MODE_ENABLED_SHIFT    (27)
#define CLOCK_I2SST_MODE_ENABLED_SHIFT    (26)
#define CLOCK_I2SSI_MODE_ENABLED_SHIFT    (25)
#define USB_LINE_STATE_WP_MASK            (0x3)
#define USB_LINE_STATE_WP_SHIFT           (15)
#define USB_DP_PU_EN_SHIFT                (14)
#define USB_ID_CFG_MASK                   (0x3)
#define USB_ID_CFG_SHIFT                  (12)
#define USB_PHY_CFG_MASK                  (0x3F)
#define USB_PHY_CFG_SHIFT                 (6)
#define USB_PHY_PON_RST_SHIFT             (3)
#define USB_SUS_CFG_SHIFT                 (2)
#define CLOCK_USB_PHY_PLL_EN_SHIFT        (1)
#define CLOCK_USB_PHY_RST_SHIFT           (0)

/*
 * CLOCK_SPI_HS_CTRL
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

/*
 * CLOCK_SOFT_RESET
 * Software Reset Control @0x0800,0020
 */
#define CLOCK_MIPI_DIS_RST_CFG_SHIFT      (31)
#define CLOCK_SARADC_RST_CFG_SHIFT        (30)
#define CLOCK_SDADC_HSRST_CFG_SHIFT       (29)
#define CLOCK_SDDAC_HSRST_CFG_SHIFT       (28)
#define CLOCK_SDADC_RST_CFG_SHIFT         (27)
#define CLOCK_SDDAC_RST_CFG_SHIFT         (26)
#define CLOCK_DPHYCLK_RST_CFG_SHIFT       (25)
#define CLOCK_DCLK_RST_CFG_SHIFT          (24)
#define VCLK_GUI_RST_CFG_SHIFT            (23)
#define VCLK_LCD_CONTROLLER_RST_CFG_SHIFT (22)
#define VCLK_VIDEO_DECODER_RST_CFG_SHIFT  (21)
#define VCLK_JPEG_CODEC_RST_CFG_SHIFT     (20)
#define VCLK_IMAGE_CAPTURE_RST_CFG_SHIFT  (19)
#define VCLK_RST_CFG_SHIFT                (18)
#define GCLK_TWI1_RST_CFG_SHIFT           (17)
#define GCLK_MAC1_RST_CFG_SHIFT           (16)
#define GCLK_USB_RST_CFG_SHIFT            (15)
#define GCLK_PDM_CONTROLLER_RST_CFG_SHIFT (14)
#define GCLK_MAC0_BUFFER_RST_CFG_SHIFT    (13)
#define GCLK_GPIO_RST_CFG_SHIFT           (12)
#define GCLK_L2BUF1_RST_CFG_SHIFT         (11)
#define GCLK_TWI0_RST_CFG_SHIFT           (10)
#define GCLK_L2BUF0_RST_CFG_SHIFT         (9)
#define GCLK_UART1_RST_CFG_SHIFT          (8)
#define GCLK_UART0_RST_CFG_SHIFT          (7)
#define GCLK_SPI1_RST_CFG_SHIFT           (6)
#define GCLK_SPI0_RST_CFG_SHIFT           (5)
#define GCLK_SDDAC_RST_CFG_SHIFT          (4)
#define GCLK_SDADC_RST_CFG_SHIFT          (3)
#define GCLK_MMC1_RST_CFG_SHIFT           (2)
#define GCLK_MMC0_RST_CFG_SHIFT           (1)
#define GCLK_RST_CFG_SHIFT                (0)

/*
 * CLOCK_LCD_PCLK_CTRL
 */
#define CLOCK_LCD_PCLK_EN_CFG_SHIFT      (8)
#define CLOCK_LCD_PCLK_DIV_NUM_CFG_MASK  (0x7F)
#define CLOCK_LCD_PCLK_DIV_NUM_CFG_SHIFT (1)
#define CLOCK_LCD_PCLK_DIV_VLD_CFG_SHIFT (0)

/*
 * clock max rate
 */
#define PLL1_MAX_RATE                   (800000000)
#define JCLK_MAX_RATE                   (800000000)
#define HCLK_MAX_RATE                   (440000000)
#define DPHY_CLK_MAX_RATE               (440000000)
#define ASIC_PLL_MAX_RATE               (500000000)
#define VCLK_MAX_RATE                   (250000000)
#define GCLK_MAX_RATE                   (125000000)

#define ADCHS_CLK_MAX_RATE              (100000000)
#define DACHS_CLK_MAX_RATE              (100000000)
#define SPI0_BUS_CLK_MAX_RATE           (250000000)
#define CSI_SCLK0_MAX_RATE              (100000000)
#define CSI_SCLK1_MAX_RATE              (100000000)
#define DSI_PCLK_MAX_RATE               (125000000)
#define OPCLK_MAX_RATE                  (100000000)
#define OPCLK1_MAX_RATE                 (100000000)
#define SDADC_CLK_MAX_RATE              (25000000)
#define SDDAC_CLK_MAX_RATE              (25000000)
#define PDMHS_CLK_MAX_RATE              (100000000)
#define PDMI2CS_CLK_MAX_RATE            (50000000)

#endif //__CLK_AK37E_H__
