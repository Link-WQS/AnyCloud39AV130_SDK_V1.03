#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <unistd.h>
#include "ak_common.h"
#include "ak_log.h"
#include "ak_drv.h"
#include "ak_conf.h"

#define STRING_LEN	     	128
#define IRCUT_A_FILE_NAME       "ircut_a"
#define IRCUT_B_FILE_NAME       "ircut_b"

#define IRLED_FILE_NAME		    "/sys/devices/platform/leds/leds/irled/brightness"


/* ircut mode define */
enum ircut_mode_type {
    IRCUT_MODE_1LINE,		//1 line mode
    IRCUT_MODE_2LINE,		//2 line mode
};


/* ir structure */
struct drv_ir {
    char *ircut_name;		//ircut name, define as sys fs
    int ircut_line_mode;	//default 1line mode
    int init_flag;			//init flag
    char ircut_a_gpio_id[8];
    char ircut_b_gpio_id[8];
};

static struct drv_ir ir_ctrl = {0};

/* ir structure */
struct drv_irled {
    char *irled_name;		//ircut name, define as sys fs
    char irled_gpio_id[8];
};

struct drv_irled_info {
    int init;
    struct ak_drv_irled_hw_param param;
};

static struct drv_irled_info irled_info;
static int ircut_time = 100;
char *ircut_a = "";
char *ircut_b = "";
int ircut_a_day_level = 0;
int ircut_b_day_level = 0;

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
        ak_print_error(MODULE_ID_DRV, "param is null.\n");
          return ERROR_TYPE_POINTER_NULL;
    }

    fd = open(path, O_RDONLY);

      if (fd < 0)
      {
          ak_print_error(MODULE_ID_DRV, "%s file open failed.\n", path);
          return ERROR_TYPE_DEV_CTRL_FAILED;
      }

      ret = read(fd, str, len);
      if (ret < 0)
      {
          ak_print_error(MODULE_ID_DRV, "%s read failed.\n", path);
        close(fd);
          return ERROR_TYPE_DEV_CTRL_FAILED;
      }
    close(fd);

    return AK_SUCCESS;
}

/*
 * file_write_value - file write value
 * path[IN]: file path
 * str[IN]: value str
 * return:  0 - success; otherwise error code;
 * notes:
 */
