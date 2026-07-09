// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#include <common.h>
#include <bloblist.h>
#include <binman_sym.h>
#include <dm.h>
#include <handoff.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <nand.h>
#include <fat.h>
#include <version.h>
#include <image.h>
#include <malloc.h>
#include <dm/root.h>
#include <linux/compiler.h>
#include <fdt_support.h>
#include <bootcount.h>
#include <wdt.h>

#ifdef CONFIG_39EV33X_CODE
#include <asm/arch-ak39ev33x/ak_cpu.h>
#endif
#ifdef CONFIG_37_D_CODE
#include <asm/arch-ak37d/ak_cpu.h>
#endif
#ifdef CONFIG_37_E_CODE
#include <asm/arch-ak37e/ak_cpu.h>
#endif
#if defined(CONFIG_3918AV100_CODE)
#include <asm/arch-ak3918av100/ak_cpu.h>
#endif

#if defined(CONFIG_3918AV130_CODE)
#include <asm/arch-ak3918av130/ak_cpu.h>
#endif

#if defined(CONFIG_KM01A_CODE)
#include <asm/arch-km01a/ak_cpu.h>
#endif

#ifdef CONFIG_3918EV300L_CODE
#include <asm/arch-ak3918ev300l/ak_cpu.h>
#endif
#ifdef CONFIG_39EV200_CODE
#include <asm/arch-ak39ev200/ak_cpu.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define BOOTTIME_DEBUG                  (1)

#if BOOTTIME_DEBUG
#define TIMER_MAX_COUNT                 (0xffffffff)
#if defined(CONFIG_39EV200_CODE)
#define TICK_TIMER_CTRL_REG1            TIMER4_CTRL1_REG    // PWM TIMER5 CFG0
#define TICK_TIMER_CTRL_REG2            TIMER4_CTRL2_REG    // PWM TIMER5 CFG1
#elif defined(CONFIG_37_D_CODE)
#define TICK_TIMER_CTRL_REG1            TIMER5_CTRL1_REG    // PWM TIMER5 CFG0
#define TICK_TIMER_CTRL_REG2            TIMER5_CTRL2_REG    // PWM TIMER5 CFG1
#elif defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
/*use timer 1*/
#define TICK_TIMER      (1)
#define TICK_TIMER_BASE (TIMER_BASE_ADDR + (TICK_TIMER * 0x1000))

/*register Offset Address*/
#define CFG_TIMER_LOAD          (0x00)
#define RET_TIMER_VALUE         (0x04)
#define CFG_TIMER_CTRL          (0x08)
#define CFG_TIMER_INTCLR        (0x0c)
#define CFG_TIMER_INT_EN        (0x10)
#define CFG_TIMER_DIV           (0x14)
#define RET_TIMER_INT_STA       (0x18)

#define AK_TIMER_INTCLR         (0x01<<0)
#define AK_TIMER_INT_EN         (0x01<<0)
#define AK_UPDATE_RELOAD        (0x1 <<3)
/*define working mode */
//00?????????????????????
#define MODE_FREE_TIMER         (0x00<<1)

//enable timer
#define ENABLE_TIMER            (0x01<<0)
//?§Ø????¦Ë
#define TIMER_INT_STA_BIT       (0x01<<0)

#else
#define TICK_TIMER_CTRL_REG1            TIMER9_CTRL1_REG    // PWM TIMER9 CFG0
#define TICK_TIMER_CTRL_REG2            TIMER9_CTRL2_REG    // PWM TIMER9 CFG1
#endif

//#define REG32(_reg)                     (*(volatile unsigned long *)(_reg))
#define TICK_NUM                        (8)

static u32 tick_z[TICK_NUM] = {0};

#define FAST_LAUNCH_GPIO    0

#endif

#ifndef CONFIG_SYS_UBOOT_START
#define CONFIG_SYS_UBOOT_START	CONFIG_SYS_TEXT_BASE
#endif
#ifndef CONFIG_SYS_MONITOR_LEN
/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
#define CONFIG_SYS_MONITOR_LEN	(200 * 1024)
#endif

u32 *boot_params_ptr = NULL;

extern unsigned long efuse_read(void);

/* See spl.h for information about this */
binman_sym_declare(ulong, u_boot_any, image_pos);

/* Define board data structure */
static bd_t bdata __attribute__ ((section(".data")));

/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak void show_boot_progress(int val) {}

#if defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF)
/* weak, default platform-specific function to initialize dram banks */
__weak int dram_init_banksize(void)
{
	return 0;
}
#endif

