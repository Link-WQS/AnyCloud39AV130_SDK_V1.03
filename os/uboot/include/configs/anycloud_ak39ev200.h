/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011 Freescale Semiconductor, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * input clock of PLL
 */
/* ak39 has 12.0000MHz input clock */
#define CONFIG_SYS_CLK_FREQ 12000000


/*#define CONFIG_SYS_TIMER_RATE     32768 */

#define PART_BIOS_NAME      "KERNEL"
#define PART_ENV_NAME       "ENV"
#define PART_DTB_NAME       "DTB"
#define PART_LOGO_NAME      "LOGO"
#define PART_UBOOT_NAME     "UBOOT"
#define PART_APP_NAME       "APP"


/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_BANKS  1
#define CONFIG_SYS_FLASH_BASE       0x0
#define CONFIG_SYS_MAX_FLASH_SECT   8192


#define CONFIG_ENV_SECT_SIZE        0x1000

//#define CONFIG_SYS_NAND_U_BOOT_OFFS     0x200000


//#define CONFIG_SYS_NAND_SELF_INIT


#define CONFIG_SYS_SPL_MALLOC_START (CONFIG_SYS_TEXT_BASE - \
                        CONFIG_ENV_SIZE + 4096*1024)
#define CONFIG_SYS_SPL_MALLOC_SIZE  (CONFIG_ENV_SIZE + 4096*1024)

//#define CONFIG_SPL_NAND_INIT

#ifdef  CONFIG_MTD_SPI_NAND
#define  MTDIDS_DEFAULT  "nand0=spi0.0"
#else
#define  MTDIDS_DEFAULT  "nor0=spi0.0"
#endif

#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE  1
#define CONFIG_SYS_NAND_BASE        0x80eff800

#endif


/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN       (CONFIG_ENV_SIZE + 2 * 1024 * 1024)

/* Physical Memory Map */

#define PHYS_SDRAM_1        0x80000000
#define PHYS_SDRAM_1_SIZE   (32 * 1024 * 1024)
#define CONFIG_SYS_SDRAM_BASE       PHYS_SDRAM_1
#define PHYS_SYS_SDRAM_PROTECT_SIZE       0x01000000 /* 16MB of DRAM */