static int file_write_value(char *path, char *str)
{
    int fd = -1;
    int ret;

    if (NULL == path || NULL == str)
    {
        ak_print_error(MODULE_ID_DRV, "param is null.\n");
          return ERROR_TYPE_POINTER_NULL;
    }

    fd = open(path, O_WRONLY);

      if (fd < 0)
      {
          ak_print_error(MODULE_ID_DRV, "%s file open failed.\n", path);
          return ERROR_TYPE_DEV_CTRL_FAILED;
      }

      ret = write(fd, str, strlen(str));
      if (ret < 0)
      {
          ak_print_error(MODULE_ID_DRV, "%s write %s failed.\n", path, str);
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

    if (NULL == path)
    {
        ak_print_error(MODULE_ID_DRV, "path is null.\n");
          return ERROR_TYPE_POINTER_NULL;
    }

    if (!strcmp(IRCUT_A_FILE_NAME, path))
    {
        strncpy(ir_ctrl.ircut_a_gpio_id, ircut_a, 8);
        strncpy(id_str, ircut_a, 8);
    }
    else if (!strcmp(IRCUT_B_FILE_NAME, path))
    {
        strncpy(ir_ctrl.ircut_b_gpio_id, ircut_b, 8);
        strncpy(id_str, ircut_b, 8);
    }
    else
    {
        ak_print_error(MODULE_ID_DRV, "%s error\n", path);
        return ERROR_TYPE_INVALID_ARG;
    }

    memset(gpio_path, 0, 64);
    sprintf(gpio_path, "/sys/class/gpio/gpio%s", id_str);

    if (-1 == stat(gpio_path, &st))
    {
        if (file_write_value("/sys/class/gpio/export", id_str))
        {
            ak_print_error(MODULE_ID_DRV, "%s can't set\n", path);
            return ERROR_TYPE_DEV_CTRL_FAILED;
        }
    }

    memset(gpio_path, 0, 64);
    sprintf(gpio_path, "/sys/class/gpio/gpio%s/direction", id_str);

    if (file_write_value(gpio_path, "out"))
    {
        ak_print_error(MODULE_ID_DRV, "%s can't set\n", path);
        return ERROR_TYPE_DEV_CTRL_FAILED;
    }

    memset(gpio_path, 0, 64);
    sprintf(gpio_path, "/sys/class/gpio/gpio%s/drive", id_str);

    if (file_write_value(gpio_path, "3"))
    {
        ak_print_error(MODULE_ID_DRV, "%s can't set\n", path);
        return ERROR_TYPE_DEV_CTRL_FAILED;
    }

    memset(gpio_path, 0, 64);
    sprintf(gpio_path, "/sys/class/gpio/gpio%s/pull_polarity", id_str);

    if (file_write_value(gpio_path, "pullup"))
    {
        ak_print_error(MODULE_ID_DRV, "%s can't set\n", path);
        return ERROR_TYPE_DEV_CTRL_FAILED;
    }

    memset(gpio_path, 0, 64);
    sprintf(gpio_path, "/sys/class/gpio/gpio%s/pull_enable", id_str);

    if (file_write_value(gpio_path, "1"))
    {
        ak_print_error(MODULE_ID_DRV, "%s can't set\n", path);
        return ERROR_TYPE_DEV_CTRL_FAILED;
    }

    memset(gpio_path, 0, 64);
    sprintf(gpio_path, "/sys/class/gpio/gpio%s/slew_rate", id_str);

    if (file_write_value(gpio_path, "fast"))
    {
        ak_print_error(MODULE_ID_DRV, "%s can't set\n", path);
        return ERROR_TYPE_DEV_CTRL_FAILED;
    }

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
    char value_str[8] = {0};
    char id_str[8] = {0};
    char gpio_path[64] = {0};

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


    memset(gpio_path, 0, 64);
    sprintf(gpio_path, "/sys/class/gpio/gpio%s/value", id_str);

    sprintf(value_str, "%d", value);

    if (file_write_value(gpio_path, value_str))
    {
        ak_print_error(MODULE_ID_DRV, "%s can't set\n", name);
        return ERROR_TYPE_DEV_CTRL_FAILED;
    }

    ak_print_normal(MODULE_ID_DRV, "set %s %d success!\n", name, value);

    return AK_SUCCESS;
}

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
 * osal_drv_ir_init - init ircut, get ircut control mode
 * @cfg_path[IN]:  ps ir config path
 * return: 0 - success; otherwise error code;
 */
int osal_drv_ir_init(char *cfg_path)
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

    if (0 == strcmp(ircut_a, "null")) {
        ak_print_info(MODULE_ID_DRV, "not use ircut_a\n");
        flags |= 1;
    } else {
        ir_ctrl.ircut_name = IRCUT_A_FILE_NAME;
    }

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
        ret = ircut_gpio_set(IRCUT_A_FILE_NAME);

        if (ret)
        {
            ak_print_error(MODULE_ID_DRV, "%s can't set gpio.\n", IRCUT_A_FILE_NAME);
            return ret;
        }

        ret = ircut_gpio_set(IRCUT_B_FILE_NAME);

        if (ret)
        {
            ak_print_error(MODULE_ID_DRV, "%s can't set gpio.\n", IRCUT_B_FILE_NAME);
            return ret;
        }
    }
    else    // 1 line
    {
        ret = ircut_gpio_set(ir_ctrl.ircut_name);

        if (ret)
        {
            ak_print_error(MODULE_ID_DRV, "%s can't set gpio.\n", ir_ctrl.ircut_name);
            return ret;
        }
    }

    ir_ctrl.init_flag = AK_TRUE;
    ak_print_normal(MODULE_ID_DRV, "ircut init OK, mode %s.\n",
        ir_ctrl.ircut_line_mode == IRCUT_MODE_1LINE ? "1 line":"2 line");

    return AK_SUCCESS;
}

