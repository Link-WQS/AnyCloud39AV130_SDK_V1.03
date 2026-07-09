// SPDX-License-Identifier: GPL-2.0+

#include <asm/io.h>
#include <clk-uclass.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <linux/clk-provider.h>

#include "clk-ak37e.h"

#define ak_log_print(fmt, arg...) printf(fmt, ##arg)

#define MHz (1000000UL)

#define CLOCK_FREQ_ADJ_WAITING_MAX_NUM (100000)


 /*
 * @BRIEF       ak_get_asic_pll_clk
 * @PARAM[in]   void
 * @RETURN      ulong
 * @RETVAL      asic_pll_clk * MHz :asic_pll_clk = 24 * M/(N*2^OD)
 */
static ulong ak_get_asic_pll_clk(void)
{
    u32 pll_m, pll_n, pll_od;
    ulong asic_pll_clk;
    u32 regval;

    regval = __raw_readl(CLK_ASIC_PLL_CTRL);
    pll_od
        = (regval >> CLOCK_ASIC_PLL_OD_CFG_SHIFT) & CLOCK_ASIC_PLL_OD_CFG_MASK;
    pll_n = (regval >> CLOCK_ASIC_PLL_N_CFG_SHIFT) & CLOCK_ASIC_PLL_N_CFG_MASK;
    pll_m = (regval >> CLOCK_ASIC_PLL_M_CFG_SHIFT) & CLOCK_ASIC_PLL_M_CFG_MASK;

    asic_pll_clk = 24 * pll_m / (pll_n * (1 << pll_od));

    return asic_pll_clk * MHz;
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
        case ASIC_GCLK_MMC0:
            ctrlbit = GCLK_MMC0_CFG_SHIFT;
            break;
        case ASIC_GCLK_MMC1:
            ctrlbit = GCLK_MMC1_CFG_SHIFT;
            break;
        case ASIC_GCLK_SD_ADC:
            ctrlbit = GCLK_SD_ADC_CFG_SHIFT;
            break;
        case ASIC_GCLK_SD_DAC:
            ctrlbit = GCLK_SD_DAC_CFG_SHIFT;
            break;
        case ASIC_GCLK_SPI0:
            ctrlbit = GCLK_SPI0_CFG_SHIFT;
            break;
        case ASIC_GCLK_SPI1:
            ctrlbit = GCLK_SPI1_CFG_SHIFT;
            break;
        case ASIC_GCLK_UART0:
            ctrlbit = GCLK_UART0_CFG_SHIFT;
            break;
        case ASIC_GCLK_UART1:
            ctrlbit = GCLK_UART1_CFG_SHIFT;
            break;
        case ASIC_GCLK_L2BUF0:
            ctrlbit = GCLK_L2BUF0_CFG_SHIFT;
            break;
        case ASIC_GCLK_TWI0:
            ctrlbit = GCLK_TWI0_CFG_SHIFT;
            break;
        case ASIC_GCLK_L2BUF1:
            ctrlbit = GCLK_L2BUF1_CFG_SHIFT;
            break;
        case ASIC_GCLK_GPIO:
            ctrlbit = GCLK_GPIO_CFG_SHIFT;
            break;
        case ASIC_GCLK_MAC0:
            ctrlbit = GCLK_MAC0_CFG_SHIFT;
            break;
        case ASIC_GCLK_PDM:
            ctrlbit = GCLK_PDM_CFG_SHIFT;
            break;
        case ASIC_GCLK_USB:
            ctrlbit = GCLK_USB_CONTROLLER_CFG_SHIFT;
            break;
        case ASIC_GCLK_MAC1:
            ctrlbit = GCLK_MAC1_CFG_SHIFT;
            break;
        case ASIC_GCLK_TWI1:
            ctrlbit = GCLK_TWI1_CFG_SHIFT;
            break;
        case ASIC_GCLK_UART2:
            ctrlbit = GCLK_UART2_CFG_SHIFT;
            break;
        case ASIC_GCLK_UART3:
            ctrlbit = GCLK_UART3_CFG_SHIFT;
            break;
        case ASIC_GCLK_SPI2:
            ctrlbit = GCLK_SPI2_CFG_SHIFT;
            break;
        case ASIC_GCLK_MMC2:
            ctrlbit = GCLK_MMC2_CFG_SHIFT;
            break;
        case ASIC_GCLK_TWI2:
            ctrlbit = GCLK_TWI2_CFG_SHIFT;
            break;
        case ASIC_GCLK_TWI3:
            ctrlbit = GCLK_TWI3_CFG_SHIFT;
            break;
        case ASIC_GCLK_IMAGE_CAPTURE:
            ctrlbit = GCLK_IMAGE_CAPTURE_CFG_SHIFT;
            break;
        case ASIC_GCLK_JPEG_CODEC:
            ctrlbit = GCLK_JPEG_CODEC_CFG_SHIFT;
            break;
        case ASIC_GCLK_VIDEO_DECODER:
            ctrlbit = GCLK_VIDEO_DECODER_CFG_SHIFT;
            break;
        case ASIC_GCLK_LCD_CONTROLLER:
            ctrlbit = GCLK_LCD_CONTROLLER_CFG_SHIFT;
            break;
        case ASIC_GCLK_GUI:
            ctrlbit = GCLK_GUI_CFG_SHIFT;
            break;
        case ASIC_GCLK_DSI:
            ctrlbit = GCLK_DSI_CONTROLLER_SHIFT;
            break;
    }

    return ctrlbit;
}
/*end of ak_gate_clk_get_ctrlbit*/

