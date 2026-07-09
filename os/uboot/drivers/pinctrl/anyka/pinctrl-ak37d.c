// SPDX-License-Identifier: GPL-2.0+
/*
 * ak37d pinctrl driver
 *
 * anyka
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <linux/io.h>
#include <linux/err.h>
#include <dt-bindings/pinctrl/ak_37d_pinctrl.h>

#define AK_GPIO_CONF_INVALID    (-1)

#define MIPI0_BASE              ((void __iomem *)0x20400000)
#define MIPI1_BASE              ((void __iomem *)0x20480000)

#define AK_VA_SYSCTRL           ((void __iomem *)(0x08000000))

#define AK_ANALOG_CTRL_REG3     (AK_VA_SYSCTRL + 0x0A4)

#define AK_SHAREPIN_CON0        (AK_VA_SYSCTRL + 0x178)
#define AK_SHAREPIN_CON1        (AK_VA_SYSCTRL + 0x17C)
#define AK_SHAREPIN_CON2        (AK_VA_SYSCTRL + 0x180)
#define AK_SHAREPIN_CON3        (AK_VA_SYSCTRL + 0x184)
#define AK_SHAREPIN_CON4        (AK_VA_SYSCTRL + 0x188)
#define AK_SHAREPIN_CON5        (AK_VA_SYSCTRL + 0x18C)
#define AK_SHAREPIN_CON6        (AK_VA_SYSCTRL + 0x190)

#define AK_GPIO_DRIVE_CON0      (AK_VA_SYSCTRL + 0x1D0)
#define AK_GPIO_DRIVE_CON1      (AK_VA_SYSCTRL + 0x1D4)
#define AK_GPIO_DRIVE_CON2      (AK_VA_SYSCTRL + 0x1D8)
#define AK_GPIO_DRIVE_CON3      (AK_VA_SYSCTRL + 0x1DC)
#define AK_GPIO_DRIVE_CON4      (AK_VA_SYSCTRL + 0x1E0)
#define AK_GPIO_DRIVE_CON5      (AK_VA_SYSCTRL + 0x1E4)

#define AK_PPU_PPD_EN0          (AK_VA_SYSCTRL + 0x194)
#define AK_PPU_PPD_EN1          (AK_VA_SYSCTRL + 0x198)
#define AK_PPU_PPD_EN2          (AK_VA_SYSCTRL + 0x19C)

#define AK_PPU_PPD_SEL0         (AK_VA_SYSCTRL + 0x1A0)
#define AK_PPU_PPD_SEL1         (AK_VA_SYSCTRL + 0x1A4)
#define AK_PPU_PPD_SEL2         (AK_VA_SYSCTRL + 0x1E8)

#define AK_PPU_PPD_EN_SEL       (AK_VA_SYSCTRL + 0x1F8)

#define AK_GPIO_SLEW_RATE       (AK_VA_SYSCTRL + 0x1FC)

#define AK_INTERRUPT_STATUS     (AK_VA_SYSCTRL + 0x30)
#define AK_ALWAYS_ON_PMU_CTRL0  (AK_VA_SYSCTRL + 0xDC)
#define AK_ALWAYS_ON_PMU_CTRL1  (AK_VA_SYSCTRL + 0xE0)

#define AK_GPIO_IE_CON0         (AK_VA_SYSCTRL + 0x1EC)
#define AK_GPIO_IE_CON1         (AK_VA_SYSCTRL + 0x1F0)
#define AK_GPIO_IE_CON2         (AK_VA_SYSCTRL + 0x1F4)

/*
 *H3D-B changed drive map
 *00->10
 *01->11
 *10->00
 *11->01
 * */
#define DRIVE_STRENGTH(strength)    ((strength) ^ 0x2)
//#define DRIVE_STRENGTH(strength)  (strength)

/*
 *  bit 8               7/6/5   4/3     2/1/0
 *      pupd_enable_bit value   width   AO,I,I/O
 * */
#define GPIO_CLASS_IO   (0x0)
#define GPIO_CLASS_I    (0x1)
#define GPIO_CLASS_O    (0x2)
#define GPIO_CLASS_AO   (0x3)

#define SET_GPIO_CLASS(c)           ((c) & 0x7)
#define SET_SHAREPIN_BITS_WIDTH(w)  (((w) & 0x3) << 3)
#define SET_VALUE_TO_GPIO(v)        (((v) & 0x7) << 5)
#define SET_PUPD_ENABLE_BIT(e)      (((e) & 0x1) << 8)

#define GET_GPIO_CLASS(c)       ((c) & 0x7)
#define GET_SHAREPIN_WIDTH(c)   (((c) >> 3) & 0x3)
#define GET_GPIO_SET_VALUE(c)   (((c) >> 5) & 0x7)
#define GET_PUPD_ENABLE_BIT(c)  (((c) >> 8) & 0x1)

/*
 *GPIO_CFG - gpio
 *GPI_CFG -  GPI
 *GPO_CFG -  GPO
 *AO_CFG -   GPIO_AO
 * */
#define GPIO_CFG(width, value)  (SET_VALUE_TO_GPIO(value) |\
        SET_SHAREPIN_BITS_WIDTH(width) | SET_GPIO_CLASS(GPIO_CLASS_IO))
#define GPI_CFG(width, value)   (SET_VALUE_TO_GPIO(value) |\
        SET_SHAREPIN_BITS_WIDTH(width) | SET_GPIO_CLASS(GPIO_CLASS_I))
#define GPO_CFG(width, value)   (SET_VALUE_TO_GPIO(value) |\
        SET_SHAREPIN_BITS_WIDTH(width) | SET_GPIO_CLASS(GPIO_CLASS_O))
#define AO_CFG(width, value)    (SET_VALUE_TO_GPIO(value) |\
        SET_SHAREPIN_BITS_WIDTH(width) | SET_GPIO_CLASS(GPIO_CLASS_AO))

