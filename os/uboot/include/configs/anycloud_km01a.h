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


/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_BANKS  1
#define CONFIG_SYS_FLASH_BASE       0x0
#define CONFIG_SYS_MAX_FLASH_SECT   8192 


#define CONFIG_ENV_SECT_SIZE        0x1000

#if 0
#if defined(CONFIG_SPL)
#if defined(CONFIG_MTD_SPI_NAND)
#define CONFIG_ENV_OFFSET           (0x20000)/*128K*/
#define CONFIG_ENV_SIZE             (4*1024)
#else
#define CONFIG_ENV_OFFSET           (0x10000)/*64K*/
#define CONFIG_ENV_SIZE             (4*1024)
#endif
#else
#if defined(CONFIG_MTD_SPI_NAND)
#define CONFIG_ENV_OFFSET           (0x40000)/*256K*/
#define CONFIG_ENV_SIZE             (4*1024)
#else
#define CONFIG_ENV_OFFSET           (0x37000)/*220K*/
#define CONFIG_ENV_SIZE             (4*1024)
#endif
#endif
#endif

#define CONFIG_SYS_NAND_U_BOOT_OFFS     0x200000


#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_NAND_BASE 0x80ff0000
#define CONFIG_SYS_MONITOR_LEN 0x100000


#define CONFIG_SYS_SPL_MALLOC_START (CONFIG_SYS_TEXT_BASE - \
                        CONFIG_ENV_SIZE + 4096*1024)
#define CONFIG_SYS_SPL_MALLOC_SIZE  (CONFIG_ENV_SIZE + 4096*1024)

#define CONFIG_SPL_NAND_INIT

#ifdef  CONFIG_MTD_SPI_NAND
#define  MTDIDS_DEFAULT  "nand0=spi0.1"
#else
#define  MTDIDS_DEFAULT  "nor0=spi0.0"
#endif

#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE  1
#endif


/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN       (CONFIG_ENV_SIZE + 2 * 1024 * 1024)

/* Physical Memory Map */

#define PHYS_SDRAM_1        0x80000000
#define PHYS_SDRAM_1_SIZE   (64 * 1024 * 1024)
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
    "kernel_offset=0x0\0" \
    "dtb_offset=0x0\0" \
    "dtb_size=0x0\0" \
    "dtb_addr=0x0\0" \
    "fdt_high=0xFFFFFFFF\0" \
    "update_flag=0x0\0" \
    "loadaddr=0x80008000\0" \
    "fdtcontroladdr=0x81300000\0" \
    "filesize=0x0\0" \
    "image_name=uImage\0" \
    "update_offset=0x0\0" \
    "update_size=0x0\0" \
    "update=tftp $(loadaddr) $(image_name); nand erase $(update_offset) $(filesize); nand write $(loadaddr) $(update_offset) $(filesize)\0" \
    "ethaddr=00:55:7b:b5:7d:f7\0" \
    "eth1addr=00:55:7b:b5:7d:f7\0" \
    "netretry=once\0" \
    "ipaddr=192.168.1.99\0" \
    "serverip=192.168.1.1\0" \
    "netmask=255.255.255.0\0" \
    "baudrate=115200\0" \
    "console=ttySAK0,115200n8\0" \
    "mtd_root=/dev/mtdblock4\0" \
    "rootfstype=yaffs2\0" \
    "init=/sbin/init\0" \
    "mem=mem=64M\0" \
    "memsize=memsize=64M\0" \
    "mtdparts=mtdparts=spi0.1:512K@0x0(SPL),512K@0x80000(ENV),512K@0x100000(ENVBK),512K@0x180000(DTB),1024K@0x200000(UBOOT),4096K@0x300000(KERNEL),1024K@0x700000(LOGO),20480K@0x800000(ROOTFS),20480K@0x1C00000(CONFIG),81920K@0x3000000(APP)\0" \
    "bootargs=console=ttySAK0,115200n8 root=/dev/mtdblock7 rootfstype=yaffs2 init=/sbin/init\0" \
    "setcmd=setenv bootargs console=${console} root=${mtd_root} rootfstype=${rootfstype} init=${init} mem=${memsize}\0" \
    "read_kernel=nand read ${loadaddr} ${kernel_offset} ${kernel_size}\0" \
    "read_dtb=nand read ${fdtcontroladdr} ${dtb_offset} ${dtb_size}\0" \
    "boot_normal=env set bootargs console=ttySAK0,115200n8 root=/dev/mtdblock7 rootfstype=yaffs2 init=/sbin/init ${mtdparts} ${mem} ${memsize}; run read_dtb; run read_kernel; bootm ${loadaddr} - ${fdtcontroladdr}\0" \
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
    "lcdloadaddr=0x82600000\0" \
    "env_version=1.1.00\0" \
    "env_platform=anycloud_ak3918av100\0" \

