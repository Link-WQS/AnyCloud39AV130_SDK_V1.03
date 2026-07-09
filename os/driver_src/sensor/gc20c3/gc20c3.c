/*
 * gc20c3 Camera Driver
 *
 * Copyright anyka
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
//#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/v4l2-mediabus.h>
#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-image-sizes.h>
#include <media/v4l2-subdev.h>

#include "ak_isp_drv.h"
#include "ak_video_priv_cmd.h"
#include "ak_video_priv_cmd_internal.h"

#if ( !defined(CONFIG_MACH_KM01A) && !defined(CONFIG_MACH_AK3918AV130) && defined(CONFIG_SYS_FAST_LAUNCH) )
//#ifdef CONFIG_SYS_FAST_LAUNCH
#include "ak_sensor.h"
#include "sensor_sys.h"
#include "sensor_i2c.h"
#else
#include "../common/sensor_i2c.h"
#include "../common/sensor_sys.h"
#include "../include/ak_sensor.h"
#endif
#include "ak_sensor_priv_cmd.h"
#ifdef CONFIG_MACH_KM01A
#include "gc20c3_1920x1080.h"
#endif

/*
 *
SENSOR_PWDN_LEVEL:  the io level when sensor power down
SENSOR_RESET_LEVEL: the io level when sensor reset
SENSOR_I2C_ADDR:    the sensor slave address, 7 bits width.
                    the i2c bus left shift 1bits.
SENSOR_ID:  the sensor ID.
SENSOR_MCLK:    mclk, unit：MHz。24MHz or 27MHz mclk for most sensors.
                NOTE: 27MHz mclk is inaccurate.
SENSOR_REGADDR_BYTE:    sensor register address bytes width, 1 byte or 2 bytes.
SENSOR_DATA_BYTE:       register's value bytes width, 1 byte or 2bytes.
SENSOR_OUTPUT_WIDTH:    use for isp cut window MAX width.
SENSOR_OUTPUT_HEIGHT:   use for isp cut window MAX height.
SENSOR_VALID_OFFSET_X:  use for isp cut window left offset.
SENSOR_VALID_OFFSET_Y:  use for isp cut window top offset.
SENSOR_BUS_TYPE:        sensor output data type.
SENSOR_IO_INTERFACE:    sensor output data bus type.
SENSOR_IO_LEVEL:        sensor output data io level.
MAX_FPS:                current sensor configurtion support max fps.
EXP_EFFECT_FRAMES:      !!IMPORT!! frequency for update exp_time
        0 - adjust exp_time for every frames
        1 - adjust exp_time for 1/2 frames
        2 - adjust exp_time for 1/3 frames
        and so on.
A_GAIN_EFFECT_FRAMES:   !!IMPORT!! frequency for update a_gain
        0 - adjust a_gain for every frames
        1 - adjust a_gain for 1/2 frames
        2 - adjust a_gain for 1/3 frames
        and so on.
D_GAIN_EFFECT_FRAMES:   !!IMPORT!! frequency for update d_gain
        0 - adjust d_gain for every frames
        1 - adjust d_gain for 1/2 frames
        2 - adjust d_gain for 1/3 frames
        and so on.
DELAY_FLAG
 *
 * */
#define SENSOR_PWDN_LEVEL     0
#define SENSOR_RESET_LEVEL    0
#define SENSOR_POWER_LEVEL    1
static int SENSOR_I2C_ADDR    = 0x31; //addr_sel=0:addr=0x31; addr_sel=1:addr=0x10
//#define ACTUAL_SENSOR_ID    0x4033//初版，PreliminaryV0.0
#define ACTUAL_SENSOR_ID_A    0x2003 //试产版本
#define ACTUAL_SENSOR_ID_B    0x20c3 //正式版本
#define SENSOR_ID             0x20c3
#define SENSOR_MCLK           24
#define SENSOR_REGADDR_BYTE   2
#define SENSOR_DATA_BYTE      1
#define SENSOR_BINING_WIDTH   960
#define SENSOR_BINING_HEIGHT  540
#define SENSOR_OUTPUT_WIDTH   1920
#define SENSOR_OUTPUT_HEIGHT  1080
#define SENSOR_REAL_HEIGHT    SENSOR_OUTPUT_HEIGHT //sensor实际输出行数
#define SENSOR_SYNC_DROP_FRAMES 2 //使用强制同步方式，需要丢帧。
#define SENSOR_VALID_OFFSET_X 0
#define SENSOR_VALID_OFFSET_Y 0
#define SENSOR_BUS_TYPE       BUS_TYPE_RAW
//#define SENSOR_IO_INTERFACE       DVP_INTERFACE
#define SENSOR_IO_INTERFACE   MIPI_INTERFACE
#define SENSOR_IO_LEVEL       IO_LEVEL_1V8
#define MIPI_MBPS             372
#define MIPI_LANES_SWAP      (0x435A1)//dcddcd
#define MIPI_LANES            2
#define RAW_FORMAT 2              //raw分量的顺序
static int MAX_FPS            = 30;
#define MAX_METERING_FPS      (30)
#if defined(CONFIG_MACH_AK39EV330) || defined(CONFIG_MACH_AK37D) /*H3B&H3D*/
#define EXP_EFFECT_FRAMES    1 /*adjust exp_time for 1/2 frames*/
#define A_GAIN_EFFECT_FRAMES 0 /*adjust a_gain for 1/2 frames*/
#define D_GAIN_EFFECT_FRAMES 0 /*adjust d_gain for 1/2 frames*/
#else /*H322&322L*/
#define EXP_EFFECT_FRAMES    1 /*adjust exp_time for 1/2 frames*/
#define A_GAIN_EFFECT_FRAMES 1 /*adjust a_gain for 1/2 frames*/
#define D_GAIN_EFFECT_FRAMES 1 /*adjust d_gain for 1/2 frames*/
#endif


//****************** REG *********************/
#define ID_REG_H    	(0x03f0)
#define ID_REG_L    	(0x03f1)
#define VTS_REG_H   	(0x0340)
#define VTS_REG_L   	(0x0341)
#define HTS_REG_H   	(0x0342)
#define HTS_REG_L   	(0x0343)

#define FLIP_MIRROR_REG (0x022c)
#define WIN_OFF_Y_L		(0x347)
#define WIN_OFF_X_L		(0xb0d)

#define EXP_REG_H		(0x0202)
#define EXP_REG_L		(0x0203)

/**************** DEBUG MARCO*****************/
#define SET_FPS_EN 		(1)			//no used,debug for
#define SET_EXP_EN 		(1)
#define SET_AGAIN_EN 	(1)
//  #define SET_DGAIN_EN 	(0)

#define SENSOR_MIN_EXP	2 /*gc20c3_Datasheet_V1.4.Watermark.pdf*/
#define EXP_DECREASE_LINES  8// used to calculate the maximum exposure value
#define EXP_MIN        (5)

#define ONE_LINE_EXP_OF_STEPS (1)

//  #define MAX_EXP_OF_FPS120   (559)
#define MAX_EXP_OF_FPS30 (1142)  //单位为曝光行
#define MAX_A_GAIN_X (4067 / 64) //(32*(2000/64))
//  #define MAX_D_GAIN_X        (2000/64)
#define MAX_D_GAIN_X (1)
//#define ONE_LINE_CYCLE (8750)
//#define HBLIANK_CYCLE (3760)
#define ONE_LINE_CYCLE (4586)
#define HBLIANK_CYCLE (746)
//  #define EXP_BINING2NORMAL   (2*100) //重要参数，小窗切大窗时，曝光需除2

#ifndef aov_slave_mode
typedef enum {
    E_SENSOR_POWER_OFF = 0,
    E_SENSOR_POWER_ON,
    E_SENSOR_INIT,
    E_SENSOR_WORK,
    E_SENSOR_STANDBY,
    E_SENSOR_ACCESS_STANDBY,
    E_SENSOR_PROBE_FAILED = 0xffff,
} E_SENSOR_STATUS;

#define UPDATE_SENSOR_STATUS(s)     (priv->status = (s))
#define IS_SENSOR_STATUS(s)         (priv->status == (s))
#endif

#ifdef aov_slave_mode
static int aov_master_mode = 0;//aov mode is slave
#else
static int aov_master_mode = 1;//aov mode is master
#endif

/*
 * Struct
 */
struct ak_sensor_param
{
    unsigned int target_vts;
    unsigned int reg_hts;
    unsigned int reg_vts;
    unsigned int pclk_freq;
    unsigned int current_fps;
    unsigned int to_fps;
    unsigned int current_exp;
    unsigned int flip_mirror;
    unsigned int framecount;
};
#ifdef CONFIG_SYS_FAST_LAUNCH
//  传感器 AE 转换系数组
static ae_convert_coef_t gc20c3_ae_convert_coef_group[] = {
    {1, 1},    // 第0组：默认系数（无转换）
    {1, 1},    // 第1组：1/2 转换
    {1, 1},    // 第2组：2/3 转换
};
#define GC20C3_AE_CONVERT_GROUP_NUM  (sizeof(gc20c3_ae_convert_coef_group) / sizeof(ae_convert_coef_t))

//  传感器 AWB 转换系数组
static awb_convert_coef_t gc20c3_awb_convert_coef = {
    .r = { .mul = 1,.div = 1 },
    .g = { .mul = 1,.div = 1 },
    .b = { .mul = 1,.div = 1 },
};
#define GC20C3_AWB_CONVERT_GROUP_NUM  (1)

#endif

struct regval_list {
    u8 reg_num;
    u8 value;
};

struct gc20c3_win_size {
    char* name;
    u32 width;
    u32 height;
    const struct regval_list* regs;
};

/*
 * store sensor fps informations
 * @current_fps: current fps
 * @to_fps: should changed to the fps
 * @to_fps_value: the fps paramter
 */
struct sensor_fps_info {
    int current_fps;
    int to_fps;
    int to_fps_value;
    int reg_fps_value;
    int video_fps;
    int aov_fps;
};

/*
 * the host callbacks for sensor
 * @isp_timing_cb_info: isp timing callback.
 *  a few sensor need the callback when fps had changed
 */
struct host_callbacks {
    struct isp_timing_cb_info isp_timing_cb_info;
};

/*
 * aec paramters. some sensors need record paramters.
 */
struct gc20c3_aec_parms {
    int auto_off;
    int off_x;
    int off_y;
    int temp_count;
    int backup_temp_flag;
    int curr_again_level;
    int curr_again_10x;
    int r0x3e02_value;
    int r0x3e01_value;
    int r0x3e00_value;
    int curr_2x_dgain;
    int curr_corse_gain;
    int target_exp_ctrl;

    int reg_frame_hts;
    int reg_frame_vts;
    int pclk_freq;
    int calc_vts_tmp;
#ifdef CONFIG_MACH_KM01A
    int a_gain;
    int d_gain;
#endif
};

static const struct v4l2_ctrl_ops gc20c3_ctrl_ops;
static const struct v4l2_ctrl_config config_sensor_get_id = {
    .ops = &gc20c3_ctrl_ops,
    .id = SENSOR_GET_ID,
    .name = "get sensor id",
    .type = V4L2_CTRL_TYPE_INTEGER,
    .min = 0,
    .max = 0xfffffff,
    .step = 1,
    .def = 0,
    .flags
    = V4L2_CTRL_FLAG_VOLATILE, /*set V4L2_CTRL_FLAG_VOLATILE avoid get cached*/
};

/*
 * the sensor struct
 * @client: point to i2c client
 * @subdev: v4l2 subdev struct
 * @hdl:    v4l2 control handler
 * @cfmt_code:
 * @win:    default window
 * @gpio_reset: gpio number for reset pin
 * @gpio_pwdn:  gpio number for power down pin
 * @cb_info:    sensor callbacks
 * @fps_info:   fps informations
 * @hcb:        host callbacks
 * @aec_parms:  aec paramters
 */
struct gc20c3_priv {
    struct list_head list;
    struct i2c_client* client;
    struct v4l2_subdev subdev;
    struct v4l2_ctrl_handler hdl;
    u32 cfmt_code;
    const struct gc20c3_win_size *win;

    int gpio_reset;
    int gpio_pwdn;
#ifdef CONFIG_MACH_KM01A
    int gpio_fsync;
    int gpio_power;
#endif
    struct sensor_cb_info cb_info;
    struct sensor_fps_info fps_info;

    struct host_callbacks hcb;
    struct gc20c3_aec_parms aec_parms;

    AK_ISP_SENSOR_INIT_PARA para;
#ifdef CONFIG_MACH_KM01A
    AK_ISP_SENSOR_INIT_PARA aov_para;
    unsigned int flip_mirror;
    unsigned int init_flag;
#ifndef aov_slave_mode
    E_SENSOR_STATUS status;
#endif
    unsigned int fast_fps_flag;
    unsigned int first_fast_fps_flag;
    unsigned int binnig_mode;


