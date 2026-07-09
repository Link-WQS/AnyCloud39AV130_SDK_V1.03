// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;
int dram_init(void)
{
    /* dram_init must store complete ramsize in gd->ram_size */
    gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
                PHYS_SDRAM_1_SIZE);
    return 0;
}


int board_early_init_f(void)
{

    return 0;
}

int board_init(void)
{
    /* address of boot parameters */
    gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

    return 0;
}

int board_late_init(void)
{
    return 0;
}


int checkboard(void)
{
    //puts("Board: MX25PDK\n");

    return 0;
}

/* Lowlevel init isn't used on mx25pdk, so just provide a dummy one here */
//void lowlevel_init(void) {}
