// SPDX-License-Identifier: GPL-2.0+
/**
 * @brief ak3918ev300l clock driver's resource headfile
 *
 */

#ifndef __CLK_AK3918EV300L_H__
#define __CLK_AK3918EV300L_H__

#include <asm/arch-ak3918ev300l/ak_cpu.h>
#include <dt-bindings/clock/ak3918ev300l-clock.h>

#define CHIP_CONF_BASE_ADDR 0x08000000

/*
 * Register Map
 */
#define CLOCK_PLL1_CTRL           (CHIP_CONF_BASE_ADDR + 0x04)
#define CLOCK_PLL2_CTRL           (CHIP_CONF_BASE_ADDR + 0x08)
#define CLOCK_SAR_ADC_CTRL        (CHIP_CONF_BASE_ADDR + 0x0C)
#define CLOCK_SD_ADC_DAC_HS_CTRL  (CHIP_CONF_BASE_ADDR + 0x10)
#define CLOCK_PLL3_CTRL           (CHIP_CONF_BASE_ADDR + 0x14)
#define CLOCK_MAC_OPCLK_CTRL      (CHIP_CONF_BASE_ADDR + 0x18)
#define CLOCK_GATE_CTRL           (CHIP_CONF_BASE_ADDR + 0x1C)
#define CLOCK_USB_I2S_CTRL        (CHIP_CONF_BASE_ADDR + 0x58)
#define CLOCK_PLL1_CONFIG         (CHIP_CONF_BASE_ADDR + 0xDC)
#define CLOCK_PLL2_PLL3_CONFIG    (CHIP_CONF_BASE_ADDR + 0x1E0)
#define CLOCK_VCLK_SOURCE_CONFIG  (CHIP_CONF_BASE_ADDR + 0x11C)
#define CLOCK_RESET_CTRL          (CHIP_CONF_BASE_ADDR + 0x20)
#define CLOCK_PLL1_PLL2_BANDWIDTH (CHIP_CONF_BASE_ADDR + 0xE0)

/*
 * PLL1 Channel Clock Control @ 0x08000004
 */
#define CPU_1X_2X_SEL_CFG_MASK  (0x1)
#define CPU_1X_2X_SEL_CFG_SHIFT (31)
#define PLL1_FREQ_ADJ_CFG_MASK  (0x1)
#define PLL1_FREQ_ADJ_CFG_SHIFT (30)
#define PLL3_FREQ_ADJ_CFG_MASK  (0x1)
#define PLL3_FREQ_ADJ_CFG_SHIFT (29)
#define PLL2_FREQ_ADJ_CFG_MASK  (0x1)
#define PLL2_FREQ_ADJ_CFG_SHIFT (28)
#define ST_MODE_MASK            (0x1)
#define ST_MODE_SHIFT           (27)
#define PLL1_CLKOD_CFG_MASK     (0xF)
#define PLL1_CLKOD_CFG_SHIFT    (19)
#define PLL1_CLKR_CFG_MASK      (0x3F)
#define PLL1_CLKR_CFG_SHIFT     (13)
#define PLL1_CLKF_CFG_MASK      (0x1FFF)
#define PLL1_CLKF_CFG_SHIFT     (0)

/*
 * PLL2 Channel Clock Control @ 0x08000008
 */
#define GCLK_DIV_VLD_CFG_MASK  (0x1)
#define GCLK_DIV_VLD_CFG_SHIFT (25)
#define GCLK_DIV_NUM_CFG_MASK  (0x3)
#define GCLK_DIV_NUM_CFG_SHIFT (23)
#define PLL2_CLKOD_CFG_MASK    (0xF)
#define PLL2_CLKOD_CFG_SHIFT   (19)
#define PLL2_CLKR_CFG_MASK     (0x3F)
#define PLL2_CLKR_CFG_SHIFT    (13)
#define PLL2_CLKF_CFG_MASK     (0x1FFF)
#define PLL2_CLKF_CFG_SHIFT    (0)

/*
 * SD_ADC/SPI0/ SD_DAC Clock Control @ 0x0800000C
 */
