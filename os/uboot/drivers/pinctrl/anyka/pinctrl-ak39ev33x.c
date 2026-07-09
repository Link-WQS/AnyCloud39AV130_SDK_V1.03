// SPDX-License-Identifier: GPL-2.0+
/*
 * ak39ev33x pinctrl driver
 *
 * anyka
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <linux/io.h>
#include <linux/err.h>
#include <dt-bindings/pinctrl/ak_39ev33x_pinctrl.h>

#define AK_GPIO_CONF_INVALID    (-1)

#define AK_VA_SYSCTRL           ((void __iomem *)(0x08000000))
#define AK_MIPI0_BASE           ((void __iomem *)(0x20400000))
#define AK_MIPI1_BASE           ((void __iomem *)(0x20480000))
#define AK_VA_CAMERA            ((void __iomem *)(0x20000000))

#define AK_PULLUPDOWN_DISABLE   0
#define AK_PULLUPDOWN_ENABLE    1

#define ePIN_AS_GPIO_IN  0      // pin as gpio in
#define ePIN_AS_GPIO_OUT 1      // pin as gpio out

#define AK_GPIO_OUT_LOW         0
#define AK_GPIO_OUT_HIGH        1

#define AK_GPIO_DIR1            0x00
#define AK_GPIO_OUT1            0x0c
#define AK_GPIO_INPUT1          0x18
#define AK_GPIO_INT_MASK1       0x24
#define AK_GPIO_INT_MODE1       0x30
#define AK_GPIO_INTP1           0x3c
#define AK_GPIO_EDGE_STATUS1    0x48

#define AK_GPIO_DIR_BASE(pin)           (((pin)>>5)*4 + AK_GPIO_DIR1)
#define AK_GPIO_OUT_BASE(pin)           (((pin)>>5)*4 + AK_GPIO_OUT1)
#define AK_GPIO_IN_BASE(pin)            (((pin)>>5)*4 + AK_GPIO_INPUT1)
#define AK_GPIO_INTEN_BASE(pin)         (((pin)>>5)*4 + AK_GPIO_INT_MASK1)
#define AK_GPIO_INTM_BASE(pin)          (((pin)>>5)*4 + AK_GPIO_INT_MODE1)
#define AK_GPIO_INTPOL_BASE(pin)        (((pin)>>5)*4 + AK_GPIO_INTP1)
#define AK_GPIO_INTEDGE_BASE(pin)       (((pin)>>5)*4 + AK_GPIO_EDGE_STATUS1)

#define AK_GPIO_REG_SHIFT(pin)          ((pin) % 32)

/*Analog Control Register 3 (0x080000A4)*/
#define AK_ANALOG_CTRL_REG3             (AK_VA_SYSCTRL + 0x0A4)

#define AK_PERI_CHN_CLOCK_REG0          (AK_VA_SYSCTRL + 0x014)

#define AK_DUAL_SENSOR_SYNC_CFG_REG0    (AK_VA_CAMERA + 0x06c)

#define AK_SHAREPIN_CON0                (AK_VA_SYSCTRL + 0x074)
#define AK_SHAREPIN_CON1                (AK_VA_SYSCTRL + 0x078)
#define AK_SHAREPIN_CON2                (AK_VA_SYSCTRL + 0x07c)
#define AK_SHAREPIN_CON3                (AK_VA_SYSCTRL + 0x0dc)

#define AK_PULL_REG0                    (AK_VA_SYSCTRL + 0x080)
#define AK_PULL_REG1                    (AK_VA_SYSCTRL + 0x084)
#define AK_PULL_REG2                    (AK_VA_SYSCTRL + 0x088)
#define AK_PULL_REG3                    (AK_VA_SYSCTRL + 0x0e0)

/* pins are just named gpio0..gpio85 */
#define AK37D_GPIO_PIN(a) PINCTRL_PIN(a, "gpio" #a)

#define DVP_DATA_MAP_REG0               (AK_VA_CAMERA + 0x060)
#define DVP_DATA_MAP_REG1               (AK_VA_CAMERA + 0x064)

#define PIN_MAP_FLAG    (0x1)
#define SHIFT_FLAG      (24)
#define SHIFT_NEW       (12)
#define SHIFT_OLD       (0)
#define PACK_PIN_MAP(pin_new, pin_old) ((PIN_MAP_FLAG << SHIFT_FLAG) |\
        ((pin_new) << SHIFT_NEW) |\
        ((pin_old) << SHIFT_OLD))
