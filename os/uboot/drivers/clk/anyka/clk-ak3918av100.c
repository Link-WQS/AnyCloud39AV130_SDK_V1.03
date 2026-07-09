// SPDX-License-Identifier: GPL-2.0+

#include <asm/io.h>
#include <clk-uclass.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <linux/clk-provider.h>

#include "clk-ak3918av100.h"

#define ak_log_print(fmt, arg...) printf(fmt, ##arg)

#define MHz (1000000UL)

#define CLOCK_FREQ_ADJ_WAITING_MAX_NUM (100000)

/**
 * @brief get PLL2 clock frequency
 *
 * @return ulong frequency
 */
static ulong ak_get_pll2_freq(void)
{
    u32 nf, nr, od;
    ulong pll2_freq;
    u32 regval;

    /*
     * PLL1 output frequency= 24*NF/NR/OD
     *      NF = PLL1_CLKF_CFG[12:0] + 1
     *      NR = PLL1_CLKR_CFG[5:0] + 1
     *      OD = PLL1_CLKOD_CFG[3:0] + 1
     */
    regval = __raw_readl(CLOCK_PLL2_CTRL);
    od = ((regval >> PLL2_CLKOD_CFG_SHIFT) & PLL2_CLKOD_CFG_MASK) + 1;
    nf = ((regval >> PLL2_CLKF_CFG_SHIFT) & PLL2_CLKF_CFG_MASK) + 1;
    nr = ((regval >> PLL2_CLKR_CFG_SHIFT) & PLL2_CLKR_CFG_MASK) + 1;
    pll2_freq = 24 * MHz * nf / nr / od;

    return pll2_freq;
}

/**
 * @brief get gclk clock frequency
 *
 * @return ulong frequency
 */
static ulong ak_get_gclk_freq(void)
{
    ulong parent_rate = ak_get_pll2_freq();
    u32 factor = 0, rate = 0;
    u32 regval;

    /*
     * GCLK frequency division coefficient configuration
     *      By default, The output frequency of GCLK is 60MHz.
     *      GCLK = PLL2_CLKOUT/2(gclk_div_num_cfg[1:0]+1)
     */
    regval = readl((void __iomem*)CLOCK_PLL2_CTRL);
    factor = (regval >> GCLK_DIV_NUM_CFG_SHIFT) & GCLK_DIV_NUM_CFG_MASK;
    rate = parent_rate / (2 * (factor + 1));

    return rate;
}

/**
 * @brief get gate ctrl bit
 *
 * @param id clock id
 * @return int -EINVAL:error  other:complete
 */
static int ak_get_gate_ctrlbit(int id)
{
    int ctrlbit = -EINVAL;

    switch (id) {
        case CLOCK_ID_GCLK_MMC0:
            ctrlbit = GCLK_MMC0_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_SD_ADC:
            ctrlbit = GCLK_SD_ADC_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_SD_DAC:
            ctrlbit = GCLK_SD_DAC_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_SPI0:
            ctrlbit = GCLK_SPI0_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_UART0:
            ctrlbit = GCLK_UART0_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_ENCRYPTION:
            ctrlbit = GCLK_ENCRYPTION_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_MAC:
            ctrlbit = GCLK_MAC_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_USB:
            ctrlbit = GCLK_USB_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_MIPI0:
            ctrlbit = GCLK_MIPI0_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_UART2:
            ctrlbit = GCLK_UART2_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_SPI2:
            ctrlbit = GCLK_SPI2_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_MIPI1:
            ctrlbit = GCLK_MIMP1_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_MMC2:
            ctrlbit = GCLK_MMC2_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_TWI1:
            ctrlbit = GCLK_TWI1_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_TWI2:
            ctrlbit = GCLK_TWI2_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_MMC1:
            ctrlbit = GCLK_MMC1_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_SPI1:
            ctrlbit = GCLK_SPI1_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_L2BUF0:
            ctrlbit = GCLK_L2BUF0_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_TWI0:
            ctrlbit = GCLK_TWI0_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_L2BUF1:
            ctrlbit = GCLK_L2BUF1_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_GPIO:
            ctrlbit = GCLK_GPIO_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_UART1:
            ctrlbit = GCLK_UART1_CFG_SHIFT;
            break;
        default:;
    }

    return ctrlbit;
} /* end of func */

