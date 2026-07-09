/**
 * @file ak3918av100-clock.h
 * @author qin_huqiang (qin_huqiang@anyka.oa)
 * @brief This header provides constants for ak3918av100 Soc CLCOK IDs
 * @version 1.0
 * @date 2021-10-19
 * 
 * @copyright Copyright AnyKa(c) 2021
 * 
 */
#ifndef __KM01A_CLOCK_H__
#define __KM01A_CLOCK_H__

/*
 * Source: OSCIN
 */
#define CLOCK_ID_PLL_CLK                   	1

/*
 * Factor CLK IDs
 * Source: PLL2
 */
#define CLOCK_ID_FACTOR_GCLK				0
#define CLOCK_ID_FACTOR_SPI0_HCLK			1
#define CLOCK_ID_FACTOR_SPI0_CLK			2
#define CLOCK_ID_FACTOR_MAC_REFCLK			3
#define CLOCK_ID_FACTOR_X_DPHYCFG			4

/*
 * Gate CLK IDs
 * Source: PLL2
 */
#define CLOCK_ID_GCLK_BRIDGE_MODULE			0
#define CLOCK_ID_GCLK_MMC0					1
#define CLOCK_ID_GCLK_SD_ADC				2
#define CLOCK_ID_GCLK_SD_DAC				3
#define CLOCK_ID_GCLK_SPI0					4
#define CLOCK_ID_GCLK_UART0					5
#define CLOCK_ID_GCLK_ENCRYPTION			6
#define CLOCK_ID_GCLK_MAC					7
#define CLOCK_ID_GCLK_USB					8
#define CLOCK_ID_GCLK_MIPI0					9
#define CLOCK_ID_GCLK_UART2					10
#define CLOCK_ID_GCLK_SPI2					11
#define CLOCK_ID_GCLK_MIPI1					12
#define CLOCK_ID_GCLK_MMC2					13
#define CLOCK_ID_GCLK_TWI1					14
#define CLOCK_ID_GCLK_TWI2					15
#define CLOCK_ID_GCLK_MMC1					16
#define CLOCK_ID_GCLK_SPI1					17
#if defined(CONFIG_KM01A_CODE)
#define CLOCK_ID_GCLK_L2BUF					18
#else
#define CLOCK_ID_GCLK_L2BUF0				18
#endif
#define CLOCK_ID_GCLK_TWI0					19
#if defined(CONFIG_KM01A_CODE)
#define CLOCK_ID_GCLK_FFT					20
#else
#define CLOCK_ID_GCLK_L2BUF1				20
#endif
#define CLOCK_ID_GCLK_GPIO					21
#define CLOCK_ID_GCLK_UART1					22
#if defined(CONFIG_KM01A_CODE)
#define CLOCK_ID_GCLK_PDM					23
#endif

#endif /* __AK3918AV100_CLOCK_H__ */