    int reg_slave_mode;
#endif
    struct ae_fast_struct ae_fast;
    ae_awb_dynamic_map_policy_t ae_awb_dynamic_map_policy;
    int slave_sync_mode ;
    struct ak_sensor_param sensor_param;
    struct v4l2_ctrl* ctrl_sensor_get_id;
    /*for 双目CIS 1. 在 CIS 的私有结构体内添加链表*/
    enum sensor_master_or_slave_mode master_or_slave;
};

/* 2. 添加一个全局变量的链表头 */
static LIST_HEAD(privs_list);

/*default aec of fast boot*/
static struct ae_fast_struct ae_fast_default = { .sensor_exp_time = 896,
    .sensor_a_gain = 256,
    .sensor_d_gain = 256,
    .isp_d_gain = 256,
    { .r_gain = 590,
        .g_gain = 256,
        .b_gain = 515,
        .r_offset = 0,
        .g_offset = 0,
        .b_offset = 0 } };

static AK_ISP_SENSOR_CB gc20c3_cb;

/*
 * check_id - check hardware sensor ID whether it meets this driver
 * 0-no check, force to meet
 * others-check, if not meet return fail
 */
static int check_id = 0;
static int dvp = 0;
module_param(check_id, int, 0644);
module_param(dvp, int, 0644);

/* 3. 添加两个入参数变量用于获取硬件上对应的 device address */
static int addr0 = 0, addr1 = 0;
module_param(addr0, int, 0644);
module_param(addr1, int, 0644);
module_param(SENSOR_I2C_ADDR, int, 0644);
module_param(MAX_FPS, int, 0644);
#define VAL_IS_SLAVE_MODE(x) ((x) == 2)
#define STORE_CONFIG_IS_AOV_MODE(priv) (VAL_IS_SLAVE_MODE((priv)->reg_slave_mode))

static int slave_sync_mode = DUAL_SYNC_BY_EFSYNC;
module_param(slave_sync_mode, int, 0644);
static int sensor_power_on = 0;
module_param(sensor_power_on, int, 0644);
static void gc20c3_set_fps_async(struct gc20c3_priv* priv);
static int __gc20c3_sen_probe_id_func(
    struct i2c_client* client);                // use IIC bus
static int gc20c3_sen_read_id_func(void* arg); // no use IIC bus
static int gc20c3_sen_get_resolution_func(
    void* arg, int *width, int *height);
static int gc20c3_sen_set_fps_func(void* arg, const int fps);
static int gc20c3_fps_to_vts(struct gc20c3_priv* priv, const int fps);
static int call_sensor_sys_init(void);
static int call_sensor_sys_deinit(void);
static int gc20c3_write(const struct i2c_client* client, int reg, int value);
static int gc20c3_read(const struct i2c_client* client, int reg);
static int gc20c3_sen_init_func(void* arg, const AK_ISP_SENSOR_INIT_PARA *npara);
static int gc20c3_sen_set_power_on_func(void* arg);
static int gc20c3_sen_get_parameter_func(void* arg, int param, void* value);
#if (defined(CONFIG_SYS_FAST_LAUNCH) || defined(CONFIG_MACH_KM01A))
static int get_ae_fast_default(void* arg,struct ae_fast_struct* ae_fast);
#else
static int get_ae_fast_default(struct ae_fast_struct* ae_fast);
#endif


static void ak_sensor_write_init(struct gc20c3_priv* priv);
static int gc20c3_sen_updata_exp_time_func(void* arg, unsigned int exp_time);
static int gc20c3_sen_update_a_gain_func(void* arg, const unsigned int a_gain);
static int __set_reg_exp_time(struct gc20c3_priv* priv, int exp_time);

//static int backup_def0_info(struct gc20c3_priv* priv,
//            unsigned short reg_addr,unsigned short value);

struct _sensor_bining_info
{
    unsigned int width;
    unsigned int height;
};

#ifdef CONFIG_SYS_FAST_LAUNCH
extern int sys_sensor_init_set(void *init_func, void* arg, void *param_func, int sensor_id);
#endif

static u32 gc20c3_codes[] = {
    MEDIA_BUS_FMT_YUYV8_2X8,
    MEDIA_BUS_FMT_UYVY8_2X8,
    MEDIA_BUS_FMT_RGB565_2X8_BE,
    MEDIA_BUS_FMT_RGB565_2X8_LE,
};

/*
 * General functions
 */
static struct gc20c3_priv* to_gc20c3(const struct i2c_client* client)
{
    return container_of(
        i2c_get_clientdata(client), struct gc20c3_priv, subdev);
}

static struct v4l2_subdev* ctrl_to_sd(struct v4l2_ctrl* ctrl)
{
    return &container_of(ctrl->handler, struct gc20c3_priv, hdl)->subdev;
}

/*
 * XXXX_s_stream -
 * set steaming enable/disable
 * soc_camera_ops functions
 *
 * @sd:             subdev
 * @enable:         enable flags
 */
static int gc20c3_s_stream(struct v4l2_subdev* sd, int enable) { return 0; }

static int gc20c3_g_volatile_ctrl(struct v4l2_ctrl* ctrl)
{
    struct v4l2_subdev* sd = ctrl_to_sd(ctrl);
    int ret = 0;

    switch (ctrl->id) {
        case SENSOR_GET_ID:
            ctrl->val = SENSOR_ID;
            break;

        default:
            ret = -EINVAL;
            //pr_debug("%s id:%d no support\n", __func__, ctrl->id);
            pr_err("%s id:%d no support\n", __func__, ctrl->id);
            break;
    }

    return ret;
}

/*
 * XXXX_s_stream -
 * set ctrls
 * soc_camera_ops functions
 *
 * @ctrl:           pointer to ctrl
 */
static int gc20c3_s_ctrl(struct v4l2_ctrl* ctrl) { return 0; }

/*
 * XXXX_s_power -
 * set power operation
 * soc_camera_ops functions
 *
 * @sd:         subdev
 * @on:         power flags
 */
static int gc20c3_core_s_power(struct v4l2_subdev* sd, int on) { return 0; }

/*
 * XXXX_get_sensor_id -
 * get sensor ID
 * private callback functions
 *
 * @ctrl:           pointer to ctrl
 */
static int gc20c3_get_sensor_id(struct v4l2_control* ctrl)
{
    ctrl->value = gc20c3_sen_read_id_func(NULL); // no use IIC bus
    return 0;
}

/*
 * XXXX_get_sensor_id -
 * get sensor callback
 * private callback functions
 *
 * @sd:             subdev
 * @ctrl:           pointer to ctrl
 */
static int gc20c3_get_sensor_cb(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc20c3_priv* priv = to_gc20c3(client);

    ctrl->value = (int)&priv->cb_info;
    return 0;
}

static int gc20c3_get_max_exp_for_fps(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc20c3_priv* priv = to_gc20c3(client);
    int fps = ctrl->value;
    int vts = gc20c3_fps_to_vts(priv, fps);

    ctrl->value = vts - EXP_DECREASE_LINES;
    return 0;
}

/*
 * XXXX_core_g_ctrl -
 * get control
 * core functions
 *
 * @sd:             subdev
 * @ctrl:           pointer to ctrl
 */
static int gc20c3_core_g_ctrl(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    int ret;

    switch (ctrl->id) {
        case GET_SENSOR_ID:
            ret = gc20c3_get_sensor_id(ctrl);
            break;

        case GET_SENSOR_CB:
            ret = gc20c3_get_sensor_cb(sd, ctrl);
            break;

        case GET_MAX_EXP_FOR_FPS:
            ret = gc20c3_get_max_exp_for_fps(sd, ctrl);
            break;

#if 0 //def CONFIG_SYS_FAST_LAUNCH
//        case GET_INIT_AE_TABLE:
//            ret = _get_def_ae_table((AK_ISP_FAST_AE_INFO *)ctrl->value);
//            break;

        case GET_LIGHT_SENSOR_AE_TABLE:
            ret = get_light_sensor_ae_table((adc_ae_para_table_t *)ctrl->value);
            break;
#endif

#if 0 //TODO YJH
#if ( !defined(CONFIG_MACH_KM01A) && defined(CONFIG_SYS_FAST_LAUNCH) )
//#ifdef CONFIG_SYS_FAST_LAUNCH
       case GET_METERING_FPS:
           ctrl->value = MAX_METERING_FPS;
           ret = 0;
           break;
#endif
#endif

        default:
            //pr_debug("%s cmd:%d not support\n", __func__, ctrl->id);
            pr_err("%s cmd:%d not support\n", __func__, ctrl->id);
            ret = -1;
            break;
    }

    return ret;
}

/*
 * XXXX_set_isp_timing_cb -
 * set isp timing callback function
 * private callback functions
 *
 * @sd:             subdev
 * @ctrl:           pointer to ctrl
 */
static int gc20c3_set_isp_timing_cb(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc20c3_priv* priv = to_gc20c3(client);
    struct isp_timing_cb_info* isp_timing_cb_info = (void*)ctrl->value;

    memcpy(&priv->hcb.isp_timing_cb_info, isp_timing_cb_info,
        sizeof(struct isp_timing_cb_info));
    return 0;
}

/*
 * XXXX_set_fps_direct -
 * set sensor fps directly, synchronously.
 * private callback functions
 *
 * @sd:             subdev
 * @ctrl:           pointer to ctrl
 */
static int gc20c3_set_fps_direct(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc20c3_priv* priv = to_gc20c3(client);
    int fps = ctrl->value;

    //pr_err("%s %d fps:%d.\n", __func__ ,__LINE__, fps);
    gc20c3_sen_set_fps_func(priv, fps);
    gc20c3_set_fps_async(priv);

    priv->fps_info.video_fps = fps; //gc20c3 get fps in video stream
    return 0;
}

#ifdef CONFIG_MACH_KM01A
static int gc20c3_set_master_or_slave(struct v4l2_subdev* sd, int is_master, int aov_flags)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc20c3_priv* priv = to_gc20c3(client);

    pr_debug("%s is_master:%d\n", __func__, is_master);

    if (is_master)
        priv->master_or_slave = MASTER_MODE;
    else
        priv->master_or_slave = SLAVER_MODE;
    UPDATE_SENSOR_STATUS(E_SENSOR_WORK);

    if (aov_flags)
    {
        priv->master_or_slave = SLAVER_AOV_MODE;

        UPDATE_SENSOR_STATUS(E_SENSOR_INIT);
    }
    return 0;
}

static int ak_sensor_set_nomal_mode(struct v4l2_subdev* sd, int value)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc20c3_priv* priv = to_gc20c3(client);
    priv->fast_fps_flag = value;
    if (!priv->fast_fps_flag)
    { //normal mode
        MAX_FPS = 30;
    }
    pr_err("%s fast_fps_flag:%d\n", __func__, priv->fast_fps_flag);

    return 0;
}
#else
static int gc20c3_set_master_or_slave(struct v4l2_subdev* sd,
                                                        int is_master)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc20c3_priv* priv = to_gc20c3(client);

    pr_debug("%s is_master:%d\n", __func__, is_master);

    if (is_master)
        priv->master_or_slave = MASTER_MODE;
    else {
        priv->master_or_slave = SLAVER_MODE;
        priv->slave_sync_mode = slave_sync_mode;
    }

    return 0;
}
#endif

static int gc20c3_set_flip_mirror(struct v4l2_subdev* sd, int enable, int force)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc20c3_priv* priv = to_gc20c3(client);
    int value = 0;
    int flip_en = enable & (0x1 << FLIP_OFFSET);
    int mirror_en = enable & (0x1 << MIRROR_OFFSET);
    int off_x, off_y;

#if defined(CONFIG_SYS_FAST_LAUNCH)
    if (!force) {
        if (priv->sensor_param.flip_mirror == enable) {
            return 0;
        }
    }
#endif

    if (flip_en) {
        if (mirror_en) {
            value |= 0x3;
            off_x = 1;
            off_y = 2;
        } else {
            value |= 0x2;
            off_x = 0;
            off_y = 2;
        }
    } else {
        if (mirror_en) {
            value |= 0x1;
            off_x = 1;
            off_y = 0;
        } else {
            value |= 0x0;
            off_x = 0;
            off_y = 0;
        }
    }

    if (priv->aec_parms.off_x - off_x >= 0) {
        off_x = priv->aec_parms.off_x - off_x - SENSOR_VALID_OFFSET_X;
    } else { //==0
        off_x += SENSOR_VALID_OFFSET_X;
    }

    if (priv->aec_parms.off_y - off_y >= 0) {
        off_y = priv->aec_parms.off_y - off_y - SENSOR_VALID_OFFSET_Y;
    } else { //==0
        off_y += SENSOR_VALID_OFFSET_Y;
    }

    gc20c3_write(client, FLIP_MIRROR_REG, value);

    if (!priv->aec_parms.auto_off) {
        gc20c3_write(client, WIN_OFF_Y_L, off_y);
        gc20c3_write(client, WIN_OFF_X_L, off_x);
    }

    priv->sensor_param.flip_mirror = enable;

    return 0;
}

