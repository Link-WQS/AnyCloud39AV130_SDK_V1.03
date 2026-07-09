/*
 * sc2337p Camera Driver
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

#include "../common/sensor_i2c.h"
#include "../common/sensor_sys.h"
#include "../include/ak_sensor.h"
#include "ak_sensor_priv_cmd.h"
#ifdef CONFIG_MACH_KM01A
#include "sc2337p_1920x1080.h"
#include "sc2337p_1920_1080_slave.h"
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
#define SENSOR_PWDN_LEVEL  0
#define SENSOR_RESET_LEVEL 0
static int SENSOR_I2C_ADDR    =  0x30; //0:0x30 1:0x32
#define SENSOR_PHY_ID         0xcb3a
#define SENSOR_ACTUAL_ID      0x9b3a //用于区分思特微p系列
#define SENSOR_ID             0x23370f //思特微p系列在id增加两位0x0f
#define SENSOR_MCLK           24
#define SENSOR_REGADDR_BYTE   2
#define SENSOR_DATA_BYTE      1
#define SENSOR_OUTPUT_WIDTH   1920
#define SENSOR_OUTPUT_HEIGHT  1080
#define SENSOR_REAL_HEIGHT    1080 //sensor实际输出行数
#define SENSOR_BINING_WIDTH   960
#define SENSOR_BINING_HEIGHT  540
#define SENSOR_VALID_OFFSET_X 0
#define SENSOR_VALID_OFFSET_Y 0
#define SENSOR_BUS_TYPE       BUS_TYPE_RAW
//#define SENSOR_IO_INTERFACE       DVP_INTERFACE
static int SENSOR_IO_INTERFACE = MIPI_INTERFACE;
#define SENSOR_IO_LEVEL IO_LEVEL_1V8
#define MIPI_MBPS       396
//#define MIPI_LANES_SWAP      (0x000531A1)
#define MIPI_LANES_SWAP      (0x435A1)
//#define MIPI_LANES_SWAP      (0x18856)//交换
#define MIPI_LANES      2
#define RAW_FORMAT      3 //raw分量的顺序
static int MAX_FPS = 30;

#if defined(CONFIG_MACH_AK39EV330) || defined(CONFIG_MACH_AK37D) /*H3B&H3D*/
#define EXP_EFFECT_FRAMES    1 /*adjust exp_time for 1/2 frames*/
#define A_GAIN_EFFECT_FRAMES 0 /*adjust a_gain for 1/2 frames*/
#define D_GAIN_EFFECT_FRAMES 0 /*adjust d_gain for 1/2 frames*/
#else /*H322&322L*/
#define EXP_EFFECT_FRAMES    1 /*adjust exp_time for 1/2 frames*/
#define A_GAIN_EFFECT_FRAMES 1 /*adjust a_gain for 1/2 frames*/
#define D_GAIN_EFFECT_FRAMES 1 /*adjust d_gain for 1/2 frames*/
#endif

#define ID_REG_H        (0x3107)
#define ID_REG_L        (0x3108)
#define VTS_REG_H       (0x320e)
#define VTS_REG_L       (0x320f)
#define HTS_REG_H       (0x320c)
#define HTS_REG_L       (0x320d)
#define SLAVE_ACTIVE_BLANK_ROWS_H       (0x322e)
#define SLAVE_ACTIVE_BLANK_ROWS_L       (0x322f)
#define FLIP_MIRROR     (0x3221)
#define EXP_REG_H       (0x3e01)
#define EXP_REG_L       (0x3e02)
#define EXP_REG_E       (0x3e00)
#define AGAIN_REG       (0x3e09)
#define DGAIN_REG       (0x3e06)
#define FINE_DGAIN_REG  (0x3e07)
#define RB_ROW_H   (0x3230)
#define RB_ROW_L   (0x3231)
#define DELAY_FLAG (0xffff)

//#define DEBUG_CIS_PARAM



#define MAX_A_GAIN_X        (53)
#define MAX_D_GAIN_X        (64)
#define ONE_LINE_CYCLE      (1600)
#define HBLIANK_CYCLE       (400)

#define HTS_VALUE   (2200) //TODO

#define AGC_CTRL        (0x3e03)

#define DELAY_FLAG (0xffff)
#define EXP_LIMITE (8)

/*
 * Struct
 */

struct ak_sensor_param
{
    unsigned int flip_mirror;
};

#ifdef CONFIG_SYS_FAST_LAUNCH
//  传感器 AE 转换系数组
static ae_convert_coef_t sc2337p_ae_convert_coef_group[] = {
    {11, 10},    // 第0组：默认系数（无转换）
    {11, 10},    // 第1组：1/2 转换
    {11, 10},    // 第2组：2/3 转换
};
#define SC2337P_AE_CONVERT_GROUP_NUM  (sizeof(sc2337p_ae_convert_coef_group) / sizeof(ae_convert_coef_t))

//  传感器 AWB 转换系数组
static awb_convert_coef_t sc2337p_awb_convert_coef = {
    .r = { .mul = 1,.div = 1 },
    .g = { .mul = 1,.div = 1 },
    .b = { .mul = 1,.div = 1 },
};
#define SC2337P_AWB_CONVERT_GROUP_NUM  (1)

#endif

struct regval_list {
    u8 reg_num;
    u8 value;
};

