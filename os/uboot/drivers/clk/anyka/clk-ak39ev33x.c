// SPDX-License-Identifier: GPL-2.0+

#include <asm/io.h>
#include <clk-uclass.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <linux/clk-provider.h>

#include "clk-ak39ev33x.h"

#define ak_log_print(fmt, arg...) printf(fmt, ##arg)

#define MHz (1000000UL)

struct ak_fixed_priv_data {
    int clk_id;
};

/*
 * @BRIEF       ak get core pll clk
 * @PARAM[in]   void
 * @RETURN      ulong
 * @RETVAL      core_pll_clk * MHz
 */
static ulong ak_get_core_pll_clk(void)
{
    u32 pll_m, pll_n, pll_od;
    u32 core_pll_clk;
    u32 regval;

    regval = readl(CLOCK_CORE_PLL_CTRL);
    pll_m = (regval & 0xff);
    pll_n = (regval >> 8) & 0xf;
    pll_od = (regval >> 12) & 0x3;
    core_pll_clk = 12 * pll_m / (pll_n * (1 << pll_od)); // clk unit: MHz

    return core_pll_clk * MHz;
}

/*
 * @BRIEF       ak get clk
 * @PARAM[in]   void
 * @RETURN      ulong
 * @RETVAL      vclk * MHz
 */
static ulong ak_get_vclk(void)
{
    u32 regval;
    u32 vclk;

    regval = readl(CLOCK_CORE_PLL_CTRL);
    regval = ((regval >> 17) & 0x7);
    if (regval == 0)
        regval = 1;
    vclk = (ak_get_core_pll_clk() / MHz) >> (regval);

    return vclk * MHz;
}

/*
 * @BRIEF       ak_gate_clk_get_ctrlbit
 * @PARAM[in]   id
 * @RETURN      int
 * @RETVAL      ctrlbit
 */
static int ak_gate_clk_get_ctrlbit(int id)
{
    int ctrlbit = -1;

    switch (id) {
        case CORE_GCLK_MMC0:
            ctrlbit = GCLK_MMC0_CFG_SHIFT;
            break;
        case CORE_GCLK_MMC1:
            ctrlbit = GCLK_MMC1_CFG_SHIFT;
            break;
        case CORE_GCLK_SD_ADC:
            ctrlbit = GCLK_SD_ADC_CFG_SHIFT;
            break;
        case CORE_GCLK_SD_DAC:
            ctrlbit = GCLK_SD_DAC_CFG_SHIFT;
            break;
        case CORE_GCLK_SPI0:
            ctrlbit = GCLK_SPI0_CFG_SHIFT;
            break;
        case CORE_GCLK_SPI1:
            ctrlbit = GCLK_SPI1_CFG_SHIFT;
            break;
        case CORE_GCLK_UART0:
            ctrlbit = GCLK_UART0_CFG_SHIFT;
            break;
        case CORE_GCLK_UART1:
            ctrlbit = GCLK_UART1_CFG_SHIFT;
            break;
        case CORE_GCLK_L2BUF:
            ctrlbit = GCLK_L2BUF_CFG_SHIFT;
            break;
        case CORE_GCLK_TWI0:
            ctrlbit = GCLK_TWI0_CFG_SHIFT;
            break;
        case CORE_GCLK_GPIO:
            ctrlbit = GCLK_GPIO_CFG_SHIFT;
            break;
        case CORE_GCLK_MAC:
            ctrlbit = GCLK_MAC_CFG_SHIFT;
            break;
        case CORE_GCLK_USB:
            ctrlbit = GCLK_USB_CONTROLLER_CFG_SHIFT;
            break;
        case CORE_GCLK_MIPI:
            ctrlbit = GCLK_MIPI_CFG_SHIFT;
            break;
        case CORE_GCLK_TWI1:
            ctrlbit = GCLK_TWI1_CFG_SHIFT;
            break;
        case CORE_GCLK_ISP:
            ctrlbit = GCLK_ISP_RELEATED_CFG_SHIFT;
            break;
        case CORE_GCLK_VENCODER:
            ctrlbit = GCLK_VENCODER_CFG_SHIFT;
            break;
        case CORE_GCLK_TWI2:
            ctrlbit = GCLK_TWI2_CFG_SHIFT;
            break;
    }

    return ctrlbit;
}

/*
 * @BRIEF       ak_factor_clk_set_rate
 * @PARAM[in]   clk
 * @PARAM[in]   rate
 * @RETURN      ulong
 * @RETVAL      new_rate
 */
