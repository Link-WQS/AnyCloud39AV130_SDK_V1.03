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
#include <dt-bindings/pinctrl/ak_3918ev300l_pinctrl.h>

#define AK_GPIO_CONF_INVALID    (-1)

#define AK_VA_SYSCTRL           ((void __iomem *)(0x08000000))
#define AK_VA_GPIO              ((void __iomem *)(0x20170000))

#define AK_ANALOG_CTRL_REG1     (AK_VA_SYSCTRL + 0x0A0)
#define AK_ANALOG_CTRL_REG2     (AK_VA_SYSCTRL + 0x0A4)

#define AK_SHAREPIN_CON0        (AK_VA_SYSCTRL + 0x178)
#define AK_SHAREPIN_CON1        (AK_VA_SYSCTRL + 0x17C)
#define AK_SHAREPIN_CON2        (AK_VA_SYSCTRL + 0x180)
#define AK_SHAREPIN_CON3        (AK_VA_SYSCTRL + 0x184)
#define AK_SHAREPIN_CON4        (AK_VA_SYSCTRL + 0x188)
#define AK_SHAREPIN_CON5        (AK_VA_SYSCTRL + 0x18C)
#define AK_SHAREPIN_CON6        (AK_VA_SYSCTRL + 0x190)
#define AK_SHAREPIN_CON7        (AK_VA_SYSCTRL + 0x194)
#define AK_SHAREPIN_CON8        (AK_VA_SYSCTRL + 0x198)
#define AK_SHAREPIN_CON9        (AK_VA_SYSCTRL + 0x19C)
#define AK_SHAREPIN_CON10       (AK_VA_SYSCTRL + 0x1A0)

#define AK_GPIO_DRIVE_CON0      (AK_VA_SYSCTRL + 0x1A4)
#define AK_GPIO_DRIVE_CON1      (AK_VA_SYSCTRL + 0x1A8)
#define AK_GPIO_DRIVE_CON2      (AK_VA_SYSCTRL + 0x1AC)
#define AK_GPIO_DRIVE_CON3      (AK_VA_SYSCTRL + 0x1B0)
#define AK_GPIO_DRIVE_CON4      (AK_VA_SYSCTRL + 0x1B4)
#define AK_GPIO_DRIVE_CON5      (AK_VA_SYSCTRL + 0x1B8)
#define AK_GPIO_DRIVE_CON6      (AK_VA_SYSCTRL + 0x1BC)

#define AK_PPU_PPD_EN0          (AK_VA_SYSCTRL + 0x264)
#define AK_PPU_PPD_EN1          (AK_VA_SYSCTRL + 0x268)
#define AK_PPU_PPD_EN2          (AK_VA_SYSCTRL + 0x26C)
#define AK_PPU_PPD_EN3          (AK_VA_SYSCTRL + 0x270)

#define AK_PPU_PPD_SEL0         (AK_VA_SYSCTRL + 0x274)
#define AK_PPU_PPD_SEL1         (AK_VA_SYSCTRL + 0x278)
#define AK_PPU_PPD_SEL2         (AK_VA_SYSCTRL + 0x27C)
#define AK_PPU_PPD_SEL3         (AK_VA_SYSCTRL + 0x280)

#define AK_GPIO_SLEW_RATE0      (AK_VA_SYSCTRL + 0x1D0)
#define AK_GPIO_SLEW_RATE1      (AK_VA_SYSCTRL + 0x1D4)
#define AK_GPIO_SLEW_RATE2      (AK_VA_SYSCTRL + 0x1D8)
#define AK_GPIO_SLEW_RATE3      (AK_VA_SYSCTRL + 0x1DC)

#define AK_GPIO_IE_CON0         (AK_VA_SYSCTRL + 0x1C0)
#define AK_GPIO_IE_CON1         (AK_VA_SYSCTRL + 0x1C4)
#define AK_GPIO_IE_CON2         (AK_VA_SYSCTRL + 0x1C8)
#define AK_GPIO_IE_CON3         (AK_VA_SYSCTRL + 0x1CC)

