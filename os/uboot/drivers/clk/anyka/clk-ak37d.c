// SPDX-License-Identifier: GPL-2.0+

#include <asm/io.h>
#include <clk-uclass.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <linux/clk-provider.h>

#include "clk-ak37d.h"

#define ak_log_print(fmt, arg...) printf(fmt, ##arg)

#define MHz                            (1000000UL)
#define CLOCK_FREQ_ADJ_WAITING_MAX_NUM (100000)

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
    pll_m = (regval >> CLOCK_CORE_PLL_M_CFG_SHIFT) & CLOCK_CORE_PLL_M_CFG_MASK;
    pll_n = (regval >> CLOCK_CORE_PLL_N_CFG_SHIFT) & CLOCK_CORE_PLL_N_CFG_MASK;
    pll_od
        = (regval >> CLOCK_CORE_PLL_OD_CFG_SHIFT) & CLOCK_CORE_PLL_OD_CFG_MASK;
    core_pll_clk = 12 * pll_m / (pll_n * (1 << pll_od)); // clk unit: MHz

    return core_pll_clk * MHz;
}

/*
 * @BRIEF       ak get peri pll clk
 * @PARAM[in]   void
 * @RETURN      ulong
 * @RETVAL      peri_pll_clk * MHz
 */
static ulong ak_get_peri_pll_clk(void)
{
    u32 pll_m, pll_n, pll_od;
    u32 peri_pll_clk;
    u32 regval;

    regval = readl(CLOCK_PERI_PLL_CTRL1);
    pll_m = (regval >> CLOCK_PERI_PLL_M_CFG_SHIFT) & CLOCK_PERI_PLL_M_CFG_MASK;
    pll_n = (regval >> CLOCK_PERI_PLL_N_CFG_SHIFT) & CLOCK_PERI_PLL_N_CFG_MASK;
    pll_od
        = (regval >> CLOCK_PERI_PLL_OD_CFG_SHIFT) & CLOCK_PERI_PLL_OD_CFG_MASK;
    peri_pll_clk = 12 * pll_m / (pll_n * (1 << pll_od)); // clk unit: MHz

    return peri_pll_clk * MHz;
}

/*
 * @BRIEF       ak get vclk
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
 * @BRIEF       ak gate clk get ctrlbit
 * @PARAM[in]   id
 * @RETURN      int
 * @RETVAL      ctrlbit:-1
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
        case CORE_GCLK_ENCRYTION:
            ctrlbit = GCLK_ENCRYPTION_CFG_SHIFT;
            break;
        case CORE_GCLK_USB:
            ctrlbit = GCLK_USB_CFG_SHIFT;
            break;
        case CORE_GCLK_MIPI:
            ctrlbit = GCLK_MIPI_CFG_SHIFT;
            break;
        case CORE_GCLK_TWI1:
            ctrlbit = GCLK_TWI1_CFG_SHIFT;
            break;
        case CORE_GCLK_ISP:
            ctrlbit = GCLK_ISP_CFG_SHIFT;
            break;
        case CORE_GCLK_VENCODER:
            ctrlbit = GCLK_VENCODER_CFG_SHIFT;
            break;
        case CORE_GCLK_VDECODER:
            ctrlbit = GCLK_VDECODER_CFG_SHIFT;
            break;
        case CORE_GCLK_LCD:
            ctrlbit = GCLK_LCD_CFG_SHIFT;
            break;
        case CORE_GCLK_GUI:
            ctrlbit = GCLK_GUI_CFG_SHIFT;
            break;
        case CORE_GCLK_UART2:
            ctrlbit = GCLK_UART2_CFG_SHIFT;
            break;
        case CORE_GCLK_UART3:
            ctrlbit = GCLK_UART3_CFG_SHIFT;
            break;
        case CORE_GCLK_DSI:
            ctrlbit = GCLK_DSI_CFG_SHIFT;
            break;
        case CORE_GCLK_MMC2:
            ctrlbit = GCLK_MMC2_CFG_SHIFT;
            break;
        case CORE_GCLK_TWI2:
            ctrlbit = GCLK_TWI2_CFG_SHIFT;
            break;
        case CORE_GCLK_TWI3:
            ctrlbit = GCLK_TWI3_CFG_SHIFT;
            break;
    }

    return ctrlbit;
}
/*end of ak_gate_clk_get_ctrlbit*/

