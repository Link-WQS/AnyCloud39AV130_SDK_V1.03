/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_sps2_ir.c
* Description: This is a simple example to show how the soft photosensitive (new method) module working.
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
#include "ak_sps2_ir.h"
#include "ak_conf.h"

enum ak_photosensitive_mode {
    HARDWARE_PHOTOSENSITIVE,	// hardware photosensitive
    AUTO_PHOTOSENSITIVE		// auto ir switch
};

enum day_night_switch_mode {
    SET_DAY_MODE,
    SET_NIGHT_MODE,
    SET_AUTO_MODE,
    SET_CLOSE_MODE
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
    int dev_cnt;
};

static struct ak_ps ps_ctrl = {0};

static const char *sps2_version = "libapp_sps2 V1.0.00";


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
    int i = 0;

    if (0 == ir_val) {
        ak_print_notice_ex(MODULE_ID_APP, "now set to day\n");
        /*set ircut before switch isp config to day */
        ak_drv_ir_set_ircut(0);
        for (i=0; i<ps_ctrl.dev_cnt; i++)
        {
            ret = ak_vi_switch_mode(i, VI_MODE_DAY_OUTDOOR);
        }
        /*set irled after switch isp config to day */
        ak_drv_irled_set_working_stat(0);
    } else {
        ak_print_notice_ex(MODULE_ID_APP, "now set to night\n");
        /*set irled before switch isp config to day */
        ak_drv_irled_set_working_stat(1);
        for (i=0; i<ps_ctrl.dev_cnt; i++)
        {
            ret = ak_vi_switch_mode(i, VI_MODE_NIGHTTIME);
        }
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
        ret = ak_vpss_get_soft_ps_level(ps_ctrl.dev_id, \
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

/**
 * ak_ps_start: start photosensitive switch
 * @dev: dev id
 * @cfg_path[IN]:  ps ir config path
 * return: 0 success, -1 failed
 */
int ak_sps2_start(int dev, char *cfg_path, int dev_cnt)
{
    /* check params */
    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_VPSS, "dev:%d error!\n", dev);
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

    ps_ctrl.cur_set_mode = SET_AUTO_MODE;
    ps_ctrl.ps_mode = AUTO_PHOTOSENSITIVE;
    ps_ctrl.dev_id = dev;
    ps_ctrl.dev_cnt = dev_cnt;

    int ret = AK_SUCCESS;

    /* auto */
    ps_ctrl.pre_status = STATE_DAY;
    ps_ctrl.cur_set_mode = SET_AUTO_MODE;
    switch_day_or_night(STATE_DAY);
    if (NULL == ps_ctrl.ps_fun_thread) {
        ps_ctrl.ps_fun_thread = ir_auto_switch_th;
    }
    ret = ak_thread_create(&ps_ctrl.ircut_tid, ps_ctrl.ps_fun_thread,
        NULL, 100 *1024, -1);

    return ret;
}

/**
 * ak_ps_stop - stop ircut auto switch
 * return: 0 success, -1 failed
 * notes:
 */
void ak_sps2_stop(void)
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

