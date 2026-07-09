// SPDX-License-Identifier: GPL-2.0+
/*
 * ak37e pinctrl driver
 *
 * anyka
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <linux/io.h>
#include <linux/err.h>
#include <dt-bindings/pinctrl/ak_37e_pinctrl.h>

#define AK_GPIO_CONF_INVALID    (-1)

#define AK_VA_SYSCTRL           ((void __iomem *)(0x08000000))

#define AK_ANALOG_CTRL_REG3     (AK_VA_SYSCTRL + 0x0A4)

#define AK_SHAREPIN_CON0        (AK_VA_SYSCTRL + 0x15C)
#define AK_SHAREPIN_CON1        (AK_VA_SYSCTRL + 0x160)
#define AK_SHAREPIN_CON2        (AK_VA_SYSCTRL + 0x164)
#define AK_SHAREPIN_CON3        (AK_VA_SYSCTRL + 0x168)
#define AK_SHAREPIN_CON4        (AK_VA_SYSCTRL + 0x16C)
#define AK_SHAREPIN_CON5        (AK_VA_SYSCTRL + 0x170)
#define AK_SHAREPIN_CON6        (AK_VA_SYSCTRL + 0x174)
#define AK_SHAREPIN_CON7        (AK_VA_SYSCTRL + 0x178)
#define AK_SHAREPIN_CON8        (AK_VA_SYSCTRL + 0x17C)
#define AK_SHAREPIN_CON9        (AK_VA_SYSCTRL + 0x180)

#define AK_GPIO_DRIVE_CON0      (AK_VA_SYSCTRL + 0x18C)
#define AK_GPIO_DRIVE_CON1      (AK_VA_SYSCTRL + 0x190)
#define AK_GPIO_DRIVE_CON2      (AK_VA_SYSCTRL + 0x194)
#define AK_GPIO_DRIVE_CON3      (AK_VA_SYSCTRL + 0x198)
#define AK_GPIO_DRIVE_CON4      (AK_VA_SYSCTRL + 0x19C)
#define AK_GPIO_DRIVE_CON5      (AK_VA_SYSCTRL + 0x1A0)

#define AK_PPU_PPD_EN0          (AK_VA_SYSCTRL + 0x1D0)
#define AK_PPU_PPD_EN1          (AK_VA_SYSCTRL + 0x1D4)
#define AK_PPU_PPD_EN2          (AK_VA_SYSCTRL + 0x1D8)

#define AK_PPU_PPD_SEL0         (AK_VA_SYSCTRL + 0x1E4)
#define AK_PPU_PPD_SEL1         (AK_VA_SYSCTRL + 0x1E8)
#define AK_PPU_PPD_SEL2         (AK_VA_SYSCTRL + 0x1EC)

#define AK_GPIO_SLEW_RATE0      (AK_VA_SYSCTRL + 0x220)
#define AK_GPIO_SLEW_RATE1      (AK_VA_SYSCTRL + 0x224)
#define AK_GPIO_SLEW_RATE2      (AK_VA_SYSCTRL + 0x228)

#define AK_GPIO_IE_CON0         (AK_VA_SYSCTRL + 0x234)
#define AK_GPIO_IE_CON1         (AK_VA_SYSCTRL + 0x238)
#define AK_GPIO_IE_CON2         (AK_VA_SYSCTRL + 0x23C)

#define AK_SHAREPIN_CON_GROUP            (10)
#define AK_SHAREPIN_CON_REG(pin)         (AK_SHAREPIN_CON0 + \
                                         (pin/AK_SHAREPIN_CON_GROUP)*4)

#define AK_SHAREPIN_CON_REG_OFFSET(pin)  ((pin%AK_SHAREPIN_CON_GROUP)*3)

#define AK_GPIO_DRIVE_CON_GROUP          (16)
#define AK_GPIO_DRIVE_CON_REG(pin)       (AK_GPIO_DRIVE_CON0 + \
                                         (pin/AK_GPIO_DRIVE_CON_GROUP)*4)

#define AK_PPU_PPD_EN_GROUP              (32)
#define AK_PPU_PPD_EN_REG(pin)           (AK_PPU_PPD_EN0 + \
                                         (pin/AK_PPU_PPD_EN_GROUP)*4)

#define AK_PPU_PPD_SEL_GROUP             (32)
#define AK_PPU_PPD_SEL_REG(pin)          (AK_PPU_PPD_SEL0 + \
                                         (pin/AK_PPU_PPD_SEL_GROUP)*4)

#define AK_GPIO_SLEW_RATE_GROUP          (32)
#define AK_GPIO_SLEW_RATE_REG(pin)       (AK_GPIO_SLEW_RATE0 + \
                                         (pin/AK_GPIO_SLEW_RATE_GROUP)*4)

#define AK_GPIO_IE_GROUP                 (32)
#define AK_GPIO_IE_REG(pin)              (AK_GPIO_IE_CON0 + \
                                         (pin/AK_GPIO_IE_GROUP)*4)

#define CFG_DRV_EN              (0x01)
#define CFG_INPUT_EN            (0x02)
#define CFG_PUPD_EN             (0x04)
#define CFG_PUPD_SEL            (0x08)
#define CFG_SLEW_RATE           (0x10)

#define CFG_ALL_DISABLE         (0x0)
#define CFG_ALL_EN              (CFG_DRV_EN|CFG_INPUT_EN|CFG_PUPD_EN \
                                |CFG_PUPD_SEL|CFG_SLEW_RATE)

struct gpio_sharepin {
    int pin;

    int8_t pad_drv_offset;
    int8_t pad_ie_offset;
    int8_t pupd_en_offset;
    int8_t pupd_sel_offset;
    int8_t pad_sl_offset;
    int8_t func_mux_max;
    int8_t gpio_cfg;
};

//share pin func config in AK37Exx
static struct gpio_sharepin ak_sharepin[] = {
#if defined(CONFIG_SPL_BUILD)
    /* GPIO22, UART0_RXD, PWM4 */
    {22, 12, 22, 22, 22, 22, 3, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO23, UART0_TXD, PWM5 */
    {23, 14, 23, 23, 23, 23, 3, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO10, SPI0_CS0 */
    {10, 20, 10, 10, 10, 10, 2, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO11, SPI0_HOLD */
    {11, 22, 11, 11, 11, 11, 2, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO12, SPI0_DIN */
    {12, 24, 12, 12, 12, 12, 2, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO13, SPI0_SCLK */
    {13, 26, 13, 13, 13, 13, 2, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO14, SPI0_WP */
    {14, 28, 14, 14, 14, 14, 2, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO15, SPI0_DOUT */
    {15, 30, 15, 15, 15, 15, 2, (CFG_INPUT_EN|CFG_PUPD_EN)},
#else
    /* PIN pad_drv_offset pad_ie_offset pupd_en_offset*/
    /*pupd_sel_offset pad_sl_offset gpio_config */
    /* GPIO0, RMII0_MDIO, SD2_D1, I2S0_DIN, TWI0_SCL */
    {0, 0, 0, 0, 0, 0, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO1, RMII0_MDC, SD2_D0, I2S0_LRCLK, TWI0_SDA */
    {1, 2, 1, 1, 1, 1, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO2, RMII0_RXER, SD2_MCLK, I2S0_BCLK, PWM5 */
    {2, 4, 2, 2, 2, 2, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO3, RMII0_RXDV, SD2_CMD, I2S0_MCLK, PWM4 */
    {3, 6, 3, 3, 3, 3, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO4, RMII0_RXD0, SD2_D3, I2S0_OUT */
    {4, 8, 4, 4, 4, 4, 4, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO5, RMII0_RXD1, SD2_D2, PWM3 */
    {5, 10, 5, 5, 5, 5, 4, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO6, OPCLK2, TWI3_SCL, PWM2, I2S1_LRCLK */
    {6, 12, 6, 6, 6, 6, 5, CFG_ALL_EN},
    /* GPIO7, RMII0_TXD0, TWI3_SDA, PWM1, I2S1_BCLK */
    {7, 14, 7, 7, 7, 7, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO8, RMII0_TXD1, UART3_RXD, PDM_CLK, I2S1_MCLK */
    {8, 16, 8, 8, 8, 8, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO9, RMII0_TXEN, UART3_TXD, PDM_DATA, I2S1_DIN */
    {9, 18, 9, 9, 9, 9, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO10, SPI0_CS0 */
    {10, 20, 10, 10, 10, 10, 2, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO11, SPI0_HOLD */
    {11, 22, 11, 11, 11, 11, 2, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO12, SPI0_DIN */
    {12, 24, 12, 12, 12, 12, 2, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO13, SPI0_SCLK */
    {13, 26, 13, 13, 13, 13, 2, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO14, SPI0_WP */
    {14, 28, 14, 14, 14, 14, 2, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO15, SPI0_DOUT */
    {15, 30, 15, 15, 15, 15, 2, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO16, SPI1_CS0, SD1_D1, TWI1_SCL, PWM1 */
    {16, 0, 16, 16, 16, 16, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO17, SPI1_DIN(IO1), SD1_D0, TWI1_SDA, PWM4 */
    {17, 2, 17, 17, 17, 17, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO18, SPI1_SCLK, SD1_MCLK */
    {18, 4, 18, 18, 18, 18, 3, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO19, SPI1_DOUT(IO0), SD1_CMD */
    {19, 6, 19, 19, 19, 19, 3, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO20, SPI1_WP(IO2), SD1_D3, UART2_RXD, PWM3 */
    {20, 8, 20, 20, 20, 20, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO21, SPI1_HOLD(IO3), SD1_D2, UART2_TXD, PWM2 */
    {21, 10, 21, 21, 21, 21, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO22, UART0_RXD, PWM4 */
    {22, 12, 22, 22, 22, 22, 3, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO23, UART0_TXD, PWM5 */
    {23, 14, 23, 23, 23, 23, 3, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO24, UART1_RXD, SPI2_CS0, PWM3, RGB_D20 */
    {24, 14, 24, 24, 24, 24, 5, CFG_ALL_EN},
    /* GPIO25, UART1_TXD, SPI2_SCLK, PWM2, RGB_D21 */
    {25, 14, 25, 25, 25, 25, 5,
    (CFG_DRV_EN|CFG_INPUT_EN|CFG_PUPD_EN|CFG_SLEW_RATE)},
    /* GPIO26, UART1_CTS, SPI2_DIN(IO1), PWM0, RGB_D22 */
    {26, 14, 26, 26, 26, 26, 5,
    (CFG_DRV_EN|CFG_INPUT_EN|CFG_PUPD_EN|CFG_SLEW_RATE)},
    /* GPIO27, UART1_RTS, SPI2_DOUT(IO0), PWM5, RGB_D23 */
    {27, 14, 27, 27, 27, 27, 5,
    (CFG_DRV_EN|CFG_INPUT_EN|CFG_PUPD_EN|CFG_SLEW_RATE)},
    /* GPIO28, UART2_RXD, PWM1, SPI2_CS1, RGB_D19 */
    {28, 14, 28, 28, 28, 28, 5,
    (CFG_DRV_EN|CFG_INPUT_EN|CFG_PUPD_EN|CFG_SLEW_RATE)},
    /* GPIO29, UART2_TXD, PWM3, SPI1_CS1, RGB_D18 */
    {29, 14, 29, 29, 29, 29, 5,
    (CFG_DRV_EN|CFG_INPUT_EN|CFG_PUPD_EN|CFG_SLEW_RATE)},
    /* GPIO30, TWI0_SCL */
    {30, 28, 30, 30, 30, 30, 2, CFG_ALL_EN},
    /* GPIO31, TWI0_SDA */
    {31, 30, 31, 31, 31, 31, 2, CFG_ALL_EN},
    /* GPIO32, TWI1_SCL */
    {32, 0, 0, 0, 0, 0, 2, CFG_ALL_EN},
    /* GPIO33, TWI1_SDA */
    {33, 2, 1, 1, 1, 1, 2, CFG_ALL_EN},
    /* GPIO34, RGB_VOGATE, MPU_RD# */
    {34, 4, 2, 2, 2, 2, 3, CFG_ALL_EN},
    /* GPIO35, RGB_VOVSYNC, MPU_A0 */
    {35, 6, 3, 3, 3, 3, 3, CFG_ALL_EN},
    /* GPIO36, RGB_VOHSYNC, MPU_CS# */
    {36, 8, 4, 4, 4, 4, 3, CFG_ALL_EN},
    /* GPIO37, RGB_VOPCLK, MPU_WR# */
    {37, 10, 5, 5, 5, 5, 3, CFG_ALL_EN},
    /* GPIO38, RGB_D0, MPU_D0 */
    {38, 12, 6, 6, 6, 6, 3, CFG_ALL_EN},
    /* GPIO39, RGB_D1, MPU_D1 */
    {39, 14, 7, 7, 7, 7, 3, CFG_ALL_EN},
    /* GPIO40, RGB_D2, MPU_D2 */
    {40, 16, 8, 8, 8, 8, 3, CFG_ALL_EN},
    /* GPIO41, RGB_D3, MPU_D3 */
    {41, 18, 9, 9, 9, 9, 3, CFG_ALL_EN},
    /* GPIO42, RGB_D4, MPU_D4 */
    {42, 20, 10, 10, 10, 10, 3, CFG_ALL_EN},
    /* GPIO43, RGB_D5, MPU_D5 */
    {43, 22, 11, 11, 11, 11, 4, CFG_ALL_EN},
    /* GPIO44, RGB_D6, MPU_D6 */
    {44, 24, 12, 12, 12, 12, 4, CFG_ALL_EN},
    /* GPIO45, RGB_D7, MPU_D7 */
    {45, 26, 13, 13, 13, 13, 4, CFG_ALL_EN},
    /* GPIO46, RGB_D8, MPU_D8, TWI2_SCL */
    {46, 28, 14, 14, 14, 14, 4, CFG_ALL_EN},
    /* GPIO47, RGB_D9, MPU_D9, TWI2_SDA */
    {47, 30, 15, 15, 15, 15, 4, CFG_ALL_EN},
    /* GPIO48, RGB_D10, MPU_D10, JTAG_RSTN */
    {48, 0, 16, 16, 16, 16, 4, CFG_ALL_EN},
    /* GPIO49, RGB_D11, MPU_D11, JTAG_TDI */
    {49, 2, 17, 17, 17, 17, 4, CFG_ALL_EN},
    /* GPIO50, RGB_D12, MPU_D12, JTAG_TMS, I2S1_LRCLK */
    {50, 4, 18, 18, 18, 18, 5, CFG_ALL_EN},
    /* GPIO51, RGB_D13, MPU_D13, JTAG_TCLK, I2S1_BCLK */
    {51, 6, 19, 19, 19, 19, 5, CFG_ALL_EN},
    /* GPIO52, RGB_D14, MPU_D14, JTAG_RTCK, I2S1_MCLK */
    {52, 8, 20, 20, 20, 20, 5, CFG_ALL_EN},
    /* GPIO53, RGB_D15, MPU_D15, JTAG_TDO, I2S1_DIN */
    {53, 10, 21, 21, 21, 21, 5, CFG_ALL_EN},
    /* GPIO54, RGB_D16, MPU_D16, PDM_CLK, TWI3_SCL */
    {54, 12, 22, 22, 22, 22, 5, CFG_ALL_EN},
    /* GPIO55, RGB_D17, MPU_D17, PDM_DATA, TWI3_SDA */
    {55, 12, 23, 23, 23, 23, 5, CFG_ALL_EN},
    /* GPIO56, CSI0_SCLK, UART3_RXD */
    {56, 16, 24, 24, 24, 24, 3, CFG_ALL_EN},
    /* GPIO57, CSI0_PCLK, UART3_TXD */
    {57, 18, 25, 25, 25, 25, 3, CFG_ALL_EN},
    /* GPIO58, CSI0_HSYNC, RMII1_MDIO, SD2_D1 */
    {58, 20, 26, 26, 26, 26, 4, CFG_ALL_EN},
    /* GPIO59, CSI0_VSYNC, RMII1_MDC, SD2_D0 */
    {59, 22, 27, 27, 27, 27, 4, CFG_ALL_EN},
    /* GPIO60, CSI0_D0, RMII1_RXER, SD2_MCLK */
    {60, 24, 28, 28, 28, 28, 4, CFG_ALL_EN},
    /* GPIO61, CSI0_D1, RMII1_RXDV, SD2_CMD */
    {61, 26, 29, 29, 29, 29, 4, CFG_ALL_EN},
    /* GPIO62, CSI0_D2, RMII1_RXD0, SD2_D3 */
    {62, 28, 30, 30, 30, 30, 4, CFG_ALL_EN},
    /* GPIO63, CSI0_D3, RMII1_RXD1, SD2_D2 */
    {63, 30, 31, 31, 31, 31, 4, CFG_ALL_EN},
    /* GPIO64, CSI0_D4, OPCLK1, TWI2_SCL, I2S1_LRCLK */
    {64, 0, 0, 0, 0, 0, 5, CFG_ALL_EN},
    /* GPIO65, CSI0_D5, RMII1_RXD0, TWI2_SDA, I2S1_BCLK */
    {65, 2, 1, 1, 1, 1, 5, CFG_ALL_EN},
    /* GPIO66, CSI0_D6, RMII1_TXD1, PDM_CLK, I2S1_MCLK */
    {66, 4, 2, 2, 2, 2, 5, CFG_ALL_EN},
    /* GPIO67, CSI0_D7, RMII1_TXEN, PDM_DATA, I2S1_DIN */
    {67, 6, 3, 3, 3, 3, 5, CFG_ALL_EN},
    /* GPIO68, CSI1_SCLK, SD0_D2, SPI2_CS0 */
    {68, 8, 4, 4, 4, 4, 4, CFG_ALL_EN},
    /* GPIO69, CSI1_PCLK, SD0_D3, SPI2_HOLD(IO3) */
    {69, 10, 5, 5, 5, 5, 4, CFG_ALL_EN},
    /* GPIO70, CSI1_HSYNC, SD0_D4, SPI2_DIN(IO1), UART1_RXD */
    {70, 12, 6, 6, 6, 6, 5, CFG_ALL_EN},
    /* GPIO71, CSI1_VSYNC, SD0_CMD, SPI2_SCLK, UART1_TXD */
    {71, 14, 7, 7, 7, 7, 5, CFG_ALL_EN},
    /* GPIO72, CSI1_D0, SD0_D5, SPI2_WP(IO2), UART1_CTS */
    {72, 16, 8, 8, 8, 8, 5, CFG_ALL_EN},
    /* GPIO73, CSI1_D1, SD0_CLK, SPI2_DOUT(IO0), UART1_RTS */
    {73, 18, 9, 9, 9, 9, 5, CFG_ALL_EN},
    /* GPIO74, CSI1_D2, SD0_D6, RGB_D23, SPI2_CS1 */
    {74, 20, 10, 10, 10, 10, 5, CFG_ALL_EN},
    /* GPIO75, CSI1_D3, SD0_D7, RGB_D22, I2S0_DIN */
    {75, 22, 11, 11, 11, 11, 5, CFG_ALL_EN},
    /* GPIO76, CSI1_D4, SD0_D1, RGB_D21, I2S0_LRCLK */
    {76, 24, 12, 12, 12, 12, 5, CFG_ALL_EN},
    /* GPIO77, CSI1_D5, SD0_D0, RGB_D20, I2S0_BCLK */
    {77, 26, 13, 13, 13, 13, 5, CFG_ALL_EN},
    /* GPIO78, CSI1_D6, TWI2_SCL, RGB_D19, I2S0_MCLK */
    {78, 28, 14, 14, 14, 14, 5, CFG_ALL_EN},
    /* GPIO79, CSI1_D7, TWI2_SDA, RGB_D18, I2S0_DOUT */
    {79, 30, 15, 15, 15, 15, 5, CFG_ALL_EN},
    /* Not support */
    {80, 0, 16, 16, 16, 16, AK_GPIO_CONF_INVALID, CFG_ALL_DISABLE},
    /* Not support */
    {81, 2, 17, 17, 17, 17, AK_GPIO_CONF_INVALID, CFG_ALL_DISABLE},
    /* GPIO82, PWM0, SPI0_CS1, SPI1_CS1 */
    {82, 4, 18, 18, 18, 18, 4, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO83, TWI2_SCL, SPI2_CS0, UART2_RXD, I2S1_LRCLK */
    {83, 6, 19, 19, 19, 19, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO84, TWI2_SDA, SPI2_SCLK, UART2_TXD, I2S1_BCLK */
    {84, 8, 20, 20, 20, 20, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO85, TWI3_SCL, SPI2_DIN(IO1), UART3_RXD, I2S1_MCLK */
    {85, 10, 21, 21, 21, 21, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* GPIO86, TWI3_SDA, SPI2_DOUT(IO0), UART3_TXD, I2S1_DIN */
    {86, 12, 22, 22, 22, 22, 5, (CFG_INPUT_EN|CFG_PUPD_EN)},
    /* Not support */
    {87, 14, 23, 23, 23, 23, AK_GPIO_CONF_INVALID, CFG_ALL_DISABLE},
    /* Not support */
    {88, 16, 24, 24, 24, 24, AK_GPIO_CONF_INVALID, CFG_ALL_DISABLE},
    /* Not support */
    {89, 18, 25, 25, 25, 25, AK_GPIO_CONF_INVALID, CFG_ALL_DISABLE},
    /* Not support */
    {90, 20, 26, 26, 26, 26, AK_GPIO_CONF_INVALID, CFG_ALL_DISABLE},
    /* Not support */
    {91, 22, 27, 27, 27, 27, AK_GPIO_CONF_INVALID, CFG_ALL_DISABLE},
    /* Not support */
    {92, 24, 28, 28, 28, 28, AK_GPIO_CONF_INVALID, CFG_ALL_DISABLE},
    /* Not support */
    {93, 26, 29, 29, 29, 29, AK_GPIO_CONF_INVALID, CFG_ALL_DISABLE},
    /* Not support */
    {94, 28, 30, 30, 30, 30, AK_GPIO_CONF_INVALID, CFG_ALL_DISABLE},
    /* MIC_P, GPI0 */
    {95, 0, 0, 0, 0, 0, 2, CFG_INPUT_EN},
    /* MIC_N, GPI1 */
    {96, 0, 0, 0, 0, 0, 2, CFG_INPUT_EN},
#endif
};

static inline int pin_gpi_groups(int pin)
{
    return ((pin == 95) || (pin == 96)) ? 1 : 0;
}

static struct gpio_sharepin *find_sharepin_struct(int pin)
{
    int i;
    int num = sizeof(ak_sharepin)/sizeof(ak_sharepin[0]);

    for (i = 0; i < num; i++) {
        if (ak_sharepin[i].pin == pin)
            return &ak_sharepin[i];
    }

    return NULL;
}

static int ak37e_pmx_set(int pin, int sel)
{
    struct gpio_sharepin *p_ak_sharepin;
    unsigned int regval;
    int offset = AK_SHAREPIN_CON_REG_OFFSET(pin);

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    if (p_ak_sharepin->func_mux_max == AK_GPIO_CONF_INVALID) {
        pr_err("%s pin_%d not support\n", __func__, pin);
        return -EINVAL;
    }

    if (sel >= p_ak_sharepin->func_mux_max) {
        pr_err("%s pin_%d max %d but sel %d\n",
                __func__, pin, p_ak_sharepin->func_mux_max, sel);
        return -EINVAL;
    }

    /*
     * Special Case: only 2 GPI pins
     * AK_ANALOG_CTRL_REG3
     * [25]:GPI0
     * [26]:GPI1
     */
    if (pin_gpi_groups(pin)) {
        regval = __raw_readl(AK_ANALOG_CTRL_REG3);
        regval &= ~(0x1 << (pin - 95 + 25));
        regval |= ((sel & 0x1) << (pin - 95 + 25));
        __raw_writel(regval, AK_ANALOG_CTRL_REG3);
        pr_debug("PIN_%d: FUNC(%d) 0x%p:0x%x\n", pin, sel, AK_ANALOG_CTRL_REG3,
                __raw_readl(AK_ANALOG_CTRL_REG3));
    } else {
        regval = __raw_readl(AK_SHAREPIN_CON_REG(pin));
        regval &= ~(0x7 << offset);
        regval |= ((sel & 0x7) << offset);
        __raw_writel(regval, AK_SHAREPIN_CON_REG(pin));
        pr_debug("PIN_%d: FUNC(%d) 0x%p:0x%x\n",
                pin, sel, AK_SHAREPIN_CON_REG(pin),
                __raw_readl(AK_SHAREPIN_CON_REG(pin)));
    }

    return 0;

}

static int pin_set_pull_enable(int pin, int enable)
{
    struct gpio_sharepin *p_ak_sharepin;
    unsigned int regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    if (!(p_ak_sharepin->gpio_cfg & CFG_PUPD_EN)) {
        return -EPERM;
    }

    regval = __raw_readl(AK_PPU_PPD_EN_REG(pin));
    if (enable) {
        regval |= (0x1 << p_ak_sharepin->pupd_en_offset);
    } else {
        regval &= (~(0x1 << p_ak_sharepin->pupd_en_offset));
    }

    __raw_writel(regval, AK_PPU_PPD_EN_REG(pin));

    pr_debug("PIN_%d: PULL_EN(%d) 0x%p:0x%x\n", pin, enable,
        AK_PPU_PPD_EN_REG(pin), __raw_readl(AK_PPU_PPD_EN_REG(pin)));

    return 0;
}

static int pin_set_pull_polarity(int pin, int pullup)
{
    struct gpio_sharepin *p_ak_sharepin;
    unsigned int regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    if (!(p_ak_sharepin->gpio_cfg & CFG_PUPD_SEL)) {
        return -EPERM;
    }

    regval = __raw_readl(AK_PPU_PPD_SEL_REG(pin));
    if (pullup) {
        regval &= (~(0x1 << p_ak_sharepin->pupd_sel_offset));
    } else {
        regval |= (0x1 << p_ak_sharepin->pupd_sel_offset);
    }

    __raw_writel(regval, AK_PPU_PPD_SEL_REG(pin));

    pr_debug("PIN_%d: PULL_pol(%d) 0x%p:0x%x\n", pin, pullup,
        AK_PPU_PPD_SEL_REG(pin),
        __raw_readl(AK_PPU_PPD_SEL_REG(pin)));

    return 0;
}

static int pin_set_drive(int pin, int strength)
{
    struct gpio_sharepin *p_ak_sharepin;
    unsigned int regval;
    void __iomem * pad_drv_reg = AK_GPIO_DRIVE_CON_REG(pin);

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    if (!(p_ak_sharepin->gpio_cfg & CFG_DRV_EN)) {
        return -EPERM;
    }

    if (strength > 3)
        return -EINVAL;

    /* !! Specail case !!*/
    if ((pin >= XGPIO_024) && (pin <= XGPIO_029)) {
        pad_drv_reg = AK_GPIO_DRIVE_CON3;
    }

    regval = __raw_readl(pad_drv_reg);
    regval &= ~(0x3 << p_ak_sharepin->pad_drv_offset);
    regval |= ((strength & 0x3) << p_ak_sharepin->pad_drv_offset);

    __raw_writel(regval, pad_drv_reg);

    pr_debug("PIN_%d: DRV(%d) 0x%p:0x%x\n", pin, p_ak_sharepin->pad_drv_offset,
            pad_drv_reg, __raw_readl(pad_drv_reg));

    return 0;
}

static int pin_set_input_enable(int pin, int enable)
{
    struct gpio_sharepin *p_ak_sharepin;
    unsigned int regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    if (pin_gpi_groups(pin)) {
        regval = __raw_readl(AK_ANALOG_CTRL_REG3);
        if (enable)
            regval |= (0x1 << (pin - 95 + 25));
        else
            regval &= ~(0x1 << (pin - 95 + 25));
        __raw_writel(regval, AK_ANALOG_CTRL_REG3);

        return 0;
    }

    if (!(p_ak_sharepin->gpio_cfg & CFG_INPUT_EN)) {
        return -EPERM;
    }

    regval = __raw_readl(AK_GPIO_IE_REG(pin));
    if (enable) {
        regval |= (0x1 << p_ak_sharepin->pad_ie_offset);
    } else {
        regval &= ~(0x1 << p_ak_sharepin->pad_ie_offset);
    }

    __raw_writel(regval, AK_GPIO_IE_REG(pin));
    pr_debug("PIN_%d: Input_EN(%d) 0x%p:0x%x\n",
            pin, enable, AK_GPIO_IE_REG(pin),
            __raw_readl(AK_GPIO_IE_REG(pin)));

    return 0;
}

static int pin_set_slew_rate(int pin, int fast)
{
    struct gpio_sharepin *p_ak_sharepin;
    unsigned int regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    if (!(p_ak_sharepin->gpio_cfg & CFG_SLEW_RATE)) {
        return -EPERM;
    }

    regval = __raw_readl(AK_GPIO_SLEW_RATE_REG(pin));
    if (fast) {
        regval |= (0x1 << p_ak_sharepin->pad_sl_offset);
    } else {
        regval &= (~(0x1 << p_ak_sharepin->pad_sl_offset));
    }
    __raw_writel(regval, AK_GPIO_SLEW_RATE_REG(pin));

    pr_debug("PIN_%d: SR(%d) 0x%p:0x%x\n", pin, fast,
            AK_GPIO_SLEW_RATE_REG(pin),
            __raw_readl(AK_GPIO_SLEW_RATE_REG(pin)));

    return 0;
}

static int ak37e_pinconf_set(int pin, unsigned int configs)
{
    unsigned char pupd, drive, ie, slew;

    /*
    * configs
    * bit[0] pull up or pull down selection
    * bit[4] pull up or pull down enable
    * bit[9:8] drive
    * bit[16]:input enable
    * bit[24]: slew rate
    */
    /* for config value bit[31:24]--slew rate, bit[23:16]--ie,
        bit[15:8]--drive, bit[7:0]--pupd */
    pupd = (configs & 0xFF);
    drive = ((configs>>8) & 0xFF);
    ie = ((configs>>16) & 0xFF);
    slew = ((configs>>24) & 0xFF);

    pin_set_pull_enable(pin, (pupd & 0x10) ? 1:0);
    pin_set_pull_polarity(pin, (pupd & 0x01) ? 0:1);
    pin_set_drive(pin, drive & 0x3);
    pin_set_input_enable(pin, ie & 0x1);
    pin_set_slew_rate(pin, slew & 0x1);

    return 0;
}

static int ak37e_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
    const fdt32_t *data_pins, *data_func, *data_pull;
    int count_pins, count_func, count_pull;
    int i;

    pr_debug("%s %s\n", __func__, config->name);

    data_pins = dev_read_prop(config, "anyka,pins", &count_pins);
    if (count_pins < 0) {
        pr_err("%s: bad pins array size %d\n", __func__, count_pins);
        return -EINVAL;
    }
    count_pins /= sizeof(fdt32_t);

    data_func = dev_read_prop(config, "anyka,function", &count_func);
    if (count_func < 0) {
        pr_err("%s: bad func array size %d\n", __func__, count_func);
        return -EINVAL;
    }
    count_func /= sizeof(fdt32_t);

    data_pull = dev_read_prop(config, "anyka,pull", &count_pull);
    if (count_pull < 0) {
        pr_err("%s: bad pull array size %d\n", __func__, count_pull);
        return -EINVAL;
    }
    count_pull /= sizeof(fdt32_t);

    if (count_func > 1 && count_func != count_pins) {
        pr_err("%s: anyka,function must have 1 or %d entries\n",
                config->name, count_pins);
        return -EINVAL;
    }

    if (count_pull > 1 && count_pull != count_pins) {
        pr_err("%s: anyka,pull must have 1 or %d entries\n",
                config->name, count_pins);
        return -EINVAL;
    }

    for (i = 0; i < count_pins; i++) {
        unsigned int conf;
        int pin, sel;

        pin = fdt32_to_cpu(data_pins[i]);
        sel = (count_func > 1) ?
            fdt32_to_cpu(data_func[i]) : fdt32_to_cpu(data_func[0]);
        conf = (count_pull > 1) ?
            fdt32_to_cpu(data_pull[i]) : fdt32_to_cpu(data_pull[0]);
        ak37e_pmx_set(pin, sel);
        ak37e_pinconf_set(pin, conf);
    }

    return 0;
}

const struct pinctrl_ops ak37e_pinctrl_ops  = {
    .set_state = ak37e_pinctrl_set_state,
};

static int ak37e_pinctrl_probe(struct udevice *dev)
{
    struct clk clk;
    int ret;

    pr_debug("%s\n", __func__);

    ret = clk_get_by_index(dev, 0, &clk);
    if (ret)
        return ret;
    ret = clk_enable(&clk);
    if (ret)
        return ret;

    return 0;
}

static const struct udevice_id ak37e_pinctrl_match[] = {
    { .compatible = "anyka,ak37e-pinctrl"},
    {}
};

U_BOOT_DRIVER(ak37e_pinctrl) = {
    .name = "pinctrl_ak37e",
    .id = UCLASS_PINCTRL,
    .of_match = ak37e_pinctrl_match,
    .probe = ak37e_pinctrl_probe,
    .ops = &ak37e_pinctrl_ops,
};