static ulong ak_factor_clk_set_rate(struct clk* clk, ulong rate)
{
    u32 regval = 0, new_rate = 0;

    switch (clk->id) {
        case PERI_FACTOR_MAC_PCLK:
            /*
             * config as fixed 50MHz
             */
            regval = readl(CLOCK_PERI_PLL_CTRL1);
            regval &= ~(0x1 << 22); // first set mac interface select Rmii
            writel(regval, (void __iomem*)CLOCK_PERI_PLL_CTRL1);
            regval |= (0x1 << 23); // mac_speed_cfg=1(100m)
            writel(regval, (void __iomem*)CLOCK_PERI_PLL_CTRL1);
            regval |= (0x1 << 21); // set bit[21],enable generate 50m
            writel(regval, (void __iomem*)CLOCK_PERI_PLL_CTRL1);
            /*
            **set bit[28],enable generate 50m,
            **    bit[18], select 25m clock of mac from pll div
            */
            regval |= (0x1 << 18 | 0x1 << 28);
            writel(regval, (void __iomem*)CLOCK_PERI_PLL_CTRL1);
            new_rate = 50 * MHz;
            break;
        default:
            pr_err("factor CLK set rate %lu on FactorCLK_%ld\n", rate, clk->id);
            break;
    }

    // ak_log_print("factor CLK set new_rate %d on FactorCLK_%ld\n", new_rate,
    // clk->id);

    return new_rate;
}

/*
 * @BRIEF       ak_factor_clk_get_rate
 * @PARAM[in]   clk
 * @RETURN      ulong
 * @RETVAL      rate
 */
static ulong ak_factor_clk_get_rate(struct clk* clk)
{
    u32 rate = 0;

    switch (clk->id) {
        case PERI_FACTOR_MAC_PCLK:
            rate = 50 * MHz;
            break;
        default:
            break;
    }

    // pr_err("factor CLK get new_rate %d on FactorCLK_%ld\n", rate, clk->id);

    return rate;
}

static const struct clk_ops ak_factor_clk_ops = {
    .set_rate = ak_factor_clk_set_rate,
    .get_rate = ak_factor_clk_get_rate,
};

/*
 * @BRIEF       ak_fator_clk_probe
 * @PARAM[in]   dev
 * @RETURN      ulong
 * @RETVAL      0
 */
static int ak_fator_clk_probe(struct udevice* dev)
{
    struct clk clk;
    int ret;

    ret = clk_get_by_index(dev, 0, &clk);
    if (ret) {
        ak_log_print("FactorCLK fail to get parent\n");
        return -ENODEV;
    }

    // ak_log_print("FactorCLK Parent: %lu(MHz)\n", clk_get_rate(&clk)/MHz);

    return 0;
}

static const struct udevice_id ak_fator_clk_ids[]
    = { { .compatible = "anyka,fator-clk" }, {} };

U_BOOT_DRIVER(ak39ev33x_factor_clk) = {
    .name = "ak39ev330_factor_clk",
    .id = UCLASS_CLK,
    .of_match = ak_fator_clk_ids,
    .probe = ak_fator_clk_probe,
    .ops = &ak_factor_clk_ops,
};

/*
 * @BRIEF       ak_gate_clk_enable
 * @PARAM[in]   clk
 * @RETURN      int
 * @RETVAL      0
 */
