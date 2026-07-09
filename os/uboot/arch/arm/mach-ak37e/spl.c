// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Gateworks Corporation
 * Copyright (C) 2011-2012 Freescale Semiconductor, Inc.
 *
 * Author: Tim Harvey <tharvey@gateworks.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/spl.h>
#include <spl.h>
#include <g_dnl.h>

#include <spi.h>
#include <spi_flash.h>


DECLARE_GLOBAL_DATA_PTR;

/**
 * ak_spl_check_clock - check chips clock
 */
static void ak_spl_check_clock(void)
{
#define CPU_PLL_OD_CFG_MASK             (0x3)
#define CPU_PLL_OD_CFG_SHIFT            (12)
#define CPU_PLL_N_CFG_MASK              (0xF)
#define CPU_PLL_N_CFG_SHIFT             (8)
#define CPU_PLL_M_CFG_MASK              (0xFF)
#define CPU_PLL_M_CFG_SHIFT             (0)

#define PLL1_MAX_RATE                   (800000000)
#define JCLK_MAX_RATE                   (800000000)
#define HCLK_MAX_RATE                   (440000000)
#define DPHY_CLK_MAX_RATE               (440000000)

    u32 regval, dphyclk, hclk, dclk, cpu_pll_clk;
    u32 pll_m, pll_n, pll_od;

    regval = __raw_readl(0x08000004);
    pll_od = (regval >> CPU_PLL_OD_CFG_SHIFT) & CPU_PLL_OD_CFG_MASK;
    pll_n = (regval >> CPU_PLL_N_CFG_SHIFT) & CPU_PLL_N_CFG_MASK;
    pll_m = (regval >> CPU_PLL_M_CFG_SHIFT) & CPU_PLL_M_CFG_MASK;

    cpu_pll_clk = 24 * pll_m /(pll_n * (1 << pll_od));

    cpu_pll_clk = cpu_pll_clk * 1000000;
    if (cpu_pll_clk > PLL1_MAX_RATE) {
        printf("Can't set PLL1 rate %d, Max rate: %d! halt!!!\n", cpu_pll_clk, PLL1_MAX_RATE);
        while (1);
    }

    if (regval & (1UL << 30)) {
        //ddr2 work 2x mode
        //dphyclk=cpu pll, hclk=dclk=1/2 cpu pll
        dphyclk = cpu_pll_clk;
        hclk = cpu_pll_clk / 2;
        dclk = hclk;
    } else {
        //ddr2 work 1x mode
        //dphy clock mode define by cpu_5x_sel
        if (regval & (1UL << 26)) {
            //cpu work 5x mode
            //jclk = cpu pll, hclk=dclk=dphyclk=1/5 cpu pll
            dphyclk = cpu_pll_clk / 5;
            hclk = dphyclk;
            dclk = hclk;
        } else {
            //cpu work 2x mode
            //jclk = cpu pll, hclk=dclk=dphyclk=cpu pll even div clk
            u32 pll_even_div = (regval >> 15) & 0x03;
            u32 pll_even_clk = cpu_pll_clk / (2 * (pll_even_div + 1));
            dphyclk = pll_even_clk;
            hclk = pll_even_clk;
            dclk = hclk;
        }
    }

    if (dphyclk > DPHY_CLK_MAX_RATE) {
        printf("Can't set dphy clk rate %u, Max rate: %d! halt!!!\n", dphyclk, DPHY_CLK_MAX_RATE);
        while (1);
    }
    if (hclk > HCLK_MAX_RATE) {
        printf("Can't set hclk rate %u, Max rate: %d! halt!!!\n", hclk, HCLK_MAX_RATE);
        while (1);
    }
    if (dclk > HCLK_MAX_RATE) {
        printf("Can't set dclk rate %u, Max rate: %d! halt!!!\n", dclk, HCLK_MAX_RATE);
        while (1);
    }

}

void spl_board_init(void)
{
    preloader_console_init();
    ak_spl_check_clock();
}

u32 spl_boot_device(void)
{

#ifdef CONFIG_MTD_SPI_NAND
    return BOOT_DEVICE_NAND;
#else
    return BOOT_DEVICE_SPI;
#endif

}