#if ( defined(CONFIG_MACH_KM01A) || defined(CONFIG_SYS_FAST_LAUNCH) )
static int gc20c3_set_flip_mirror_flag(struct v4l2_subdev *sd,
                                                int enable)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct gc20c3_priv *priv = to_gc20c3(client);

    priv->sensor_param.flip_mirror = enable;
    return 0;
}
#endif

static int gc20c3_set_init_fps(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc20c3_priv* priv = to_gc20c3(client);
    int fps = ctrl->value;
    pr_err("%s, fps = %d\n", __func__,fps);
    priv->fps_info.to_fps = fps;
    priv->fps_info.video_fps = priv->fps_info.to_fps;

    return 0;
}

/*
 * XXXX_core_s_ctrl -
 * set core s_ctrl
 * core functions
 *
 * @sd:             subdev
 * @ctrl:           pointer to ctrl
 */
static int gc20c3_core_s_ctrl(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    int ret;

    //pr_err("%s cmd:%d setting support\n", __func__, ctrl->id);
    switch (ctrl->id) {
        case SET_ISP_MISC_CALLBACK:
            ret = gc20c3_set_isp_timing_cb(sd, ctrl);
            break;

        case SET_FPS_DIRECT:
            ret = gc20c3_set_fps_direct(sd, ctrl);
            break;
#ifdef CONFIG_MACH_KM01A
        case SET_MASTER:
            ret = gc20c3_set_master_or_slave(sd, 1, ctrl->value);
            break;

        case SET_SLAVER: /*for dual sensor*/
            ret = gc20c3_set_master_or_slave(sd, 0, ctrl->value);
            break;
#else
        case SET_MASTER:/*for dual sensor*/
            ret = gc20c3_set_master_or_slave(sd, 1);
            break;

        case SET_SLAVER:/*for dual sensor*/
            ret = gc20c3_set_master_or_slave(sd, 0);
            break;
#endif
        case SET_FLIP_MIRROR:
            ret = gc20c3_set_flip_mirror(sd, ctrl->value, 0);
            break;
#if ( defined(CONFIG_MACH_KM01A) || defined(CONFIG_SYS_FAST_LAUNCH) )
        case SET_FLIP_MIRROR_FLAG:
            ret = gc20c3_set_flip_mirror_flag(sd, ctrl->value);
            break;
#endif

#ifdef CONFIG_SYS_FAST_LAUNCH
        case SET_INIT_FPS:
            ret = gc20c3_set_init_fps(sd, ctrl);
            break;
#endif
#ifdef CONFIG_MACH_KM01A
        //小窗
        case SET_NORMAL_MODE:
            ret = ak_sensor_set_nomal_mode(sd, ctrl->value);
            break;
        case SET_INIT_FPS:
            ret = gc20c3_set_init_fps(sd, ctrl);
            break;

        //最新的fastae功能，CIS的SET_FAST_AE是放到这里进行调用
        case SET_FAST_AE:
            ret = sensor_set_fast_ae(sd, (struct ae_fast_struct *)ctrl->value);
            break;
#endif
        default:
            pr_err("%s cmd:%d not support\n", __func__, ctrl->id);
            ret = -1;
            break;
    }

    return ret;
}

#ifdef CONFIG_MACH_KM01A
static void ak_sensor_parms_init_f(struct gc20c3_priv* priv,
                                  AK_ISP_SENSOR_REG_INFO* preg_info, int para_num)
{
    long int vts_h_tmp = 0x04, vts_l_tmp = 0x65;
    int i = 0;
    priv->reg_slave_mode = 0;

    pr_info("priv->fast_fps_flag:%d\r\n", priv->fast_fps_flag);
    pr_info("para_num:%d, MAX_FPS:%d\r\n", para_num, MAX_FPS);

    for (i = 0; i < para_num; i++) {
        if (preg_info->reg_addr == VTS_REG_H)
            vts_h_tmp = preg_info->value;
        else if (preg_info->reg_addr == VTS_REG_L)
            vts_l_tmp = preg_info->value;
        preg_info++;
    }

    priv->aec_parms.calc_vts_tmp = ((vts_h_tmp << 8) | vts_l_tmp) * MAX_FPS * 100;

    pr_info("calc_vts_tmp %d \n", priv->aec_parms.calc_vts_tmp);
}

static int ak_sensor_get_aov_flag_func( void *arg)
{
    struct gc20c3_priv* priv = (struct gc20c3_priv *)arg;
    return STORE_CONFIG_IS_AOV_MODE(priv);
}
#else
static void ak_sensor_parms_init_f(struct gc20c3_priv* priv,
                                   AK_ISP_SENSOR_INIT_PARA* para)
{
    AK_ISP_SENSOR_REG_INFO* preg_info;
    long int vts_h_tmp = 0x06, vts_l_tmp = 0x42;//default reg
    //long int vts_h_tmp = 0x04, vts_l_tmp = 0x76;//from isp conf
    int i = 0;

    preg_info = para->reg_info;
    for (i = 0; i < para->num; i++) {
        if (preg_info->reg_addr == VTS_REG_H)
            vts_h_tmp = preg_info->value;
        else if (preg_info->reg_addr == VTS_REG_L)
            vts_l_tmp = preg_info->value;
        preg_info++;
    }

    priv->aec_parms.calc_vts_tmp = ((vts_h_tmp << 8) | vts_l_tmp) * MAX_FPS * 100;

    pr_debug("%d calc_vts_tmp %d \n", __LINE__, priv->aec_parms.calc_vts_tmp);
}
#endif

static int gc20c3_store_initial_regs(struct gc20c3_priv* priv, void* arg)
{
    struct i2c_client* client = priv->client;
    AK_ISP_SENSOR_INIT_PARA* sensor_para = arg;
    void* p_reg_info;
    int ret;

    p_reg_info = sensor_para->reg_info;
    sensor_para->reg_info = kmalloc(
        sizeof(AK_ISP_SENSOR_REG_INFO) * sensor_para->num, GFP_KERNEL);
    if (!sensor_para->reg_info) {
        pr_err("%s alloc1 fail\n", __func__);
        ret = -ENOMEM;
        goto alloc1_fail;
    } else {
        if (copy_from_user(sensor_para->reg_info, p_reg_info,
                sizeof(AK_ISP_SENSOR_REG_INFO) * sensor_para->num)) {
            pr_err("%s cp2 fail\n", __func__);
            ret = -EFAULT;
            goto cp2_fail;
        }

        if (priv->para.reg_info) {
            devm_kfree(&client->dev, priv->para.reg_info);
            priv->para.reg_info = NULL;
            priv->para.num = 0;
        }

        /*store sensor configurtion*/
        priv->para.num = sensor_para->num;
        priv->para.reg_info = devm_kzalloc(&client->dev,
            sizeof(AK_ISP_SENSOR_REG_INFO) * sensor_para->num, GFP_KERNEL);
        if (!priv->para.reg_info) {
            pr_err("%s alloc2 fail\n", __func__);
            goto alloc2_fail;
        }
        memcpy(priv->para.reg_info, sensor_para->reg_info,
            sizeof(AK_ISP_SENSOR_REG_INFO) * sensor_para->num);

        pr_debug("%s para.num;%d\n", __func__, priv->para.num);
    }
#if ( !defined(CONFIG_MACH_KM01A) && defined(CONFIG_SYS_FAST_LAUNCH) )
//#ifdef CONFIG_SYS_FAST_LAUNCH
    ak_sensor_parms_init_f(priv, sensor_para);
#endif
    kfree(sensor_para->reg_info);
    return 0;

alloc2_fail:
cp2_fail:
    kfree(sensor_para->reg_info);
alloc1_fail:
    return ret;
}

static long gc20c3_core_ioctl(
    struct v4l2_subdev* sd, unsigned int cmd, void* arg)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc20c3_priv* priv = to_gc20c3(client);
    int ret = 0;

    pr_debug("%s\n", __func__);

    switch (cmd) {
        case AK_SENSOR_SET_INIT:
            gc20c3_store_initial_regs(priv, arg);
            break;

        case AK_SENSOR_GET_MAX_EXP_FOR_FPS: {
            struct sensor_max_exp_for_fps* exp_for_fps = arg;
            struct v4l2_control ctrl;

            ctrl.value = exp_for_fps->fps;
            ret = gc20c3_get_max_exp_for_fps(sd, &ctrl);
            if (!ret)
                exp_for_fps->max_exp = ctrl.value;
            break;
        }
#ifdef CONFIG_SYS_FAST_LAUNCH
        case AK_SENSOR_GET_AE_AWB_CONVERT_COEF: {
            ae_awb_dynamic_map_policy_t* ae_awb_dynamic_map_policy = arg;
            struct i2c_client* client = v4l2_get_subdevdata(sd);
            struct gc20c3_priv* priv = to_gc20c3(client);

            ae_awb_dynamic_map_policy->ae_convert_group_num = GC20C3_AE_CONVERT_GROUP_NUM;
            if(GC20C3_AE_CONVERT_GROUP_NUM != 0) {
                ae_awb_dynamic_map_policy->dynamic_map_enable |= 1;

                memcpy(&priv->ae_awb_dynamic_map_policy.ae_convert_coef_group,
                        &gc20c3_ae_convert_coef_group,
                        sizeof(ae_convert_coef_t) * ae_awb_dynamic_map_policy->ae_convert_group_num);
                memcpy(&ae_awb_dynamic_map_policy->ae_convert_coef_group,
                        &priv->ae_awb_dynamic_map_policy.ae_convert_coef_group,
                        sizeof(ae_convert_coef_t) * ae_awb_dynamic_map_policy->ae_convert_group_num);
            }

            if(GC20C3_AWB_CONVERT_GROUP_NUM != 0) {
                ae_awb_dynamic_map_policy->dynamic_map_enable |= 1<<1;
                memcpy(&priv->ae_awb_dynamic_map_policy.awb_convert_coef,
                        &gc20c3_awb_convert_coef,
                        sizeof(awb_convert_coef_t) );
                memcpy(&ae_awb_dynamic_map_policy->awb_convert_coef,
                        &priv->ae_awb_dynamic_map_policy.awb_convert_coef,
                        sizeof(awb_convert_coef_t));
            }
            break;
        }
#endif
        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

/*
 * XXXX_get_fmt -
 * get format
 * pad functions
 *
 * @sd:             subdev
 * @cfg:            pointer to pad config
 * @format:         return format
 */
static int gc20c3_get_fmt(struct v4l2_subdev* sd,
    struct v4l2_subdev_pad_config* cfg, struct v4l2_subdev_format* format)
{
    struct v4l2_mbus_framefmt* mf = &format->format;
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc20c3_priv* priv = to_gc20c3(client);
    static struct gc20c3_win_size win = {
        .name = "default",
        .width = SENSOR_OUTPUT_WIDTH,
        .height = SENSOR_OUTPUT_HEIGHT,
        .regs = NULL,
    };

    if (format->pad)
        return -EINVAL;

    if (!priv->win) {
        priv->win = &win;
        priv->cfmt_code = MEDIA_BUS_FMT_UYVY8_2X8;
    }

    mf->width = priv->win->width;
    mf->height = priv->win->height;
    mf->code = priv->cfmt_code;

    switch (mf->code) {
        case MEDIA_BUS_FMT_RGB565_2X8_BE:
        case MEDIA_BUS_FMT_RGB565_2X8_LE:
            mf->colorspace = V4L2_COLORSPACE_SRGB;
            break;
        default:
        case MEDIA_BUS_FMT_YUYV8_2X8:
        case MEDIA_BUS_FMT_UYVY8_2X8:
            mf->colorspace = V4L2_COLORSPACE_JPEG;
    }
    mf->field = V4L2_FIELD_NONE;

    return 0;
}

/*
 * XXXX_set_fmt -
 * set format
 * pad functions
 *
 * @sd:             subdev
 * @cfg:            pointer to pad config
 * @format:         format to set
 */
static int gc20c3_set_fmt(struct v4l2_subdev* sd,
    struct v4l2_subdev_pad_config* cfg, struct v4l2_subdev_format* format)
{
    struct v4l2_mbus_framefmt* mf = &format->format;

    if (format->pad)
        return -EINVAL;

    mf->field = V4L2_FIELD_NONE;

    switch (mf->code) {
        case MEDIA_BUS_FMT_RGB565_2X8_BE:
        case MEDIA_BUS_FMT_RGB565_2X8_LE:
            mf->colorspace = V4L2_COLORSPACE_SRGB;
            break;
        default:
            mf->code = MEDIA_BUS_FMT_UYVY8_2X8;
        case MEDIA_BUS_FMT_YUYV8_2X8:
        case MEDIA_BUS_FMT_UYVY8_2X8:
            mf->colorspace = V4L2_COLORSPACE_JPEG;
    }

    if (cfg)
        cfg->try_fmt = *mf;
    return 0;
}

/*
 * XXXX_set_fmt -
 * set format
 * pad functions
 *
 * @sd:             subdev
 * @cfg:            pointer to pad config
 * @sel:            return selection
 */
static int gc20c3_get_selection(struct v4l2_subdev* sd,
    struct v4l2_subdev_pad_config* cfg, struct v4l2_subdev_selection* sel)
{
    sel->r.left = SENSOR_VALID_OFFSET_X;
    sel->r.top = SENSOR_VALID_OFFSET_Y;
    sel->r.width = SENSOR_OUTPUT_WIDTH;
    sel->r.height = SENSOR_OUTPUT_HEIGHT;

    return 0;
}

/*
 * XXXX_enum_bus -
 * enum bus
 * pad functions
 *
 * @sd:             subdev
 * @cfg:            pointer to pad config
 * @code:           return code of bus type
 */
static int gc20c3_enum_mbus_code(struct v4l2_subdev* sd,
    struct v4l2_subdev_pad_config* cfg,
    struct v4l2_subdev_mbus_code_enum* code)
{
    if (code->pad || code->index >= ARRAY_SIZE(gc20c3_codes))
        return -EINVAL;

    code->code = MEDIA_BUS_FMT_YUYV8_2X8;

    return 0;
}

/*
 * XXXX_g_crop -
 * get crop
 * video functions
 *
 * @sd:             subdev
 * @a:              return crop
 */
static int gc20c3_g_crop(struct v4l2_subdev* sd, struct v4l2_crop* a)
{
    a->c.left = SENSOR_VALID_OFFSET_X;
    a->c.top = SENSOR_VALID_OFFSET_Y;
    a->c.width = SENSOR_OUTPUT_WIDTH;
    a->c.height = SENSOR_OUTPUT_HEIGHT;
    a->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    return 0;
}

/*
 * XXXX_cropcap -
 * get crop capbilities
 * video functions
 *
 * @sd:             subdev
 * @a:              return crop capbilities
 */
static int gc20c3_cropcap(struct v4l2_subdev* sd, struct v4l2_cropcap* a)
{
    gc20c3_sen_get_resolution_func(
        NULL, &a->bounds.width, &a->bounds.height);
    a->bounds.left = SENSOR_VALID_OFFSET_X;
    a->bounds.top = SENSOR_VALID_OFFSET_Y;
    a->defrect = a->bounds;
    a->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    a->pixelaspect.numerator = 1;
    a->pixelaspect.denominator = 1;

    //pr_err("%s bounds.width:%d, bounds.height:%d\n", __func__, a->bounds.width,
    //    a->bounds.height);

    return 0;
}

/*
 * XXXX_video_probe -
 * do some process in subdev probe
 * locale functions
 *
 * @sd:             subdev
 * @a:              return crop capbilities
 */
static int gc20c3_video_probe(struct i2c_client* client)
{
    struct gc20c3_priv* priv = to_gc20c3(client);
    int ret;

    ret = v4l2_ctrl_handler_setup(&priv->hdl);

    return ret;
}

/*ctrl ops*/
static const struct v4l2_ctrl_ops gc20c3_ctrl_ops = {
    .g_volatile_ctrl = gc20c3_g_volatile_ctrl,
    .s_ctrl = gc20c3_s_ctrl,
};

/*core ops*/
static struct v4l2_subdev_core_ops gc20c3_subdev_core_ops = {
    .s_power = gc20c3_core_s_power,
    .g_ctrl = gc20c3_core_g_ctrl,
    .s_ctrl = gc20c3_core_s_ctrl,
    .ioctl = gc20c3_core_ioctl,
};

/*
 * XXXX_g_mbus_config -
 * get buf config
 * video functions
 *
 * @sd:             subdev
 * @cfg:            return config
 */
static int gc20c3_g_mbus_config(
    struct v4l2_subdev* sd, struct v4l2_mbus_config *cfg)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);

    cfg->flags = V4L2_MBUS_PCLK_SAMPLE_RISING | V4L2_MBUS_MASTER
        | V4L2_MBUS_VSYNC_ACTIVE_HIGH | V4L2_MBUS_HSYNC_ACTIVE_HIGH
        | V4L2_MBUS_DATA_ACTIVE_HIGH;
    cfg->type = V4L2_MBUS_PARALLEL;
    cfg->flags = 0;

    return 0;
}

