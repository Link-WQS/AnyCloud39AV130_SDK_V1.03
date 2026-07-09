/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_ps.c
* Description: This is a simple example to show how the photosensitive module working.
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
#include "ak_vi.h"
#include "ak_mem.h"
#include "ak_log.h"
#include "ak_vpss.h"
#include "ak_drv.h"
#include "ak_thread.h"
#include "ak_ps_ir.h"
#include "ak_conf.h"

/* ain0 */
#define PS_AIN0_FILE_NAME   "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
/* ain1 */
#define PS_AIN1_FILE_NAME   "/sys/bus/iio/devices/iio:device0/in_voltage1_raw"
/* ain2 */
#define PS_AIN2_FILE_NAME   "/sys/bus/iio/devices/iio:device0/in_voltage2_raw"

enum ak_hw_ps_mode {
    HARDWARE_PS_AIN,	// ain mode
    HARDWARE_PS_GPIO	// gpio mode
};

struct hw_ps {
    int init_flag;
    enum ak_hw_ps_mode mode;   //0 ain , 1 gpio
    char ain_file[128];
    char gpio_id[8];
    int pre_status;	//0 day, 1 night
    int day_lv; //0 low, 1 high
    int h_lv;
    int l_lv;
};

struct ak_ps {
    int ircut_run_flag;	    //photosensitive and ircut switch run flag
    int ps_switch_enable;   //store photosensitive switch status
    //int day_level_ctrl;		//day level default led-ircut config
    int ipc_run_flag;		//ipc run flag
    int pre_status;			//current state,0 day, 1 night
    int dev_id;
    ak_pthread_t ircut_tid;
    int cur_set_mode;
    int thread_run;
    enum ak_photosensitive_mode ps_mode;
    thread_func ps_fun_thread;// //photosensitive thread function
};

static struct hw_ps hw_ps_ctrl = {0};

static struct ak_ps ps_ctrl = {0};

static const char *ps_version = "libapp_ps V1.0.00";
char *photosensitive_port = "";
char *photosensitive_day_level = "";
int photosensitive_higher_level = 0;
int photosensitive_lower_level = 0;

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
        ak_print_error(MODULE_ID_APP, "param is null.\n");
          return ERROR_TYPE_POINTER_NULL;
    }

    fd = open(path, O_WRONLY);

      if (fd < 0)
      {
          ak_print_error(MODULE_ID_APP, "%s file open failed.\n", path);
          return ERROR_TYPE_DEV_CTRL_FAILED;
      }

      ret = write(fd, str, strlen(str));
      if (ret < 0)
      {
          ak_print_error(MODULE_ID_APP, "%s write %s failed.\n", path, str);
        close(fd);
          return ERROR_TYPE_DEV_CTRL_FAILED;
      }
    close(fd);

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


    get_conf_str(pc_buff, "photosensitive_port", &photosensitive_port);
    get_conf_str(pc_buff, "photosensitive_day_level", &photosensitive_day_level);
    get_conf_int(pc_buff, "photosensitive_higher_level", &photosensitive_higher_level);
    get_conf_int(pc_buff, "photosensitive_lower_level", &photosensitive_lower_level);

    free(pc_buff) ;
    return AK_SUCCESS;
}

/*
 * hw_ps_init - init ps and ircut hardware
 * cfg_path[IN]: ps ir cfg_path
 * return: 0 - success; otherwise error code;
 * notes:
 */