#define UNPACK_FLAG(config)     ((config) >> SHIFT_FLAG)
#define UNPACK_PIN_MAP_NEW(config)      (((config) >> SHIFT_NEW) & \
                                        (~((~0UL) << (SHIFT_FLAG - SHIFT_NEW))))
#define UNPACK_PIN_MAP_OLD(config)      (((config) >> SHIFT_OLD) & \
                                        (~((~0UL) << (SHIFT_NEW - SHIFT_OLD))))

struct gpio_sharepin {
    int pin;

    /*first set mask*/
    void __iomem *reg0;
    unsigned int reg0_mask;
    void __iomem *reg1;
    unsigned int reg1_mask;
    void __iomem *reg2;
    unsigned int reg2_mask;

    /*extern function-0*/
    int func0;  /*if none extern-func then set -1*/
    void __iomem *func0_reg;
    unsigned int func0_mask;
    unsigned int func0_value;

    /*extern function-1*/
    int func1; /*if none extern-func then set -1*/
    void __iomem *func1_reg;
    unsigned int func1_mask;
    unsigned int func1_value;

    /*extern function-2*/
    int func2;  /*if none extern-func then set -1*/
    void __iomem *func2_reg;
    unsigned int func2_mask;
    unsigned int func2_value;

    void __iomem *pupd_en_reg;
    int sel_bit;
    int is_pullup;
};

//share pin func config in AK39XXEV33X
static struct gpio_sharepin ak_sharepin[] = {
#if defined(CONFIG_SPL_BUILD)
    /* gpio, UART0_RXD*/
    {1, AK_SHAREPIN_CON0, 1<<2, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 1, 1},
    /* gpio, UART0_TXD*/
    {2, AK_SHAREPIN_CON0, 1<<3, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 2, 1},
    {25, AK_SHAREPIN_CON3, 1<<0, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 0, 1},
    /* gpio, SPI0_SCLK*/
    {26, AK_SHAREPIN_CON3, 1<<1, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 1, 1},
    /* gpio, SD0_D[1], SPI0_WP*/
    {34, AK_SHAREPIN_CON3, 0x3<<10, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 7, 1},
    /* gpio, SD0_D[2], SPI0_DI*/
    {35, AK_SHAREPIN_CON3, 0x3<<12, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 8, 1},
    /* gpio, SD0_D[3], SPI0_DO*/
    /*SPI0 set extern reg*/
    {36, AK_SHAREPIN_CON3, 0x3<<12, NULL, 0, NULL, 0,
        2, AK_SHAREPIN_CON0, 1<<25, 1<<25, 3, AK_SHAREPIN_CON0,
        1<<25, 1<<25, -1, NULL, 0, 0,
        AK_PULL_REG3, 9, 1},
    /* gpio, SPI0_HOLD */
    {79, AK_SHAREPIN_CON3, 1<<9, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 20, 1},
#else
    /* gpio, JTAG_TRST, UART1_RXD, TWI2_SDA*/
    {0, AK_SHAREPIN_CON0, 0x3<<0, AK_SHAREPIN_CON1, 0x1UL<<31, NULL, 0,
        2, AK_SHAREPIN_CON0, 1<<29, 1<<29, 3, AK_SHAREPIN_CON1,
        1UL<<31, 1UL<<31, -1, NULL, 0, 0,
        AK_PULL_REG0, 0, 1},
    /* gpio, UART0_RXD*/
    {1, AK_SHAREPIN_CON0, 1<<2, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 1, 1},
    /* gpio, UART0_TXD*/
    {2, AK_SHAREPIN_CON0, 1<<3, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 2, 1},
    /* gpio, PWM0, CIS1_SCLK, CIS1_SCLK*/
    {3, AK_SHAREPIN_CON0, (1<<30) | (1<<4), NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 3, 0},
    /* gpio, PWM1, SD0_D[1]*/
    {4, AK_SHAREPIN_CON0, 1<<5, AK_SHAREPIN_CON1, 1<<28, NULL, 0,
        2, AK_SHAREPIN_CON1, 1<<28, 1<<28, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 4, 1},
    /* gpio, PWM2, UART1_TXD, JTAG_RTCK, TWI2_SCLK, AIN1*/
    {5, AK_SHAREPIN_CON0, 0x3<<6, AK_SHAREPIN_CON1, 1UL<<31,
        AK_ANALOG_CTRL_REG3, 1<<29,
        4, AK_SHAREPIN_CON1, 1UL<<31, 1UL<<31, 5, AK_ANALOG_CTRL_REG3,
        1<<29, 1<<29, -1, NULL, 0, 0,
        AK_PULL_REG0, 5, 1},