/*
 * XXXX_s_crop -
 * set crop
 * video functions
 *
 * @sd:             subdev
 * @crop:           crop to set
 */
static int gc20c3_s_crop(struct v4l2_subdev* sd, const struct v4l2_crop *crop)
{
    printk(KERN_ERR "%s %d, left:%d, top:%d, width:%d, height:%d\n", __func__,
        __LINE__, crop->c.left, crop->c.top, crop->c.width, crop->c.height);
    return 0;
}

/*video ops*/
static struct v4l2_subdev_video_ops gc20c3_subdev_video_ops = {
    .s_stream = gc20c3_s_stream,
    .cropcap = gc20c3_cropcap,
    .g_crop = gc20c3_g_crop,
    .g_mbus_config = gc20c3_g_mbus_config,
    .s_crop = gc20c3_s_crop,
};

/*pad ops*/
static const struct v4l2_subdev_pad_ops gc20c3_subdev_pad_ops = {
    .enum_mbus_code = gc20c3_enum_mbus_code,
    .get_fmt = gc20c3_get_fmt,
    .set_fmt = gc20c3_set_fmt,
    .get_selection = gc20c3_get_selection,
};

/*sensor driver subdev ops*/
static struct v4l2_subdev_ops gc20c3_subdev_ops = {
    .core = &gc20c3_subdev_core_ops,
    .video = &gc20c3_subdev_video_ops,
    .pad = &gc20c3_subdev_pad_ops,
};

/*
 * sensor_of_parse -
 * parse node of device
 *
 * @client:         pointor to i2c client
 * @priv:           sensor struct
 */
static int sensor_of_parse(
    struct i2c_client* client, struct gc20c3_priv* priv)
{
    struct device_node* np = client->dev.of_node;

    /*it is exist reset but lack of pwdn gpio in most case*/
    priv->gpio_reset = of_get_named_gpio(np, "reset-gpio", 0);
    priv->gpio_pwdn = of_get_named_gpio(np, "pwdn-gpio", 0);
#ifdef CONFIG_MACH_KM01A
    priv->gpio_fsync = of_get_named_gpio(np, "fsync-gpio", 0);
    priv->gpio_power = of_get_named_gpio(np, "power-gpio", 0);
#endif
    if (priv->gpio_reset >= 0 && priv->gpio_reset != 0xffff)
    {
        devm_gpio_request(&client->dev, priv->gpio_reset, "sensor-reset");
    }

    if (priv->gpio_pwdn >= 0 && priv->gpio_pwdn != 0xffff)
    {
        devm_gpio_request(&client->dev, priv->gpio_pwdn, "sensor-pwdn");
    }
#ifdef CONFIG_MACH_KM01A
    if (priv->gpio_power >= 0)
    {
        devm_gpio_request(&client->dev, priv->gpio_power, "sensor-power");
    }
#endif

    return 0;
}

/*
 * XXXX_match - check sensor id for match driver
 * @priv:           pointer to pirvate structure
 *
 * @RETURN: 0-match fail; 1-match success
 */
static int gc20c3_match(struct gc20c3_priv* priv)
{
    int pid;

    if (!check_id)
        return 1;

    priv->cb_info.cb->sensor_set_power_on_func(priv);
    pid = priv->cb_info.cb->sensor_probe_id_func(priv);
    if (pid <= 0) {
        pr_err("%s fail\n", __func__);
        return 0;
    }

    return 1;
}

static void client_new_addr(void)
{
    struct gc20c3_priv* t_priv=NULL;
    struct i2c_client* client;

    list_for_each_entry(t_priv, &privs_list, list)
    {
        if (!t_priv->subdev.devnode)
            continue;

        client = t_priv->client;
        switch (t_priv->subdev.devnode->num)
        {
            case 0:
                if(addr0 > 0)
                {
                    pr_err("%s %d new_addr0:0x%x\n", __func__, __LINE__, addr0);
                    client->addr = addr0;
                }
                break;

            case 1:
                if(addr1 > 0)
                {
                    pr_err("%s %d new_addr1:0x%x\n", __func__, __LINE__, addr1);
                    client->addr = addr1;
                }
                break;

            default:
                break;
        }
        //pr_debug("%s devnode:%d reset:%d i2c name:%s addr:0x%x\n", __func__,
          //t_priv->subdev.devnode->num, t_priv->gpio_reset, client->name, client->addr);
    }
}

/*
 * XXXX_probe -
 * driver probe after platform probe ok
 * i2c_driver functions
 *
 * @client:         pointor to i2c client
 * @did:            driver ids
 */
static int gc20c3_probe(
    struct i2c_client* client, const struct i2c_device_id* did)
{
    struct gc20c3_priv* priv;
    struct i2c_adapter* adapter = to_i2c_adapter(client->dev.parent);
    int ret;

    //pr_err("%s %s %s\n", __func__, __DATE__, __TIME__);
    pr_err("%s,%d\n", __func__,__LINE__);

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_err(&adapter->dev, "gc20c3: I2C-Adapter doesn't support SMBUS\n");
        return -EIO;
    }

    priv = devm_kzalloc(&client->dev, sizeof(struct gc20c3_priv), GFP_KERNEL);
    if (!priv) {
        dev_err(&adapter->dev,
            "Failed to allocate memory for private data!\n");
        return -ENOMEM;
    }
#ifdef CONFIG_MACH_KM01A
    priv->fast_fps_flag = 0;
    priv->first_fast_fps_flag = 0;
#endif

    /*parse of node*/
    sensor_of_parse(client, priv);

#ifdef CONFIG_MACH_KM01A
    if(!sensor_power_on && priv->gpio_reset >= 0){
        gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
    }
#ifndef aov_slave_mode
    UPDATE_SENSOR_STATUS(E_SENSOR_POWER_ON);
#endif
#endif
    /*
     * the current sensor slave address to client.
     * the address from dts may incorrect
     * */
    client->addr = SENSOR_I2C_ADDR;
    priv->cb_info.cb = &gc20c3_cb;
    priv->cb_info.arg = priv;
    priv->client = client;
#if ( defined(CONFIG_MACH_KM01A) || defined(CONFIG_SYS_FAST_LAUNCH) )
    memcpy(&priv->ae_fast, &ae_fast_default, sizeof(struct ae_fast_struct));
#endif

    if (!client->dev.of_node) {
        dev_err(&client->dev, "Missing platform_data for driver\n");
        ret = -EINVAL;
        goto err_clk;
    }

    if (!gc20c3_match(priv)) {
        ret = -ENODEV;
        goto err_clk;
    }

    /*subdev init*/
    v4l2_i2c_subdev_init(&priv->subdev, client, &gc20c3_subdev_ops);
    priv->subdev.flags
        |= /*V4L2_SUBDEV_FL_HAS_EVENTS | */ V4L2_SUBDEV_FL_HAS_DEVNODE;
    v4l2_ctrl_handler_init(&priv->hdl, 2);
    v4l2_ctrl_new_std(
        &priv->hdl, &gc20c3_ctrl_ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
    v4l2_ctrl_new_std(
        &priv->hdl, &gc20c3_ctrl_ops, V4L2_CID_HFLIP, 0, 1, 1, 0);

    /*private ctrl*/
    priv->ctrl_sensor_get_id
        = v4l2_ctrl_new_custom(&priv->hdl, &config_sensor_get_id, NULL);
    if (!priv->ctrl_sensor_get_id)
        pr_err("creat ctrl_sensor_get_id fail\n");

    priv->subdev.ctrl_handler = &priv->hdl;
    if (priv->hdl.error) {
        dev_err(&client->dev, "Hdl error\n");
        ret = priv->hdl.error;
        goto err_clk;
    }

    ret = gc20c3_video_probe(client);
    if (ret < 0) {
        dev_err(&client->dev, "gc20c3 probe fail\n");
        goto err_videoprobe;
    }

    ret = v4l2_async_register_subdev(&priv->subdev);
    if (ret < 0) {
        dev_err(&client->dev,
            "v4l2 async register subdev fail, ret:%d\n", ret);
        goto err_videoprobe;
    }
#ifdef CONFIG_MACH_KM01A
    priv->master_or_slave = SINGLE_MODE;
#endif

    //pr_err("%s pwdn:%d i2c addr:0x%x\n", __func__,priv->gpio_pwdn,priv->client->addr);
   /* 添加初始化链表，并添加到 priv_list 链表尾 */
    INIT_LIST_HEAD(&priv->list);
    list_add_tail(&priv->list, &privs_list);

    //client->addr = SENSOR_I2C_ADDR;
    client_new_addr();

    dev_err(&adapter->dev, "gc20c3 Probed success, subdev:%p\n", &priv->subdev);

    /*ak platform need export some informations from sensor*/
    call_sensor_sys_init();

#if ( !defined(CONFIG_MACH_KM01A) && defined(CONFIG_SYS_FAST_LAUNCH) )
//#ifdef CONFIG_SYS_FAST_LAUNCH
    //if (priv->gpio_reset >= 0)
    {
        //gc20c3_sen_set_power_on_func(priv);

        sys_sensor_init_set((void *)gc20c3_sen_init_func,priv,gc20c3_sen_get_parameter_func,SENSOR_ID);
    }
#endif
    return 0;

err_videoprobe:
    v4l2_ctrl_handler_free(&priv->hdl);
err_clk:
    return ret;
}