#elif defined(CONFIG_MTD_NOR_FLASH)
#if defined(CONFIG_ENV_64M)
#define CONFIG_EXTRA_ENV_SETTINGS \
    "a_uboot_flags=0x1\0" \
    "sf_hz=20000000\0"\
    "part_table_offset=0x0\0"\
    "kernel_offset=0x52000\0" \
    "kernel_size=0x19000\0" \
    "kernel_addr=0x0\0" \
    "dtb_offset=0x42000\0" \
    "dtb_size=0xdab1\0" \
    "dtb_addr=0x0\0" \
    "fdt_high=0xFFFFFFFF\0" \
    "update_flag=0x0\0" \
    "loadaddr=0x81000000\0" \
    "fdtcontroladdr=0x81300000\0" \
    "filesize=0x0\0" \
    "image_name=uImage\0" \
    "update_offset=0x0\0" \
    "update_size=0x0\0" \
    "update=tftp $(loadaddr) $(image_name); sf probe ; sf update $(loadaddr) $(update_offset) $(filesize) \0" \
    "ethaddr=00:55:7b:b5:7d:f7\0" \
    "ipaddr=192.168.1.99\0" \
    "serverip=192.168.1.1\0" \
    "netmask=255.255.255.0\0" \
    "baudrate=115200\0" \
    "console=ttySAK0,115200n8\0" \
    "mtd_root=/dev/mtdblock8\0" \
    "rootfstype=squashfs\0" \
    "init=/sbin/init\0" \
    "mem=mem=64M\0" \
    "memsize=memsize=64M\0" \
    "mtdparts=mtdparts=spi0.0:256K@0x0(UBOOT),4K@0x40000(ENV),4K@0x41000(ENVBK),64K@0x42000(DTB),1600K@0x52000(KERNEL),2200K@0x1E2000(ROOTFS),400K@0x408000(CONFIG),3664K@0x46C000(APP),2196K@0x1E3000(ROOTFS_MOUNT)\0" \
    "bootargs=console=ttySAK0,115200n8 root=/dev/mtdblock5 rootfstype=squashfs init=/sbin/init \0" \
    "setcmd=setenv bootargs console=${console} root=${mtd_root} rootfstype=${rootfstype} init=${init} mem=${memsize}\0" \
    "read_kernel=sf probe 0:0 ${sf_hz} 0; sf read ${loadaddr} ${kernel_offset} ${kernel_size}\0" \
    "read_dtb=sf probe 0:0 ${sf_hz} 0; sf read ${fdtcontroladdr} ${dtb_offset} ${dtb_size}\0" \
    "boot_normal=env set bootargs console=ttySAK0,115200n8 root=/dev/mtdblock8 rootfstype=squashfs init=/sbin/init ${mtdparts} ${mem} ${memsize}; run read_kernel; run read_dtb;sf read 81800000 1e2000 226000;bootm ${loadaddr} 81800000 ${fdtcontroladdr}\0" \
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
    "env_version=1.0.01\0" \
    "env_platform=anycloud_ak3918av100\0"

