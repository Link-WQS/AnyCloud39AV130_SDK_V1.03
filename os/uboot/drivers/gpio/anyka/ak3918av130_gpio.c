// SPDX-License-Identifier: GPL-2.0+
/*
 * ak3918av100_gpio.c
 *
 * anyka
 *
 */

#include <config.h>
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include <asm/gpio.h>

#define AK3918AV130_NUM_GPIOS         105
#define AK3918AV130_NUM_BANKS         1

#define AK_GPIO_OUT_LOW         0
#define AK_GPIO_OUT_HIGH        1

#define AK_GPIO_VA_SYSCTRL          ((void __iomem *)(0x20170000))

#define AK_GPIO_DIR1            0x00
#define AK_GPIO_OUT1            0x14
#define AK_GPIO_INPUT1          0x28
#define AK_GPIO_INT_MASK1       0x3c
#define AK_GPIO_INT_MODE1       0x50
#define AK_GPIO_INTP1           0x64
#define AK_GPIO_EDGE_STATUS1    0x78
#define AK_GPIO_D_EDGE_STATUS1  0x8c

#define AK_GPIO_DIR_BASE(pin)           (AK_GPIO_VA_SYSCTRL + \
                                        (((pin)>>5)*4 + AK_GPIO_DIR1))
#define AK_GPIO_OUT_BASE(pin)           (AK_GPIO_VA_SYSCTRL + \
                                        (((pin)>>5)*4 + AK_GPIO_OUT1))
#define AK_GPIO_IN_BASE(pin)            (AK_GPIO_VA_SYSCTRL + \
                                        (((pin)>>5)*4 + AK_GPIO_INPUT1))
#define AK_GPIO_INTEN_BASE(pin)         (AK_GPIO_VA_SYSCTRL + \
                                        (((pin)>>5)*4 + AK_GPIO_INT_MASK1))
#define AK_GPIO_INTM_BASE(pin)          (AK_GPIO_VA_SYSCTRL + \
                                        (((pin)>>5)*4 + AK_GPIO_INT_MODE1))
#define AK_GPIO_INTPOL_BASE(pin)        (AK_GPIO_VA_SYSCTRL + \
                                        (((pin)>>5)*4 + AK_GPIO_INTP1))
#define AK_GPIO_INTEDGE_BASE(pin)       (AK_GPIO_VA_SYSCTRL + \
                                        (((pin)>>5)*4 + AK_GPIO_EDGE_STATUS1))

#define AK_GPIO_REG_SHIFT(pin)          ((pin) % 32)

/**
*
*@brief: set GPIO pin 'gpio' as an input
*@param[in] 
*@param[in] 
*@return: int
*
**/
static int ak3918av130_gpio_direction_input(struct udevice *dev,
                        unsigned offset)
{
    void __iomem *reg_dir = AK_GPIO_DIR_BASE(offset);
    unsigned int bit = AK_GPIO_REG_SHIFT(offset);
    unsigned int regval;

    /*set input*/
    regval = __raw_readl(reg_dir);
    regval &= ~(1 << bit);
    __raw_writel(regval, reg_dir);

    return 0;
}

/* set GPIO pin 'gpio' as an output, with polarity 'value' */
/**
*
*@brief: set GPIO pin 'gpio' as an input
*@param[in] 
*@param[in] 
*@return: int
*
**/
static int ak3918av130_gpio_direction_output(struct udevice *dev,
                       unsigned offset,
                       int value)
{
    void __iomem *reg_dir = AK_GPIO_DIR_BASE(offset);
    void __iomem *reg_out = AK_GPIO_OUT_BASE(offset);
    unsigned int bit = AK_GPIO_REG_SHIFT(offset);
    unsigned regval;

    /*set ouput*/
    regval = __raw_readl(reg_dir);
    regval |= (1 << bit);
    __raw_writel(regval, reg_dir);

    /*set value*/
    if (AK_GPIO_OUT_LOW == value) {
        regval = __raw_readl(reg_out);
        regval &= ~(1 << bit);
        __raw_writel(regval, reg_out);
    } else if (AK_GPIO_OUT_HIGH == value) {
        regval = __raw_readl(reg_out);
        regval |= (1 << bit);
        __raw_writel(regval, reg_out);
    }

    return 0;
}

/**
*
*@brief: read GPIO IN value of pin 'gpio'
*@param[in] 
*@param[in] 
*@return: int
*
**/
static int ak3918av130_gpio_get_value(struct udevice *dev, unsigned offset)
{
    void __iomem *reg_direction;
    void __iomem *reg;
    unsigned int direction;
    unsigned int bit;

    reg_direction = AK_GPIO_DIR_BASE(offset);
    bit = AK_GPIO_REG_SHIFT(offset);

    direction = ((__raw_readl(reg_direction) & (1 << bit)) ? 0:1);
    if (direction) {
        reg = AK_GPIO_IN_BASE(offset);
    } else {
        reg = AK_GPIO_OUT_BASE(offset);
    }

    pr_debug("GPIO_%d(%s): VAL(%d) 0x%p:0x%x\n",
            offset, direction ? "IN" : "OUT",
            ((__raw_readl(reg) & (1 << bit)) ? 1:0), reg, __raw_readl(reg));

    return (__raw_readl(reg) & (1 << bit)) ? 1:0;
}

/**
*
*@brief: write GPIO OUT value to pin 'gpio'
*@param[in] 
*@param[in] 
*@return: int
*
**/
static int ak3918av130_gpio_set_value(struct udevice *dev, unsigned offset,
                   int value)
{
    void __iomem *reg_out = AK_GPIO_OUT_BASE(offset);
    unsigned int bit = AK_GPIO_REG_SHIFT(offset);
    unsigned regval;

    /*set value*/
    if (AK_GPIO_OUT_LOW == value) {
        regval = __raw_readl(reg_out);
        regval &= ~(1 << bit);
        __raw_writel(regval, reg_out);
    } else if (AK_GPIO_OUT_HIGH == value) {
        regval = __raw_readl(reg_out);
        regval |= (1 << bit);
        __raw_writel(regval, reg_out);
    }

    return 0;
}

static const struct dm_gpio_ops gpio_ak3918av130_ops = {
    .direction_input    = ak3918av130_gpio_direction_input,
    .direction_output   = ak3918av130_gpio_direction_output,
    .get_value      = ak3918av130_gpio_get_value,
    .set_value      = ak3918av130_gpio_set_value,
};

static int ak3918av130_gpio_probe(struct udevice *dev)
{
    struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

    pr_debug("%s\n", __func__);

    uc_priv->bank_name = "gpio0";
    uc_priv->gpio_count = AK3918AV130_NUM_GPIOS;

    return 0;
}

static const struct udevice_id ak3918av130_gpio_ids[] = {
    { .compatible = "anyka,ak3918av130-gpio" },
    { }
};

U_BOOT_DRIVER(gpio_ak3918av130) = {
    .name   = "gpio_ak3918av130",
    .id = UCLASS_GPIO,
    .of_match = ak3918av130_gpio_ids,
    .ops    = &gpio_ak3918av130_ops,
    .probe  = ak3918av130_gpio_probe,
};