/*
 * XXXX_remove -
 * driver probe after platform remove ok
 * i2c_driver functions
 *
 * @client:         pointor to i2c client
 */
static int gc20c3_remove(struct i2c_client* client)
{
    struct gc20c3_priv* priv = to_gc20c3(client);

    pr_err("%s %d\n", __func__, __LINE__);

    if (!priv) {
        pr_err("%s had remove\n", __func__);
        return 0;
    }

    /* 移除 priv_list 链表中的链表项*/
    list_del_init(&priv->list);

    /*unregister async subdev*/
    v4l2_async_unregister_subdev(&priv->subdev);
    /*unregister subdev*/
    v4l2_device_unregister_subdev(&priv->subdev);
    /*unregister handler*/
    v4l2_ctrl_handler_free(&priv->hdl);
    /*unregister sys node*/
    call_sensor_sys_deinit();
    return 0;
}

static const struct i2c_device_id gc20c3_id[] = { { "gc20c3", 0 }, {} };
MODULE_DEVICE_TABLE(i2c, gc20c3_id);

static const struct of_device_id gc20c3_of_match[] = {
    /*donot changed compatible, must the same as dts*/
    {
        .compatible = "anyka,sensor0",
    },
    {
        .compatible = "anyka,sensor1",
    },
    {},
};
MODULE_DEVICE_TABLE(of, gc20c3_of_match);

static struct i2c_driver gc20c3_i2c_driver = {
    .driver = {
        .name = "gc20c3",
        .of_match_table = of_match_ptr(gc20c3_of_match),
    },
    .probe = gc20c3_probe,
    .remove = gc20c3_remove,
    .id_table = gc20c3_id,
};

#if ( defined(CONFIG_MACH_KM01A) || !defined(CONFIG_SYS_FAST_LAUNCH) )
module_i2c_driver(gc20c3_i2c_driver);
#else
static int __init gc20c3_i2c_driver_init(void)
{
    printk("%s\n",__func__);

    return i2c_add_driver(&(gc20c3_i2c_driver));
}
subsys_initcall(gc20c3_i2c_driver_init);
#endif

MODULE_DESCRIPTION("SoC Camera driver for gc20c3 sensor");
MODULE_AUTHOR("Anyka Microelectronic");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0.03");
//1.0.00_ach02:修复曝光问题
//1.0.00_ach03:增加双目支持
//1.0.00_ach04:增加正式版sensor ID支持，更新AEC策略
//1.0.00_ach05:增加AEC高温逻辑
//1.0.01:提升版本好，解决快启图像效果差的问题
//1.0.02:解决除零报错
//1.0.03:slave修改为强制同步的工作模式，添加slave模式快启的ae awb 的补偿系数

#if 1
//gc的SENSOR有个需求,就是发送读指令后,不能有停止信号.
//也就是说发送读取信号和接收数据需要合成一起发，不能分二次调用i2c_transfer
static int gc20c3_read(const struct i2c_client *client, int reg)
{
    int ret;
    struct i2c_adapter *adap;
    struct i2c_msg msg[2] = {0};
    unsigned char reg_addr[SENSOR_REGADDR_BYTE];
    unsigned char data[SENSOR_DATA_BYTE] = {0};
    int value = 0;
    int i;
    char tmp;

    if(client == NULL){
        pr_err("%s client == NULL\n", __func__);
        return -1;
    }
    adap = client->adapter;

    for (i = SENSOR_REGADDR_BYTE, tmp = reg & 0xff; i > 0; i--) {
        reg_addr[i - 1] = tmp;
        tmp = reg >> 8;
    }

    msg[0].addr = client->addr;
    msg[0].flags = client->flags & I2C_M_TEN;
    msg[0].len = SENSOR_REGADDR_BYTE;
    msg[0].buf = reg_addr;

    msg[1].addr = client->addr;
    msg[1].flags = client->flags & I2C_M_TEN;
    msg[1].flags |= I2C_M_RD;
    msg[1].len = SENSOR_DATA_BYTE;
    msg[1].buf = data;

    ret = i2c_transfer(adap, msg, 2);

    for (i = 0; i < SENSOR_DATA_BYTE; i++) {
        value <<= 8;
        value |= data[i];
    }

    /*
     * If everything went ok (i.e. 2 msg transmitted), return msgs
     * transmitted, else error code.
     */
    if (ret != 2) {
        pr_err("%s %x write reg:%x failed\n", __func__, client->addr, reg);
        return ret;
    }

    return value;
}


/*
 * sensor_write_register -
 * write sensor register
 * local functions
 *
 * @arg:                i2c_bus_id
 * @reg:                reg addr
 * @data                data
 */
static int gc20c3_write(const struct i2c_client *client,
                                    int reg, int value)
{
    int ret;
    struct i2c_adapter *adap;
    struct i2c_msg msg[2] = {0};
    unsigned char buf[SENSOR_REGADDR_BYTE + SENSOR_DATA_BYTE];
    int i;
    char tmp;
    if(client == NULL){
        pr_err("%s client == NULL\n", __func__);
        return -1;
    }
    adap = client->adapter;

    for (i = SENSOR_REGADDR_BYTE, tmp = reg & 0xff; i > 0; i--) {
        buf[i - 1] = tmp;
        tmp = (reg >> 8) & 0xff;
    }

    for (i = SENSOR_DATA_BYTE, tmp = value & 0xff; i > 0; i--) {
        buf[i - 1 + SENSOR_REGADDR_BYTE] = tmp;
        tmp = (value >> 8) & 0xff;
    }

    msg[0].addr = client->addr;
    msg[0].flags = client->flags & I2C_M_TEN;
    msg[0].len = SENSOR_REGADDR_BYTE + SENSOR_DATA_BYTE;
    msg[0].buf = buf;

    ret = i2c_transfer(adap, msg, 1);

    /*
     * If everything went ok (i.e. 2 msg transmitted), return #bytes
     * transmitted, else error code.
     */
    if (ret != 1)
        pr_err("%s %x write reg:%x value:%x failed\n", __func__,
                client->addr, reg, value);

    return ret;

}
#else
/*sensor i2c write*/
static int gc20c3_write(const struct i2c_client* client, int reg, int value)
{
    struct i2c_transfer_struct trans = {
        .client = client,
        .reg_bytes = SENSOR_REGADDR_BYTE,
        .value_bytes = SENSOR_DATA_BYTE,
        .reg = reg,
        .value = value,
    };

    return i2c_write(&trans);
}

/*sensor i2c read*/
static int gc20c3_read(const struct i2c_client* client, int reg)
{
    struct i2c_transfer_struct trans = {
        .client = client,
        .reg_bytes = SENSOR_REGADDR_BYTE,
        .value_bytes = SENSOR_DATA_BYTE,
        .reg = reg,
        .value = 0,
    };

    //pr_debug("%s client:%p, addr:%x\n", __func__, client, client->addr);
    return i2c_read(&trans);
}
#endif

/*
 * The sensor must set callbacks
 *
 * */
