#include <common.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <memalign.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <part.h>
#include <usb.h>
#include <config.h>
#include <errno.h>
#include <env.h>
#include <mapmem.h>
#include <fat.h>
#include <fs.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <bmp_layout.h>
#include <fdt_support.h>

#define COMMAND_BUFLEN (256)

__attribute__((unused))
static int _update(const char *loadaddr_str, const char *filename_str,
    const char *sf_offset, const char *sf_size)
{
    char cmd[COMMAND_BUFLEN];
    int ret;
    loff_t bmp_size = 0;

    /* check exist */
    if(fs_set_blk_dev("usb", "0", FS_TYPE_FAT))
        return 0;
    ret = fs_size(filename_str, &bmp_size);
    if(ret){
        //printf("found file=%s failed(ret=%d)\n", filename_str, ret);
        return -1;
    }
    //printf("find file=%s ok, size=%lld\n", filename_str, bmp_size);
    printf("%s, load file=%s into sf part offset=%s, sf part size=%s\n", __func__,
        filename_str, sf_offset, sf_size);
    
    /* load */
    memset(cmd, 0, COMMAND_BUFLEN);
    snprintf(cmd, COMMAND_BUFLEN, "fatload usb 0 %s %s", loadaddr_str, filename_str);
    //printf("%s run {%s}\n", __func__, cmd);
    run_command(cmd, 0);

    printf("load file=%s, ${filesize}=%s\n", filename_str, env_get("filesize"));

    /*sf probe */
    memset(cmd, 0, COMMAND_BUFLEN);
    snprintf(cmd, COMMAND_BUFLEN, "sf probe");
    run_command(cmd, 0);

    /* sf update */
    memset(cmd, 0, COMMAND_BUFLEN);
    snprintf(cmd, COMMAND_BUFLEN, "sf update %s %s ${filesize}",
        loadaddr_str, sf_offset);
    //printf("%s run {%s}\n", __func__, cmd);
    run_command(cmd, 0);

    return 0;
}

struct update_obj{
    char *env_filename;
    char *env_sf_offset;
    char *env_sf_size;
};

static const struct update_obj parts[] ={
    {"usbupdate_name_uboot",    "usbupdate_sf_offset_uboot",    "usbupdate_sf_size_uboot"},
    {"usbupdate_name_env",      "usbupdate_sf_offset_env",      "usbupdate_sf_size_env"},
    {"usbupdate_name_envbak",   "usbupdate_sf_offset_envbak",   "usbupdate_sf_size_envbak"},
    {"usbupdate_name_dtb",      "usbupdate_sf_offset_dtb",      "usbupdate_sf_size_dtb"},
    {"usbupdate_name_image",    "usbupdate_sf_offset_image",    "usbupdate_sf_size_image"},
    {"usbupdate_name_config",   "usbupdate_sf_offset_config",   "usbupdate_sf_size_config"},
    {"usbupdate_name_app",      "usbupdate_sf_offset_app",      "usbupdate_sf_size_app"},
};

static int do_usbupdate(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    const char *loadaddr_str;
    int parts_num, i;
    char file_path[256];

    /* get loadaddr */
    loadaddr_str = env_get("loadaddr");
    if(loadaddr_str == NULL){
        printf("%s, no found env{loadaddr}\n", __func__);
        return 0;
    }

    parts_num = sizeof(parts)/sizeof(struct update_obj);
    //printf("%s parts_num=%d, {loadaddr}=%s\n", __func__, parts_num, loadaddr_str);
    for(i = 0; i < parts_num; i++){
        snprintf(file_path, 256, "/%s", env_get(parts[i].env_filename));
        _update(loadaddr_str, (const char *)file_path,
            env_get(parts[i].env_sf_offset), env_get(parts[i].env_sf_size));
    }

    return 0;
}

/* usbupdate zImage */
U_BOOT_CMD(
	usbupdate,   1,  1,  do_usbupdate,
	"update from usb disk",
    "usbupdate"
);

