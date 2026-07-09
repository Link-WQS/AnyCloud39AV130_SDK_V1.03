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

extern void ak3918av130_check_clock(void);

void spl_board_init(void)
{
    preloader_console_init();
    ak3918av130_check_clock();
}

u32 spl_boot_device(void)
{

#ifdef CONFIG_MTD_SPI_NAND
    return BOOT_DEVICE_NAND;
#else
    return BOOT_DEVICE_SPI;
#endif

}