#define CONFIG_SYS_INIT_SP_OFFSET \
    (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
/*#define CONFIG_SYS_INIT_SP_ADDR \
    (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)
*/

/*
 * Stack should be on the SRAM because
 * DRAM is not init ,(arch/arm/lib/crt0.S)
 */
#define CONFIG_SYS_INIT_SP_ADDR     (0x80ff0000 - GENERATED_GBL_DATA_SIZE)



#define CONFIG_LOADADDR     0x81000000  /* loadaddr env var */
#define CONFIG_SYS_LOAD_ADDR    CONFIG_LOADADDR
/*"mtdparts=mtdparts=spi0.0:464K(UBOOT),4K(ENV),4K(ENVBK),64K(DTB),2M(KERNEL),2M(ROOTFS),300K(CONFIG),-(APP)\0" \*/
/*"mtdparts=mtdparts=spi0.0:400K@0x0(UBOOT),4K@0x64000(ENV),64K@0x65000(DTB),2048K@0x75000(KERNEL)\0" \*/

#if defined(CONFIG_MTD_SPI_NAND)
#define CONFIG_EXTRA_ENV_SETTINGS \
    "a_uboot_flags=0x1\0" \
    "sf_hz="    __stringify(CONFIG_SF_DEFAULT_SPEED)    "\0"\
    "kernel_addr=0x0\0" \
    "kernel_size=0x0\0" \
    "dtb_offset=0x0\0" \
    "dtb_size=0x0\0" \
    "dtb_addr=0x0\0" \
    "fdt_high=0xFFFFFFFF\0" \
    "update_flag=0x0\0" \
    "loadaddr=0x80008000\0" \
    "filesize=0x0\0" \
    "image_name=uImage\0" \
    "update_offset=0x0\0" \
    "update_size=0x0\0" \
    "ethaddr=00:55:7b:b5:7d:f7\0" \
    "eth1addr=00:55:7b:b5:7d:f7\0" \
    "netretry=once\0" \
    "ipaddr=192.168.1.99\0" \
    "serverip=192.168.1.1\0" \
    "netmask=255.255.255.0\0" \
    "baudrate=115200\0" \
    "console=ttySAK0,115200n8\0" \
    "mtd_root=/dev/mtdblock5\0" \
    "rootfstype=yaffs2\0" \
    "init=/sbin/init\0" \
    "mem=mem=64M\0" \
    "memsize=memsize=64M\0" \
    "mtdparts=mtdparts=nand0.0:128K(SPL),128K(ENV),512K(UBOOT),128K(DTB),2M(KERNEL),20M(ROOTFS),40M(APP),-(CONFIG)\0" \
    "bootargs=console=${console} root=${mtd_root} rootfstype=${rootfstype} init=${init} \0" \
    "setcmd=setenv bootargs console=${console} root=${mtd_root} rootfstype=${rootfstype} init=${init} mem=${memsize}\0" \
    "read_kernel=nand read ${loadaddr} ${kernel_offset} ${kernel_size}\0" \
    "read_dtb=nand read ${fdtcontroladdr} ${dtb_offset} ${dtb_size};fdt addr ${fdtcontroladdr}\0" \
    "boot_normal=env set bootargs console=${console} root=${mtd_root} rootfstype=${rootfstype} init=${init} ${mtdparts}${mtdparts1} ${mem} ${memsize};run read_dtb; run read_kernel; bootm ${loadaddr} - ${fdtcontroladdr}\0" \
    "bootcmd=run boot_normal\0" \
    "logo_switch=0x0\0" \
    "bootargs_mode=0x0\0" \
    "sd_det_mode=0x0\0" \
    "root_name=ROOTFS\0" \
    "kernel_name=KERNEL\0" \
    "env_name=ENV\0" \
    "dtb_name=DTB\0" \
    "uboot_name=UBOOT\0" \
    "parttab_type=AK\0" \
    "setgpio=0\0" \
    "envup_flags=1\0" \
    "lcdloadaddr=0x83d02000\0" \
    "env_version=1.1.00\0" \
    "env_platform=anycloud_ak37e_nand_60E\0" \

#else
#define CONFIG_EXTRA_ENV_SETTINGS \
    "a_uboot_flags=0x1\0" \
    "sf_hz="    __stringify(CONFIG_SF_DEFAULT_SPEED)    "\0"\
    "kernel_addr=0x0\0" \
    "kernel_size=0x0\0" \
    "dtb_offset=0x0\0" \
    "dtb_size=0x0\0" \
    "dtb_addr=0x0\0" \
    "fdt_high=0xFFFFFFFF\0" \
    "update_flag=0x0\0" \
    "loadaddr=0x80008000\0" \
    "filesize=0x0\0" \
    "image_name=uImage\0" \
    "update_offset=0x0\0" \
    "update_size=0x0\0" \
    "update=tftp $(loadaddr) $(image_name); sf probe ; sf update $(loadaddr) $(update_offset) $(filesize) \0" \
    "ethaddr=00:55:7b:b5:7d:f7\0" \
    "eth1addr=00:55:7b:b5:7d:f7\0" \
    "netretry=once\0" \
    "ipaddr=192.168.1.99\0" \
    "serverip=192.168.1.1\0" \
    "netmask=255.255.255.0\0" \
    "baudrate=115200\0" \
    "console=ttySAK0,115200n8\0" \
    "mtd_root=/dev/mtdblock6\0" \
    "rootfstype=squashfs\0" \
    "init=/sbin/init\0" \
    "mem=mem=64M\0" \
    "memsize=memsize=64M\0" \
    "mtdparts=mtdparts=spi0.0:464K(UBOOT),4K(ENV),64K(DTB),2M(KERNEL),2M(ROOTFS),300K(CONFIG),-(APP)\0" \
    "bootargs=console=${console} root=${mtd_root} rootfstype=${rootfstype} init=${init} \0" \
    "setcmd=setenv bootargs console=${console} root=${mtd_root} rootfstype=${rootfstype} init=${init} mem=${memsize}\0" \
    "read_kernel=sf probe 0:0 ${sf_hz} 0; sf read ${loadaddr} ${kernel_offset} ${kernel_size}\0" \
    "read_dtb=sf probe 0:0 ${sf_hz} 0; sf read ${fdtcontroladdr} ${dtb_offset} ${dtb_size};fdt addr ${fdtcontroladdr}\0" \
    "boot_normal=env set bootargs console=${console} root=${mtd_root} rootfstype=${rootfstype} init=${init} ${mtdparts}${mtdparts1} ${mem} ${memsize};run read_dtb; run read_kernel; bootm ${loadaddr} - ${fdtcontroladdr}\0" \
    "bootcmd=run boot_normal\0" \
    "logo_switch=0x0\0" \
    "bootargs_mode=0x0\0" \
    "sd_det_mode=0x0\0" \
    "root_name=ROOTFS\0" \
    "kernel_name=KERNEL\0" \
    "env_name=ENV\0" \
    "dtb_name=DTB\0" \
    "uboot_name=UBOOT\0" \
    "parttab_type=AK\0" \
    "setgpio=0\0" \
    "envup_flags=1\0" \
    "lcdloadaddr=0x83d02000\0" \
    "env_version=1.1.00\0" \
    "env_platform=anycloud_ak37e_nor_60E\0" \

#endif
#if 0
#define CONFIG_BOOTCOMMAND \
       "mmc dev ${mmcdev}; if mmc rescan; then " \
           "if run loadbootscript; then " \
               "run bootscript; " \
           "else " \
               "if run loadimage; then " \
                   "run mmcboot; " \
               "else run netboot; " \
               "fi; " \
           "fi; " \
       "else run netboot; fi"

/* Miscellaneous configurable options */
#endif

#endif /* __CONFIG_H */