/**
 * @brief get reset ctrl bit
 *
 * @param id clock id
 * @return int -EINVAL:error  other:complete
 */
static int ak_get_reset_ctrlbit(int id)
{
    int rstbit = -EINVAL;

    switch (id) {
        case CLOCK_ID_GCLK_MMC0:
            rstbit = GCLK_MMC0_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_SD_ADC:
            rstbit = GCLK_SDADC_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_SD_DAC:
            rstbit = GCLK_SDDAC_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_SPI0:
            rstbit = GCLK_SPI0_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_UART0:
            rstbit = GCLK_UART0_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_ENCRYPTION:
            rstbit = GCLK_ECRYPTION_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_MAC:
            rstbit = GCLK_MAC_BUFFER_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_USB:
            rstbit = GCLK_USB_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_MIPI0:
            rstbit = GCLK_MIPI0_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_UART2:
            rstbit = GCLK_UART2_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_SPI2:
            rstbit = GCLK_SPI2_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_MIPI1:
            rstbit = GCLK_MIPI1_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_MMC2:
            rstbit = GCLK_MMC2_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_TWI1:
            rstbit = GCLK_TWI1_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_TWI2:
            rstbit = GCLK_TWI2_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_MMC1:
            rstbit = GCLK_MMC1_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_SPI1:
            rstbit = GCLK_SPI1_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_L2BUF0:
            rstbit = GCLK_L2BUF0_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_TWI0:
            rstbit = GCLK_TWI0_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_L2BUF1:
            rstbit = GCLK_L2BUF1_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_GPIO:
            rstbit = GCLK_GPIO_RST_CFG_SHIFT;
            break;
        case CLOCK_ID_GCLK_UART1:
            rstbit = GCLK_UART1_RST_CFG_SHIFT;
            break;
        default:;
    }

    return rstbit;
} /* end of func */

/**
 * @brief wait PLL adjustment complete
 *
 * @param res_reg ctrl register address
 * @param shift ctrl bit
 * @param mask  mask
 * @return int 0:complete  -1:error
 */
static int ak_wait_pll_adjust_complete(
    void __iomem* res_reg, u32 shift, u32 mask)
{
    int timeout = CLOCK_FREQ_ADJ_WAITING_MAX_NUM;
    u32 regval;

    do {
        timeout--;
        regval = ((readl(res_reg)) >> shift) & mask;
    } while (regval && timeout);

    if (!timeout) {
        pr_err("Waiting 0x%p:(0x%x) shift %d mask 0x%x timeout\n", res_reg,
            readl(res_reg), shift, mask);
        return -1;
    }

    return 0;
}

static int ak_adjust_clk_even_divider_freq(void __iomem* res_reg,
    u32 div_vld_cfg_shift, u32 en_cfg_shift, u32 div_num_cfg,
    u32 div_num_cfg_shift, u32 div_num_cfg_mask)
{
    u32 regval;

    /*
     * [from PG : 3.2.6.3.1 Adjusting Clock Divider Frequency]
     * For those clocks with even divider, the configuration steps are similar.
     * Taking mac_opclk as an example, the steps of adjusting mac_opclk are as
     * follows: (1) Disable the module driven by mac_opclk(Ethernet MAC). (2)
     * Enable the output clock when mac_opclk_div_vld_cfg(bit[9] of
     *          CSI1_SCLK/CSI2_SCLK/MAC_OPCLK Clock Control(Address:
     * 0x0800,0018)) is 0 by setting mac_opclk_en_cfg(bit[8] of the same
     * register) to be 1. (3) Change the divider coefficient
     * mac_opclk_div_num_cfg(bits[5:0] of this register). (4) Set
     * mac_opclk_div_vld_cfg(bit[9] of this register) to be 1 and wait until it
     * is cleared. (5) Enable the module driven by mac_opclk(Ethernet MAC).
     */

    /* Step2: */
    regval = readl(res_reg);
    regval |= ((0x01) << en_cfg_shift);
    writel(regval, res_reg);

    /* Step3: */
    regval = readl(res_reg);
    regval &= ~(div_num_cfg_mask << div_num_cfg_shift);
    regval |= ((div_num_cfg & div_num_cfg_mask) << div_num_cfg_shift);

    /* Step4: */
    regval |= ((0x01) << div_vld_cfg_shift);
    writel(regval, res_reg);
    ak_wait_pll_adjust_complete(res_reg, div_vld_cfg_shift, 0x01);

    return 0;
}

