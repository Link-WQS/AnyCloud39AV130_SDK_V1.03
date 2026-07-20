#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "PwmControl.h"

#define PWM_DEV_MAX_NUM 5
#define PERIOD_S_MIN 167L      /* 1s /6Mhz */
#define PERIOD_S_MAX 10869565L /* 1s / 92hz */

/*
 * AkDrvPwmOpen - open pwm device
 * device_no[IN]: pwm device minor-number,[0-4]
 * return: 0 - success; otherwise error code;
 */
int AkDrvPwmOpen(int device_no)
{
    int fd = -1;
    int ret = 0;
    char dev_name[64] = {0};
    char path[64] = {0};

    if (device_no < 0 || device_no >= PWM_DEV_MAX_NUM)
    {
        printf("device_no error, %d, must in [0-%d]\n", device_no, PWM_DEV_MAX_NUM - 1);
        return -1;
    }

    sprintf(path, "/sys/class/pwm/pwmchip%d/pwm0", device_no);

    if (!access(path, F_OK))
    {
        printf("pwm dev %d already opened.\n", device_no);
        return 0;
    }

    /*
     *create pwm dev use export;
     *close pwm dev use unexport.
     */
    sprintf(dev_name, "/sys/class/pwm/pwmchip%d/export", device_no);

    fd = open(dev_name, O_WRONLY);
    if (fd < 0)
    {
        printf("Open pwm dev %d fail\n", device_no);
        return -1;
    }

    /*
     *write 0 to export, it will create pwm0 folder.
     */
    ret = write(fd, "0", strlen("0"));
    if (ret < 0)
    {
        close(fd);
        printf("create pwm dev %d fail\n", device_no);
        return -1;
    }
    close(fd);

    return 0;
}

/*
 * AkDrvPwmSet - set pwd working param
 * device_no[IN]: pwm device minor-number,[0-4]
 * duty_ns[IN]: pwm duty time in ns.
 * period_ns[IN]: pwm period time in ns.
 * return: 0 on success, otherwise error code.
 */
int AkDrvPwmSet(int device_no, long long duty_ns, long long period_ns)
{
    int fd = -1;
    int ret;
    char path[64] = {0};
    char str[32] = {0};

    /* arguments check */
    if (device_no < 0 || device_no >= PWM_DEV_MAX_NUM)
    {
        printf("device_no error, %d, must in [0-%d]\n", device_no, PWM_DEV_MAX_NUM - 1);
        return -1;
    }

    if (period_ns < PERIOD_S_MIN || period_ns > PERIOD_S_MAX)
    {
        printf("period_ns error, must in [%ld-%ld]\n", PERIOD_S_MIN, PERIOD_S_MAX);
        return -1;
    }

    if (duty_ns <= 0 || duty_ns >= period_ns)
    {
        printf("duty_ns error, must less than period_ns\n");
        return -1;
    }

    sprintf(path, "/sys/class/pwm/pwmchip%d/pwm0", device_no);

    if (access(path, F_OK))
    {
        printf("pwm dev %d not open.\n", device_no);
        return -1;
    }

    /*
     *disable pwm before set pwm period and duty_cycle
     */
    memset(path, 0, 64);
    sprintf(path, "/sys/class/pwm/pwmchip%d/pwm0/enable", device_no);
    fd = open(path, O_RDWR);

    if (fd < 0)
    {
        printf("pwm dev %d enable file open failed.\n", device_no);
        return -1;
    }

    ret = write(fd, "0", strlen("0"));
    if (ret < 0)
    {
        printf("pwm dev %d set disable failed.\n", device_no);
        close(fd);
        return -1;
    }
    close(fd);

    /*
     *set pwn period
     */
    memset(path, 0, 64);
    sprintf(path, "/sys/class/pwm/pwmchip%d/pwm0/period", device_no);
    fd = open(path, O_RDWR);

    if (fd < 0)
    {
        printf("pwm dev %d period file open failed.\n", device_no);
        return -1;
    }

    sprintf(str, "%lld", period_ns);
    ret = write(fd, str, strlen(str));
    if (ret < 0)
    {
        printf("pwm dev %d set period_ns failed.\n", device_no);
        close(fd);
        return -1;
    }
    close(fd);

    /*
     *set pwn duty_cycle
     */
    memset(path, 0, 64);
    sprintf(path, "/sys/class/pwm/pwmchip%d/pwm0/duty_cycle", device_no);
    fd = open(path, O_RDWR);

    if (fd < 0)
    {
        printf("pwm dev %d duty file open failed.\n", device_no);
        return -1;
    }

    memset(str, 0, 32);
    sprintf(str, "%lld", duty_ns);
    ret = write(fd, str, strlen(str));
    if (ret < 0)
    {
        printf("pwm dev %d set duty_ns failed.\n", device_no);
        close(fd);
        return -1;
    }

    close(fd);

    /*
     *enable pwm after set pwm period and duty_cycle
     */
    memset(path, 0, 64);
    sprintf(path, "/sys/class/pwm/pwmchip%d/pwm0/enable", device_no);
    fd = open(path, O_RDWR);

    if (fd < 0)
    {
        printf("pwm dev %d enable file open failed.\n", device_no);
        return -1;
    }

    ret = write(fd, "1", strlen("1"));
    if (ret < 0)
    {
        printf("pwm dev %d set enable failed.\n", device_no);
        close(fd);
        return -1;
    }
    close(fd);

    return 0;
}