#define SDDAC_DIV_VLD_CFG_MASK        (0x1)
#define SDDAC_DIV_VLD_CFG_SHIFT       (29)
#define SDDAC_CLK_EN_CFG_MASK         (0x1)
#define SDDAC_CLK_EN_CFG_SHIFT        (28)
#define SDDAC_CLK_DIV_NUM_CFG_MASK    (0xFF)
#define SDDAC_CLK_DIV_NUM_CFG_SHIFT   (20)
#define SPI0_HSCLK_DIV_VLD_CFG_MASK   (0x1)
#define SPI0_HSCLK_DIV_VLD_CFG_SHIFT  (19)
#define SPI0_HSCLK_EN_CFG_MASK        (0x1)
#define SPI0_HSCLK_EN_CFG_SHIFT       (18)
#define SPI0_HSCLK_DIV_NUM_CFG_MASK   (0xFF)
#define SPI0_HSCLK_DIV_NUM_CFG_SHIFT  (10)
#define DPHY_CFGCLK_DIV_VLD_CFG_MASK  (0x1)
#define DPHY_CFGCLK_DIV_VLD_CFG_SHIFT (9)
#define DPHY_CFGCLK_EN_CFG_MASK       (0x1)
#define DPHY_CFGCLK_EN_CFG_SHIFT      (8)
#define DPHY_CFGCLK_DIV_NUM_CFG_MASK  (0xF)
#define DPHY_CFGCLK_DIV_NUM_CFG_SHIFT (4)
#define SAR_ADC_CLK_EN_CFG_MASK       (0x1)
#define SAR_ADC_CLK_EN_CFG_SHIFT      (3)
#define SAR_ADC_CLK_DIV_NUM_CFG_MASK  (0x7)
#define SAR_ADC_CLK_DIV_NUM_CFG_SHIFT (0)

/*
 * SD_ADC/SD_DAC High Speed Clock Control @ 0x08000010
 */
#define SDADC_HSDIV_VLD_CFG_MASK      (0x1)
#define SDADC_HSDIV_VLD_CFG_SHIFT     (29)
#define SDADC_HSCLK_EN_CFG_MASK       (0x1)
#define SDADC_HSCLK_EN_CFG_SHIFT      (28)
#define SDADC_HSCLK_DIV_NUM_CFG_MASK  (0xFF)
#define SDADC_HSCLK_DIV_NUM_CFG_SHIFT (20)
#define SDDAC_HSDIV_VLD_CFG_MASK      (0x1)
#define SDDAC_HSDIV_VLD_CFG_SHIFT     (19)
#define SDDAC_HSCLK_EN_CFG_MASK       (0x1)
#define SDDAC_HSCLK_EN_CFG_SHIFT      (18)
#define SDDAC_HSCLK_DIV_NUM_CFG_MASK  (0xFF)
#define SDDAC_HSCLK_DIV_NUM_CFG_SHIFT (10)
#define SDADC_DIV_VLD_CFG_MASK        (0x1)
#define SDADC_DIV_VLD_CFG_SHIFT       (9)
#define SDADC_CLK_EN_CFG_MASK         (0x1)
#define SDADC_CLK_EN_CFG_SHIFT        (8)
#define SDADC_CLK_DIV_NUM_CFG_MASK    (0xFF)
#define SDADC_CLK_DIV_NUM_CFG_SHIFT   (0)

/*
 * PLL3 Channel Clock Control @ 0x08000014
 */
#define CSI_PCLK_POL_MASK                 (0x1)
#define CSI_PCLK_POL_SHIFT                (30)
#define ISP_PP_DUAL_SENSOR_NODE_CFG_MASK  (0x1)
#define ISP_PP_DUAL_SENSOR_NODE_CFG_SHIFT (29)
#define CSI_MIMP_DVP_SEL_CFG_MASK         (0x1)
#define CSI_MIMP_DVP_SEL_CFG_SHIFT        (26)
#define CSI_PRST_CFG_MASK                 (0x1)
#define CSI_PRST_CFG_SHIFT                (25)
#define MAC_MEM_ICG_BYPASS_MASK           (0x1)
#define MAC_MEM_ICG_BYPASS_SHIFT          (24)
#define MAC_SPEED_CFG_MASK                (0x1)
#define MAC_SPEED_CFG_SHIFT               (23)
#define PLL3_CLKOD_CFG_MASK               (0xF)
#define PLL3_CLKOD_CFG_SHIFT              (19)
#define PLL3_CLKR_CFG_MASK                (0x3F)
#define PLL3_CLKR_CFG_SHIFT               (13)
#define PLL3_CLKF_CFG_MASK                (0x1FFF)
#define PLL3_CLKF_CFG_SHIFT               (0)

/*
 * CSI1_SCLK/CSI2_SCLK/MAC_OPCLK Clock Control @ 0x08000018
 */
