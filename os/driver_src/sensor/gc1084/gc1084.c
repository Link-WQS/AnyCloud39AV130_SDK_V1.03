/*
 * gc1084 Camera Driver
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
static int SENSOR_I2C_ADDR      = 0x37;  //使用两路不同的i2c，相同设备地址
#define SENSOR_ID             0x1084
#define SENSOR_MCLK           24
#define SENSOR_REGADDR_BYTE   2
#define SENSOR_DATA_BYTE      1
#define SENSOR_OUTPUT_WIDTH   1280
#define SENSOR_OUTPUT_HEIGHT  720
#define SENSOR_REAL_HEIGHT    720 //sensor实际输出行数
#define SENSOR_VALID_OFFSET_X 0
#define SENSOR_VALID_OFFSET_Y 0
#define SENSOR_BUS_TYPE       BUS_TYPE_RAW
//#define SENSOR_IO_INTERFACE       DVP_INTERFACE
#define SENSOR_IO_INTERFACE MIPI_INTERFACE
#define SENSOR_IO_LEVEL     IO_LEVEL_1V8
#define I2C_NUM             2
#define MIPI_MBPS           400
#define MIPI_LANES          1
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

#define INIT_EXP_EFFECT_FRAMES      0
#define FLIP_MIRROR_EFFECT_FRAMES   0

#define ID_REG_H    (0x03f0)
#define ID_REG_L    (0x03f1)
#define VTS_REG_H   (0x0d41)
#define VTS_REG_L   (0x0d42)
#define EXP_REG_H   (0x0d03)
#define EXP_REG_L   (0x0d04)
#define ID_REG_H    (0x03f0)
#define ID_REG_L    (0x03f1)

#define DGAIN_REG_H (0x00b1)
#define DGAIN_REG_L (0x00b2)

#define WIN_OFF_Y_L (0x192)
#define WIN_OFF_X_L (0x194)

#define SET_FPS_EN      (1)
#define SET_EXP_EN      (1)
#define SET_AGAIN_EN    (1)
#define SET_DGAIN_EN    (1)



#define DELAY_FLAG  (0xffff)

/*
 * Struct
 */
struct regval_list {
    u8 reg_num;
    u8 value;
};