int hw_ps_init(char *cfg_path)
{
    char gpio_path[64] = {0};
    struct stat st = {0};

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

    if (0 == strncmp("saradc", photosensitive_port, 6))
    {
        /* ain mode */
        hw_ps_ctrl.mode = HARDWARE_PS_AIN;

        int ain_id = atoi(&photosensitive_port[6]);

        switch (ain_id)
        {
        case 0:
            strcpy(hw_ps_ctrl.ain_file, PS_AIN0_FILE_NAME);
            break;

        case 1:
            strcpy(hw_ps_ctrl.ain_file, PS_AIN1_FILE_NAME);
            break;

        case 2:
            strcpy(hw_ps_ctrl.ain_file, PS_AIN2_FILE_NAME);
            break;

        default:
            ak_print_error(MODULE_ID_APP, "ain id err.\n");
            return ERROR_TYPE_DEV_CTRL_FAILED;
        }

        //day-level
        if (0 == strncmp("low", photosensitive_day_level, 3))
            hw_ps_ctrl.day_lv = 0;
        else
            hw_ps_ctrl.day_lv = 1;

        /* higher-level */
        hw_ps_ctrl.h_lv = photosensitive_higher_level;


        /* lower-level */
        hw_ps_ctrl.l_lv = photosensitive_lower_level;

    }
    else
    {
        /* gpio mode */
        hw_ps_ctrl.mode = HARDWARE_PS_GPIO;
        strcpy(hw_ps_ctrl.gpio_id, photosensitive_port);

        memset(gpio_path, 0, 64);
        sprintf(gpio_path, "/sys/class/gpio/gpio%s", photosensitive_port);

        if (-1 == stat(gpio_path, &st))
        {
            if (file_write_value("/sys/class/gpio/export", photosensitive_port))
            {
                ak_print_error(MODULE_ID_APP, "gpio %s can't set\n", photosensitive_port);
                return ERROR_TYPE_DEV_CTRL_FAILED;
            }
        }

        /* input */
        memset(gpio_path, 0, 64);
        sprintf(gpio_path, "/sys/class/gpio/gpio%s/direction", photosensitive_port);

        if (file_write_value(gpio_path, "in"))
        {
            ak_print_error(MODULE_ID_APP, "gpio %s can't set\n", photosensitive_port);
            return ERROR_TYPE_DEV_CTRL_FAILED;
        }

        memset(gpio_path, 0, 64);
        sprintf(gpio_path, "/sys/class/gpio/gpio%s/drive", photosensitive_port);

        if (file_write_value(gpio_path, "3"))
        {
            ak_print_error(MODULE_ID_APP, "gpio %s can't set\n", photosensitive_port);
            return ERROR_TYPE_DEV_CTRL_FAILED;
        }

        memset(gpio_path, 0, 64);
        sprintf(gpio_path, "/sys/class/gpio/gpio%s/pull_polarity", photosensitive_port);

        if (file_write_value(gpio_path, "pullup"))
        {
            ak_print_error(MODULE_ID_APP, "gpio %s can't set\n", photosensitive_port);
            return ERROR_TYPE_DEV_CTRL_FAILED;
        }

        memset(gpio_path, 0, 64);
        sprintf(gpio_path, "/sys/class/gpio/gpio%s/pull_enable", photosensitive_port);

        if (file_write_value(gpio_path, "1"))
        {
            ak_print_error(MODULE_ID_APP, "gpio %s can't set\n", photosensitive_port);
            return ERROR_TYPE_DEV_CTRL_FAILED;
        }

        /*input enable 1 */
        memset(gpio_path, 0, 64);
        sprintf(gpio_path, "/sys/class/gpio/gpio%s/input_enable", photosensitive_port);

        if (file_write_value(gpio_path, "1"))
        {
            ak_print_error(MODULE_ID_APP, "gpio %s can't set\n", photosensitive_port);
            return ERROR_TYPE_DEV_CTRL_FAILED;
        }

        memset(gpio_path, 0, 64);
        sprintf(gpio_path, "/sys/class/gpio/gpio%s/slew_rate", photosensitive_port);

        if (file_write_value(gpio_path, "fast"))
        {
            ak_print_error(MODULE_ID_APP, "gpio %s can't set\n", photosensitive_port);
            return ERROR_TYPE_DEV_CTRL_FAILED;
        }

        /*day-level */
        if (0 == strncmp("low", photosensitive_day_level, 3))
            hw_ps_ctrl.day_lv = 0;
        else
            hw_ps_ctrl.day_lv = 1;
    }

    hw_ps_ctrl.init_flag = 1;
    return AK_SUCCESS;
}

/*
 * hw_ps_get_level - hw_ps_get_level
 * day_night_lv[OUT]: day_night_lv
 * return: 0 - success; otherwise error code;
 * notes:
 */