#define CSI2_SCLK_EN_CFG_MASK       (0x1)
#define CSI2_SCLK_EN_CFG_SHIFT      (29)
#define CSI2_SCLK_SRC_CFG_MASK      (0x7)
#define CSI2_SCLK_SRC_CFG_SHIFT     (26)
#define CSI2_SCLK_DIV_NUM_CFG_MASK  (0x3F)
#define CSI2_SCLK_DIV_NUM_CFG_SHIFT (20)
#define CSI1_SCLK_EN_CFG_MASK       (0x1)
#define CSI1_SCLK_EN_CFG_SHIFT      (19)
#define CSI1_SCLK_SRC_CFG_MASK      (0x7)
#define CSI1_SCLK_SRC_CFG_SHIFT     (16)
#define CSI1_SCLK_DIV_NUM_CFG_MASK  (0x3F)
#define CSI1_SCLK_DIV_NUM_CFG_SHIFT (10)
#define MAC_OPCLK_DIV_VLD_CFG_MASK  (0x1)
#define MAC_OPCLK_DIV_VLD_CFG_SHIFT (9)
#define MAC_OPCLK_EN_CFG_MASK       (0x1)
#define MAC_OPCLK_EN_CFG_SHIFT      (8)
#define MAC_OPCLK_DIV_NUM_CFG_MASK  (0x3F)
#define MAC_OPCLK_DIV_NUM_CFG_SHIFT (0)

/*
 * Clock Gate Control @ 0x0800001C
 */
#define GCLK_DDR_PHY_CFG_SHIFT    (25)
#define GCLK_DRAM_CFG_SHIFT       (24)
#define GCLK_TWI2_CFG_SHIFT       (22)
#define GCLK_MMC2_CFG_SHIFT       (21)
#define GCLK_SPI2_CFG_SHIFT       (20)
#define GCLK_MIMP1_CFG_SHIFT      (19)
#define GCLK_UART2_CFG_SHIFT      (18)
#define GCLK_TWI1_CFG_SHIFT       (17)
#define GCLK_MIPI0_CFG_SHIFT      (16)
#define GCLK_USB_CFG_SHIFT        (15)
#define GCLK_ENCRYPTION_CFG_SHIFT (14)
#define GCLK_MAC_CFG_SHIFT        (13)
#define GCLK_GPIO_CFG_SHIFT       (12)
#define GCLK_L2BUF1_CFG_SHIFT     (11)
#define GCLK_TWI0_CFG_SHIFT       (10)
#define GCLK_L2BUF0_CFG_SHIFT     (9)
#define GCLK_UART1_CFG_SHIFT      (8)
#define GCLK_UART0_CFG_SHIFT      (7)
#define GCLK_SPI1_CFG_SHIFT       (6)
#define GCLK_SPI0_CFG_SHIFT       (5)
#define GCLK_SD_DAC_CFG_SHIFT     (4)
#define GCLK_SD_ADC_CFG_SHIFT     (3)
#define GCLK_MMC1_CFG_SHIFT       (2)
#define GCLK_MMC0_CFG_SHIFT       (1)

/*
 * USB Operate Mode and I2S Mode Control @ 0x08000058
 */
#define I2S_MCLK_SEL_SHIFT          (30)
#define I2SM_ADC_MODE_ENABLED_SHIFT (29)
#define I2SM_MODE_ENABLED_SHIFT     (28)
#define I2SSR_MODE_ENABLED_SHIFT    (27)
#define I2SST_MODE_ENABLED_SHIFT    (26)
#define I2SSI_MODE_ENABLED_SHIFT    (25)
#define USBPHY_BIST_MODE_SHIFT      (18)
#define OTG_PHY_BIST_OK_SHIFT       (17)
#define USB_LINE_STATE_WP_MASK      (0x3)
#define USB_LINE_STATE_WP_SHIFT     (15)
#define USB_DP_PU_EN_SHIFT          (14)
#define USB_ID_CFG_MASK             (0x3)
#define USB_ID_CFG_SHIFT            (12)
#define USB_PHY_CFG_MASK            (0x3F)
#define USB_PHY_CFG_SHIFT           (6)
#define OTG_TM1_CFG_SHIFT           (4)
#define USB_PHY_PON_RST_SHIFT       (3)
#define USB_SUS_CFG_SHIFT           (2)
#define USB_PHY_PLL_EN_SHIFT        (1)
#define USB_PHY_RST_SHIFT           (0)

/*
 * PLL1 Bandwidth Adjustment Configuration @ 0x080000DC
 */
#define PLL1_BWADJ_CFG_MASK  (0XFFF)
#define PLL1_BWADJ_CFG_SHIFT (20)

/*
 * PLL2/PLL3 Bandwidth Adjustment Configuration @ 0x080000E0
 */
