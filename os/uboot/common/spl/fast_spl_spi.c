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

#ifdef CONFIG_3918AV130_CODE
#include <asm/arch-ak3918av130/ak_cpu.h>
#include <asm/io.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#if 0
#define PRINTF(fmt,args...)         printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

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


#if defined(CONFIG_AK_AUTOUPGRADE_JUDGE_PIN)
#define SHARED_PIN_CTRL_REG0                (CHIP_CONF_BASE_ADDR + 0x178)
#define PAD_DRIVE_CFG_REG0                  (CHIP_CONF_BASE_ADDR + 0x1A4)
#define PAD_IE_CFG_REG0                     (CHIP_CONF_BASE_ADDR + 0x1C0)
#define PAD_SLEW_RATE_CFG_REG0              (CHIP_CONF_BASE_ADDR + 0x1D0)
#define PUPD_EN_CONF_REG0                   (CHIP_CONF_BASE_ADDR + 0x264)
#define PUPD_SELECTION_CFG_REG0             (CHIP_CONF_BASE_ADDR + 0x274)

#define AK_PIN_MUX_REG(pin)                 (SHARED_PIN_CTRL_REG0 + (pin / 10) * 4)
#define AK_PIN_MUX_REG_OFFSET(pin)          ((pin % 10) * 3)
#define AK_PIN_MUX_REG_MASK                 (0x3)

#define AK_PIN_DRV_REG(pin)                 (PAD_DRIVE_CFG_REG0 + ((pin) / 16 ) * 4)
#define AK_PIN_DRV_REG_OFFSET(pin)          ((pin % 16) * 2)
#define AK_PIN_DRV_REG_MASK                 (0x3)

#define AK_PIN_IE_REG(pin)                  (PAD_IE_CFG_REG0 + ((pin) / 32) * 4)
#define AK_PIN_IE_REG_OFFSET(pin)           ((pin) % 32 )

#define AK_PIN_SLEW_RATE_REG(pin)           (PAD_SLEW_RATE_CFG_REG0 + ((pin) / 32) * 4)
#define AK_PIN_SLEW_RATE_REG_OFFSET(pin)    ((pin) % 32)

#define AK_PIN_PUPD_EN_REG(pin)             (PUPD_EN_CONF_REG0 + ((pin) / 32) * 4)
#define AK_PIN_PUPD_EN_REG_OFFSET(pin)      ((pin) % 32)

#define AK_PIN_PUPD_SEL_REG(pin)            (PUPD_SELECTION_CFG_REG0 + ((pin) / 32) * 4)
#define AK_PIN_PUPD_SEL_REG_OFFSET(pin)     ((pin) % 32)


#define AK_GPIO_DIR0                (0x20000000 + 0x170000 + 0x00)
#define AK_GPIO_OUT0                (0x20000000 + 0x170000 + 0x14)
#define AK_GPIO_INPUT0              (0x20000000 + 0x170000 + 0x28)

#define AK_GPIO_DIR_BASE(pin)           (((pin) >> 5) * 4 + AK_GPIO_DIR0)
#define AK_GPIO_OUT_BASE(pin)           (((pin) >> 5) * 4 + AK_GPIO_OUT0)
#define AK_GPIO_IN_BASE(pin)            (((pin) >> 5) * 4 + AK_GPIO_INPUT0)

#define AK_GPIO_REG_SHIFT(pin)      ((pin) % 32)

static int is_valid_pin(int pin)
{
    return ((pin >= 0 && pin <= 59) ||
            (pin >= 65 && pin <= 69) ||
            (pin >= 70 && pin <= 75) ||
            (pin >= 78 && pin <= 79) ||
            (pin >= 80 && pin <= 81) ||
            (pin >= 99 && pin <= 104));
}