static int ak_adjust_clk_int_divider_freq(void __iomem* res_reg,
    u32 div_vld_cfg_shift, u32 en_cfg_shift, u32 div_num_cfg,
    u32 div_num_cfg_shift, u32 div_num_cfg_mask)
{
    u32 regval;

    /*
     * [from PG : 3.2.6.3.1 Adjusting Clock Divider Frequency]
     * For those clocks with integer divider, the configuration steps are
     * similar. Taking sdadc_clk as an example, the steps of adjusting sdadc_clk
     * are as follows: (1) Disable the module driven by sdadc_clk(Sigma-Delta
     * ADC). (2) Set sdadc_clk_en_cfg(bit[8] of SD_ADC/SD_DAC High Speed Clock
     * Control(Address: 0x0800,0010) to be 0 to disable the output clock when
     * sdadc_clk_div_vld_cfg(bit[9] of the same register) is 0. (3) Change the
     * divider coefficient sdadc_clk_div_num_cfg(bits[7:0] of this register).
     *      (4) Set sdadc_clk_div_vld_cfg(bit[9] of the same register) to be 1
     * and wait until it is cleared. (5) Set sdadc_clk_en_cfg(bit[8] of this
     * register) to be 1 to enable the output clock. (6) Enable the module
     * driven by sdadc_clk(Sigma-Delta ADC).
     */

    /* Step2: */
    regval = readl(res_reg);
    regval &= ~((0x1) << en_cfg_shift);
    writel(regval, res_reg);

    /* Step3: */
    regval = readl(res_reg);
    regval &= ~(div_num_cfg_mask << div_num_cfg_shift);
    regval |= ((div_num_cfg & div_num_cfg_mask) << div_num_cfg_shift);
    writel(regval, res_reg);

    /* Step4: */
    regval = readl(res_reg);
    regval |= ((0x01) << div_vld_cfg_shift);
    writel(regval, res_reg);
    ak_wait_pll_adjust_complete(res_reg, div_vld_cfg_shift, 0x01);

    /* Step5: */
    regval = readl(res_reg);
    regval |= ((0x1) << en_cfg_shift);
    writel(regval, res_reg);

    return 0;
}

/**
 * @brief reset clock
 *
 * @param shift bit shift
 * @return int 0:complete  -EINVAL:error
 */
static int ak_clock_reset(u32 shift)
{
    u32 regval;

    if (shift < 0) {
        return -EINVAL;
    }

    regval = readl((void __iomem*)CLOCK_RESET_CTRL);
    regval |= (0x1 << shift);
    writel(regval, (void __iomem*)CLOCK_RESET_CTRL);
    mdelay(1);
    regval = readl((void __iomem*)CLOCK_RESET_CTRL);
    regval &= ~(0x1 << shift);
    writel(regval, (void __iomem*)CLOCK_RESET_CTRL);

    return 0;
}