#elif defined(CONFIG_ENV_128M)
#define CONFIG_EXTRA_ENV_SETTINGS \
    "a_uboot_flags=0x1\0" \
    "sf_hz="    __stringify(CONFIG_SF_DEFAULT_SPEED)    "\0"\
    "part_table_offset=0x0\0"\
    "kernel_offset=0x52000\0" \
    "kernel_size=0x19000\0" \
    "kernel_addr=0x0\0" \
    "dtb_offset=0x42000\0" \
    "dtb_size=0xdab1\0" \
    "dtb_addr=0x0\0" \
    "fdt_high=0xFFFFFFFF\0" \
    "update_flag=0x0\0" \
    "loadaddr=0x81000000\0" \
    "fdtcontroladdr=0x81300000\0" \
    "filesize=0x0\0" \
    "image_name=uImage\0" \
    "update_offset=0x0\0" \
    "update_size=0x0\0" \
    "update=tftp $(loadaddr) $(image_name); sf probe ; sf update $(loadaddr) $(update_offset) $(filesize) \0" \
    "ethaddr=00:55:7b:b5:7d:f7\0" \
    "ipaddr=192.168.1.99\0" \
    "serverip=192.168.1.1\0" \
    "netmask=255.255.255.0\0" \
    "baudrate=115200\0" \
    "console=ttySAK0,115200n8\0" \
    "mtd_root=/dev/mtdblock8\0" \
    "rootfstype=squashfs\0" \
    "init=/sbin/init\0" \
    "mem=mem=128M\0" \
    "memsize=memsize=128M\0" \
    "mtdparts=mtdparts=spi0.0:256K@0x0(UBOOT),4K@0x40000(ENV),4K@0x41000(ENVBK),64K@0x42000(DTB),1600K@0x52000(KERNEL),2200K@0x1E2000(ROOTFS),400K@0x408000(CONFIG),3664K@0x46C000(APP),2196K@0x1E3000(ROOTFS_MOUNT)\0" \
    "bootargs=console=ttySAK0,115200n8 root=/dev/mtdblock5 rootfstype=squashfs init=/sbin/init \0" \
    "setcmd=setenv bootargs console=${console} root=${mtd_root} rootfstype=${rootfstype} init=${init} mem=${memsize}\0" \
    "read_kernel=sf probe 0:0 ${sf_hz} 0; sf read ${loadaddr} ${kernel_offset} ${kernel_size}\0" \
    "read_dtb=sf probe 0:0 ${sf_hz} 0; sf read ${fdtcontroladdr} ${dtb_offset} ${dtb_size}\0" \
    "boot_normal=env set bootargs console=ttySAK0,115200n8 root=/dev/mtdblock8 rootfstype=squashfs init=/sbin/init ${mtdparts} ${mem} ${memsize}; run read_kernel; run read_dtb;sf read 81800000 1e2000 226000;bootm ${loadaddr} 81800000 ${fdtcontroladdr}\0" \
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
    "env_version=1.0.01\0" \
    "env_platform=anycloud_ak3918av100\0"

#else
#ifdef CONFIG_Z1_H322D

#define CONFIG_EXTRA_ENV_SETTINGS \
    "a_uboot_flags=0x1\0" \
    "sf_hz="    __stringify(CONFIG_SF_DEFAULT_SPEED)    "\0"\
    "kernel_addr=0x0\0" \
    "kernel_size=0x0\0" \
    "dtb_offset=0x0\0" \
    "dtb_size=0x0\0" \
    "dtb_addr=0x0\0" \
    "fdt_high=0xFFFFFFFF\0" \
	"initrd_high=0xFFFFFFFF\0" \
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
    "mem=mem=128M\0" \
    "memsize=memsize=128M\0" \
    "mtdparts=mtdparts=spi-nor:220K@0x0(UBOOT),4K@0x37000(ENV),4K@0x38000(ENVBK),64K@0x39000(DTB),1600K@0x49000(KERNEL),3072K@0x1d9000(ROOTFS),400K@0x4d9000(CONFIG),4500K@0x53d000(APP)\0" \
    "bootargs=console=${console} root=${mtd_root} rootfstype=${rootfstype} init=${init} \0" \
    "setcmd=setenv bootargs console=${console} root=${mtd_root} rootfstype=${rootfstype} init=${init} mem=${memsize}\0" \
    "read_kernel=sf probe 0:0 ${sf_hz} 0; sf read ${loadaddr} ${kernel_offset} ${kernel_size}\0" \
    "read_dtb=sf probe 0:0 ${sf_hz} 0; sf read ${fdtcontroladdr} ${dtb_offset} ${dtb_size};fdt addr ${fdtcontroladdr}\0" \
    "boot_normal=env set bootargs console=${console} root=/dev/mtdblock5 rootfstype=${rootfstype} init=/sbin/init ${mtdparts}${mtdparts1} ${mem} ${memsize}; bootm 0x80007fc0 - 0x81300000\0" \
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
    "env_platform=anycloud_ak3918av100\0" \

#endif
#endif


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
