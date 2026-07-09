/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-06-27 15:30:04
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-09-26 16:34:49
 * @FilePath: /uboot/common/custom/upgrade_file_load.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <fs.h>
#include <fat.h>
#include <common.h>
#include <spi_flash.h>
#define UPGRADE_BASE_ADDR "0x80008000"
#define DEFAULT_UPGRADE_DEVICE "nor"
#define TFTP_SERVER_CONFIG "tftpboot.conf"

#ifdef CONFIG_3918AV100_CODE
#define UPGRADE_NAME "SAT_ANYKA_AV100.IMG"
#elif defined CONFIG_37_E_CODE
#define UPGRADE_NAME "SAT_ANYKA_37E.IMG"
#else
#define UPGRADE_NAME "SAT_ANYKA.IMG"
#endif

int do_tftpb(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]);

static void prepare_tftp_load_buffer(char *loadaddr)
{
    struct spi_flash *flash;
    unsigned long addr;
    unsigned long clear_size;
    char cmd[128] = {0};
    int ret;

    if (!loadaddr)
    {
        printf("[%s] skip memory prepare: loadaddr is NULL\n", __func__);
        return;
    }

    flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
        CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
    if (!flash)
    {
        printf("[%s] skip memory prepare: spi flash probe failed\n", __func__);
        return;
    }

    addr = simple_strtoul(loadaddr, NULL, 16);
    clear_size = (unsigned long)flash->size;
    printf("[%s] clear load buffer start=0x%08lx size=0x%08lx(%lu) fill=0xFF\n",
        __func__, addr, clear_size, clear_size);
    snprintf(cmd, sizeof(cmd), "mw.b 0x%lx 0xFF 0x%lx", addr, clear_size);
    printf("[%s] clear load buffer cmd=%s\n", __func__, cmd);
    ret = run_command(cmd, 0);
    printf("[%s] clear load buffer ret=%d start=0x%08lx end=0x%08lx\n",
        __func__, ret, addr, addr + clear_size);
    spi_flash_free(flash);
}

static void fat32_file_load_memory(char *dev, char *part, char *addr, int size)
{
    char hex_str[32] = {0};
    sprintf(hex_str, "0x%x", size);

    char *arg[7] = {0};
    arg[1] = dev;
    arg[2] = part;
    arg[3] = addr;
    arg[4] = UPGRADE_NAME;
    arg[5] = hex_str;
    if (do_load(NULL, 0, sizeof(arg) / sizeof(char *), arg, FS_TYPE_FAT))
    {
        printf("\033[33m [%s]:[%d],do_load %s fail\033[0m\n", __func__, __LINE__, arg[0]);
    }
}

static loff_t fat32_file_size(char *dev, char *part)
{
    if (fs_set_blk_dev(dev, part, FS_TYPE_FAT))
        return -1;

    loff_t actread;
    int ret;
    ret = fat_read_file(UPGRADE_NAME, NULL, 0, 0, &actread);
    if (ret == -1)
    {
        printf("** Unable to read upgrade file %s **\n", UPGRADE_NAME);
        return -1;
    }
    printf("[%s]Get %s file size:%lld\n", __func__, UPGRADE_NAME, actread);
    return actread;
}

loff_t fat32_upgrade_check(char **dev, char **part, char **file_name)
{
    loff_t size = -1;
    int dev_index = 0;
    char *dev_part[][2] = {{DEFAULT_UPGRADE_DEVICE, "0:0"}, {"mmc", "0"}};
    for (dev_index = 0; dev_index < sizeof(dev_part) / sizeof(dev_part[0]); dev_index++)
    {
        if ((size = fat32_file_size(dev_part[dev_index][0], dev_part[dev_index][1])) >= 0)
        {
            break;
        }
    }

    if (size < 0)
        return -1;

    char *loadaddr = env_get("loadaddr");
    if (!loadaddr)
        return -1;

    *file_name = UPGRADE_NAME;
    *dev = dev_part[dev_index][0];
    *part = dev_part[dev_index][1];

    char cmd[128] = {0};
    sprintf(cmd, "mw.b %s 0x0 0x%llx", loadaddr, size + 1);
    run_command(cmd, 0);

    fat32_file_load_memory(dev_part[dev_index][0], dev_part[dev_index][1], loadaddr, size);

    return size;
}