int hw_ps_get_level(int *day_night_lv)
{
    char ain_lv[8] = {0};
    char gpio_value[8] = {0};
    char gpio_path[64] = {0};
    int value = 0;

    if (NULL == day_night_lv)
    {
        ak_print_error(MODULE_ID_APP, "param is null.\n");
          return ERROR_TYPE_POINTER_NULL;
    }

    if (!hw_ps_ctrl.init_flag)
    {
        ak_print_error(MODULE_ID_APP, "hw ps is not inited.\n");
          return ERROR_TYPE_NOT_INIT;
    }

    if (HARDWARE_PS_AIN == hw_ps_ctrl.mode)
    {
        /*read ain level */
        if (file_read_value(hw_ps_ctrl.ain_file, ain_lv, 8))
        {
            ak_print_error(MODULE_ID_APP, "%s can't read\n", hw_ps_ctrl.ain_file);
            return ERROR_TYPE_DEV_CTRL_FAILED;
        }

        value = atoi(ain_lv);

        if (hw_ps_ctrl.day_lv)
        {
            /*day level is high */
            if (value >= hw_ps_ctrl.h_lv)
            {
                *day_night_lv = STATE_DAY;
                hw_ps_ctrl.pre_status = STATE_DAY;
            }
            else if (value <= hw_ps_ctrl.l_lv)
            {
                *day_night_lv = STATE_NIGHT;
                hw_ps_ctrl.pre_status = STATE_NIGHT;
            }
            else
            {
                *day_night_lv = hw_ps_ctrl.pre_status;
            }
        }
        else
        {
            /*day level is low */
            if (value >= hw_ps_ctrl.h_lv)
            {
                *day_night_lv = STATE_NIGHT;
                hw_ps_ctrl.pre_status = STATE_NIGHT;
            }
            else if (value <= hw_ps_ctrl.l_lv)
            {
                *day_night_lv = STATE_DAY;
                hw_ps_ctrl.pre_status = STATE_DAY;
            }
            else
            {
                *day_night_lv = hw_ps_ctrl.pre_status;
            }
        }
    }
    else
    {
        sprintf(gpio_path, "/sys/class/gpio/gpio%s/value", hw_ps_ctrl.gpio_id);
        /*read gpio value */
        if (file_read_value(hw_ps_ctrl.ain_file, gpio_value, 8))
        {
            ak_print_error(MODULE_ID_APP, "%s can't read\n", hw_ps_ctrl.ain_file);
            return ERROR_TYPE_DEV_CTRL_FAILED;
        }

        value = atoi(gpio_value);

        if (value == hw_ps_ctrl.day_lv)
            *day_night_lv = STATE_DAY;
        else
            *day_night_lv = STATE_NIGHT;
    }

    return AK_SUCCESS;
}

/**
 * set_video_day_night: set video day or night mode, according to IR value
 * @vi_handle: opened vi handle
 * @ir_val: IR value, [0, 1]
 * @day_level: day control level, [1, 4]
 * return: 0 success, -1 failed
 */
static int set_video_day_night(int dev, int ir_val)
{
    int ret = AK_FAILED;
    if (0 == ir_val) {
        ak_print_notice_ex(MODULE_ID_APP, "now set to day\n");
        /*set ircut before switch isp config to day */
        ak_drv_ir_set_ircut(0);
        ret = ak_vi_switch_mode(ps_ctrl.dev_id, VI_MODE_DAY_OUTDOOR);
        /*set irled after switch isp config to day */
        ak_drv_irled_set_working_stat(0);
    } else {
        ak_print_notice_ex(MODULE_ID_APP, "now set to night\n");
        /*set irled before switch isp config to day */
        ak_drv_irled_set_working_stat(1);
        ret = ak_vi_switch_mode(ps_ctrl.dev_id, VI_MODE_NIGHTTIME);
        /*set ircut after switch isp config to night */
        ak_drv_ir_set_ircut(1);
    }

    ak_sleep_ms(300);

    return ret;
}

/**
 * switch_day_or_night: switch_day_or_night
 * @mode: mode , day or night
 * return: 0 success, -1 failed
 */