    /* CIS0_D[0], gpio,ISP_PWM0,UART1_RXD */
    /*注意：这里跟PG不一致，gpio和ISP_PWM0分开两个function了*/
    {6, AK_SHAREPIN_CON1, 0x3<<7, AK_DUAL_SENSOR_SYNC_CFG_REG0, 1<<2, NULL, 0,
        2, AK_SHAREPIN_CON1, 0x3<<7, 0x3<<7, 2, AK_DUAL_SENSOR_SYNC_CFG_REG0,
        1<<2, 1<<2, 3, AK_SHAREPIN_CON0, 1<<29, 0<<29,
        AK_PULL_REG1, 4, 0},
    /* CIS0_D[1], gpio, ISP_PWM1,UART1_TXD */
    /*注意：这里跟PG不一致，gpio和ISP_PWM1分开两个function了*/
    /*UART1 set extern reg*/
    {7, AK_SHAREPIN_CON1, 0x3<<9, AK_DUAL_SENSOR_SYNC_CFG_REG0, 1<<3, NULL, 0,
        2, AK_SHAREPIN_CON1, 0x3<<9, 0x3<<9, 2, AK_DUAL_SENSOR_SYNC_CFG_REG0,
        1<<3, 1<<3, 3, AK_SHAREPIN_CON0, 1<<29, 0<<29,
        AK_PULL_REG1, 5, 0},