/**
 * osal_drv_ir_set_ircut - set ircut to switch
 * @status_level[IN]:  status level to control day or night, [0,1]
 * return: 0 - success; otherwise error code;
 */
int osal_drv_ir_set_ircut(int status_level)
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
        ak_print_debug(MODULE_ID_DRV, "Ircut 1line mode, ir_feed:%d\n", status_level);
        if (0 == strcmp(IRCUT_A_FILE_NAME, ir_ctrl.ircut_name)) {
            day_level = ircut_a_day_level;
        }
        else {
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

    ak_sleep_ms(ircut_time);
    /* ircut_set_idle */
    camera_set_ircut(IRCUT_A_FILE_NAME, 0);
    camera_set_ircut(IRCUT_B_FILE_NAME, 0);

    return AK_SUCCESS;
}

/**
 * osal_drv_ir_set_ircut_time - set ircut time
 * @time_ms[IN]:  time in ms
 * return: void;
 */
void osal_drv_ir_set_ircut_time(int time_ms)
{
    ircut_time = time_ms;
}

/**
 * osal_drv_ir_get_ircut_time - set ircut time
 * @time_ms[OUT]:  time in ms
 * return: 0 - success; otherwise error code;
 */
int osal_drv_ir_get_ircut_time(int *time_ms)
{
    if (NULL == time_ms)
        return ERROR_TYPE_INVALID_ARG;

    *time_ms = ircut_time;
    return AK_SUCCESS;
}



/*
 * camera_set_ircut - switch ircut
 * name[IN]: device name reflect on sys-fs
 * value[IN]: 0 or 1 to set day or night
 * return: 0 - success; otherwise error code;
 * notes:
 */
static int camera_set_irled(char *name, int value)
{
    char value_str[8] = {0};

    if (NULL == name)
    {
        ak_print_error(MODULE_ID_DRV, "name is null.\n");
          return ERROR_TYPE_POINTER_NULL;
    }

    if ((0 != value) && (1 != value)){
        ak_print_error(MODULE_ID_DRV, "invalid arg\n");
        return ERROR_TYPE_INVALID_ARG;
    }

    sprintf(value_str, "%d", value);

    if (file_write_value(name, value_str)) {
        ak_print_error(MODULE_ID_DRV, "%s can't set\n", name);
        return ERROR_TYPE_DEV_CTRL_FAILED;
    }

    return AK_SUCCESS;
}

/**
 * osal_drv_irled_init - init irled, get irled control mode
 * return: 0 - success; otherwise error code;
 */
int osal_drv_irled_init(void)
{
    int ret = access(IRLED_FILE_NAME, F_OK);
    if (ret) {
        ak_print_error(MODULE_ID_DRV, "access %s error %d\n", IRLED_FILE_NAME, ret);
          return ERROR_TYPE_INVALID_ARG;
    }

    ak_print_normal(MODULE_ID_DRV, "irled init OK\n");
    return AK_SUCCESS;
}

/**
 * osal_drv_irled_get_status - get irled module working status
 * @status_level[OUT]:  status level to control on or off [0,1]
 * return: 0 - success; otherwise error code;
 */
int osal_drv_irled_get_status(int *status_level)
{
    char value_str[8] = {0};

    if (file_read_value(IRLED_FILE_NAME, value_str, 8)) {
        ak_print_error(MODULE_ID_DRV, "%s can't get\n", IRLED_FILE_NAME);
        return ERROR_TYPE_DEV_CTRL_FAILED;
    }
    *status_level = atoi(value_str);

    return AK_SUCCESS;
}

/**
 * osal_drv_ir_init - init ircut, get ircut control mode
 * @status_level[IN]:  status level to control on or off [0,1]
 * return: 0 - success; otherwise error code;
 */