#define AK_GPIO_ST_CON0         (AK_VA_SYSCTRL + 0x284)
#define AK_GPIO_ST_CON1         (AK_VA_SYSCTRL + 0x288)
#define AK_GPIO_ST_CON2         (AK_VA_SYSCTRL + 0x28C)
#define AK_GPIO_ST_CON3         (AK_VA_SYSCTRL + 0x290)

#define AK_SHAREPIN_CON_GROUP            (10)

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

#define AK_GPIO_ST_GROUP                 (32)
#define AK_GPIO_ST_REG(pin)              (AK_GPIO_ST_CON0 + \
                                         (pin/AK_GPIO_ST_GROUP)*4)

#define AK_GPIO_DIR0_REG        (AK_VA_GPIO + 0x00)
#define AK_GPIO_OUT0_REG        (AK_VA_GPIO + 0x14)

#define AK_GPIO_DIR_REG(pin)    (((pin)>>5)*4 + AK_GPIO_DIR0_REG)
#define AK_GPIO_OUT_REG(pin)    (((pin)>>5)*4 + AK_GPIO_OUT0_REG)

#define AK_GPIO_REG_SHIFT(pin)  ((pin) % 32)

#define CFG_DRV_EN              (0x01)
#define CFG_INPUT_EN            (0x02)
#define CFG_PUPD_EN             (0x04)
#define CFG_PUPD_SEL            (0x08)
#define CFG_SLEW_RATE           (0x10)
#define CFG_SCHMITT_EN          (0x20)

#define CFG_ALL_DISABLE         (0x0)
#define CFG_ALL_EN              (CFG_DRV_EN|CFG_INPUT_EN|CFG_PUPD_EN| \
                                CFG_PUPD_SEL|CFG_SLEW_RATE|CFG_SCHMITT_EN)

struct gpio_sharepin {
    int pin;
    int funcmux_reg_no;
    int funcmux_reg_offset;
    int8_t pad_drv_offset;
    int8_t pad_ie_offset;
    int8_t pupd_en_offset;
    int8_t pupd_sel_offset;
    int8_t pad_sl_offset;
    int8_t pad_st_offset;
    int8_t func_mux_max;
    int8_t gpio_cfg;
};