/*
 * @BRIEF       ak gate clk get rstbit
 * @PARAM[in]   id
 * @RETURN      int
 * @RETVAL      rstbit:-1
 */
static int ak_gate_clk_get_rstbit(int id)
{
    int rstbit = -1;

    switch (id) {
        case CORE_GCLK_MMC0:
            rstbit = GCLK_MMC0_RST_SHIFT;
            break;
        case CORE_GCLK_MMC1:
            rstbit = GCLK_MMC1_RST_SHIFT;
            break;
        case CORE_GCLK_SD_ADC:
            rstbit = GCLK_SD_ADC_RST_SHIFT;
            break;
        case CORE_GCLK_SD_DAC:
            rstbit = GCLK_SD_DAC_RST_SHIFT;
            break;
        case CORE_GCLK_SPI0:
            rstbit = GCLK_SPI0_RST_SHIFT;
            break;
        case CORE_GCLK_SPI1:
            rstbit = GCLK_SPI1_RST_SHIFT;
            break;
        case CORE_GCLK_UART0:
            rstbit = GCLK_UART0_RST_SHIFT;
            break;
        case CORE_GCLK_UART1:
            rstbit = GCLK_UART1_RST_SHIFT;
            break;
        case CORE_GCLK_L2BUF:
            rstbit = GCLK_L2BUF_RST_SHIFT;
            break;
        case CORE_GCLK_TWI0:
            rstbit = GCLK_TWI0_RST_SHIFT;
            break;
        case CORE_GCLK_GPIO:
            rstbit = GCLK_GPIO_RST_SHIFT;
            break;
        case CORE_GCLK_MAC:
            rstbit = GCLK_MAC_RST_SHIFT;
            break;
        case CORE_GCLK_ENCRYTION:
            rstbit = GCLK_ENCRYPTION_RST_SHIFT;
            break;
        case CORE_GCLK_USB:
            rstbit = GCLK_USB_RST_SHIFT;
            break;
        case CORE_GCLK_MIPI:
            rstbit = GCLK_MIPI_RST_SHIFT;
            break;
        case CORE_GCLK_TWI1:
            rstbit = GCLK_TWI1_RST_SHIFT;
            break;
        case CORE_GCLK_ISP:
            rstbit = GCLK_ISP_RST_SHIFT;
            break;
        case CORE_GCLK_VENCODER:
            rstbit = GCLK_VENCODER_RST_SHIFT;
            break;
        case CORE_GCLK_VDECODER:
            rstbit = GCLK_VDECODER_RST_SHIFT;
            break;
        case CORE_GCLK_LCD:
            rstbit = GCLK_LCD_RST_SHIFT;
            break;
        case CORE_GCLK_GUI:
            rstbit = GCLK_GUI_RST_SHIFT;
            break;
        case CORE_GCLK_UART2:
            rstbit = GCLK_UART2_RST_SHIFT;
            break;
        case CORE_GCLK_UART3:
            rstbit = GCLK_UART3_RST_SHIFT;
            break;
        case CORE_GCLK_DSI:
            rstbit = GCLK_DSI_RST_SHIFT;
            break;
        case CORE_GCLK_MMC2:
            rstbit = GCLK_MMC2_RST_SHIFT;
            break;
        case CORE_GCLK_TWI2:
            rstbit = GCLK_TWI2_RST_SHIFT;
            break;
        case CORE_GCLK_TWI3:
            rstbit = GCLK_TWI3_RST_SHIFT;
            break;
    }

    return rstbit;
}
/*end of ak_gate_clk_get_rstbit*/

/*
 * @BRIEF       ak gate clk get rstbit
 * @PARAM[in]      res_reg
 * @PARAM[in]      shift
 * @PARAM[in]      mask
 * @RETURN      int
 * @RETVAL      0
 */
static int ak_clock_divider_adjust_complete(
    void __iomem* res_reg, u32 shift, u32 mask)
{
    int timeout = CLOCK_FREQ_ADJ_WAITING_MAX_NUM;
    u32 regval;

    do {
        timeout--;
        regval = ((readl(res_reg)) >> shift) & mask;
    } while (regval && (timeout > 0));

    if (timeout <= 0) {
        pr_err("Waiting 0x%p:(0x%x) shift %d mask 0x%x timeout\n", res_reg,
            readl(res_reg), shift, mask);
    }

    return 0;
}