static int check_autoupgrade_judge_pin(int pin, int active_level, int timeout)
{
    unsigned int mux_reg_val, dir_reg_val, ie_reg_val, pupd_en_reg_val, pupd_sel_reg_val;
    unsigned int regval;
    int active_state = 0;
    int pullup = active_level ? 0 : 1;
    int pin_level;
    unsigned int start_ms;
    unsigned int current_ms;

    //1. 设置为输入
    dir_reg_val = __raw_readl(AK_GPIO_DIR_BASE(pin));
    regval = dir_reg_val;
    regval &= ~(0x1 << AK_GPIO_REG_SHIFT(pin));
    __raw_writel(regval, AK_GPIO_DIR_BASE(pin));

    ie_reg_val = __raw_readl(AK_PIN_IE_REG(pin));
    regval = ie_reg_val;
    regval |= (0x1 << AK_PIN_IE_REG_OFFSET(pin));
    __raw_writel(regval, AK_PIN_IE_REG(pin));

    //2. 设置上下拉
    pupd_sel_reg_val = __raw_readl(AK_PIN_PUPD_SEL_REG(pin));
    regval = pupd_sel_reg_val;
    if (pullup) {
        regval |= (0x1 << AK_PIN_PUPD_SEL_REG_OFFSET(pin));
    } else {
        regval &= ~(0x1 << AK_PIN_PUPD_SEL_REG_OFFSET(pin));
    }
    __raw_writel(regval, AK_PIN_PUPD_SEL_REG(pin));

    pupd_en_reg_val = __raw_readl(AK_PIN_PUPD_EN_REG(pin));
    regval = pupd_en_reg_val;
    regval |= (0x1 << AK_PIN_PUPD_EN_REG_OFFSET(pin));
    __raw_writel(regval, AK_PIN_PUPD_EN_REG(pin));

    //3. 设置为GPIO
    mux_reg_val = __raw_readl(AK_PIN_MUX_REG(pin));
    regval = mux_reg_val;
    regval &= ~(0x7 << AK_PIN_MUX_REG_OFFSET(pin));
    regval |= ((0 & 0x7) << AK_PIN_MUX_REG_OFFSET(pin));
    __raw_writel(regval, AK_PIN_MUX_REG(pin));

    pin_level = (REG32(AK_GPIO_IN_BASE(pin)) & (1 << AK_GPIO_REG_SHIFT(pin))) ? 1 : 0;
    PRINTF("%s: active_level[%d], pin_level[%d], t[%u]\n", __func__, active_level, pin_level);
    if (pin_level == active_level) {
        start_ms = (0xffffffff - REG32(0x20000000 + 0x1110000 + 0x1000 + 0x04)) / 12000;
        do {
            current_ms = (0xffffffff - REG32(0x20000000 + 0x1110000 + 0x1000 + 0x04)) / 12000;
            pin_level = (REG32(AK_GPIO_IN_BASE(pin)) & (1 << AK_GPIO_REG_SHIFT(pin))) ? 1 : 0;
        } while ((pin_level == active_level) && ((current_ms - start_ms) < timeout));

        printf("%s: active_level[%d], pin_level[%d], t[%u]\n", __func__,
                    active_level, pin_level, current_ms - start_ms);

        if (pin_level == active_level)
            active_state = 1;
    }

    //恢复设置
    __raw_writel(mux_reg_val, AK_PIN_MUX_REG(pin));

    __raw_writel(dir_reg_val, AK_GPIO_DIR_BASE(pin));

    __raw_writel(ie_reg_val, AK_PIN_IE_REG(pin));

    __raw_writel(pupd_sel_reg_val, AK_PIN_PUPD_SEL_REG(pin));

    __raw_writel(pupd_en_reg_val, AK_PIN_PUPD_EN_REG(pin));

    return active_state;
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

#if defined(CONFIG_3918AV130_CODE)
static unsigned int check_sum(const void *buf, unsigned int bytes)
{
    const unsigned char *_u8a = (const unsigned char *)buf;
    unsigned int sum = 0;
    unsigned int i;

    for (i = 0; i < bytes; ++i) {
        sum += _u8a[i];
    }
    return sum;
}

static int ak_image_check_hsum(const image_header_t *hdr)
{
    ulong data = (ulong)hdr;
    ulong len = image_get_header_size();
    ulong hsum = check_sum((unsigned char *)(data + 8), len - 8);

//    printf("check_hsum %x %x\n", hsum, hdr->ih_hcrc);

    return (hsum == hdr->ih_hcrc);
}

static int ak_image_check_dsum(const image_header_t *hdr)
{
    ulong data = image_get_data(hdr);
    ulong len = image_get_data_size(hdr);
    ulong dsum = check_sum((unsigned char *)data, len);

//    printf("check_dsum %x %x\n", dsum, hdr->ih_dcrc);

    return (dsum == hdr->ih_dcrc);
}

#endif

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
#if !defined(CONFIG_3918AV130_CODE)
    int use_ramfs = 0;
#endif
    unsigned long fastsys_offset = 0, fastsys_len = 0;
    // ulong load_end;
    uint magic;
    size_t realsize = 0;
    u32 first_load_size;
    unsigned int fastsys_image_load_addr;
    unsigned int fastsys_param_load_addr;
    unsigned int fastsys_ispcfg_load_addr;
    unsigned long cpioaddr = 0;

#ifdef CONFIG_KERNEL_POST_LOAD
    unsigned long kernel_post_load_len = 0;
#endif
    int	ret = 0;
#ifdef CONFIG_KERNEL_POST_LOAD
    int kernel_post_load_len_tmp = 0;
#endif
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
    PRINTF("spl_spi_load_image@@@@@@@@@@@@@@:%x\r\n", CONFIG_SF_DEFAULT_MODE);
    flash = spi_flash_probe(bus, cs,speed, mode);
#endif
    if (!flash) {
        puts("SPI probe failed.\n");
        return -ENODEV;
    }

    PRINTF("flash->size:%d\r\n", flash->size);

    header = spl_get_load_buffer(-sizeof(*header), sizeof(*header));
    PRINTF("spl_get_load_buffer:%x\r\n", header);
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
    PRINTF("load_addr:%x\n",load_addr);
    memset(out_buf, 0, 128);
    pare_str_form_env(env_buf, "fdtcontroladdr=", out_buf);
    fdtcontroladdr = simple_strtoul(&out_buf[15], NULL, 16); //getenv_ulong("fdtcontroladdr", 16, fdtcontroladdr);
    PRINTF("fdtcontroladdr:%x\n",fdtcontroladdr);
    spl_image->arg = (void *)fdtcontroladdr;

    /***********************************************
    * 读分区表mtdparts
    */
    memset(out_buf, 0, 1024);
    pare_str_form_env(env_buf, "mtdparts=mtdparts=", out_buf);
    part_num = spl_device_parse(&out_buf[9], flash->size);
    PRINTF("part_num:%d\n", part_num);

    /***********************************************
    * AUTOUPARDE 逻辑
    * 1. 优先判断 upgrade_flag
    * 2. 判断PIN, 需要做消抖2s
    * 根据条件设置 image_flag
    */
#if defined(CONFIG_AK_AUTOUPGRADE)
    pare_str_form_env(env_buf, "upgrade_flag=", out_buf);
    if (memcmp(out_buf, "upgrade_flag=", 13) == 0) {
        image_flag = simple_strtoul(&out_buf[13], NULL, 16);
        if (image_flag) printf("AK_AUTOUPGRADE %s\n", out_buf);
    }
#if defined(CONFIG_AK_AUTOUPGRADE_JUDGE_PIN)
    PRINTF("is_valid_pin %d %d\n", CONFIG_AK_AUTOUPGRADE_PIN, is_valid_pin(CONFIG_AK_AUTOUPGRADE_PIN));
    if (!image_flag && (is_valid_pin(CONFIG_AK_AUTOUPGRADE_PIN))) {
        image_flag = check_autoupgrade_judge_pin(CONFIG_AK_AUTOUPGRADE_PIN,
                                                    CONFIG_AK_AUTOUPGRADE_PIN_ACTIVE_LEVEL, 2000);
        if (image_flag) printf("AK_AUTOUPGRADE pin[%d]\n", CONFIG_AK_AUTOUPGRADE_PIN);
    }
#endif /*CONFIG_AK_AUTOUPGRADE_JUDGE_PIN*/
#endif /*CONFIG_AK_AUTOUPGRADE*/

    /***********************************************
    * a_uboot_flags. spi nand常电版本一般使用spl->uboot->kernel的逻辑
    */
    if (!image_flag) {
        pare_str_form_env(env_buf, "a_uboot_flags=", out_buf);
        image_flag = simple_strtoul(&out_buf[14], NULL, 16); //getenv_ulong("fdtcontroladdr", 16, fdtcontroladdr);	memset(out_buf, 0, 1024);
    }

    /***********************************************
    * 直接跳到加载uboot逻辑
    */
    if (image_flag)
        goto LOAD_UBOOT;

#if !defined(CONFIG_3918AV130_CODE)
    pare_str_form_env(env_buf, "rootfstype=", out_buf);
    if(out_buf[11] == 'r' && out_buf[12] == 'a' && out_buf[13] == 'm'){
        use_ramfs = 1;
    }
#endif

#ifdef CONFIG_FAST_LAUNCH //cpio file for init ramfs
    pare_str_form_env(env_buf, "cpioaddr=", out_buf);
    cpioaddr = simple_strtoul(&out_buf[9], NULL, 16);
    PRINTF("cpioaddr:%x\n",cpioaddr);
#endif

#if defined(CONFIG_3918AV130_CODE)
    /* read fastsys header */
    /*
     * 加载小核资源
     * 1. 读取环境变量里的分区信息
     * 2. 读取小核分区的到 out_buf
     * 3. header = (struct image_header *)out_buf, 获取header信息
     * 4. 根据header获取小核 load_addr
     * 5. 拷贝 1024 - sizeof(header)数据到 load_addr
     * 6. 在小核load_addr地址偏移0x48 0x4c存放param与isp.conf的加载地址
     * 7. 读取param分区信息, 读取param分区数据到DDR
     * 8. 读取fastsys剩余数据到 load_addr
     * 9. 读取isp.conf分区信息, 读取isp.conf分区数据到DDR
     * 10. 启动小核
     *
     */

    /*
     * 1-读取环境变量里的分区信息
     */
    if(get_mtdparts_by_partition_name (PART_FASTSYS_NAME, part_num, &fastsys_offset, &fastsys_len) == -1){
        puts("spinor_get_mtdparts FASTSYS failed.\n");
        // return -EINVAL;
        goto LOAD_FASTSYS_END;
    }

    /*
     * 2-读取小核分区的到 out_buf
     */
    header = (struct image_header *)out_buf;
    if( (err = spi_flash_read(flash, fastsys_offset, 1024, out_buf)) ){
        printf("spi_flash_read FASTSYS header fail\r\n");
        return err;
    }

    /*
     * 3-header = (struct image_header *)out_buf, 获取header信息
     */
    magic = image_get_magic(header);
    if(magic == FDT_MAGIC){
        realsize = fdt_totalsize(header);
    }
    else if(magic == IH_MAGIC){
        realsize = image_get_data_size(header) + sizeof(image_header_t);
    }
    else{
        printf("fastsys not supported, magic=0x%x\n", magic);
        return -EINVAL;
    }

    /*
     * 4-根据header获取小核 load_addr
     */
    fastsys_image_load_addr = image_get_load(header);
    first_load_size = 1024 - sizeof(struct image_header);

    /*
     * 5-拷贝 1024 - sizeof(header)数据到 load_addr
     */
    memcpy((void *)(unsigned long)fastsys_image_load_addr, out_buf + sizeof(struct image_header), first_load_size);

    /*
     * 6-在小核load_addr地址偏移0x48 0x4c存放param与isp.conf的加载地址
     */
#if defined(CONFIG_3918AV130_CODE)
    fastsys_param_load_addr = *(unsigned int *)((unsigned long)fastsys_image_load_addr + 0x48);
    fastsys_ispcfg_load_addr = *(unsigned int *)((unsigned long)fastsys_image_load_addr + 0x4C);
#else
    fastsys_param_load_addr = *(unsigned int *)((unsigned long)fastsys_image_load_addr + 16);
    fastsys_ispcfg_load_addr = *(unsigned int *)((unsigned long)fastsys_image_load_addr + 20);
#endif

    //debug
//    printf("fastsys_image_load_addr 0x%x\r\n", fastsys_image_load_addr);
//    printf("fastsys_param_load_addr 0x%x\r\n", fastsys_param_load_addr);
//    printf("fastsys_ispcfg_load_addr 0x%x\r\n", fastsys_ispcfg_load_addr);

    /*
     * 7-读取param分区信息, 读取param分区数据到DDR
     */

    /* read fastsys param */
    if (get_mtdparts_by_partition_name (PART_PARAM_NAME, part_num, &run_add, &run_len) == -1){
        puts("spinor_get_mtdparts param failed.\n");
        return -EINVAL;
    }

    if ((err = spi_flash_read(flash, run_add, run_len, (void *)(unsigned long)fastsys_param_load_addr)) ){
        printf("spi_flash_read fastsys param data fail\r\n");
        return err;
    }

#ifdef CONFIG_3918AV130_CODE
    /*
     * AV130 dss参数使用mkimage打包. 校验使用checksum
     */
    header = (struct image_header *)fastsys_param_load_addr;
    if (!ak_image_check_hsum(header) || !ak_image_check_dsum(header)) {
        printf("image_check_dcrc ARGS header fail\r\n");
        return -1;
    }
#endif

#ifndef CONFIG_3918AV130_CODE
    echck_part_md5sum((unsigned char *)(unsigned long)fastsys_param_load_addr, FASTSYS_PARAM_MD5_INDEX);
#endif
    // fastsys_early_init((char *)(unsigned long)fastsys_param_load_addr);
    /*
     * 8-读取fastsys剩余数据到 load_addr
     */
    if ((err = spi_flash_read(flash, fastsys_offset + 1024, realsize - first_load_size,
                        (void *)((unsigned long)fastsys_image_load_addr + first_load_size))) ){
        printf("spi_flash_read kernel data fail\r\n");
        return err;
    }

#ifndef CONFIG_3918AV130_CODE
    echck_part_md5sum((unsigned char *)(unsigned long)fastsys_image_load_addr, FASTSYS_MD5_INDEX);
#endif

    /*
     * 9-读取isp.conf分区信息, 读取isp.conf分区数据到DDR
     *      - 添加header
     */
    /* read ispcfg */
     if(get_mtdparts_by_partition_name (PART_ISPCONF_NAME, part_num, &run_add, &run_len) == -1){
         puts("spinor_get_mtdparts ispconf failed.\n");
         return -EINVAL;
     }

     if( (err = spi_flash_read(flash, run_add, run_len, (void *)(unsigned long)fastsys_ispcfg_load_addr)) ){
         printf("spi_flash_read fastsys ispconf data fail\r\n");
         return err;
     }
     {
         struct isp_ptheader {
             u32 magic;
             u32 hcrc;//从dcrc到isp_partition_header最后一个字节的校验。累加和。
             u32 dcrc;//所有 {headerX+ispX}总的校验。累加和。
             u32 isps_offset[8];//表示每个isp文件的偏移。如第一个是偏移为0，表示header0的开头
             u8 valid_index[8];//用于标识isps_offset[]中哪些属于正在使用的文件。重新匹配后可以修改。制作镜像时相同SensorID的文件挨一起。
             u8 valid_num;//表示isps_offset[]中前几个的数据是有效的。制作镜像时已确定好，系统不可更改。
             u8 reserved[11];//凑够64字节大小
         };
         struct isp_ptheader *isp_ptheader;
         struct image_header *isp_header;
         ulong isp_header_start;
         int i;

         isp_ptheader = (void *)((unsigned long)fastsys_ispcfg_load_addr);
         isp_header_start = (ulong)(fastsys_ispcfg_load_addr + 64);

//         printf("isp_ptheader->magic:%x\r\n",isp_ptheader->magic);
//         printf("isp_ptheader->hcrc:%x\r\n",isp_ptheader->hcrc);
//         printf("isp_ptheader->dcrc:%x\r\n",isp_ptheader->dcrc);
//        for (i = 0; i < 8; i++) {
//            printf("isp_ptheader->isps_offset[i]:%x\r\n",isp_ptheader->isps_offset[i]);
//        }
        for (i = 0; i < 8; i++) {
//            printf("isp_ptheader->valid_index[i]:%x\r\n", isp_ptheader->valid_index[i]);
            if (isp_ptheader->valid_index[i]) {
                isp_header = (struct image_header *)(isp_header_start + isp_ptheader->isps_offset[i]);
//                if (!ak_image_check_hsum(isp_header) || !ak_image_check_dsum(isp_header)) {
                if (!ak_image_check_hsum(isp_header)) {
                    printf("isp[%d].conf checksum failure\r\n", i);
//                    return -1;
                }
            }
        }
//         printf("isp_ptheader->valid_num:%x\r\n",isp_ptheader->valid_num);

//         printf("isp_header->ih_magic:%x\r\n",isp_header->ih_magic);
//         printf("isp_header->ih_hcrc:%x\r\n",isp_header->ih_hcrc);
//         printf("isp_header->ih_time:%x\r\n",isp_header->ih_time);
//         printf("isp_header->ih_size:%x\r\n",isp_header->ih_size);
//         printf("isp_header->ih_dcrc:%x\r\n",isp_header->ih_dcrc);
//         printf("isp_header->ih_os:%x\r\n",isp_header->ih_os);
//         printf("isp_header->ih_arch:%x\r\n",isp_header->ih_arch);
//         printf("isp_header->ih_type:%x\r\n",isp_header->ih_type);
//         printf("isp_header->ih_comp:%x\r\n",isp_header->ih_comp);
//         printf("isp_header->ih_name:%s\r\n",isp_header->ih_name);

     }



#ifndef CONFIG_3918AV130_CODE
    echck_part_md5sum(fastsys_ispcfg_load_addr, run_len, FASTSYS_ISPCFG_MD5_INDEX);
#endif

    /*
     * 10-启动小核
     */

    // fastsys_after_init();
    // fastsys_mem_info = (struct fastsys_ipi_mem_info *)(unsigned long)(*(unsigned int *)((unsigned long)fastsys_image_load_addr + 24));

#if defined(CONFIG_3918AV130_CODE)
    extern void run_slave_cpu(u32 run_addr, u32 size);
    run_slave_cpu(fastsys_image_load_addr, 0x300000);
#else
    /*start second hart. from ROMCODE jump to fastsys*/
    (*(volatile unsigned long*)(0xd28027fc)) = fastsys_image_load_addr;
    (*(volatile unsigned long*)(0x2000004)) = 1;
#endif

LOAD_FASTSYS_END:
#endif

#ifdef CONFIG_KERNEL_POST_LOAD
    memset(out_buf, 0, 1024);
    pare_str_form_env(env_buf, "kernel_post_len=", out_buf);
    kernel_post_load_len = simple_strtoul(&out_buf[16], NULL, 16); //getenv_ulong("fdtcontroladdr", 16, fdtcontroladdr);
#endif

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

LOAD_UBOOT:
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
        //printf("spi_flash_read, payload_offs:0x%lx\r\n", payload_offs);
        /* Load u-boot, mkimage header is 64 bytes. */
        header = (struct image_header *)out_buf;
        err = spi_flash_read(flash, payload_offs, sizeof(*header),
                (void *)header);
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
            err = spl_parse_image_header(spl_image, header);
            if (err)
                return err;

            //printf("read payload_offs:0x%lx, spl_image->size:%lx, spl_image->load_addr:0x%lx\r\n", payload_offs, spl_image->size, spl_image->load_addr);
#ifdef CONFIG_KERNEL_POST_LOAD
            for(i = 0; i < sizeof(header->ih_name) - strlen("#0x8"); i++){
                if(memcmp(&header->ih_name[i], "#0x8", strlen("#0x8")) == 0){
                    break;
                }
            }

            if(i == (sizeof(header->ih_name) - strlen("#0x8"))){
                kernel_post_load_len_tmp = 0;
            }

            kernel_post_load_len_tmp = simple_strtoul(&header->ih_name[i + 3], NULL, 16);//3 ->#0x
            kernel_post_load_len_tmp = kernel_post_load_len_tmp - 0x80000000;

            //printf("kernel_post_load_len_tmp:%x \r\n", kernel_post_load_len_tmp);

            if((kernel_post_load_len_tmp <= 0) || (kernel_post_load_len_tmp > spl_image->size)){
                if((kernel_post_load_len > spl_image->size) || (kernel_post_load_len == 0))
                    kernel_post_load_len = spl_image->size;
            }
            else{
                kernel_post_load_len = kernel_post_load_len_tmp;
            }

            //printf("kernel_post_load_len:0x%lx, spl_image->size:0x%x\r\n", kernel_post_load_len, spl_image->size);
            err = spi_flash_read(flash, payload_offs, kernel_post_load_len,(void *)spl_image->load_addr);
#else
            err = spi_flash_read(flash, payload_offs, spl_image->size,(void *)spl_image->load_addr);
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
#ifdef CONFIG_FAST_LAUNCH

#if !defined(CONFIG_3918AV130_CODE)

#define PRELOAD_LEN  ((96+4)*1024)  //ISP_CONFIG/2 + ae_awb_table.bin

                if (!use_ramfs)
                    goto LOAD_DTB;
                void *app_addr = NULL;
                //read app2 info
                if(get_mtdparts_by_partition_name ("APP2", part_num, &run_add, &run_len) == -1){
                    printf("spinor_get_mtdparts APP2 failed, and read kernel.\n");
                    return -EINVAL;
                }

                //read isp conf
                payload_offs = run_add;
                run_len = PRELOAD_LEN; //add 4K for ae data
                app_addr = (void *)(fdtcontroladdr-0x100000); //default use fdt prev 1M addr area is safe
                //printf("read APP2 payload_offs:0x%lx, run_len:0x%lx, app_addr:0x%lx\r\n", payload_offs, run_len, app_addr);
                err = spi_flash_read(flash, payload_offs, run_len, (void *)app_addr);
                if (err) {
                    printf("read ispconf (err=%d)\r\n",err);
                    return err;
                }

#if 1	//preload -initramfs part
                char *file_data= (char *)app_addr; //(fdtcontroladdr-0x100000);
                int i = 0; //offset

                //boot loading fastfs partition, for fast3.fs, fast4.fs
                {
                    //read fast fs real size
                    i = 0;
                    while(i<12) //tag must be begin char
                    {
                        if( *(file_data+i) == 'F' && *(file_data+i+1) == 0x0a ) //fast3.fs or fast4.fs size 'F' mark
                        {
                            i += 2;
                            break;
                        }
                        i++;
                    }

                    if(i<12)
                    {
                        //parse list file size
                        run_len = 0;
                        if( (*(file_data+i) - 0x30)>=0 && (*(file_data+i) - 0x30)<=9 )
                        {
                            for(; *(file_data+i) != 0x0a && i<=22; i++) //size max < 1000000000
                            {
                                run_len = run_len*10 + (*(file_data+i) - 0x30);
                            }
                            i++;//skip 0x0a

                            if(i>22)
                                printf("i:%d,run_len=%lu\r\n",i,run_len);

                            run_len += i;//fs size + offset
                            if(run_len < (6*1024* 1024))
                            {
                                app_addr = (void *)(0x83A00000);//constant addr as initrd_mem
                                printf("offs:%d,run_len:%d\n",payload_offs,run_len);
                                //cp readed data to begin addr
                                memcpy(app_addr, (void *)(fdtcontroladdr-0x100000), PRELOAD_LEN);
                                payload_offs += PRELOAD_LEN;
                                app_addr += PRELOAD_LEN;
                                run_len -= PRELOAD_LEN;
                                err = spi_flash_read(flash, payload_offs, run_len, (void *)app_addr);
                                if (err) {
                                    printf("%s: Failed to read fast fs (err=%d)\n",__func__, err);
                                    return err;
                                }
                            }
                            else
                            {
                                printf("fs too big:%lu\n",run_len);
                            }
                        }
                        else //fastfs size lost warning!!!
                        {
                            printf("i=%d,not found fastfs_size:%02x %02x %02x %02x\n",i,*(file_data+i+0),*(file_data+i+1),*(file_data+i+2),*(file_data+i+3));
                        }
                    }
                    else //normal, allow there don't load full fastfs
                    {
                        //printf("not found load fastfs flag:%d\n",i);
                        i=0;
                    }
                }

                //ramfs init file load
                if( ((cpioaddr>>24)&0xfc)==0x80 ) //cpioaddr>0x80000000 && cpioaddr<0x84000000
                {
                    if(get_mtdparts_by_partition_name ("ROOTFS", part_num, &run_add, &run_len) == -1){
                        printf("spinor_get_mtdparts ROOTFS failed\n");
                        return -EINVAL;
                    }
                    payload_offs = run_add;
                    //set cpio file load addr
                    app_addr = (void *)cpioaddr;
                    //printf("read rootfs payload_offs:0x%lx, run_len:0x%lx, initrd:0x%lx\r\n", payload_offs, run_len, app_addr);
                    //read cpiofs real size
                    i = 0; //reset search begin position
                    while(i<32)
                    {
                        if( *(file_data+i) == 0x53 && *(file_data+i+1) == 0x0a ) //fastfs start tag 'S' mark
                        {
                            i += 2;
                            break;
                        }
                        i++;
                    }

                    if(i<32)
                    {
                        //parse list file size
                        run_len = 0;
                        if( *(file_data+i)>=0x30 && *(file_data+i)<=0x39 )
                        {
                            for(; *(file_data+i) != 0x0a; i++)
                            {
                                run_len = run_len*10 + (*(file_data+i) - 0x30);
                            }
                            i++;//skip 0x0a
                            //printf("real_len:%d\r\n",run_len);
                            run_len += 16;//reserved byte FFFF avoid cpio data parse error
                            err = spi_flash_read(flash, payload_offs, run_len, (void *)app_addr);
                            if (err) {
                                printf("%s: Failed to read dtb from SPI flash (err=%d)\n",
                                        __func__, err);
                                return err;
                            }

                        }
                        else //initfs size lost warning!!!
                        {
                            printf("i=%d,not found initfs_size:%02x %02x %02x %02x\n",i,*(file_data+i+0),*(file_data+i+1),*(file_data+i+2),*(file_data+i+3));
                        }
                    }
                    else //fast error warning!!!
                    {
                        printf("not found fastfs:%02x %02x %02x %02x\n",*(file_data+0),*(file_data+1),*(file_data+2),*(file_data+3));
                    }
                }
#endif

                //for aov share area
                char *entry_addr   = (char *)((0x81330000-4));          //share mem
                if( *entry_addr == 0x10 )//first power on, reset share status
                {
                    memset((void *)(0x81330000-0x40), 0xff, 0x40);
                }

#else
                //AV130 加载 initfs.cpio image

                if(get_mtdparts_by_partition_name ("ROOTFS", part_num, &run_add, &run_len) == -1){
                    puts("spinor_get_mtdparts INITFS failed, and read kernel.\n");
                    return -EINVAL;
                }

                header = (struct image_header *)out_buf;
                if( (err = spi_flash_read(flash, run_add, sizeof(image_header_t), out_buf)) ){
                    printf("nand_read_data INITFS data fail\r\n");
                    return err;
                }

                magic = image_get_magic(header);
//                if (magic == FDT_MAGIC) {
//                    realsize = fdt_totalsize(header);
//                }
//                else
                if (magic == IH_MAGIC) {
//                    realsize = image_get_data_size(header) + sizeof(image_header_t);
                    realsize = image_get_data_size(header);

                    PRINTF("read initfs:run_add:0x%lx, realsize:0x%lx\r\n", run_add, realsize);
                    if ((err = spi_flash_read(flash, run_add + sizeof(image_header_t), realsize, (u_char *)(cpioaddr)))){
                        printf("nand_read_data opensbi data fail\r\n");
                        return err;
                    }
                } else {
                    PRINTF("INITFS not supported, magic=0x%x, part->addr[%lx], size[%x]\n", magic, run_add, realsize);
                    /*romfs does not have magic*/
                }

#endif

#endif

#if !defined(CONFIG_3918AV130_CODE)
LOAD_DTB:
#endif
                //partition info to DTB
                of_size = fdt_totalsize((void *)fdtcontroladdr);
                of_len = of_size + CONFIG_SYS_FDT_PAD;
                //printf("%s, line:%d, fdt:0x%x, of_len:0x%x!\n", __func__, __LINE__, fdtcontroladdr, of_len);
                spl_fdt_set_totalsize((void *)fdtcontroladdr, of_len);
                spl_fdt_chosen((void *)fdtcontroladdr, 1, env_buf);
            }

        }
    }

    return err;
}
/* Use priorty 1 so that boards can override this */
SPL_LOAD_IMAGE_METHOD("SPI", 1, BOOT_DEVICE_SPI, spl_spi_load_image);