//share pin func config in ak3918ev300l
static struct gpio_sharepin ak_sharepin[] = {
#if defined(CONFIG_SPL_BUILD)
    /* GPIO51, UART0_TXD */
    {51, 5, 6, 6, 19, 19, 19, 19, 19, 2, CFG_ALL_EN},
    /* GPIO52, UART0_RXD */
    {52, 5, 9, 8, 20, 20, 20, 20, 20, 2, CFG_ALL_EN},
    /* GPIO42, eMMC_D3, SPI0_WP(IO1) */
    {42, 4, 9, 20, 10, 10, 10, 10, 10, 3, CFG_ALL_EN},
    /* GPIO43, eMMC_D0, SPI0_DIN(IO1) */
    {43, 4, 12, 22, 11, 11, 11, 11, 11, 3, CFG_ALL_EN},
    /* GPIO44, eMMC_D1, SPI0_CS */
    {44, 4, 15, 24, 12, 12, 12, 12, 12, 3, CFG_ALL_EN},
    /* GPIO45, eMMC_D2, SPI0_HOLD(IO3) */
    {45, 4, 18, 26, 13, 13, 13, 13, 13, 3, CFG_ALL_EN},
    /* GPIO46, eMMC_MCLK, SPI0_CLK */
    {46, 4, 21, 28, 14, 14, 14, 14, 14, 3, CFG_ALL_EN},
    /* GPIO47, eMMC_CMD, SPI0_DOUT(IO0 */
    {47, 4, 24, 30, 15, 15, 15, 15, 15, 3, CFG_ALL_EN},
#else
    /* PIN funcmux_reg_no funcmux_reg_offset pad_drv_offset
    pad_ie_offset pupd_en_offset 
    pupd_sel_offset pad_sl_offset pad_st_offset gpio_config */
    /* GPIO0, UART1_TXD, SPI1_CLK, OPCLK1 */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 4, CFG_ALL_EN},
    /* GPIO1, UART1_RXD, SPI1_DIN(IO1), RMII_RXER */
    {1, 0, 3, 2, 1, 1, 1, 1, 1, 4, CFG_ALL_EN},
    /* GPIO2, RMII0_RXER, SD2_MCLK, I2S0_BCLK, PWM5 */
    {2, 0, 6, 4, 2, 2, 2, 2, 2, 5, CFG_ALL_EN},
    /* GPIO3, SPI1_CS, RMII_MDIO */
    {3, 0, 9, 6, 3, 3, 3, 3, 3, 3, CFG_ALL_EN},
    /* GPIO4, TWI2_SCL, SD1_D3, SPI2_CS1, RMII_TXEN */
    {4, 0, 12, 8, 4, 4, 4, 4, 4, 5, CFG_ALL_EN},
    /* GPIO5, TWI2_SDA, SD1_D2, RMII_MDC */
    {5, 0, 15, 10, 5, 5, 5, 5, 5, 4, CFG_ALL_EN},
    /* GPIO6, TWI0_SDA, SD1_D1, RMII_RXD1*/
    {6, 0, 18, 12, 6, 6, 6, 6, 6, 4, CFG_ALL_EN},
    /* GPIO7, TWI0_SCL, SD1_D0, RMII_TXD0 */
    {7, 0, 21, 14, 7, 7, 7, 7, 7, 4, CFG_ALL_EN},
    /* GPIO8, SD1_MCLK, SPI2_CLK, RMII_TXD1 */
    {8, 0, 24, 16, 8, 8, 8, 8, 8, 6, CFG_ALL_EN},
    /* GPIO9, CSI_HSYNC, SD1_CMD, SPI2_DIN(IO1) */
    {9, 0, 27, 18, 9, 9, 9, 9, 9, 4, CFG_ALL_EN},
    /* GPIO10, CSI_VSYNC, SPI2_DOUT(IO0), TWI1_SCL */
    {10, 1, 0, 20, 10, 10, 10, 10, 10, 4, CFG_ALL_EN},
    /* GPIO11, CSI0_SCLK, PWM2, SPI2_CS0, RMII_RXD0, TWI1_SDA */
    {11, 1, 3, 22, 11, 11, 11, 11, 11, 7, CFG_ALL_EN},
    /* GPIO16, SD1_MCLK, PWM0, UART2_RXD */
    {16, 1, 21, 0, 16, 16, 16, 16, 16, 4, CFG_ALL_EN},
    /* GPIO17, CSI_DO, SPI1_CS, SD1_CMD, PWM3, UART2_TXD */
    {17, 1, 24, 2, 17, 17, 17, 17, 17, 6, CFG_ALL_EN},
    /* GPIO18, TWI2_SCL, CSI_D1, SPI1_CLK */
    {18, 1, 27, 4, 18, 18, 18, 18, 18, 4, CFG_ALL_EN},
    /* GPIO19, TWI2_SDA, CSI_D2, SPI1_DOUT(IO0) */
    {19, 2, 0, 6, 19, 19, 19, 19, 19, 4, CFG_ALL_EN},
    /* GPIO20, TWI0_SDA, CSI_D3 */
    {20, 2, 3, 8, 20, 20, 20, 20, 20, 3, CFG_ALL_EN},
    /* GPIO21, TWI0_SCL, CSI_D5 */
    {21, 2, 6, 10, 21, 21, 21, 21, 21, 3, CFG_ALL_EN},
    /* CSI0_CLK, CSI_D6, GPIO22 */
    {22, 2, 9, 12, 22, 22, 22, 22, 22, 4, CFG_ALL_EN},
    /* GPIO23, CSI_D4 */
    {23, 2, 12,14, 23, 23, 23, 23, 23, 3, CFG_ALL_EN},
    /* GPIO24, SPI1_CS, SD1_D1 */
    {24, 2, 18, 14, 24, 24, 24, 24, 24, 3, CFG_ALL_EN},
    /* GPIO25, SPI1_CLK, SD1_D0 */
    {25, 2, 21, 14, 25, 25, 25, 25, 25, 3, CFG_ALL_EN},
    /* GPIO26, SPI1_DOUT(IO0), SD1_CMD */
    {26, 2, 24, 14, 26, 26, 26, 26, 26, 3, CFG_ALL_EN},
    /* GPIO27, SPI_DIN(IO1), SD1_MCLK, PWM1 */
    {27, 2, 27, 14, 27, 27, 27, 27, 27, 4, CFG_ALL_EN},

    /* GPIO31, SD0_D2, eMMC_D3, SD1_D1 */
    {31, 3, 6, 30, 31, 31, 31, 31, 31, 5, CFG_ALL_EN},
    /* GPIO32, SD0_D3, eMMC_D0, SD1_D0 */
    {32, 3, 9, 0, 0, 0, 0, 0, 0, 5, CFG_ALL_EN},
    /* GPIO33, SD0_CMD, eMMC_CMD, SD1_CMD */
    {33, 3, 12, 2, 1, 1, 1, 1, 1, 4, CFG_ALL_EN},
    /* GPIO34, SD0_MCLK, eMMC_MCLK, SD1_MCLK */
    {34, 3, 15, 4, 2, 2, 2, 2, 2, 5, CFG_ALL_EN},
    /* GPIO35, SD0_D0, eMMC_D1, SD1_D3 */
    {35, 3, 18, 6, 3, 3, 3, 3, 3, 5, CFG_ALL_EN},
    /* GPIO36, SD0_D1, eMMC_D2, SD1_D2 */
    {36, 3, 21, 8, 4, 4, 4, 4, 4, 5, CFG_ALL_EN},
    /* GPIO only */
    {37, 0, 0, 10, 5, 5, 5, 5, 5, 1, CFG_ALL_EN},
    /* GPIO42, eMMC_D3, SPI0_WP(IO1) */
    {42, 4, 9, 20, 10, 10, 10, 10, 10, 3, CFG_ALL_EN},
    /* GPIO43, eMMC_D0, SPI0_DIN(IO1) */
    {43, 4, 12, 22, 11, 11, 11, 11, 11, 3, CFG_ALL_EN},
    /* GPIO44, eMMC_D1, SPI0_CS */
    {44, 4, 15, 24, 12, 12, 12, 12, 12, 3, CFG_ALL_EN},
    /* GPIO45, eMMC_D2, SPI0_HOLD(IO3) */
    {45, 4, 18, 26, 13, 13, 13, 13, 13, 3, CFG_ALL_EN},
    /* GPIO46, eMMC_MCLK, SPI0_CLK */
    {46, 4, 21, 28, 14, 14, 14, 14, 14, 3, CFG_ALL_EN},
    /* GPIO47, eMMC_CMD, SPI0_DOUT(IO0 */
    {47, 4, 24, 30, 15, 15, 15, 15, 15, 3, CFG_ALL_EN},
    /* GPIO48, PWM0, UART1_TXD, TWI1_SCL */
    {48, 4, 27, 0, 16, 16, 16, 16, 16, 4, CFG_ALL_EN},
    /* GPIO49, PWM1, UART1_RXD, TWI1_SDA */
    {49, 5, 0, 2, 17, 17, 17, 17, 17, 4, CFG_ALL_EN},
    /* GPIO50*/
    {50, 0, 0, 4, 18, 18, 18, 18, 18, 1, CFG_ALL_EN},
    /* GPIO51, UART0_TXD */
    {51, 5, 6, 6, 19, 19, 19, 19, 19, 2, CFG_ALL_EN},
    /* GPIO52, UART0_RXD */
    {52, 5, 9, 8, 20, 20, 20, 20, 20, 2, CFG_ALL_EN},
    /* GPIO55, SPI2_CS0, UART2_RXD, PWM3 */
    {55, 5, 18, 12, 23, 23, 23, 23, 23, 4, CFG_ALL_EN},
    /* GPIO56, SPI2_CLK, UART2_TXD, PWM4*/
    {56, 5, 21, 16, 24, 24, 24, 24, 24, 4, CFG_ALL_EN},
    /* GPIO58, SPI2_CS1, UART2_RTS, TWI1_SDA */
    {58, 5, 27, 20, 26, 26, 26, 26, 26, 5, CFG_ALL_EN},
    /* GPIO59, SPI2_DOUT(IO0), PWM1 */


    /* SPI2_CS0, GPIO65, UART2_RXD, I2S_MCLK, SD1_MCLK */
    {65, 6, 18, 2, 1, 1, 1, 1, 1, 6, CFG_ALL_EN},
    /* SPI2_CLK, GPIO66, UART2_TXD, I2S_BCLK, SD1_D2 */
    {66, 6, 21, 4, 2, 2, 2, 2, 2, 6, CFG_ALL_EN},
    /* GPIO67, PWM2, UART2_CTS, I2S_LRCLK, SD1_D3 */
    {67, 6, 24, 6, 3, 3, 3, 3, 3, 7, CFG_ALL_EN},
    /* SPI2_CS1, GPIO68, PWM3, I2S_DOUT, SD1_D0 */
    {68, 6, 27, 8, 4, 4, 4, 4, 4, 7, CFG_ALL_EN},
    /* SPI2_DOUT(IO0),GPIO69, UART2_RTS, I2S_DIN, SD1_D1*/
    {69, 7, 0, 10, 5, 5, 5, 5, 5, 6, CFG_ALL_EN},
    /* GPIO70, AIN1 */
    {70, 8, 24, 12, 6, 6, 6, 6, 6, 2, CFG_ALL_EN},
    /* GPIO71, AIN0 */
    {71, 8, 27, 14, 7, 7, 7, 7, 7, 2, CFG_ALL_EN},

    /* SYS_RSTN_OUT, GPIO98 */
    {98, 8, 21, 4, 2, 2, 2, 2, 2, 2, CFG_ALL_EN},
    /* MIPI_RX_L0P, CSI_D7, GPIO99 */
    {99, 9, 0, 0, 0, 6, 12, 0, 0, 3, CFG_ALL_EN},
    /* MIPI_RX_L0N, CSI_CLK, GPIO100 */
    {100, 9, 3, 0, 1, 7, 13, 0, 0, 3, CFG_ALL_EN},
    /* MIPI_RX_L1P, CSI_D9, GPIO101 */
    {101, 9, 12, 0, 2, 8, 14, 0, 0, 3, CFG_ALL_EN},
    /* MIPI_RX_L1N, CSI_D8, GPIO102 */
    {102, 9, 15, 0, 3, 9, 15, 0, 0, 3, CFG_ALL_EN},
    /* MIPI_RX_L2P, CSI_D10, GPIO103 */
    {103, 9, 18, 0, 4, 10, 16, 0, 0, 3, CFG_ALL_EN},
    /* MIPI_RX_L2N, CSI_D11, GPIO104 */
    {104, 9, 21, 0, 5, 11, 17, 0, 0, 3, CFG_ALL_EN},