static int switch_day_or_night(enum mode_state mode)
{
    ak_print_normal_ex(MODULE_ID_APP, "(switch day_night)cur_status=%s\n", \
        mode == STATE_DAY ? "day" : "night");
    /* disable md */

    int ret = set_video_day_night(ps_ctrl.dev_id, mode);
    ps_ctrl.pre_status = mode;	/* store new state */
    ak_sleep_ms(3000);
    /* enable md */
    return ret;
}

/*
 * photosensitive_switch_th - according to photosensitive status
 *                            to switch ircut and video
 */
static void *photosensitive_switch_th(void *arg)
{
    long int tid = ak_thread_get_tid();
    int ir_val = 0;

    ak_print_normal_ex(MODULE_ID_APP, "Thread start, id: %ld\n", tid);
    ak_thread_set_name("PS_switch");

    ps_ctrl.thread_run = AK_TRUE;
    /* get ir state and switch day-night */
    while (ps_ctrl.thread_run) {
        ak_print_info_ex(MODULE_ID_APP, "ps_ctrl.cur_set_mode=%d\n", \
            ps_ctrl.cur_set_mode);
        if (ps_ctrl.cur_set_mode != SET_AUTO_MODE)
            break;

        hw_ps_get_level(&ir_val);

        if (ir_val != -1 && ps_ctrl.pre_status != ir_val) {
            ak_print_info_ex(MODULE_ID_APP, "prev_state=%d, ir_val=%d\n",
                ps_ctrl.pre_status, ir_val);
            set_video_day_night(ps_ctrl.dev_id, ir_val);
            ps_ctrl.pre_status = ir_val;
        }

        ak_sleep_ms(5000);
    }

    ak_print_normal_ex(MODULE_ID_APP, "Thread exit, id: %ld\n", tid);
    ak_thread_exit();
    return NULL;
}


/*
 * ir_auto_switch_th - switch ircut and led by auto
 */
static void *ir_auto_switch_th(void *arg)
{
    long int tid = ak_thread_get_tid();
    int cur_status = -1;
    int ret = -1;

    ak_print_normal_ex(MODULE_ID_APP, "Thread start, id: %ld\n", tid);
    ak_thread_set_name("PS_switch");

    ak_print_normal_ex(MODULE_ID_APP, "(switch day_night) cur_set_mode=%d\n", \
        ps_ctrl.cur_set_mode);

    ps_ctrl.thread_run = AK_TRUE;

    /* get ir state and switch day-night */
    while (ps_ctrl.thread_run) {
        if (ps_ctrl.cur_set_mode != SET_AUTO_MODE)
            break;
        /* check day night mode */
        ret = ak_vpss_get_auto_day_night_level(ps_ctrl.dev_id, \
            ps_ctrl.pre_status, &cur_status);
        if (ret) {
            ak_print_error_ex(MODULE_ID_APP, "ak_vpss_isp_get_input_level failed.\n");
            continue;
        }

        if (ps_ctrl.pre_status != cur_status) {
            if (STATE_DAY == cur_status )
                switch_day_or_night(STATE_DAY);
            else
                switch_day_or_night(STATE_NIGHT);
        }
        ak_sleep_ms(100);

    }

    ak_print_normal_ex(MODULE_ID_APP, "Thread exit, id: %ld\n", tid);
    ak_thread_exit();

    return NULL;
}

const char* ak_ps_get_version(void)
{
    return ps_version;
}

/**
 * ak_ps_start: start photosensitive switch
 * @dev: dev id
 * @ps_mode: photosensitive mode
 * @day_night_mode: day night switch mode
 * @cfg_path[IN]:  ps ir config path
 * return: 0 success, -1 failed
 */