static int get_conf_info(char *headr, const char *dest, char *info)
{
    char *enter = NULL;
    while (1)
    {
        headr = strstr(headr, dest);
        if (headr == NULL)
        {
            return 0;
        }

        headr += strlen(dest);
        if (headr)
        {
            enter = strchr(headr, '\n');
            if (enter)
                memcpy(info, headr, enter - headr);
            else
                memcpy(info, headr, strlen(headr));
            return 1;
        }
    }
    return 1;
}

static int fat32_partition_detection_tftpboot(char *buf, int buf_len)
{
    if (fs_set_blk_dev(DEFAULT_UPGRADE_DEVICE, "0:0", FS_TYPE_FAT))
        return -1;

    int ret;
    loff_t actread;
    ret = fat_read_file(TFTP_SERVER_CONFIG, buf, 0, buf_len - 1, &actread);
    if (ret < 0)
    {
        return -1;
    }
    return 0;
}

static int jffs2_partition_detection_tftpboot(char *buf, int buf_len)
{
#define DEFAULT_JFFS2_PART_NAME "CONFIG"
    char cmd[64];
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "fsload %s 0x%lx %s", DEFAULT_JFFS2_PART_NAME, (ulong)buf, TFTP_SERVER_CONFIG);
    return run_command(cmd, 0) == 0 ? 0 : -1;
}

static __maybe_unused int tftpboot_service_addr_check(void)
{
    char buf[128];
    memset(buf, 0, sizeof(buf));

    if (jffs2_partition_detection_tftpboot(buf, sizeof(buf)) == -1)
    {
        if (fat32_partition_detection_tftpboot(buf, sizeof(buf)) == -1)
        {
            return -1;
        }
        fs_unlink(TFTP_SERVER_CONFIG); /* 删除文件 */
    }

    char serverip[16] = {0};
    char bootfile[32] = {0};
    int is_tftpfile_valid = 0;
    if (get_conf_info(buf, "serverip=", serverip))
    {
        is_tftpfile_valid = 1;
        printf("%s serverip:%s\n", TFTP_SERVER_CONFIG, serverip);
        env_set("serverip", serverip);
    }
    if (get_conf_info(buf, "bootfile=", bootfile))
    {
        is_tftpfile_valid = 1;
        printf("%s bootfile:%s\n", TFTP_SERVER_CONFIG, bootfile);
        env_set("bootfile", bootfile);
    }

    if (is_tftpfile_valid)
    {
        env_save();
        return 0;
    }
    return -1;
}

loff_t tftpboot_upgrade_check(char **dev, char **part, char **file_name)
{
    // if (tftpboot_service_addr_check() == 0)
    // {
    //     *dev = DEFAULT_UPGRADE_DEVICE;
    //     *part = "0:0";
    //     *file_name = TFTP_SERVER_CONFIG;
    // }

    net_server_ip = string_to_ip(env_get("serverip"));

    char *loadaddr = env_get("loadaddr");
    char *bootfile = env_get("bootfile");
    char serverip[32] = {0};
    char tftp_target[128] = {0};
    char *argv[4] = {0};

    if (!loadaddr || !bootfile || net_server_ip.s_addr == 0)
        return 0;

    ip_to_string(net_server_ip, serverip);
    snprintf(tftp_target, sizeof(tftp_target), "%s:%s", serverip, bootfile);

    printf("[%s] tftp prepare loadaddr=%s bootfile=%s serverip=%s\n",
        __func__, loadaddr, bootfile, serverip);
    prepare_tftp_load_buffer(loadaddr);

    argv[0] = "tftpboot";
    argv[1] = loadaddr;
    argv[2] = tftp_target;

    printf("[%s] start tftpboot target=%s addr=%s\n",
        __func__, tftp_target, loadaddr);
    loff_t size = do_tftpb(NULL, 0, 3, argv);

    if (!size)
    {
        char *filesize = env_get("filesize");
        size = simple_strtoul(filesize, NULL, 16);
        printf("[%s] tftp done ret=%lld fallback_filesize=%s parsed_size=%lld\n",
            __func__, (long long)0, filesize ? filesize : "NULL", (long long)size);
    }
    else
    {
        printf("[%s] tftp done size=%lld\n", __func__, (long long)size);
    }
    return size < 0 || size == 1 ? 0 : size;
}

int del_fat32_file(char *dev, char *part, char *file_name)
{
    if (fs_set_blk_dev(dev, part, FS_TYPE_FAT))
        return 1;

    if (fs_unlink(file_name))
        return 1;

    return 0;
}
