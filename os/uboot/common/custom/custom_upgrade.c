#include <fs.h>
#include <common.h>
#include <env.h>

DECLARE_GLOBAL_DATA_PTR;

#define UPGRADE_INFO_PATH "# File Parttion: "
#define UPGRADE_VERSION_PATH "#<upgrade_bin_version="
#define UPGRADE_IMAGE_BASE_PATH "# <- this is end of image parttion\n"
#define UPGRADE_HEADER_PARSE_SIZE 4096

loff_t tftpboot_upgrade_check(char **dev, char **part, char **file_name);

typedef struct
{
    char name[32];
    unsigned long fileoffset;
    unsigned long partitionoffset;
    unsigned long partitionsize;
    unsigned long filesize;
} upgrade_info;

typedef enum
{
    NONE_MODE,
    TFTP_BOOT,
} upgrade_mode;

struct
{
    char *file_name;
    upgrade_info info[12];
    char *buffer;
    char *dev;
    char *part;
    int total;
    upgrade_mode mode;
    unsigned long upgrade_image_size;
    unsigned long upgrade_bin_size;
    unsigned long upgrade_offset_base;
    char upgrade_version[32];
} upgrade = {.total = 0};

static const char *strrstr(const char *start_str, const char *end_str, const char *dest_str)
{
    const char *p1 = start_str;
    const char *p2 = NULL;
    while (1)
    {
        p1 = strstr(p1, dest_str);
        if (p1 == NULL)
        {
            break;
        }
        if (p1 < end_str)
        {
            p2 = p1 = p1 + strlen(dest_str);
        }
        else
        {
            break;
        }
    }
    return p2;
}

static int upgrade_version_check(const char *sour)
{
    char *p = NULL;
    p = env_get("uprade_image_version");

    if (p)
    {
        printf("Current Version:%s\n", p);
    }

    char version[128] = {0};
    memcpy(version, sour, sizeof(version) - 1);
    version[127] = '\0';
    // printf("[%s]version:%s\n", __func__, version);
    char *start_str = strstr(version, UPGRADE_VERSION_PATH);
    if (start_str == NULL || !(start_str += strlen(UPGRADE_VERSION_PATH)))
    {
        printf("[%s]:[%d] Version Check Fail\n", __func__, __LINE__);
        return 0;
    }

    char *end_str = strstr(start_str, ">");
    if (end_str == NULL)
    {
        printf("[%s]:[%d] Version Check Fail\n", __func__, __LINE__);
        return 0;
    }

    int len = end_str - start_str;
    if (len > sizeof(upgrade.upgrade_version))
    {
        printf("[%s]:[%d] Version Check Fail\n", __func__, __LINE__);
        return 0;
    }

    memcpy(upgrade.upgrade_version, start_str, len);
    if (p != NULL)
    {
        if (strncmp(p, upgrade.upgrade_version, strlen(upgrade.upgrade_version)) == 0)
        {
            printf("[%s]:[%d] Version consistency %s\n", __func__, __LINE__, upgrade.upgrade_version);
            return 0;
        }
        printf("[%s]:[%d]upgrade_version:%s,env version:%s\n", __func__, __LINE__, upgrade.upgrade_version, p);
    }
    printf("[%s]:[%d] Version Check Succeed\n", __func__, __LINE__);
    return 1;
}

static int upgrade_file_info_pares(const char *sour, upgrade_info *info_group, int *total, int info_max)
{
    if (info_max == 0)
    {
        return 0;
    }
#define SPLIT_SYMBOL ", "
    memset(info_group, 0, sizeof(upgrade_info));
    char *start_str = strstr(sour, UPGRADE_INFO_PATH);

    if (start_str == NULL || !(start_str += strlen(UPGRADE_INFO_PATH)))
    {
        return 0;
    }

    char *end_str = strstr(start_str, SPLIT_SYMBOL);
    if (end_str == NULL)
    {
        return 0;
    }
    memset(info_group->name, 0, sizeof(info_group->name));
    memcpy(info_group->name, start_str, end_str - start_str);

    start_str = end_str + strlen(SPLIT_SYMBOL);
    if (start_str == NULL)
        return 0;

    info_group->fileoffset = simple_strtoul(start_str, &start_str, 10);

    start_str += strlen(SPLIT_SYMBOL);
    if (start_str == NULL)
        return 0;

    info_group->filesize = simple_strtoul(start_str, &start_str, 10);
    (*total)++;

    return upgrade_file_info_pares(start_str, info_group + 1, total, --info_max);
}

