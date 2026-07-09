/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_ir_sample.c
* Description: This is a simple example to show \
* how the ircut and irled module working.
* Notes:
* History: V1.0.0
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#include "ak_common.h"
#include "ak_log.h"
#include "ak_conf.h"

#define LEN_HINT                512
#define LEN_OPTION_SHORT        512

#define IRCUT_A_FILE_NAME       "ircut_a"
#define IRCUT_B_FILE_NAME       "ircut_b"


/* ircut mode define */
enum ircut_mode_type {
    IRCUT_MODE_1LINE,
    IRCUT_MODE_2LINE,
};


/* ir structure */
struct drv_ir {
    /*ircut name, define as sys fs*/
    char *ircut_name;
    /*default 1line mode*/
    int ircut_line_mode;
    /*init flag*/
    int init_flag;
    char ircut_a_gpio_id[8];
    char ircut_b_gpio_id[8];
};

static struct drv_ir ir_ctrl = {0};

char *ircut_a = "";
char *ircut_b = "";
int ircut_a_day_level = 0;
int ircut_b_day_level = 0;


static int mode = 0;
static int value = 0;
static char *pc_prog_name = NULL;
char *ps_ir_path = NULL;

static char ac_option_hint[  ][ LEN_HINT ] = {
    /*操作提示数组*/
    /*"help info"*/
    "help info" ,
    /*"mode"*/
    "[NUM] mode [0,1], 0 ircut, 1 irled" ,
    /*"value"*/
    "[NUM] value [0,1]" ,
    /*"ps ir cfg file path"*/
    "[PATH] ps ir cfg file path" ,
    "" ,
};

static struct option option_long[ ] = {
    /*"help info"*/
    { "help"            , no_argument       , NULL , 'h' } ,
    /*"[NUM] mode [0,1] , 0 ircut, 1 irled"*/
    { "mode"            , required_argument , NULL , 'm' } ,
    /*"[NUM] value [0,1]"*/
    { "value"           , required_argument , NULL , 'v' } ,
    /*"[PATH] ps ir cfg file path"*/
    { "ps_ir_cfg"       , required_argument , NULL , 'Z' } ,
    {0, 0, 0, 0}
 };


void usage(const char * name)
{
    ak_print_normal(MODULE_ID_APP, \
        "usage: %s -m mode -v value -Z ps ir cfg path!\n", name);
    ak_print_normal(MODULE_ID_APP, \
        "eg.: %s -m 0 -v 1 -Z /etc/config/ps_ir.conf\n", name);
    ak_print_normal(MODULE_ID_APP, "mode : 0:ircut,1:irled, default 0\n");
    ak_print_normal(MODULE_ID_APP, "value : 0 or 1, default 0\n");
}

/*
 * help_hint: use the -h --help option.Print option of help information
 * return: 0
 */
static int help_hint(char *pc_prog_name)
{
    int i;

    printf("%s\n" , pc_prog_name);
    for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++) {
        if( option_long[ i ].val != 0 ) {
            printf("\t--%-16s -%c %s\n" , option_long[ i ].name , \
                option_long[ i ].val , ac_option_hint[ i ]);
        }
    }

    usage(pc_prog_name);

    printf("\n\n");
    return 0;
}

/*
 * get_option_short: fill the stort option string.
 * return: option short string addr.
 */
static char *get_option_short( struct option *p_option, int i_num_option, \
    char *pc_option_short, int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    for( i = 0 ; i < i_num_option ; i ++ ) {
        if( ( c_option = p_option[ i ].val ) == 0 ) {
            continue;
        }
        switch( p_option[ i ].has_arg ){
        case no_argument:
            i_offset += snprintf( pc_option_short + i_offset , \
                i_len_option - i_offset , "%c" , c_option );
            break;
        case required_argument:
            i_offset += snprintf( pc_option_short + i_offset , \
                i_len_option - i_offset , "%c:" , c_option );
            break;
        case optional_argument:
            i_offset += snprintf( pc_option_short + i_offset , \
                i_len_option - i_offset , "%c::" , c_option );
            break;
        }
    }
    return pc_option_short;
}

/*
 * parse_option: parse_option
 * return:
 */
