// SPDX-License-Identifier: GPL-2.0+
/*
 * ak37d_gpio.c
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

#define AK37D_NUM_GPIOS         123
#define AK37D_NUM_BANKS         1

#define AK_GPIO_OUT_LOW         0
#define AK_GPIO_OUT_HIGH        1

#define AK_GPIO_VA_SYSCTRL          ((void __iomem *)(0x20170000))

#define AK_GPIO_DIR1            0x00
#define AK_GPIO_OUT1            0x14
#define AK_GPIO_INPUT1          0x28
#define AK_GPIO_INPUT3          0x34
#define AK_GPIO_INT_MASK1       0x3c
//#define AK_GPIO_INT_MASK3       0x48
#define AK_GPIO_INT_MODE1       0x50
//#define AK_GPIO_INT_MODE3       0x5C
#define AK_GPIO_INTP1           0x64
//#define AK_GPIO_INTP3           0x70
#define AK_GPIO_EDGE_STATUS1    0x78
//#define AK_GPIO_EDGE_STATUS3    0x84

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

#define AK_GPI_IN_BASE(pin)             (AK_GPIO_VA_SYSCTRL + AK_GPIO_INPUT3)

#define AK_GPIO_REG_SHIFT(pin)          ((pin) % 32)
#define AK_GPI_REG_SHIFT(pin)           ((pin) - 109 + 13)

static inline int pin_gpi_groups(int pin)
{
    /*gpi0~13 map to gpio109~122*/
    return ((pin >= 109) && (pin <= 122)) ? 1 : 0;
}

/**
*
*@brief: set GPIO pin 'gpio' as an input
*@param[in] 
*@param[in] 
*@return: int
*
**/
static int ak37d_gpio_direction_input(struct udevice *dev, unsigned offset)
{
    void __iomem *reg_dir = AK_GPIO_DIR_BASE(offset);
    unsigned int bit = AK_GPIO_REG_SHIFT(offset);
    unsigned int regval;

    /*set input*/
    if (!pin_gpi_groups(offset)) {
        regval = __raw_readl(reg_dir);
        regval &= ~(1 << bit);
        __raw_writel(regval, reg_dir);
    }

    return 0;
}

/**
*
*@brief: set GPIO pin 'gpio' as an output, with polarity 'value'
*@param[in] 
*@param[in] 
*@return: int
*
**/
static int ak37d_gpio_direction_output(struct udevice *dev, unsigned offset,
                       int value)
{
    void __iomem *reg_dir = AK_GPIO_DIR_BASE(offset);
    void __iomem *reg_out = AK_GPIO_OUT_BASE(offset);
    unsigned int bit = AK_GPIO_REG_SHIFT(offset);
    unsigned regval;

    if (pin_gpi_groups(offset))
        return -EPERM;

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
static int ak37d_gpio_get_value(struct udevice *dev, unsigned offset)
{
    void __iomem *reg_direction;
    void __iomem *reg;
    unsigned int direction = 0;
    unsigned int bit;

    if (pin_gpi_groups(offset)) {
        reg = AK_GPI_IN_BASE(offset);
        bit = AK_GPI_REG_SHIFT(offset);
    } else {
        reg_direction = AK_GPIO_DIR_BASE(offset);
        bit = AK_GPIO_REG_SHIFT(offset);

        direction = ((__raw_readl(reg_direction) & (1 << bit)) ? 0:1);
        if (direction) {
            reg = AK_GPIO_IN_BASE(offset);
        } else {
            reg = AK_GPIO_OUT_BASE(offset);
        }
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
static int ak37d_gpio_set_value(struct udevice *dev, unsigned offset,
                   int value)
{
    void __iomem *reg_out = AK_GPIO_OUT_BASE(offset);
    unsigned int bit = AK_GPIO_REG_SHIFT(offset);
    unsigned regval;

    if (pin_gpi_groups(offset))
        return -EPERM;

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

static const struct dm_gpio_ops gpio_ak37d_ops = {
    .direction_input    = ak37d_gpio_direction_input,
    .direction_output   = ak37d_gpio_direction_output,
    .get_value      = ak37d_gpio_get_value,
    .set_value      = ak37d_gpio_set_value,
};

static int ak37d_gpio_probe(struct udevice *dev)
{
    struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

    pr_debug("%s\n", __func__);

    uc_priv->bank_name = "gpio0";
    uc_priv->gpio_count = AK37D_NUM_GPIOS;

    return 0;
}

static const struct udevice_id ak37d_gpio_ids[] = {
    { .compatible = "anyka,ak37d-gpio" },
    { }
};

U_BOOT_DRIVER(gpio_ak37d) = {
    .name   = "gpio_ak37d",
    .id = UCLASS_GPIO,
    .of_match = ak37d_gpio_ids,
    .ops    = &gpio_ak37d_ops,
    .probe  = ak37d_gpio_probe,
};