static int gc20c3_sen_init_func(
    void* arg, const AK_ISP_SENSOR_INIT_PARA* npara)
{
    /*gc20c3_Full_AA_1920x1080_30fps_390M_2L_24Mhz.ini*/
    int i;
    AK_ISP_SENSOR_REG_INFO* preg_info;
    struct gc20c3_priv* priv = arg;
    struct i2c_client* client = priv->client;
    AK_ISP_SENSOR_INIT_PARA* para = &priv->para;
    unsigned int exp = (unsigned int)(priv->ae_fast.sensor_exp_time);
    unsigned int hts_h_tmp = 0x02, hts_l_tmp = 0x03;
    unsigned int vts_h_tmp = 0x06, vts_l_tmp = 0x42;
#ifdef CONFIG_MACH_KM01A
    int fast_fps = 0;
    int para_num = 0;
    priv->init_flag = 0;

    //gc20c3 stream on/off 无需重新初始化、无硬件操作
    if (IS_SENSOR_STATUS(E_SENSOR_ACCESS_STANDBY))
    {
        pr_err("ac_stand.sta %d\n", priv->status);
        return 0;
    }
    //gc20c3 stream on/off 切换时机最好多留一点vblank
    if (IS_SENSOR_STATUS(E_SENSOR_INIT))
    { //进入aov模式
        priv->fps_info.aov_fps = 10;
        priv->fps_info.to_fps = priv->fps_info.aov_fps; //10fps
        pr_err("sen_init.sta %d\n", priv->status);
    }
    else if (IS_SENSOR_STATUS(E_SENSOR_WORK))
    {                                                     //普流工作
        priv->fps_info.to_fps = priv->fps_info.video_fps; //when in video stream
        pr_err("sen_work.sta %d\n", priv->status);
    }

    pr_info("%s sensor_mode %s\n", __func__,
            (priv->master_or_slave == MASTER_MODE) ? "MASTER" : "SLAVER");
    if (priv->master_or_slave == MASTER_MODE || priv->master_or_slave == SLAVER_AOV_MODE)
    {
        if (!priv->fast_fps_flag)
        { //normal mode
            preg_info = (void *)gc20c3_1920x1080_30fps;
            para_num = sizeof(gc20c3_1920x1080_30fps) / sizeof(gc20c3_1920x1080_30fps[0]) / 2;
            if (priv->first_fast_fps_flag)
            {
                priv->first_fast_fps_flag = 0;
            }
            MAX_FPS = 30;
        }
    }
    else if (priv->master_or_slave == SLAVER_MODE)
    {
        pr_err("[%s]%dSLAVER mode not support\n", __func__, __LINE__);
    } else {
        if (para->num <= 0 && npara)
            para = (void*)npara;

        preg_info = para->reg_info;
    }
    ak_sensor_parms_init_f(priv, preg_info, para_num);
    pr_info("priv->fast_fps_flag:%d, para_num:%d\n", priv->fast_fps_flag, para_num);
#else
    if (para->num <= 0 && npara)
        para = (void*)npara;


#if 0 //TODO YJH
#if ( !defined(CONFIG_MACH_KM01A) && defined(CONFIG_SYS_FAST_LAUNCH) )
//#ifdef CONFIG_SYS_FAST_LAUNCH
    pr_info("init_regnum=%d,allow_reinit:%d,inited:%d,client:%p\n", 
            para->num,get_allow_reinit(),get_sensor_fast_inited(),client);

    if( get_sensor_fast_inited() && !get_allow_reinit() ) return 0;

    if(para->num==0)
    {
       int sensor_len = 0;
       preg_info = (AK_ISP_SENSOR_REG_INFO *)get_sensor_preg(&sensor_len);
       para->num = sensor_len / sizeof(AK_ISP_SENSOR_REG_INFO);
       pr_debug("fast para->num=%d\n",para->num);
    }
    else
#endif
#endif
    preg_info = para->reg_info;
#endif

#ifdef CONFIG_MACH_KM01A
    for (i = 0; i < para_num; i++) {
#else
    for (i = 0; i < para->num; i++) {
#endif
#if 0
        {
            int value;

            value = gc20c3_read(client, preg_info->reg_addr);
            pr_err("reg:%x read value:%x\n", preg_info->reg_addr, value);
        }
#endif
#if ( defined(CONFIG_MACH_KM01A) || defined(CONFIG_SYS_FAST_LAUNCH) )
        if (preg_info->reg_addr == 0x0100)
        {
            pr_debug("init exp_again:%d-%d\r\n", exp, priv->ae_fast.sensor_a_gain);
#if 0
            //if (priv->aec_parms.reg_frame_hts<=0)
                priv->aec_parms.reg_frame_hts = ((hts_h_tmp << 8) | hts_l_tmp);
            //if (priv->aec_parms.reg_frame_vts<=0)
                priv->aec_parms.reg_frame_vts = ((vts_h_tmp << 8) | vts_l_tmp);
            priv->aec_parms.pclk_freq = priv->aec_parms.reg_frame_hts * 2 * \
                                        priv->aec_parms.reg_frame_vts * MAX_FPS;
            priv->aec_parms.calc_vts_tmp = (priv->aec_parms.reg_frame_vts * MAX_FPS * 100);
#endif
#if 1
            if (priv->fps_info.to_fps > 0)
            {
                gc20c3_sen_set_fps_func(priv, priv->fps_info.to_fps);
            }
            gc20c3_sen_updata_exp_time_func(priv, priv->ae_fast.sensor_exp_time);
            priv->fps_info.current_fps = priv->fps_info.to_fps;
#endif
            gc20c3_sen_update_a_gain_func(priv, priv->ae_fast.sensor_a_gain);
            gc20c3_set_flip_mirror(&priv->subdev, priv->sensor_param.flip_mirror, 1);
        }
#endif
        if (preg_info->reg_addr == WIN_OFF_X_L)
            priv->aec_parms.off_x = preg_info->value;
        else if (preg_info->reg_addr == WIN_OFF_Y_L)
            priv->aec_parms.off_y = preg_info->value;
        else if (preg_info->reg_addr == WIN_OFF_Y_L)
            priv->aec_parms.off_y = preg_info->value;
        else if (preg_info->reg_addr == HTS_REG_L)
            hts_l_tmp = preg_info->value;
        else if (preg_info->reg_addr == HTS_REG_H)
            hts_h_tmp = preg_info->value;
        else if (preg_info->reg_addr == VTS_REG_L)
            vts_l_tmp = preg_info->value;
        else if (preg_info->reg_addr == VTS_REG_H)
            vts_h_tmp = preg_info->value;

        gc20c3_write(client, preg_info->reg_addr, preg_info->value);

#if 0
        {
            int value;

            value = gc20c3_read(client, preg_info->reg_addr);
            pr_err("reg:%x write value:%x, read back value:%x\n",
                    preg_info->reg_addr, preg_info->value, value);
        }
#endif

        preg_info++;
    }
#ifdef CONFIG_MACH_KM01A
    priv->init_flag = 1;
#endif

    //if (priv->aec_parms.reg_frame_hts<=0)
        priv->aec_parms.reg_frame_hts = ((hts_h_tmp << 8) | hts_l_tmp);
    //if (priv->aec_parms.reg_frame_vts<=0)
        priv->aec_parms.reg_frame_vts = ((vts_h_tmp << 8) | vts_l_tmp);

    priv->aec_parms.pclk_freq = priv->aec_parms.reg_frame_hts * 2 * \
                                priv->aec_parms.reg_frame_vts * MAX_FPS;
    priv->aec_parms.calc_vts_tmp = (priv->aec_parms.reg_frame_vts * MAX_FPS * 100);

     pr_debug("pclk %d vts %d hts %d calc_tmp %d\n",
             priv->aec_parms.pclk_freq,
             priv->aec_parms.reg_frame_vts,
             priv->aec_parms.reg_frame_hts,
             priv->aec_parms.calc_vts_tmp);

#ifndef CONFIG_MACH_KM01A
    priv->aec_parms.pclk_freq = priv->aec_parms.reg_frame_hts * 2 * \
     priv->aec_parms.reg_frame_vts * MAX_FPS;
#endif

#ifdef CONFIG_MACH_KM01A
    if (priv->fps_info.current_fps == 0)
        priv->fps_info.current_fps = MAX_FPS;
    priv->fps_info.to_fps = priv->fps_info.current_fps;
    priv->fps_info.to_fps_value = priv->aec_parms.reg_frame_vts;
    priv->fps_info.reg_fps_value = priv->aec_parms.reg_frame_vts;
#else
    priv->fps_info.current_fps = MAX_FPS;
    priv->fps_info.to_fps = priv->fps_info.current_fps;
    priv->fps_info.to_fps_value = priv->aec_parms.reg_frame_vts;
    priv->fps_info.reg_fps_value = priv->aec_parms.reg_frame_vts;
#endif
    pr_debug("init.cu_fps %d\n", priv->fps_info.current_fps);
    pr_debug("init.vi_fps %d, aov_fps %d\n", priv->fps_info.video_fps, priv->fps_info.aov_fps);

    return 0;
}

/*read sensor register*/
static int gc20c3_sen_read_reg_func(void* arg, const int reg_addr)
{
    struct gc20c3_priv* priv = arg;
    struct i2c_client* client = priv->client;

    return gc20c3_read(client, reg_addr);
}
/*write sensor register*/
static int gc20c3_sen_write_reg_func(
    void* arg, const int reg_addr, int value)
{
    struct gc20c3_priv* priv = arg;
    struct i2c_client* client = priv->client;

    return gc20c3_write(client, reg_addr, value);
}
/*read sensor register, NO i2c ops*/
static int gc20c3_sen_read_id_func(void* arg) // no use IIC bus
{
    return SENSOR_ID;
}

/*
    release_AEC_v1.2.0_00&02&03&05&06_liner_GC20C3.txt
*/
unsigned int regValTable[13][7] = {
  //   0d04  0d05  0e36  0e39   04a8   04a9   0052      |  实际倍数   | Again dB|
     { 0x00, 0x01, 0x15, 0x15,  0x01,  0x00,  0x64},    //|  X1        | 0.00    |
     { 0x00, 0x02, 0x15, 0x15,  0x01,  0x1b,  0x64},    //|  X1.43     | 3.09    |
     { 0x00, 0x03, 0x16, 0x16,  0x02,  0x00,  0x64},    //|  X2.01     | 6.05    |
     { 0x00, 0x04, 0x17, 0x17,  0x02,  0x37,  0x64},    //|  X2.87     | 9.17    |
     { 0x00, 0x05, 0x17, 0x17,  0x04,  0x02,  0x84},    //|  X4.03     | 12.11   |
     { 0x00, 0x06, 0x18, 0x18,  0x05,  0x32,  0x84},    //|  X5.79     | 15.25   |
     { 0x00, 0x07, 0x19, 0x19,  0x08,  0x05,  0x84},    //|  X8.09     | 18.16   |
     { 0x04, 0x97, 0x1a, 0x1a,  0x0b,  0x10,  0x84},    //|  X11.25    | 21.03   |
     { 0x08, 0x07, 0x1b, 0x1b,  0x10,  0x04,  0x84},    //|  X16.07    | 24.12   |
     { 0x0a, 0x4f, 0x1c, 0x1c,  0x16,  0x22,  0x88},    //|  X22.54    | 27.06   |
     { 0x0c, 0x07, 0x1d, 0x1d,  0x20,  0x06,  0x88},    //|  X32.10    | 30.13   |
     { 0x0d, 0x2f, 0x1e, 0x1e,  0x2d,  0x10,  0x88},    //|  X45.26    | 33.11   |
     { 0x0e, 0x07, 0x20, 0x20,  0x3f,  0x23,  0x72},    //|  X63.56    | 36.06   |
};

unsigned int gainLevelTable[14] = {
    64,
    91,
    128,
    183,
    257,
    370,
    517,
    720,
    1028,
    1442,
    2054,
    2896,
    4067,
    0xffffffff,
};

int SetSensorGain(struct gc20c3_priv* priv, unsigned int gain)
{
    int i;

    unsigned int tol_dig_gain = 0;
    int total = sizeof(gainLevelTable) / sizeof(gainLevelTable[0]);
    unsigned int tmp_gain = gain / 4;  //1/64 ISP传参精度
    struct i2c_client* client = priv->client;

    if (priv->aec_parms.curr_again_level>0)
        total = priv->aec_parms.curr_again_level;
    for (i = 0; i < total-2 ; i++)
    {
        if ((gainLevelTable[i] <= tmp_gain) && (tmp_gain < gainLevelTable[i + 1]))
            break;
    }

    tol_dig_gain = tmp_gain * 1024 / gainLevelTable[i];
    //tol_dig_gain = tmp_gain*64 / gainLevelTable[i];

    //高温逻辑，限制dgain
    if(tol_dig_gain>2*1024)
        tol_dig_gain=2*1024;

    //pr_err("[%s] gain:%d total_dig_gain:%d\n", __func__, tmp_gain, tol_dig_gain);

#if SET_AGAIN_EN
    gc20c3_write(client, 0x0d04, regValTable[i][0]);
    gc20c3_write(client, 0x0d05, regValTable[i][1]);
    gc20c3_write(client, 0x0e36, regValTable[i][2]);
    gc20c3_write(client, 0x0e39, regValTable[i][3]);
    gc20c3_write(client, 0x04a8, regValTable[i][4]);
    gc20c3_write(client, 0x04a9, regValTable[i][5]);
    gc20c3_write(client, 0x0052, regValTable[i][6]);

    gc20c3_write(client, 0x0474, (tol_dig_gain >> 8) & 0xff);
    gc20c3_write(client, 0x0475, (tol_dig_gain & 0xff));
#endif
    return 0;
}

/*set sensor again*/
static int gc20c3_sen_update_a_gain_func(
    void* arg, const unsigned int a_gain)
{
    /*
    * max again < (4067/64)x * 256 = 63.546x * 256
    * */
    struct gc20c3_priv* priv = arg;
    struct i2c_client* client = priv->client;

    priv->aec_parms.curr_corse_gain = a_gain;

    SetSensorGain(priv, a_gain); //64=1x

    return A_GAIN_EFFECT_FRAMES;
}

/*set sensor dgain*/
static int gc20c3_sen_update_d_gain_func(
    void* arg, const unsigned int d_gain)
{
    /* don't use */
    return 0;
}
/*set sensor exp time*/
static int __set_reg_exp_time(struct gc20c3_priv* priv, int exp_time)
{
    int ret = 0;
    struct i2c_client* client = priv->client;
    int exp_h, exp_l;

    if (exp_time < EXP_MIN)
        exp_time = EXP_MIN;
    if (exp_time > 65536)
        exp_time = 65536; // 2^16

    //gc20c3 do not need 2N.
    // exp_time /=2;
    // exp_time *= 2;

    exp_l = ((exp_time)) & 0xff;
    exp_h = (exp_time >> 8) & 0xff;

#if SET_EXP_EN
    gc20c3_write(client, EXP_REG_H, exp_h);
    gc20c3_write(client, EXP_REG_L, exp_l);
#endif

    return ret;
}

static int __set_reg_frame_vts(struct gc20c3_priv* priv, int vts)
{
    int ret = 0;
    struct i2c_client* client = priv->client;
    if (vts < 1)
        vts = 1;

    pr_debug("%s, vts:%d\n", __func__, vts);
#if SET_FPS_EN
     if ((priv->master_or_slave == SLAVER_MODE && priv->slave_sync_mode == DUAL_SYNC_BY_EFSYNC)) {
         if (priv->fps_info.reg_fps_value < vts) { //从高帧率切到低帧率(vts变大)
             gc20c3_write(client, 0x031d, 0x2d);
             gc20c3_write(client, 0x0340, ((vts-2) >> 8));
             gc20c3_write(client, 0x0341, ((vts-2) & 0xff));
             gc20c3_write(client, 0x023d, ((vts-3) >> 8));
             gc20c3_write(client, 0x023e, ((vts-3) & 0xff));
             gc20c3_write(client, 0x031d, 0x28);

         } else if (priv->fps_info.reg_fps_value > vts) {//从低帧率切到高帧率(vts变小)
             gc20c3_write(client, 0x031d, 0x2d);
             gc20c3_write(client, 0x0340, ((vts-2) >> 8));
             gc20c3_write(client, 0x0341, ((vts-2) & 0xff));
             gc20c3_write(client, 0x031d, 0x28);
             gc20c3_write(client, 0x031d, 0x2e);
             gc20c3_write(client, 0x023d, ((vts-3) >> 8));
             gc20c3_write(client, 0x023e, ((vts-3) & 0xff));
             gc20c3_write(client, 0x031d, 0x28);
         }
     } else{
         gc20c3_write(client, VTS_REG_H, (vts >> 8));
         gc20c3_write(client, VTS_REG_L, (vts & 0xff));
     }

#endif
    priv->fps_info.reg_fps_value = vts;

    return ret;
}

static int __get_reg_frame_vts(struct gc20c3_priv* priv)
{
    struct i2c_client* client = priv->client;

    return ((gc20c3_read(client, VTS_REG_H)) << 8 | gc20c3_read(client, VTS_REG_L));
}

static void update_frame_vts_and_exp_ctrl(struct gc20c3_priv* priv)
{
    struct i2c_client* client = priv->client;

    /*
     * exp_time =1 mean half line exptime
     * max exp_time = {R0x0202,R0x0203} - 8
     *
     * now:
     * at max_exptime_30fps
     * = {R0x0202,R0x0203} - 8
     * = 0x465 - 8 //shaokc...update!!!
     * = 1125 -8
     * = 1117
     *
     * exp_reg_value = VTS - exp_time !
     *
     */

    int const cur_frame_vts = __get_reg_frame_vts(priv);

    int const max_exp_ctrl = cur_frame_vts - 8; // VTS - 8

    int const target_frame_vts = (priv->fps_info.to_fps_value);

    int target_exp_ctrl = (priv->aec_parms.target_exp_ctrl > max_exp_ctrl) ?
        max_exp_ctrl : priv->aec_parms.target_exp_ctrl;


    if ((target_frame_vts <= 0) || (target_exp_ctrl <= 0))
        return;

    if (target_frame_vts >= cur_frame_vts) { //切低帧率
        if (target_frame_vts != cur_frame_vts)
            __set_reg_frame_vts(priv, target_frame_vts);
        __set_reg_exp_time(priv, target_exp_ctrl);
    } else { //切高帧率
        __set_reg_exp_time(priv, target_exp_ctrl);
        __set_reg_frame_vts(priv, target_frame_vts);
    }
}

/*set sensor exp time*/
static int gc20c3_sen_updata_exp_time_func(void* arg, unsigned int exp_time)
{
    struct gc20c3_priv* priv = arg;
    struct i2c_client* client = priv->client;

    priv->aec_parms.target_exp_ctrl = exp_time;
    priv->ae_fast.sensor_exp_time = exp_time;
    update_frame_vts_and_exp_ctrl(priv);

    return EXP_EFFECT_FRAMES;
}

//TODO YJH
static int aec_parms_init(struct gc20c3_priv* priv)
{
    priv->aec_parms.curr_again_level = 0;
    priv->aec_parms.curr_again_10x = -1;
    priv->aec_parms.r0x3e02_value = 0;
    priv->aec_parms.r0x3e01_value = 0;
    priv->aec_parms.r0x3e00_value = 0;
    priv->aec_parms.curr_2x_dgain = 0;
    priv->aec_parms.curr_corse_gain = -1;
    priv->aec_parms.backup_temp_flag = 0;

    priv->aec_parms.reg_frame_vts = 0x0642;
    priv->aec_parms.reg_frame_hts = 0x0203;
    priv->aec_parms.pclk_freq = priv->aec_parms.reg_frame_hts * 2 * \
                                priv->aec_parms.reg_frame_vts * MAX_FPS;

    return 0;
}

static int gc20c3_get_temperature(void* arg)
{
    struct gc20c3_priv* priv = arg;
    struct i2c_client* client = priv->client;

    int i;
    int temperature_value;
    int temperature_flag;
    const  int temperature_total = sizeof(gainLevelTable) / sizeof(gainLevelTable[0]);	//完整曝光表长短
    int total = priv->aec_parms.curr_again_level;		//被限制的曝光表长度

    if (total<=0)
        total = temperature_total;

    temperature_flag = priv->aec_parms.backup_temp_flag;
    temperature_value = gc20c3_read(client, 0x040c);

    if(temperature_value>=0x20) //进入高温逻辑的阈值判断
    {
        priv->aec_parms.temp_count=priv->aec_parms.temp_count+1;
        if(priv->aec_parms.temp_count>2)
        {
          temperature_flag=1;  //进入高温标志
          priv->aec_parms.temp_count=0;
        }
    }
    else if(priv->aec_parms.temp_count<3)
    {
         priv->aec_parms.temp_count=0;
    }

    if(temperature_flag==1)
    {
          if(temperature_value>0x1d)  // 这个值越大，在同样环境下，最大档模拟增益会越大；值越小，最大档模拟增益会越小，也就是增益限制的越多；
          {
            total=total-1;
            if(total<8) total=8;  //8x  // 可以限制到的最大模拟增益挡位，当total = 8时，i的最大值为6 (i=8-2)
          }

          if(temperature_value<0x18)  // 随着温度降低，ob统计值开始降低，对模拟增益挡位逐渐恢复；
          {
              total=total+1;
              if(total>temperature_total)     // 恢复到正常(常温)模拟增益挡位，退出高温逻辑；
              {
                total=temperature_total;
                temperature_flag=0;
              }
          }
    }
   //if(gc20c3_again<5*64)  //根据增益退出，此增益是sensor的总增益
    if(priv->aec_parms.curr_corse_gain<5*256) // < 5x,退出高温逻辑
    {
       temperature_flag=0;
       total=temperature_total;
       priv->aec_parms.temp_count=0;
    }

    if ( (temperature_flag!= priv->aec_parms.backup_temp_flag) ||  \
                        total!=priv->aec_parms.curr_again_level)//高温逻辑发生变化或高温加剧，则set新的gain值
    {
        priv->aec_parms.curr_again_level = total;
        priv->aec_parms.backup_temp_flag = temperature_flag;
        if (priv->aec_parms.curr_corse_gain!=-1)
            SetSensorGain(priv, priv->aec_parms.curr_corse_gain);
    }

    pr_debug("temperature_flag=%x,gc20c3_again=%d,total=%d,0x040c=%x\n", temperature_flag,
             priv->aec_parms.curr_corse_gain,total,temperature_value);

    return 0;
}

/*sensor timer*/
static int gc20c3_sen_timer_func(void* arg)
{
    struct gc20c3_priv* priv = arg;
    struct i2c_client* client = priv->client;

#ifndef CONFIG_MACH_KM01A
    //高温逻辑隔10帧进行调用
    priv->sensor_param.framecount++;
    if (priv->sensor_param.framecount >= 10)
    {
       gc20c3_get_temperature(arg);
       priv->sensor_param.framecount = 0;
       //pr_err("%s calling\n", __func__);
    }
#endif
    return 0;
}

#ifdef CONFIG_MACH_KM01A
static int gc20c3_soft_standby_in(struct gc20c3_priv* priv)
{
    struct i2c_client* client = priv->client;

    pr_err("%s calling\n", __func__);
#if 0
    gc20c3_write(client, 0x0202, 0x01); //exp? 是否可设置为需要值
    gc20c3_write(client, 0x0203, 0x00);
    gc20c3_write(client, 0x0261, 0x13);
    gc20c3_write(client, 0x03bc, 0x0f);
    gc20c3_write(client, 0x03b0, 0x03);

    gc20c3_write(client, 0x0209, 0x01);
    gc20c3_write(client, 0x03b1, 0x02);
    gc20c3_write(client, 0x03b5, 0x11);
    gc20c3_write(client, 0x03b6, 0x00);
    gc20c3_write(client, 0x03b7, 0x20);
    gc20c3_write(client, 0x03b9, 0x00);
    gc20c3_write(client, 0x03ba, 0x40);
#endif

    //需要在vb位置写standby
    gc20c3_write(client, 0x031c, 0x18);
    gc20c3_write(client, 0x03bb, 0x00);
    gc20c3_write(client, 0x03be, 0x00);
    gc20c3_write(client, 0x0d10, 0x04);
    gc20c3_write(client, 0x0b4d, 0x00);
    gc20c3_write(client, 0x0100, 0x00);
    gc20c3_write(client, 0x0d40, 0x00);

    if (IS_SENSOR_STATUS(E_SENSOR_INIT))
    {
        return 0;
    }
    UPDATE_SENSOR_STATUS(E_SENSOR_ACCESS_STANDBY);

    return 0;
}

static int gc20c3_soft_standby_out(struct gc20c3_priv* priv)
{
    struct i2c_client* client = priv->client;

    pr_err("%s calling\n", __func__);
#if 0
    gc20c3_write(client, 0x0261, 0x1a);
    gc20c3_write(client, 0x03bc, 0x00);
    gc20c3_write(client, 0x03b0, 0x06);

    gc20c3_write(client, 0x0209, 0x00);
    gc20c3_write(client, 0x03b1, 0x00);
    gc20c3_write(client, 0x03b5, 0x2b);
    gc20c3_write(client, 0x03b6, 0xf2);
    gc20c3_write(client, 0x03b7, 0x20);
    gc20c3_write(client, 0x03b9, 0x00);
    gc20c3_write(client, 0x03ba, 0x40);
#endif

    gc20c3_write(client, 0x03fe,0x10);
    gc20c3_write(client, 0x0d40,0x01);
    gc20c3_write(client, 0x0b4d,0x02);
    gc20c3_write(client, 0x03be,0x7f);
    gc20c3_write(client, 0x03bb,0xff);
    gc20c3_write(client, 0x0d10,0x06);
    //usleep(100); //100us
    mdelay(1);
    gc20c3_write(client, 0x0d10,0x07);
    gc20c3_write(client, 0x0100,0x03);
    gc20c3_write(client, 0x031c,0x1f);
    gc20c3_write(client, 0x0336,0x01);
    gc20c3_write(client, 0x0336,0x00);
    gc20c3_write(client, 0x03fe,0x00);

    return 0;
}
#endif

#ifdef CONFIG_MACH_KM01A
/*standby in*/
static int gc20c3_sen_set_standby_in_func(void* arg)
{
    struct gc20c3_priv* priv = arg;

    struct i2c_client* client = priv->client;
    gc20c3_soft_standby_in(priv);

    return 0;
}

/*standby out*/
static int gc20c3_sen_set_standby_out_func(void* arg)
{
    struct gc20c3_priv* priv = arg;

    {
        {
            struct i2c_client* client = priv->client;

            //sensor stream on
            gc20c3_soft_standby_out(priv);

            UPDATE_SENSOR_STATUS(E_SENSOR_WORK);
        }
        //AOV sensor init work time
    }

    return 0;
}
#else
/*standby in*/
static int gc20c3_sen_set_standby_in_func(void* arg) { return 0; }

/*standby out*/
static int gc20c3_sen_set_standby_out_func(void* arg) { return 0; }
#endif

/*low level read sensor ID, user i2c ops*/
static int gc20c3_sen_probe_id_func(void* arg) // use IIC bus
{
    struct gc20c3_priv* priv = arg;
    struct i2c_client* client = priv->client;
    int id;
    int value;


    value = gc20c3_read(client, ID_REG_H);
    id = value << 8;

    value = gc20c3_read(client, ID_REG_L);
    id |= value;

    pr_err("id:%x\n", id);

    if ( (id==ACTUAL_SENSOR_ID_A) || (id==ACTUAL_SENSOR_ID_B) )
        return SENSOR_ID;

    return 0;
}
/*get resolution*/
static int gc20c3_sen_get_resolution_func(
    void* arg, int* width, int* height)
{
    *width = SENSOR_OUTPUT_WIDTH;
    *height = SENSOR_OUTPUT_HEIGHT;
    return 0;
}
/*get mclk*/
static int gc20c3_sen_get_mclk_func(void* arg) { return SENSOR_MCLK; }
/*get current fps*/
static int gc20c3_sen_get_fps_func(void* arg)
{
    struct gc20c3_priv* priv = arg;

    return priv->fps_info.current_fps;
}
/*get valid coordinate*/
static int gc20c3_sen_get_valid_coord_func(void* arg, int* x, int* y)
{
    *x = SENSOR_VALID_OFFSET_X;
    *y = SENSOR_VALID_OFFSET_Y;
    return 0;
}
/*get bus type*/
static enum sensor_bus_type gc20c3_sen_get_bus_type_func(void* arg)
{
    return SENSOR_BUS_TYPE;
}

#ifdef CONFIG_MACH_KM01A
static int get_ae_fast_default(void* arg,struct ae_fast_struct* ae_fast)
{
    struct gc20c3_priv* priv = arg;
    *ae_fast = priv->ae_fast;
    return 0;
}

static int sensor_set_fast_ae(struct v4l2_subdev* sd,struct ae_fast_struct *ae_fast)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc20c3_priv* priv = to_gc20c3(client);
    if(ae_fast)
    {
        pr_err("%s-exp_time:%d\n",__func__,ae_fast->sensor_exp_time);
        if(ae_fast->sensor_exp_time > 0){
            memcpy(&priv->ae_fast, ae_fast, sizeof(struct ae_fast_struct));
        }
        else{
            memcpy(ae_fast, &priv->ae_fast, sizeof(struct ae_fast_struct));    \
        }
        //pr_err("%s-sensor_exp_time=%d\n",__func__,ae_fast_default.sensor_exp_time);
    }

    return 0;
}

