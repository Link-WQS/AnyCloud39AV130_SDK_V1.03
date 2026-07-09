// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 */
#include <common.h>
#include <config.h>
#include <spl.h>
#include <asm/io.h>
#include <nand.h>
#include <linux/libfdt_env.h>
#include <fdt.h>
#include <linux/mtd/mtd.h>

#if CONFIG_ANYKA
#ifndef CONFIG_SYS_FDT_PAD
#define CONFIG_SYS_FDT_PAD 0x3000
#endif

#ifndef ENV_PARTITION_SIZE
#define ENV_PARTITION_SIZE 0x80000
#endif

typedef struct Nand_phy_info{
	u32  chip_id;//chip id
	u16  page_size; //page size
	u16  page_per_blk; //page of one blok
	u16  blk_num;//total block number
	u16  group_blk_num;//the same concept as die, according to nand's struture
	u16  plane_blk_num;
	u8	 spare_size;//spare
	u8	 col_cycle;//column address cycle

	u8	 lst_col_mask;//
	u8	 row_cycle;//
	u8	 delay_cnt;

	u8	 custom_nd;//nand type flag, used to detect the original invilid block
	u32  flag;//
	u32  cmd_len;//nandflash command length
	u32  data_len;//nandflash data length
	u8	 des_str[32];//descriptor string
}T_SPI_NAND_PARAM;


extern int device_probe(struct udevice *dev);
extern int pare_str_form_env(char *buf, char *str, char *out_buf);
extern int spl_device_parse(const char *const mtd_dev, ulong flash_size);
extern int get_mtdparts_by_partition_name (char *partition_name, unsigned long partition_num, unsigned long *run_add, unsigned long *run_len);
extern void spl_fdt_set_totalsize(void *fdt, uint32_t val);
extern int spl_fdt_chosen(void *fdt, int force, char *env_buf);

int nand_read_data(struct mtd_info *mtd, uint32_t start_pos, unsigned int buf_len, unsigned int partition_size,void *buf)
{
	u32 read_len = buf_len;
	u32 write_idex = 0;
	u32 page_idex = 0;
	u8 *tmp_buf = NULL;
	u32 will_read_len = 0;
	u32 buf_idex = 0, b_idex = 0;
	int ret = -1;
	u32 page_size = mtd->writesize;
	u32 erase_size = mtd->erasesize;
	struct mtd_oob_ops ops = {0};


	// printf("start_pos:%d, partition_size:%d, buf_len:%d\n", start_pos, partition_size, buf_len);
	while(1)
	{

		if(read_len == 0)
		{
			break;
		}
		if(page_idex*page_size >= partition_size)
		{
			printf("error: more than partition size, saved_size = %d, write_size:%d\n", partition_size, page_idex*page_size);
			return -1;
		}

		if(mtd->_block_isbad(mtd, start_pos + b_idex*erase_size))
		{
			printf("the block is bad block = %d\n", (start_pos + b_idex*erase_size)/erase_size);
			b_idex++;
			page_idex += (erase_size/page_size);
			if(buf_len == partition_size){
				read_len -= erase_size;
			}
			continue;
		}
// printf("buf_idex:%d, read_len:%d, write_idex:%d, page_idex:%d, %d, %d\r\n", buf_idex, read_len, write_idex, page_idex, erase_size, page_size);
		tmp_buf = buf + write_idex*page_size;
		buf_idex = 0;
		while(1)
		{

			if(read_len > page_size)
			{
				will_read_len = page_size;
			}
			else
			{
				will_read_len = read_len;
			}

			ops.datbuf = &tmp_buf[buf_idex];
			ops.len = will_read_len;
			ops.oobbuf = NULL;
			ops.ooblen = 0;

			// printf("will_read_len:%d, page_pos:%d\r\n", will_read_len, start_pos+page_idex*page_size);
			ret = mtd->_read_oob(mtd, start_pos+page_idex*page_size, &ops);
			if (ret == -1)
			{
				printf("error: spinand_flash_read, pos:%d\n", start_pos+page_idex*page_size);
				return -1;
			}
			page_idex++;
			write_idex++;
			read_len -= will_read_len;
			buf_idex += will_read_len;

			// printf("buf_idex:%d, read_len:%d, write_idex:%d, page_idex:%d, %d, %d\r\n", buf_idex, read_len, write_idex, page_idex, erase_size, page_size);

			if(read_len == 0)
			{
				break;
			}

			if((start_pos+page_idex*page_size)%erase_size == 0)
			{
				// printf("write next block\r\n");
				break;
			}

		}
		b_idex++;
	}
	return 0;
}

