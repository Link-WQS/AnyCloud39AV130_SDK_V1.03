// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * based on drivers/mtd/nand/raw/nand_spl_load.c
 *
 * Copyright (C) 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#include <common.h>
#include <spi.h>
#include <spi_flash.h>
#include <errno.h>
#include <spl.h>
#ifdef CONFIG_39EV33X_CODE
#include <asm/arch-ak39ev33x/ak_cpu.h>
#endif

#ifdef CONFIG_39EV200_CODE
#include <asm/arch-ak39ev200/ak_cpu.h>
#endif

DECLARE_GLOBAL_DATA_PTR;


typedef struct
{
    u32 chip_id;
    u32 total_size;             ///< flash total size in bytes
    u32	page_size;       ///< total bytes per page
    u32	program_size;    ///< program size at 02h command
    u32	erase_size;      ///< erase size at d8h command
    u32	clock;           ///< spi clock, 0 means use default clock

    //chip character bits:
    //bit 0: under_protect flag, the serial flash under protection or not when power on
    //bit 1: fast read flag
    u8  flag;            ///< chip character bits
    u8	protect_mask;
	u8  reserved1;
    u8  reserved2;
    u8  des_str[32];
}T_SFLASH_PHY_INFO;


extern int device_probe(struct udevice *dev);
extern int pare_str_form_env(char *buf, char *str, char *out_buf);
extern int spl_device_parse(const char *const mtd_dev, ulong flash_size);
extern int get_mtdparts_by_partition_name (char *partition_name, unsigned long partition_num, unsigned long *run_add, unsigned long *run_len);
extern void spl_fdt_set_totalsize(void *fdt, uint32_t val);
extern int spl_fdt_chosen(void *fdt, int force, char *env_buf);


#ifndef CONFIG_SYS_FDT_PAD
#define CONFIG_SYS_FDT_PAD 0x3000
#endif

#if 0
//#ifdef CONFIG_SPL_OS_BOOT
/*
 * Load the kernel, check for a valid header we can parse, and if found load
 * the kernel and then device tree.
 */
static int spi_load_image_os(struct spl_image_info *spl_image,
			     struct spi_flash *flash,
			     struct image_header *header)
{
	int err;

	/* Read for a header, parse or error out. */
	spi_flash_read(flash, CONFIG_SYS_SPI_KERNEL_OFFS, sizeof(*header),
		       (void *)header);

	if (image_get_magic(header) != IH_MAGIC)
		return -1;

	err = spl_parse_image_header(spl_image, header);
	if (err)
		return err;

	spi_flash_read(flash, CONFIG_SYS_SPI_KERNEL_OFFS,
		       spl_image->size, (void *)spl_image->load_addr);

	/* Read device tree. */
	spi_flash_read(flash, CONFIG_SYS_SPI_ARGS_OFFS,
		       CONFIG_SYS_SPI_ARGS_SIZE,
		       (void *)CONFIG_SYS_SPL_ARGS_ADDR);

	return 0;
}
#endif



static ulong spl_spi_fit_read(struct spl_load_info *load, ulong sector,
			      ulong count, void *buf)
{
	struct spi_flash *flash = load->dev;
	ulong ret;

	ret = spi_flash_read(flash, sector, count, buf);
	if (!ret)
		return count;
	else
		return 0;
}
/*
 * The main entry for SPI booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from SPI into SDRAM and starts it from there.
 */
static int spl_spi_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	int err = 0;
	unsigned long payload_offs = 0x85000; //CONFIG_SYS_SPI_U_BOOT_OFFS;
	struct spi_flash *flash;
	struct image_header *header;
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	char env_buf[CONFIG_ENV_SIZE] = {0};
	char out_buf[1024] = {0};
	//unsigned long load_addr = 0;
	unsigned long fdtcontroladdr = 0;
	unsigned long image_flag = 0;
	unsigned long  run_add = 0;
	unsigned long  run_len = 0;
	unsigned long  part_num = 0;
	ulong	of_size = 0;
	ulong	of_len = 0;
	#ifdef CONFIG_KERNEL_POST_LOAD
	unsigned long kernel_post_load_len = 0;
	#endif
	int	ret, i = 0;
	int kernel_post_load_len_tmp = 0;

	#ifdef CONFIG_DM_SPI_FLASH
	struct udevice *dev;
	
	/*
	 * Load U-Boot image from SPI flash into RAM
	 * In DM mode: defaults speed and mode will be
	 * taken from DT when available
	 */
	uclass_get_device(UCLASS_SPI, 0, &dev);
	device_probe(dev);

	ret = spi_flash_probe_bus_cs(bus, cs,speed, mode,&dev);
	if (ret) {
		printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
		return ret;
	}

	flash = dev_get_uclass_priv(dev);

	#else