#endif
};

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

static int ak3918ev300l_pmx_set(int pin, int sel)
{
    struct gpio_sharepin *p_ak_sharepin;
    unsigned int regval;
    int offset = 0;

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
    
    if (ak_sharepin[pin].func_mux_max == 1) { // skip GPIO37 and GPIO50
        return 0;
    }

    offset = p_ak_sharepin->funcmux_reg_offset;

    regval = __raw_readl(AK_SHAREPIN_CON0+4*p_ak_sharepin->funcmux_reg_no);
    regval &= ~(0x7 << offset);
    regval |= ((sel & 0x7) << offset);
    __raw_writel(regval, AK_SHAREPIN_CON0+4*p_ak_sharepin->funcmux_reg_no);

    pr_debug("PIN_%d: FUNC(%d) 0x%p:0x%x\n",
            pin, sel, AK_SHAREPIN_CON0+4*p_ak_sharepin->funcmux_reg_no,
            __raw_readl(AK_SHAREPIN_CON0+4*p_ak_sharepin->funcmux_reg_no));

    return 0;

}

/**
*@brief: pin_set_pull_enable
*@param[in] int pin
*@param[in] int enable
*@return: int
**/
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
        AK_PPU_PPD_EN_REG(pin),
        __raw_readl(AK_PPU_PPD_EN_REG(pin)));

    return 0;
}