static int parse_option( int argc, char **argv )
{
    int i_option;
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = 1;
    pc_prog_name = argv[ 0 ];

    get_option_short( option_long, i_array_num , ac_option_short , \
        LEN_OPTION_SHORT );
    while((i_option = getopt_long(argc , argv , ac_option_short , \
        option_long , NULL)) > 0) {
        switch(i_option) {
        case 'h' :  //help
            help_hint(argv[0]);
            c_flag = 0;
            goto parse_option_end;
        case 'm' :  //mode
            mode = atoi(optarg);
            break;
        case 'v' :  //value
            value = atoi(optarg);
            break;
        case 'Z' :  //ps ir conf path
            ps_ir_path = optarg;
            break;
        default :
            help_hint(argv[0]);
            c_flag = AK_FALSE;
            goto parse_option_end;
        }
    }
parse_option_end:
    return c_flag;
}

/*
 * file_read_value - file read value
 * path[IN]: file path
 * str[OUT]: value str
 * len[IN]: str buf len
 * return:  0 - success; otherwise error code;
 * notes:
 */
static int file_read_value(char *path, char *str, int len)
{
    int fd = -1;
    int ret;

    if (NULL == path || NULL == str)
    {
        ak_print_error(MODULE_ID_APP, "param is null.\n");
          return ERROR_TYPE_POINTER_NULL;
    }

    fd = open(path, O_RDONLY);

      if (fd < 0)
      {
          ak_print_error(MODULE_ID_APP, "%s file open failed.\n", path);
          return ERROR_TYPE_DEV_CTRL_FAILED;
      }

      ret = read(fd, str, len);
      if (ret < 0)
      {
          ak_print_error(MODULE_ID_APP, "%s read failed.\n", path);
        close(fd);
          return ERROR_TYPE_DEV_CTRL_FAILED;
      }
    close(fd);

    return AK_SUCCESS;
}

/*
 * ircut_gpio_set - ircut gpio set
 * path[IN]: ircut file path
 * return:  0 - success; otherwise error code;
 * notes:
 */
static int ircut_gpio_set(char *path)
{
    char id_str[8] = {0};
    char gpio_path[64] = {0};
    struct stat st = {0};
    char cmd[128] = {0};

    if (NULL == path)
    {
        ak_print_error(MODULE_ID_DRV, "path is null.\n");
          return ERROR_TYPE_POINTER_NULL;
    }

    if (!strcmp(IRCUT_A_FILE_NAME, path))
    {
        /* ircut a */
        strncpy(ir_ctrl.ircut_a_gpio_id, ircut_a, 8);
        strncpy(id_str, ircut_a, 8);
    }
    else if (!strcmp(IRCUT_B_FILE_NAME, path))
    {
        /* ircut b */
        strncpy(ir_ctrl.ircut_b_gpio_id, ircut_b, 8);
        strncpy(id_str, ircut_b, 8);
    }
    else
    {
        ak_print_error(MODULE_ID_DRV, "%s error\n", path);
        return ERROR_TYPE_INVALID_ARG;
    }

    /* gpio id */
    memset(gpio_path, 0, 64);
    sprintf(gpio_path, "/sys/class/gpio/gpio%s", id_str);

    if (-1 == stat(gpio_path, &st))
    {
        memset(cmd, 0, 128);
        /* gpio export */
        sprintf(cmd, "echo %s > /sys/class/gpio/export", id_str);
        system(cmd);
    }

    memset(cmd, 0, 128);
    /* gpio direction */
    sprintf(cmd, "echo out > /sys/class/gpio/gpio%s/direction", id_str);
    system(cmd);

    memset(cmd, 0, 128);
    /* gpio drive */
    sprintf(cmd, "echo 3 > /sys/class/gpio/gpio%s/drive", id_str);
    system(cmd);

    memset(cmd, 0, 128);
    /* gpio pull_polarity */
    sprintf(cmd, "echo pullup > /sys/class/gpio/gpio%s/pull_polarity", id_str);
    system(cmd);

    memset(cmd, 0, 128);
    /* gpio pull_enable */
    sprintf(cmd, "echo 1 > /sys/class/gpio/gpio%s/pull_enable", id_str);
    system(cmd);

    memset(cmd, 0, 128);
    /* gpio slew_rate */
    sprintf(cmd, "echo fast > /sys/class/gpio/gpio%s/slew_rate", id_str);
    system(cmd);

    return AK_SUCCESS;
}

/*
 * camera_set_ircut - switch ircut
 * name[IN]: device name reflect on sys-fs
 * value[IN]: 0 or 1 to set day or night
 * return: 0 - success; otherwise error code;
 * notes:
 */
