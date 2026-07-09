#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "ak_drv_common.h"

#include "../../include/ak_efuse.h"

#define AK3918AV100 1

#define AK_EFUSE_FILE_PATH "/dev/efuse"

/**
 * @brief display_efuse_global_id
 * 
 * @param unsigned char *global_id
 * @param int len
 */
static void display_efuse_global_id(unsigned char *global_id, int len) {
    int tmp = 0;

    printf("Chip globalID: ");
    for (tmp = 0; tmp < len; tmp++) {
        printf("%02X ", global_id[tmp]);
    }
    printf("\n");
}

#if defined(AK3918AV100)
/**
 * @brief 显示安全启动级别
 * 
 * @param level 安全启动级别
 */
static void display_efuse_secure_level(secure_level level) {
    const char* secure_level_str[] = {
        [0] = "LEVEL4_SPECIAL_SECURE",
        [1] = "LEVEL0_NO_SECURE",
        [3] = "LEVEL1_RSA2048",
        [5] = "LEVEL2_AES256",
        [7] = "LEVEL3_RSA2048_AES256",
    };

    printf("Secure Level: %d(%s)\n", (int)level, secure_level_str[(int)level]);
}
#endif /* AK3918AV100 */

/**
 * @brief read_globalID
 * @param void
 * @return globalID
 */
int ak_drv_read_globalID(void)
{
    int ret;
    int len;
    int fd = -1;
    unsigned char *buffer;

    fd = open(AK_EFUSE_FILE_PATH, O_RDONLY);
    if (fd < 0) {
        ak_print_error("open efuse fail.\n");
        return -EACCES;
    }

    len = ioctl(fd, DO_GLOBE_ID_GET_LEN, NULL);
    if (len > 0) {
        buffer = malloc(len);

        if (buffer) {
            ret = ioctl(fd, DO_GLOBE_ID_GET, buffer);
            if (ret) {
                ak_print_error_ex("read global id fail, ret=%d.\n", ret);
                goto fail;
            } else {
                display_efuse_global_id(buffer, len);
            }

            free(buffer);
        } else {
            ak_print_error_ex("malloc %d bytes fail.\n", len);
            ret = -ENOMEM;
            goto fail;
        }
    } else {
        ak_print_error("get global id fail, len=%d.\n", len);
        ret = -EIO;
        goto fail;
    }

    close(fd);

    return 0;
fail:
    return ret;
}

#if defined(AK3918AV100)
/**
 * @brief 读取安全启动级别
 * 
 * @return int 0: 成功
 * @return int !0: 失败
 */
int ak_drv_read_secure_level(void)
{
    int ret;
    int fd = -1;
    secure_level level;

    fd = open(AK_EFUSE_FILE_PATH, O_RDONLY);
    if (fd < 0) {
        ak_print_error("open efuse fail.\n");
        return -EACCES;
    }

    ret = ioctl(fd, DO_READ_SECURE_LEVEL, &level);
    if (ret) {
        ak_print_error_ex("read secure level fail, ret=%d.\n", ret);
        close(fd);
        return ret;
    } else {
        display_efuse_secure_level(level);
    }

    close(fd);

    return 0;
}

/**
 * @brief read_lock
 * @param void
 * @return islock
 */
char ak_drv_read_lock(void)
{
    int ret;
    int fd = -1;
    char islock;

    fd = open(AK_EFUSE_FILE_PATH, O_RDONLY);
    if (fd < 0) {
        ak_print_error("open efuse fail.\n");
        return -EACCES;
    }

    ret = ioctl(fd, DO_READ_LOCK, &islock);
    if(ret){
        close(fd);
        return ret;
    }

    close(fd);

    return islock;
}

/**
 * @brief burn_lock
 * @param void
 * @return islock
 */
char ak_drv_burn_lock(void)
{
    int ret;
    int fd = -1;
    char islock;

    fd = open(AK_EFUSE_FILE_PATH, O_RDONLY);
    if (fd < 0) {
        ak_print_error("open efuse fail.\n");
        return -EACCES;
    }

    ret = ioctl(fd, DO_READ_LOCK, &islock);
    if(ret){
        close(fd);
        return ret;
    }
    if(islock){
        close(fd);
        return islock;
    }

    ret = ioctl(fd, DO_WRITE_FINISH, NULL);
    if(ret){
        close(fd);
        return ret;
    }

    ret = ioctl(fd, DO_READ_LOCK, &islock);
    if(ret){
        close(fd);
        return ret;
    }

    close(fd);

    return islock;
}
#endif /* AK3918AV100 */