/**
*@brief: pin_set_pull_polarity
*@param[in] int pin
*@param[in] int pullup
*@return: int
**/
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
        regval |= (0x1 << p_ak_sharepin->pupd_sel_offset);
    } else {
        regval &= (~(0x1 << p_ak_sharepin->pupd_sel_offset));
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

    regval = __raw_readl(pad_drv_reg);
    regval &= ~(0x3 << p_ak_sharepin->pad_drv_offset);
    regval |= ((strength & 0x3) << p_ak_sharepin->pad_drv_offset);

    __raw_writel(regval, pad_drv_reg);

    pr_debug("PIN_%d: DRV(%d) 0x%p:0x%x\n", pin, strength,
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
        regval &= (~(0x1 << p_ak_sharepin->pad_sl_offset));
    } else {
        regval |= (0x1 << p_ak_sharepin->pad_sl_offset);
    }
    __raw_writel(regval, AK_GPIO_SLEW_RATE_REG(pin));

    pr_debug("PIN_%d: SR(%d) 0x%p:0x%x\n", pin, fast,
            AK_GPIO_SLEW_RATE_REG(pin),
            __raw_readl(AK_GPIO_SLEW_RATE_REG(pin)));

    return 0;
}

static int pin_set_schmitt(int pin, int enable)
{
    struct gpio_sharepin *p_ak_sharepin;
    unsigned int regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    if (!(p_ak_sharepin->gpio_cfg & CFG_SCHMITT_EN)) {
        return -EPERM;
    }

    regval = __raw_readl(AK_GPIO_ST_REG(pin));
    if (enable) {
        regval |= (0x1 << p_ak_sharepin->pad_st_offset);
    } else {
        regval &= ~(0x1 << p_ak_sharepin->pad_st_offset);
    }

    __raw_writel(regval, AK_GPIO_ST_REG(pin));
    pr_debug("PIN_%d: schmitt_EN(%d) 0x%p:0x%x\n",
            pin, enable, AK_GPIO_ST_REG(pin),
            __raw_readl(AK_GPIO_ST_REG(pin)));

    return 0;
}