static int gc20c3_get_binging_max_selection(struct _sensor_bining_info *binging)
{
    binging->width = SENSOR_BINING_WIDTH;
    binging->height = SENSOR_BINING_HEIGHT;
    return 0;
}
#elif ( defined(CONFIG_SYS_FAST_LAUNCH) )
static int sensor_set_fast_ae(void* arg, struct ae_fast_struct *ae_fast)
{
    struct gc20c3_priv* priv = arg;
    if(ae_fast)
    {
        pr_err("%s-exp_time:%d\n",__func__,ae_fast->sensor_exp_time);
        if(ae_fast->sensor_exp_time > 0){
            memcpy(&priv->ae_fast, ae_fast, sizeof(struct ae_fast_struct));
        }
        else{
            memcpy(ae_fast, &priv->ae_fast, sizeof(struct ae_fast_struct));    \
        }
        //pr_err("%s-sensor_exp_time=%d\n",__func__,ae_fast_default.sensor_exp_time);
    }

    return 0;
}

static int get_ae_fast_default(void* arg,struct ae_fast_struct* ae_fast)
{
    struct gc20c3_priv* priv = arg;
    *ae_fast = priv->ae_fast;
    return 0;
}
#else
static int get_ae_fast_default(struct ae_fast_struct* ae_fast)
{
    *ae_fast = ae_fast_default;
    return 0;
}
#endif