static int spl_nand_load_image(struct spl_image_info *spl_image,
			       struct spl_boot_device *bootdev)
{
	int err = -EINVAL;
	struct image_header *header;
	struct mtd_info *mtd;
	char env_buf[CONFIG_ENV_SIZE] = {0};
	char out_buf[2048] = {0};
	//unsigned long load_addr = 0;
	unsigned long fdtcontroladdr = 0;
	unsigned long image_flag = 0;
	unsigned long  run_add = 0;
	unsigned long  run_len = 0;
	unsigned long  part_num = 0;
	ulong	of_size = 0;
	ulong	of_len = 0;
	//char *splbuf = NULL;
	int ret;

    unsigned int    uboot_part_valid = 0, uboot_part_num = 0;
#ifdef PART_UBOOTBK_NAME
    unsigned long   ubootbk_add = 0;
    unsigned long   ubootbk_len = 0;
#endif
	//printf("spl_nand_load_image@@@@@@@@@@@\r\n");
	nand_init();

	mtd  = get_nand_dev_by_index(0);
	//printf("mtd:%x, mtd->size:%d\r\n", mtd, mtd->size);

	//splbuf = (void *)(CONFIG_SPL_TEXT_BASE);
	//if(nand_read_data(mtd, 0, mtd->writebufsize, mtd->writebufsize,  (void *)splbuf))
	//{
	//	puts("spinand_read_partition failed.\n");
	//	hang();
	//}