/*
 * @BRIEF       ak factor clk set rate
 * @PARAM[in]      clk
 * @PARAM[in]      rate
 * @RETURN      ulong
 * @RETVAL      new_rate
 */
static ulong ak_factor_clk_set_rate(struct clk* clk, ulong rate)
{
    u32 regval = 0, new_rate = 0, factor = 0;
    ulong parent_rate = 0;

    switch (clk->id) {
        case CORE_FACTOR_SPI0_CLK:
            parent_rate = ak_get_core_pll_clk();
            factor = (parent_rate) / (rate * 2) - 1;
            /* hsclk must be disable,before adjustment clock divider parameter.
             */
            regval = readl(CLOCK_SPI_HIGH_SPEED_CTRL);
            regval &= ~(0x1 << CLOCK_SPI_HSCLK_EN_CFG_SHIFT);
            writel(regval, (void __iomem*)CLOCK_SPI_HIGH_SPEED_CTRL);

            regval = readl(CLOCK_SPI_HIGH_SPEED_CTRL);
            regval &= ~(CLOCK_SPI_HSCLK_DIV_NUM_CFG_MASK);
            regval |= (CLOCK_SPI_HSCLK_DIV_NUM_CFG_MASK & factor);
            writel(regval, (void __iomem*)CLOCK_SPI_HIGH_SPEED_CTRL);

            regval = __raw_readl(CLOCK_SPI_HIGH_SPEED_CTRL);
            regval |= (0x1 << CLOCK_SPI_HSDIV_VLD_CFG_SHIFT);
            __raw_writel(regval, CLOCK_SPI_HIGH_SPEED_CTRL);
            ak_clock_divider_adjust_complete(
                (void __iomem*)CLOCK_SPI_HIGH_SPEED_CTRL,
                CLOCK_SPI_HSDIV_VLD_CFG_SHIFT, CLOCK_SPI_HSDIV_VLD_CFG_MASK);
            regval |= (0x1 << CLOCK_SPI_HSCLK_EN_CFG_SHIFT);
            __raw_writel(regval, CLOCK_SPI_HIGH_SPEED_CTRL);

            new_rate = parent_rate / (2 * (factor + 1));
            break;
        case PERI_FACTOR_MAC_PCLK:
            /*
             * config as fixed 50MHz
             */
            regval = readl(CLOCK_PERI_PLL_CTRL1);
            regval &= ~(
                0x1 << CLOCK_PERI_PLL_MAC_RELATE_CLK_SHIFT); // first set mac
                                                             // interface select
                                                             // Rmii
            writel(regval, (void __iomem*)CLOCK_PERI_PLL_CTRL1);
            regval |= (0x1
                << CLOCK_PERI_PLL_MAC_SPEED_SHIFT); // mac_speed_cfg=1(100m)
            writel(regval, (void __iomem*)CLOCK_PERI_PLL_CTRL1);
            regval |= (0x1
                << CLOCK_PERI_PLL_CLK50M_EN_SHIFT); // set bit[21],enable
                                                    // generate 50m
            writel(regval, (void __iomem*)CLOCK_PERI_PLL_CTRL1);
            regval |= ((0x1 << CLOCK_PERI_PLL_MAC_RESERVED1_SHIFT)
                | (0x1
                    << CLOCK_PERI_PLL_MAC_RESERVED2_SHIFT)); // set
                                                             // bit[28],enable
                                                             // generate 50m,
                                                             // bit[18], select
                                                             // 25m clock of mac
                                                             // from pll div
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
 * @BRIEF       ak factor clk get rate
 * @PARAM[in]      clk
 * @RETURN      ulong
 * @RETVAL      rate
 */
static ulong ak_factor_clk_get_rate(struct clk* clk)
{
    u32 rate = 0, factor = 0, regval = 0;
    ulong parent_rate = 0;

    switch (clk->id) {
        case CORE_FACTOR_SPI0_CLK:
            parent_rate = ak_get_core_pll_clk();
            regval = __raw_readl(CLOCK_SPI_HIGH_SPEED_CTRL);
            factor = regval & CLOCK_SPI_HSCLK_DIV_NUM_CFG_MASK;
            rate = parent_rate / (2 * (factor + 1));
            break;
        case PERI_FACTOR_MAC_PCLK:
            rate = 50 * MHz;
            break;
        default:
            break;
    }

    //  pr_err("factor CLK get new_rate %d on FactorCLK_%ld\n", rate, clk->id);

    return rate;
}

static const struct clk_ops ak_factor_clk_ops = {
    .set_rate = ak_factor_clk_set_rate,
    .get_rate = ak_factor_clk_get_rate,
};

/*
 * @BRIEF       ak factor clk probe
 * @PARAM[in]   dev
 * @RETURN      int
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

U_BOOT_DRIVER(ak37d_factor_clk) = {
    .name = "ak37d_factor_clk",
    .id = UCLASS_CLK,
    .of_match = ak_fator_clk_ids,
    .probe = ak_fator_clk_probe,
    .ops = &ak_factor_clk_ops,
};

/*
 * @BRIEF       ak gate clk enable
 * @PARAM[in]   clk
 * @RETURN      int
 * @RETVAL      0
 */
static int ak_gate_clk_enable(struct clk* clk)
{
    u32 regval;
    int ctrlbit = -1, rstbit = -1;
    void __iomem* cfg_reg;
    void __iomem* rst_reg;

    // ak_log_print("%s: id %lu\n", __func__, clk->id);

    ctrlbit = ak_gate_clk_get_ctrlbit(clk->id);
    if (ctrlbit < 0) {
        pr_err("%s: fail ctrlbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }

    rstbit = ak_gate_clk_get_rstbit(clk->id);
    if (rstbit < 0) {
        pr_err("%s: fail rstbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }

    cfg_reg = (void __iomem*)CLOCK_GATE_CTRL1;
    rst_reg = (void __iomem*)CLOCK_SOFT_RESET;
    if ((clk->id >= CORE_GCLK_UART2) && (clk->id <= CORE_GCLK_TWI3)) {
        cfg_reg = (void*)CLOCK_GATE_RESET_CTRL2;
        rst_reg = (void*)CLOCK_GATE_RESET_CTRL2;
    }

    regval = readl(cfg_reg);
    regval &= ~(1 << ctrlbit);
    writel(regval, cfg_reg);

    regval = readl(rst_reg);
    regval |= (0x1 << ctrlbit);
    writel(regval, rst_reg);
    mdelay(1);
    regval = readl(rst_reg);
    regval &= ~(1 << ctrlbit);
    writel(regval, rst_reg);

    return 0;
}

/*
 * @BRIEF       ak gate clk disable
 * @PARAM[in]   clk
 * @RETURN      int
 * @RETVAL      0
 */
static int ak_gate_clk_disable(struct clk* clk)
{
    u32 regval;
    int ctrlbit = -1;
    void __iomem* cfg_reg;

    // ak_log_print("%s: id %lu\n", __func__, clk->id);

    ctrlbit = ak_gate_clk_get_ctrlbit(clk->id);
    if (ctrlbit < 0) {
        pr_err("%s: fail ctrlbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }

    cfg_reg = (void __iomem*)CLOCK_GATE_CTRL1;
    if ((clk->id >= CORE_GCLK_UART2) && (clk->id <= CORE_GCLK_TWI3)) {
        cfg_reg = (void*)CLOCK_GATE_RESET_CTRL2;
    }

    regval = readl(cfg_reg);
    regval |= (0x1 << ctrlbit);
    writel(regval, cfg_reg);

    return 0;
}

/*
 * @BRIEF       ak gate clk get rate
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
        case CORE_GCLK_VDECODER:
        case CORE_GCLK_LCD:
        case CORE_GCLK_GUI:
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

/*
 * @BRIEF       ak gate clk probe
 * @PARAM[in]   dev
 * @RETURN      int
 * @RETVAL      0/-ENODEV
 */
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

U_BOOT_DRIVER(ak37d_gate_clk) = {
    .name = "ak37d_gate_clk",
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
            rate = ak_get_peri_pll_clk();
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
    debug("CORE: %lu(Mhz) VCLK: %lu(Mhz)\n", ak_get_core_pll_clk() / MHz,
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

U_BOOT_DRIVER(ak37d_fixed_clk) = {
    .name = "ak37d_fixed_clk",
    .id = UCLASS_CLK,
    .of_match = ak_fixed_clk_ids,
    .probe = ak_fixed_clk_probe,
    .ops = &ak_fixed_clk_ops,
    .priv_auto_alloc_size = sizeof(struct ak_fixed_priv_data),
};