static int upgrade_image_base_offset(const char *sour)
{
    char *base = strstr(sour, UPGRADE_IMAGE_BASE_PATH);
    int base_offset;
    if (base == NULL || !(base += strlen(UPGRADE_IMAGE_BASE_PATH)))
    {
        printf("\033[33m not found upgrade_base!!!\033[0m\n\n");
        return -1;
    }
    base_offset = base - sour;

    upgrade.upgrade_offset_base = (unsigned long)upgrade.buffer + base_offset;

    upgrade.upgrade_bin_size = upgrade.upgrade_image_size - base_offset;
    printf("[%s] image_base_offset=%d loadaddr=0x%08lx upgrade_offset_base=0x%08lx upgrade_image_size=%lu upgrade_bin_size=%lu\n",
        __func__, base_offset, (unsigned long)upgrade.buffer,
        upgrade.upgrade_offset_base, upgrade.upgrade_image_size,
        upgrade.upgrade_bin_size);
    return base_offset;
}

static int upgrade_env_img_pares(const char *sour)
{
    int i;
    const char *p = NULL;
    for (i = 0; i < upgrade.total; i++)
    {
        if (strncmp(upgrade.info[i].name, "ENV", 3) == 0)
        {
            int count = 0;
            char *env_addr = (char *)(upgrade.upgrade_offset_base + upgrade.info[i].fileoffset);
            while (count != 60)
            {
                count++;
                if ((p = strstr(env_addr, "mtdparts=")))
                {
                    printf("[%s] Env is found after %d searches!!!!!!\n", __func__, count);
                    goto ENV_PARES;
                }
                env_addr += strlen(env_addr) + 1;
            }
            break;
        }
    }

    p = env_get("mtdparts");
    if (p == NULL)
    {
        return 0;
    }
    printf("[%s] NULL,Use Default Env mtdparts!!!!!!\n", __func__);

ENV_PARES:

    for (i = 0; i < upgrade.total; i++)
    {
        if (upgrade.info[i].filesize != 0)
        {
            char *name = strstr(p, upgrade.info[i].name);
            if (name != NULL)
            {
                const char *parttion_size = strrstr(p, name, ",");

                if (parttion_size == NULL)
                {
                    parttion_size = strrstr(p, name, ":");
                    if (parttion_size == NULL)
                    {
                        break;
                    }
                }

                const char *parttion_offset = strrstr(p, name, "@");
                if (!parttion_offset)
                {
                    break;
                }
                upgrade.info[i].partitionsize = simple_strtoul(parttion_size, NULL, 10) * 1024;
                upgrade.info[i].partitionoffset = simple_strtoul(parttion_offset, NULL, 16);
                printf("[%s] part=%s file_offset=%lu file_size=%lu partition_offset=0x%lx partition_size=0x%lx(%lu)\n",
                    __func__, upgrade.info[i].name, upgrade.info[i].fileoffset,
                    upgrade.info[i].filesize, upgrade.info[i].partitionoffset,
                    upgrade.info[i].partitionsize, upgrade.info[i].partitionsize);
            }
        }
    }
    return 1;
}

static int upgrade_img_head_pares(const char *sour)
{
    char buffer[UPGRADE_HEADER_PARSE_SIZE] = {0};
    memcpy(buffer, sour, sizeof(buffer) - 1);

    upgrade_file_info_pares(buffer, &upgrade.info[0], &upgrade.total, sizeof(upgrade.info) / sizeof(upgrade_info));

    upgrade_image_base_offset(buffer);

    upgrade_env_img_pares(buffer);
    return 0;
}