/*all capacity can't cfg*/
#define ALL_DIS         (0)
/*expect drive fixed, pull up fixed*/
#define DFIX_UP         (1)
/*expect drive fixed, pull down fixed*/
#define DFIX_DOWN       (2)
/*expect drive fixed, pull up fixed, IE enable fixed*/
#define DFIX_UP_IEEN    (3)
/*expect drive fixed, pull down fixed, IE enable fixed*/
#define DFIX_DOWN_IEEN  (4)
/*expect IE enable fixed*/
#define IEEN            (5)
/*expect drive fixed, IE enable fixed*/
#define DFIX_IEEN       (6)
/*all capacity support*/
#define ALL_EN          (0xf)

/*
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
#define CFG_ALL_EN              (CFG_DRV_EN|CFG_INPUT_EN|\
                                CFG_PUPD_EN|CFG_PUPD_SEL|CFG_SLEW_RATE)
*/

struct pin_pos {
    int offset;
    int width;
    int gpio0;
    int func0;
    int gpio1;
    int func1;
    int gpio2;
    int func2;
    int gpio3;
    int func3;
};

struct gpio_sharepin {
    int pin;
    void __iomem * sharepin_reg;
    void __iomem * drive_reg;
    u32 reg_off;

    void __iomem * pupd_en;
    void __iomem * pupd_sel;
    void __iomem * ie;
    u32 sel_bit;

    int slew_bit;
    int gpio_config;
    int capacity;
};