static int camera_set_ircut(char *name, int value)
{
    char id_str[8] = {0};
    char cmd[128] = {0};

    if (NULL == name)
    {
        ak_print_error(MODULE_ID_DRV, "name is null.\n");
          return ERROR_TYPE_POINTER_NULL;
    }

    if ((0 != value) && (1 != value)){
        ak_print_error(MODULE_ID_DRV, "invalid arg\n");
        return ERROR_TYPE_INVALID_ARG;
    }

    if (!strcmp(IRCUT_A_FILE_NAME, name))
        strncpy(id_str, ir_ctrl.ircut_a_gpio_id, 8);
    else if (!strcmp(IRCUT_B_FILE_NAME, name))
        strncpy(id_str, ir_ctrl.ircut_b_gpio_id, 8);
    else
    {
        ak_print_error(MODULE_ID_DRV, "%s error\n", name);
        return ERROR_TYPE_INVALID_ARG;
    }

    /* gpio value */
    memset(cmd, 0, 128);
    sprintf(cmd, "echo %d > /sys/class/gpio/gpio%s/value", value, id_str);
    system(cmd);

    ak_print_normal(MODULE_ID_DRV, "set %s %d success!\n", name, value);

    return AK_SUCCESS;
}

/*
 * read_conf - read_conf
 * pc_conf[IN]: pc_conf
 * return: 0 - success; otherwise error code;
 * notes:
 */
static int read_conf(char *pc_conf)
{
    int i_size ;
    char *pc_buff = NULL;

    if (NULL == (pc_buff = read_file(pc_conf, &i_size)))
    {
        return AK_FAILED;
    }


    get_conf_str(pc_buff, "ircut_a", &ircut_a);
    get_conf_str(pc_buff, "ircut_b", &ircut_b);
    get_conf_int(pc_buff, "ircut_a_day_level", &ircut_a_day_level);
    get_conf_int(pc_buff, "ircut_b_day_level", &ircut_b_day_level);

    free(pc_buff) ;
    return AK_SUCCESS;
}


/**
 * init_ircut - init ircut, get ircut control mode
 * @cfg_path[IN]:  ps ir config path
 * return: 0 - success; otherwise error code;
 */
int init_ircut(char *cfg_path)
{
    if (ir_ctrl.init_flag){
        ak_print_notice_ex(MODULE_ID_DRV, "have been inited.\n");
        return 0;
    }

    if (NULL == cfg_path)
    {
        ak_print_error_ex(MODULE_ID_DRV, "cfg_path NULL.\n");
        return ERROR_TYPE_POINTER_NULL;
    }

    if (read_conf(cfg_path))
    {
        ak_print_error_ex(MODULE_ID_DRV, "read conf failed.\n");
        return ERROR_TYPE_FILE_READ_FAILED;
    }

    int flags = 0;
    int ret;

    /* ircut a */
    if (0 == strcmp(ircut_a, "null")) {
        ak_print_info(MODULE_ID_DRV, "not use ircut_a\n");
        flags |= 1;
    } else {
        ir_ctrl.ircut_name = IRCUT_A_FILE_NAME;
    }

    /* ircut b */
    if (0 == strcmp(ircut_b, "null")) {
        ak_print_info(MODULE_ID_DRV, "not use ircut_b\n");
        flags |= 1 << 1;
    } else {
        ir_ctrl.ircut_name = IRCUT_B_FILE_NAME;
    }

    switch (flags) {
    case 0:
        ak_print_info(MODULE_ID_DRV, "Ircut a & b interface all can access\n");
        ir_ctrl.ircut_line_mode = IRCUT_MODE_2LINE;
        break;
    case 3:
        ak_print_error(MODULE_ID_DRV, "Ircut a & b interface can't access\n");
        return ERROR_TYPE_DEV_INIT_FAILED;
    default:
        ak_print_info(MODULE_ID_DRV, "Only can access:%s\n", ir_ctrl.ircut_name);
        ir_ctrl.ircut_line_mode = IRCUT_MODE_1LINE;
        break;
    }

    if (IRCUT_MODE_2LINE == ir_ctrl.ircut_line_mode)
    {
        /* 2 line*/
        ret = ircut_gpio_set(IRCUT_A_FILE_NAME);

        if (ret)
        {
            ak_print_error(MODULE_ID_DRV, "%s can't set gpio.\n", \
                IRCUT_A_FILE_NAME);
            return ret;
        }

        ret = ircut_gpio_set(IRCUT_B_FILE_NAME);

        if (ret)
        {
            ak_print_error(MODULE_ID_DRV, "%s can't set gpio.\n", \
                IRCUT_B_FILE_NAME);
            return ret;
        }
    }
    else
    {
        /* 1 line*/
        ret = ircut_gpio_set(ir_ctrl.ircut_name);

        if (ret)
        {
            ak_print_error(MODULE_ID_DRV, "%s can't set gpio.\n", \
                ir_ctrl.ircut_name);
            return ret;
        }
    }

    ir_ctrl.init_flag = AK_TRUE;
    ak_print_normal(MODULE_ID_DRV, "ircut init OK, mode %s.\n",
        ir_ctrl.ircut_line_mode == IRCUT_MODE_1LINE ? "1 line":"2 line");

    return AK_SUCCESS;
}