static int ak_gate_clk_enable(struct clk* clk)
{
    u32 regval;
    int ctrlbit = -1;

    // ak_log_print("%s: id %lu\n", __func__, clk->id);

    ctrlbit = ak_gate_clk_get_ctrlbit(clk->id);
    if (ctrlbit < 0) {
        pr_err("%s: fail ctrlbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }

    regval = readl(CLOCK_GATE_CTRL1);
    regval &= ~(1 << ctrlbit);
    writel(regval, (void __iomem*)CLOCK_GATE_CTRL1);

    regval = readl(CLOCK_SOFT_RESET);
    regval |= (0x1 << ctrlbit);
    writel(regval, (void __iomem*)CLOCK_SOFT_RESET);
    mdelay(1);
    regval = readl(CLOCK_SOFT_RESET);
    regval &= ~(1 << ctrlbit);
    writel(regval, (void __iomem*)CLOCK_SOFT_RESET);

    return 0;
}

/*
 * @BRIEF       ak_gate_clk_disable
 * @PARAM[in]   clk
 * @RETURN      int
 * @RETVAL      0
 */
static int ak_gate_clk_disable(struct clk* clk)
{
    u32 regval;
    int ctrlbit = -1;

    // ak_log_print("%s: id %lu\n", __func__, clk->id);

    ctrlbit = ak_gate_clk_get_ctrlbit(clk->id);
    if (ctrlbit < 0) {
        pr_err("%s: fail ctrlbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }

    regval = readl(CLOCK_GATE_CTRL1);
    regval |= (0x1 << ctrlbit);
    writel(regval, (void __iomem*)CLOCK_GATE_CTRL1);

    return 0;
}

/*
 * @BRIEF       ak_gate_clk_get_rate
 * @PARAM[in]   clk
 * @RETURN      ulong
 * @RETVAL      rate
 */
static ulong ak_gate_clk_get_rate(struct clk* clk)
{
    u32 regval = readl(CLOCK_CORE_PLL_CTRL);
    ulong rate;
    ulong core_pll_clk = ak_get_core_pll_clk();
    int ctrlbit = -1;

    ctrlbit = ak_gate_clk_get_ctrlbit(clk->id);
    if (ctrlbit < 0) {
        pr_err("%s: fail ctrlbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }

    regval = ((regval >> 17) & 0x7);
    if (regval == 0)
        regval = 1;

    switch (clk->id) {
        case CORE_GCLK_ISP:
        case CORE_GCLK_VENCODER:
        case CORE_GCLK_TWI2:
            rate = core_pll_clk >> (regval);
            break;
        default:
            rate = core_pll_clk >> (regval);
            regval = readl(CLOCK_CORE_PLL_CTRL);
            regval = ((regval >> 24) & 0x1);
            if (regval)
                rate >>= 1;
            break;
    }

    // ak_log_print("%s: %luHz@GCLK%lu\n", __func__, rate, clk->id);

    return rate;
}

static const struct clk_ops ak_gate_clk_ops = {
    .enable = ak_gate_clk_enable,
    .disable = ak_gate_clk_disable,
    .get_rate = ak_gate_clk_get_rate,
};

static int ak_gate_clk_probe(struct udevice* dev)
{
    struct clk clk;
    int ret;

    ret = clk_get_by_index(dev, 0, &clk);
    if (ret) {
        ak_log_print("GCLK fail to get parent\n");
        return -ENODEV;
    }

    // ak_log_print("GCLK Parent:%lu(MHz)\n", clk_get_rate(&clk)/MHz);

    return 0;
}

static const struct udevice_id ak_gate_clk_ids[]
    = { { .compatible = "anyka,gate-clk" }, {} };

U_BOOT_DRIVER(ak39ev33x_gate_clk) = {
    .name = "ak39ev330_gate_clk",
    .id = UCLASS_CLK,
    .of_match = ak_gate_clk_ids,
    .probe = ak_gate_clk_probe,
    .ops = &ak_gate_clk_ops,
};

static ulong ak_fixed_clk_get_rate(struct clk* clk)
{
    struct udevice* dev = clk->dev;
    struct ak_fixed_priv_data* priv = dev_get_priv(dev);
    ulong rate = 0;

    switch (priv->clk_id) {
        case CORE_PLL_CLK:
            rate = ak_get_core_pll_clk();
            break;
        case PERI_PLL_CLK:
            rate = 0;
            break;
        default:
            break;
    }

    return rate;
}

static const struct clk_ops ak_fixed_clk_ops = {
    .get_rate = ak_fixed_clk_get_rate,
};

static int ak_fixed_core_pll_init(u32 core_pll, u32 div_od, u32 div_n)
{
    u32 div_m, regval;
    u32 rd_div_m, rd_div_n, rd_div_od;

    if ((div_n < 1) || (div_n > 12))
        return -EINVAL;

    /* core_pll = 480M=2vclk; vclk=240M=4gclk; gclk=120M */
    div_m = ((core_pll / MHz) * (div_n * (1 << div_od))) / 12;

    if (div_m < 2)
        return -EINVAL;

    regval = readl(CLOCK_CPU_PLL_CTRL);
    rd_div_m
        = (regval >> CLOCK_CORE_PLL_M_CFG_SHIFT) & CLOCK_CORE_PLL_M_CFG_MASK;
    rd_div_n
        = (regval >> CLOCK_CORE_PLL_N_CFG_SHIFT) & CLOCK_CORE_PLL_N_CFG_MASK;
    rd_div_od
        = (regval >> CLOCK_CORE_PLL_OD_CFG_SHIFT) & CLOCK_CORE_PLL_OD_CFG_MASK;

    if ((div_m == rd_div_m) && (rd_div_n == div_n) && (rd_div_od == div_od)) {
        // ak_log_print("CORE: has inited\n");
        goto exit;
    }

    /* set core pll frequency */
    regval = ((1 << 24) | (1 << 23) | (1 << 17) | (div_od << 12) | (div_n << 8)
        | (div_m));
    writel(regval, (void __iomem*)CLOCK_CORE_PLL_CTRL);

    /* enable core pll freq change valid */
    regval = readl(CLOCK_CPU_PLL_CTRL);
    regval |= (1 << 28);
    writel(regval, (void __iomem*)CLOCK_CPU_PLL_CTRL);

exit:
    ak_log_print("CORE: %lu(Mhz) VCLK: %lu(Mhz)\n", ak_get_core_pll_clk() / MHz,
        ak_get_vclk() / MHz);

    return 0;
}

static int ak_fixed_peri_pll_init(u32 peri_pll, u32 div_od, u32 div_n)
{
    u32 div_m;
    u32 regval;
    u32 rd_div_m, rd_div_n, rd_div_od;

    if ((div_n < 2) || (div_n > 6) || (div_od < 1) || (div_od > 3)) {
        return -EINVAL;
    }

    /* peri_pll = 600M */
    div_m = ((peri_pll / MHz) * (div_n * (1 << div_od))) / 12;

    regval = readl(CLOCK_PERI_PLL_CTRL1);
    rd_div_m
        = (regval >> CLOCK_PERI_PLL_M_CFG_SHIFT) & CLOCK_PERI_PLL_M_CFG_MASK;
    rd_div_n
        = (regval >> CLOCK_PERI_PLL_N_CFG_SHIFT) & CLOCK_PERI_PLL_N_CFG_MASK;
    rd_div_od
        = (regval >> CLOCK_PERI_PLL_OD_CFG_SHIFT) & CLOCK_PERI_PLL_OD_CFG_MASK;

    if ((div_m == rd_div_m) && (rd_div_n == div_n) && (rd_div_od == div_od)) {
        // ak_log_print("PERI: has inited\n");
        goto exit;
    }

    /* set peri pll frequency */
    regval = readl(CLOCK_PERI_PLL_CTRL1);
    regval &= ~((0x3 << 12) | (0xf << 8) | 0xff);
    regval |= ((div_od << 12) | (div_n << 8) | (div_m));
    writel(regval, (void __iomem*)CLOCK_PERI_PLL_CTRL1);

    /* enable peri pll freq change valid */
    regval = readl(CLOCK_CPU_PLL_CTRL);
    regval |= (1 << 29);
    writel(regval, (void __iomem*)CLOCK_CPU_PLL_CTRL);

exit:
    return 0;
}

static int ak_fixed_clk_probe(struct udevice* dev)
{
    struct ak_fixed_priv_data* priv = dev_get_priv(dev);
    u32 div_n, div_od, clk_freq, clk_id;
    int ret = 0;

    div_n = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "clock-div-n", 0);
    div_od
        = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "clock-div-od", 0);
    clk_freq = fdtdec_get_int(
        gd->fdt_blob, dev_of_offset(dev), "clock-frequency", 0);
    clk_id = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "clock-id", 0);

    // ak_log_print("%s: n %d od %d pll %d id %d\n", __func__, div_n, div_od,
    // clk_freq, clk_id);

    priv->clk_id = clk_id;

    switch (clk_id) {
        case CORE_PLL_CLK:
            ret = ak_fixed_core_pll_init(clk_freq, div_od, div_n);
            break;
        case PERI_PLL_CLK:
            ret = ak_fixed_peri_pll_init(clk_freq, div_od, div_n);
            break;
        default:
            ret = -EINVAL;
            break;
    }

    // ak_log_print("ak_fixed: probe @%d with %d\n", priv->clk_id, ret);

    return ret;
}

static const struct udevice_id ak_fixed_clk_ids[]
    = { { .compatible = "anyka,fixed-clk" }, {} };

U_BOOT_DRIVER(ak39ev33x_fixed_clk) = {
    .name = "ak39ev330_fixed_clk",
    .id = UCLASS_CLK,
    .of_match = ak_fixed_clk_ids,
    .probe = ak_fixed_clk_probe,
    .ops = &ak_fixed_clk_ops,
    .priv_auto_alloc_size = sizeof(struct ak_fixed_priv_data),
};