/*
 * Default function to determine if u-boot or the OS should
 * be started. This implementation always returns 1.
 *
 * Please implement your own board specific funcion to do this.
 *
 * RETURN
 * 0 to not start u-boot
 * positive if u-boot should start
 */
#ifdef CONFIG_SPL_OS_BOOT
__weak int spl_start_uboot(void)
{
	puts(SPL_TPL_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(SPL_TPL_PROMPT "Direct Linux boot not active!\n");
	return 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
int __weak bootz_setup(ulong image, ulong *start, ulong *end)
{
	 return 1;
}
#endif

/* Weak default function for arch/board-specific fixups to the spl_image_info */
void __weak spl_perform_fixups(struct spl_image_info *spl_image)
{
}

void spl_fixup_fdt(void)
{
#if defined(CONFIG_SPL_OF_LIBFDT) && defined(CONFIG_SYS_SPL_ARGS_ADDR)
	void *fdt_blob = (void *)CONFIG_SYS_SPL_ARGS_ADDR;
	int err;

	err = fdt_check_header(fdt_blob);
	if (err < 0) {
		printf("fdt_root: %s\n", fdt_strerror(err));
		return;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_to_minimum(fdt_blob, 0);
	if (err == 0) {
		printf(SPL_TPL_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		return;
	}

	err = arch_fixup_fdt(fdt_blob);
	if (err) {
		printf(SPL_TPL_PROMPT "arch_fixup_fdt err - %d\n", err);
		return;
	}
#endif
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak void spl_board_prepare_for_linux(void)
{
	/* Nothing to do! */
}

__weak void spl_board_prepare_for_boot(void)
{
	/* Nothing to do! */
}

__weak struct image_header *spl_get_load_buffer(ssize_t offset, size_t size)
{
	return (struct image_header *)(CONFIG_SYS_TEXT_BASE + offset);
}

void spl_set_header_raw_uboot(struct spl_image_info *spl_image)
{
	ulong u_boot_pos = binman_sym(ulong, u_boot_any, image_pos);

	spl_image->size = CONFIG_SYS_MONITOR_LEN;

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	if (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->entry_point = u_boot_pos;
		spl_image->load_addr = u_boot_pos;
	} else {
		spl_image->entry_point = CONFIG_SYS_UBOOT_START;
		spl_image->load_addr = CONFIG_SYS_TEXT_BASE;
	}
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = "U-Boot";
}

#ifdef CONFIG_SPL_LOAD_FIT_SEPARATE
static int spl_load_fit_image(struct spl_image_info *spl_image,const struct image_header *header)
{
	bootm_headers_t images;
	const char *fit_uname_config = NULL;
	int ret;
	
	memset(&images,0,sizeof(bootm_headers_t));
#ifdef CONFIG_SPL_FIT_SIGNATURE
	images.verify = 1;
#endif
	ulong fw_data = 0, os_data = 0;
	ulong fw_len = 0,  os_len = 0;
	
	ret = fit_image_load(&images, (ulong)header,
			     NULL, &fit_uname_config,
			     IH_ARCH_DEFAULT, IH_TYPE_STANDALONE, -1,
			     FIT_LOAD_REQUIRED, &fw_data, &fw_len);
	if(ret >= 0)
	{
		spl_image->size = fw_len;
		spl_image->entry_point = fw_data;
		spl_image->load_addr = fw_data;
		spl_image->os = IH_OS_U_BOOT;
		spl_image->name = "U-Boot";

		debug(SPL_TPL_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
		      spl_image->name, spl_image->load_addr, spl_image->size);			
		return 0;
	}

	ret = fit_image_load(&images, (ulong)header,
			     NULL, &fit_uname_config,
			     IH_ARCH_DEFAULT, IH_TYPE_KERNEL, -1,
			     FIT_LOAD_IGNORED, &os_data, &os_len);
	
	if (ret < 0)
		return ret;

	spl_image->os = image_get_os((struct image_header *)os_data);
	spl_image->name = image_get_name((struct image_header *)os_data);
	spl_image->entry_point = os_data + sizeof(struct image_header);			
	spl_image->load_addr = os_data ;
	spl_image->size = os_len;
	
	debug(SPL_TPL_PROMPT "payload image: %32s load addr: 0x%lx entry_point:0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->entry_point, spl_image->size);
	
	return 0;	
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT_FULL
/* Parse and load full fitImage in SPL */
static int spl_load_fit_image(struct spl_image_info *spl_image,
			      const struct image_header *header)
{
	bootm_headers_t images;
	const char *fit_uname_config = NULL;
	const char *fit_uname_fdt = FIT_FDT_PROP;
	const char *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	int idx, conf_noffset;
	int ret;

	memset(&images,0,sizeof(bootm_headers_t));
#ifdef CONFIG_SPL_FIT_SIGNATURE
	images.verify = 1;
#endif
	ret = fit_image_load(&images, (ulong)header,
			     NULL, &fit_uname_config,
			     IH_ARCH_DEFAULT, IH_TYPE_STANDALONE, -1,
			     FIT_LOAD_REQUIRED, &fw_data, &fw_len);
	if (ret < 0)
		return ret;

	spl_image->size = fw_len;
	spl_image->entry_point = fw_data;
	spl_image->load_addr = fw_data;
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = "U-Boot";

	debug(SPL_TPL_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

#ifdef CONFIG_SPL_FIT_SIGNATURE
	images.verify = 1;
#endif
	ret = fit_image_load(&images, (ulong)header,
		       &fit_uname_fdt, &fit_uname_config,
		       IH_ARCH_DEFAULT, IH_TYPE_FLATDT, -1,
		       FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	if (ret >= 0)
		spl_image->fdt_addr = (void *)dt_data;

	conf_noffset = fit_conf_get_node((const void *)header,
					 fit_uname_config);
	if (conf_noffset <= 0)
		return 0;

	for (idx = 0;
	     uname = fdt_stringlist_get((const void *)header, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				NULL), uname;
	     idx++)
	{
#ifdef CONFIG_SPL_FIT_SIGNATURE
		images.verify = 1;
#endif
		ret = fit_image_load(&images, (ulong)header,
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_LOAD_OPTIONAL_NON_ZERO,
				     &img_data, &img_len);
		if (ret < 0)
			return ret;
	}

	return 0;
}
#endif

int spl_parse_image_header(struct spl_image_info *spl_image,
			   const struct image_header *header)
{
#if defined (CONFIG_SPL_LOAD_FIT_FULL) || defined(CONFIG_SPL_LOAD_FIT_SEPARATE)
	int ret = spl_load_fit_image(spl_image, header);

	if (!ret)
		return ret;
#endif
	if (image_get_magic(header) == IH_MAGIC) {
#ifdef CONFIG_SPL_LEGACY_IMAGE_SUPPORT
		u32 header_size = sizeof(struct image_header);

#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
		/* check uImage header CRC */
		if (!image_check_hcrc(header)) {
			puts("SPL: Image header CRC check failed!\n");
			return -EINVAL;
		}
#endif

		if (spl_image->flags & SPL_COPY_PAYLOAD_ONLY) {
			/*
			 * On some system (e.g. powerpc), the load-address and
			 * entry-point is located at address 0. We can't load
			 * to 0-0x40. So skip header in this case.
			 */
			spl_image->load_addr = image_get_load(header);
			spl_image->entry_point = image_get_ep(header);
			spl_image->size = image_get_data_size(header);
		} else {
			spl_image->entry_point = image_get_load(header);
			/* Load including the header */
			spl_image->load_addr = spl_image->entry_point -
				header_size;
			spl_image->size = image_get_data_size(header) +
				header_size;
		}
#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
		/* store uImage data length and CRC to check later */
		spl_image->dcrc_data = image_get_load(header);
		spl_image->dcrc_length = image_get_data_size(header);
		spl_image->dcrc = image_get_dcrc(header);
#endif

		spl_image->os = image_get_os(header);
		spl_image->name = image_get_name(header);
		debug(SPL_TPL_PROMPT
		      "payload image: %32s load addr: 0x%lx size: %d\n",
		      spl_image->name, spl_image->load_addr, spl_image->size);
#else
		/* LEGACY image not supported */
		debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
		return -EINVAL;
#endif
	} else {
#ifdef CONFIG_SPL_PANIC_ON_RAW_IMAGE
		/*
		 * CONFIG_SPL_PANIC_ON_RAW_IMAGE is defined when the
		 * code which loads images in SPL cannot guarantee that
		 * absolutely all read errors will be reported.
		 * An example is the LPC32XX MLC NAND driver, which
		 * will consider that a completely unreadable NAND block
		 * is bad, and thus should be skipped silently.
		 */
		panic("** no mkimage signature but raw image not supported");
#endif

#ifdef CONFIG_SPL_OS_BOOT
		ulong start, end;

		if (!bootz_setup((ulong)header, &start, &end)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
			spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
			spl_image->size = end - start;
			debug(SPL_TPL_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			return 0;
		}
#endif

#ifdef CONFIG_SPL_RAW_IMAGE_SUPPORT
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
			header->ih_magic);
		spl_set_header_raw_uboot(spl_image);
#else
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		return -EINVAL;
#endif
	}

	return 0;
}

__weak void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)spl_image->entry_point;

	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	image_entry();
}

#if CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
static int setup_spl_handoff(void)
{
	struct spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_SPL_HANDOFF, sizeof(struct spl_handoff));
	if (!ho)
		return -ENOENT;

	return 0;
}

static int write_spl_handoff(void)
{
	struct spl_handoff *ho;

	ho = bloblist_find(BLOBLISTT_SPL_HANDOFF, sizeof(struct spl_handoff));
	if (!ho)
		return -ENOENT;
	handoff_save_dram(ho);
#ifdef CONFIG_SANDBOX
	ho->arch.magic = TEST_HANDOFF_MAGIC;
#endif
	debug(SPL_TPL_PROMPT "Wrote SPL handoff\n");

	return 0;
}
#else
static inline int setup_spl_handoff(void) { return 0; }
static inline int write_spl_handoff(void) { return 0; }

#endif /* HANDOFF */

static int spl_common_init(bool setup_malloc)
{
	int ret;

#if CONFIG_VAL(SYS_MALLOC_F_LEN)
	if (setup_malloc) {
#ifdef CONFIG_MALLOC_F_ADDR
		gd->malloc_base = CONFIG_MALLOC_F_ADDR;
#endif
		gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#endif
	ret = bootstage_init(true);
	if (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		return ret;
	}
	bootstage_mark_name(BOOTSTAGE_ID_START_SPL, "spl");
#if CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	if (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		return ret;
	}
#endif
	if (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		if (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			return ret;
		}
	}
	if (CONFIG_IS_ENABLED(HANDOFF)) {
		int ret;

		ret = setup_spl_handoff();
		if (ret) {
			puts(SPL_TPL_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}
	if (CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)) {
		ret = fdtdec_setup();
		if (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			return ret;
		}
	}
	if (CONFIG_IS_ENABLED(DM)) 
	{
		bootstage_start(BOOTSTATE_ID_ACCUM_DM_SPL, "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_scan(!CONFIG_IS_ENABLED(OF_PLATDATA));
		bootstage_accum(BOOTSTATE_ID_ACCUM_DM_SPL);
		if (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			return ret;
		}
	}

	return 0;
}

void spl_set_bd(void)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	if (!gd->bd)
		gd->bd = &bdata;
}

int spl_early_init(void)
{
	int ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	if (ret)
		return ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	return 0;
}

int spl_init(void)
{
	int ret;
	bool setup_malloc = !(IS_ENABLED(CONFIG_SPL_STACK_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	if (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(setup_malloc);
		if (ret)
			return ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	return 0;
}

#ifndef BOOT_DEVICE_NONE
#define BOOT_DEVICE_NONE 0xdeadbeef
#endif

__weak void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

static struct spl_image_loader *spl_ll_find_loader(uint boot_device)
{
	struct spl_image_loader *drv =
		ll_entry_start(struct spl_image_loader, spl_image_loader);
	const int n_ents =
		ll_entry_count(struct spl_image_loader, spl_image_loader);
	struct spl_image_loader *entry;

	for (entry = drv; entry != drv + n_ents; entry++) {
		if (boot_device == entry->boot_device)
			return entry;
	}

	/* Not found */
	return NULL;
}
#if BOOTTIME_DEBUG
__attribute__((unused)) static void set_gpio(int pin, int level)
{
	#define LED_GPIO  29
	int grp=0, offset=0;	

	grp = pin / 32;
	offset = pin % 32;

	//set as gpio;	
	if(pin==5)
	{
		(*(volatile unsigned long *)0x08000074) &= ~(0x3 << 6); //REG1[7:6]
		(*(volatile unsigned long *)0x08000078) &= ~(0x1 << 31); //REG2[31]
		(*(volatile unsigned long *)0x080000A4) &= ~(0x1 << 29); //ANA REG3[29]
	}
	else if(pin==0)
	{
		(*(volatile unsigned long *)0x08000074) &= ~(0x3 << 0); //REG1[7:6]
		(*(volatile unsigned long *)0x08000078) &= ~(0x1 << 31); //REG2[31]
	}
	else if(pin==29)
		(*(volatile unsigned long *)0x080000DC) &= ~(0x3 << 2); //REG4[3:2]
	
	// set REG1 output dir
	(*(volatile unsigned long *)(GPIO_DIR_REG1 + grp*4)) |= (1 << offset); //DIR_REG1;	
	
	// set output level
	if(level)
		(*(volatile unsigned long *)(GPIO_OUT_REG1 + grp*4)) |= (1 << offset); //OUT_REG1; 
	else
		(*(volatile unsigned long *)(GPIO_OUT_REG1 + grp*4)) &= ~(1 << offset); //OUT_REG1; 

}

static unsigned long timer_read_current_count(void)
{
    unsigned long count;

#if !defined(CONFIG_KM01A_CODE) && !defined(CONFIG_3918AV130_CODE)
    //select read current count mode
    REG32(TICK_TIMER_CTRL_REG2) |= (1<<26);
    count = TIMER_MAX_COUNT - REG32(TICK_TIMER_CTRL_REG1);
    // recover read mode
    REG32(TICK_TIMER_CTRL_REG2) &= ~(1<<26);
#else
    count = TIMER_MAX_COUNT - __raw_readl(TICK_TIMER_BASE + RET_TIMER_VALUE);
#endif
    return count;
}

static void rec_tick_us(int i)
{
    tick_z[i] = timer_read_current_count() / 12;
}

static void prt_tick(void)
{
    printf("spl[%dus],ker[%dus] [tc:0x%08x]\r\n", tick_z[1], tick_z[2],
            __raw_readl(TICK_TIMER_BASE + RET_TIMER_VALUE));
}
#endif

static int spl_load_image(struct spl_image_info *spl_image,
			  struct spl_image_loader *loader)
{
	int ret;
	struct spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = NULL;

	ret = loader->load_image(spl_image, &bootdev);
#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	if (!ret && spl_image->dcrc_length) {
		/* check data crc */
		ulong dcrc = crc32_wd(0, (unsigned char *)spl_image->dcrc_data,
				      spl_image->dcrc_length, CHUNKSZ_CRC32);
		if (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#endif
	return ret;
}

/**
 * boot_from_devices() - Try loading a booting U-Boot from a list of devices
 *
 * @spl_image: Place to put the image details if successful
 * @spl_boot_list: List of boot devices to try
 * @count: Number of elements in spl_boot_list
 * @return 0 if OK, -ve on error
 */
static int boot_from_devices(struct spl_image_info *spl_image,
			     u32 spl_boot_list[], int count)
{
	int i;

	for (i = 0; i < count && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		struct spl_image_loader *loader;

		loader = spl_ll_find_loader(spl_boot_list[i]);
#if defined(CONFIG_SPL_SERIAL_SUPPORT) && defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
		if (loader)
			debug("Trying to boot from %s\n", loader->name);
		else
			puts(SPL_TPL_PROMPT "Unsupported Boot Device!\n");
#endif
		if (loader && !spl_load_image(spl_image, loader)) {
			spl_image->boot_device = spl_boot_list[i];
			return 0;
		}
	}

	return -ENODEV;
}

#ifdef CONFIG_37_E_CODE
static void spl_check_pll1_clock(void)
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

    if (regval & (1UL << 30))
    {
        //ddr2 work 2x mode
        //dphyclk=cpu pll, hclk=dclk=1/2 cpu pll
        dphyclk = cpu_pll_clk;
        hclk = cpu_pll_clk / 2;
        dclk = hclk;
    }
    else
    {
        //ddr2 work 1x mode
        //dphy clock mode define by cpu_5x_sel
        if (regval & (1UL << 26))
        {
            //cpu work 5x mode
            //jclk = cpu pll, hclk=dclk=dphyclk=1/5 cpu pll
            dphyclk = cpu_pll_clk / 5;
            hclk = dphyclk;
            dclk = hclk;
        }
        else
        {
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
        printf("Can't set dphy clk rate %u, Max rate: %u! halt!!!\n", dphyclk, DPHY_CLK_MAX_RATE);
        while (1);
    }
    if (hclk > HCLK_MAX_RATE) {
        printf("Can't set hclk rate %u, Max rate: %u! halt!!!\n", hclk, HCLK_MAX_RATE);
        while (1);
    }
    if (dclk > HCLK_MAX_RATE) {
        printf("Can't set dclk rate %u, Max rate: %u! halt!!!\n", dclk, HCLK_MAX_RATE);
        while (1);
    }

}
#endif

static void gpio70_on_and_off(void)
{
    /* set pinmux */
    *(volatile unsigned int *)(0x08000198) &= ~(0x7 << 24);
    *(volatile unsigned int *)(0x08000198) |= 0 << 24;
    /* set direction output mode */
    *(volatile unsigned int *)(0x20170008) |= 0x1 << 6;


    /* set output low level */
    *(volatile unsigned int *)(0x2017001c) &= ~(0x1 << 6);
    //  mdelay(1000);
    /* set output high level */
    *(volatile unsigned int *)(0x2017001c) |= 0x1 << 6;
    //  mdelay(1000);
    /* set output low level */
    *(volatile unsigned int *)(0x2017001c) &= ~(0x1 << 6);

    }

void board_init_r(gd_t *dummy1, ulong dummy2)
{
    gpio70_on_and_off();
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	struct spl_image_info spl_image;
	int ret;
#if defined(CONFIG_39EV33X_CODE) || defined(CONFIG_37_D_CODE)
	unsigned long value = 0;
#endif
#if defined(CONFIG_3918AV130_CODE)
    unsigned long value;
#endif

//spl fast boot,for no DM
#ifndef CONFIG_DM_SPI
	unsigned long value_tmp = 0;
#ifdef CONFIG_37_E_CODE
    REG32(CORE_PLL_CHANNEL_CTRL_REG) = 0x100137D;
#endif
#ifdef CONFIG_37_D_CODE
	REG32(CORE_PLL_CHANNEL_CTRL_REG) = 0x018213F0;
#endif
#ifdef CONFIG_39EV33X_CODE
	REG32(CORE_PLL_CHANNEL_CTRL_REG) = 0x018213C8; ///400 for spi clk 50M
#endif
#if (defined(CONFIG_3918AV100_CODE) || defined(CONFIG_3918EV300L_CODE) || defined(CONFIG_KM01A_CODE))
	REG32(CORE_PLL_CHANNEL_CTRL_REG) = 0x280063;
#endif
#if defined(CONFIG_3918AV130_CODE)
    //PLL1 = 800M
    // check if pll1 adjust processing

    while ((__raw_readl(CPU_PLL_CHANNEL_CTRL_REG) & (1 << 28)) != 0) {
        ;
    }

    __raw_writel(0x008840C7, CORE_PLL_CHANNEL_CTRL_REG);//nf=200, nr=3, od=2. pll2=24M * nf / nr / od=800M
    __arch_getl(0x00000000);

    value = __raw_readl(CPU_PLL_CHANNEL_CTRL_REG);
    value |= (1 << 28);
    __raw_writel(value, CPU_PLL_CHANNEL_CTRL_REG);
    __arch_getl(0x00000000);

    // wait pll adjust finish
    while ((__raw_readl(CPU_PLL_CHANNEL_CTRL_REG) & (1 << 28)) != 0) {
        ;
    }

    //GCLK = 200M
    // check if asic pll adjust processing
    while ((__raw_readl(CORE_PLL_CHANNEL_CTRL_REG) & (1 << 25)) != 0) {
        ;
    }

    value = __raw_readl(CORE_PLL_CHANNEL_CTRL_REG);
    value &= ~(0x3 << 23);
    value |= 1 << 23;
    __raw_writel(value, CORE_PLL_CHANNEL_CTRL_REG);
    __arch_getl(0x00000000);

    value = __raw_readl(CORE_PLL_CHANNEL_CTRL_REG);
    value |= (1 << 25);
    __raw_writel(value, CORE_PLL_CHANNEL_CTRL_REG);
    __arch_getl(0x00000000);

    // wait pll adjust finish
    while ((__raw_readl(CORE_PLL_CHANNEL_CTRL_REG) & (1 << 25)) != 0) {
        ;
    }
#endif
#ifdef CONFIG_39EV200_CODE
	REG32(CORE_PLL_CHANNEL_CTRL_REG) = 0x018213C8;
#endif
	REG32(CPU_PLL_CHANNEL_CTRL_REG) |= (0x1<<28);
	while(1) {
		value_tmp = REG32(CPU_PLL_CHANNEL_CTRL_REG);
		if (!(value_tmp&(0x1<<28))) {
			break;
		}
	}
#endif

#if !defined(CONFIG_KM01A_CODE) && !defined(CONFIG_3918AV130_CODE)
	//close timer1, timer1 was used in bootcore
	REG32(TIMER1_CTRL1_REG) = 0;
	REG32(TIMER1_CTRL2_REG) = 0x40000000;
#endif

	//close watchdog
	REG32(0x080000e8) = 0xaa000000;

	debug(">>" SPL_TPL_PROMPT "board_init_r()\n");

	spl_set_bd();

#if BOOTTIME_DEBUG
	//set_gpio(FAST_LAUNCH_GPIO, 0);
#if !defined(CONFIG_KM01A_CODE) && !defined(CONFIG_3918AV130_CODE)
	REG32(TICK_TIMER_CTRL_REG1) = TIMER_MAX_COUNT;
	REG32(TICK_TIMER_CTRL_REG2) = (1 << 28) | (1 << 29) | (0x1 << 24);
#else
    unsigned long regval;

    __raw_writel(TIMER_MAX_COUNT, TICK_TIMER_BASE + CFG_TIMER_LOAD);
    __raw_writel(AK_TIMER_INT_EN, TICK_TIMER_BASE + CFG_TIMER_INT_EN);

    regval = __raw_readl(TICK_TIMER_BASE + CFG_TIMER_DIV);
    regval &= ~(0xFF);
    regval |=  0x1;
    __raw_writel(regval, TICK_TIMER_BASE + CFG_TIMER_DIV);

    regval = __raw_readl(TICK_TIMER_BASE + CFG_TIMER_CTRL);
    regval &= ~(AK_UPDATE_RELOAD);//don't update reload value
    regval &= ~(0x3 << 1);
    regval |= MODE_FREE_TIMER;
    __raw_writel(regval, TICK_TIMER_BASE + CFG_TIMER_CTRL);

    regval = __raw_readl(TICK_TIMER_BASE + CFG_TIMER_CTRL);
    regval |=ENABLE_TIMER;
    __raw_writel(regval, TICK_TIMER_BASE + CFG_TIMER_CTRL);
#endif
    rec_tick_us(0);
#endif

#if defined(CONFIG_SYS_SPL_MALLOC_START)
	mem_malloc_init(CONFIG_SYS_SPL_MALLOC_START,
			CONFIG_SYS_SPL_MALLOC_SIZE);
	gd->flags |= GD_FLG_FULL_MALLOC_INIT;
#endif
	if (!(gd->flags & GD_FLG_SPL_INIT)) {
		if (spl_init())
			hang();
	}
#if !defined(CONFIG_PPC) && !defined(CONFIG_ARCH_MX6)
	/*
	 * timer_init() does not exist on PPC systems. The timer is initialized
	 * and enabled (decrementer) in interrupt_init() here.
	 */
	timer_init();
#endif

#if CONFIG_IS_ENABLED(BOARD_INIT)
	spl_board_init();
#endif

#if defined(CONFIG_39EV33X_CODE) || defined(CONFIG_37_D_CODE)
	value = efuse_read();
	debug("efuse_read:0x%lx\n", value);
#endif

#if defined(CONFIG_SPL_WATCHDOG_SUPPORT) && CONFIG_IS_ENABLED(WDT)
	initr_watchdog();
#endif

	if (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF))
		dram_init_banksize();

	bootcount_inc();

	memset(&spl_image, '\0', sizeof(spl_image));
#ifdef CONFIG_SYS_SPL_ARGS_ADDR
	spl_image.arg = (void *)CONFIG_SYS_SPL_ARGS_ADDR;
#endif
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_order(spl_boot_list);
#if BOOTTIME_DEBUG
	rec_tick_us(1);
#endif

	if (boot_from_devices(&spl_image, spl_boot_list,
			      ARRAY_SIZE(spl_boot_list))) {
		puts(SPL_TPL_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_fixups(&spl_image);
	if (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		if (ret)
			printf(SPL_TPL_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}

	if (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_finish();
		if (ret)
			printf("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

#if BOOTTIME_DEBUG
	//set_gpio(FAST_LAUNCH_GPIO,1);
	rec_tick_us(2);
	prt_tick();
#endif	

#ifdef CONFIG_CPU_V7M
	spl_image.entry_point |= 0x1;
#endif
	switch (spl_image.os) {
	case IH_OS_U_BOOT:
		printf("Jumping to U-Boot\n");
		break;
#if CONFIG_IS_ENABLED(ATF)
	case IH_OS_ARM_TRUSTED_FIRMWARE:
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_invoke_atf(&spl_image);
		break;
#endif
#if CONFIG_IS_ENABLED(OPTEE)
	case IH_OS_TEE:
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_optee_entry(NULL, NULL, spl_image.fdt_addr,
				(void *)spl_image.entry_point);
		break;
#endif
#if CONFIG_IS_ENABLED(OPENSBI)
	case IH_OS_OPENSBI:
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		spl_invoke_opensbi(&spl_image);
		break;
#endif
#ifdef CONFIG_SPL_OS_BOOT
	case IH_OS_LINUX:
//		printf("Jumping to linux\n");
#ifndef CONFIG_KERNEL_POST_LOAD
		spl_fixup_fdt();
		spl_board_prepare_for_linux();
#endif
		jump_to_image_linux(&spl_image);
#endif
	default:
		printf("Unsupported OS image.. Jumping nevertheless..\n");
	}
#if CONFIG_VAL(SYS_MALLOC_F_LEN) && !defined(CONFIG_SYS_SPL_MALLOC_SIZE)
	debug("SPL malloc() used 0x%lx bytes (%ld KB)\n", gd->malloc_ptr,
	      gd->malloc_ptr / 1024);
#endif
#ifdef CONFIG_BOOTSTAGE_STASH
	bootstage_mark_name(BOOTSTAGE_ID_END_SPL, "end_spl");
	ret = bootstage_stash((void *)CONFIG_BOOTSTAGE_STASH_ADDR,
			      CONFIG_BOOTSTAGE_STASH_SIZE);
	if (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);
#endif

	debug("loaded - jumping to U-Boot...\n");
	spl_board_prepare_for_boot();
	jump_to_image_no_args(&spl_image);
}

#ifdef CONFIG_SPL_SERIAL_SUPPORT
/*
 * This requires UART clocks to be enabled.  In order for this to work the
 * caller must ensure that the gd pointer is valid.
 */
void preloader_console_init(void)
{
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->have_console = 1;

#if CONFIG_IS_ENABLED(BANNER_PRINT)
	/*
		puts("\nU-Boot " SPL_TPL_NAME " " PLAIN_VERSION " (" U_BOOT_DATE " - "
			 U_BOOT_TIME " " U_BOOT_TZ ")\n");
	*/
		puts("\n" SPL_TPL_NAME " " PLAIN_VERSION "\n");
#endif
#ifdef CONFIG_SPL_DISPLAY_PRINT
	spl_display_print();
#endif
}
#endif

/**
 * This function is called before the stack is changed from initial stack to
 * relocated stack. It tries to dump the stack size used
 */
__weak void spl_relocate_stack_check(void)
{
#if CONFIG_IS_ENABLED(SYS_REPORT_STACK_F_USAGE)
	ulong init_sp = gd->start_addr_sp;
	ulong stack_bottom = init_sp - CONFIG_VAL(SIZE_LIMIT_PROVIDE_STACK);
	u8 *ptr = (u8 *)stack_bottom;
	ulong i;

	for (i = 0; i < CONFIG_VAL(SIZE_LIMIT_PROVIDE_STACK); i++) {
		if (*ptr != CONFIG_VAL(SYS_STACK_F_CHECK_BYTE))
			break;
		ptr++;
	}
	printf("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(SIZE_LIMIT_PROVIDE_STACK) - i);
#endif
}

/**
 * spl_relocate_stack_gd() - Relocate stack ready for board_init_r() execution
 *
 * Sometimes board_init_f() runs with a stack in SRAM but we want to use SDRAM
 * for the main board_init_r() execution. This is typically because we need
 * more stack space for things like the MMC sub-system.
 *
 * This function calculates the stack position, copies the global_data into
 * place, sets the new gd (except for ARM, for which setting GD within a C
 * function may not always work) and returns the new stack position. The
 * caller is responsible for setting up the sp register and, in the case
 * of ARM, setting up gd.
 *
 * All of this is done using the same layout and alignments as done in
 * board_init_f_init_reserve() / board_init_f_alloc_reserve().
 *
 * @return new stack location, or 0 to use the same stack
 */
ulong spl_relocate_stack_gd(void)
{
#ifdef CONFIG_SPL_STACK_R
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_STACK_R_ADDR;

	if (CONFIG_IS_ENABLED(SYS_REPORT_STACK_F_USAGE))
		spl_relocate_stack_check();

#if defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_VAL(SYS_MALLOC_F_LEN)
	if (CONFIG_SPL_STACK_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%lx bytes (%ld KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_STACK_R_MALLOC_SIMPLE_LEN;
		gd->malloc_base = ptr;
		gd->malloc_limit = CONFIG_SPL_STACK_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#endif
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr = CONFIG_SPL_STACK_R_ADDR - roundup(sizeof(gd_t),16);
	new_gd = (gd_t *)ptr;
	memcpy(new_gd, (void *)gd, sizeof(gd_t));
#if CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#endif
#if !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#endif
	return ptr;
#else
	return 0;
#endif
}