struct gc1084_win_size {
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
struct gc1084_aec_parms {
    int auto_off;
    int off_x;
    int off_y;
    int target_exp_ctrl;
    int reg_frame_vts;
};

static const struct v4l2_ctrl_ops gc1084_ctrl_ops;
static const struct v4l2_ctrl_config config_sensor_get_id = {
    .ops = &gc1084_ctrl_ops,
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
struct gc1084_priv {
    struct list_head list;
    struct i2c_client* client;
    struct v4l2_subdev subdev;
    struct v4l2_ctrl_handler hdl;
    u32 cfmt_code;
    const struct gc1084_win_size* win;

    int gpio_reset;
    int gpio_pwdn;
    char power_on_flag;//0->off, 1->on
    unsigned int sensor_id_addr_v;//0->low, 1->high
    struct sensor_cb_info cb_info;
    struct sensor_fps_info fps_info;

    struct host_callbacks hcb;
    struct gc1084_aec_parms aec_parms;

    AK_ISP_SENSOR_INIT_PARA para;

    struct v4l2_ctrl* ctrl_sensor_get_id;
    
    /*for 双目CIS 1. 在 CIS 的私有结构体内添加链表*/
    enum sensor_master_or_slave_mode master_or_slave;
    struct dual_sensor_attr dual;
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

static AK_ISP_SENSOR_CB gc1084_cb;

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
static int sensor0_switch = 0;
static int sensor1_switch = 0;

module_param(addr0, int, 0644);
module_param(addr1, int, 0644);
module_param(SENSOR_I2C_ADDR, int, 0644);
module_param(sensor0_switch, int, 0644);
module_param(sensor1_switch, int, 0644);
module_param(MAX_FPS, int, 0644);

static int gc1084_write(const struct i2c_client* client, int reg, int value);
static int gc1084_read(const struct i2c_client* client, int reg);
static void gc1084_set_fps_async(struct gc1084_priv* priv);
static int __gc1084_sen_probe_id_func(
    struct i2c_client* client); // use IIC bus
static int gc1084_sen_read_id_func(void* arg); // no use IIC bus
static int gc1084_sen_get_resolution_func(
    void* arg, int* width, int* height);
static int gc1084_sen_set_fps_func(void* arg, const int fps);
static int gc1084_fps_to_vts(struct gc1084_priv* priv, const int fps);
static int call_sensor_sys_init(void);
static int call_sensor_sys_deinit(void);
static int gc1084_sen_probe_id_func(void* arg); // use IIC bus

static u32 gc1084_codes[] = {
    MEDIA_BUS_FMT_YUYV8_2X8,
    MEDIA_BUS_FMT_UYVY8_2X8,
    MEDIA_BUS_FMT_RGB565_2X8_BE,
    MEDIA_BUS_FMT_RGB565_2X8_LE,
};

/*
 * General functions
 */
static struct gc1084_priv* to_gc1084(const struct i2c_client* client)
{
    return container_of(i2c_get_clientdata(client), struct gc1084_priv, subdev);
}

static struct v4l2_subdev* ctrl_to_sd(struct v4l2_ctrl* ctrl)
{
    return &container_of(ctrl->handler, struct gc1084_priv, hdl)->subdev;
}

/*
 * XXXX_s_stream -
 * set steaming enable/disable
 * soc_camera_ops functions
 *
 * @sd:             subdev
 * @enable:         enable flags
 */
static int gc1084_s_stream(struct v4l2_subdev* sd, int enable) { return 0; }

static int gc1084_g_volatile_ctrl(struct v4l2_ctrl* ctrl)
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
static int gc1084_s_ctrl(struct v4l2_ctrl* ctrl) { return 0; }

/*
 * XXXX_s_power -
 * set power operation
 * soc_camera_ops functions
 *
 * @sd:         subdev
 * @on:         power flags
 */
static int gc1084_core_s_power(struct v4l2_subdev* sd, int on) { return 0; }

/*
 * XXXX_get_sensor_id -
 * get sensor ID
 * private callback functions
 *
 * @ctrl:           pointer to ctrl
 */
static int gc1084_get_sensor_id(struct v4l2_control* ctrl)
{
    ctrl->value = gc1084_sen_read_id_func(NULL); // no use IIC bus
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
static int gc1084_get_sensor_cb(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc1084_priv* priv = to_gc1084(client);

    ctrl->value = (int)&priv->cb_info;
    return 0;
}

static int gc1084_get_max_exp_for_fps(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc1084_priv* priv = to_gc1084(client);
    int fps = ctrl->value;
    int vts = gc1084_fps_to_vts(priv, fps);

    ctrl->value = vts;
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
static int gc1084_core_g_ctrl(struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    int ret;

    switch (ctrl->id) {
        case GET_SENSOR_ID:
            ret = gc1084_get_sensor_id(ctrl);
            break;

        case GET_SENSOR_CB:
            ret = gc1084_get_sensor_cb(sd, ctrl);
            break;

        case GET_MAX_EXP_FOR_FPS:
            ret = gc1084_get_max_exp_for_fps(sd, ctrl);
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
static int gc1084_set_isp_timing_cb(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc1084_priv* priv = to_gc1084(client);
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
static int gc1084_set_fps_direct(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc1084_priv* priv = to_gc1084(client);
    int fps = ctrl->value;

    gc1084_sen_set_fps_func(priv, fps);
    gc1084_set_fps_async(priv);
    return 0;
}

static int ak_sensor_set_master_or_slave(struct v4l2_subdev *sd,
                                                        int is_master)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc1084_priv* priv = to_gc1084(client);

    pr_info("%s is_master:%d\n",__func__,is_master);

    if (is_master)
        priv->master_or_slave = MASTER_MODE;
    else
        priv->master_or_slave = SLAVER_MODE;

    return 0;
}

static int ak_sensor_set_flip_mirror(struct v4l2_subdev* sd, int enable)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc1084_priv* priv = to_gc1084(client);
    int flip_en = enable & (0x1 << FLIP_OFFSET);
    int mirror_en = enable & (0x1 << MIRROR_OFFSET);
    int reg0x0015_value = 0;
    int reg0x0d15_value = 0;

    if (flip_en && mirror_en) {
        reg0x0015_value = 0x03;
        reg0x0d15_value = 0x03;
    } else if (flip_en && !mirror_en) {
        reg0x0015_value = 0x01;
        reg0x0d15_value = 0x01;
    } else if (!flip_en && mirror_en) {
        reg0x0015_value = 0x02;
        reg0x0d15_value = 0x02;
    } else {
        reg0x0015_value = 0x00;
        reg0x0d15_value = 0x00;
    }

    gc1084_write(client, (0x0015), reg0x0015_value);
    gc1084_write(client, (0x0d15), reg0x0d15_value);

    return FLIP_MIRROR_EFFECT_FRAMES;
}

/*
 * XXXX_core_s_ctrl -
 * set core s_ctrl
 * core functions
 *
 * @sd:             subdev
 * @ctrl:           pointer to ctrl
 */
static int gc1084_core_s_ctrl(struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    int ret;

    switch (ctrl->id) {
        case SET_ISP_MISC_CALLBACK:
            ret = gc1084_set_isp_timing_cb(sd, ctrl);
            break;

        case SET_FPS_DIRECT:
            ret = gc1084_set_fps_direct(sd, ctrl);
            break;

        case SET_MASTER:/*for dual sensor*/
            ret = ak_sensor_set_master_or_slave(sd, 1);
            break;

        case SET_SLAVER:/*for dual sensor*/
            ret = ak_sensor_set_master_or_slave(sd, 0);
            break;

        case SET_FLIP_MIRROR:
            ret = ak_sensor_set_flip_mirror(sd, ctrl->value);
            break;

        default:
            pr_err("%s cmd:%d not support\n", __func__, ctrl->id);
            ret = -1;
            break;
    }

    return ret;
}

static int gc1084_store_initial_regs(struct gc1084_priv* priv, void* arg)
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

static long gc1084_core_ioctl(
    struct v4l2_subdev* sd, unsigned int cmd, void* arg)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc1084_priv* priv = to_gc1084(client);
    int ret = 0;

    pr_debug("%s\n", __func__);

    switch (cmd) {
        case AK_SENSOR_SET_INIT:
            gc1084_store_initial_regs(priv, arg);
            break;

        case AK_SENSOR_GET_MAX_EXP_FOR_FPS: {
            struct sensor_max_exp_for_fps* exp_for_fps = arg;
            struct v4l2_control ctrl;

            ctrl.value = exp_for_fps->fps;
            ret = gc1084_get_max_exp_for_fps(sd, &ctrl);
            if (!ret)
                exp_for_fps->max_exp = ctrl.value;
            break;
        }

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
static int gc1084_get_fmt(struct v4l2_subdev* sd,
    struct v4l2_subdev_pad_config* cfg, struct v4l2_subdev_format* format)
{
    struct v4l2_mbus_framefmt* mf = &format->format;
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc1084_priv* priv = to_gc1084(client);
    static struct gc1084_win_size win = {
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
static int gc1084_set_fmt(struct v4l2_subdev* sd,
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
static int gc1084_get_selection(struct v4l2_subdev* sd,
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
static int gc1084_enum_mbus_code(struct v4l2_subdev* sd,
    struct v4l2_subdev_pad_config* cfg, struct v4l2_subdev_mbus_code_enum* code)
{
    if (code->pad || code->index >= ARRAY_SIZE(gc1084_codes))
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
static int gc1084_g_crop(struct v4l2_subdev* sd, struct v4l2_crop* a)
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
static int gc1084_cropcap(struct v4l2_subdev* sd, struct v4l2_cropcap* a)
{
    gc1084_sen_get_resolution_func(
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
static int gc1084_video_probe(struct i2c_client* client)
{
    struct gc1084_priv* priv = to_gc1084(client);
    int ret;

    ret = v4l2_ctrl_handler_setup(&priv->hdl);

    return ret;
}

/*ctrl ops*/
static const struct v4l2_ctrl_ops gc1084_ctrl_ops = {
    .g_volatile_ctrl = gc1084_g_volatile_ctrl,
    .s_ctrl = gc1084_s_ctrl,
};

/*core ops*/
static struct v4l2_subdev_core_ops gc1084_subdev_core_ops = {
    .s_power = gc1084_core_s_power,
    .g_ctrl = gc1084_core_g_ctrl,
    .s_ctrl = gc1084_core_s_ctrl,
    .ioctl = gc1084_core_ioctl,
};

/*
 * XXXX_g_mbus_config -
 * get buf config
 * video functions
 *
 * @sd:             subdev
 * @cfg:            return config
 */
static int gc1084_g_mbus_config(
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
static int gc1084_s_crop(struct v4l2_subdev* sd, const struct v4l2_crop* crop)
{
    printk(KERN_ERR "%s %d, left:%d, top:%d, width:%d, height:%d\n", __func__,
        __LINE__, crop->c.left, crop->c.top, crop->c.width, crop->c.height);
    return 0;
}

/*video ops*/
static struct v4l2_subdev_video_ops gc1084_subdev_video_ops = {
    .s_stream = gc1084_s_stream,
    .cropcap = gc1084_cropcap,
    .g_crop = gc1084_g_crop,
    .g_mbus_config = gc1084_g_mbus_config,
    .s_crop = gc1084_s_crop,
};

/*pad ops*/
static const struct v4l2_subdev_pad_ops gc1084_subdev_pad_ops = {
    .enum_mbus_code = gc1084_enum_mbus_code,
    .get_fmt = gc1084_get_fmt,
    .set_fmt = gc1084_set_fmt,
    .get_selection = gc1084_get_selection,
};

/*sensor driver subdev ops*/
static struct v4l2_subdev_ops gc1084_subdev_ops = {
    .core = &gc1084_subdev_core_ops,
    .video = &gc1084_subdev_video_ops,
    .pad = &gc1084_subdev_pad_ops,
};

/*
 * sensor_of_parse -
 * parse node of device
 *
 * @client:         pointor to i2c client
 * @priv:           sensor struct
 */
static int sensor_of_parse(struct i2c_client* client, struct gc1084_priv* priv)
{
    struct device_node* np = client->dev.of_node;

    /*it is exist reset but lack of pwdn gpio in most case*/
    priv->gpio_reset = of_get_named_gpio(np, "reset-gpio", 0);
    priv->gpio_pwdn = of_get_named_gpio(np, "pwdn-gpio", 0);

    of_property_read_u32(np, "sensor-id-addr", &priv->sensor_id_addr_v);

    if (priv->gpio_reset >= 0) 
    {
        devm_gpio_request(&client->dev, priv->gpio_reset, "sensor-reset");
    }

    if (priv->gpio_pwdn >= 0) 
    {
        devm_gpio_request(&client->dev, priv->gpio_pwdn, "sensor-pwdn");
    }

    return 0;
}

/*
 * XXXX_match - check sensor id for match driver
 * @priv:           pointer to pirvate structure
 *
 * @RETURN: 0-match fail; 1-match success
 */
static int gc1084_match(struct gc1084_priv* priv)
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
    struct gc1084_priv* t_priv = NULL;
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
                pr_info("%s %d new_addr0:0x%x\n",__func__,__LINE__,
                                                    client->addr);
                break;

            case 1:
                if(addr1 > 0)
                {
                    pr_err("%s %d new_addr1:0x%x\n", __func__, __LINE__, addr1);
                    client->addr = addr1;
                }
                pr_info("%s %d new_addr1:0x%x\n",__func__,__LINE__,
                                                    client->addr);
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
static int gc1084_probe(
    struct i2c_client* client, const struct i2c_device_id* did)
{
    struct gc1084_priv* priv;
    struct i2c_adapter* adapter = to_i2c_adapter(client->dev.parent);
    int ret;

    pr_err("%s %s %s\n", __func__, __DATE__, __TIME__);

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_err(&adapter->dev, "gc1084: I2C-Adapter doesn't support SMBUS\n");
        return -EIO;
    }

    priv = devm_kzalloc(&client->dev, sizeof(struct gc1084_priv), GFP_KERNEL);
    if (!priv) {
        dev_err(&adapter->dev, "Failed to allocate memory for private data!\n");
        return -ENOMEM;
    }

    /*parse of node*/
    sensor_of_parse(client, priv);

    /*
     * the current sensor slave address to client.
     * the address from dts may incorrect
     * */
    client->addr = SENSOR_I2C_ADDR;
    priv->cb_info.cb = &gc1084_cb;
    priv->cb_info.arg = priv;
    priv->client = client;

    if (!client->dev.of_node) {
        dev_err(&client->dev, "Missing platform_data for driver\n");
        ret = -EINVAL;
        goto err_clk;
    }

    if (!gc1084_match(priv)) {
        ret = -ENODEV;
        goto err_clk;
    }

    /*subdev init*/
    v4l2_i2c_subdev_init(&priv->subdev, client, &gc1084_subdev_ops);
    priv->subdev.flags
        |= /*V4L2_SUBDEV_FL_HAS_EVENTS | */ V4L2_SUBDEV_FL_HAS_DEVNODE;
    v4l2_ctrl_handler_init(&priv->hdl, 2);
    v4l2_ctrl_new_std(&priv->hdl, &gc1084_ctrl_ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
    v4l2_ctrl_new_std(&priv->hdl, &gc1084_ctrl_ops, V4L2_CID_HFLIP, 0, 1, 1, 0);

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

    ret = gc1084_video_probe(client);
    if (ret < 0) {
        dev_err(&client->dev, "gc1084 probe fail\n");
        goto err_videoprobe;
    }

    ret = v4l2_async_register_subdev(&priv->subdev);
    if (ret < 0) {
        dev_err(&client->dev, "v4l2 async register subdev fail, ret:%d\n", ret);
        goto err_videoprobe;
    }

   /* 添加初始化链表，并添加到 priv_list 链表尾 */
    INIT_LIST_HEAD(&priv->list);
    list_add_tail(&priv->list, &privs_list);

    //client->addr = SENSOR_I2C_ADDR;
    client_new_addr();

    dev_err(&adapter->dev, "gc1084 Probed success, subdev:%p\n", &priv->subdev);

    /*ak platform need export some informations from sensor*/
    call_sensor_sys_init();

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
static int gc1084_remove(struct i2c_client* client)
{
    struct gc1084_priv* priv = to_gc1084(client);

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

static const struct i2c_device_id gc1084_id[] = { { "gc1084", 0 }, {} };
MODULE_DEVICE_TABLE(i2c, gc1084_id);

static const struct of_device_id gc1084_of_match[] = {
    /*donot changed compatible, must the same as dts*/
    {
        .compatible = "anyka,sensor0",
    },
    {},
};
MODULE_DEVICE_TABLE(of, gc1084_of_match);

static struct i2c_driver gc1084_i2c_driver = {
    .driver = {
        .name = "gc1084",
        .of_match_table = of_match_ptr(gc1084_of_match),
    },
    .probe    = gc1084_probe,
    .remove   = gc1084_remove,
    .id_table = gc1084_id,
};

module_i2c_driver(gc1084_i2c_driver);

MODULE_DESCRIPTION("SoC Camera driver for gc1084 sensor");
MODULE_AUTHOR("Anyka Microelectronic");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0.04");

//v1.0.02增加了双目功能 2023.06.29
//v1.0.03更正翻转功能，修改返回值，返回值非零都代表翻转失败 2023.07.04

/*sensor i2c write*/
static int gc1084_write(const struct i2c_client* client, int reg, int value)
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
static int gc1084_read(const struct i2c_client* client, int reg)
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

/*
 * The sensor must set callbacks
 *
 * */
static int gc1084_sen_init_func(
    void* arg, const AK_ISP_SENSOR_INIT_PARA* npara)
{
    int i;
    int value;
    AK_ISP_SENSOR_REG_INFO* preg_info;
    struct gc1084_priv* priv = arg;
    struct i2c_client* client = priv->client;
    AK_ISP_SENSOR_INIT_PARA* para = &priv->para;
    int addr = client->addr;
    int vts_h = -1;
    int vts_l = -1;

    client->addr = SENSOR_I2C_ADDR;

    gc1084_sen_probe_id_func(arg); // use IIC bus

    if (para->num <= 0 && npara)
        para = (void*)npara;

    preg_info = para->reg_info;
    for (i = 0; i < para->num; i++) {
#if 0
        {
            int value;

            value = gc1084_read(client, preg_info->reg_addr);
            pr_err("reg:%x read value:%x\n", preg_info->reg_addr, value);
        }
#endif
        if(preg_info->reg_addr == VTS_REG_H)
            vts_h = preg_info->value;
        else if(preg_info->reg_addr == VTS_REG_L)
            vts_l = preg_info->value;

        gc1084_write(client, preg_info->reg_addr, preg_info->value);
#if 0
        {
            int value;

            value = gc1084_read(client, preg_info->reg_addr);
            pr_err("reg:%x write value:%x, read back value:%x\n",
                    preg_info->reg_addr, preg_info->value, value);
        }
#endif

        preg_info++;
    }
    if(addr != client->addr){
        gc1084_write(client, 0x3fb, addr<<1);
        client->addr = addr;
        gc1084_sen_probe_id_func(arg); // use IIC bus
    }

    if ((vts_h != -1) && (vts_l != -1))
        priv->aec_parms.reg_frame_vts = ((vts_h & 0x3f) << 8 | vts_l);
    else
        priv->aec_parms.reg_frame_vts = ((
            gc1084_read(client, VTS_REG_H) & 0x3f) << 8
            | gc1084_read(client, VTS_REG_L));

    priv->fps_info.current_fps = MAX_FPS;
    priv->fps_info.to_fps = priv->fps_info.current_fps;
    priv->fps_info.to_fps_value = 0;

    pr_info("%s %d vts, MAX_FPS %d\n", __func__,
        priv->aec_parms.reg_frame_vts, MAX_FPS);

    return 0;
}

/*read sensor register*/
static int gc1084_sen_read_reg_func(void* arg, const int reg_addr)
{
    struct gc1084_priv* priv = arg;
    struct i2c_client* client = priv->client;

    return gc1084_read(client, reg_addr);
}
/*write sensor register*/
static int gc1084_sen_write_reg_func(
    void* arg, const int reg_addr, int value)
{
    struct gc1084_priv* priv = arg;
    struct i2c_client* client = priv->client;

    return gc1084_write(client, reg_addr, value);
}
/*read sensor register, NO i2c ops*/
static int gc1084_sen_read_id_func(void* arg) // no use IIC bus
{
    return SENSOR_ID;
}

/*
 * from gc fae
 * refer to v0.1_release_GC1084_AEC_20220105@64倍.c
 * */
static unsigned char regValTable[25][6] = {
//0x00d1 0x00d0 0x0dc1 0x155 0x00b8 0x00b9
    {0x00, 0x00, 0x00, 0x80, 0x01, 0x00},
    {0x0a, 0x00, 0x00, 0x80, 0x01, 0x0b},
    {0x00, 0x01, 0x00, 0x80, 0x01, 0x19},
    {0x0a, 0x01, 0x00, 0x80, 0x01, 0x2a},
    {0x00, 0x02, 0x00, 0x80, 0x02, 0x00},
    {0x0a, 0x02, 0x00, 0x80, 0x02, 0x17},
    {0x00, 0x03, 0x00, 0x80, 0x02, 0x33},
    {0x0a, 0x03, 0x00, 0x80, 0x03, 0x14},
    {0x00, 0x04, 0x00, 0x90, 0x04, 0x00},
    {0x0a, 0x04, 0x00, 0x90, 0x04, 0x2f},
    {0x00, 0x05, 0x00, 0x90, 0x05, 0x26},
    {0x0a, 0x05, 0x00, 0x90, 0x06, 0x28},
    {0x00, 0x06, 0x00, 0xa0, 0x08, 0x00},
    {0x0a, 0x06, 0x00, 0xa0, 0x09, 0x1e},
    {0x12, 0x46, 0x00, 0xa0, 0x0b, 0x0c},
    {0x19, 0x66, 0x00, 0xa0, 0x0d, 0x10},
    {0x00, 0x04, 0x01, 0xa0, 0x10, 0x00},
    {0x0a, 0x04, 0x01, 0xa0, 0x12, 0x3d},
    {0x00, 0x05, 0x01, 0xb0, 0x16, 0x19},
    {0x0a, 0x05, 0x01, 0xc0, 0x1a, 0x23},
    {0x00, 0x06, 0x01, 0xc0, 0x20, 0x00},
    {0x0a, 0x06, 0x01, 0xc0, 0x25, 0x3b},
    {0x12, 0x46, 0x01, 0xc0, 0x2c, 0x30},
    {0x19, 0x66, 0x01, 0xd0, 0x35, 0x01},
    {0x20, 0x06, 0x01, 0xe0, 0x3f, 0x3f},
};

static unsigned int gainLevelTable[26] = {
    64,
    76,
    90,
    106,
    128,
    152,
    179,
    212,
    256,
    303,
    358,
    425,
    512,
    607,
    716,
    848,
    1024,
    1214,
    1434,
    1699,
    2048,
    2427,
    2865,
    3393,
    4096,
    0xffffffff,
};

/*************************************************************************
 ** from gc fae
 ** refer to v0.1_release_GC1084_AEC_20220105@64倍.c
 * * FUNCTION
 * *    set_gain
 * *
 * * DESCRIPTION
 * *    This function is to set global gain to sensor.
 * *
 * * PARAMETERS
 * *    iGain : sensor global gain(base: 0x40)
 * *
 * * RETURNS
 * *    the actually gain set to sensor.
 * *
 * * GLOBALS AFFECTED
 * *
 * *************************************************************************/
static int SetSensor_Gain(const struct i2c_client* client, unsigned int again)
{
    int i;
    int total = sizeof(gainLevelTable) / sizeof(unsigned int);
    unsigned int tol_dig_gain = 0;

    for (i = 0; i < total; i++) {
        if ((gainLevelTable[i] <= again) && (again < gainLevelTable[i + 1]))
            break;
    }

    tol_dig_gain = again * 64 / gainLevelTable[i];

    gc1084_write(client, 0x00d1, regValTable[i][0]);
    gc1084_write(client, 0x00d0, regValTable[i][1]);
    gc1084_write(client, 0x031d, 0x2e);
    gc1084_write(client, 0x0dc1, regValTable[i][2]);
    gc1084_write(client, 0x031d, 0x28);
    gc1084_write(client, 0x0155, regValTable[i][3]);
    gc1084_write(client, 0x00b8, regValTable[i][4]);
    gc1084_write(client, 0x00b9, regValTable[i][5]);

    gc1084_write(client, DGAIN_REG_H, (tol_dig_gain >> 6));
    gc1084_write(client, DGAIN_REG_L, ((tol_dig_gain & 0x3f) << 2));

    return 0;
}

/*set sensor again*/
/*max gain: 64 */
static int gc1084_sen_update_a_gain_func(
    void* arg, const unsigned int a_gain)
{
    struct gc1084_priv* priv = arg;
    struct i2c_client* client = priv->client;

    pr_debug("gc1084 again:%d\n", a_gain);

    SetSensor_Gain(client, a_gain / 4);

    return A_GAIN_EFFECT_FRAMES;
}
/*set sensor dgain*/
static int gc1084_sen_update_d_gain_func(
    void* arg, const unsigned int d_gain)
{
    return D_GAIN_EFFECT_FRAMES;
}

/*set sensor exp time*/
static int __set_reg_exp_time(struct gc1084_priv* priv, int exp_time)
{
    int ret = 0;
    struct i2c_client* client = priv->client;

    if (exp_time < 1)
        exp_time = 1;
    if (exp_time > 8191)
        exp_time = 8191;

    pr_debug("%s, exp:%d\n", __func__, exp_time);

    gc1084_write(client, 0x0d03, exp_time >> 8);
    gc1084_write(client, 0x0d04, exp_time & 0xff);

    return ret;
}

static int __set_reg_frame_vts(struct gc1084_priv* priv, int vts)
{
    int ret = 0;
    struct i2c_client* client = priv->client;
    if (vts < 1)
        vts = 1;

    pr_debug("%s, vts:%d\n", __func__, vts);

    gc1084_write(client, VTS_REG_H, (vts >> 8));
    gc1084_write(client, VTS_REG_L, (vts & 0xff));

    return ret;
}

static int __get_reg_frame_vts(struct gc1084_priv* priv)
{
    struct i2c_client* client = priv->client;

    return (gc1084_read(client, VTS_REG_H)) << 8
        | gc1084_read(client, VTS_REG_L);
}

static void update_frame_vts_and_exp_ctrl(struct gc1084_priv* priv)
{
    struct i2c_client* client = priv->client;

    /*
     * exp_time=1 mean half line exptime
     * max exp_time = {R0x0d41,0x0d42} - 16
     *
     * now:
     * at max_exptime_30fps
     * = {R0x0d41,0x0d42} - 16
     * = 0x2ee - 16
     * = 750-16
     * = 734
     */

    int const cur_frame_vts = __get_reg_frame_vts(priv);
    int const target_frame_vts = priv->fps_info.to_fps_value;

    int const max_exp_ctrl = priv->fps_info.to_fps_value - 16; // Vts-16

    int target_exp_ctrl = 0;
    if (priv->aec_parms.target_exp_ctrl <= max_exp_ctrl) {
        target_exp_ctrl = priv->aec_parms.target_exp_ctrl;
        // pr_err("priv->target_exp_ctrl <= max_exp_ctrl, use
        // priv->aec_parms.target_exp_ctrl:%d\n",
        //priv->aec_parms.target_exp_ctrl);
    } else {
        target_exp_ctrl = max_exp_ctrl;
        //pr_err("priv->target_exp_ctrl > max_exp_ctrl, use max_exp_ctrl:%d\n",
        // max_exp_ctrl);
    }

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
static int gc1084_sen_updata_exp_time_func(void* arg, unsigned int exp_time)
{
    struct gc1084_priv* priv = arg;
    struct i2c_client* client = priv->client;

    /*
     * max_exp_time = frame_length -16
     * */
    priv->aec_parms.target_exp_ctrl = exp_time;
    update_frame_vts_and_exp_ctrl(priv);

    return EXP_EFFECT_FRAMES;
}
/*sensor timer*/
static int gc1084_sen_timer_func(void* arg) { return 0; }
/*standby in*/
static int gc1084_sen_set_standby_in_func(void* arg) { return 0; }
/*standby out*/
static int gc1084_sen_set_standby_out_func(void* arg) { return 0; }
/*low level read sensor ID, user i2c ops*/
static int gc1084_sen_probe_id_func(void* arg) // use IIC bus
{
    struct gc1084_priv* priv = arg;
    struct i2c_client* client = priv->client;
    int id;
    int value;

    value = gc1084_read(client, ID_REG_H);
    id = value << 8;
    value = gc1084_read(client, ID_REG_L);
    id |= value;
    pr_err("%s gc1084_id:0x%x\n", __func__, id);

    if (id == SENSOR_ID)
        return SENSOR_ID;

    return 0;
}
/*get resolution*/
static int gc1084_sen_get_resolution_func(void* arg, int* width, int* height)
{
    *width = SENSOR_OUTPUT_WIDTH;
    *height = SENSOR_OUTPUT_HEIGHT;
    return 0;
}
/*get mclk*/
static int gc1084_sen_get_mclk_func(void* arg) { return SENSOR_MCLK; }
/*get current fps*/
static int gc1084_sen_get_fps_func(void* arg)
{
    struct gc1084_priv* priv = arg;

    return priv->fps_info.current_fps;
}
/*get valid coordinate*/
static int gc1084_sen_get_valid_coord_func(void* arg, int* x, int* y)
{
    *x = SENSOR_VALID_OFFSET_X;
    *y = SENSOR_VALID_OFFSET_Y;
    return 0;
}
/*get bus type*/
static enum sensor_bus_type gc1084_sen_get_bus_type_func(void* arg)
{
    return SENSOR_BUS_TYPE;
}
static int get_ae_fast_default(struct ae_fast_struct* ae_fast)
{
    *ae_fast = ae_fast_default;
    return 0;
}

/*get self definded params*/
static int gc1084_sen_get_parameter_func(void* arg, int param, void* value)
{
    int ret = 0;
    struct gc1084_priv* priv = arg;

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
            *((int *)value) = DUAL_SYNC_PWM_FREQ_FIXED;
            break;

        case GET_MAX_FPS:
            *((int *)value) = MAX_FPS;
            break;

        case GET_AE_FAST_DEFAULT:
            get_ae_fast_default(value);
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
/*sensor power on*/
static int gc1084_sen_set_power_on_func(void* arg)
{
    struct gc1084_priv* priv = arg;

    if (priv->gpio_pwdn >= 0) {
        gpio_direction_output(priv->gpio_pwdn, !SENSOR_PWDN_LEVEL);
    }

    if (priv->gpio_reset >= 0) {
        gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
        msleep(50);
        gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
        msleep(50);
        gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
        msleep(100);
    }

    return 0;
}
/*sensor power off*/
static int gc1084_sen_set_power_off_func(void* arg)
{
    struct gc1084_priv* priv = arg;

    if (priv->gpio_pwdn >= 0) {
        gpio_direction_output(priv->gpio_pwdn, SENSOR_PWDN_LEVEL);
    }

    if (priv->gpio_reset >= 0) {
        gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
    }

    return 0;
}

static int gc1084_fps_to_vts(struct gc1084_priv* priv, const int fps)
{
    unsigned int vts;
    unsigned int pclk;

    switch (fps) {
        case 12:
            vts = 1250;
            break;

        case 14:
            vts = 1428;
            break;

        default:
            vts = fps * 100;
            break;
    }

    if (priv->master_or_slave != SINGLE_MODE &&
            priv->master_or_slave != MASTER_MODE) {
            if (fps != MAX_FPS){
                vts = vts * 1000000 / (1000000 - vts*3);   //  从模式时，帧率要比PWM频率快一点
            }
        }

    pclk = priv->aec_parms.reg_frame_vts * MAX_FPS;

    vts = pclk * 100 / vts;

    return vts;
}

/*set sensor fps*/
static int gc1084_sen_set_fps_func(void* arg, const int fps)
{
    int tmp;
    struct gc1084_priv* priv = arg;

    tmp = gc1084_fps_to_vts(priv, fps);

    if (tmp > 0) {
        priv->fps_info.to_fps_value = tmp;
        priv->fps_info.to_fps = fps;
    } else {
        return -EINVAL;
    }

    return 0;
}

static void gc1084_set_fps_async(struct gc1084_priv* priv)
{
    struct i2c_client* client = priv->client;

    if (priv->fps_info.to_fps != priv->fps_info.current_fps) {
        int value = priv->fps_info.to_fps_value;

        gc1084_write(client, VTS_REG_H, (value >> 8) & 0xff);
        gc1084_write(client, VTS_REG_L, value & 0xff);
        priv->fps_info.current_fps = priv->fps_info.to_fps;
    }
}

static AK_ISP_SENSOR_CB gc1084_cb = {
    .sensor_init_func = gc1084_sen_init_func,
    .sensor_read_reg_func = gc1084_sen_read_reg_func,
    .sensor_write_reg_func = gc1084_sen_write_reg_func,
    .sensor_read_id_func = gc1084_sen_read_id_func,
    .sensor_update_a_gain_func = gc1084_sen_update_a_gain_func,
    .sensor_update_d_gain_func = gc1084_sen_update_d_gain_func,
    .sensor_updata_exp_time_func = gc1084_sen_updata_exp_time_func,
    .sensor_timer_func = gc1084_sen_timer_func,
    .sensor_set_standby_in_func = gc1084_sen_set_standby_in_func,
    .sensor_set_standby_out_func = gc1084_sen_set_standby_out_func,
    .sensor_probe_id_func = gc1084_sen_probe_id_func,
    .sensor_get_resolution_func = gc1084_sen_get_resolution_func,
    .sensor_get_mclk_func = gc1084_sen_get_mclk_func,
    .sensor_get_fps_func = gc1084_sen_get_fps_func,
    .sensor_get_valid_coordinate_func = gc1084_sen_get_valid_coord_func,
    .sensor_get_bus_type_func = gc1084_sen_get_bus_type_func,
    .sensor_get_parameter_func = gc1084_sen_get_parameter_func,
    .sensor_set_power_on_func = gc1084_sen_set_power_on_func,
    .sensor_set_power_off_func = gc1084_sen_set_power_off_func,
    .sensor_set_fps_func = gc1084_sen_set_fps_func,
};

static int sensor_id_func(void) { return gc1084_sen_read_id_func(NULL); }

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