static ulong ak_factor_clk_set_rate(struct clk* clk, ulong rate)
{
    ulong parent_rate = ak_get_pll2_freq();
    u32 factor = 0, new_rate = 0;

    /*
     * spi0 port clk = spi0_hclk/2;
     */
    if (clk->id == CLOCK_ID_FACTOR_SPI0_CLK) {
        rate *= 2;
    }

    /*
     * check range
     */
    if (parent_rate > rate) {
        factor = parent_rate / rate - 1;
    } else {
        factor = 0;
    }

    if (clk->id == CLOCK_ID_FACTOR_SPI0_CLK) {
        //确保计算的clk要比rate小
        while (1) {
            if (parent_rate / (2 * (factor + 1)) > rate / 2) {
                factor++;
            } else
                break;
        }
        // printf("wanted:%d, got:%d\r\n", rate/2, parent_rate/(2*(factor +
        // 1)));
    }

    /*
     *   No. |   Clock Name    | Divider Type    |   Calculation Formula
     *   ----|-----------------|-----------------|-----------------------
     *   1       gclk Special    Even Divider        1/(2(N+1))
     *   2       mac_opclk       Even Divider        1/(N+1)
     *   3       dphy_cfgclk     Even Divider        1/(N+1)
     *   4       spi0_hs_clk     Integer Divider     1/(N+1)
     *   5       sdadc_hsclk     Integer Divider     1/(N+1)
     *   6       sddac_hsclk     Integer Divider     1/(N+1)
     *   7       sdadc_clk       Integer Divider     1/(N+1)
     *   8       sddac_clk       Integer Divider     1/(N+1)
     *   9       sar_adc_clk     Integer Divider     1/(N+1)
     *   10      isp_vclk        MS Integer Divider  1/(N+1)
     *   11      npu_vclk        MS Integer Divider  1/(N+1)
     *   12      enc_vclk        MS Integer Divider  1/(N+1)
     *   13      cis1_sclk       MS Integer Divider  1/(N+1)
     *   14      cis2_sclk       MS Integer Divider  1/(N+1)
     */

    switch (clk->id) {
        case CLOCK_ID_FACTOR_GCLK:
            /* Ignore */
            break;
        case CLOCK_ID_FACTOR_SPI0_CLK:
        case CLOCK_ID_FACTOR_SPI0_HCLK:
            ak_adjust_clk_int_divider_freq(
                (void __iomem*)CLOCK_SAR_ADC_CTRL, SPI0_HSCLK_DIV_VLD_CFG_SHIFT,
                SPI0_HSCLK_EN_CFG_SHIFT, factor, SPI0_HSCLK_DIV_NUM_CFG_SHIFT,
                SPI0_HSCLK_DIV_NUM_CFG_MASK);
            break;
        case CLOCK_ID_FACTOR_MAC_REFCLK:
            ak_adjust_clk_even_divider_freq(
                (void __iomem*)CLOCK_MAC_OPCLK_CTRL,
                MAC_OPCLK_DIV_VLD_CFG_SHIFT, MAC_OPCLK_EN_CFG_SHIFT, factor,
                MAC_OPCLK_DIV_NUM_CFG_SHIFT, MAC_OPCLK_DIV_NUM_CFG_MASK);
            break;
        case CLOCK_ID_FACTOR_X_DPHYCFG:
            ak_adjust_clk_even_divider_freq((void __iomem*)CLOCK_SAR_ADC_CTRL,
                DPHY_CFGCLK_DIV_VLD_CFG_SHIFT, DPHY_CFGCLK_EN_CFG_SHIFT, factor,
                DPHY_CFGCLK_DIV_NUM_CFG_SHIFT, DPHY_CFGCLK_DIV_NUM_CFG_MASK);
            break;
    }

    /* reset */
    ak_clock_reset(ak_get_reset_ctrlbit(clk->id));

    if (clk->id == CLOCK_ID_FACTOR_GCLK
        || clk->id == CLOCK_ID_FACTOR_SPI0_CLK) {
        new_rate = parent_rate / (2 * (factor + 1));
    } else {
        new_rate = parent_rate / (factor + 1);
    }

    // pr_err("factor CLK set new_rate %d on FactorCLK_%ld\n", new_rate,
    // clk->id);

    return new_rate;
}
/*end of ak_factor_clk_set_rate*/