int osal_drv_irled_set_status(int status_level)
{
    if ((0 != status_level) && (1 != status_level)){
        ak_print_error_ex(MODULE_ID_DRV, "invalid arg\n");
        return ERROR_TYPE_INVALID_ARG;
    }

    camera_set_irled(IRLED_FILE_NAME, status_level);
    return AK_SUCCESS;
}


/**
 * drv_irled_set_working_level - set irled module working status
 * 		(nighttime level)
 * return: 0 - success; otherwise -1;
 */
static int drv_irled_set_working_level(struct ak_drv_irled_hw_param *param)
{
    int level = param->irled_working_level;
    if (level)
        irled_info.param.irled_working_level = 1;
    else
        irled_info.param.irled_working_level = 0;

    ak_print_info_ex(MODULE_ID_DRV, "irled_working_level:%d\n",
            irled_info.param.irled_working_level);

    return 0;
}

/**
 * ak_drv_irled_init - set irled module initialize
 * param[IN]: pointer to struct ak_drv_irled_hw_param, use to init
 *            ir_led model
 * return: 0 - success; otherwise -1;
 */
int ak_drv_irled_init(struct ak_drv_irled_hw_param *param)
{
    int ret;
    if (!param) {
        ak_print_error_ex(MODULE_ID_DRV, "param is NULL\n");
        return -1;
    }

    ret = drv_irled_set_working_level(param);
    if (!ret && !osal_drv_irled_init())
        irled_info.init = 1;

    return ret;
}

/**
 * ak_drv_irled_get_working_stat - get irled module working status
 * @working[OUT]: set irled working or not. 0 - NOT working; 1 - working;
 * return: 0 - success; otherwise -1;
 */
int ak_drv_irled_get_working_stat(int *status_level)
{
    if (irled_info.init != 1) {
        ak_print_error_ex(MODULE_ID_DRV, "not init.\n");
        return -1;
    }

    if (!status_level) {
        ak_print_error_ex(MODULE_ID_DRV, "status_level is NULL\n");
        return -1;
    }

    if (osal_drv_irled_get_status(status_level)) {
        ak_print_error_ex(MODULE_ID_DRV, "osal_drv_irled_get_status failed \n");
        return -1;
    }
    return 0;// success
}

/**
 * ak_drv_irled_set_working_stat - set irled module working status
 * @working[IN]: set irled working or not. 0 - NOT working; 1 - working;
 * return: 1 - irled is working; 0 - irled is NOT working; -1 - get fail;
 */
int ak_drv_irled_set_working_stat(int status_level)
{
    if (irled_info.init != 1) {
        ak_print_error_ex(MODULE_ID_DRV, "not init.\n");
        return -1;
    }

    return osal_drv_irled_set_status(status_level);
}

/**
 * ak_drv_ir_init - init ircut, get ircut control mode, set gpio
 * @cfg_path[IN]:  ps ir config path
 * return: 0 - success; otherwise error code;
 */
int ak_drv_ir_init(char *cfg_path)
{
    return osal_drv_ir_init(cfg_path);
}

/**
 * ak_drv_ir_set_ircut - set ircut to switch
 * @status_level[IN]:  status level to control day or night, [0,1]
 * return: 0 - success; otherwise error code;
 */
int ak_drv_ir_set_ircut(int status_level)
{
    return osal_drv_ir_set_ircut(status_level);
}

/**
 * ak_drv_ir_set_ircut_time - set ircut time
 * @time_ms[IN]:  time in ms
 * return: void;
 */
void ak_drv_ir_set_ircut_time(int time_ms)
{
    osal_drv_ir_set_ircut_time(time_ms);
}

/**
 * ak_drv_ir_get_ircut_time - set ircut time
 * @time_ms[OUT]:  time in ms
 * return: 0 - success; otherwise error code;
 */
int ak_drv_ir_get_ircut_time(int *time_ms)
{
    return osal_drv_ir_get_ircut_time(time_ms);
}

/* end of file */