    /* CIS0_D[2], gpio, UART1_CTS */
    {8, AK_SHAREPIN_CON1, 0x3<<11, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG1, 6, 0},
    /* CIS0_D[3], gpio, UART1_RTS */
    {9, AK_SHAREPIN_CON1, 0x3<<13, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG1, 7, 0},
    /* gpio, MII_MDC, I2S_MCLK, PWM2*/
    {10, AK_SHAREPIN_CON2, 0x3<<0, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 0, 0},
    /* gpio, MII_MDIO, I2S_LRCLK, SPI1_CS*/
    /*I2S两组pin没有交叉，在LRCLK配置extern reg*/
    /*SPI1两组pin没有交叉，在CLK配置extern reg*/
    /*I2S set extern reg*/
    {11, AK_SHAREPIN_CON2, 0x3<<2, NULL, 0, NULL, 0,
        2, AK_SHAREPIN_CON0, 1<<24, 1<<24, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 1, 1},
    /* gpio, MII_TXER*/
    {12, AK_SHAREPIN_CON2, 1<<4, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 2, 0},
    /* gpio, MII_TXEN, I2S_BCLK, SPI1_SCLK*/
    /*I2S两组pin没有交叉，在LRCLK配置extern reg*/
    /*SPI1两组pin没有交叉，在CLK配置extern reg*/
    /*SPI1 extern reg set*/
    {13, AK_SHAREPIN_CON2, 0x3<<5, NULL, 0, NULL, 0,
        3, AK_SHAREPIN_CON0, 1<<27, 1<<27, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 3, 0},
    /* gpio, MII_TXD[0], I2S_DI, SPI1_DI*/
    {14, AK_SHAREPIN_CON2, 0x3<<8, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 5, 0},
    /* gpio, MII_TXD[1], I2S_DO, SPI1_DO*/
    {15, AK_SHAREPIN_CON2, 0x3<<10, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 6, 0},
    /* gpio, MII_TXD[2]*/
    {16, AK_SHAREPIN_CON2, 1<<12, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 7, 0},
    /* gpio, MII_TXD[3]*/
    {17, AK_SHAREPIN_CON2, 1<<13, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 8, 0},
    /* gpio, MII_CRS*/
    {18, AK_SHAREPIN_CON2, 1<<14, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 9, 0},
    /* gpio, MII_RXD[0], SD1_D[3], SPI1_HOLD*/
    /*SD1两组pin的clk和cmd是一样的，只能通过D0判别了*/
    {19, AK_SHAREPIN_CON2, 0x3<<16, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 11, 0},
    /* gpio, MII_RXD[1], SD1_D[2], SPI1_WP*/
    /*SD1两组pin的clk和cmd是一样的，只能通过D0判别了*/
    {20, AK_SHAREPIN_CON2, 0x3<<18, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 12, 0},
    /* gpio, MII_RXD[2]*/
    {21, AK_SHAREPIN_CON2, 1<<20, NULL, 0, NULL, 0,
        -1, NULL, 0, 0,  -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 13, 0},
    /* gpio, MII_RXD[3]*/
    {22, AK_SHAREPIN_CON2, 1<<21, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 14, 0},
    /* gpio, MII_RXER, SD1_D[1], PWM3*/
    /*SD1两组pin的clk和cmd是一样的，只能通过D0判别了*/
    {23, AK_SHAREPIN_CON2, 0x3<<22, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 15, 0},
    /* gpio, MII_RXDV, SD1_D[0], PWM4*/
    /*SD1两组pin的clk和cmd是一样的，只能通过D0判别了*/
    /*SD1 extern reg*/
    {24, AK_SHAREPIN_CON2, 0x3<<24, NULL, 0, NULL, 0,
        2, AK_SHAREPIN_CON0, 1<<28, 1<<28, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 16, 0},
    /* gpio, SPI0_CS*/
    /*SPI0只有cs和sclk固定为gpio[26:25]，其他分为两组:
     *  (1)[40:37]
     *  (2)[79],[36:34]*/
    {25, AK_SHAREPIN_CON3, 1<<0, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 0, 1},
    /* gpio, SPI0_SCLK*/
    {26, AK_SHAREPIN_CON3, 1<<1, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 1, 1},
    /* gpio, TWI0_SCL*/
    {27, AK_SHAREPIN_CON0, 1<<8, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 6, 1},
    /* gpio, TWI0_SDA*/
    {28, AK_SHAREPIN_CON0, 1<<9, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 7, 1},
    /* gpio, IRDA_RX, SPI1_SCLK*/
    /*SPI1 extern reg set*/
    {29, AK_SHAREPIN_CON3, 0x3<<2, NULL, 0, NULL, 0,
        2, AK_SHAREPIN_CON0, 1<<27, 0<<27, 3, AK_SHAREPIN_CON0,
        1<<27, 0<<27, -1, NULL, 0, 0,
        AK_PULL_REG3, 2, 1},
    /* gpio, PWM4, SPI1_CS*/
    {30, AK_SHAREPIN_CON3, 0x3<<4, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 3, 1},
    /* gpio, SD0_CMD*/
    {31, AK_SHAREPIN_CON3, 1<<6, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 4, 1},
    /* gpio, SD0_CLK*/
    {32, AK_SHAREPIN_CON3, 1<<7, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 5, 1},
    /* gpio, SD0_D[0]*/
    {33, AK_SHAREPIN_CON3, 1<<8, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 6, 1},
    /* gpio, SD0_D[1], SPI0_WP*/
    {34, AK_SHAREPIN_CON3, 0x3<<10, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 7, 1},
    /* gpio, SD0_D[2], SPI0_DI*/
    {35, AK_SHAREPIN_CON3, 0x3<<12, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 8, 1},
    /* gpio, SD0_D[3], SPI0_DO*/
    /*SPI0 set extern reg*/
    {36, AK_SHAREPIN_CON3, 0x3<<12, NULL, 0, NULL, 0,
        2, AK_SHAREPIN_CON0, 1<<25, 1<<25, 3, AK_SHAREPIN_CON0,
        1<<25, 1<<25, -1, NULL, 0, 0,
        AK_PULL_REG3, 9, 1},
    /* gpio, SD0_D[4], SPI0_DI*/
    {37, AK_SHAREPIN_CON3, 0x3<<14, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 10, 1},
    /* gpio, SD0_D[5], SPI0_DO*/
    /*SPI0 set extern reg*/
    {38, AK_SHAREPIN_CON3, 0x3<<14, NULL, 0, NULL, 0,
        2, AK_SHAREPIN_CON0, 1<<25, 0<<25, 3, AK_SHAREPIN_CON0,
        1<<25, 0<<25, -1, NULL, 0, 0,
        AK_PULL_REG3, 11, 1},
    /* gpio, SD0_D[6], SPI0_WP*/
    {39, AK_SHAREPIN_CON3, 0x3<<16, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 12, 1},
    /* gpio, SD0_D[7], SPI0_HOLD*/
    {40, AK_SHAREPIN_CON3, 0x3<<18, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 13, 1},
    /* gpio, SD1_CMD*/
    {41, AK_SHAREPIN_CON3, 1<<20, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 14, 1},
    /* gpio, SD1_CLK*/
    {42, AK_SHAREPIN_CON3, 1<<21, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 15, 1},
    /* gpio, SD1_D[0], SPI1_HOLD*/
    /*SD1两组的clk一样所以在D0配置extern reg*/
    /*SD1 extern reg*/
    {43, AK_SHAREPIN_CON3, 0x3<<22, NULL, 0, NULL, 0,
        1, AK_SHAREPIN_CON0, 1<<28, 0<<28, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 16, 1},
    /* gpio, SD1_D[1], SPI1_WP*/
    {44, AK_SHAREPIN_CON3, 0x3<<24, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 17, 1},
    /* gpio, SD1_D[2], SPI1_DO*/
    {45, AK_SHAREPIN_CON3, 0x3<<26, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 18, 1},
    /* gpio, SD1_D[3], SPI1_DI*/
    {46, AK_SHAREPIN_CON3, 0x3<<26, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 19, 1},
    /* gpio, OPCLK, PWM4*/
    {47, AK_SHAREPIN_CON0, 0x3<<10, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 8, 0},
    /* gpio, PWM0, CIS1_PCLK*/
    {48, AK_SHAREPIN_CON0, 1<<12, AK_PERI_CHN_CLOCK_REG0, 1<<29, NULL, 0,
        2, AK_PERI_CHN_CLOCK_REG0, 1<<29, 1<<29, 3, AK_PERI_CHN_CLOCK_REG0,
        1<<29, 1<<29, -1, NULL, 0, 0,
        AK_PULL_REG0, 9, 0},
    {49, NULL, 0, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 27, 0},
    /* gpio, PWM1, CIS1_HSYNC*/
    {50, AK_SHAREPIN_CON0, 1<<13, AK_PERI_CHN_CLOCK_REG0, 1<<29, NULL, 0,
        2, AK_PERI_CHN_CLOCK_REG0, 1<<29, 1<<29, 3, AK_PERI_CHN_CLOCK_REG0,
        1<<29, 1<<29, -1, NULL, 0, 0,
        AK_PULL_REG0, 10, 1},
    /* gpio, PWM2, CIS1_VSYNC*/
    {51, AK_SHAREPIN_CON0, 1<<14, AK_PERI_CHN_CLOCK_REG0, 1<<29, NULL, 0,
        2, AK_PERI_CHN_CLOCK_REG0, 1<<29, 1<<29, 3, AK_PERI_CHN_CLOCK_REG0,
        1<<29, 1<<29, -1, NULL, 0, 0,
        AK_PULL_REG0, 11, 1},
    /* gpio, I2S_DO, CIS1_D[0], TWI1_SCL*/
    {52, AK_SHAREPIN_CON0, (1<<26) | (1<<15), NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 12, 1},
    /* gpio, I2S_MCLK, CIS1_D[1], TWI1_SDA*/
    {53, AK_SHAREPIN_CON0, (1<<26) | (1<<16), NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 13, 1},
    /* gpio, I2S_BCLK, CIS1_D[2] */
    {54, AK_SHAREPIN_CON0, (1UL<<31) | (1<<17), NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 14, 0},
    /* gpio, I2S_LRCLK, CIS1_D[3] */
    /*I2S set extern reg*/
    {55, AK_SHAREPIN_CON0, (1UL<<31) | (1<<18), NULL, 0, NULL, 0,
        1, AK_SHAREPIN_CON0, 1<<24, 0<<24, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 15, 0},
    /* gpio, I2S_DIN, CIS1_D[4] */
    {56, AK_SHAREPIN_CON0, 1<<19, AK_PERI_CHN_CLOCK_REG0, 1<<29, NULL, 0,
        2, AK_PERI_CHN_CLOCK_REG0, 1<<29, 1<<29, 3, AK_PERI_CHN_CLOCK_REG0,
        1<<29, 1<<29, -1, NULL, 0, 0,
        AK_PULL_REG0, 16, 1},
    /* gpio, PWM3, CIS1_D[5] */
    {57, AK_SHAREPIN_CON0, 1<<20, AK_PERI_CHN_CLOCK_REG0, 1<<29, NULL, 0,
        2, AK_PERI_CHN_CLOCK_REG0, 1<<29, 1<<29, 3, AK_PERI_CHN_CLOCK_REG0,
        1<<29, 1<<29, -1, NULL, 0, 0,
        AK_PULL_REG0, 17, 1},
    /* gpio, PWM4, CIS1_D[6] */
    {58, AK_SHAREPIN_CON0, 1<<21, AK_PERI_CHN_CLOCK_REG0, 1<<29, NULL, 0,
        2, AK_PERI_CHN_CLOCK_REG0, 1<<29, 1<<29, 3, AK_PERI_CHN_CLOCK_REG0,
        1<<29, 1<<29, -1, NULL, 0, 0,
        AK_PULL_REG0, 18, 1},
    /* gpio, CIS1_D[7] */
    {59, AK_PERI_CHN_CLOCK_REG0, 1<<29, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 19, 0},
    /* gpio, CIS1_D[8] */
    {60, AK_PERI_CHN_CLOCK_REG0, 1<<29, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 20, 0},
    /* gpio, CIS1_D[9] */
    {61, AK_PERI_CHN_CLOCK_REG0, 1<<29, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 21, 0},
    /* gpio, CIS1_D[10] */
    {62, AK_PERI_CHN_CLOCK_REG0, 1<<29, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 22, 1},
    /* gpio, CIS1_D[11] */
    {63, AK_PERI_CHN_CLOCK_REG0, 1<<29, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 23, 1},
    /* CIS0_SCLK, gpio, JTAG_TDO */
    {64, AK_SHAREPIN_CON1, 0x3<<0, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG1, 0, 0},
    /* CIS0_PCLK, gpio */
    {65, AK_SHAREPIN_CON1, 1<<2, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG1, 1, 0},
    /* CIS0_HSYNC, gpio, PWM3 */
    {66, AK_SHAREPIN_CON1, 0x3<<3, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG1, 2, 0},
    /* CIS0_VSYNC, gpio, PWM4 */
    {67, AK_SHAREPIN_CON1, 0x3<<5, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG1, 3, 0},
    /* CIS0_D[4], gpio, PWM0 */
    {68, AK_SHAREPIN_CON1, 0x3<<15, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG1, 8, 0},
    /* CIS0_D[5], gpio, PWM1 */
    {69, AK_SHAREPIN_CON1, 0x3<<17, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG1, 9, 0},
    /* CIS0_D[9], gpi, JTAG_TCK, reserved, MIPI_CLKP */
    {70, AK_SHAREPIN_CON1, 0x3<<19, AK_PERI_CHN_CLOCK_REG0, 1<<26, NULL, 0,
        3, AK_PERI_CHN_CLOCK_REG0, 1<<26, 1<<26, -1, NULL, 0, 0, -1, NULL, 0, 0,
        /*MIPI pins pull set by mipi registers*/NULL, 0, 0},
    /* CIS0_D[8], gpi, JTAG_TMS, reserved, MIPI_CLKN */
    {71, AK_SHAREPIN_CON1, 0x3<<21, AK_PERI_CHN_CLOCK_REG0, 1<<26, NULL, 0,
        3, AK_PERI_CHN_CLOCK_REG0, 1<<26, 1<<26, -1, NULL, 0, 0, -1, NULL, 0, 0,
        /*MIPI pins pull set by mipi registers*/NULL, 0, 0},
    /* CIS0_D[10], gpi, JTAG_TDI, reserved, MIPI_DP0 */
    {72, AK_SHAREPIN_CON1, 0x3<<23, AK_PERI_CHN_CLOCK_REG0, 1<<26, NULL, 0,
        3, AK_PERI_CHN_CLOCK_REG0, 1<<26, 1<<26, -1, NULL, 0, 0, -1, NULL, 0, 0,
        /*MIPI pins pull set by mipi registers*/NULL, 0, 0},
    /* CIS0_D[11], gpi, MIPI_DN0 */
    {73, AK_SHAREPIN_CON1, 1<<25, AK_PERI_CHN_CLOCK_REG0, 1<<26, NULL, 0,
        2, AK_PERI_CHN_CLOCK_REG0, 1<<26, 1<<26, -1, NULL, 0, 0, -1, NULL, 0, 0,
        /*MIPI pins pull set by mipi registers*/NULL, 0, 0},
    /* CIS0_D[7], gpi, MIPI_DP1 */
    {74, AK_SHAREPIN_CON1, 1<<26, AK_PERI_CHN_CLOCK_REG0, 1<<26, NULL, 0,
        2, AK_PERI_CHN_CLOCK_REG0, 1<<26, 1<<26, -1, NULL, 0, 0, -1, NULL, 0, 0,
        /*MIPI pins pull set by mipi registers*/NULL, 0, 0},
    /* CIS0_D[6], gpi, MIPI_DN1 */
    {75, AK_SHAREPIN_CON1, 1<<27, AK_PERI_CHN_CLOCK_REG0, 1<<26, NULL, 0,
        2, AK_PERI_CHN_CLOCK_REG0, 1<<26, 1<<26, -1, NULL, 0, 0, -1, NULL, 0, 0,
        /*MIPI pins pull set by mipi registers*/NULL, 0, 0},
    /* gpio, MII_TXCLK */
    {76, AK_SHAREPIN_CON2, 1<<7, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 4, 0},
    /* gpio, MII_RXCLK */
    {77, AK_SHAREPIN_CON2, 1<<15, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 10, 0},
    /* gpio, MII_COL */
    {78, AK_SHAREPIN_CON2, 1<<26, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG2, 17, 0},
    /* gpio, SPI0_HOLD */
    {79, AK_SHAREPIN_CON3, 1<<9, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG3, 20, 1},
    /* gpio, PWM0, SD0_D[2],TWI1_SCL */
    {80, AK_SHAREPIN_CON0, 1<<22, AK_SHAREPIN_CON1, 0x3<<29, NULL, 0,
        2, AK_SHAREPIN_CON1, 1<<29, 1<<29, 3, AK_SHAREPIN_CON1,
        1<<30, 1<<30, -1, NULL, 0, 0,
        AK_PULL_REG0, 24, 1},
    /* gpio, PWM2, SD0_D[3],TWI1_SDA */
    {81, AK_SHAREPIN_CON0, 1<<23, AK_SHAREPIN_CON1, 0x3<<29, NULL, 0,
        2, AK_SHAREPIN_CON1, 1<<29, 1<<29, 3, AK_SHAREPIN_CON1,
        1<<30, 1<<30, -1, NULL, 0, 0,
        AK_PULL_REG0, 25, 1},
    /* gpio*/
    {82, NULL, 0, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 26, 1},
    /* MICP, gpi */
    {83, AK_ANALOG_CTRL_REG3, 0x3<<25, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        NULL, 0, 0},
    /* LINEIN, gpi */
    {84, AK_ANALOG_CTRL_REG3, 0x3<<27, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        NULL, 0, 0},
    /* gpio*/
    {85, NULL, 0, NULL, 0, NULL, 0,
        -1, NULL, 0, 0, -1, NULL, 0, 0, -1, NULL, 0, 0,
        AK_PULL_REG0, 28, 1},
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

static int set_reg0_sel(int pin, int sel)
{
    struct gpio_sharepin *p_ak_sharepin;
    int reg0_mask_bits = 0;
    int offset = 0;
    unsigned int sel_tmp = sel;
    unsigned int mask;
    unsigned int regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    mask = p_ak_sharepin->reg0_mask;

    if (!p_ak_sharepin->reg0)
        return 0;

    regval = __raw_readl(p_ak_sharepin->reg0);
    while (mask) {
        if (mask & 0x1) {
            regval |= (sel_tmp & 0x1) << offset;
            sel_tmp >>= 1;
            reg0_mask_bits++;
        }
        mask >>= 1;
        offset++;
    }

    if (sel < (1<<reg0_mask_bits)) {
        __raw_writel(regval, p_ak_sharepin->reg0);
        return 0;
    }

    return -1;
}

static int ak39ev33x_pmx_set(int pin, int sel)
{
    struct gpio_sharepin *p_ak_sharepin;
    unsigned int regval;

    p_ak_sharepin = find_sharepin_struct(pin);
    if (!p_ak_sharepin) {
        pr_err("%s cannot find pin:%d\n", __func__, pin);
        return -EINVAL;
    }

    if ((pin > XGPIO_085) || (pin < XGPIO_000)) {
        pr_err("%s pin_%d not support\n", __func__, pin);
        return -EINVAL;
    }

    if (p_ak_sharepin->reg0) {
        regval = __raw_readl(p_ak_sharepin->reg0);
        regval &= ~p_ak_sharepin->reg0_mask;
        __raw_writel(regval, p_ak_sharepin->reg0);
    }
    if (p_ak_sharepin->reg1) {
        regval = __raw_readl(p_ak_sharepin->reg1);
        regval &= ~p_ak_sharepin->reg1_mask;
        __raw_writel(regval, p_ak_sharepin->reg1);
    }
    if (p_ak_sharepin->reg2) {
        regval = __raw_readl(p_ak_sharepin->reg2);
        regval &= ~p_ak_sharepin->reg2_mask;
        __raw_writel(regval, p_ak_sharepin->reg2);
    }

    set_reg0_sel(pin, sel);

    if ((p_ak_sharepin->func0 == sel) &&
            p_ak_sharepin->func0_reg) {
        regval = __raw_readl(p_ak_sharepin->func0_reg);
        regval &= ~p_ak_sharepin->func0_mask;
        regval |= p_ak_sharepin->func0_value;
        __raw_writel(regval, p_ak_sharepin->func0_reg);
    }
    if ((p_ak_sharepin->func1 == sel) &&
            p_ak_sharepin->func1_reg) {
        regval = __raw_readl(p_ak_sharepin->func1_reg);
        regval &= ~p_ak_sharepin->func1_mask;
        regval |= p_ak_sharepin->func1_value;
        __raw_writel(regval, p_ak_sharepin->func1_reg);
    }
    if ((p_ak_sharepin->func2 == sel) &&
            p_ak_sharepin->func2_reg) {
        regval = __raw_readl(p_ak_sharepin->func2_reg);
        regval &= ~p_ak_sharepin->func2_mask;
        regval |= p_ak_sharepin->func2_value;
        __raw_writel(regval, p_ak_sharepin->func2_reg);
    }

    /*
    * mipi pins: Ignore mipi in uboot.
    */
    //mipi_pins_set_share_ex(pc, pin, sel);

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

    /* enable PU/PD function */
    if (p_ak_sharepin->pupd_en_reg) {
        regval = __raw_readl(p_ak_sharepin->pupd_en_reg);
        regval &= ~(1<<p_ak_sharepin->sel_bit);
        if (!enable)/*disable pull*/
            regval |= (1<<p_ak_sharepin->sel_bit);
        __raw_writel(regval, p_ak_sharepin->pupd_en_reg);
    }

    return 0;
}

static int mipi_pins_set_pull_polarity(int pin, int enable, int pullup)
{
    void __iomem *mipi_base;

    if (pin >= XGPIO_070 && pin <= XGPIO_075) {
        mipi_base = AK_MIPI0_BASE;

        if (enable) {
            if (pullup) {
                __raw_writeb(0x80, mipi_base + 0x0d);//enable pullup
                __raw_writeb(0x02, mipi_base + 0xb8);//disable pulldown
            } else {
                __raw_writeb(0xb0, mipi_base + 0x0d);//disable pullup
                __raw_writeb(0x04, mipi_base + 0xb8);//enable pulldown
            }
        } else {
            __raw_writeb(0xb0, mipi_base + 0x0d);//disable pullup
            __raw_writeb(0x02, mipi_base + 0xb8);//disable pulldown
        }
    }

    return 0;
}

static int pin_set_pull_polarity(int pin, int enable, int pullup)
{
    /*H3B只有mipi pins有上下拉可选，其他pins只有其中一种*/

    mipi_pins_set_pull_polarity(pin, enable, pullup);

    return 0;
}

static int ak39ev33x_pinconf_set(int pin, unsigned int configs)
{
    unsigned char pupd;

    /*
    * configs
    * bit[0] pull up or pull down selection
    * bit[4] pull up or pull down enable
    */
    pupd = (configs & 0xFF);

    pin_set_pull_enable(pin, (pupd & 0x10) ? 1:0);
    pin_set_pull_polarity(pin, (pupd & 0x10) ? 1:0, (pupd & 0x01) ? 0:1);

    return 0;
}

static int ak39ev33x_pinctrl_set_state(struct udevice *dev,
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
        pr_debug("%s: FUNC_%d CONF_0x%08x\n", config->name, sel, conf);
        ak39ev33x_pmx_set(pin, sel);
        ak39ev33x_pinconf_set(pin, conf);
    }

    return 0;
}

const struct pinctrl_ops ak39ev33x_pinctrl_ops  = {
    .set_state = ak39ev33x_pinctrl_set_state,
};

static int ak39ev33x_pinctrl_probe(struct udevice *dev)
{
    struct clk clk;
    int ret = 0;

    pr_debug("%s\n", __func__);

    ret = clk_get_by_index(dev, 0, &clk);
    if (ret)
        return ret;
    ret = clk_enable(&clk);
    if (ret)
        return ret;

    return 0;
}

static const struct udevice_id ak39ev33x_pinctrl_match[] = {
    { .compatible = "anyka,ak39ev330-pinctrl"},
    {}
};

U_BOOT_DRIVER(ak39ev33x_pinctrl) = {
    .name = "pinctrl_ak39ev330",
    .id = UCLASS_PINCTRL,
    .of_match = ak39ev33x_pinctrl_match,
    .probe = ak39ev33x_pinctrl_probe,
    .ops = &ak39ev33x_pinctrl_ops,
};