#define PLL3_BWADJ_CFG_MASK  (0XFFF)
#define PLL3_BWADJ_CFG_SHIFT (12)
#define PLL2_BWADJ_CFG_MASK  (0XFFF)
#define PLL2_BWADJ_CFG_SHIFT (0)

/*
 * ISP/NPU/ENC VCLK Clock Source Configuration @ 0x0800011C
 */
#define ENC_VCLK_RST_CFG_MASK         (0x1)
#define ENC_VCLK_RST_CFG_SHIFT        (23)
#define ENC_SRC_CLK_EN_CFG_MASK       (0x1)
#define ENC_SRC_CLK_EN_CFG_SHIFT      (22)
#define ENC_SRC_CLK_SEL_CFG_MASK      (0x7)
#define ENC_SRC_CLK_SEL_CFG_SHIFT     (19)
#define ENC_SRC_CLK_DIV_NUM_CFG_MASK  (0x7)
#define ENC_SRC_CLK_DIV_NUM_CFG_SHIFT (16)
#define NPU_VCLK_RST_CFG_MASK         (0x1)
#define NPU_VCLK_RST_CFG_SHIFT        (15)
#define NPU_SRC_CLK_EN_CFG_MASK       (0x1)
#define NPU_SRC_CLK_EN_CFG_SHIFT      (14)
#define NPU_SRC_CLK_SEL_CFG_MASK      (0x7)
#define NPU_SRC_CLK_SEL_CFG_SHIFT     (11)
#define NPU_SRC_CLK_DIV_NUM_CFG_MASK  (0x7)
#define NPU_SRC_CLK_DIV_NUM_CFG_SHIFT (8)
#define ISP_VCLK_RST_CFG_MASK         (0x1)
#define ISP_VCLK_RST_CFG_SHIFT        (7)
#define ISP_SRC_CLK_EN_CFG_MASK       (0x1)
#define ISP_SRC_CLK_EN_CFG_SHIFT      (6)
#define ISP_SRC_CLK_SEL_CFG_MASK      (0x7)
#define ISP_SRC_CLK_SEL_CFG_SHIFT     (3)
#define ISP_SRC_CLK_DIV_NUM_CFG_MASK  (0x7)
#define ISP_SRC_CLK_DIV_NUM_CFG_SHIFT (0)

/*
 * Software Reset Control @ 0x08000020
 */
#define CLOCK_SPI0_HSCLK_RST_CFG_SHIFT (31)
#define CLOCK_SARADC_RST_CFG_SHIFT     (30)
#define CLOCK_SDADC_HSRST_CFG_SHIFT    (29)
#define CLOCK_SDDAC_HSRST_CFG_SHIFT    (28)
#define CLOCK_SDADC_RST_CFG_SHIFT      (27)
#define CLOCK_SDDAC_RST_CFG_SHIFT      (26)
#define CLOCK_DPHYCLK_RST_CFG_SHIFT    (25)
#define CLOCK_DCLK_RST_CFG_SHIFT       (24)
#define GCLK_TWI2_RST_CFG_SHIFT        (22)
#define GCLK_MMC2_RST_CFG_SHIFT        (21)
#define GCLK_SPI2_RST_CFG_SHIFT        (20)
#define GCLK_MIPI1_RST_CFG_SHIFT       (19)
#define GCLK_UART2_RST_CFG_SHIFT       (18)
#define GCLK_TWI1_RST_CFG_SHIFT        (17)
#define GCLK_MIPI0_RST_CFG_SHIFT       (16)
#define GCLK_USB_RST_CFG_SHIFT         (15)
#define GCLK_ECRYPTION_RST_CFG_SHIFT   (14)
#define GCLK_MAC_BUFFER_RST_CFG_SHIFT  (13)
#define GCLK_GPIO_RST_CFG_SHIFT        (12)
#define GCLK_L2BUF1_RST_CFG_SHIFT      (11)
#define GCLK_TWI0_RST_CFG_SHIFT        (10)
#define GCLK_L2BUF0_RST_CFG_SHIFT      (9)
#define GCLK_UART1_RST_CFG_SHIFT       (8)
#define GCLK_UART0_RST_CFG_SHIFT       (7)
#define GCLK_SPI1_RST_CFG_SHIFT        (6)
#define GCLK_SPI0_RST_CFG_SHIFT        (5)
#define GCLK_SDDAC_RST_CFG_SHIFT       (4)
#define GCLK_SDADC_RST_CFG_SHIFT       (3)
#define GCLK_MMC1_RST_CFG_SHIFT        (2)
#define GCLK_MMC0_RST_CFG_SHIFT        (1)
#define GCLK_RST_CFG_SHIFT             (0)

#endif /* __CLK_AK3918EV300L_H__ */