static ulong ak_factor_clk_get_rate(struct clk* clk)
{
    ulong parent_rate = ak_get_pll2_freq();
    u32 factor = 0, rate = 0;
    u32 regval;

    switch (clk->id) {
        case CLOCK_ID_FACTOR_GCLK:
            /*
             * GCLK frequency division coefficient configuration
             *      By default, The output frequency of GCLK is 60MHz.
             *      GCLK = PLL2_CLKOUT/2(gclk_div_num_cfg[1:0]+1)
             */
            regval = readl((void __iomem*)CLOCK_PLL2_CTRL);
            factor = (regval >> GCLK_DIV_NUM_CFG_SHIFT) & GCLK_DIV_NUM_CFG_MASK;
            rate = parent_rate / (2 * (factor + 1));
            break;
        case CLOCK_ID_FACTOR_SPI0_HCLK:
            /*
             * spi0_hsclk=pll2_clk/(spi0_hsclk_div_num_cfg+1)
             *      spi0_hsclk frequency is 12MHz by default.
             */
            regval = __raw_readl((void __iomem*)CLOCK_SAR_ADC_CTRL);
            factor = (regval >> SPI0_HSCLK_DIV_NUM_CFG_SHIFT)
                & SPI0_HSCLK_DIV_NUM_CFG_MASK;
            rate = parent_rate / (factor + 1);
            break;
        case CLOCK_ID_FACTOR_SPI0_CLK:
            /*
             * spi0 port clk = spi0_hsclk/2
             */
            regval = __raw_readl((void __iomem*)CLOCK_SAR_ADC_CTRL);
            factor = (regval >> SPI0_HSCLK_DIV_NUM_CFG_SHIFT)
                & SPI0_HSCLK_DIV_NUM_CFG_MASK;
            rate = parent_rate / (2 * (factor + 1));
            break;
        case CLOCK_ID_FACTOR_MAC_REFCLK:
            /*
             * MAC_OPCLK = pll2_clk/(mac_opclk_div_num_cfg+1)
             *       The OPCLK frequency is 12MHz by default.
             */
            regval = __raw_readl((void __iomem*)CLOCK_MAC_OPCLK_CTRL);
            factor = (regval >> MAC_OPCLK_DIV_NUM_CFG_SHIFT)
                & MAC_OPCLK_DIV_NUM_CFG_MASK;
            rate = parent_rate / (factor + 1);
            break;
        case CLOCK_ID_FACTOR_X_DPHYCFG:
            /*
             * dphy_cfgclk=pll2_clk/(dphy_cfgclk_div_num_cfg+1)
             * NOTE: dphy_cfgclk frequency is 120MHz by default and
             *      the acceptable frequency range is from 80 MHz to 120MHz
             */
            regval = __raw_readl((void __iomem*)CLOCK_SAR_ADC_CTRL);
            factor = (regval >> DPHY_CFGCLK_DIV_NUM_CFG_SHIFT)
                & DPHY_CFGCLK_DIV_NUM_CFG_MASK;
            rate = parent_rate / (factor + 1);
            break;
    }

    // pr_err("factor CLK get new_rate %d on FactorCLK_%ld\n", rate, clk->id);

    return rate;
}

static const struct clk_ops ak_factor_clk_ops = {
    .set_rate = ak_factor_clk_set_rate,
    .get_rate = ak_factor_clk_get_rate,
};

static int ak_factor_clk_probe(struct udevice* dev)
{
    struct clk clk;
    int ret;

    ret = clk_get_by_index(dev, 0, &clk);
    if (ret) {
        ak_log_print("FactorCLK fail to get parent\n");
        return -EINVAL;
    }

    // ak_log_print("FactorCLK Parent: %lu(MHz)\n", clk_get_rate(&clk)/MHz);

    return 0;
}

static const struct udevice_id ak_factor_clk_ids[]
    = { { .compatible = "anyka,factor-clk" }, {} };

U_BOOT_DRIVER(ak3918av100_factor_clk) = {
    .name = "ak3918av100_factor_clk",
    .id = UCLASS_CLK,
    .of_match = ak_factor_clk_ids,
    .probe = ak_factor_clk_probe,
    .ops = &ak_factor_clk_ops,
};

