/*
 * This header provides constants for AK39EV33X Soc CLCOK IDs
 *
 */
#ifndef __AK39EV33X_CLOCK_H__
#define __AK39EV33X_CLOCK_H__

/*
* Source: OSCIN
*/
#define CORE_PLL_CLK                   1
#define PERI_PLL_CLK                   2

/*
* Factor CLK IDs
* Source: PERI
*/
#define PERI_FACTOR_CSI0_SCLK          0
#define PERI_FACTOR_CSI1_SCLK          1
#define PERI_FACTOR_MAC_PCLK           2
#define PERI_FACTOR_USB_CLK            3

/*
* Gate CLK IDs
* Source: CORE
*/
#define CORE_GCLK_BRIDGE_MODULE        0
#define CORE_GCLK_MMC0                 1
#define CORE_GCLK_MMC1                 2
#define CORE_GCLK_SD_ADC               3
#define CORE_GCLK_SD_DAC               4
#define CORE_GCLK_SPI0                 5
#define CORE_GCLK_SPI1                 6
#define CORE_GCLK_UART0                7
#define CORE_GCLK_UART1                8
#define CORE_GCLK_L2BUF                9
#define CORE_GCLK_TWI0                 10
#define CORE_GCLK_IRDA                 11
#define CORE_GCLK_GPIO                 12
#define CORE_GCLK_MAC                  13
#define CORE_GCLK_ENCRYTION            14
#define CORE_GCLK_USB                  15
#define CORE_GCLK_MIPI                 16
#define CORE_GCLK_TWI1                 17
#define CORE_GCLK_ISP                  18
#define CORE_GCLK_VENCODER             19
#define CORE_GCLK_TWI2                 20

#endif //__AK39EV33X_CLOCK_H__