static int pin_set_gpio_direction(int pin, int is_input)
{
    struct gpio_sharepin *p_ak_sharepin;
    unsigned int regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }
    
    regval = __raw_readl(AK_GPIO_DIR_REG(pin));
    if (is_input) {
        regval &= ~(0x1 << AK_GPIO_REG_SHIFT(pin));
    } else {
        regval |= (0x1 << AK_GPIO_REG_SHIFT(pin));
    }
    __raw_writel(regval, AK_GPIO_DIR_REG(pin));

    pr_debug("PIN_%d: is_input(%d) 0x%p:0x%x\n",
            pin, is_input, AK_GPIO_DIR_REG(pin),
            __raw_readl(AK_GPIO_DIR_REG(pin)));

    return 0;
}

static int pin_set_gpio_output_level(int pin, int level)
{
    struct gpio_sharepin *p_ak_sharepin;
    unsigned int regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }
    
    regval = __raw_readl(AK_GPIO_OUT_REG(pin));
    regval &= ~(0x1 << AK_GPIO_REG_SHIFT(pin));
    regval |= ((level & 1) << AK_GPIO_REG_SHIFT(pin));
    __raw_writel(regval, AK_GPIO_OUT_REG(pin));

    pr_debug("PIN_%d: level(%d) 0x%p:0x%x\n",
            pin, level, AK_GPIO_OUT_REG(pin),
            __raw_readl(AK_GPIO_OUT_REG(pin)));

    return 0;
}