//	printf("spl_spi_load_image@@@@@@@@@@@@@@:%x\r\n", CONFIG_SF_DEFAULT_MODE);
	flash = spi_flash_probe(bus, cs,speed, mode);
	#endif
	if (!flash) {
		puts("SPI probe failed.\n");
		return -ENODEV;
	}

	//printf("flash->size:%d\r\n", flash->size);

	header = spl_get_load_buffer(-sizeof(*header), sizeof(*header));
	//printf("spl_get_load_buffer:%x\r\n", header);
#ifndef CONFIG_KERNEL_POST_LOAD
#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
	payload_offs = fdtdec_get_config_int(gd->fdt_blob,
					     "u-boot,spl-payload-offset",
					     payload_offs);
#endif
#endif

	//read env for partiton info
	err = spi_flash_read(flash, CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE, env_buf);
	if (err) {
		debug("%s: Failed to read from SPI flash (err=%d)\n",
		      __func__, err);
		return err;
	}

	//memset(out_buf, 0, 1024);
	//pare_str_form_env(env_buf, "loadaddr=", out_buf);
	//load_addr = simple_strtoul(&out_buf[9], NULL, 16); //getenv_ulong("loadaddr", 16, load_addr);
	//printf("load_addr:%x\n",load_addr);
#ifdef FIXED_FDTCONTROLADDR
    fdtcontroladdr = FIXED_FDTCONTROLADDR;
#else
	memset(out_buf, 0, 1024);    
	pare_str_form_env(env_buf, "fdtcontroladdr=", out_buf);
	fdtcontroladdr = simple_strtoul(&out_buf[15], NULL, 16); //getenv_ulong("fdtcontroladdr", 16, fdtcontroladdr);
	//printf("fdtcontroladdr:%x\n",fdtcontroladdr);
#endif
	spl_image->arg = (void *)fdtcontroladdr;
	memset(out_buf, 0, 1024);
	pare_str_form_env(env_buf, "mtdparts=mtdparts=", out_buf);
	part_num = spl_device_parse(&out_buf[9], flash->size);
	//printf("part_num:%x\n",part_num);

	pare_str_form_env(env_buf, "a_uboot_flags=", out_buf);
	image_flag = simple_strtoul(&out_buf[14], NULL, 16); //getenv_ulong("fdtcontroladdr", 16, fdtcontroladdr);


#if defined(CONFIG_39EV33X_CODE)
	/*add for juan, enter into uboot mode for update from SD card */
#ifdef CONFIG_LOAD_IMAGE_JUNGE_UART
    //judge uart rx (v330)
    REG32(SHARE_PIN_CFG0_REG) &= ~(0x1<<2);
	REG32(GPIO_DIR_REG1) &= ~(0x1<<1);
	if(!(REG32(GPIO_IN_REG1)&0x2)) {
		image_flag = 0x1;
		printf("uart rx check low level, load uboot:%ld\n", image_flag);
	}
	REG32(SHARE_PIN_CFG0_REG) |= (0x1<<2);
#endif
#endif

#if defined(CONFIG_39EV200_CODE)
	/*add for juan, enter into uboot mode for update from SD card */
#ifdef CONFIG_LOAD_IMAGE_JUNGE_UART
    //judge uart rx (v200)
    REG32(SHARE_PIN_CFG0_REG) &= ~(0x1<<1);
	REG32(GPIO_DIR_REG1) &= ~(0x1<<1);
	if(!(REG32(GPIO_IN_REG1)&0x2)) {
		image_flag = 0x1;
		printf("uart rx check low level, load uboot:%ld\n", image_flag);
	}
	REG32(SHARE_PIN_CFG0_REG) |= (0x1<<1);
