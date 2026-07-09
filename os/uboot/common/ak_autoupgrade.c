#include <common.h>

#if 1
#define PRINTF(fmt,args...)         printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

#define FIRMWARE_PATH               "autoupgrade"
#define FIRMWARE_NAME               "uImage_fast"
#define UPDATE_PARTITION            "KERNEL"

int autoupgrade_firmware(void)
{
    char cmd[128];
    int ret;

    PRINTF("====================AUTOUPGRADE!!!====================\n");

    //1. 指示灯
    PRINTF("AUTOUPGRADE LED flash\n");

    //2. 检查升级条件, 检查SD卡
    snprintf(cmd, sizeof(cmd), "fatinfo mmc 0");
    PRINTF("cmd[%s]\n", cmd);
    ret = run_command(cmd, 0);
    if (ret != 0) {
        printf("cmd[%s], failure\n", cmd);
        return ret;
    }

    //3. 检查升级条件, 检查升级包
    snprintf(cmd, sizeof(cmd), "fatls mmc 0 /%s", FIRMWARE_PATH);
    PRINTF("cmd[%s]\n", cmd);
    ret = run_command(cmd, 0);
    if (ret != 0) {
        printf("cmd[%s], failure\n", cmd);
        return ret;
    }

    //4. 加载升级文件到DDR
    snprintf(cmd, sizeof(cmd), "fatload mmc 0 ${loadaddr} /%s/%s", FIRMWARE_PATH, FIRMWARE_NAME);
    PRINTF("cmd[%s]\n", cmd);
    ret = run_command(cmd, 0);
    if (ret != 0) {
        printf("cmd[%s], failure\n", cmd);
        return ret;
    }
    //5. 检查固件 - fixme
    //md5sum?
    PRINTF("fixme -> check firmware\n");

    //6. spi nor flash probe
    snprintf(cmd, sizeof(cmd), "sf probe");
    PRINTF("cmd[%s]\n", cmd);
    ret = run_command(cmd, 0);
    if (ret != 0) {
        printf("cmd[%s], failure\n", cmd);
        return ret;
    }

    //7. 更新分区
    snprintf(cmd, sizeof(cmd), "sf update ${loadaddr} %s $filesize", UPDATE_PARTITION);
    PRINTF("cmd[%s]\n", cmd);
    ret = run_command(cmd, 0);
    if (ret != 0) {
        printf("cmd[%s], failure\n", cmd);
        return ret;
    }

    //8. 校验分区 - fixme
    //md5sum?
    PRINTF("fixme -> readback partition data, check ok?\n");

    //9. 指示灯
    PRINTF("AUTOUPGRADE LED flash\n");

    PRINTF("====================AUTOUPGRADE OK====================\n");

    return ret;
}