struct sc2337p_win_size {
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
    int rb_rows;
    int src_rb_rows;
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
struct sc2337p_aec_parms {
    int curr_again_level;
    int curr_again_10x;
    int r0x3e02_value;
    int r0x3e01_value;
    int r0x3e00_value;
    int target_exp_ctrl;
    int curr_2x_dgain;
    int curr_corse_gain;
    int r0x3e08_corse_gain;
    int r0x3e09_fine_gain;
    int reg_frame_hts;
    int reg_frame_vts;
    int pclk_freq;
    int calc_vts_tmp;
    int a_gain;
    int d_gain;
    int curr_max_again_flag; // again must be set to maximum before adjusting dgain
};

static const struct v4l2_ctrl_ops sc2337p_ctrl_ops;
static const struct v4l2_ctrl_config config_sensor_get_id = {
    .ops = &sc2337p_ctrl_ops,
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

struct dual_sensor_attr {
    int init_count;
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
struct sc2337p_priv {
    struct list_head list;
    struct i2c_client* client;
    struct v4l2_subdev subdev;
    struct v4l2_ctrl_handler hdl;
    u32 cfmt_code;
    const struct sc2337p_win_size* win;

    int gpio_reset;
    int gpio_pwdn;
#ifdef CONFIG_MACH_KM01A
    int gpio_fsync;
    int gpio_power;
#endif
    struct sensor_cb_info cb_info;
    struct sensor_fps_info fps_info;

    struct host_callbacks hcb;
    struct sc2337p_aec_parms aec_parms;

    AK_ISP_SENSOR_INIT_PARA para;
//#ifdef CONFIG_MACH_KM01A
    unsigned int flip_mirror;
    unsigned int init_flag;

    unsigned int fast_fps_flag;
    unsigned int first_fast_fps_flag;
//#endif

    enum sensor_master_or_slave_mode master_or_slave;
    struct dual_sensor_attr dual;

    int mode_freeze;
    int reg_slave_mode;
    struct v4l2_ctrl* ctrl_sensor_get_id;
#ifdef CONFIG_SYS_FAST_LAUNCH
    struct ae_fast_struct ae_fast;
    ae_awb_dynamic_map_policy_t ae_awb_dynamic_map_policy;
    struct  ak_sensor_param sensor_param;
    unsigned int initial_frame; //用于约束黑夜场景首帧 exp小于 rbrows，否则会出现图像异常的问题
#endif
};
/*default aec of fast boot*/
static struct ae_fast_struct ae_fast_default = { .sensor_exp_time = 2152,
    .sensor_a_gain = 256,
    .sensor_d_gain = 256,
    .isp_d_gain = 256,
    { .r_gain = 445,
        .g_gain = 256,
        .b_gain = 396,
        .r_offset = 0,
        .g_offset = 0,
        .b_offset = 0 } };

#define PIN_STATE_NUM 8
struct pin_state {
    int pin;
    int using_count;
};

/*这个结构体为了多目中reset和pwdn复用时只能操作一次*/
struct global_params {
    struct pin_state reset_s[PIN_STATE_NUM];
    struct pin_state pwdn_s[PIN_STATE_NUM];
    int reset_num;
    int pwdn_num;
};

static LIST_HEAD(privs_list);

static AK_ISP_SENSOR_CB sc2337p_cb;

/*
 * check_id - check hardware sensor ID whether it meets this driver
 * 0-no check, force to meet
 * others-check, if not meet return fail
 */
static int check_id = 0;
static int dvp = 0;

/*
 * addr0 - The addree of CIS which is connect with CSI0
 * addr1 - The addree of CIS which is connect with CSI1
 * sensor0_switch -CSI0 connects multiple CIS through MIPI_Switch chip
 * sensor1_switch -CSI1 connects multiple CIS through MIPI_Switch chip
 * for example:
 * Dual CSI
 * dual CSI  :   addr0=0xxx addr1=0xxx;
 * dual CSI + Switch:   addr0=0xxx addr1=0xxx  sensor0_switch=0;
 * dual CSI + dual Switch :    addr0=0xxx   addr1=0xxx  sensor0_switch=1
 * sensor1_switch=1; Single CSI CSI +  Switch :  sensor0_switch=1; or
 * sensor1_switch=1 CSI +  dual Switch :  addr0=0xxx   sensor1_switch=1; or
 * addr1=0xxx sensor1_switch=1;
 */
static int addr0 = 0, addr1 = 0;
static int sensor0_switch = 0;
static int sensor1_switch = 0;
#define SENSOR_I2C_ADDR0 (0x3f)
#define SENSOR_I2C_ADDR1 (0x37)

module_param(check_id, int, 0644);
module_param(SENSOR_I2C_ADDR, int, 0644);
module_param(dvp, int, 0644);
module_param(addr0, int, 0644);
module_param(addr1, int, 0644);
module_param(sensor0_switch, int, 0644);
module_param(sensor1_switch, int, 0644);
module_param(MAX_FPS, int, 0644);

static struct global_params g_params = {
    .reset_num    = 0,
    .pwdn_num     = 0,
};

/*
*  @sensor0_master:  When loading ko add 'sensor0_master=1' parameter,this mean that the sensor0
*                    will be configure to master mode.
*  @sensor1_master:  When loading ko add 'sensor1_master=1' parameter,this mean that the sensor1
*                    will be configure to master mode.
*  Note:1.if dual csi use two I2C channels, the definition of sensor0/1 is hardware-dependent define as,which.
*       The sensor uses a low serial number I2C，defined as sensor0.
*       2.if dual csi use same route I2C channels，you can usr the parameter 'addr0' & 'addr1'
*         and dts to defined sensor0/1.
*/
static int sensor0_master = -1;
static int sensor1_master = -1;
module_param(sensor0_master, int, 0644);
module_param(sensor1_master, int, 0644);

/* allow i2c bus transfer in interrupt context
 *  for 300L/100N + none-MIPI Switch
 */
static int i2c_transfer_allow_in_irq = 0;
module_param(i2c_transfer_allow_in_irq, int, 0644);
static int sensor_power_on = 0;
module_param(sensor_power_on, int, 0644);
static void sc2337p_set_fps_async(struct sc2337p_priv* priv);
static int sc2337p_sen_read_id_func(void* arg); // no use IIC bus
static int sc2337p_sen_get_resolution_func(
    void* arg, int* width, int* height);
static int sc2337p_sen_set_fps_func(void* arg, const int fps);
static int sc2337p_fps_to_vts(struct sc2337p_priv* priv, const int fps);
static int call_sensor_sys_init(void);
static int call_sensor_sys_deinit(void);
static int sc2337p_sen_probe_id_func(void* arg); // use IIC bus
static int sc2337p_write(struct i2c_client* client, int reg, int value);
static int sc2337p_sen_updata_exp_time_func(void* arg, unsigned int exp_time);
static int sc2337p_sen_update_a_gain_func(
    void* arg, const unsigned int a_gain);
static void ak_sensor_parms_init_f(struct sc2337p_priv* priv,
                                  AK_ISP_SENSOR_REG_INFO* preg_info, int para_num);

static u32 sc2337p_codes[] = {
    MEDIA_BUS_FMT_YUYV8_2X8,
    MEDIA_BUS_FMT_UYVY8_2X8,
    MEDIA_BUS_FMT_RGB565_2X8_BE,
    MEDIA_BUS_FMT_RGB565_2X8_LE,
};

struct _sensor_bining_info
{
    unsigned int width;
    unsigned int height;
};


/*
 * General functions
 */
static struct sc2337p_priv* to_sc2337p(const struct i2c_client* client)
{
    return container_of(i2c_get_clientdata(client), struct sc2337p_priv, subdev);
}

static struct v4l2_subdev* ctrl_to_sd(struct v4l2_ctrl* ctrl)
{
    return &container_of(ctrl->handler, struct sc2337p_priv, hdl)->subdev;
}

/*
 * XXXX_s_stream -
 * set steaming enable/disable
 * soc_camera_ops functions
 *
 * @sd:             subdev
 * @enable:         enable flags
 */
static int sc2337p_s_stream(struct v4l2_subdev* sd, int enable) { return 0; }

static int sc2337p_g_volatile_ctrl(struct v4l2_ctrl* ctrl)
{
    struct v4l2_subdev* sd = ctrl_to_sd(ctrl);
    int ret = 0;

    switch (ctrl->id) {
        case SENSOR_GET_ID:
            ctrl->val = SENSOR_ID;
            break;

        default:
            ret = -EINVAL;
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
static int sc2337p_s_ctrl(struct v4l2_ctrl* ctrl) { return 0; }

/*
 * XXXX_s_power -
 * set power operation
 * soc_camera_ops functions
 *
 * @sd:         subdev
 * @on:         power flags
 */
static int sc2337p_core_s_power(struct v4l2_subdev* sd, int on) { return 0; }

/*
 * XXXX_get_sensor_id -
 * get sensor ID
 * private callback functions
 *
 * @ctrl:           pointer to ctrl
 */
static int sc2337p_get_sensor_id(struct v4l2_control* ctrl)
{

    ctrl->value = sc2337p_sen_read_id_func(NULL); // no use IIC bus
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
static int sc2337p_get_sensor_cb(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct sc2337p_priv* priv = to_sc2337p(client);

    ctrl->value = (int)&priv->cb_info;
    return 0;
}

static int sc2337p_get_max_exp_for_fps(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct sc2337p_priv* priv = to_sc2337p(client);
    int fps = ctrl->value;
    int vts = sc2337p_fps_to_vts(priv, fps);

    ctrl->value = vts - 6;
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
static int sc2337p_core_g_ctrl(struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    int ret;

    switch (ctrl->id) {
        case GET_SENSOR_ID:
            ret = sc2337p_get_sensor_id(ctrl);
            break;

        case GET_SENSOR_CB:
            ret = sc2337p_get_sensor_cb(sd, ctrl);
            break;

        case GET_MAX_EXP_FOR_FPS:
            ret = sc2337p_get_max_exp_for_fps(sd, ctrl);
            break;

        default:
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
static int sc2337p_set_isp_timing_cb(
    struct v4l2_subdev* sd,struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct sc2337p_priv* priv = to_sc2337p(client);
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
static int sc2337p_set_fps_direct(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct sc2337p_priv* priv = to_sc2337p(client);
    int fps = ctrl->value;

    sc2337p_sen_set_fps_func(priv, fps);
    sc2337p_set_fps_async(priv);
    return 0;
}

static int sc2337p_set_master_or_slave(struct v4l2_subdev* sd, int is_master)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct sc2337p_priv* priv = to_sc2337p(client);

    pr_debug("%s is_master:%d\n", __func__, is_master);

    if (is_master)
        priv->master_or_slave = MASTER_MODE;
    else
        priv->master_or_slave = SLAVER_MODE;

    return 0;
}

static int sc2337p_set_flip_mirror(struct v4l2_subdev* sd, int enable, int force)
{
    struct i2c_client* client   = v4l2_get_subdevdata(sd);
    struct sc2337p_priv* priv   = to_sc2337p(client);
    int value = 0;
    int flip_en = enable & (0x1 << FLIP_OFFSET);
    int mirror_en = enable & (0x1 << MIRROR_OFFSET);

#if defined(CONFIG_SYS_FAST_LAUNCH)
    if (!force) {
        if (priv->sensor_param.flip_mirror == enable) {
            return 0;
        }
    }
#endif
    if (flip_en)
        value |= 0x3 << 5;
    else
        value &= ~(0x3 << 5);

    if (mirror_en)
        value |= 0x3 << 1;
    else
        value &= ~(0x3 << 1);

    sc2337p_write(client, FLIP_MIRROR, value);

#if defined(CONFIG_SYS_FAST_LAUNCH)
    priv->sensor_param.flip_mirror = enable;
#endif

    return 0;
}

#ifdef CONFIG_MACH_KM01A
static int ak_sensor_set_nomal_mode(struct v4l2_subdev *sd, int value)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct sc2337p_priv *priv = to_sc2337p(client);
    priv->fast_fps_flag = value;
    //priv->power_on_flag = 0;
    pr_info("%s fast_fps_flag:%d\n",__func__,priv->fast_fps_flag);

    return 0;
}
static int sc2337p_set_init_fps(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct sc2337p_priv* priv = to_sc2337p(client);
    int fps = ctrl->value;
    pr_err("%s, fps = %d\n", __func__,fps);
    sc2337p_sen_set_fps_func(priv, fps);
    return 0;
}
#endif

#if defined(CONFIG_SYS_FAST_LAUNCH)
static int sc2337p_set_init_fps(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct sc2337p_priv* priv = to_sc2337p(client);
    int fps = ctrl->value;
    pr_err("%s, fps = %d\n", __func__,fps);
    priv->fps_info.to_fps = fps;
    //sc2337p_sen_set_fps_func(priv, fps);
    return 0;
}
#endif

/*
 * XXXX_core_s_ctrl -
 * set core s_ctrl
 * core functions
 *
 * @sd:             subdev
 * @ctrl:           pointer to ctrl
 */
static int sc2337p_core_s_ctrl(struct v4l2_subdev* sd, struct v4l2_control* ctrl)
//static long sc2337p_core_command(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
    int ret;

    switch (ctrl->id) {


        case SET_ISP_MISC_CALLBACK:
            ret = sc2337p_set_isp_timing_cb(sd, ctrl);
            break;

        case SET_FPS_DIRECT:
            ret = sc2337p_set_fps_direct(sd, ctrl);
            break;

        case SET_FLIP_MIRROR:
            ret = sc2337p_set_flip_mirror(sd, ctrl->value, 0);
            break;
#ifdef CONFIG_SYS_FAST_LAUNCH
        case SET_INIT_FPS:
            ret = sc2337p_set_init_fps(sd, ctrl);
            break;
#endif
#ifdef CONFIG_MACH_KM01A
        case SET_NORMAL_MODE:
            ret = ak_sensor_set_nomal_mode(sd, ctrl->value);
            break;
#endif
        case SET_MASTER: /*for dual sensor*/
            ret = sc2337p_set_master_or_slave(sd, 1);
            break;

        case SET_SLAVER: /*for dual sensor*/
            ret = sc2337p_set_master_or_slave(sd, 0);
            break;
        default:
            pr_err("%s cmd:%d not support\n", __func__, ctrl->id);
            ret = -1;
            break;
    }

    return ret;
}

static void ak_sensor_parms_init_f(struct sc2337p_priv* priv,
                                  AK_ISP_SENSOR_REG_INFO* preg_info, int para_num)
{
    int vts_h_tmp = 0x4, vts_l_tmp = 0x65;
    int rb_row_l = 0x04, rb_row_h = 0x00;
    int i = 0;
    priv->reg_slave_mode = 0;

    pr_info("priv->fast_fps_flag:%d\r\n", priv->fast_fps_flag);
    pr_info("para_num:%d, MAX_FPS:%d\r\n", para_num, MAX_FPS);

    for (i = 0; i < para_num; i++) {
        if (preg_info->reg_addr == VTS_REG_H)
            vts_h_tmp = preg_info->value;
        else if (preg_info->reg_addr == VTS_REG_L)
            vts_l_tmp = preg_info->value;
        else if (preg_info->reg_addr == RB_ROW_H)
            rb_row_h = preg_info->value;
        else if (preg_info->reg_addr == RB_ROW_L)
            rb_row_l = preg_info->value;
        else if (preg_info->reg_addr == 0x3222)
            priv->reg_slave_mode = preg_info->value;
        preg_info++;
    }

    priv->aec_parms.calc_vts_tmp = (vts_h_tmp << 8 | vts_l_tmp) * MAX_FPS * 100;
    priv->aec_parms.pclk_freq = HTS_VALUE * (vts_h_tmp << 8 | vts_l_tmp) * MAX_FPS;

#ifdef CONFIG_SYS_FAST_LAUNCH
    //解决首帧采集不到问题:
    /*
     *
     * 目前思特威安防这边的sensor，跑slave模式，首帧基本都有这样的限制。
     * 关于第一帧sc2337p 从模式，曝光时间要小于rbrows，首帧才是正常的，曝光大于rbrow会有图像问题 或者第一帧采集不到。
     * 大窗出完第一帧后可以调到更大的曝光时间了。
     *
     * */
    /********** 1.初始化 priv->to_fps、to_fps_value 等参数 **************/
    if (priv->fps_info.to_fps > 0)
    {
        sc2337p_sen_set_fps_func(priv, priv->fps_info.to_fps);
    }
    priv->fps_info.current_fps = priv->fps_info.to_fps;//TODO

    /*********** 2.从模式 rbrow = 双目帧率的vts*2，还需要确保exp小于rbrow，否则首帧图像会有问题 *******************/
    //if (priv->master_or_slave == SLAVER_MODE) { //双目start_streaming 的时候才能会设置为从模式,这里不做约束
        if (priv->fps_info.current_fps <= 15) {
            priv->fps_info.src_rb_rows = sc2337p_fps_to_vts(priv, priv->fps_info.current_fps * 2);
        } else {
            priv->fps_info.src_rb_rows = sc2337p_fps_to_vts(priv, priv->fps_info.current_fps);
        }
    //}
    //pr_info("@@@YJH %s rb_rows:%d, %d\n", __func__, priv->fps_info.src_rb_rows, priv->fps_info.current_fps);
#else
    priv->fps_info.src_rb_rows = (rb_row_h << 8) | rb_row_l;
#endif
    priv->fps_info.rb_rows = priv->fps_info.src_rb_rows;

    pr_info("%s rb_rows:%d, %x, %x, %d\n", __func__, priv->fps_info.rb_rows, rb_row_h, rb_row_l,
            para_num);
    pr_info("calc_vts_tmp %d, pclk_freq=%d ,current_fps=%d \n", priv->aec_parms.calc_vts_tmp,
            priv->aec_parms.pclk_freq,priv->fps_info.current_fps);
}

static int sc2337p_store_initial_regs(struct sc2337p_priv* priv, void* arg)
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

    kfree(sensor_para->reg_info);
    return 0;

alloc2_fail:
cp2_fail:
    kfree(sensor_para->reg_info);
alloc1_fail:
    return ret;
}



static int sc2337p_get_slave_init_max_exp_for_fps(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct sc2337p_priv* priv = to_sc2337p(client);
    int fps = ctrl->value;
    AK_ISP_SENSOR_INIT_PARA* para = &priv->para;
    AK_ISP_SENSOR_REG_INFO* preg_info;
    if (para) {
        preg_info = para->reg_info;
        ak_sensor_parms_init_f(priv, preg_info, para->num);
        ctrl->value = priv->fps_info.rb_rows-1;//思特微cis，slave模式，首帧最大的exp必须小于rbrows
    } else if (NULL == para) {
        pr_err("%s get cis para error\n",__func__);
        ctrl->value=0;
        return -1;
    }
    return 0;
}


static long sc2337p_core_ioctl(
    struct v4l2_subdev* sd, unsigned int cmd, void* arg)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct sc2337p_priv* priv = to_sc2337p(client);
    int ret = 0;

    pr_debug("%s\n", __func__);

    switch (cmd) {
        case AK_SENSOR_SET_INIT:
            sc2337p_store_initial_regs(priv, arg);
            break;

        case AK_SENSOR_GET_MAX_EXP_FOR_FPS: {
            struct sensor_max_exp_for_fps* exp_for_fps = arg;
            struct v4l2_control ctrl;

            ctrl.value = exp_for_fps->fps;
            ret = sc2337p_get_max_exp_for_fps(sd, &ctrl);
            if (!ret)
                exp_for_fps->max_exp = ctrl.value;
            break;
        }
        //思特威CIS,约束slave模式首帧exp，必须满足曝光时间要小于rbrows.
        case AK_SENSOR_GET_SLAVE_MAX_EXP_FOR_FPS: {
            struct sensor_slave_max_exp_for_fps* slave_exp_for_fps = arg;
            struct v4l2_control ctrl;

            ctrl.value = slave_exp_for_fps->fps;
            ret = sc2337p_get_slave_init_max_exp_for_fps(sd,&ctrl);
            if (!ret)
                slave_exp_for_fps->max_exp = ctrl.value;
            break;
        }
#ifdef CONFIG_SYS_FAST_LAUNCH
        case AK_SENSOR_GET_AE_AWB_CONVERT_COEF: {
            ae_awb_dynamic_map_policy_t* ae_awb_dynamic_map_policy = arg;
            struct i2c_client* client = v4l2_get_subdevdata(sd);
            struct sc2337p_priv* priv = to_sc2337p(client);

            ae_awb_dynamic_map_policy->ae_convert_group_num = SC2337P_AE_CONVERT_GROUP_NUM;
            if(SC2337P_AE_CONVERT_GROUP_NUM != 0) {
                ae_awb_dynamic_map_policy->dynamic_map_enable |= 1;

                memcpy(&priv->ae_awb_dynamic_map_policy.ae_convert_coef_group,
                        &sc2337p_ae_convert_coef_group,
                        sizeof(ae_convert_coef_t) * ae_awb_dynamic_map_policy->ae_convert_group_num);
                memcpy(&ae_awb_dynamic_map_policy->ae_convert_coef_group,
                        &priv->ae_awb_dynamic_map_policy.ae_convert_coef_group,
                        sizeof(ae_convert_coef_t) * ae_awb_dynamic_map_policy->ae_convert_group_num);
            }

            if(SC2337P_AWB_CONVERT_GROUP_NUM != 0) {
                ae_awb_dynamic_map_policy->dynamic_map_enable |= 1<<1;
                memcpy(&priv->ae_awb_dynamic_map_policy.awb_convert_coef,
                        &sc2337p_awb_convert_coef,
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
static int sc2337p_get_fmt(struct v4l2_subdev* sd,
    struct v4l2_subdev_pad_config* cfg, struct v4l2_subdev_format* format)
{
    struct v4l2_mbus_framefmt* mf = &format->format;
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct sc2337p_priv* priv = to_sc2337p(client);
    static struct sc2337p_win_size win = {
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
static int sc2337p_set_fmt(struct v4l2_subdev* sd,
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
static int sc2337p_get_selection(struct v4l2_subdev* sd,
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
static int sc2337p_enum_mbus_code(struct v4l2_subdev* sd,
    struct v4l2_subdev_pad_config* cfg, struct v4l2_subdev_mbus_code_enum* code)
{
    if (code->pad || code->index >= ARRAY_SIZE(sc2337p_codes))
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
static int sc2337p_g_crop(struct v4l2_subdev* sd, struct v4l2_crop* a)
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
static int sc2337p_cropcap(struct v4l2_subdev* sd, struct v4l2_cropcap* a)
{
    sc2337p_sen_get_resolution_func(
        NULL, &a->bounds.width, &a->bounds.height);
    a->bounds.left = SENSOR_VALID_OFFSET_X;
    a->bounds.top = SENSOR_VALID_OFFSET_Y;
    a->defrect = a->bounds;
    a->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    a->pixelaspect.numerator = 1;
    a->pixelaspect.denominator = 1;

    pr_err("%s bounds.width:%d, bounds.height:%d\n", __func__, a->bounds.width,
        a->bounds.height);

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
static int sc2337p_video_probe(struct i2c_client* client)
{
    struct sc2337p_priv* priv = to_sc2337p(client);
    int ret;

    ret = v4l2_ctrl_handler_setup(&priv->hdl);

    return ret;
}

/*ctrl ops*/
static const struct v4l2_ctrl_ops sc2337p_ctrl_ops = {
    .g_volatile_ctrl = sc2337p_g_volatile_ctrl,
    .s_ctrl = sc2337p_s_ctrl,
};

/*core ops*/
static struct v4l2_subdev_core_ops sc2337p_subdev_core_ops = {
    .s_power = sc2337p_core_s_power,
    .g_ctrl = sc2337p_core_g_ctrl,
    .s_ctrl = sc2337p_core_s_ctrl,
    .ioctl = sc2337p_core_ioctl,
};

/*
 * XXXX_g_mbus_config -
 * get buf config
 * video functions
 *
 * @sd:             subdev
 * @cfg:            return config
 */
static int sc2337p_g_mbus_config(
    struct v4l2_subdev* sd, struct v4l2_mbus_config* cfg)
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
static int sc2337p_s_crop(struct v4l2_subdev* sd, const struct v4l2_crop* crop)
{
    printk(KERN_ERR "%s %d, left:%d, top:%d, width:%d, height:%d\n", __func__,
        __LINE__, crop->c.left, crop->c.top, crop->c.width, crop->c.height);
    return 0;
}

/*video ops*/
static struct v4l2_subdev_video_ops sc2337p_subdev_video_ops = {
    .s_stream = sc2337p_s_stream,
    .cropcap = sc2337p_cropcap,
    .g_crop = sc2337p_g_crop,
    .g_mbus_config = sc2337p_g_mbus_config,
    .s_crop = sc2337p_s_crop,

};

/*pad ops*/
static const struct v4l2_subdev_pad_ops sc2337p_subdev_pad_ops = {
    .enum_mbus_code = sc2337p_enum_mbus_code,
    .get_fmt = sc2337p_get_fmt,
    .set_fmt = sc2337p_set_fmt,
    .get_selection = sc2337p_get_selection,
};

/*sensor driver subdev ops*/
static struct v4l2_subdev_ops sc2337p_subdev_ops = {
    .core = &sc2337p_subdev_core_ops,
    .video = &sc2337p_subdev_video_ops,
    .pad = &sc2337p_subdev_pad_ops,
};

/*
 * sensor_of_parse -
 * parse node of device
 *
 * @client:         pointor to i2c client
 * @priv:           sensor struct
 */
static int sensor_of_parse(struct i2c_client* client, struct sc2337p_priv* priv)
{
    struct device_node* np = client->dev.of_node;

    /*it is exist reset but lack of pwdn gpio in most case*/
    priv->gpio_reset = of_get_named_gpio(np, "reset-gpio", 0);
    priv->gpio_pwdn = of_get_named_gpio(np, "pwdn-gpio", 0);
#ifdef CONFIG_MACH_KM01A
    priv->gpio_fsync = of_get_named_gpio(np, "fsync-gpio", 0);
    priv->gpio_power = of_get_named_gpio(np, "power-gpio", 0);
#endif
    if (priv->gpio_reset >= 0) {
        devm_gpio_request(&client->dev, priv->gpio_reset, "sensor-reset");
    }

    if (priv->gpio_pwdn >= 0) {
        devm_gpio_request(&client->dev, priv->gpio_pwdn, "sensor-pwdn");
    }
#ifdef CONFIG_MACH_KM01A
    if (priv->gpio_power >= 0) {
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
static int sc2337p_match(struct sc2337p_priv* priv)
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
    struct sc2337p_priv* t_priv = NULL;
    struct i2c_client* client;

    list_for_each_entry(t_priv, &privs_list, list)
    {
        if (!t_priv || !t_priv->subdev.devnode)
            continue;

        client = t_priv->client;
        switch (t_priv->subdev.devnode->num) {
            case 0:
                if (addr0 > 0) {
                    pr_err("%s %d new_addr0:0x%x\n", __func__, __LINE__, addr0);
                    client->addr = addr0;
                }
                break;

            case 1:
                if (addr1 > 0) {
                    pr_err("%s %d new_addr1:0x%x\n", __func__, __LINE__, addr1);
                    client->addr = addr1;
                }
                break;

            default:
                break;
        }
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
static int sc2337p_probe(
    struct i2c_client* client, const struct i2c_device_id* did)
{
    struct sc2337p_priv* priv;
    struct i2c_adapter* adapter = to_i2c_adapter(client->dev.parent);
    int ret;

    pr_err("%s %s %s\n", __func__, __DATE__, __TIME__);

    if (dvp)
        SENSOR_IO_INTERFACE = DVP_INTERFACE;
    else
        SENSOR_IO_INTERFACE = MIPI_INTERFACE;

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_err(&adapter->dev, "sc2337p: I2C-Adapter doesn't support SMBUS\n");
        return -EIO;
    }

    priv = devm_kzalloc(&client->dev, sizeof(struct sc2337p_priv), GFP_KERNEL);
    if (!priv) {
        dev_err(&adapter->dev, "Failed to allocate memory for private data!\n");
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
#endif
    /*
     * the current sensor slave address to client.
     * the address from dts may incorrect
     * */
    client->addr = SENSOR_I2C_ADDR;
    priv->cb_info.cb = &sc2337p_cb;
    priv->cb_info.arg = priv;
    priv->client = client;
#ifdef CONFIG_MACH_KM01A
    memcpy(&priv->ae_fast, &ae_fast_default, sizeof(struct ae_fast_struct));
#endif
    if (!client->dev.of_node) {
        dev_err(&client->dev, "Missing platform_data for driver\n");
        ret = -EINVAL;
        goto err_clk;
    }

    if (!sc2337p_match(priv)) {
        ret = -ENODEV;
        goto err_clk;
    }

    /*subdev init*/
    v4l2_i2c_subdev_init(&priv->subdev, client, &sc2337p_subdev_ops);
    priv->subdev.flags
        |= /*V4L2_SUBDEV_FL_HAS_EVENTS | */ V4L2_SUBDEV_FL_HAS_DEVNODE;
    v4l2_ctrl_handler_init(&priv->hdl, 2);
    v4l2_ctrl_new_std(&priv->hdl, &sc2337p_ctrl_ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
    v4l2_ctrl_new_std(&priv->hdl, &sc2337p_ctrl_ops, V4L2_CID_HFLIP, 0, 1, 1, 0);

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

    ret = sc2337p_video_probe(client);
    if (ret < 0) {
        dev_err(&client->dev, "sc2337p probe fail\n");
        goto err_videoprobe;
    }

    ret = v4l2_async_register_subdev(&priv->subdev);
    if (ret < 0) {
        dev_err(&client->dev, "v4l2 async register subdev fail, ret:%d\n", ret);
        goto err_videoprobe;
    }

    INIT_LIST_HEAD(&priv->list);
    list_add_tail(&priv->list, &privs_list);

    client_new_addr();

    priv->master_or_slave =  SINGLE_MODE;

    dev_err(&adapter->dev, "sc2337p Probed success, subdev:%p\n", &priv->subdev);

    /*ak platform need export some informations from sensor*/
    call_sensor_sys_init();

#ifdef CONFIG_MACH_KM01A
    /*reset pwdn引脚管理*/
    if (priv->gpio_reset >= 0) {
        if (g_params.reset_num < PIN_STATE_NUM) {
            int i;
            for (i = 0; i < g_params.reset_num; i++) {
                if (g_params.reset_s[i].pin == priv->gpio_reset)
                    break;
            }
            if (i == g_params.reset_num) {
                g_params.reset_s[g_params.reset_num].pin = priv->gpio_reset;
                g_params.reset_s[g_params.reset_num].using_count = 0;
                g_params.reset_num++;
            }
        }
    }

    if (priv->gpio_pwdn >= 0) {
        if (g_params.pwdn_num < PIN_STATE_NUM) {
            int i;
            for (i = 0; i < g_params.pwdn_num; i++) {
                if (g_params.pwdn_s[i].pin == priv->gpio_pwdn)
                    break;
            }
            if (i == g_params.pwdn_num) {
                g_params.pwdn_s[g_params.pwdn_num].pin = priv->gpio_pwdn;
                g_params.pwdn_s[g_params.pwdn_num].using_count = 0;
                g_params.pwdn_num++;
            }
        }
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
static int sc2337p_remove(struct i2c_client* client)
{
    struct sc2337p_priv* priv = to_sc2337p(client);

    pr_err("%s %d\n", __func__, __LINE__);

    if (!priv) {
        pr_err("%s had remove\n", __func__);
        return 0;
    }

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

static const struct i2c_device_id sc2337p_id[] = { { "sc2337p", 0 }, {} };
MODULE_DEVICE_TABLE(i2c, sc2337p_id);

static const struct of_device_id sc2337p_of_match[] = {
    /*donot changed compatible, must the same as dts*/
    {
        .compatible = "anyka,sensor0",
    },
    {},
};
MODULE_DEVICE_TABLE(of, sc2337p_of_match);

static struct i2c_driver sc2337p_i2c_driver = {
    .driver = {
        .name = "sc2337p",
        .of_match_table = of_match_ptr(sc2337p_of_match),
    },
    .probe    = sc2337p_probe,
    .remove   = sc2337p_remove,
    .id_table = sc2337p_id,
};

module_i2c_driver(sc2337p_i2c_driver);

MODULE_DESCRIPTION("SoC Camera driver for sc2337p sensor");
MODULE_AUTHOR("Anyka Microelectronic");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0.04");

/*sensor i2c write*/
static int __sc2337p_write(const struct i2c_client* client, int reg, int value)
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

static int _sc2337p_write(
    struct i2c_client* client, int reg, int value, int sensor_switch)
{
    struct i2c_client* client_p = client;
    unsigned short tmp_addr = client_p->addr;
    int ret = 0;

    if (sensor_switch == 1) {
        client_p->addr = SENSOR_I2C_ADDR0;
        __sc2337p_write(client_p, reg, value);
        client_p->addr = SENSOR_I2C_ADDR1;
        ret = __sc2337p_write(client_p, reg, value);
        client_p->addr = tmp_addr;
        return ret;
    } else {
        return __sc2337p_write(client_p, reg, value);
    }
}

static int sc2337p_write_cisx(struct sc2337p_priv* t_priv, int reg, int value)
{
    switch (t_priv->subdev.devnode->num) {
        case 0:
            return _sc2337p_write(t_priv->client, reg, value, sensor0_switch);
            break;

        case 1:
            return _sc2337p_write(t_priv->client, reg, value, sensor1_switch);
            break;

        default:
            break;
    }
    return 0;
}

static int sc2337p_write(struct i2c_client* client, int reg, int value)
{
    struct sc2337p_priv* t_priv = NULL;
    struct i2c_client* client_p;

    list_for_each_entry(t_priv, &privs_list, list)
    {
        if (!t_priv || !t_priv->subdev.devnode)
            continue;

        if (t_priv->master_or_slave == SLAVER_MODE) {
            // Dual CSI
            if (client == t_priv->client) {
                return sc2337p_write_cisx(t_priv, reg, value);
            }
        } else {
            // single CSI
            // when use Two_way_I2C + MIPI switch need write register twice.
            sc2337p_write_cisx(t_priv, reg, value);
        }
    }
    return 0;
}

/*sensor i2c read*/
static int sc2337p_read(const struct i2c_client* client, int reg)
{
    struct i2c_transfer_struct trans = {
        .client = client,
        .reg_bytes = SENSOR_REGADDR_BYTE,
        .value_bytes = SENSOR_DATA_BYTE,
        .reg = reg,
        .value = 0,
    };

    pr_debug("%s client:%p, addr:%x\n", __func__, client, client->addr);
    return i2c_read(&trans);
}

#if 0
static void sc2337p_dual_master_vts(struct sc2337p_priv *priv, int cur_vts)
{
    struct i2c_client *client = priv->client;
    int value;

    if (priv->master_or_slave != MASTER_MODE)
        return;

    /*
     * sc2337p主从模式时，主的帧率要稍微大于从的帧率。
     * 调试发现0x42原为0x65调整到0x6b就可以稳定同步，
     * 因此我们认为当前初始化序列时主比次大0x6b-0x65=6即可。
     */

    if (cur_vts <= 0) {
        value = sc2337p_read(client, 0x41) << 8;
        value |= sc2337p_read(client, 0x42) & 0xff;
    } else {
        value = cur_vts;
    }
    value += 6;
    sc2337p_write(client, 0x41, value >> 8);
    sc2337p_write(client, 0x42, value & 0xff);
}

static void sc2337p_dual_init_func(struct sc2337p_priv *priv,
        AK_ISP_SENSOR_INIT_PARA *para)
{
    /*
     * sc2337p双目目前使用单次触发单次输出，但为了兼容单次触发连续输出，
     * 我们要求先初始化slaver后初始化master。
     */

    struct i2c_client *client = priv->client;
    AK_ISP_SENSOR_REG_INFO *preg_info;
    int value;
    int i;

    priv->dual.init_count++;

    if (priv->master_or_slave == SLAVER_MODE) {
        pr_debug("%s slaver init_count:%d\n",__func__,priv->dual.init_count);

        if (priv->dual.init_count == 1) {
            preg_info = para->reg_info;
            for (i = 0; i < para->num; i++) {
                sc2337p_write(client, preg_info->reg_addr, preg_info->value);

                preg_info++;
            }

            /*slaver*/
            sc2337p_write(client,0xfe,0x00);
            sc2337p_write(client,0x7f,0x09);
            sc2337p_write(client,0x82,0x0a);
            sc2337p_write(client,0x83,0x0b);
            sc2337p_write(client,0x84,0x80);
            sc2337p_write(client,0x85,0x51);
        }
    } else {//MASTER_MODE
        /*
         * when
         * priv->dual.init_count == 1
         * do nothing.
         *
         * wait slaver ready first then master
         */
        pr_debug("%s master init_count:%d\n",__func__,priv->dual.init_count);
        if (priv->dual.init_count == 2) {
            preg_info = para->reg_info;
            for (i = 0; i < para->num; i++) {
                sc2337p_write(client, preg_info->reg_addr, preg_info->value);

                preg_info++;
            }

            /*master*/
            sc2337p_write(client,0xfe,0x00);
            sc2337p_write(client,0x7f,0x09);
            sc2337p_write(client,0x82,0x01);
            sc2337p_write(client,0x83,0x0c);
            sc2337p_write(client,0x84,0x80);

            sc2337p_dual_master_vts(priv, 0);
        }
    }
}
#endif

static void sc2337p_agc_mode(struct sc2337p_priv* priv)
{
    int value;
    struct i2c_client* client = priv->client;

    value = sc2337p_read(client, AGC_CTRL) & 0xf0;
    value |= 0xb; //AGC_CTRL[3:0] set to 0xb
    sc2337p_write(client, AGC_CTRL, value);
}

/*
 * The sensor must set callbacks
 *
 * */
static int sc2337p_sen_init_func(
    void* arg, const AK_ISP_SENSOR_INIT_PARA* npara)
{
    int i;
    AK_ISP_SENSOR_REG_INFO* preg_info;
    struct sc2337p_priv* priv = arg;
    struct i2c_client* client = priv->client;
    AK_ISP_SENSOR_INIT_PARA* para = &priv->para;

#ifdef CONFIG_SYS_FAST_LAUNCH
    unsigned int exp = (unsigned int)(priv->ae_fast.sensor_exp_time);
#endif
#ifdef CONFIG_MACH_KM01A
    int fast_fps = 0;
    int para_num = 0;

    unsigned int exp = (unsigned int)(priv->ae_fast.sensor_exp_time);
    priv->init_flag = 0;

    sc2337p_sen_probe_id_func(arg); // use IIC bus
    pr_info("%s sensor_mode %s\n", __func__,
        (priv->master_or_slave == MASTER_MODE) ? "MASTER" : "SLAVER");
    if (priv->master_or_slave == MASTER_MODE) {
        preg_info = (void*)sc2337p_1920x1080_30fps;
        para->num = sizeof(sc2337p_1920x1080_30fps)
            / sizeof(sc2337p_1920x1080_30fps[0]) / 2;
    } else if (priv->master_or_slave == SLAVER_MODE) {
        pr_err("[%s]%dSLAVER mode not support\n", __func__, __LINE__);
    } else {
        if (para->num <= 0 && npara)
            para = (void*)npara;

        preg_info = para->reg_info;
    }
    para_num = para->num;
    ak_sensor_parms_init_f(priv, preg_info, para_num);

    pr_err("priv->fast_fps_flag:%d, para_num:%d\n", priv->fast_fps_flag, para_num);
#else

    if (para->num <= 0 && npara)
        para = (void*)npara;

    preg_info = para->reg_info;
    ak_sensor_parms_init_f(priv, preg_info, para->num);
#endif
#ifdef CONFIG_MACH_KM01A
    for (i = 0; i < para_num; i++) {
#else
    for (i = 0; i < para->num; i++) {
#endif
#if 0
        {
            int value;

            value = sc2337p_read(client, preg_info->reg_addr);
            pr_err("reg:%x read value:%x\n", preg_info->reg_addr, value);
        }
#endif
#ifdef CONFIG_SYS_FAST_LAUNCH
        if( (preg_info->reg_addr == 0x100) && (preg_info->value == 1)){
#ifdef DEBUG_CIS_PARAM
            pr_info("init exp-again-dgain:%d-%d-%d\r\n", exp,priv->ae_fast.sensor_a_gain,
                    priv->ae_fast.sensor_d_gain);
#endif
            if(priv->master_or_slave == SLAVER_MODE)
                priv->initial_frame = 1;
            sc2337p_write(client, RB_ROW_H, priv->fps_info.src_rb_rows >> 8);
            sc2337p_write(client, RB_ROW_L, priv->fps_info.src_rb_rows & 0xff);

            sc2337p_sen_updata_exp_time_func(priv, priv->ae_fast.sensor_exp_time);
            sc2337p_sen_update_a_gain_func(priv, priv->ae_fast.sensor_a_gain);
            sc2337p_set_flip_mirror(&priv->subdev,priv->sensor_param.flip_mirror, 1);
        }
#endif
        sc2337p_write(client, preg_info->reg_addr, preg_info->value);
#if 0
        {
            int value;

            value = sc2337p_read(client, preg_info->reg_addr);
            pr_err("reg:%x write value:%x, read back value:%x\n",
                    preg_info->reg_addr, preg_info->value, value);
        }
#endif

        preg_info++;
    }

    //mdelay(10); // 操作完0x0100后马上读，i2c会读错为0xff91，加个延时才能正常获取vts
#ifdef CONFIG_MACH_KM01A
    priv->init_flag = 1;
#endif

//    pr_err("vts reg_H:%08x reg_L:%08x param_vts:%d\n",
//        sc2337p_read(client,VTS_REG_H), sc2337p_read(client,VTS_REG_L),
//        priv->aec_parms.reg_frame_vts);
//
//    priv->aec_parms.reg_frame_vts =
//        (sc2337p_read(client,VTS_REG_H) << 8 | sc2337p_read(client,VTS_REG_L));
//     priv->aec_parms.reg_frame_hts =
//         (sc2337p_read(client,HTS_REG_H) << 8 | sc2337p_read(client,HTS_REG_L));
//     priv->aec_parms.pclk_freq = priv->aec_parms.reg_frame_hts *
//         priv->aec_parms.reg_frame_vts * MAX_FPS;
//
//    pr_err("%s hts %d vts %d pclk %d\n", __func__,
//        priv->aec_parms.reg_frame_hts, priv->aec_parms.reg_frame_vts,
//        priv->aec_parms.pclk_freq);
    if(priv->fps_info.current_fps == 0)
        priv->fps_info.current_fps = MAX_FPS;
    priv->fps_info.to_fps = priv->fps_info.current_fps;
//    priv->fps_info.to_fps_value = priv->aec_parms.reg_frame_vts;
//    priv->fps_info.reg_fps_value = priv->aec_parms.reg_frame_vts;

    sc2337p_agc_mode(priv);

    return 0;
}

/*read sensor register*/
static int sc2337p_sen_read_reg_func(void* arg, const int reg_addr)
{
    struct sc2337p_priv* priv = arg;
    struct i2c_client* client = priv->client;

    return sc2337p_read(client, reg_addr);
}
/*write sensor register*/
static int sc2337p_sen_write_reg_func(
    void* arg, const int reg_addr, int value)
{
    struct sc2337p_priv* priv = arg;
    struct i2c_client* client = priv->client;

    return sc2337p_write(client, reg_addr, value);
}
/*read sensor register, NO i2c ops*/
static int sc2337p_sen_read_id_func(void* arg) // no use IIC bus
{
    return SENSOR_ID;
}
static int aec_parms_init(struct sc2337p_priv* priv)
{
    priv->aec_parms.curr_again_level = 0;
    priv->aec_parms.curr_again_10x = -1;
    priv->aec_parms.r0x3e02_value = 0;
    priv->aec_parms.r0x3e01_value = 0;
    priv->aec_parms.r0x3e00_value = 0;
    priv->aec_parms.target_exp_ctrl = 0;
    priv->aec_parms.curr_2x_dgain = 0;
    priv->aec_parms.curr_corse_gain = -1;

    priv->aec_parms.reg_frame_vts = 0;
    //priv->aec_parms.reg_frame_hts = 0;
    //priv->aec_parms.pclk_freq = 0;
    //priv->aec_parms.calc_vts_tmp = 0;
    priv->aec_parms.r0x3e08_corse_gain = 0;
    priv->aec_parms.r0x3e09_fine_gain = 0;

    return 0;
}

/*set sensor again*/
/*max gain: 8000 (31.25 times)*/
static int sc2337p_sen_update_a_gain_func(
    void* arg, const unsigned int a_gain)
{
    struct sc2337p_priv* priv = arg;
    struct i2c_client* client = priv->client;
    int const gain = a_gain * 1000 / 256;
    int ana_gain = 0x00;
    int dig_gain = 0x00;
    int dig_fine_gain = 0x80;
    int range = 0;
    range = 0x100 - 0x80;

    if (gain < 1000) {
        ana_gain = 0x00;
        dig_gain = 0x00;
        dig_fine_gain = 0x80;
        priv->aec_parms.curr_max_again_flag = 0;
    } else if (gain < 2000) {
        ana_gain = 0x00;
        dig_gain = 0x00;
        dig_fine_gain = (gain - 1000) * range / (2000 - 1000) + 0x80;
        priv->aec_parms.curr_max_again_flag = 0;
    } else if (gain < 4000) {
        ana_gain = 0x08;
        dig_gain = 0x00;
        dig_fine_gain = (gain - 2000) * range / (4000 - 2000) + 0x80;
        priv->aec_parms.curr_max_again_flag = 0;
    } else if (gain < 8000) {
        ana_gain = 0x09;
        dig_gain = 0x00;
        dig_fine_gain = (gain - 4000) * range / (8000 - 4000) + 0x80;
        priv->aec_parms.curr_max_again_flag = 0;
    } else if (gain < 16000) {
        ana_gain = 0x0b;
        dig_gain = 0x00;
        dig_fine_gain = (gain - 8000) * range / (16000 - 8000) + 0x80;
        priv->aec_parms.curr_max_again_flag = 0;
    } else if (gain < 32000) {
        ana_gain = 0x0f;
        dig_gain = 0x00;
        dig_fine_gain = (gain - 16000) * range / (32000 - 16000) + 0x80;
        priv->aec_parms.curr_max_again_flag = 0;

    } else if (gain < 64000) {

        ana_gain = 0x1f;
        dig_gain = 0x00;
        dig_fine_gain = (gain - 32000) * range / (64000 - 32000) + 0x80;
        priv->aec_parms.curr_max_again_flag = 0;

    } else if (gain < 128000) {

        ana_gain = 0x1f;
        dig_gain = 0x01;
        dig_fine_gain = (gain - 64000) * range / (128000 - 64000) + 0x80;
        priv->aec_parms.curr_max_again_flag = 0;

    } else {
        ana_gain = 0x1f;
        dig_gain = 0x03;
        dig_fine_gain = 0x80;
        priv->aec_parms.curr_max_again_flag = 1;
    }

    dig_fine_gain = dig_fine_gain & 0xfc;
    sc2337p_write(client, AGAIN_REG, ana_gain);
    sc2337p_write(client, DGAIN_REG, dig_gain);
    ////2337p的again没有fine gain，需要用DIG FINE GAIN来协助过渡
    sc2337p_write(client, FINE_DGAIN_REG, dig_fine_gain);
#ifdef DEBUG_CIS_PARAM
    pr_info("%s a_gain=%d,ana_gain=%x,dig_gain=%x,dig_fine_gain=%x\n",__func__,a_gain,ana_gain,
            dig_gain,dig_fine_gain);
    pr_info("%s 0x3e09=%x,0x3e06=%x,0x3e07=%x\n",__func__,sc2337p_read(client, 0x3e09),
            sc2337p_read(client, 0x3e06),sc2337p_read(client, 0x3e07));
#endif

    return A_GAIN_EFFECT_FRAMES;
}

/*set sensor dgain*/
/*max gain: 4000 (15.625 times)*/
static int sc2337p_sen_update_d_gain_func(
    void* arg, const unsigned int d_gain)
{
    struct sc2337p_priv* priv = arg;
    struct i2c_client* client = priv->client;
    unsigned int dgain = d_gain * 1000 / 256;       //*4
    int dig_gain = 0x00;
    int dig_fine_gain = 0x80;
    int range = 0;
    range = 0x100 - 0x80;

    //again must be set to maximum before adjusting dgain
    if (!(priv->aec_parms.curr_max_again_flag))
        goto end;
    //gain = 256 * GAIN Value
    //dgain = 1000 * GAIN Value
    if (dgain < 1000) {             //dgain<1x
        dig_gain = 0x00;
        dig_fine_gain = 0x80;
    } else if (dgain < 2000) {      //dgain<2x
        dig_gain = 0x00;
        dig_fine_gain = (dgain - 1000) * range / (2000 - 1000) + 0x80;
    } else if (dgain < 4000) {      //dgain<4x
        dig_gain = 0x01;
        dig_fine_gain = (dgain - 2000) * range / (4000 - 2000) + 0x80;
    } else {
        dig_gain = 0x03;
        dig_fine_gain = 0x80;
    }

    dig_fine_gain = dig_fine_gain & 0xfc;
    sc2337p_write(client, DGAIN_REG, dig_gain);
    sc2337p_write(client, FINE_DGAIN_REG, dig_fine_gain);

#ifdef DEBUG_CIS_PARAM
    pr_info("%s d_gain=%d,dig_gain=%x,dig_fine_gain=%x\n",__func__,d_gain,
            dig_gain,dig_fine_gain);
    pr_info("%s 0x3e06=%x,0x3e07=%x\n",__func__,sc2337p_read(client, 0x3e06),
            sc2337p_read(client, 0x3e07));
#endif
end:
    return D_GAIN_EFFECT_FRAMES;
}

/*set sensor exp time*/
static int __set_reg_exp_time(struct sc2337p_priv* priv, int exp_time)
{
    int ret = 0;
    struct i2c_client* client = priv->client;

    unsigned char exposure_time_ext;
    unsigned char exposure_time_msb;
    unsigned char exposure_time_lsb;

    pr_debug("%s, exp:%d\n", __func__, exp_time);
    if (exp_time < 1) {
        exp_time = 1;
    } else if (exp_time > 65535) {
        exp_time = 65535; // 2^16
    }

    exposure_time_ext = (exp_time >> 12) & 0xf;
    exposure_time_msb = (exp_time >> 4) & 0xff;
    exposure_time_lsb = ((exp_time)&0xf) << 4;

    sc2337p_write(client, EXP_REG_E, exposure_time_ext);
    sc2337p_write(client, EXP_REG_H, exposure_time_msb);
    sc2337p_write(client, EXP_REG_L, exposure_time_lsb);

    priv->aec_parms.r0x3e00_value = exposure_time_ext;
    priv->aec_parms.r0x3e01_value = exposure_time_msb;
    priv->aec_parms.r0x3e02_value = exposure_time_lsb;

#ifdef DEBUG_CIS_PARAM
    pr_info("%s exp_time=%d\n",__func__,exp_time);
    pr_info("%s 0x3e0~02 = 0x%x 0x%x 0x%x \n",__func__,sc2337p_read(client, 0x3e00),
            sc2337p_read(client, 0x3e01),sc2337p_read(client, 0x3e02));
#endif
    // priv->aec_parms.target_exp_ctrl = exp_time;

    return ret;
}

static int __set_reg_frame_vts(struct sc2337p_priv* priv, int vts)
{
    int ret = 0, active_blank_rows = vts - priv->fps_info.rb_rows - 1;
    struct i2c_client* client = priv->client;
    if (vts < 1)
        vts = 1;

    sc2337p_write(client, VTS_REG_H, (vts >> 8));
    sc2337p_write(client, VTS_REG_L, vts & 0xff);

    //pr_info("vts %d rb_rows %d active_blank_rows %d\n",vts,priv->fps_info.rb_rows,active_blank_rows);
    if (priv->master_or_slave == SLAVER_MODE) {
        sc2337p_write(client, SLAVE_ACTIVE_BLANK_ROWS_H, (active_blank_rows >> 8));
        sc2337p_write(client, SLAVE_ACTIVE_BLANK_ROWS_L, (active_blank_rows & 0xff));
    }
    priv->fps_info.reg_fps_value = vts;

    return ret;
}

static int __get_reg_frame_vts(struct sc2337p_priv* priv)
{
    struct i2c_client* client = priv->client;

    return (sc2337p_read(client, VTS_REG_H)) << 8
        | sc2337p_read(client, VTS_REG_L);
}

static void __update_frame_vts_and_exp_ctrl(struct sc2337p_priv* priv)
{
    struct i2c_client* client = priv->client;

    /*
     * exp_time=1 mean half line exptime
     * max exp_time = {R0x320e[7:0],R0x320f} - 6
     */

    int const cur_frame_vts = __get_reg_frame_vts(priv);
    int const target_frame_vts = priv->fps_info.to_fps_value;

    int const max_exp_ctrl = priv->fps_info.to_fps_value - 6; // vts - 6

    int target_exp_ctrl = 0;
    if (priv->aec_parms.target_exp_ctrl <= max_exp_ctrl) {
        target_exp_ctrl = priv->aec_parms.target_exp_ctrl;
        // pr_err("priv->target_exp_ctrl <= max_exp_ctrl, use
        //     priv->aec_parms.target_exp_ctrl:%d\n",
        //     priv->aec_parms.target_exp_ctrl);
    } else {
        target_exp_ctrl = max_exp_ctrl;
        // pr_err("priv->target_exp_ctrl > max_exp_ctrl, use max_exp_ctrl:%d\n",
        // max_exp_ctrl);
    }
#ifdef CONFIG_SYS_FAST_LAUNCH
    if (priv->initial_frame == 1) {
        if (target_exp_ctrl > priv->fps_info.rb_rows) {
            pr_info("The first frame must ensure that exp is less than rbrows exp:%d->%d\n"
                    ,target_exp_ctrl,priv->fps_info.rb_rows-1);
            target_exp_ctrl = priv->fps_info.rb_rows - 1;
        }
        priv->initial_frame = 0;
    }
#endif

    // pr_err("VTS = %d->%d \n", cur_frame_vts, target_frame_vts);
    // pr_err("EXP = %d/%d \n", target_exp_ctrl,
    // priv->aec_parms.target_exp_ctrl);

    if ((target_frame_vts <= 0) || (target_exp_ctrl <= 0))
        return;

    if (target_frame_vts >= cur_frame_vts) {
        if (target_frame_vts != cur_frame_vts)
            __set_reg_frame_vts(priv, target_frame_vts);
        __set_reg_exp_time(priv, target_exp_ctrl);
    } else {
        __set_reg_exp_time(priv, target_exp_ctrl);
        __set_reg_frame_vts(priv, target_frame_vts);
    }
}

/*set sensor exp time*/
/*max exp: 0x3fff*/
static int sc2337p_sen_updata_exp_time_func(void* arg, unsigned int exp_time)
{
    struct sc2337p_priv* priv = arg;
    struct i2c_client* client = priv->client;

    priv->aec_parms.target_exp_ctrl = exp_time;
    __update_frame_vts_and_exp_ctrl(priv);
    return EXP_EFFECT_FRAMES;
}
/*sensor timer*/
static int sc2337p_sen_timer_func(void* arg) { return 0; }
/*standby in*/
static int sc2337p_sen_set_standby_in_func(void* arg) { return 0; }
/*standby out*/
static int sc2337p_sen_set_standby_out_func(void* arg) { return 0; }
/*low level read sensor ID, user i2c ops*/
static int sc2337p_sen_probe_id_func(void* arg) // use IIC bus
{
    struct sc2337p_priv* priv = arg;
    struct i2c_client* client = priv->client;
    int id;
    int value;

    /*
     * TODO:2337p用的序列和2336p一致
     * 0x0100写0x01后sensorID会由0xcb3a变为0x9b3a，用于区分2336和2336p
     * 但操作完0x0100后再加载序列，会出现第一帧概率性异常的问题
     * 原厂提供了0x3021[5:6]写0的办法，但是实际测下来无效
     * av200暂时不支持2336，暂时屏蔽掉读0x9b3a的操作
     */

    // sc2337p_write(client, 0x0100, 0x01);
    // mdelay(10);

    value = sc2337p_read(client, ID_REG_H);
    id = value << 8;

    value = sc2337p_read(client, ID_REG_L);
    id |= value;

    // sc2337p_write(client, 0x0100, 0x00);
    // mdelay(10);

    printk(KERN_ERR "id:%x\n", id);

#ifdef CONFIG_SYS_FAST_LAUNCH
    //双核快启的话小核已经让cis出流了。
    //双核快启双目解决cis check id 异常的问题
    if (id == SENSOR_ACTUAL_ID)
#else 
    //常电的check id
    if (id == SENSOR_PHY_ID)
#endif
        return SENSOR_ID;

    return 0;
}
/*get resolution*/
static int sc2337p_sen_get_resolution_func(void* arg, int* width, int* height)
{
    *width = SENSOR_OUTPUT_WIDTH;
    *height = SENSOR_OUTPUT_HEIGHT;
    return 0;
}
/*get mclk*/
static int sc2337p_sen_get_mclk_func(void* arg) { return SENSOR_MCLK; }
/*get current fps*/
static int sc2337p_sen_get_fps_func(void* arg)
{
    struct sc2337p_priv* priv = arg;

    return priv->fps_info.current_fps;
}
/*get valid coordinate*/
static int sc2337p_sen_get_valid_coord_func(void* arg, int* x, int* y)
{
    *x = SENSOR_VALID_OFFSET_X;
    *y = SENSOR_VALID_OFFSET_Y;
    return 0;
}
/*get bus type*/
static enum sensor_bus_type sc2337p_sen_get_bus_type_func(void* arg)
{
    return SENSOR_BUS_TYPE;
}
#ifdef CONFIG_MACH_KM01A
static int get_ae_fast_default(void* arg,struct ae_fast_struct* ae_fast)
{
    struct sc2337p_priv* priv = arg;
    *ae_fast = priv->ae_fast;
    return 0;
}
static int sensor_set_fast_ae(void* arg,struct ae_fast_struct *ae_fast)
{
    struct sc2337p_priv* priv = arg;
    if(ae_fast)
    {
        pr_err("%s-exp_time:%d\n",__func__,ae_fast->sensor_exp_time);
        if(ae_fast->sensor_exp_time > 0){
            memcpy(&priv->ae_fast, ae_fast, sizeof(struct ae_fast_struct));
        }
        else{
            memcpy(ae_fast, &priv->ae_fast, sizeof(struct ae_fast_struct));	\
        }
        //pr_err("%s-sensor_exp_time=%d\n",__func__,ae_fast_default.sensor_exp_time);
    }

    return 0;
}

static int sc2337p_get_binging_max_selection(struct _sensor_bining_info *binging)
{
    binging->width = SENSOR_BINING_WIDTH;
    binging->height = SENSOR_BINING_HEIGHT;
    return 0;
}
#elif defined (CONFIG_SYS_FAST_LAUNCH)
static int get_ae_fast_default(void* arg,struct ae_fast_struct* ae_fast)
{
    struct sc2337p_priv* priv = arg;
    *ae_fast = priv->ae_fast;
    return 0;
}
static int sensor_set_fast_ae(void* arg,struct ae_fast_struct *ae_fast)
{
    struct sc2337p_priv* priv = arg;
    if(ae_fast)
    {
        pr_err("%s-exp_time:%d\n",__func__,ae_fast->sensor_exp_time);
        if(ae_fast->sensor_exp_time > 0){
            memcpy(&priv->ae_fast, ae_fast, sizeof(struct ae_fast_struct));
        }
        else{
            memcpy(ae_fast, &priv->ae_fast, sizeof(struct ae_fast_struct));	\
        }
        //pr_err("%s-sensor_exp_time=%d\n",__func__,ae_fast_default.sensor_exp_time);
    }

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
static int sc2337p_sen_get_parameter_func(void* arg, int param, void* value)
{
    int ret = 0;
    struct sc2337p_priv* priv = arg;

    switch (param) {
        case GET_INTERFACE:
            *((int*)value) = SENSOR_IO_INTERFACE;
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
            *((int*)value) = DUAL_SYNC_BY_EFSYNC;
            break;


        case GET_AE_FAST_DEFAULT:
#ifdef CONFIG_SYS_FAST_LAUNCH
            get_ae_fast_default(arg,value);
#else
            get_ae_fast_default(value);
#endif
            break;
#ifdef CONFIG_SYS_FAST_LAUNCH
        case SET_FAST_AE:
            ret = sensor_set_fast_ae(arg,value);
            break;
        case GET_MAX_FPS:
            *((int*)value) = MAX_FPS;
            break;
#endif
#ifdef CONFIG_MACH_KM01A
        case GET_RAW_FORMAT:
            *((int*)value) = RAW_FORMAT;
            break;

        case SET_FAST_AE:
            ret = sensor_set_fast_ae(arg,value);
            break;
        case GET_BINING_MAX:
            ret = sc2337p_get_binging_max_selection((struct _sensor_bining_info *)value);
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
#endif
        case GET_SCAN_METHOD:
            ret = -1;
            break;

        case GET_REAL_HEIGHT:
            *((int*)value) = SENSOR_REAL_HEIGHT;
            break;

        default:
            pr_err("%s param:%d not support\n", __func__, param);
            ret = -1;
            break;
    }

    return ret;
}

#ifdef CONFIG_MACH_KM01A
/*以下几个函数 reset/pwdn inc/dec，为了解决多目复用reset/pwdn的问题*/
static bool reset_inc_and_check(struct sc2337p_priv *priv)
{
    int i;

    for (i = 0; i < g_params.reset_num; i++) {
        if (g_params.reset_s[i].pin == priv->gpio_reset) {
            g_params.reset_s[i].using_count++;
            return (g_params.reset_s[i].using_count == 1) ? true:false;
        }
    }

    return false;
};

static bool pwdn_inc_and_check(struct sc2337p_priv *priv)
{
    int i;

    for (i = 0; i < g_params.pwdn_num; i++) {
        if (g_params.pwdn_s[i].pin == priv->gpio_pwdn) {
            g_params.pwdn_s[i].using_count++;
            return (g_params.pwdn_s[i].using_count == 1) ? true:false;
        }
    }

    return false;
};

static bool reset_dec_and_check(struct sc2337p_priv *priv)
{
    int i;

    for (i = 0; i < g_params.reset_num; i++) {
        if (g_params.reset_s[i].pin == priv->gpio_reset) {
            if(g_params.reset_s[i].using_count > 0) {
                g_params.reset_s[i].using_count--;
            }
            return (g_params.reset_s[i].using_count == 0) ? true:false;
        }
    }

    return false;
};

static bool pwdn_dec_and_check(struct sc2337p_priv *priv)
{
    int i;

    for (i = 0; i < g_params.pwdn_num; i++) {
        if (g_params.pwdn_s[i].pin == priv->gpio_pwdn) {
            if(g_params.pwdn_s[i].using_count > 0) {
                g_params.pwdn_s[i].using_count--;
            }
            return (g_params.pwdn_s[i].using_count == 0) ? true:false;
        }
    }

    return false;
};
#endif

/*sensor power on*/
static int sc2337p_sen_set_power_on_func(void* arg)
{
    struct sc2337p_priv* priv = arg;

#ifdef CONFIG_MACH_KM01A
    if (priv->gpio_pwdn >= 0 && pwdn_inc_and_check(priv)) {
#else
    if (priv->gpio_pwdn >= 0) {
#endif
        gpio_direction_output(priv->gpio_pwdn, !SENSOR_PWDN_LEVEL);
    }

#ifdef CONFIG_MACH_KM01A
    if (priv->gpio_reset >= 0 && reset_inc_and_check(priv)) {
#else
    if (priv->gpio_reset >= 0) {
#endif
        if(sensor_power_on){
            gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
            msleep(50);
            gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
            msleep(50);
            gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
            msleep(100);
        } else {
            gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
        }
    }

    // aec_parms_init(priv);

    priv->dual.init_count = 0;
    // priv->master_or_slave = SINGLE_MODE;

    return 0;
}
/*sensor power off*/
static int sc2337p_sen_set_power_off_func(void* arg)
{
    struct sc2337p_priv* priv = arg;

#ifdef CONFIG_MACH_KM01A
    if (priv->gpio_pwdn >= 0 && pwdn_dec_and_check(priv)) {
#else
    if (priv->gpio_pwdn >= 0) {
#endif
        gpio_direction_output(priv->gpio_pwdn, SENSOR_PWDN_LEVEL);
    }

#ifdef CONFIG_MACH_KM01A
    if (priv->gpio_reset >= 0 && reset_dec_and_check(priv)) {
#else
    if (priv->gpio_reset >= 0) {
#endif
        gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
    }

    return 0;
}

static int sc2337p_fps_to_vts(struct sc2337p_priv* priv, const int fps)
{
    /*
     * sc431ai:
     *
     * mipi:
     * HTS=(0x320c,0x320d)*2
     * VTS=(0x320e[6:0],0x320f)
     * fps = pclk / HTS / VTS
     *
     */
    int vts;
    unsigned int fps_tmp;

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

//    vts = priv->aec_parms.pclk_freq / priv->aec_parms.reg_frame_hts;
//    vts = vts * 100;
//    vts = vts / fps_tmp;
     vts = (priv->aec_parms.calc_vts_tmp) / fps_tmp;
     pr_debug("%s calc %d, vts %d\n",__func__,priv->aec_parms.calc_vts_tmp,vts);

    return vts;
}

/*set sensor fps*/
static int sc2337p_sen_set_fps_func(void* arg, const int fps)
{
    struct sc2337p_priv* priv = arg;
    int tmp;

    tmp = sc2337p_fps_to_vts(priv, fps);
    if (tmp > 0) {
        priv->fps_info.to_fps = fps;
        priv->fps_info.to_fps_value = tmp;
    } else {
        return -EINVAL;
    }

    return 0;
}

static void sc2337p_set_fps_async(struct sc2337p_priv* priv)
{
    struct i2c_client* client = priv->client;

    if (priv->fps_info.to_fps != priv->fps_info.current_fps) {
        __update_frame_vts_and_exp_ctrl(priv);
        priv->fps_info.current_fps = priv->fps_info.to_fps;
    }
}

static AK_ISP_SENSOR_CB sc2337p_cb = {
    .sensor_init_func = sc2337p_sen_init_func,
    .sensor_read_reg_func = sc2337p_sen_read_reg_func,
    .sensor_write_reg_func = sc2337p_sen_write_reg_func,
    .sensor_read_id_func = sc2337p_sen_read_id_func,
    .sensor_update_a_gain_func = sc2337p_sen_update_a_gain_func,
    .sensor_update_d_gain_func = sc2337p_sen_update_d_gain_func,
    .sensor_updata_exp_time_func = sc2337p_sen_updata_exp_time_func,
    .sensor_timer_func = sc2337p_sen_timer_func,
    .sensor_set_standby_in_func = sc2337p_sen_set_standby_in_func,
    .sensor_set_standby_out_func = sc2337p_sen_set_standby_out_func,
    .sensor_probe_id_func = sc2337p_sen_probe_id_func,
    .sensor_get_resolution_func = sc2337p_sen_get_resolution_func,
    .sensor_get_mclk_func = sc2337p_sen_get_mclk_func,
    .sensor_get_fps_func = sc2337p_sen_get_fps_func,
    .sensor_get_valid_coordinate_func = sc2337p_sen_get_valid_coord_func,
    .sensor_get_bus_type_func = sc2337p_sen_get_bus_type_func,
    .sensor_get_parameter_func = sc2337p_sen_get_parameter_func,
    .sensor_set_power_on_func = sc2337p_sen_set_power_on_func,
    .sensor_set_power_off_func = sc2337p_sen_set_power_off_func,
    .sensor_set_fps_func = sc2337p_sen_set_fps_func,
};

static int sensor_id_func(void) { return sc2337p_sen_read_id_func(NULL); }

static char* sensor_if_func(void)
{
    static char ifstr[16] = "dvp";

    if (SENSOR_IO_INTERFACE == MIPI_INTERFACE)
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