/*get self definded params*/
static int gc20c3_sen_get_parameter_func(void* arg, int param, void* value)
{
    int ret = 0;
    struct gc20c3_priv* priv = arg;

    //pr_err("%s param:%d getting support\n", __func__, param);
    switch (param) {
        case GET_INTERFACE:
            *((int*)value) = dvp ? DVP_INTERFACE : MIPI_INTERFACE;
            break;

        case GET_IO_LEVEL:
            *((int*)value) = SENSOR_IO_LEVEL;
            break;

        case GET_MIPI_MHZ:
            *((int*)value) = MIPI_MBPS;
            break;
#ifdef CONFIG_MACH_KM01A
        case GET_MIPI_LANES_SWAP:
            *((int*)value) = MIPI_LANES_SWAP;
            break;
#endif
        case GET_MIPI_LANES:
            *((int*)value) = MIPI_LANES;
            break;

        case GET_RESET_GPIO:
            *((int*)value) = priv->gpio_reset;
            break;

        case GET_PWDN_GPIO:
            *((int*)value) = priv->gpio_pwdn;
            break;

        case GET_DUAL_SYNC_MODE:
            *((int*)value) = priv->slave_sync_mode;//DUAL_SYNC_BY_EFSYNC;
            break;

        case GET_MAX_FPS:
            *((int*)value) = MAX_FPS;
            break;

#if defined(CONFIG_MACH_AK3918AV130)
        case GET_REAL_HEIGHT:
            *((int*)value) = SENSOR_REAL_HEIGHT;
            break;

        case GET_SYNC_DROP_FRAME:
            *((int*)value) = SENSOR_SYNC_DROP_FRAMES;
            break;
#endif

        //YJH:最新的fastae功能需要挪走到gc20c3_core_s_ctrl
#if ( !defined(CONFIG_MACH_KM01A) && defined(CONFIG_SYS_FAST_LAUNCH) )
        case SET_FAST_AE:
            ret = sensor_set_fast_ae(arg,value);
            break;

#endif
        case GET_AE_FAST_DEFAULT:

#if (defined(CONFIG_SYS_FAST_LAUNCH))
            get_ae_fast_default(arg,value);
#else
            get_ae_fast_default(value);
#endif
            break;
#ifdef CONFIG_MACH_KM01A
        case GET_RAW_FORMAT:
            *((int*)value) = RAW_FORMAT;
            break;
        case GET_BINING_MAX:
            ret = gc20c3_get_binging_max_selection((struct _sensor_bining_info *)value);
            break;
        case GET_AOV_MASTER_MODE:
            *((int*)value) = aov_master_mode;
            break;
        case GET_STEPS_OF_1LINE_EXP:
            *((int*)value) = ONE_LINE_EXP_OF_STEPS;
            break;
        case GET_MAX_EXP:
            //if(MAX_FPS == 120)
            //    *((int*)value) = MAX_EXP_OF_FPS120;
            //else
                *((int*)value) = MAX_EXP_OF_FPS30;
            break;
        case GET_MAX_AGAIN:
            *((int*)value) = MAX_A_GAIN_X;
            break;
        case GET_MAX_DGAIN:
            *((int*)value) = MAX_D_GAIN_X;
            break;
        case GET_HBLIANK_CYCLE:
            *((int*)value) = HBLIANK_CYCLE;
            break;
        case GET_ONE_LINE_CYCLE:
            *((int*)value) = ONE_LINE_CYCLE;
            break;
        //case GET_EXP_BINING2NORMAL:
        //    *((int*)value) = EXP_BINING2NORMAL;
        //    break;
#endif
        default:
            pr_err("%s param:%d not support\n", __func__, param);
            ret = -1;
            break;
    }

    return ret;
}
/*sensor power on*/
static int gc20c3_sen_set_power_on_func(void* arg)
{
    struct gc20c3_priv* priv = arg;

#ifdef CONFIG_MACH_KM01A
    if (priv->gpio_power >= 0) {
        //pr_err("gpio_power:%d\n",priv->gpio_power);
        gpio_direction_output(priv->gpio_power, SENSOR_POWER_LEVEL);
        //mdelay(5);
    }
    if (priv->gpio_reset >= 0) {
        if(sensor_power_on  && priv->master_or_slave != SLAVER_AOV_MODE){
            //gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
            //mdelay(50);
            gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
            mdelay(50);
            gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
            mdelay(80);
            pr_err("%s %d not slaver_AOV_MODE\n", __func__, __LINE__);
        }
        else{
            gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
            pr_err("%s %d power_on:%d or slaver_AOV_MODE\n", __func__, __LINE__, sensor_power_on);
        }
    }
#else
    if (priv->gpio_reset >= 0 && priv->gpio_reset != 0xffff)
    {
        if (sensor_power_on) {
            gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
            msleep(20);
            gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
            msleep(50);
            gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
            msleep(90);
        } else {
            gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
            mdelay(10);
        }
    }
#endif
    if (priv->gpio_pwdn >= 0&& priv->gpio_pwdn != 0xffff)
    {
        gpio_direction_output(priv->gpio_pwdn, !SENSOR_PWDN_LEVEL);
    }
    aec_parms_init(priv);

    return 0;
}
/*sensor power off*/
static int gc20c3_sen_set_power_off_func(void* arg)
{
    struct gc20c3_priv* priv = arg;

    if (priv->gpio_reset >= 0 && priv->gpio_reset != 0xffff)
    {
        gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
    }

    if (priv->gpio_pwdn >= 0 && priv->gpio_pwdn != 0xffff)
    {
        gpio_direction_output(priv->gpio_pwdn, SENSOR_PWDN_LEVEL);
    }
#ifdef CONFIG_MACH_KM01A
    if (priv->gpio_reset >= 0 && priv->master_or_slave != SLAVER_AOV_MODE)
    {
        gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
    }

    if (priv->gpio_power >= 0)
    {
        gpio_direction_output(priv->gpio_power, !SENSOR_POWER_LEVEL);
    }
#endif
    // sensor_power_on = 1;

    return 0;
}
/*set sensor fps*/
static int gc20c3_fps_to_vts(struct gc20c3_priv* priv, const int fps)
{
    /*
     * gc20c3:
     * pclk= HTS*VTS* init_seq_fps
     *
     */
    int vts = 0;
    int fps_tmp;

    pr_debug("%s %d fps:%d\n", __func__, __LINE__, fps);

    switch (fps) {
        case 12:
            fps_tmp = 1250;
            break;

        case 14:
            fps_tmp = 1428;
            break;

        default:
            fps_tmp = fps * 100;
            break;
    }

    if (priv->master_or_slave != SINGLE_MODE && priv->master_or_slave != MASTER_MODE) {
        if( priv->slave_sync_mode == DUAL_SYNC_PWM_FREQ_FIXED){ //YJH:测试强制同步方式，跑下面逻辑会出现跨帧曝光问题。
            if (fps != MAX_FPS){
                //fps_tmp = fps_tmp * 10000 / 9974;   //  从模式时，帧率要比PWM频率快一点
                fps_tmp = fps_tmp * 1000000 / (1000000 - fps_tmp*3);
            }
        }
    }
#if ( defined(CONFIG_MACH_KM01A) || defined(CONFIG_SYS_FAST_LAUNCH) )
    vts = (priv->aec_parms.calc_vts_tmp) / fps_tmp;
    // vts = (3408000UL) / vts;
#else
    if (priv->aec_parms.reg_frame_hts) {
        vts = priv->aec_parms.pclk_freq / priv->aec_parms.reg_frame_hts / 2;
        vts = vts * 100;
        vts = vts / fps_tmp;
    }
#endif
    return vts;
}

/*set sensor fps*/
static int gc20c3_sen_set_fps_func(void* arg, const int fps)
{
    int tmp;
    struct gc20c3_priv* priv = arg;

    tmp = gc20c3_fps_to_vts(priv, fps);

    if (tmp > 0) {
        priv->fps_info.to_fps_value = tmp;
        priv->fps_info.to_fps = fps;
    }
    return 0;
}

static void gc20c3_set_fps_async(struct gc20c3_priv* priv)
{
    struct i2c_client* client = priv->client;

    if (priv->fps_info.to_fps != priv->fps_info.current_fps) {
        update_frame_vts_and_exp_ctrl(priv);
        priv->fps_info.current_fps = priv->fps_info.to_fps;
    }
}

#ifdef CONFIG_MACH_KM01A
#ifdef aov_slave_mode
static int ak_sensor_fsync_func(void* arg)
{
    struct gc20c3_priv* priv = arg;
    struct i2c_client* client = priv->client;

    return 0;
}

static int ak_sensor_get_current_rb_rows_func(void* arg)
{
    struct gc20c3_priv* priv = arg;
    int time = 0;

    return time;
}
#else
/*
 * NOTE: 目前修改为在中断里面调用,不希望有休眠函数,I2C操作暂时不允许在这里操作*/
static int ak_sensor_event_func(void* arg, int evt)
{
    struct gc20c3_priv* priv = arg;



    if ((priv->master_or_slave != SLAVER_AOV_MODE) && !IS_SENSOR_STATUS(E_SENSOR_STANDBY))
        return 0;

    pr_err("master_or_slave:%d, status:%d\r\n", priv->master_or_slave, priv->status);
    switch (evt)
    {
        case EVENT_FRAME_TARGET:
            pr_err("EVENT_FRAME_TARGET sensor stream off\r\n");
            //sensor tream on
            if (IS_SENSOR_STATUS(E_SENSOR_WORK)){
                //gc20c3_soft_standby_in(priv);
                UPDATE_SENSOR_STATUS(E_SENSOR_ACCESS_STANDBY);
            }
            break;
        case EVENT_FRAME_ALLDONE:
            pr_err("EVENT_FRAME_ALLDONE priv->status:%d\r\n", priv->status);
            if (IS_SENSOR_STATUS(E_SENSOR_ACCESS_STANDBY)){
                if (priv->gpio_pwdn >= 0){
                    //pr_err("EVENT_FRAME_ALLDONE in standby\r\n");
                    //sensor in standby
                    //UPDATE_SENSOR_STATUS(E_SENSOR_STANDBY);

                    //gc20c3_soft_standby_in(priv);//见本函数的NOTE说明
                    //gpio_direction_output(priv->gpio_pwdn, SENSOR_PWDN_LEVEL);//中断调用会有问题用gpio_set_value()函数替代
                    gpio_set_value(priv->gpio_pwdn, SENSOR_PWDN_LEVEL);
                }
            }
            break;
    }

    return 0;
}
#endif
#endif

static AK_ISP_SENSOR_CB gc20c3_cb = {
    .sensor_init_func = gc20c3_sen_init_func,
    .sensor_read_reg_func = gc20c3_sen_read_reg_func,
    .sensor_write_reg_func = gc20c3_sen_write_reg_func,
    .sensor_read_id_func = gc20c3_sen_read_id_func,
    .sensor_update_a_gain_func = gc20c3_sen_update_a_gain_func,
    .sensor_update_d_gain_func = gc20c3_sen_update_d_gain_func,
    .sensor_updata_exp_time_func = gc20c3_sen_updata_exp_time_func,
    .sensor_timer_func = gc20c3_sen_timer_func,
    .sensor_set_standby_in_func = gc20c3_sen_set_standby_in_func,
    .sensor_set_standby_out_func = gc20c3_sen_set_standby_out_func,
    .sensor_probe_id_func = gc20c3_sen_probe_id_func,
    .sensor_get_resolution_func = gc20c3_sen_get_resolution_func,
    .sensor_get_mclk_func = gc20c3_sen_get_mclk_func,
    .sensor_get_fps_func = gc20c3_sen_get_fps_func,
    .sensor_get_valid_coordinate_func
    = gc20c3_sen_get_valid_coord_func,
    .sensor_get_bus_type_func = gc20c3_sen_get_bus_type_func,
    .sensor_get_parameter_func = gc20c3_sen_get_parameter_func,
    .sensor_set_power_on_func = gc20c3_sen_set_power_on_func,
    .sensor_set_power_off_func = gc20c3_sen_set_power_off_func,
    .sensor_set_fps_func = gc20c3_sen_set_fps_func,
#ifdef CONFIG_MACH_KM01A
#if 0 //无实现gc20c3 aov功能上使用slave mode
//  #ifdef aov_slave_mode
//      .sensor_fsync_func = ak_sensor_fsync_func,
//      .sensor_get_current_rb_rows_func = ak_sensor_get_current_rb_rows_func,
//      .sensor_get_aov_flag_func = ak_sensor_get_aov_flag_func,
//  #else
#endif
    .sensor_event_func = ak_sensor_event_func,
//  #endif
#endif
};

static int sensor_id_func(void) { return gc20c3_sen_read_id_func(NULL); }

static char* sensor_if_func(void)
{
    static char ifstr[16] = "dvp";

    if (!dvp)
        sprintf(ifstr, "mipi%d", MIPI_LANES);

    return ifstr;
}

static int call_sensor_sys_init(void)
{
    return sensor_sys_init(sensor_id_func, sensor_if_func);
}

static int call_sensor_sys_deinit(void)
{
    sensor_sys_exit();
    return 0;
}
/*end of file*/