/*
 * @BRIEF       ak_gate_clk_get_rstbit
 * @PARAM[in]   id
 * @RETURN      int
 * @RETVAL      ctrlbit
 */
static int ak_gate_clk_get_rstbit(int id)
{
    int rstbit = -1;

    switch (id) {
        case ASIC_GCLK_MMC0:
            rstbit = GCLK_MMC0_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_MMC1:
            rstbit = GCLK_MMC1_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_SD_ADC:
            rstbit = GCLK_SDADC_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_SD_DAC:
            rstbit = GCLK_SDDAC_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_SPI0:
            rstbit = GCLK_SPI0_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_SPI1:
            rstbit = GCLK_SPI1_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_UART0:
            rstbit = GCLK_UART0_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_UART1:
            rstbit = GCLK_UART1_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_L2BUF0:
            rstbit = GCLK_L2BUF0_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_TWI0:
            rstbit = GCLK_TWI0_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_L2BUF1:
            rstbit = GCLK_L2BUF1_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_GPIO:
            rstbit = GCLK_GPIO_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_MAC0:
            rstbit = GCLK_MAC0_BUFFER_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_PDM:
            rstbit = GCLK_PDM_CONTROLLER_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_USB:
            rstbit = GCLK_USB_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_MAC1:
            rstbit = GCLK_MAC1_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_TWI1:
            rstbit = GCLK_TWI1_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_UART2:
            rstbit = GCLK_UART2_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_UART3:
            rstbit = GCLK_UART3_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_SPI2:
            rstbit = GCLK_SPI2_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_MMC2:
            rstbit = GCLK_MMC2_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_TWI2:
            rstbit = GCLK_TWI2_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_TWI3:
            rstbit = GCLK_TWI3_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_IMAGE_CAPTURE:
            rstbit = VCLK_IMAGE_CAPTURE_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_JPEG_CODEC:
            rstbit = VCLK_JPEG_CODEC_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_VIDEO_DECODER:
            rstbit = VCLK_VIDEO_DECODER_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_LCD_CONTROLLER:
            rstbit = VCLK_LCD_CONTROLLER_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_GUI:
            rstbit = VCLK_GUI_RST_CFG_SHIFT;
            break;
        case ASIC_GCLK_DSI:
            rstbit = CLOCK_MIPI_DIS_RST_CFG_SHIFT;
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
 * ak_adjust_clk_even_divider_freq:clock with even divider
 * like mac1_opclk:
 * 1. disable module driven by mac1_opclk
 * 2. enable the output clock when mac1_opclk_div_vld_cfg is 0 by settting
 * mac1_opclk_en_cfg to 1
 * 3. change the mac1_opclk_div_num_cfg and mac1_opclk_div_vld_cfg to be 1.
 *    wait mac1_opclk_div_vld_cfg to 0
 * 4. enable the module driven by mac1_opclk
 */
static int ak_adjust_clk_even_divider_freq(void __iomem* res_reg,
    u32 div_vld_cfg_shift, u32 en_cfg_shift, u32 div_num_cfg,
    u32 div_num_cfg_shift, u32 div_num_cfg_mask)
{
    u32 regval;

    regval = readl(res_reg);
    regval |= ((0x01) << en_cfg_shift);
    writel(regval, res_reg);

    regval = readl(res_reg);
    regval &= ~(div_num_cfg_mask << div_num_cfg_shift);
    regval |= ((div_num_cfg & div_num_cfg_mask) << div_num_cfg_shift);
    regval |= ((0x01) << div_vld_cfg_shift);
    writel(regval, res_reg);
    ak_clock_divider_adjust_complete(res_reg, div_vld_cfg_shift, 0x01);

    return 0;
}

/*
 * ak_adjust_clk_int_divider_freq
 * like sdadc_clk:
 * 1. disable the sd-adc module
 * 2. set sdadc_clk_en_cfg to 0 to disable output when sdadc_clk_div_vld_cfg is
 * 0
 * 3. change the sdadc_clk_div_num_cfg and set sdadc_clk_div_vld_cfg to 1.
 *    wait sdadc_clk_div_vld_cfg to 0
 * 4. set sdadc_clk_en_cfg to 1 t enable the output clock.
 * 5. enable the sd-adc module
 */
static int ak_adjust_clk_int_divider_freq(void __iomem* res_reg,
    u32 div_vld_cfg_shift, u32 en_cfg_shift, u32 div_num_cfg,
    u32 div_num_cfg_shift, u32 div_num_cfg_mask)
{
    u32 regval;

    regval = readl(res_reg);
    regval &= ~((0x1) << en_cfg_shift);
    writel(regval, res_reg);

    regval = readl(res_reg);
    regval &= ~(div_num_cfg_mask << div_num_cfg_shift);
    regval |= ((div_num_cfg & div_num_cfg_mask) << div_num_cfg_shift);
    writel(regval, res_reg);

    regval = readl(res_reg);
    regval |= ((0x01) << div_vld_cfg_shift);
    writel(regval, res_reg);
    ak_clock_divider_adjust_complete(res_reg, div_vld_cfg_shift, 0x01);

    regval = readl(res_reg);
    regval |= ((0x1) << en_cfg_shift);
    writel(regval, res_reg);

    return 0;
}

/*
 * @BRIEF       ak_module_reset_by_clock
 * @PARAM[in]   res_reg
 * @PARAM[in]   rst_cfg_shift
 */
static void ak_module_reset_by_clock(void __iomem* res_reg, u32 rst_cfg_shift)
{
    u32 regval;

    regval = readl(res_reg);
    regval |= (0x1 << rst_cfg_shift);
    writel(regval, res_reg);

    mdelay(1);

    regval = readl(res_reg);
    regval &= ~(0x1 << rst_cfg_shift);
    writel(regval, res_reg);
}


/*
 * @BRIEF       ak factor clk check rate
 * @PARAM[in]      clk
 * @PARAM[in]      rate
 * @RETURN      new_rate
 * @RETVAL      new_rate
 */
static ulong ak_factor_clk_check_rate(struct clk* clk, ulong rate)
{
    char *clk_name = 0;
    ulong   max_rate = 0;

    switch (clk->id) {
        case ASIC_FACTOR_CSI0_SCLK:
            if (rate > CSI_SCLK0_MAX_RATE) {
                max_rate = CSI_SCLK0_MAX_RATE;
                clk_name = "csi_sclk0";
            }
        case ASIC_FACTOR_CSI1_SCLK:
            if (rate > CSI_SCLK1_MAX_RATE) {
                max_rate = CSI_SCLK1_MAX_RATE;
                clk_name = "csi_sclk1";
            }
            break;
        case ASIC_FACTOR_MAC0_OPCLK:
            if (rate > OPCLK_MAX_RATE) {
                max_rate = OPCLK_MAX_RATE;
                clk_name = "opclk";
            }
            break;
        case ASIC_FACTOR_MAC1_OPCLK:
            if (rate > OPCLK1_MAX_RATE) {
                max_rate = OPCLK1_MAX_RATE;
                clk_name = "opclk1";
            }
            break;
        case ASIC_FACTOR_LCD_PCLK:
            if (rate > DSI_PCLK_MAX_RATE) {
                max_rate = DSI_PCLK_MAX_RATE;
                clk_name = "lcd pclk";
            }
            break;
        case ASIC_FACTOR_SPI0_CLK:
            if (rate > SPI0_BUS_CLK_MAX_RATE) {
                max_rate = SPI0_BUS_CLK_MAX_RATE;
                clk_name = "spi0 clk";
            }
            break;
    }

    if (clk_name) {
        ak_log_print("Can't set %s rate %u, Max rate: %u!\n", clk_name, rate, max_rate);
        rate = max_rate;
    }
    return rate;
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
    ulong parent_rate = ak_get_asic_pll_clk();
    u32 factor = 0, new_rate = 0;

    rate = ak_factor_clk_check_rate(clk, rate);

    if (parent_rate > rate) {
        factor = parent_rate / rate - 1;
    } else {
        factor = 0;
    }

    switch (clk->id) {
        case ASIC_FACTOR_CSI0_SCLK:
        case ASIC_FACTOR_CSI1_SCLK:
            /*
             * Ignore the CIS*_SCLK
             */
            break;
        case ASIC_FACTOR_MAC0_OPCLK:
            ak_adjust_clk_even_divider_freq(
                (void __iomem*)CLOCK_MAC_OPCLK_CTRL,
                CLOCK_MAC0_OPCLK_DIV_VLD_CFG_SHIFT,
                CLOCK_MAC0_OPCLK_EN_CFG_SHIFT, factor,
                CLOCK_MAC0_DIV_NUM_CFG_SHIFT, CLOCK_MAC0_DIV_NUM_CFG_MASK);
            break;
        case ASIC_FACTOR_MAC1_OPCLK:
            ak_adjust_clk_even_divider_freq(
                (void __iomem*)CLOCK_MAC_OPCLK_CTRL,
                CLOCK_MAC1_OPCLK_DIV_VLD_CFG_SHIFT,
                CLOCK_MAC1_OPCLK_EN_CFG_SHIFT, factor,
                CLOCK_MAC1_DIV_NUM_CFG_SHIFT, CLOCK_MAC1_DIV_NUM_CFG_MASK);
            break;
        case ASIC_FACTOR_LCD_PCLK:
            ak_adjust_clk_even_divider_freq(
                (void __iomem*)CLOCK_LCD_PCLK_CTRL,
                CLOCK_LCD_PCLK_DIV_VLD_CFG_SHIFT, CLOCK_LCD_PCLK_EN_CFG_SHIFT,
                factor, CLOCK_LCD_PCLK_DIV_VLD_CFG_SHIFT,
                CLOCK_LCD_PCLK_DIV_NUM_CFG_MASK);
            ak_module_reset_by_clock(
                (void __iomem*)CLOCK_SPI_HS_CTRL, CLOCK_LCD_PCLK_RST_CFG_SHIFT);
            break;
        case ASIC_FACTOR_SPI0_CLK:
            factor = parent_rate / (rate * 2) - 1;
            ak_adjust_clk_int_divider_freq(
                (void __iomem*)CLOCK_SPI_HS_CTRL, CLOCK_SPI_HSDIV_VLD_CFG_SHIFT,
                CLOCK_SPI_HSCLK_EN_CFG_SHIFT, factor,
                CLOCK_SPI_HSCLK_DIV_NUM_CFG_SHIFT,
                CLOCK_SPI_HSCLK_DIV_NUM_CFG_MASK);
            ak_module_reset_by_clock((void __iomem*)CLOCK_SPI_HS_CTRL,
                CLOCK_SPI0_HSCLK_RST_CFG_SHIFT);
            break;
    }

    new_rate = parent_rate / (factor + 1);
    if (clk->id == ASIC_FACTOR_SPI0_CLK) {
        new_rate = parent_rate / (2 * (factor + 1));
    }

    // pr_err("factor CLK set new_rate %d on FactorCLK_%ld\n", new_rate,
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
    ulong parent_rate = ak_get_asic_pll_clk();
    u32 factor = 0, rate = 0;
    u32 regval;

    switch (clk->id) {
        case ASIC_FACTOR_CSI0_SCLK:
        case ASIC_FACTOR_CSI1_SCLK:
            /*
             * Ignore the CIS*_SCLK
             */
            rate = 0;
            break;
        case ASIC_FACTOR_MAC0_OPCLK:
            regval = readl((void __iomem*)CLOCK_MAC_OPCLK_CTRL);
            factor = (regval >> CLOCK_MAC0_DIV_NUM_CFG_SHIFT)
                & CLOCK_MAC0_DIV_NUM_CFG_MASK;
            rate = parent_rate / (factor + 1);
            break;
        case ASIC_FACTOR_MAC1_OPCLK:
            regval = __raw_readl((void __iomem*)CLOCK_MAC_OPCLK_CTRL);
            factor = (regval >> CLOCK_MAC1_DIV_NUM_CFG_SHIFT)
                & CLOCK_MAC1_DIV_NUM_CFG_MASK;
            rate = parent_rate / (factor + 1);
            break;
        case ASIC_FACTOR_LCD_PCLK:
            regval = __raw_readl((void __iomem*)CLOCK_LCD_PCLK_CTRL);
            factor = (regval >> CLOCK_LCD_PCLK_DIV_NUM_CFG_SHIFT)
                & CLOCK_LCD_PCLK_DIV_NUM_CFG_MASK;
            rate = parent_rate / (factor + 1);
            break;
        case ASIC_FACTOR_SPI0_CLK:
            regval = __raw_readl((void __iomem*)CLOCK_SPI_HS_CTRL);
            factor = (regval >> CLOCK_SPI_HSCLK_DIV_NUM_CFG_SHIFT)
                & CLOCK_SPI_HSCLK_DIV_NUM_CFG_MASK;
            rate = parent_rate / (2 * (factor + 1));
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

U_BOOT_DRIVER(ak37e_factor_clk) = {
    .name = "ak37e_factor_clk",
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
    void __iomem* reg;
    int ctrlbit = -1, rstbit = -1;

    // ak_log_print("%s: id %lu\n", __func__, clk->id);

    ctrlbit = ak_gate_clk_get_ctrlbit(clk->id);
    if (ctrlbit < 0) {
        pr_err("%s: fail ctrlbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }
    reg = (void*)CLOCK_CTRL_REG;
    if ((clk->id >= ASIC_GCLK_UART2) && (clk->id < ASIC_GCLK_TWI3)) {
        reg = (void*)CLOCK_CTRL2_REG;
    }
    regval = readl(reg);
    regval &= ~(1 << ctrlbit);
    writel(regval, reg);

    rstbit = ak_gate_clk_get_rstbit(clk->id);
    if (rstbit < 0) {
        pr_err("%s: fail rstbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }
    reg = (void*)RESET_CTRL_REG;
    if ((clk->id >= ASIC_GCLK_UART2) && (clk->id < ASIC_GCLK_TWI3)) {
        reg = (void*)CLOCK_CTRL2_REG;
    }
    regval = readl(reg);
    regval |= (0x1 << rstbit);
    writel(regval, reg);
    mdelay(1);
    regval = readl(reg);
    regval &= ~(1 << rstbit);
    writel(regval, reg);

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
    void __iomem* reg;
    int ctrlbit = -1;

    // ak_log_print("%s: id %lu\n", __func__, clk->id);

    ctrlbit = ak_gate_clk_get_ctrlbit(clk->id);
    if (ctrlbit < 0) {
        pr_err("%s: fail ctrlbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }
    reg = (void*)CLOCK_CTRL_REG;
    if ((clk->id >= ASIC_GCLK_UART2) && (clk->id < ASIC_GCLK_TWI3)) {
        reg = (void*)CLOCK_CTRL2_REG;
    }
    regval = readl(reg);
    regval |= (0x1 << ctrlbit);
    writel(regval, reg);

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
    u32 regval;
    void __iomem* reg;
    int ctrlbit = -1;
    ulong rate;
    ulong asic_pll_clk = ak_get_asic_pll_clk();

    ctrlbit = ak_gate_clk_get_ctrlbit(clk->id);
    if (ctrlbit < 0) {
        pr_err("%s: fail ctrlbit on %lu\n", __func__, clk->id);
        return -EINVAL;
    }

    reg = (void*)CLOCK_CTRL_REG;
    if ((clk->id >= ASIC_GCLK_UART2) && (clk->id < ASIC_GCLK_TWI3)) {
        reg = (void*)CLOCK_CTRL2_REG;
    }
    regval = readl(reg);
    if ((regval >> ctrlbit) & 0x01) {
        return 0;
    }

    regval = readl(CLK_ASIC_PLL_CTRL);
    regval = ((regval >> CLOCK_ASIC_VCLK_DIV_NUM_CFG_SHIFT)
        & CLOCK_ASIC_VCLK_DIV_NUM_CFG_MASK);
    rate = asic_pll_clk / (2 * (regval + 1));

    if ((clk->id >= ASIC_GCLK_MMC0) && (clk->id <= ASIC_GCLK_TWI3)) {
        regval = __raw_readl(CLK_ASIC_PLL_CTRL);
        regval = ((regval >> CLOCK_ASIC_GCLK_SEL_SHIFT)
            & CLOCK_ASIC_GCLK_SEL_MASK);
        if (regval) {
            rate = rate / 2;
        }
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

U_BOOT_DRIVER(ak37e_gate_clk) = {
    .name = "ak37e_gate_clk",
    .id = UCLASS_CLK,
    .of_match = ak_gate_clk_ids,
    .probe = ak_gate_clk_probe,
    .ops = &ak_gate_clk_ops,
};

static ulong ak_fixed_clk_get_rate(struct clk* clk)
{
    return ak_get_asic_pll_clk();
}

static const struct clk_ops ak_fixed_clk_ops = {
    .get_rate = ak_fixed_clk_get_rate,
};

static int ak_fixed_clk_probe(struct udevice* dev)
{
    u32 div_m, div_n, div_od, asic_pll;
    u32 rd_div_m, rd_div_n, rd_div_od;
    u32 regval;

    div_n = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "clock-div-n", 0);
    div_od
        = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "clock-div-od", 0);
    asic_pll = fdtdec_get_int(
        gd->fdt_blob, dev_of_offset(dev), "clock-frequency", 0);

    if (asic_pll > ASIC_PLL_MAX_RATE) {
        ak_log_print("Can't set asic PLL rate %u, Max rate: %u!\n", asic_pll, ASIC_PLL_MAX_RATE);
        asic_pll = ASIC_PLL_MAX_RATE;
    }

    if ((div_n == 0) || (div_od == 0) || (asic_pll == 0)) {
        ak_log_print(
            "%s: n %d od %d pll %d\n", __func__, div_n, div_od, asic_pll);
        return -EINVAL;
    }
    if ((div_od < 1) || (div_n < 1) || (div_n > 24)) {
        ak_log_print("%s: n %d od %d pll %d param ERR\n", __func__, div_n,
            div_od, asic_pll);
        return -EINVAL;
    }
    // ak_log_print("%s: n %d od %d pll %d\n", __func__, div_n, div_od,
    // asic_pll);

    regval = readl((void __iomem*)CLOCK_ASIC_PLL_CTRL);
    rd_div_m
        = (regval >> CLOCK_ASIC_PLL_M_CFG_SHIFT) & CLOCK_ASIC_PLL_M_CFG_MASK;
    rd_div_n
        = (regval >> CLOCK_ASIC_PLL_N_CFG_SHIFT) & CLOCK_ASIC_PLL_N_CFG_MASK;
    rd_div_od
        = (regval >> CLOCK_ASIC_PLL_OD_CFG_SHIFT) & CLOCK_ASIC_PLL_OD_CFG_MASK;

    div_m = ((asic_pll / MHz) * (div_n * (1 << div_od))) / 24;
    if ((div_m == rd_div_m) && (rd_div_n == div_n) && (rd_div_od == div_od)) {
        // ak_log_print("ASIC: has inited\n");
        goto exit;
    }
    if (div_m < 2) {
        ak_log_print("ASIC: frequency parameter Error(%d)\n", div_m);
        return -EINVAL;
    }

    /*
     * 1. First wait the asic_pll_freq_adj_cfg to 0
     */
    ak_clock_divider_adjust_complete((void __iomem*)CLOCK_CPU_PLL_CTRL,
        ASIC_PLL_FREQ_ADJ_CFG_SHIFT, ASIC_PLL_FREQ_ADJ_CFG_MASK);

    /*
     * 2. Update the configuration for asic_pll
     */
    regval = ((0x1 << CLOCK_ASIC_GCLK_SEL_SHIFT)
        | (0x1 << CLOCK_ASIC_VCLK_DIV_VLD_CFG_SHIFT)
        | ((0x0 & CLOCK_ASIC_VCLK_DIV_NUM_CFG_MASK)
            << CLOCK_ASIC_VCLK_DIV_NUM_CFG_SHIFT)
        | (div_od << CLOCK_ASIC_PLL_OD_CFG_SHIFT)
        | (div_n << CLOCK_ASIC_PLL_N_CFG_SHIFT)
        | (div_m << CLOCK_ASIC_PLL_M_CFG_SHIFT));
    writel(regval, ((void __iomem*)CLOCK_ASIC_PLL_CTRL));

    /*
     * 3. SET asic_pll_freq_adj_cfg to 1 to make valid
     */
    regval = readl((void __iomem*)CLOCK_CPU_PLL_CTRL);
    regval |= ((0x1) << ASIC_PLL_FREQ_ADJ_CFG_SHIFT);
    __raw_writel(regval, (void __iomem*)CLOCK_CPU_PLL_CTRL);

    /*
     * 4. wait the asic_pll_freq_adj_cfg to 0
     */
    ak_clock_divider_adjust_complete((void __iomem*)CLOCK_CPU_PLL_CTRL,
        ASIC_PLL_FREQ_ADJ_CFG_SHIFT, ASIC_PLL_FREQ_ADJ_CFG_MASK);

exit:
    ak_log_print("ASIC: %lu(Mhz)\n", ak_get_asic_pll_clk() / MHz);

    return 0;
}

static const struct udevice_id ak_fixed_clk_ids[]
    = { { .compatible = "anyka,fixed-clk" }, {} };

U_BOOT_DRIVER(ak37e_fixed_clk) = {
    .name = "ak37e_fixed_clk",
    .id = UCLASS_CLK,
    .of_match = ak_fixed_clk_ids,
    .probe = ak_fixed_clk_probe,
    .ops = &ak_fixed_clk_ops,
};