//share pin func config in AK37Dxx
static struct gpio_sharepin ak_sharepin[] = {
#if defined(CONFIG_SPL_BUILD)
    /* gpio, uart0_rx, pwm4 */
    {1, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 0, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 0, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, uart0_tx, pwm3 */
    {2, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 2, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 1, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* spi0_cs, gpio */
    {12, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 28, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 30, -1, GPIO_CFG(2, 0B01), ALL_EN},
    /* gpio, rgb_d7/mpu_d7 */
    {82, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 28, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 14, 11, GPIO_CFG(2, 0B00), ALL_EN},
    /* spi0_din, gpio */
    {67, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 30, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 31, -1, GPIO_CFG(2, 0B01), DFIX_UP},
    /* spi0_dout, gpio */
    {68, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 0, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 0, -1, GPIO_CFG(2, 0B01), DFIX_UP},
    /* spi0_wp, gpio */
    {69, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 2, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 1, -1, GPIO_CFG(2, 0B01), DFIX_UP},
    /* spi0_hold, gpio */
    {70, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 4, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 2, -1, GPIO_CFG(2, 0B01), DFIX_UP},
#else
    /* goio, uart3_rx */
    {0, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 8, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 20, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, uart0_rx, pwm4 */
    {1, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 0, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 0, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, uart0_tx, pwm3 */
    {2, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 2, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 1, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, pwm0, mci2_d0 */
    {3, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 4, AK_PPU_PPD_EN0,
        0, AK_GPIO_IE_CON0, 2, -1, GPIO_CFG(2, 0B00), DFIX_UP},
    /* gpio, pwm1, uart3_tx, mci2_cmd */
    {4, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 6, AK_PPU_PPD_EN0,
        0, AK_GPIO_IE_CON0, 3, -1, GPIO_CFG(2, 0B00), DFIX_DOWN},
    /* gpio, pwm2, uart3_rx, mci2_clk */
    {5, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 8, AK_PPU_PPD_EN0,
        0, AK_GPIO_IE_CON0, 4, -1, GPIO_CFG(2, 0B00), DFIX_DOWN},
    /* gpio, uart1_rx, pwm0, i2c3_scl */
    {6, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 10, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 5, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, pwm1, i2c3_sda */
    {7, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 12, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 6, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, uart1_cts, irda_rx */
    {8, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 14, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 7, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, uart1_rts */
    {9, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 16, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 8, 28, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rmii_mdc, i2c3_scl, pwm0 */
    {10, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 18, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 9, 28, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rmii_mdio, i2c3_sda */
    {11, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 20, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 10, 28, GPIO_CFG(2, 0B00), ALL_EN},
    /* spi0_cs, gpio */
    {12, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 28, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 30, -1, GPIO_CFG(2, 0B01), ALL_EN},
    /* gpio, rmii_txen */
    {13, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 22, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 11, 28, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rmii_txd0, mci2_d3 */
    {14, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 24, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 12, 28, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rmii_txd1,mci2_d2 */
    {15, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 26, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 13, 28, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, gpo0_ao */
    {16, 0, 0, 0, 0, 0, 0, 0, -1, AO_CFG(0, 0B0), ALL_DIS},
    /* gpio, gpo1_ao */
    {17, 0, 0, 1, 0, 0, 0, 0, -1, AO_CFG(0, 0B0), ALL_DIS},
    /* gpio, uart3_txd */
    {18, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 6, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 19, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rmii_rxd0, mci2_clk */
    {19, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 28, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 14, 28, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rmii_rxd1, mci2_cmd */
    {20, AK_SHAREPIN_CON0, AK_GPIO_DRIVE_CON0, 30, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 15, 28, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, gpo2_ao */
    {21, 0, 0, 2, 0, 0, 0, 0, -1, AO_CFG(0, 0B0), ALL_DIS},
    /* gpio, gpo3_ao */
    {22, 0, 0, 3, 0, 0, 0, 0, -1, AO_CFG(0, 0B0), ALL_DIS},
    /* gpio, rmii_rxer, mci2_d1, pwm3 */
    {23, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 0, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 16, 28,
        GPIO_CFG(2, 0B00), DFIX_DOWN_IEEN},
    /* gpio, rmii_rxdv, mci2_d0, pwm4 */
    {24, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 2, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 17, 28, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, cis1_sclk */
    {25, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 6, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 19, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, cis_pclk, cis1_sclk */
    {26, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 8, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 20, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, cis_hsync, pwm3, i2c1_scl */
    {27, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 10, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 21, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, cis_vsync, pwm4, i2c1_sda */
    {28, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 12, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 22, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, pwm0 */
    {29, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 10, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 21, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, pwm1 */
    {30, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 12, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 22, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, i2c0_scl */
    {31, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 14, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 23, -1,
        GPIO_CFG(2, 0B00) | SET_PUPD_ENABLE_BIT(1), DFIX_UP},
    /* gpio, i2c0_sda */
    {32, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 16, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 24, -1,
        GPIO_CFG(2, 0B00) | SET_PUPD_ENABLE_BIT(1), DFIX_UP},
    /* gpio, mci0_cmd */
    {33, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 18, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 25, 29, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, mci0_clk, uart2_tx */
    {34, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 20, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 26, 29, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, mci0_d0, uart2_rx, jtag_rstn */
    {35, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 22, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 27, 29, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, mci0_d1, uart1_rx, jtag_rtck */
    {36, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 24, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 28, 29, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, mci0_d2, pwm1, uart1_tx */
    {37, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 26, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 29, 29, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, mci0_d3, pwm1, jtag_tck */
    {38, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 28, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 30, 29, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, mci0_d4, pwm2, jtag_tms */
    {39, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 30, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 31, 29, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, mci0_d5, pwm3, jtag_tdi */
    {40, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 0, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 0, 29, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, mci0_d6, pwm4, jtag_tdout */
    {41, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 2, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 1, 29, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, mci0_d7 */
    {42, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 4, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 2, 29, GPIO_CFG(2, 0B00), ALL_EN},

    /* gpio, mci1_cmd, spi1_cs */
    {43, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 6, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 3, -1, GPIO_CFG(2, 0B00), DFIX_UP},
    /* gpio, mci1_clk, spi1_sclk */
    {44, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 8, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 4, -1, GPIO_CFG(2, 0B00), DFIX_DOWN},
    /* gpio, mci1_d0, spi1_hold */
    {45, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 10, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 5, -1, GPIO_CFG(2, 0B00), DFIX_UP},

    
    /* gpio, mci1_d1, spi1_wp */
    {46, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 12, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 6, -1, GPIO_CFG(2, 0B00), DFIX_UP},
    /* gpio, mci1_d2, spi1_dout */
    {47, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 14, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 7, -1, GPIO_CFG(2, 0B00), DFIX_UP},
    /* gpio, mci1_d3, spi1_din */
    {48, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 16, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 8, -1, GPIO_CFG(2, 0B00), DFIX_UP},
    /* gpio, opclk, mci2_d0, pwm1 */
    {49, AK_SHAREPIN_CON1, AK_GPIO_DRIVE_CON1, 4, AK_PPU_PPD_EN0,
        AK_PPU_PPD_SEL0, AK_GPIO_IE_CON0, 18, 28, GPIO_CFG(2, 0B00), IEEN},
    /* gpio, pwm0, uart1_tx */
    {50, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 18, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 9, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, pwm1, uart1_cts */
    {51, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 20, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 10, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, pwm2, uart1_rx */
    {52, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 22, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 11, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, i2s_dout, uart1_rts*/
    {53, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 24, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 12, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, i2s_mclk */
    {54, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 26, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 13, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, i2s_bclk */
    {55, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 28, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 14, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, i2s_lrclk */
    {56, AK_SHAREPIN_CON2, AK_GPIO_DRIVE_CON2, 30, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 15, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, i2s_din */
    {57, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 0, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 16, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, pwm3, i2c2_sda */
    {58, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 2, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 17, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, pwm4, i2c2_scl */
    {59, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 4, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 18, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, pwm0, ain1 */
    {60, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 14, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 23, -1,
        GPIO_CFG(2, 0B00), DFIX_DOWN_IEEN},
    /* gpio, pwm2 */
    {61, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 16, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 24, -1, GPIO_CFG(2, 0B00), DFIX_UP},
    /* gpio, pwm1 */
    {62, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 18, AK_PPU_PPD_EN1, 
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 25, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio */
    {63, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 20, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 26, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio */
    {64, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 22, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 27, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio */
    {65, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 24, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 28, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio */
    {66, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 26, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 29, -1, GPIO_CFG(2, 0B00), ALL_EN},
    /* spi0_din, gpio */
    {67, AK_SHAREPIN_CON3, AK_GPIO_DRIVE_CON3, 30, AK_PPU_PPD_EN1,
        AK_PPU_PPD_SEL1, AK_GPIO_IE_CON1, 31, -1, GPIO_CFG(2, 0B01), DFIX_UP},
    /* spi0_dout, gpio */
    {68, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 0, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 0, -1, GPIO_CFG(2, 0B01), DFIX_UP},
    /* spi0_wp, gpio */
    {69, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 2, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 1, -1, GPIO_CFG(2, 0B01), DFIX_UP},
    /* spi0_hold, gpio */
    {70, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 4, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 2, -1, GPIO_CFG(2, 0B01), DFIX_UP},
    /* gpio, rgb_vogate/mpu_rd */
    {71, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 6, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 3, 0, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_vovsync/mpu_a0 */
    {72, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 8, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 4, 1, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_vohsync/mpu_cs */
    {73, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 10, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 5, 2, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_vopclk/mpu_wr,rgb_pclk */
    {74, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 12, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 6, 3, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d0/mpu_d0 */
    {75, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 14, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 7, 4, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d1/mpu_d1 */
    {76, AK_SHAREPIN_CON4, 0/*Note:b[17:18] drive_gpio76_92_cfg*/, 16,
        AK_PPU_PPD_EN2, AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 8, 5,
        GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d2/mpu_d2 */
    {77, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 18, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 9, 6, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d3/mpu_d3 */
    {78, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 20, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 10, 7, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d4/mpu_d4 */
    {79, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 22, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 11, 8, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d5/mpu_d5 */
    {80, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 24, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 12, 9, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d6/mpu_d6 */
    {81, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 26, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 13, 10, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d7/mpu_d7 */
    {82, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 28, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 14, 11, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d8/mpu_d8, i2c2_scl */
    {83, AK_SHAREPIN_CON4, AK_GPIO_DRIVE_CON4, 30, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 15, 12, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d9/mpu_d9, i2c2_sda */
    {84, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 0, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 16, 13, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d10/mpu_d10, i2s_din */
    {85, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 2, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 17, 14, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d11/mpu_d11, i2s_lrclk */
    {86, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 4, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 18, 15, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d12/mpu_d12, i2s_bclk */
    {87, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 6, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 19, 16, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d13/mpu_d13, i2s_mclk */
    {88, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 8, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 20, 17, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d14/mpu_d14, i2s_dout */
    {89, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 10, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 21, 18, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d15/mpu_d15, pwm5 */
    {90, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 12, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 22, 19, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d16/mpu_d16, mci2_d2 */
    {91, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 14, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 23, 20, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d17/mpu_d17, mci2_d3 */
    {92, AK_SHAREPIN_CON5, 0/*Note:b[17]&[6] drive_gpio76_92_cfg*/, 16,
        AK_PPU_PPD_EN2, AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 24, 21,
        GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d18, spi1_cs, mci2_cmd */
    {93, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 18, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 25, 22, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d19, spi1_sclk, mci2_clk */
    {94, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 20, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 26, 23, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d20, spi1_dout, mci2_d0 */
    {95, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 22, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 27, 24, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d21, spi1_din, mci2_d1 */
    {96, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 24, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 28, 25, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d22, spi1_hold, i2c2_sda */
    {97, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 26, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 29, 26, GPIO_CFG(2, 0B00), ALL_EN},
    /* gpio, rgb_d23, spi1_wp, i2c2_scl, spi1_din */
    {98, AK_SHAREPIN_CON5, AK_GPIO_DRIVE_CON5, 28, AK_PPU_PPD_EN2,
        AK_PPU_PPD_SEL2, AK_GPIO_IE_CON2, 30, 27, GPIO_CFG(3, 0B000), ALL_EN},
    /* gpio */
    {99, 0, AK_GPIO_DRIVE_CON5, 30, AK_PPU_PPD_EN2, AK_PPU_PPD_SEL2,
        AK_GPIO_IE_CON2, 31, -1, GPIO_CFG(0, 0B0), ALL_EN},
    /* gpio */
    {100, 0, AK_PPU_PPD_EN_SEL, 0, AK_PPU_PPD_EN_SEL,
        0/*Note: ie_reg3_gpio100_103_cfg*/, 0/*Note: ie_reg3_gpio100_103_cfg*/,
        10, -1, GPIO_CFG(2, 0B0), ALL_EN},
    /* gpio */
    {101, 0, AK_PPU_PPD_EN_SEL, 2, AK_PPU_PPD_EN_SEL,
        0/*Note: ie_reg3_gpio100_103_cfg*/, 0/*Note: ie_reg3_gpio100_103_cfg*/,
        11, -1, GPIO_CFG(2, 0B0), ALL_EN},
    /* gpio */
    {102, 0, AK_PPU_PPD_EN_SEL, 4, AK_PPU_PPD_EN_SEL,
        0/*Note: ie_reg3_gpio100_103_cfg*/, 0/*Note: ie_reg3_gpio100_103_cfg*/,
        12, -1, GPIO_CFG(2, 0B0), ALL_EN},
    /* gpio */
    {103, 0, AK_PPU_PPD_EN_SEL, 6, AK_PPU_PPD_EN_SEL,
        0/*Note: ie_reg3_gpio100_103_cfg*/, 0/*Note: ie_reg3_gpio100_103_cfg*/,
        13, -1, GPIO_CFG(2, 0B0), ALL_EN},

#if 0
    /* gpio, gpi4_ao */
    {104, 0, 0, 0, 0, 0, 0, 0, -1, AO_CFG(0, 0B0), ALL_DIS},
    /* gpio, gpi3_ao */
    {105, 0, 0, 0, 0, 0, 0, 0, -1, AO_CFG(0, 0B0), ALL_DIS},
    /* gpio, gpi2_ao */
    {106, 0, 0, 0, 0, 0, 0, 0, -1, AO_CFG(0, 0B0), ALL_DIS},
    /* gpio, gpi1_ao */
    {107, 0, 0, 0, 0, 0, 0, 0, -1, AO_CFG(0, 0B0), ALL_DIS},
    /* gpio, gpi0_ao */
    {108, 0, 0, 0, 0, 0, 0, 0, -1, AO_CFG(0, 0B0), ALL_DIS},
    /* gpi0 */
    {109, AK_ANALOG_CTRL_REG3, 0, 25, 0, 0, 0, 0, -1, GPI_CFG(1, 0B1), ALL_DIS},
    /* gpi1 */
    {110, AK_ANALOG_CTRL_REG3, 0, 27, 0, 0, 0, 0, -1, GPI_CFG(1, 0B1), ALL_DIS},
    /* gpi2 */
    {111, AK_SHAREPIN_CON6, 0, 30, 0, 0, 0, 0, -1, GPI_CFG(1, 0B0), DFIX_IEEN},
    /* gpi3 */
    {112, AK_SHAREPIN_CON6, 0, 30, 0, 0, 0, 0, -1, GPI_CFG(1, 0B0), DFIX_IEEN},
    /* gpi4 */
    {113, AK_SHAREPIN_CON6, 0, 29, 0, 0, 0, 0, -1, GPI_CFG(1, 0B0), DFIX_IEEN},
    /* gpi5 */
    {114, AK_SHAREPIN_CON6, 0, 29, 0, 0, 0, 0, -1, GPI_CFG(1, 0B0), DFIX_IEEN},
    /* gpi6 */
    {115, AK_SHAREPIN_CON6, 0, 31, 0, 0, 0, 0, -1, GPI_CFG(1, 0B0), DFIX_IEEN},
    /* gpi7 */
    {116, AK_SHAREPIN_CON6, 0, 31, 0, 0, 0, 0, -1, GPI_CFG(1, 0B0), DFIX_IEEN},
    /* gpi8 */
    {117, AK_SHAREPIN_CON5, 0, 31, 0, 0, 0, 0, -1, GPI_CFG(1, 0B0), DFIX_IEEN},
    /* gpi9 */
    {118, AK_SHAREPIN_CON5, 0, 31, 0, 0, 0, 0, -1, GPI_CFG(1, 0B0), DFIX_IEEN},
    /* gpi10 */
    {119, AK_SHAREPIN_CON5, 0, 31, 0, 0, 0, 0, -1, GPI_CFG(1, 0B0), DFIX_IEEN},
    /* gpi11 */
    {120, AK_SHAREPIN_CON5, 0, 31, 0, 0, 0, 0, -1, GPI_CFG(1, 0B0), DFIX_IEEN},
    /* gpi12 */
    {121, AK_SHAREPIN_CON5, 0, 31, 0, 0, 0, 0, -1, GPI_CFG(1, 0B0), DFIX_IEEN},
    /* gpi13 */
    {122, AK_SHAREPIN_CON5, 0, 31, 0, 0, 0, 0, -1, GPI_CFG(1, 0B0), DFIX_IEEN},
#endif  
#endif
}; 

static struct pin_pos ak_pin_pos[] = {
    /*offset,width, gpio0, func0, gpio1, func1, gpio2, func2, gpio3, func3*/
    /*TWI2_SDA*/
    {22,    2,      58,     2,      97,     3,      84,     2,      -1,     -1},
    /*I2S_DIN*/
    {21,    1,      85,     2,      57,     1,      -1,     -1,     -1,     -1},
    /*I2C_LRCLK*/
    {20,    1,      86,     2,      56,     1,      -1,     -1,     -1,     -1},
    /*I2S_BCLK*/
    {19,    1,      87,     2,      55,     1,      -1,     -1,     -1,     -1},
    /*UART1_CTS*/
    {18,    1,      8,      1,      51,     2,      -1,     -1,     -1,     -1},
    /*UART1_RXD*/
    {16,    2,      6,      1,      36,     2,      52,     2,      -1,     -1},
    /*UART3_RXD*/
    {15,    1,      5,      2,      0,      1,      -1,     -1,     -1,     -1},
    /*TWI3_SDA*/
    {14,    1,      7,      3,      11,     2,      -1,     -1,     -1,     -1},
    /*SPI1_DIN*/
    {12,    2,      48,     2,      96,     2,      98,     4,      -1,     -1},
    /*SPI1_DOUT*/
    {11,    1,      47,     2,      95,     2,      -1,     -1,     -1,     -1},
    /*SPI1_WP*/
    {10,    1,      46,     2,      98,     2,      -1,     -1,     -1,     -1},
    /*SPI1_HOLD*/
    {9,     1,      45,     2,      97,     2,      -1,     -1,     -1,     -1},
    /*SPI1_SCLK*/
    {8,     1,      44,     2,      94,     2,      -1,     -1,     -1,     -1},
    /*SPI1_CS*/
    {7,     1,      43,     2,      93,     2,      -1,     -1,     -1,     -1},
    /*MCI2_D[3]*/
    {6,     1,      14,     2,      92,     2,      -1,     -1,     -1,     -1},
    /*MCI2_D[2]*/
    {5,     1,      15,     2,      91,     2,      -1,     -1,     -1,     -1},
    /*MCI2_D[1]*/
    {4,     1,      23,     2,      96,     3,      -1,     -1,     -1,     -1},
    /*MCI2_D[0]*/
    {2,     2,      3,      2,      24,     2,      49,     2,      95,     3},
    /*MCI2_CMD*/
    {0,     2,      4,      3,      20,     2,      93,     3,      -1,     -1},
};

static inline int pin_gpi_groups(int pin)
{
    return ((pin >= 109) && (pin <= 122)) ? 1 : 0;
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

static void pmu_reg_write(u32 reg, u32 value)
{
    u32 regval;
    
    /*wait bit 17 pmu ready int status to be 1*/
    do {
    } while (!(__raw_readl(AK_INTERRUPT_STATUS) & (1<<17)));

    regval = (0x1<<21)|(0x2<<18)|(0x0<<17)
        |((reg&0x7)<<14)|(value&0x3FFF);
    __raw_writel(regval, AK_ALWAYS_ON_PMU_CTRL0);

    /*wait bit 17 pmu ready int status to be 1*/
    do {
    } while (!(__raw_readl(AK_INTERRUPT_STATUS) & (1<<17)));
}

static u32 pmu_reg_read(u32 reg)
{
    u32 regval;

    /*wait bit 17 pmu ready int status to be 1*/
    do {
    } while (!(__raw_readl(AK_INTERRUPT_STATUS) & (1<<17)));

    regval = (0x1<<21)|(0x2<<18)|(0x1<<17)
        |((reg&0x7)<<14);
    __raw_writel(regval, AK_ALWAYS_ON_PMU_CTRL0);

    /*wait bit 17 pmu ready int status to be 1*/
    do {
    } while (!(__raw_readl(AK_INTERRUPT_STATUS) & (1<<17)));

    return (__raw_readl(AK_ALWAYS_ON_PMU_CTRL1))&0x3FFF;
}

static int gpi_and_mipi_pins_set_share_ex(int pin, int sel)
{
    u32 regval;
    //int mipi_lanes_num;
    void __iomem *mipi_base;

    switch (pin) {
        case 23:
            if (sel == 4) {
                /*here set AIN1 mux to pin GPIO23*/
                regval = __raw_readl(AK_ANALOG_CTRL_REG3);
                regval |= 1<<29;
                __raw_writel(regval, AK_ANALOG_CTRL_REG3);
            } else {
                regval = __raw_readl(AK_ANALOG_CTRL_REG3);
                regval &= ~(1<<29);
                __raw_writel(regval, AK_ANALOG_CTRL_REG3);
            }
            break;

        case 60:
            if (sel == 4) {
                /*here set AIN1 mux to pin GPIO60*/
                regval = __raw_readl(AK_ANALOG_CTRL_REG3);
                regval |= 1<<28;
                __raw_writel(regval, AK_ANALOG_CTRL_REG3);
            } else {
                regval = __raw_readl(AK_ANALOG_CTRL_REG3);
                regval &= ~(1<<28);
                __raw_writel(regval, AK_ANALOG_CTRL_REG3);
            }
            break;

        case 111:
        case 112:
        case 113:
        case 114:
        case 115:
        case 116:
        case 117:
        case 118:
        case 119:
        case 120:
        case 121:
        case 122:
            /*here set mipi-gpi pins extern paramters*/
            if (pin >= 111 && pin <= 116) {
                mipi_base = MIPI0_BASE;
                //mipi_lanes_num = pc->mipi0_lanes_num;
            } else {
                mipi_base = MIPI1_BASE;
                //mipi_lanes_num = pc->mipi1_lanes_num;
            }

            if (sel == 1) {
                /*dvp mode*/
                __raw_writeb(0x7d, mipi_base);
                __raw_writeb(0x3f, mipi_base + 0x20);
                __raw_writeb(0x01, mipi_base + 0xb3);
                //__raw_writeb(ttl_io_voltage & 0xff, mipi_base + 0xb8);
            } else if (sel == 2) {
                /*mipi mode*/
                __raw_writeb(0x7d, mipi_base);
#if 0
                if (mipi_lanes_num == 1)
                    __raw_writeb(0xf8, mipi_base + 0xe0);
                else if (mipi_lanes_num == 2) {
                    /*2lanes mode is default, but I don't know default value*/
                    __raw_writeb(0xf9, mipi_base + 0xe0);
                } else {
                    pr_err("%s err mipi_lanes_num:%d, pin:%d\n",
                            __func__, mipi_lanes_num, pin);
                    return -1;
                }
#endif
            } else if (sel == 0) {
                /*GPI mode*/
                __raw_writeb(0x7d, mipi_base);
                __raw_writeb(0x3f, mipi_base + 0x20);
                __raw_writeb(0x01, mipi_base + 0xb3);

                __raw_writeb(0xb0, mipi_base + 0x0d);//disable pullup
                __raw_writeb(0x08, mipi_base + 0xb8);//2.5V ttl input enable
            } else {
                pr_err("%s don't support sel:%d, pin:%d\n", __func__, sel, pin);
                return -1;
            }
            break;

        case 109:
        case 110:
            /*had cfg in ak_sharepin*/
            break;

        default:
            return -1;
            break;
    }

    return 0;
}
/*end of gpi_and_mipi_pins_set_share_ex*/

static int pin_pos_cfg(int gpio, int fsel)
{
    int i;
    int num;
    int bits = 0;
    u32 regval;

    num = sizeof(ak_pin_pos) / sizeof(ak_pin_pos[0]);
    for (i = 0; i < num; i++) {
        if (gpio == ak_pin_pos[i].gpio0 && fsel == ak_pin_pos[i].func0) {
            bits = 0;
            break;
        }

        if (gpio == ak_pin_pos[i].gpio1 && fsel == ak_pin_pos[i].func1) {
            bits = 0x1;
            break;
        }

        if (gpio == ak_pin_pos[i].gpio2 && fsel == ak_pin_pos[i].func2) {
            bits = 0x2;
            break;
        }

        if (gpio == ak_pin_pos[i].gpio3 && fsel == ak_pin_pos[i].func3) {
            bits = 0x3;
            break;
        }
    }

    if (i < sizeof(ak_pin_pos) / sizeof(ak_pin_pos[0])) {
        regval = __raw_readl(AK_SHAREPIN_CON6);
        regval &= ~((~((~(unsigned long)0) << ak_pin_pos[i].width)) \
                    << ak_pin_pos[i].offset);
        regval |= bits << ak_pin_pos[i].offset;
        __raw_writel(regval, AK_SHAREPIN_CON6);

        return 0;
    }

    return -1;
}

static int ak37d_pmx_set(int pin, int sel)
{
    struct gpio_sharepin *p_ak_sharepin;
    int sharepin_bits;
    int gpio_class;
    u32 regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    sharepin_bits = GET_SHAREPIN_WIDTH(p_ak_sharepin->gpio_config);
    gpio_class = GET_GPIO_CLASS(p_ak_sharepin->gpio_config);

    if (gpio_class == GPIO_CLASS_AO) {
        /* Always on gpio, do some special config here */
        u8 bit;
        if (pin == 16)
            bit = 0;
        else if (pin == 17)
            bit = 1;
        else if (pin == 21)
            bit = 2;
        else if (pin == 22)
            bit = 3;
        else if (pin == 104)
            bit = 4;
        else if (pin == 105)
            bit = 5;
        else if (pin == 106)
            bit = 6;
        else if (pin == 107)
            bit = 7;
        else if (pin == 108)
            bit = 8;
        else {
            pr_err("%s error pin:%d, sel:%d\n", __func__, pin, sel);
            return -1;
        }

        if (sel == 0) {
            regval = pmu_reg_read(0x4);
            regval |= 0x1;
            pmu_reg_write(0x4, regval);
            /* just config reg0_1 for function select */
            /* keep the default value of reg0_0 */

            regval = pmu_reg_read(0x0);
            regval &= ~(0x1<<bit);
            pmu_reg_write(0x0, regval);
        }else if (sel == 1) {
            regval = pmu_reg_read(0x4);
            regval |= 0x1;
            pmu_reg_write(0x4, regval);
            /* just config reg0_1 for function select */
            /* keep the default value of reg0_0 */

            regval = pmu_reg_read(0x0);
            regval |= (0x1<<bit);
            pmu_reg_write(0x0, regval);
        }
    } else {
        if (p_ak_sharepin->sharepin_reg != 0 &&
                sel <= (~(0xffffffff << sharepin_bits))) {
            regval = __raw_readl(p_ak_sharepin->sharepin_reg);
            regval &= ~((~(0xffffffff << sharepin_bits)) \
                        << p_ak_sharepin->reg_off);
            regval |= (sel << p_ak_sharepin->reg_off);
            __raw_writel(regval, p_ak_sharepin->sharepin_reg);
        }

        gpi_and_mipi_pins_set_share_ex(pin, sel);
        pin_pos_cfg(pin, sel);
    }

    return 0;
}

#if !defined(CONFIG_SPL_BUILD)

static int pin_set_open_drain(int pin, int enable)
{
    int bit = -1;
    u32 regval;

    if (pin == 31)
        bit = 30;
    else if (pin == 32)
        bit = 31;

    if (bit >= 0) {
        pr_debug("%s pin:%d, enable:%d\n", __func__, pin, enable);
        regval = __raw_readl(AK_PPU_PPD_EN_SEL);
        if (enable)
            regval |= (1<<bit);
        else
            regval &= ~(1<<bit);
        __raw_writel(regval, AK_PPU_PPD_EN_SEL);
    }

    return 0;
}
#endif

#if !defined(CONFIG_SPL_BUILD)

static int pin_set_pull_enable(int pin, int enable)
{
    struct gpio_sharepin *p_ak_sharepin;
    int gpio_class;
    int pupd_enable_value;
    u32 regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    gpio_class = GET_GPIO_CLASS(p_ak_sharepin->gpio_config);
    pupd_enable_value = GET_PUPD_ENABLE_BIT(p_ak_sharepin->gpio_config);

    if (gpio_class == GPIO_CLASS_AO) {
        /* Always on gpio, do some special config here */
        /* keep the default IE/PU config value of reg0_0 */
    } else {
        /* enable PU/PD function */
        if (p_ak_sharepin->pupd_en) {
            int set_value;
            if (enable) {
                set_value = pupd_enable_value;
            } else {
                set_value = !pupd_enable_value;
            }
            regval = __raw_readl(p_ak_sharepin->pupd_en);
            regval &= ~(1<<p_ak_sharepin->sel_bit);
            regval |= (set_value<<p_ak_sharepin->sel_bit);
            __raw_writel(regval, p_ak_sharepin->pupd_en);
        }

        return 0;
    }

    return 0;
}
#endif

#if !defined(CONFIG_SPL_BUILD)

static int pupd_sel_reg3_gpio100_103_cfg(int gpio, int pu_enable)
{
    int offset = 20;
    u32 regval;

    if ((gpio >= 100) && (gpio <= 103))
        offset += gpio - 100;
    else
        return -1;

    regval =__raw_readl(AK_PPU_PPD_EN_SEL);
    if (pu_enable)
        regval &= ~(1 << offset);
    else
        regval |= 1 << offset;
    __raw_writel(regval, AK_PPU_PPD_EN_SEL);

    return 0;
}
#endif

#if !defined(CONFIG_SPL_BUILD)

static int pin_set_pull_polarity(int pin, int pullup)
{
    struct gpio_sharepin *p_ak_sharepin;
    int gpio_class;
    u32 regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    gpio_class = GET_GPIO_CLASS(p_ak_sharepin->gpio_config);

    if (gpio_class == GPIO_CLASS_AO) {
        /* Always on gpio, do some special config here */
        /* keep the default IE/PU config value of reg0_0 */
    } else {
        if (p_ak_sharepin->pupd_sel) {
            if (!pullup) {
                regval = __raw_readl(p_ak_sharepin->pupd_sel);
                regval |= (1<<p_ak_sharepin->sel_bit);
                __raw_writel(regval, p_ak_sharepin->pupd_sel);
            } else {
                regval = __raw_readl(p_ak_sharepin->pupd_sel);
                regval &= ~(1<<p_ak_sharepin->sel_bit);
                __raw_writel(regval, p_ak_sharepin->pupd_sel);
            }
        }

        pupd_sel_reg3_gpio100_103_cfg(pin, pullup);
    }

    return 0;
}
#endif

#if !defined(CONFIG_SPL_BUILD)

static int drive_gpio76_92_cfg(int gpio, int drive)
{
    u32 regval;

    switch (gpio) {
        case 76:
            /*AK_GPIO_DRIVE_CON4 Note:b[17:18]*/
            regval =__raw_readl(AK_GPIO_DRIVE_CON4);
            regval &= ~(0x3<<17);
            regval |= ((drive >> 1) & 0x1) << 17;
            regval |= (drive & 0x1) << 18;
            __raw_writel(regval, AK_GPIO_DRIVE_CON4);

            break;

        case 92:
            /*AK_GPIO_DRIVE_CON5 Note:b[17]&[6]*/
            regval =__raw_readl(AK_GPIO_DRIVE_CON5);
            regval &= ~(0x1<<17);
            regval &= ~(0x1<<6);
            regval |= ((drive >> 1) & 0x1) << 17;
            regval |= (drive & 0x1) << 6;
            __raw_writel(regval, AK_GPIO_DRIVE_CON5);
            break;

        default:
            return -1;
            break;
    }

    return 0;
}
#endif

#if !defined(CONFIG_SPL_BUILD)

static int pin_set_drive(int pin, int strength)
{
    struct gpio_sharepin *p_ak_sharepin;
    int gpio_class;
    u32 regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    gpio_class = GET_GPIO_CLASS(p_ak_sharepin->gpio_config);

    strength = DRIVE_STRENGTH(strength);

    if (gpio_class == GPIO_CLASS_AO) {
        /* Always on gpio, do some special config here */
        /* keep the default IE/PU config value of reg0_0 */
    } else {
        if (p_ak_sharepin->drive_reg) {
            regval = __raw_readl(p_ak_sharepin->drive_reg);
            regval &= ~(0x3<<p_ak_sharepin->reg_off);
            regval |= ((strength&0x3)<<p_ak_sharepin->reg_off);
            __raw_writel(regval, p_ak_sharepin->drive_reg);
        }

        drive_gpio76_92_cfg(pin, strength);
    }

    return 0;
}
#endif

#if !defined(CONFIG_SPL_BUILD)

static int ie_reg3_gpio100_103_cfg(int gpio, int ie_enable)
{
    int offset = 15;
    u32 regval;

    if ((gpio >= 100) && (gpio <= 103))
        offset += gpio - 100;
    else
        return -1;

    regval =__raw_readl(AK_PPU_PPD_EN_SEL);
    if (ie_enable)
        regval |= 1 << offset;
    else
        regval &= ~(1 << offset);
    __raw_writel(regval, AK_PPU_PPD_EN_SEL);

    return 0;
}
#endif

#if !defined(CONFIG_SPL_BUILD)

static int pin_set_input_enable(int pin, int enable)
{
    struct gpio_sharepin *p_ak_sharepin;
    int ie = enable ? 1:0;
    int gpio_class;
    u32 regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    gpio_class = GET_GPIO_CLASS(p_ak_sharepin->gpio_config);

    if (gpio_class == GPIO_CLASS_AO) {
        /* Always on gpio, do some special config here */
        /* keep the default IE/PU config value of reg0_0 */
    } else {
        if (p_ak_sharepin->ie) {
            regval = __raw_readl(p_ak_sharepin->ie);
            regval &= ~(0x1<<p_ak_sharepin->sel_bit);
            regval |= (ie<<p_ak_sharepin->sel_bit);
            __raw_writel(regval, p_ak_sharepin->ie);
        }

        ie_reg3_gpio100_103_cfg(pin, ie);
    }

    return 0;
}
#endif

#if !defined(CONFIG_SPL_BUILD)

static int pin_set_slew_rate(int pin, int fast)
{
    struct gpio_sharepin *p_ak_sharepin;
    int gpio_class;
    u32 regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    gpio_class = GET_GPIO_CLASS(p_ak_sharepin->gpio_config);

    if (gpio_class == GPIO_CLASS_AO) {
        /* Always on gpio, do some special config here */
        /* keep the default IE/PU config value of reg0_0 */
    } else {
        if (p_ak_sharepin->slew_bit != -1) {
            regval = __raw_readl(AK_GPIO_SLEW_RATE);
            regval &= ~(0x1<<p_ak_sharepin->slew_bit);
            regval |= ((fast ? 1:0)<<p_ak_sharepin->slew_bit);
            __raw_writel(regval, AK_GPIO_SLEW_RATE);
        } else {
            //pr_err("%s can't set pin:%d slew\n", __func__, pin);
        }
    }

    return 0;
}
#endif

#if !defined(CONFIG_SPL_BUILD)

static int ak37d_pinconf_set(int pin, unsigned int configs)
{
    unsigned char pupd, drive, ie, slew;

/* anyka,function sharepin func index according to table 2-1 sharepin-list */
/* index: 0: PIN default func, 1: FUNCTION1,*/
/* 2: FUNCTION2, 3: FUNCTION3, 4: FUNCTION4 */
/* anyka,pull config value bit[31:24]--slew rate, bit[23:16]--ie,*/
/*                         bit[15:8]--drive, bit[7:0]--pupd config */
/* bit[31:24]--slew rate, 0: slow, 1: fast */ 
/* bit[23:16]--ie, input enable, 0: disable, 1: enable */
/* bit[15:8]--drive, drive strength, 4levels value: 0x0-0x3 */
/* bit[7:0]--bit[7]: open drain 0:disable, 1:enable (only for gpio31/32);
   pupd config, bit[6:4] 1:enable/ 0:disable,
   bit[3:0] 0:select PU /1:select PD */

    pupd    = (configs & 0xFF);
    drive   = ((configs>>8) & 0xFF);
    ie      = ((configs>>16) & 0xFF);
    slew    = ((configs>>24) & 0xFF);

    pin_set_open_drain(pin, (pupd >> 7) & 0x1);
    pin_set_pull_enable(pin, (pupd & 0x10) ? 1:0);
    pin_set_pull_polarity(pin, (pupd & 0x01) ? 0:1);
    pin_set_drive(pin, drive & 0x3);
    pin_set_input_enable(pin, ie & 0x1);
    pin_set_slew_rate(pin, slew & 0x1);

    return 0;
}
#endif

static int ak37d_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
    const fdt32_t *data_pins, *data_func;
    //const fdt32_t *data_pull;
    int count_pins, count_func, count_pull = 0;
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
    
#if !defined(CONFIG_SPL_BUILD)
    const fdt32_t *data_pull;
    data_pull = dev_read_prop(config, "anyka,pull", &count_pull);
    if (count_pull < 0) {
        pr_err("%s: bad pull array size %d\n", __func__, count_pull);
        return -EINVAL;
    }

    count_pull /= sizeof(fdt32_t);
#endif

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
        int pin, sel;

        pin = fdt32_to_cpu(data_pins[i]);
        sel = (count_func > 1) ?
            fdt32_to_cpu(data_func[i]) : fdt32_to_cpu(data_func[0]);
        //conf = (count_pull > 1) ?
        //    fdt32_to_cpu(data_pull[i]) : fdt32_to_cpu(data_pull[0]);
        ak37d_pmx_set(pin, sel);
#if !defined(CONFIG_SPL_BUILD)
        unsigned int conf;
        conf = (count_pull > 1) ?
            fdt32_to_cpu(data_pull[i]) : fdt32_to_cpu(data_pull[0]);
        ak37d_pinconf_set(pin, conf);
#endif
    }

    return 0;
}

const struct pinctrl_ops ak37d_pinctrl_ops  = {
    .set_state = ak37d_pinctrl_set_state,
};

static int ak37d_pinctrl_probe(struct udevice *dev)
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

static const struct udevice_id ak37d_pinctrl_match[] = {
    { .compatible = "anyka,ak37d-pinctrl"},
    {}
};

U_BOOT_DRIVER(ak37d_pinctrl) = {
    .name = "pinctrl_ak37d",
    .id = UCLASS_PINCTRL,
    .of_match = ak37d_pinctrl_match,
    .probe = ak37d_pinctrl_probe,
    .ops = &ak37d_pinctrl_ops,
};
