#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include "ak_drv_common.h"

#define MAX_HINT_LEN 512
#define DEFULAT_CFG_VALUE (-1)

#define READ_GLOBAL_ID      (1)
#define READ_SECURE_LEVEL   (2)
#define READ_LOCK           (3)
#define BURN_LOCK           (4)

#define AK3918AV100 1

#define SAMPLE_NAME "ak_drv_efuse_sample"

int ak_drv_led_set(char *type, int status);

#define OPTION_STRING   "c:h"

static struct option long_options[] =
{
    {"cmd", required_argument, NULL, 'c'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0},
};

static char option_hint[][MAX_HINT_LEN] =
{
#if defined(AK3918AV100)
    /* AK3918AV100 */
    "命令 可选择的类型有\n \
        1：显示global ID\n \
        2：读取安全启动级别\n \
        3：读数据锁定位\n \
        4：写数据锁定位",
#else
    "命令 可选择的类型有 1：显示global ID",
#endif
    "显示帮助信息",
};

int ak_drv_read_efuse(void);
int ak_drv_read_globalID(void);
#if defined(AK3918AV100)
/* AK3918AV100 */
int ak_drv_read_secure_level(void);
#endif
int ak_drv_read_lock(void);
int ak_drv_burn_lock(void);


/*
*1.version
*2.usage
*3.options
*4.example
*/
static void usage(const char *app)
{
    int opt_num = sizeof(long_options)/sizeof(struct option) - 1;
    int i;

    ak_print_normal("version:\n");
    ak_print_normal("  %s V1.0.0\n", app);
    ak_print_normal("\n");
    ak_print_normal("usage:\n");
    ak_print_normal("  %s [options] <>\n", app);
    ak_print_normal("\n");
    ak_print_normal("options:\n");
    for (i = 0; i < opt_num; i++) {
        ak_print_normal("  -%c, --%s  %s\n",
            long_options[i].val, long_options[i].name, option_hint[i]);
    }
    ak_print_normal("\n");
    ak_print_normal("example:\n");
    ak_print_normal("  %s -c 1\n", app);
    ak_print_normal("  %s --cmd 1\n", app);
}

/*
*"命令 可选择的类型有\n \
    1：显示global ID\n \
    2：读取安全启动级别\n \
    3：读数据锁定位\n \
    4：写数据锁定位",

*"显示帮助信息"
*/
int main (int argc, char **argv)
{
    int cmd = 0;
    int opt_val = 0, option_idx = 0;
    char islock;

    if (argc <= 1) {
        usage(SAMPLE_NAME);
        return 0;
    }

    while ((opt_val = getopt_long(argc, argv, OPTION_STRING,
                long_options, &option_idx)) != -1) {
        switch (opt_val) {
            /*
            *"命令 可选择的类型有 
                1：显示global ID
                2：读取安全启动级别
                3：写数据锁定位
                4：读数据锁定位
            */
            case 'c':
                cmd = atoi(optarg);
                break;
            /*
            *"显示帮助信息"
            */
            case 'h':
            default:
                usage(SAMPLE_NAME);
                return -EINVAL;
        }
    }

    ak_print_normal("CMD:@%d\n", cmd);
    switch (cmd) {
        case READ_GLOBAL_ID:
            ak_drv_read_globalID();
            break;
#if defined(AK3918AV100)
        /* AK3918AV100 */
        case READ_SECURE_LEVEL:
            ak_drv_read_secure_level();
            break;
        case READ_LOCK:
            islock = ak_drv_read_lock();
            if(islock){
                ak_print_normal("chip is locked!\n");
            }else{
                ak_print_normal("chip not locked!\n");
            }
            break;
        case BURN_LOCK:
            islock = ak_drv_burn_lock();
            if(islock){
                ak_print_normal("chip is locked!\n");
            }else{
                ak_print_normal("chip not locked!\n");
            }
            break;
#endif
        default:
            ak_print_normal("unknow CMD@%d\n", cmd);
            break;
    }

    return 0;
}