static int update_flash_partition(void)
{
    int i;
    unsigned long bin_size = 0;
    int ret;
    run_command("sf probe", 0);
    for (i = 0; i < upgrade.total; i++)
    {
        if (upgrade.info[i].filesize <= upgrade.info[i].partitionsize)
        {
            printf("+++start bin+++\n");
            printf("partition_name:%s\n", upgrade.info[i].name);
            printf("file_size:%lu\n", upgrade.info[i].filesize);
            printf("file_offset:%lu\n", upgrade.info[i].fileoffset);
            printf("partition_size:%luK\n", upgrade.info[i].partitionsize / 1024);
            printf("partition_offset:0x%lx\n", upgrade.info[i].partitionoffset);
            printf("[%s] erase partition=%s offset=0x%lx size=0x%lx(%lu)\n",
                __func__, upgrade.info[i].name, upgrade.info[i].partitionoffset,
                upgrade.info[i].partitionsize, upgrade.info[i].partitionsize);

            char cmd[128] = {0};
            sprintf(cmd, "sf erase 0x%lx 0x%lx", upgrade.info[i].partitionoffset, upgrade.info[i].partitionsize);
            ret = run_command(cmd, 0);
            printf("[%s] erase ret=%d partition=%s\n",
                __func__, ret, upgrade.info[i].name);
            bin_size += (upgrade.info[i].filesize / 2);

            memset(cmd, 0, sizeof(cmd));
            sprintf(cmd, "sf write 0x%lx 0x%lx 0x%lx", upgrade.upgrade_offset_base + upgrade.info[i].fileoffset, upgrade.info[i].partitionoffset, upgrade.info[i].filesize);
            printf("[%s] write partition=%s src=0x%08lx dst=0x%lx size=0x%lx(%lu)\n",
                __func__, upgrade.info[i].name,
                upgrade.upgrade_offset_base + upgrade.info[i].fileoffset,
                upgrade.info[i].partitionoffset, upgrade.info[i].filesize,
                upgrade.info[i].filesize);
            ret = run_command(cmd, 0);
            printf("[%s] write ret=%d partition=%s\n",
                __func__, ret, upgrade.info[i].name);
            bin_size += (upgrade.info[i].filesize - (upgrade.info[i].filesize / 2));
        }
        else
        {
            printf("[%s] skip partition=%s file_size=%lu partition_size=%lu\n",
                __func__, upgrade.info[i].name, upgrade.info[i].filesize,
                upgrade.info[i].partitionsize);
            bin_size += upgrade.info[i].filesize;
        }
    }
    ret = env_load();
    if (ret) {
        printf("[%s] skip env_save: env_load failed ret=%d\n",
            __func__, ret);
    } else if (gd->flags & GD_FLG_ENV_DEFAULT) {
        printf("[%s] skip env_save: using default environment\n",
            __func__);
    } else {
        env_set("uprade_image_version", upgrade.upgrade_version);
        ret = env_save();
        printf("[%s] env_save ret=%d\n", __func__, ret);
    }
    mdelay(3000);
    return 0;
}

static upgrade_mode check_upgrade_mode(void)
{
    int size = tftpboot_upgrade_check(&upgrade.dev, &upgrade.part, &upgrade.file_name);
    if (size != 0)
    {
            upgrade.upgrade_image_size = size;
            printf("Upgrade mode:TFTP_BOOT!!! upgrade_image_size:%d\n",size);
            return TFTP_BOOT;
    }
    printf("The upgrade package is not detected and exits!!!\n");
    return NONE_MODE;
}

static int do_custom_upgrade(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    if (!(upgrade.mode = check_upgrade_mode()))
    {
        return 0;
    }

    if (!(upgrade.buffer = env_get("loadaddr")))
    {
        return 0;
    }

    upgrade.buffer = (char *)simple_strtoul(upgrade.buffer, NULL, 16);
    printf("[%s] loadaddr=0x%08lx upgrade_image_size=%lu\n",
        __func__, (unsigned long)upgrade.buffer, upgrade.upgrade_image_size);

    if (upgrade_version_check(upgrade.buffer))
    {
        upgrade_img_head_pares(upgrade.buffer);

        update_flash_partition();

        run_command("reset", 0);
    }
    return 0;
}

U_BOOT_CMD(
    custom_upgrade, 1, 0, do_custom_upgrade,
    " fat upgrade flash",
    "fat card spi flash image upgrade");