static int ak_gate_clk_enable(struct clk* clk)
{
    u32 regval;
    int ctrlbit = -1, rstbit = -1;

    // ak_log_print("%s: id %lu\n", __func__, clk->id);

    ctrlbit = ak_get_gate_ctrlbit(clk->id);
    if (ctrlbit < 0) {
        pr_err("%s: fail ctrlbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }

    /* enable */
    regval = readl((void __iomem*)CLOCK_GATE_CTRL);
    regval &= ~(1 << ctrlbit);
    writel(regval, (void __iomem*)CLOCK_GATE_CTRL);

    rstbit = ak_get_reset_ctrlbit(clk->id);
    if (rstbit < 0) {
        pr_err("%s: fail rstbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }

    /* reset */
    ak_clock_reset(rstbit);

    return 0;
}

static int ak_gate_clk_disable(struct clk* clk)
{
    u32 regval;
    int ctrlbit = -1;

    // ak_log_print("%s: id %lu\n", __func__, clk->id);

    ctrlbit = ak_get_gate_ctrlbit(clk->id);
    if (ctrlbit < 0) {
        pr_err("%s: fail ctrlbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }
    regval = readl((void __iomem*)CLOCK_GATE_CTRL);
    regval |= (0x1 << ctrlbit);
    writel(regval, (void __iomem*)CLOCK_GATE_CTRL);

    return 0;
}

static ulong ak_gate_clk_get_rate(struct clk* clk)
{
    u32 regval;
    int ctrlbit = -1;
    ulong rate;
    ulong pll2_clk = ak_get_pll2_freq();

    ctrlbit = ak_get_gate_ctrlbit(clk->id);
    if (ctrlbit < 0) {
        pr_err("%s: fail ctrlbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }

    /* has it enabled? */
    regval = readl((void __iomem*)CLOCK_GATE_CTRL);
    if ((regval >> ctrlbit) & 0x01) {
        return 0;
    }

    regval = readl((void __iomem*)CLOCK_PLL2_CTRL);
    regval = ((regval >> GCLK_DIV_NUM_CFG_SHIFT) & GCLK_DIV_NUM_CFG_MASK);

    /*
     * GCLK = PLL2_CLKOUT / 2 / (gclk_div_num_cfg[1:0] + 1)
     */
    rate = pll2_clk / 2 / (regval + 1);

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
        return -EINVAL;
    }

    // ak_log_print("GCLK Parent:%lu(MHz)\n", clk_get_rate(&clk)/MHz);

    return 0;
}

static const struct udevice_id ak_gate_clk_ids[]
    = { { .compatible = "anyka,gate-clk" }, {} };

U_BOOT_DRIVER(ak3918av100_gate_clk) = {
    .name = "ak3918av100_gate_clk",
    .id = UCLASS_CLK,
    .of_match = ak_gate_clk_ids,
    .probe = ak_gate_clk_probe,
    .ops = &ak_gate_clk_ops,
};

static ulong ak_fixed_clk_get_rate(struct clk* clk)
{
    return ak_get_pll2_freq();
}

static const struct clk_ops ak_fixed_clk_ops = {
    .get_rate = ak_fixed_clk_get_rate,
};

static int ak_fixed_clk_probe(struct udevice* dev)
{
    u32 nf, nr, od;
    u32 param_freq, param_nf, param_nr, param_od, param_nb, gclk_freq;
    u32 regval, temp, gclk_div;

    /* read node value */
    param_nf = fdtdec_get_int(
        gd->fdt_blob, dev_of_offset(dev), "clock-param-nf", 40);
    param_nr
        = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "clock-param-nr", 1);
    param_freq = fdtdec_get_int(
        gd->fdt_blob, dev_of_offset(dev), "clock-frequency", 240 * MHz);
    gclk_freq = fdtdec_get_int(
        gd->fdt_blob, dev_of_offset(dev), "gclk-frequency", 60 * MHz);
    param_nb = fdtdec_get_int(
        gd->fdt_blob, dev_of_offset(dev), "clock-param-nb", 16);

    /*
     * Check parameters:
     *      PLL2 parameters setting requirements
     *      500MHz ≤ 24*NF/NR ≤ 2.5GHz,
     *      pll_clk ≤ 800MHz, the maximum value depends on the MODE and VDD.
     */
    if (param_nf <= 0 || param_nr <= 0) {
        ak_log_print("%s: parameters error, the (nf nr) must >= 1\n", __func__);
        return -EINVAL;
    }
    temp = 24 * MHz * param_nf / param_nr;
    if (temp < 500 * MHz || temp > 2500 * MHz) {
        ak_log_print("%s: parameters error, nf=%u nr=%u freq=%u\n", __func__,
            param_nf, param_nr, param_freq);
        return -EINVAL;
    }
    if (param_freq > 800 * MHz) {
        ak_log_print("%s: parameters error, freq=%u\n", __func__, param_freq);
        return -EINVAL;
    }
    if (param_nb > 0x1000 || param_nb <= 0) {
        ak_log_print("%s: parameters error, nb=%u\n", __func__, param_nb);
        return -EINVAL;
    }
    ak_log_print("%s: parameters ok, nf=%u nr=%u nb=%u freq=%u\n", __func__,
        param_nf, param_nr, param_nb, param_freq);
    param_od = 24 * MHz * param_nf / param_nr / param_freq;
    /*
     * check
     */
    if (gclk_freq * 2 > param_freq) {
        ak_log_print(
            "%s: gclk parameters error, freq=%u\n", __func__, gclk_freq);
        return -EINVAL;
    }
    /*
     * GCLK = PLL2_CLKOUT/(2*(gclk_div_num_cfg+1))
     */
    gclk_div = (param_freq / (2 * gclk_freq)) - 1;

    /* read register */
    regval = __raw_readl((void __iomem*)CLOCK_PLL2_CTRL);
    od = ((regval >> PLL2_CLKOD_CFG_SHIFT) & PLL2_CLKOD_CFG_MASK) + 1;
    nf = ((regval >> PLL2_CLKF_CFG_SHIFT) & PLL2_CLKF_CFG_MASK) + 1;
    nr = ((regval >> PLL2_CLKR_CFG_SHIFT) & PLL2_CLKR_CFG_MASK) + 1;

    if ((param_nf == nf) && (param_nr == nr) && (param_od == od)) {
        ak_log_print("%s pll2 has inited!\n", __func__);
        goto exit;
    }

    /*
     * Adjusting PLL2
     * The steps of adjusting PLL2 are described as follows:
     *      (1) Read the value of core_freq_adj_cfg and wait until it becomes 0.
     *      (2) Set the frequency parameters of the PLL2, including
     * core_pll_m_cfg, core_pll_n_cfg and core_pll_od_cfg. (3) Set
     * core_freq_adj_cfg to 1.
     */

    /* Step 1 */
    ak_wait_pll_adjust_complete((void __iomem*)CLOCK_PLL1_CTRL,
        PLL2_FREQ_ADJ_CFG_SHIFT, PLL2_FREQ_ADJ_CFG_MASK);

    /* Step 2 */
    regval = ((0x1 << GCLK_DIV_VLD_CFG_SHIFT)
        | (gclk_div << GCLK_DIV_NUM_CFG_SHIFT) /* GCLK */
        | ((param_nf - 1) << PLL2_CLKF_CFG_SHIFT)
        | ((param_nr - 1) << PLL2_CLKR_CFG_SHIFT)
        | ((param_od - 1) << PLL2_CLKOD_CFG_SHIFT));
    writel(regval, (void __iomem*)CLOCK_PLL2_CTRL);

    /* bandwidth */
    regval = __raw_readl((void __iomem*)CLOCK_PLL1_PLL2_BANDWIDTH);
    regval &= ~0xFFF;
    regval |= param_nb - 1;
    writel(regval, (void __iomem*)CLOCK_PLL1_PLL2_BANDWIDTH);

    /* Step 3 */
    regval = readl((void __iomem*)CLOCK_PLL1_CTRL);
    regval |= ((0x1) << PLL2_FREQ_ADJ_CFG_SHIFT);
    writel(regval, (void __iomem*)CLOCK_PLL1_CTRL);

exit:
    ak_log_print("pll2 frequency out: %luMHz\n", ak_get_pll2_freq() / MHz);
    ak_log_print("gclk frequency out: %luMHz\n", ak_get_gclk_freq() / MHz);

    return 0;
}

static const struct udevice_id ak_fixed_clk_ids[]
    = { { .compatible = "anyka,fixed-clk" }, {} };

U_BOOT_DRIVER(ak3918av100_fixed_clk) = {
    .name = "ak3918av100_fixed_clk",
    .id = UCLASS_CLK,
    .of_match = ak_fixed_clk_ids,
    .probe = ak_fixed_clk_probe,
    .ops = &ak_fixed_clk_ops,
};