/**
 * set_ircut - set ircut to switch
 * @status_level[IN]:  status level to control day or night, [0,1]
 * return: 0 - success; otherwise error code;
 */
int set_ircut(int status_level)
{
    int day_level = 0;
    int level = 0;


    if (!ir_ctrl.init_flag){
        ak_print_error_ex(MODULE_ID_DRV, "not inited\n");
        return ERROR_TYPE_NOT_INIT;
    }

    if ((0 != status_level) && (1 != status_level)){
        ak_print_error_ex(MODULE_ID_DRV, "invalid arg\n");
        return ERROR_TYPE_INVALID_ARG;
    }

    if (IRCUT_MODE_1LINE == ir_ctrl.ircut_line_mode) {
        /* 1 line */
        ak_print_debug(MODULE_ID_DRV, "Ircut 1line mode, ir_feed:%d\n", status_level);
        if (0 == strcmp(IRCUT_A_FILE_NAME, ir_ctrl.ircut_name)) {
            /* ircut a */
            day_level = ircut_a_day_level;
        }
        else {
            /* ircut b */
            day_level = ircut_b_day_level;
        }

        if (0 == day_level){
            level = status_level;
            }
        else {
            level = !status_level;
            }
        return camera_set_ircut(ir_ctrl.ircut_name, level);
    }

    ak_print_debug(MODULE_ID_DRV, "Ircut 2line mode, ir_feed:%d\n", status_level);

    /* 2 line */

    day_level = ircut_a_day_level;

    if (0 == day_level)
        level = status_level;
    else
        level = !status_level;

    camera_set_ircut(IRCUT_A_FILE_NAME, level);

    day_level = ircut_b_day_level;

    if (0 == day_level)
        level = status_level;
    else
        level = !status_level;
    camera_set_ircut(IRCUT_B_FILE_NAME, level);

    ak_sleep_ms(200);
    /* ircut_set_idle */
    camera_set_ircut(IRCUT_A_FILE_NAME, 0);
    camera_set_ircut(IRCUT_B_FILE_NAME, 0);

    return AK_SUCCESS;
}




/**
 * Preconditions:
 * your main video progress must stop
 */
int main(int argc, char **argv)
{
    char cmd[128] = {0};

    /* start the application */
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_APP, "** ir sample start**\n");


    if (0 == parse_option(argc, argv))
    {
        return 0;
    }

    if (mode != 0 &&  mode != 1)
    {
        ak_print_error_ex(MODULE_ID_VPSS, "mode:%d error!\n", mode);
        return 0;
    }

    if (value != 0 &&  value != 1)
    {
        ak_print_error_ex(MODULE_ID_VPSS, "value:%d error!\n", value);
        return 0;
    }

    if (1 == mode)
    {
        /* irled */
        ak_print_normal_ex(MODULE_ID_VPSS, "set irled %d !\n", value);
        sprintf(cmd, "echo %d > /sys/devices/platform/leds/leds/irled/brightness", value);
        system(cmd);
    }
    else
    {
        /* ircut */
        ak_print_normal_ex(MODULE_ID_VPSS, "set irut %d !\n", value);
        init_ircut(ps_ir_path);
        set_ircut(value);
    }

exit:
    /*
     * step 4: release resource
     */


    /* exit */
    ak_sdk_exit();
    ak_print_normal(MODULE_ID_APP, "exit ir sample\n");
    return 0;
}