#endif
#endif

	//printf("image_flag:%d\n",image_flag);
	if(image_flag)
	{
		if(get_mtdparts_by_partition_name (PART_UBOOT_NAME, part_num, &run_add, &run_len) == -1){
			puts("spinor_get_mtdparts UBOOT failed, and read kernel.\n");
			return -EINVAL;
		}
	}else{
		if(get_mtdparts_by_partition_name (PART_BIOS_NAME, part_num, &run_add, &run_len) == -1){
			puts("spinor_get_mtdparts KERNEL failed.\n");
			return -EINVAL;
		}
	}

	payload_offs = run_add;

#ifdef CONFIG_SPL_OS_BOOT
	if (1)//spl_start_uboot() || spi_load_image_os(spl_image, flash, header))
#endif
	{
		u32 header_size = sizeof(struct image_header);
		//printf("spi_flash_read, payload_offs:0x%lx\r\n", payload_offs);
		/* Load u-boot, mkimage header is 64 bytes. */
		memset(out_buf, 0, 1024);
		err = spi_flash_read(flash, payload_offs, 1024, (void *)out_buf);
		memcpy(header, out_buf, header_size);
		if (err) {
			debug("%s: Failed to read from SPI flash (err=%d)\n",
			      __func__, err);
			return err;
		}
		//load uboot,fdt,loadable type fit image
		if (IS_ENABLED(CONFIG_SPL_LOAD_FIT_FULL) &&
		    image_get_magic(header) == FDT_MAGIC) {
			err = spi_flash_read(flash, payload_offs,
					     roundup(fdt_totalsize(header), 4),
					     (void *)CONFIG_SYS_LOAD_ADDR);
			if (err)
				return err;
			err = spl_parse_image_header(spl_image,
					(struct image_header *)CONFIG_SYS_LOAD_ADDR);
		}
		//load uboot,kernel,dtb single fit image	
		else if (IS_ENABLED(CONFIG_SPL_LOAD_FIT_SEPARATE) &&
		    image_get_magic(header) == FDT_MAGIC) {
			err = spi_flash_read(flash, payload_offs,
					     roundup(fdt_totalsize(header), 4),
					     (void *)CONFIG_SYS_LOAD_ADDR);
			if (err)
				return err;
			err = spl_parse_image_header(spl_image,
					(struct image_header *)CONFIG_SYS_LOAD_ADDR);

			if(spl_image->os == IH_OS_LINUX)
			{
				//read dtb
				if(get_mtdparts_by_partition_name (PART_DTB_NAME, part_num, &run_add, &run_len) == -1){
					printf("spinor_get_mtdparts DTB failed, and read kernel.\n");
					return -EINVAL;
				}

				payload_offs = run_add;				
				err = spi_flash_read(flash, payload_offs, run_len, (void *)fdtcontroladdr);
				if (err) {
					printf("%s: Failed to read dtb from SPI flash (err=%d)\n",
					      __func__, err);
					return err;
				}
				
				bootm_headers_t images;
				const char *fit_uname_config = NULL;
				ulong fdt_data = 0, fdt_len = 0;
				memset(&images,0,sizeof(bootm_headers_t));
				ret = fit_image_load(&images, fdtcontroladdr,
				       NULL, &fit_uname_config,IH_ARCH_DEFAULT, IH_TYPE_FLATDT, -1,
				       FIT_LOAD_OPTIONAL, &fdt_data, &fdt_len);
				if(ret < 0){
					printf("%s: Failed to load fdt fit image (err=%d)\n",
					      __func__, ret);
					return ret;
				}
				memmove((void*)fdtcontroladdr, (const void*)fdt_data, fdt_len);		
				of_size = fdt_totalsize((void *)fdtcontroladdr);
				of_len = of_size + CONFIG_SYS_FDT_PAD;
				//printf("%s, line:%d, fdt:0x%x, of_len:0x%x!\n", __func__, __LINE__, fdtcontroladdr, of_len);
				spl_fdt_set_totalsize((void *)fdtcontroladdr, of_len);
				spl_fdt_chosen((void *)fdtcontroladdr, 1, env_buf);
			}
		
		}
		//load single image	
		else if (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
			   image_get_magic(header) == FDT_MAGIC) {
			struct spl_load_info load;

			debug("Found FIT\n");
			load.dev = flash;
			load.priv = NULL;
			load.filename = NULL;
			load.bl_len = 1;
			load.read = spl_spi_fit_read;
			err = spl_load_simple_fit(spl_image, &load,
						  payload_offs,
						  header);
		} else {
			//printf("spl_parse_image_header@@@@@\r\n");
			u32 first_load_size = 1024 - header_size;
			spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;
			err = spl_parse_image_header(spl_image, header);
			if (err)
				return err;
			spl_image->entry_point = spl_image->load_addr;
			memcpy((void *)spl_image->load_addr, out_buf + header_size, first_load_size);
			//printf("read payload_offs:0x%lx, spl_image->size:%lx, spl_image->load_addr:0x%lx\r\n", payload_offs, spl_image->size, spl_image->load_addr);
#ifdef CONFIG_KERNEL_POST_LOAD
			for(i = 0; i < sizeof(header->ih_name) - strlen("#0x8"); i++){
				if(memcmp(&header->ih_name[i], "#0x8", strlen("#0x8")) == 0){
					break;
				}
			}

			if(i == (sizeof(header->ih_name) - strlen("#0x8"))){
				kernel_post_load_len_tmp = 0;
			} else {
				kernel_post_load_len_tmp = simple_strtoul(&header->ih_name[i + 3], NULL, 16);//3 ->#0x
				kernel_post_load_len_tmp = kernel_post_load_len_tmp - 0x80000000;
			}

			//printf("kernel_post_load_len_tmp:%x \r\n", kernel_post_load_len_tmp);

			if((kernel_post_load_len_tmp <= 0) || (kernel_post_load_len_tmp > spl_image->size)){
				kernel_post_load_len = spl_image->size;
			}
			else{
				kernel_post_load_len = kernel_post_load_len_tmp;
			}

			//printf("kernel_post_load_len:0x%lx, spl_image->size:0x%x\r\n", kernel_post_load_len, spl_image->size);
			err = spi_flash_read(flash, payload_offs + 1024, kernel_post_load_len - first_load_size,
						(void *)(spl_image->load_addr + first_load_size));
#else
			err = spi_flash_read(flash, payload_offs + 1024, spl_image->size - first_load_size,
						(void *)(spl_image->load_addr + first_load_size));
#endif
			if(spl_image->os == IH_OS_LINUX) {
				//read dtb
				if(get_mtdparts_by_partition_name (PART_DTB_NAME, part_num, &run_add, &run_len) == -1){
					printf("spinor_get_mtdparts DTB failed, and read kernel.\n");
					return -EINVAL;
				}
				payload_offs = run_add;
				//printf("read dtb payload_offs:0x%lx, run_len:0x%lx, fdtcontroladdr:0x%lx\r\n", payload_offs, run_len, fdtcontroladdr);
				err = spi_flash_read(flash, payload_offs, run_len, (void *)fdtcontroladdr);
				if (err) {
					printf("%s: Failed to read dtb from SPI flash (err=%d)\n",
					      __func__, err);
					return err;
				}
				
//#ifndef CONFIG_KERNEL_POST_LOAD
				//partition info to DTB
				of_size = fdt_totalsize((void *)fdtcontroladdr);
				of_len = of_size + CONFIG_SYS_FDT_PAD;
				//printf("%s, line:%d, fdt:0x%x, of_len:0x%x!\n", __func__, __LINE__, fdtcontroladdr, of_len);
				spl_fdt_set_totalsize((void *)fdtcontroladdr, of_len);
				spl_fdt_chosen((void *)fdtcontroladdr, 1, env_buf);
//#endif
			}

		}
	}

	return err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);