static int ak3918ev300l_pinconf_set(int pin, unsigned int configs)
{
    unsigned char pupd, drive, ie, slew, st, dir, output;

    /*
    * configs
    * bit[0] pull up or pull down selection
    * bit[4] pull up or pull down enable
    * bit[9:8] drive
    * bit[12:15] gpio direction
    *   bit[12]: direction. 0 is output, 1 is input
    *   bit[15]: enable direction. 1 is enable
    * bit[16]:input enable
    * bit[20-23]: gpio output level
    *   bit[20]: 0 is low level, 1 is high level
    *   bit[23]: enable gpio output level. 1 is enable
    * bit[24]: slew rate
    * bit[28]: schmitt enable
    */
    pupd = (configs & 0xFF);
    drive = ((configs>>8) & 0xFF);
    dir = ((configs>>12) & 0x0F);
    ie = ((configs>>16) & 0xFF);
    output = ((configs>>20) & 0x0F);
    slew = ((configs>>24) & 0x0F);
    st = ((configs>>28) & 0x0F);

    pin_set_pull_enable(pin, (pupd & 0x10) ? 1:0);
    pin_set_pull_polarity(pin, (pupd & 0x01) ? 0:1);
    pin_set_drive(pin, drive & 0x3);
    pin_set_input_enable(pin, ie & 0x1);
    pin_set_slew_rate(pin, slew & 0x1);
    pin_set_schmitt(pin, st & 0x1);

    if (dir & 0x8) { // gpio direction enable?
        if (dir & 1) { // input?
            pin_set_gpio_direction(pin, 1);
        } else {
            pin_set_gpio_direction(pin, 0);
        }
    }
    if ((output & 0x8) && (dir & 0x8) && ((dir & 1) == 0)) {
            pin_set_gpio_output_level(pin, (output & 1));
    }

    return 0;
}

static int ak3918ev300l_pinctrl_set_state(struct udevice *dev,
                    struct udevice *config)
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
        ak3918ev300l_pmx_set(pin, sel);
        ak3918ev300l_pinconf_set(pin, conf);
    }

    return 0;
}

const struct pinctrl_ops ak3918ev300l_pinctrl_ops  = {
    .set_state = ak3918ev300l_pinctrl_set_state,
};

static void disable_gpio_retention(void)
{
    u32 regval;

    regval = __raw_readl(AK_PPU_PPD_SEL3);
    regval &= ~(1UL << 31);
    __raw_writel(regval, AK_PPU_PPD_SEL3);
}

static int ak3918ev300l_pinctrl_probe(struct udevice *dev)
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

    disable_gpio_retention();
    return 0;
}

static const struct udevice_id ak3918ev300l_pinctrl_match[] = {
    { .compatible = "anyka,ak3918ev300l-pinctrl"},
    {}
};

U_BOOT_DRIVER(ak3918ev300l_pinctrl) = {
    .name = "pinctrl_ak3918ev300l",
    .id = UCLASS_PINCTRL,
    .of_match = ak3918ev300l_pinctrl_match,
    .probe = ak3918ev300l_pinctrl_probe,
    .ops = &ak3918ev300l_pinctrl_ops,
};