	//read env

#if defined(CONFIG_TARGET_ANYCLOUD_AK3918AV100) || defined(CONFIG_TARGET_ANYCLOUD_AK3918AV130)
	if(nand_read_data(mtd, CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE, ENV_PARTITION_SIZE, env_buf)){
#else
	if(nand_read_data(mtd, CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE, mtd->erasesize, env_buf)){
#endif
		printf("nand_read_data read env fail\r\n");
		return err;
	}

	//memset(out_buf, 0, 1024);
	//pare_str_form_env(env_buf, "loadaddr=", out_buf);
	//load_addr = simple_strtoul(&out_buf[9], NULL, 16); //getenv_ulong("loadaddr", 16, load_addr);
	//printf("load_addr:%x\n",load_addr);
#ifdef FIXED_FDTCONTROLADDR
    fdtcontroladdr = FIXED_FDTCONTROLADDR;
#else
	memset(out_buf, 0, 2048);
	pare_str_form_env(env_buf, "fdtcontroladdr=", out_buf);
	fdtcontroladdr = simple_strtoul(&out_buf[15], NULL, 16); //getenv_ulong("fdtcontroladdr", 16, fdtcontroladdr);
#endif
	//printf("fdtcontroladdr:%x\n",fdtcontroladdr);
	spl_image->arg = (void *)fdtcontroladdr;
	memset(out_buf, 0, 2048);
	pare_str_form_env(env_buf, "mtdparts=mtdparts=", out_buf);
	part_num = spl_device_parse(&out_buf[9], mtd->size);
	//printf("part_num:%x\n",part_num);

	memset(out_buf, 0, 2048);
	pare_str_form_env(env_buf, "a_uboot_flags=", out_buf);
	image_flag = simple_strtoul(&out_buf[14], NULL, 16); //getenv_ulong("fdtcontroladdr", 16, fdtcontroladdr);

	//printf("image_flag:%x\n",image_flag);
	if(image_flag)
	{
		if(get_mtdparts_by_partition_name (PART_UBOOT_NAME, part_num, &run_add, &run_len) == -1){
			puts("missing UBOOT mtdparts\n");
//			return -EINVAL;
        } else {
            uboot_part_valid |= (0x01UL << 0);
            uboot_part_num++;
        }
#ifdef PART_UBOOTBK_NAME
        if(get_mtdparts_by_partition_name (PART_UBOOTBK_NAME, part_num, &ubootbk_add, &ubootbk_len) == -1){
            puts("missing UBOOTBK mtdparts\n");
//            return -EINVAL;
        } else {
            uboot_part_valid |= (0x01UL << 1);
            uboot_part_num++;
        }
#endif
        if (0 == uboot_part_valid) {
            puts("spinand_get_mtdparts UBOOT and UBOOTBK failed.\n");
            return -EINVAL;
        }
	}else{
		if(get_mtdparts_by_partition_name (PART_BIOS_NAME, part_num, &run_add, &run_len) == -1){
			puts("spinor_get_mtdparts KERNEL failed.\n");
			return -EINVAL;
		}
	}

	// printf("read header:run_add:0x%x, sizeof(*header):0x%x, run_len:0x%x,mtd->writesize:0x%x\r\n", run_add, sizeof(*header), run_len, mtd->writesize);
#ifdef PART_UBOOTBK_NAME
    if (0 == uboot_part_valid) {
#endif
    	header = (struct image_header *)out_buf;
    	if(nand_read_data(mtd, run_add, mtd->writesize, run_len, out_buf)){
    		printf("nand_read_data image headers fail\r\n");
    		return err;
    	}
#ifdef PART_UBOOTBK_NAME
    }
#endif

	if (IS_ENABLED(CONFIG_SPL_LOAD_FIT_SEPARATE) &&
		    image_get_magic(header) == FDT_MAGIC)
	{

		//printf("read data run_add:0x%x len:%d addr:0x%x\n",run_add,roundup(fdt_totalsize(header),4),(CONFIG_SYS_LOAD_ADDR+0x800000));
		if(nand_read_data(mtd, run_add, roundup(fdt_totalsize(header),4), run_len, (void *)(CONFIG_SYS_LOAD_ADDR+0x800000))){
			printf("nand_read_data image data fail\r\n");
			return err;
		}
		printf("nand read success\n");
		spl_parse_image_header(spl_image,(struct image_header *)(CONFIG_SYS_LOAD_ADDR+0x800000));
		if(spl_image->os == IH_OS_LINUX)
		{
			/*read DTB*/
			if(get_mtdparts_by_partition_name (PART_DTB_NAME, part_num, &run_add, &run_len) == -1){
				puts("spinor_get_mtdparts UBOOT failed, and read kernel.\n");
				return -EINVAL;
			}

			if(nand_read_data(mtd, run_add, run_len, run_len, (void *)fdtcontroladdr)){
					printf("nand_read_data dtb data fail\r\n");
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

			spl_fdt_set_totalsize((void *)fdtcontroladdr, of_len);
			spl_fdt_chosen((void *)fdtcontroladdr, 1, env_buf);
		}
   	}
	else
	{
#ifdef PART_UBOOTBK_NAME
        if (0 == uboot_part_valid) {
#endif
            err = spl_parse_image_header(spl_image, header);
            if (err){
                printf("spl_parse_image_header fail\r\n");
                return err;
            }

            // printf("read image:run_add:0x%x, spl_image->size:0x%x, run_len:0x%x, (void *)spl_image->load_addr:0x%x \r\n", run_add, spl_image->size, run_len, (void *)spl_image->load_addr);
            if( (err = nand_read_data(mtd, run_add, spl_image->size, run_len, (void *)spl_image->load_addr)) ){
                printf("nand_read_data image data fail\r\n");
                return err;
            }
#ifdef PART_UBOOTBK_NAME
        } else {

            ulong image_dcrc, image_dcrc_length, image_dcrc_data, dcrc;
            int s32success = 0;
            unsigned long flash_addr, part_len;
            int i;

            for (i = 0; i < 2; ++i)
            {
                if (!(uboot_part_valid & (0x01UL << i))) {
                    continue;
                }

                flash_addr = i == 0 ? run_add : ubootbk_add;
                part_len = i == 0 ? run_len : ubootbk_len;

                header = (struct image_header *)out_buf;
                if (nand_read_data(mtd, flash_addr, mtd->writesize, part_len, out_buf)) {
                    printf("nand_read_data image headers fail\r\n");
                    continue;
                }

                err = spl_parse_image_header(spl_image, header);
                if (err){
                    printf("spl_parse_image_header fail\r\n");
//                    return err;
                    continue;
                }

                //printf("read image:run_add:0x%x, spl_image->size:0x%x, run_len:0x%x, (void *)spl_image->load_addr:0x%x \r\n", run_add, spl_image->size, run_len, (void *)spl_image->load_addr);
                if(err = nand_read_data(mtd, flash_addr, spl_image->size, part_len, (void *)spl_image->load_addr)) {
                    printf("nand_read_data image data fail\r\n");
//                    return err;
                    continue;
                }

                if (uboot_part_num == 1) {
                    s32success = 1;
                    break;
                }

                image_dcrc_data = image_get_load(header);
                image_dcrc_length = image_get_data_size(header);
                image_dcrc = image_get_dcrc(header);

                if (image_dcrc_length < part_len) {
                    dcrc = crc32_wd(0, (unsigned char *)image_dcrc_data,
                                  image_dcrc_length, CHUNKSZ_CRC32);

                    if (image_dcrc == dcrc) {
                        s32success = 1;
                        printf("uboot.img %d success\r\n", i);
                        break;
                    } else {
                        printf("uboot image%d crc. 0x%x,0x%x. (addr,len)0x%x-%d 0x%x-%d\r\n", i, image_dcrc, dcrc,
                                                                        image_dcrc_data, image_dcrc_length,
                                                                        spl_image->load_addr, spl_image->size);
                    }
                }

            }

            if (0 == s32success) {
                printf("nand_read_data image data fail\r\n");
                return -1;
            }

        }
#endif

		if (header->ih_os == IH_OS_LINUX) {
			/*read DTB*/
			if(get_mtdparts_by_partition_name (PART_DTB_NAME, part_num, &run_add, &run_len) == -1){
				puts("spinor_get_mtdparts UBOOT failed, and read kernel.\n");
				return -EINVAL;
			}
			//printf("read dtb:run_add:0x%x, spl_image->size:0x%x, run_len:0x%x, (void *)spl_image->load_addr:0x%x \r\n", run_add, spl_image->size, run_len, (void *)spl_image->load_addr);
			if(nand_read_data(mtd, run_add, run_len, run_len, (void *)fdtcontroladdr)){
				printf("nand_read_data dtb data fail\r\n");
				return err;
			}
			of_size = fdt_totalsize((void *)fdtcontroladdr);
			of_len = of_size + CONFIG_SYS_FDT_PAD;

			//printf("%s, line:%d, fdt:0x%x, of_len:0x%x!\n", __func__, __LINE__, fdtcontroladdr, of_len);
			spl_fdt_set_totalsize((void *)fdtcontroladdr, of_len);

			spl_fdt_chosen((void *)fdtcontroladdr, 1, env_buf);
		} else {
			debug("Trying to start u-boot now...\n");
		}
	}
	return 0;

}
#else
#if defined(CONFIG_SPL_NAND_RAW_ONLY)
static int spl_nand_load_image(struct spl_image_info *spl_image,
			struct spl_boot_device *bootdev)
{
	nand_init();

	printf("Loading U-Boot from 0x%08x (size 0x%08x) to 0x%08x\n",
	       CONFIG_SYS_NAND_U_BOOT_OFFS, CONFIG_SYS_NAND_U_BOOT_SIZE,
	       CONFIG_SYS_NAND_U_BOOT_DST);

	nand_spl_load_image(CONFIG_SYS_NAND_U_BOOT_OFFS,
			    CONFIG_SYS_NAND_U_BOOT_SIZE,
			    (void *)CONFIG_SYS_NAND_U_BOOT_DST);
	spl_set_header_raw_uboot(spl_image);
	nand_deselect();

	return 0;
}
#else

static ulong spl_nand_fit_read(struct spl_load_info *load, ulong offs,
			       ulong size, void *dst)
{
	int ret;

	ret = nand_spl_load_image(offs, size, dst);
	if (!ret)
		return size;
	else
		return 0;
}

static int spl_nand_load_element(struct spl_image_info *spl_image,
				 int offset, struct image_header *header)
{
	int err;

	err = nand_spl_load_image(offset, sizeof(*header), (void *)header);
	if (err)
		return err;

	if (IS_ENABLED(CONFIG_SPL_LOAD_FIT) &&
	    image_get_magic(header) == FDT_MAGIC) {
		struct spl_load_info load;

		debug("Found FIT\n");
		load.dev = NULL;
		load.priv = NULL;
		load.filename = NULL;
		load.bl_len = 1;
		load.read = spl_nand_fit_read;
		return spl_load_simple_fit(spl_image, &load, offset, header);
	} else {
		err = spl_parse_image_header(spl_image, header);
		if (err)
			return err;
		return nand_spl_load_image(offset, spl_image->size,
					   (void *)(ulong)spl_image->load_addr);
	}
}

static int spl_nand_load_image(struct spl_image_info *spl_image,
			       struct spl_boot_device *bootdev)
{
	int err;
	struct image_header *header;
	int *src __attribute__((unused));
	int *dst __attribute__((unused));

#ifdef CONFIG_SPL_NAND_SOFTECC
	debug("spl: nand - using sw ecc\n");
#else
	debug("spl: nand - using hw ecc\n");
#endif
	nand_init();

	header = spl_get_load_buffer(0, sizeof(*header));

#ifdef CONFIG_SPL_OS_BOOT
	if (!spl_start_uboot()) {
		/*
		 * load parameter image
		 * load to temp position since nand_spl_load_image reads
		 * a whole block which is typically larger than
		 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite
		 * following sections like BSS
		 */
		nand_spl_load_image(CONFIG_CMD_SPL_NAND_OFS,
			CONFIG_CMD_SPL_WRITE_SIZE,
			(void *)CONFIG_SYS_TEXT_BASE);
		/* copy to destintion */
		for (dst = (int *)CONFIG_SYS_SPL_ARGS_ADDR,
				src = (int *)CONFIG_SYS_TEXT_BASE;
				src < (int *)(CONFIG_SYS_TEXT_BASE +
				CONFIG_CMD_SPL_WRITE_SIZE);
				src++, dst++) {
			writel(readl(src), dst);
		}

		/* load linux */
		nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
			sizeof(*header), (void *)header);
		err = spl_parse_image_header(spl_image, header);
		if (err)
			return err;
		if (header->ih_os == IH_OS_LINUX) {
			/* happy - was a linux */
			err = nand_spl_load_image(
				CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				spl_image->size,
				(void *)spl_image->load_addr);
			nand_deselect();
			return err;
		} else {
			puts("The Expected Linux image was not "
				"found. Please check your NAND "
				"configuration.\n");
			puts("Trying to start u-boot now...\n");
		}
	}
#endif
#ifdef CONFIG_NAND_ENV_DST
	spl_nand_load_element(spl_image, CONFIG_ENV_OFFSET, header);
#ifdef CONFIG_ENV_OFFSET_REDUND
	spl_nand_load_element(spl_image, CONFIG_ENV_OFFSET_REDUND, header);
#endif
#endif
	/* Load u-boot */
	err = spl_nand_load_element(spl_image, CONFIG_SYS_NAND_U_BOOT_OFFS,
				    header);
#ifdef CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
#if CONFIG_SYS_NAND_U_BOOT_OFFS != CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND
	if (err)
		err = spl_nand_load_element(spl_image,
					    CONFIG_SYS_NAND_U_BOOT_OFFS_REDUND,
					    header);
#endif
#endif
	nand_deselect();
	return err;
}
#endif
#endif
/* Use priorty 1 so that Ubi can override this */
SPL_LOAD_IMAGE_METHOD("NAND", 1, BOOT_DEVICE_NAND, spl_nand_load_image);