/*
 * ak_drv_pwm_close - close pwm
 * device_no[IN]: pwm device minor-number,[0-4]
 * return: 0 on success, otherwise error code.
 */
int AkDrvPwmClose(int device_no)
{
    int fd = -1;
    int ret = 0;
    char path[64] = {0};
    char dev_name[64] = {0};
    char str[32] = {0};

    if (device_no < 0 || device_no >= PWM_DEV_MAX_NUM)
    {
        printf("device_no error, %d, must in [0-%d]\n", device_no, PWM_DEV_MAX_NUM - 1);
        return -1;
    }

    sprintf(path, "/sys/class/pwm/pwmchip%d/pwm0", device_no);

    if (access(path, F_OK))
    {
        printf("pwm dev %d not open.\n", device_no);
        return -1;
    }

    /*
     *disable pwm before close
     */
    memset(path, 0, 64);
    sprintf(path, "/sys/class/pwm/pwmchip%d/pwm0/enable", device_no);
    fd = open(path, O_RDWR);

    if (fd < 0)
    {
        printf("pwm dev %d enable file open failed.\n", device_no);
        return -1;
    }

    ret = write(fd, "0", strlen("0"));
    if (ret < 0)
    {
        printf("pwm dev %d set disable failed.\n", device_no);
        close(fd);
        return -1;
    }
    close(fd);

    /*
     *set pwn period
     */
    memset(path, 0, 64);
    sprintf(path, "/sys/class/pwm/pwmchip%d/pwm0/period", device_no);
    fd = open(path, O_RDWR);

    if (fd < 0)
    {
        printf("pwm dev %d period file open failed.\n", device_no);
        return -1;
    }

    sprintf(str, "%ld", PERIOD_S_MAX);
    ret = write(fd, str, strlen(str));
    if (ret < 0)
    {
        printf("pwm dev %d set period_ns failed.\n", device_no);
        close(fd);
        return -1;
    }
    close(fd);

    /*
     *set pwn duty_cycle
     */
    memset(path, 0, 64);
    sprintf(path, "/sys/class/pwm/pwmchip%d/pwm0/duty_cycle", device_no);
    fd = open(path, O_RDWR);

    if (fd < 0)
    {
        printf("pwm dev %d duty file open failed.\n", device_no);
        return -1;
    }

    memset(str, 0, 32);
    sprintf(str, "%d", 0);
    ret = write(fd, str, strlen(str));
    if (ret < 0)
    {
        printf("pwm dev %d set duty_ns failed.\n", device_no);
        close(fd);
        return -1;
    }

    /*
     *close pwm dev
     */
    sprintf(dev_name, "/sys/class/pwm/pwmchip%d/unexport", device_no);

    fd = open(dev_name, O_WRONLY);

    if (fd < 0)
    {
        printf("Open pwm dev unexport %d fail\n", device_no);
        return -1;
    }

    /*
     *write 0 to unexport, it will close pwm dev.
     */
    ret = write(fd, "0", strlen("0"));
    if (ret < 0)
    {
        close(fd);
        printf("close pwm dev %d fail\n", device_no);
        return -1;
    }
    close(fd);

    return 0;
}