int ak_ps_start(int dev, enum ak_photosensitive_mode ps_mode,
                  enum day_night_switch_mode day_night_mode, char *cfg_path)
{
    /* check params */
    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_VPSS, "dev:%d error!\n", dev);
        return -1;
    }

    if (ps_mode > AUTO_PHOTOSENSITIVE)
    {
        ak_print_error_ex(MODULE_ID_VPSS, "ps_mode:%d error!\n", ps_mode);
        return -1;
    }

    if (day_night_mode > SET_AUTO_MODE)
    {
        ak_print_error_ex(MODULE_ID_VPSS, "day_night_mode:%d error!\n", day_night_mode);
        return -1;
    }

    if (NULL == cfg_path)
    {
        ak_print_error_ex(MODULE_ID_DRV, "cfg_path NULL.\n");
        return -1;
    }

    /* already start */
    if (ps_ctrl.ircut_run_flag) {
        ak_print_error_ex(MODULE_ID_APP, "misc already start\n");
        return AK_SUCCESS;
    }

    /* ircut init */
    ak_drv_ir_init(cfg_path);

    /* irled init */
    struct ak_drv_irled_hw_param irled_param = {0};
    irled_param.irled_working_level = 1;
    ak_drv_irled_init(&irled_param);

    ps_ctrl.ircut_run_flag = AK_TRUE;

    ps_ctrl.cur_set_mode = day_night_mode;
    ps_ctrl.ps_mode = ps_mode;
    ps_ctrl.dev_id = dev;

    int ret = AK_SUCCESS;

    if (ps_ctrl.cur_set_mode == SET_DAY_MODE) {
        /* day */
        ps_ctrl.pre_status = STATE_DAY;
        ps_ctrl.cur_set_mode = SET_DAY_MODE;
        switch_day_or_night(STATE_DAY);
    } else if (ps_ctrl.cur_set_mode == SET_NIGHT_MODE) {
        /*night */
        ps_ctrl.pre_status = STATE_NIGHT;
        ps_ctrl.cur_set_mode = SET_NIGHT_MODE;
        switch_day_or_night(STATE_NIGHT);
    } else if (ps_ctrl.cur_set_mode == SET_AUTO_MODE) {
        /* auto */
        ps_ctrl.pre_status = STATE_DAY;
        ps_ctrl.cur_set_mode = SET_AUTO_MODE;
        switch_day_or_night(STATE_DAY);
        if (NULL == ps_ctrl.ps_fun_thread) {
            if (ps_mode == HARDWARE_PHOTOSENSITIVE)
            {
                if (hw_ps_init(cfg_path))
                {
                    ak_print_error_ex(MODULE_ID_APP, "hw_ps_init failed!\n");
                    return AK_FAILED;
                }
                ps_ctrl.ps_fun_thread = photosensitive_switch_th;
            }
            else
                ps_ctrl.ps_fun_thread = ir_auto_switch_th;
        }
        ret = ak_thread_create(&ps_ctrl.ircut_tid, ps_ctrl.ps_fun_thread,
            NULL, 100 *1024, -1);
    }
    return ret;
}

/**
 * ak_ps_stop - stop ircut auto switch
 * return: 0 success, -1 failed
 * notes:
 */
void ak_ps_stop(void)
{
    ps_ctrl.ircut_run_flag = AK_FALSE;
    ps_ctrl.thread_run = AK_FALSE;
    ak_print_normal_ex(MODULE_ID_APP, "set pircut_th_runflag to 0\n");

    if (ps_ctrl.cur_set_mode == SET_AUTO_MODE) {
        ak_print_normal_ex(MODULE_ID_APP, "join photosensitive switch thread...\n");
        ak_thread_join(ps_ctrl.ircut_tid);
        ak_print_normal_ex(MODULE_ID_APP, "photosensitive switch thread join OK\n");
    }
}

/**
 * ak_ps_set_ain_th: set ain threshold
 * @th_high: threshold high level
 * @th_low: threshold low level
 * return: 0 success, -1 failed
 */
 int ak_ps_set_ain_th(int th_high, int th_low)
{
    if (!hw_ps_ctrl.init_flag)
    {
        ak_print_error(MODULE_ID_APP, "hw ps is not inited.\n");
          return ERROR_TYPE_NOT_INIT;
    }

    if (th_high > 0)
        hw_ps_ctrl.h_lv = th_high;

    if (th_low > 0)
        hw_ps_ctrl.l_lv = th_low;

    ak_print_normal_ex(MODULE_ID_APP, "set ain th_high %d, th_low %d\n", \
        hw_ps_ctrl.h_lv, hw_ps_ctrl.l_lv);

    return AK_SUCCESS;
}

