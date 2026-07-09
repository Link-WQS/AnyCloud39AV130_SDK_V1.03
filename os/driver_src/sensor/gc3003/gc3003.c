/*
 * gc3003 Camera Driver
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
#define SENSOR_PWDN_LEVEL  0
#define SENSOR_RESET_LEVEL 0
static int SENSOR_I2C_ADDR = 0x37; // or 0x3f
#define SENSOR_ID             0x3003
#define SENSOR_MCLK           24
#define SENSOR_REGADDR_BYTE   2
#define SENSOR_DATA_BYTE      1
#define SENSOR_OUTPUT_WIDTH   2304
#define SENSOR_OUTPUT_HEIGHT  1296
#define SENSOR_REAL_HEIGHT    1296 //sensor实际输出行数
#define SENSOR_VALID_OFFSET_X 0
#define SENSOR_VALID_OFFSET_Y 0
#define SENSOR_BUS_TYPE       BUS_TYPE_RAW
//#define SENSOR_IO_INTERFACE       DVP_INTERFACE
#define SENSOR_IO_INTERFACE MIPI_INTERFACE
#define SENSOR_IO_LEVEL     IO_LEVEL_1V8
/*
 * gc3003 at 15fps, file from IC fae said:
 *      mipiclk 324Mhz,
 * but it is not stable in the frequency.
 * so increase to 620Mhzbps
 */
#define MIPI_MBPS  620
#define MIPI_LANES 2
static int MAX_FPS = 25;
#if defined(CONFIG_MACH_AK39EV330) || defined(CONFIG_MACH_AK37D) /*H3B&H3D*/
#define EXP_EFFECT_FRAMES    1 /*adjust exp_time for 1/2 frames*/
#define A_GAIN_EFFECT_FRAMES 0 /*adjust a_gain for 1/2 frames*/
#define D_GAIN_EFFECT_FRAMES 0 /*adjust d_gain for 1/2 frames*/
#else /*H322&322L*/
#define EXP_EFFECT_FRAMES    1 /*adjust exp_time for 1/2 frames*/
#define A_GAIN_EFFECT_FRAMES 1 /*adjust a_gain for 1/2 frames*/
#define D_GAIN_EFFECT_FRAMES 1 /*adjust d_gain for 1/2 frames*/
#endif

#define DELAY_FLAG (0xffff)

/*
 * Struct
 */
struct regval_list {
    u8 reg_num;
    u8 value;
};

struct gc3003_win_size {
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
struct gc3003_aec_parms {
    int curr_again_level;
    int curr_again_10x;
    int r0x3e02_value;
    int r0x3e01_value;
    int r0x3e00_value;
    int curr_2x_dgain;
    int curr_corse_gain;
    int reg_frame_hts;
    int reg_frame_vts;
    int pclk_freq;
};

static const struct v4l2_ctrl_ops gc3003_ctrl_ops;
static const struct v4l2_ctrl_config config_sensor_get_id = {
    .ops = &gc3003_ctrl_ops,
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
struct gc3003_priv {
    struct list_head list;
    struct i2c_client* client;
    struct v4l2_subdev subdev;
    struct v4l2_ctrl_handler hdl;
    u32 cfmt_code;
    const struct gc3003_win_size* win;

    int gpio_reset;
    int gpio_pwdn;
    struct sensor_cb_info cb_info;
    struct sensor_fps_info fps_info;

    struct host_callbacks hcb;
    struct gc3003_aec_parms aec_parms;

    AK_ISP_SENSOR_INIT_PARA para;
    enum sensor_master_or_slave_mode master_or_slave;

    struct v4l2_ctrl* ctrl_sensor_get_id;
};

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
static LIST_HEAD(privs_list);

static AK_ISP_SENSOR_CB gc3003_cb;

/*
 * check_id - check hardware sensor ID whether it meets this driver
 * 0-no check, force to meet
 * others-check, if not meet return fail
 */
static int check_id = 0;

/*
 * addr0 - The addree of CIS which is connect with CSI0
 * addr1 - The addree of CIS which is connect with CSI1
 * sensor0_switch -CSI0 connects multiple CIS through MIPI_Switch chip
 * sensor1_switch -CSI1 connects multiple CIS through MIPI_Switch chip
 * for example:
 * Dual CSI
 * dual CSI  :   addr0=0xxx addr1=0xxx;
 * dual CSI + Switch:   addr0=0xxx addr1=0xxx  sensor0_switch=0;
 * dual CSI + dual Switch :    addr0=0xxx   addr1=0xxx
 *              sensor0_switch=1   sensor1_switch=1;
 * Single CSI
 * CSI +  Switch :  sensor0_switch=1;
 *             or sensor1_switch=1
 * CSI +  dual Switch :  addr0=0xxx   sensor1_switch=1;
 *                or addr1=0xxx sensor1_switch=1;
 */

static int addr0 = 0, addr1 = 0;
static int sensor0_switch = 0;
static int sensor1_switch = 0;
#define SENSOR_I2C_ADDR0 (0x3f)
#define SENSOR_I2C_ADDR1 (0x37)

module_param(check_id, int, 0644);
module_param(SENSOR_I2C_ADDR, int, 0644);
module_param(addr0, int, 0644);
module_param(addr1, int, 0644);
module_param(sensor0_switch, int, 0644);
module_param(sensor1_switch, int, 0644);
module_param(MAX_FPS, int, 0644);

static int gc3003_write(struct i2c_client* client, int reg, int value);
static int gc3003_read(const struct i2c_client* client, int reg);
static void gc3003_set_fps_async(struct gc3003_priv* priv);
static int __gc3003_sen_probe_id_func(
    struct i2c_client* client); // use IIC bus
static int gc3003_sen_read_id_func(void* arg); // no use IIC bus
static int gc3003_sen_get_resolution_func(
    void* arg, int* width, int* height);
static int gc3003_sen_set_fps_func(void* arg, const int fps);
static int gc3003_fps_to_vts(struct gc3003_priv* priv, const int fps);
static int call_sensor_sys_init(void);
static int call_sensor_sys_deinit(void);
static int gc3003_sen_probe_id_func(void* arg); // use IIC bus

static u32 gc3003_codes[] = {
    MEDIA_BUS_FMT_YUYV8_2X8,
    MEDIA_BUS_FMT_UYVY8_2X8,
    MEDIA_BUS_FMT_RGB565_2X8_BE,
    MEDIA_BUS_FMT_RGB565_2X8_LE,
};

/*
 * General functions
 */
static struct gc3003_priv* to_gc3003(const struct i2c_client* client)
{
    return container_of(i2c_get_clientdata(client), struct gc3003_priv, subdev);
}

static struct v4l2_subdev* ctrl_to_sd(struct v4l2_ctrl* ctrl)
{
    return &container_of(ctrl->handler, struct gc3003_priv, hdl)->subdev;
}

/*
 * XXXX_s_stream -
 * set steaming enable/disable
 * soc_camera_ops functions
 *
 * @sd:             subdev
 * @enable:         enable flags
 */
static int gc3003_s_stream(struct v4l2_subdev* sd, int enable) { return 0; }

static int gc3003_g_volatile_ctrl(struct v4l2_ctrl* ctrl)
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
static int gc3003_s_ctrl(struct v4l2_ctrl* ctrl) { return 0; }

/*
 * XXXX_s_power -
 * set power operation
 * soc_camera_ops functions
 *
 * @sd:         subdev
 * @on:         power flags
 */
static int gc3003_core_s_power(struct v4l2_subdev* sd, int on) { return 0; }

/*
 * XXXX_get_sensor_id -
 * get sensor ID
 * private callback functions
 *
 * @ctrl:           pointer to ctrl
 */
static int gc3003_get_sensor_id(struct v4l2_control* ctrl)
{
    ctrl->value = gc3003_sen_read_id_func(NULL); // no use IIC bus
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
static int gc3003_get_sensor_cb(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc3003_priv* priv = to_gc3003(client);

    ctrl->value = (int)&priv->cb_info;
    return 0;
}

static int gc3003_get_max_exp_for_fps(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc3003_priv* priv = to_gc3003(client);
    int fps = ctrl->value;
    int vts = gc3003_fps_to_vts(priv, fps);

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
static int gc3003_core_g_ctrl(struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    int ret;

    switch (ctrl->id) {
        case GET_SENSOR_ID:
            ret = gc3003_get_sensor_id(ctrl);
            break;

        case GET_SENSOR_CB:
            ret = gc3003_get_sensor_cb(sd, ctrl);
            break;

        case GET_MAX_EXP_FOR_FPS:
            ret = gc3003_get_max_exp_for_fps(sd, ctrl);
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
static int gc3003_set_isp_timing_cb(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc3003_priv* priv = to_gc3003(client);
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
static int gc3003_set_fps_direct(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc3003_priv* priv = to_gc3003(client);
    int fps = ctrl->value;

    gc3003_sen_set_fps_func(priv, fps);
    gc3003_set_fps_async(priv);
    return 0;
}

static int gc3003_set_master_or_slave(struct v4l2_subdev* sd, int is_master)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc3003_priv* priv = to_gc3003(client);

    pr_debug("%s is_master:%d\n", __func__, is_master);

    if (is_master)
        priv->master_or_slave = MASTER_MODE;
    else
        priv->master_or_slave = SLAVER_MODE;

    return 0;
}

static int gc3003_set_flip_mirror(struct v4l2_subdev* sd, int enable)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc3003_priv* priv = to_gc3003(client);
    int flip_en = enable & (0x1 << FLIP_OFFSET);
    int mirror_en = enable & (0x1 << MIRROR_OFFSET);
    int reg0x0015_value = 0;
    int reg0x0d15_value = 0;

    if (flip_en && mirror_en) {
        reg0x0015_value = 0x03;
        reg0x0d15_value = 0x02;
    } else if (flip_en && !mirror_en) {
        reg0x0015_value = 0x02;
        reg0x0d15_value = 0x02;
    } else if (!flip_en && mirror_en) {
        reg0x0015_value = 0x01;
        reg0x0d15_value = 0x00;
    } else {
        reg0x0015_value = 0x00;
        reg0x0d15_value = 0x00;
    }

    gc3003_write(client, (0x0015), reg0x0015_value);
    gc3003_write(client, (0x0d15), reg0x0d15_value);

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
static int gc3003_core_s_ctrl(struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    int ret;

    switch (ctrl->id) {
        case SET_ISP_MISC_CALLBACK:
            ret = gc3003_set_isp_timing_cb(sd, ctrl);
            break;

        case SET_FPS_DIRECT:
            ret = gc3003_set_fps_direct(sd, ctrl);
            break;

        case SET_MASTER:
            ret = gc3003_set_master_or_slave(sd, 1);
            break;

        case SET_SLAVER: /*for dual sensor*/
            ret = gc3003_set_master_or_slave(sd, 0);
            break;

        case SET_FLIP_MIRROR:
            ret = gc3003_set_flip_mirror(sd, ctrl->value);
            break;

        default:
            pr_err("%s cmd:%d not support\n", __func__, ctrl->id);
            ret = -1;
            break;
    }

    return ret;
}

static int gc3003_store_initial_regs(struct gc3003_priv* priv, void* arg)
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

static long gc3003_core_ioctl(
    struct v4l2_subdev* sd, unsigned int cmd, void* arg)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc3003_priv* priv = to_gc3003(client);
    int ret = 0;

    pr_debug("%s\n", __func__);

    switch (cmd) {
        case AK_SENSOR_SET_INIT:
            gc3003_store_initial_regs(priv, arg);
            break;

        case AK_SENSOR_GET_MAX_EXP_FOR_FPS: {
            struct sensor_max_exp_for_fps* exp_for_fps = arg;
            struct v4l2_control ctrl;

            ctrl.value = exp_for_fps->fps;
            ret = gc3003_get_max_exp_for_fps(sd, &ctrl);
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
static int gc3003_get_fmt(struct v4l2_subdev* sd,
    struct v4l2_subdev_pad_config* cfg, struct v4l2_subdev_format* format)
{
    struct v4l2_mbus_framefmt* mf = &format->format;
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc3003_priv* priv = to_gc3003(client);
    static struct gc3003_win_size win = {
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
static int gc3003_set_fmt(struct v4l2_subdev* sd,
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
static int gc3003_get_selection(struct v4l2_subdev* sd,
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
static int gc3003_enum_mbus_code(struct v4l2_subdev* sd,
    struct v4l2_subdev_pad_config* cfg, struct v4l2_subdev_mbus_code_enum* code)
{
    if (code->pad || code->index >= ARRAY_SIZE(gc3003_codes))
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
static int gc3003_g_crop(struct v4l2_subdev* sd, struct v4l2_crop* a)
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
static int gc3003_cropcap(struct v4l2_subdev* sd, struct v4l2_cropcap* a)
{
    gc3003_sen_get_resolution_func(
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
static int gc3003_video_probe(struct i2c_client* client)
{
    struct gc3003_priv* priv = to_gc3003(client);
    int ret;

    ret = v4l2_ctrl_handler_setup(&priv->hdl);

    return ret;
}

/*ctrl ops*/
static const struct v4l2_ctrl_ops gc3003_ctrl_ops = {
    .g_volatile_ctrl = gc3003_g_volatile_ctrl,
    .s_ctrl = gc3003_s_ctrl,
};

/*core ops*/
static struct v4l2_subdev_core_ops gc3003_subdev_core_ops = {
    .s_power = gc3003_core_s_power,
    .g_ctrl = gc3003_core_g_ctrl,
    .s_ctrl = gc3003_core_s_ctrl,
    .ioctl = gc3003_core_ioctl,
};

/*
 * XXXX_g_mbus_config -
 * get buf config
 * video functions
 *
 * @sd:             subdev
 * @cfg:            return config
 */
static int gc3003_g_mbus_config(
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
static int gc3003_s_crop(struct v4l2_subdev* sd, const struct v4l2_crop* crop)
{
    printk(KERN_ERR "%s %d, left:%d, top:%d, width:%d, height:%d\n", __func__,
        __LINE__, crop->c.left, crop->c.top, crop->c.width, crop->c.height);
    return 0;
}

/*video ops*/
static struct v4l2_subdev_video_ops gc3003_subdev_video_ops = {
    .s_stream = gc3003_s_stream,
    .cropcap = gc3003_cropcap,
    .g_crop = gc3003_g_crop,
    .g_mbus_config = gc3003_g_mbus_config,
    .s_crop = gc3003_s_crop,
};

/*pad ops*/
static const struct v4l2_subdev_pad_ops gc3003_subdev_pad_ops = {
    .enum_mbus_code = gc3003_enum_mbus_code,
    .get_fmt = gc3003_get_fmt,
    .set_fmt = gc3003_set_fmt,
    .get_selection = gc3003_get_selection,
};

/*sensor driver subdev ops*/
static struct v4l2_subdev_ops gc3003_subdev_ops = {
    .core = &gc3003_subdev_core_ops,
    .video = &gc3003_subdev_video_ops,
    .pad = &gc3003_subdev_pad_ops,
};

/*
 * sensor_of_parse -
 * parse node of device
 *
 * @client:         pointor to i2c client
 * @priv:           sensor struct
 */
static int sensor_of_parse(struct i2c_client* client, struct gc3003_priv* priv)
{
    struct device_node* np = client->dev.of_node;

    /*it is exist reset but lack of pwdn gpio in most case*/
    priv->gpio_reset = of_get_named_gpio(np, "reset-gpio", 0);
    priv->gpio_pwdn = of_get_named_gpio(np, "pwdn-gpio", 0);

    if (priv->gpio_reset >= 0) {
        devm_gpio_request(&client->dev, priv->gpio_reset, "sensor-reset");
    }

    if (priv->gpio_pwdn >= 0) {
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
static int gc3003_match(struct gc3003_priv* priv)
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
    struct gc3003_priv* t_priv = NULL;
    struct i2c_client* client;

    list_for_each_entry(t_priv, &privs_list, list)
    {
        if (!t_priv->subdev.devnode)
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
static int gc3003_probe(
    struct i2c_client* client, const struct i2c_device_id* did)
{
    struct gc3003_priv* priv;
    struct i2c_adapter* adapter = to_i2c_adapter(client->dev.parent);
    int ret;

    pr_err("%s %s %s\n", __func__, __DATE__, __TIME__);

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_err(&adapter->dev, "gc3003: I2C-Adapter doesn't support SMBUS\n");
        return -EIO;
    }

    priv = devm_kzalloc(&client->dev, sizeof(struct gc3003_priv), GFP_KERNEL);
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
    priv->cb_info.cb = &gc3003_cb;
    priv->cb_info.arg = priv;
    priv->client = client;

    if (!client->dev.of_node) {
        dev_err(&client->dev, "Missing platform_data for driver\n");
        ret = -EINVAL;
        goto err_clk;
    }

    if (!gc3003_match(priv)) {
        ret = -ENODEV;
        goto err_clk;
    }

    /*subdev init*/
    v4l2_i2c_subdev_init(&priv->subdev, client, &gc3003_subdev_ops);
    priv->subdev.flags
        |= /*V4L2_SUBDEV_FL_HAS_EVENTS | */ V4L2_SUBDEV_FL_HAS_DEVNODE;
    v4l2_ctrl_handler_init(&priv->hdl, 2);
    v4l2_ctrl_new_std(&priv->hdl, &gc3003_ctrl_ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
    v4l2_ctrl_new_std(&priv->hdl, &gc3003_ctrl_ops, V4L2_CID_HFLIP, 0, 1, 1, 0);

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

    ret = gc3003_video_probe(client);
    if (ret < 0) {
        dev_err(&client->dev, "OV2640 probe fail\n");
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

    priv->master_or_slave = SINGLE_MODE;

    dev_err(&adapter->dev, "gc3003 Probed success, subdev:%p\n", &priv->subdev);

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
static int gc3003_remove(struct i2c_client* client)
{
    struct gc3003_priv* priv = to_gc3003(client);

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

static const struct i2c_device_id gc3003_id[] = { { "gc3003", 0 }, {} };
MODULE_DEVICE_TABLE(i2c, gc3003_id);

static const struct of_device_id gc3003_of_match[] = {
    /*donot changed compatible, must the same as dts*/
    {
        .compatible = "anyka,sensor0",
    },
    {},
};
MODULE_DEVICE_TABLE(of, gc3003_of_match);

static struct i2c_driver gc3003_i2c_driver = {
    .driver = {
        .name = "gc3003",
        .of_match_table = of_match_ptr(gc3003_of_match),
    },
    .probe    = gc3003_probe,
    .remove   = gc3003_remove,
    .id_table = gc3003_id,
};

module_i2c_driver(gc3003_i2c_driver);

MODULE_DESCRIPTION("SoC Camera driver for gc3003 sensor");
MODULE_AUTHOR("anyka");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0.09");

/*sensor i2c write*/
static int __gc3003_write(const struct i2c_client* client, int reg, int value)
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

static int _gc3003_write(
    struct i2c_client* client, int reg, int value, int sensor_switch)
{
    struct i2c_client* client_p = client;
    unsigned short tmp_addr = client_p->addr;
    int ret = 0;

    if (sensor_switch == 1) {
        client_p->addr = SENSOR_I2C_ADDR0;
        __gc3003_write(client_p, reg, value);
        client_p->addr = SENSOR_I2C_ADDR1;
        ret = __gc3003_write(client_p, reg, value);
        client_p->addr = tmp_addr;
        return ret;
    } else {
        return __gc3003_write(client_p, reg, value);
    }
}

static int gc3003_write_cisx(struct gc3003_priv* t_priv, int reg, int value)
{
    switch (t_priv->subdev.devnode->num) {
        case 0:
            return _gc3003_write(t_priv->client, reg, value, sensor0_switch);
            break;

        case 1:
            return _gc3003_write(t_priv->client, reg, value, sensor1_switch);
            break;

        default:
            break;
    }
    return 0;
}

static int gc3003_write(struct i2c_client* client, int reg, int value)
{
    struct gc3003_priv* t_priv = NULL;
    struct i2c_client* client_p;

    list_for_each_entry(t_priv, &privs_list, list)
    {
        if (!t_priv || !t_priv->subdev.devnode)
            continue;

        if (t_priv->master_or_slave == SLAVER_MODE) {
            // Dual CSI
            if (client == t_priv->client) {
                return gc3003_write_cisx(t_priv, reg, value);
            }
        } else {
            // single CSI
            // when use Two_way_I2C + MIPI switch need write register twice.
            gc3003_write_cisx(t_priv, reg, value);
        }
    }
    return 0;
}

/*sensor i2c read*/
static int gc3003_read(const struct i2c_client* client, int reg)
{
    struct i2c_transfer_struct trans = {
        .client = client,
        .reg_bytes = SENSOR_REGADDR_BYTE,
        .value_bytes = SENSOR_DATA_BYTE,
        .reg = reg,
        .value = 0,
    };

    pr_err("%s client:%p, addr:%x\n", __func__, client, client->addr);
    return i2c_read(&trans);
}

/*
 * The sensor must set callbacks
 *
 * */
static int gc3003_sen_init_func(
    void* arg, const AK_ISP_SENSOR_INIT_PARA* npara)
{
    int i;
    int value;
    AK_ISP_SENSOR_REG_INFO* preg_info;
    struct gc3003_priv* priv = arg;
    struct i2c_client* client = priv->client;
    AK_ISP_SENSOR_INIT_PARA* para = &priv->para;

    gc3003_sen_probe_id_func(arg); // use IIC bus

    if (para->num <= 0 && npara)
        para = (void*)npara;

    preg_info = para->reg_info;
    for (i = 0; i < para->num; i++) {
#if 0
        {
            int value;

            value = gc3003_read(client, preg_info->reg_addr);
            pr_err("reg:%x read value:%x\n", preg_info->reg_addr, value);
        }
#endif
        gc3003_write(client, preg_info->reg_addr, preg_info->value);
#if 0
        {
            int value;

            value = gc3003_read(client, preg_info->reg_addr);
            pr_err("reg:%x write value:%x, read back value:%x\n",
                    preg_info->reg_addr, preg_info->value, value);
        }
#endif

        preg_info++;
    }
    priv->aec_parms.reg_frame_vts = ((gc3003_read(client, 0x0d41) & 0x3f) << 8
        | gc3003_read(client, 0x0d42));
    priv->aec_parms.reg_frame_hts = ((gc3003_read(client, 0x0d05) & 0x3f) << 8
        | gc3003_read(client, 0x0d06))  * 2;

    switch (priv->aec_parms.reg_frame_hts) {
        case  3224:
            MAX_FPS = 25;
            break;
        case  2688:
            MAX_FPS = 30;
            break;
        default:
            MAX_FPS = 25;
            break;
    }

    priv->aec_parms.pclk_freq = priv->aec_parms.reg_frame_hts
        * priv->aec_parms.reg_frame_vts * MAX_FPS;
    pr_info("%s hts %d vts %d pclk %d,MAX_FPS %d\n", __func__,
        priv->aec_parms.reg_frame_hts, priv->aec_parms.reg_frame_vts,
        priv->aec_parms.pclk_freq,MAX_FPS);

    priv->fps_info.current_fps = MAX_FPS;
    priv->fps_info.to_fps = priv->fps_info.current_fps;
    priv->fps_info.to_fps_value = 0;

    return 0;
}

/*read sensor register*/
static int gc3003_sen_read_reg_func(void* arg, const int reg_addr)
{
    struct gc3003_priv* priv = arg;
    struct i2c_client* client = priv->client;

    return gc3003_read(client, reg_addr);
}
/*write sensor register*/
static int gc3003_sen_write_reg_func(
    void* arg, const int reg_addr, int value)
{
    struct gc3003_priv* priv = arg;
    struct i2c_client* client = priv->client;

    return gc3003_write(client, reg_addr, value);
}
/*read sensor register, NO i2c ops*/
static int gc3003_sen_read_id_func(void* arg) // no use IIC bus
{
    return SENSOR_ID;
}

/*
 * from gc fae
 * */
static unsigned int regValTable[25][6] = {
    // 0x00d1   0x00d0 0x00b8 0x00b9 0x0155 0x80
    { 0x00, 0x00, 0x01, 0x00, 0x04, 0x09 },
    { 0x0A, 0x00, 0x01, 0x0c, 0x04, 0x0b },
    { 0x00, 0x01, 0x01, 0x1a, 0x04, 0x0d },
    { 0x0A, 0x01, 0x01, 0x2a, 0x04, 0x0e },
    { 0x20, 0x00, 0x02, 0x00, 0x04, 0x10 },
    { 0x25, 0x00, 0x02, 0x18, 0x04, 0x11 },
    { 0x20, 0x01, 0x02, 0x33, 0x04, 0x12 },
    { 0x25, 0x01, 0x03, 0x14, 0x04, 0x14 },
    { 0x30, 0x00, 0x04, 0x00, 0x04, 0x15 },
    { 0x32, 0x80, 0x04, 0x2f, 0x04, 0x16 },
    { 0x30, 0x01, 0x05, 0x26, 0x04, 0x18 },
    { 0x32, 0x81, 0x06, 0x29, 0x04, 0x19 },
    { 0x38, 0x00, 0x08, 0x00, 0x04, 0x1a },
    { 0x39, 0x40, 0x09, 0x1f, 0x06, 0x1c },
    { 0x38, 0x01, 0x0b, 0x0d, 0x06, 0x1e },
    { 0x39, 0x41, 0x0d, 0x12, 0x06, 0x20 },
    { 0x30, 0x08, 0x10, 0x00, 0x06, 0x24 },
    { 0x32, 0x88, 0x12, 0x3e, 0x06, 0x26 },
    { 0x30, 0x09, 0x16, 0x1a, 0x06, 0x2a },
    { 0x32, 0x89, 0x1a, 0x23, 0x06, 0x2d },
    { 0x38, 0x08, 0x20, 0x00, 0x06, 0x31 },
    { 0x39, 0x48, 0x25, 0x3b, 0x09, 0x36 },
    { 0x38, 0x09, 0x2c, 0x33, 0x0b, 0x3a },
    { 0x39, 0x49, 0x35, 0x06, 0x0d, 0x3c },
    { 0x38, 0x0A, 0x3f, 0x3f, 0x0f, 0x3f },
};

static unsigned int analog_gain_table[26] = {
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
static int SetSensor_Gain(struct i2c_client* client, unsigned int gain)
{
    unsigned char i;
    unsigned char total;
    unsigned int tol_dig_gain = 0;
    unsigned int tmp_gain = gain / 4;

    total = sizeof(analog_gain_table) / sizeof(analog_gain_table[0]);
    for (i = 0; i < total - 1 - 1; i++) {
        if ((analog_gain_table[i] <= tmp_gain)
            && (tmp_gain < analog_gain_table[i + 1]))
            break;
    }

    tol_dig_gain = tmp_gain * 64 / analog_gain_table[i];

    gc3003_write(client, 0x00d1, regValTable[i][0]);
    gc3003_write(client, 0x00d0, regValTable[i][1]);
    gc3003_write(client, 0x00b8, regValTable[i][2]);
    gc3003_write(client, 0x00b9, regValTable[i][3]);
    gc3003_write(client, 0x0155, regValTable[i][4]);
    gc3003_write(client, 0x0080, regValTable[i][5]);

    gc3003_write(client, 0x00b1, (tol_dig_gain >> 6));
    gc3003_write(client, 0x00b2, ((tol_dig_gain & 0x3f) << 2));

    return 0;
}

/*************************************************************************
 * * FUNCTION
 * *    set_shutter
 * *
 * * DESCRIPTION
 * *    This function set e-shutter of sensor to change exposure time.
 * *
 * * PARAMETERS
 * *    iShutter : exposured lines
 * *
 * * RETURNS
 * *    None
 * *
 * * GLOBALS AFFECTED
 * *
 * *************************************************************************/
static int SetSensor_shutter(struct i2c_client* client, unsigned int intt)
{
    /*
       reg_gc3003_wr(0x0d03,(intt>>8));
       reg_gc3003_wr(0x0d04,(intt&0xff));
    */

    gc3003_write(client, 0x0d03, (intt >> 8));
    gc3003_write(client, 0x0d04, (intt & 0xff));

    return 0;
}

/*set sensor again*/
/*max gain: 4853*/
static int gc3003_sen_update_a_gain_func(
    void* arg, const unsigned int a_gain)
{
    struct gc3003_priv* priv = arg;
    struct i2c_client* client = priv->client;

    SetSensor_Gain(client, a_gain);

    return A_GAIN_EFFECT_FRAMES;
}
/*set sensor dgain*/
static int gc3003_sen_update_d_gain_func(
    void* arg, const unsigned int d_gain)
{
    return D_GAIN_EFFECT_FRAMES;
}
/*set sensor exp time*/
/*max exp: 0x3fff*/
static int gc3003_sen_updata_exp_time_func(void* arg, unsigned int exp_time)
{
    struct gc3003_priv* priv = arg;
    struct i2c_client* client = priv->client;

    /*
     * max_exp_time = frame_length = R{0x340,0x341}
     * */
    SetSensor_shutter(client, exp_time);

    return EXP_EFFECT_FRAMES;
}
/*sensor timer*/
static int gc3003_sen_timer_func(void* arg) { return 0; }
/*standby in*/
static int gc3003_sen_set_standby_in_func(void* arg) { return 0; }
/*standby out*/
static int gc3003_sen_set_standby_out_func(void* arg) { return 0; }
/*low level read sensor ID, user i2c ops*/
static int gc3003_sen_probe_id_func(void* arg) // use IIC bus
{
    struct gc3003_priv* priv = arg;
    struct i2c_client* client = priv->client;
    int id;
    int value;

    value = gc3003_read(client, 0x03f0);
    id = value << 8;

    value = gc3003_read(client, 0x03f1);
    id |= value;

    pr_err("%s gc3003_id:0x%x\n", __func__, id);

    if (id == SENSOR_ID)
        return SENSOR_ID;

    return 0;
}
/*get resolution*/
static int gc3003_sen_get_resolution_func(void* arg, int* width, int* height)
{
    *width = SENSOR_OUTPUT_WIDTH;
    *height = SENSOR_OUTPUT_HEIGHT;
    return 0;
}
/*get mclk*/
static int gc3003_sen_get_mclk_func(void* arg) { return SENSOR_MCLK; }
/*get current fps*/
static int gc3003_sen_get_fps_func(void* arg)
{
    struct gc3003_priv* priv = arg;

    return priv->fps_info.current_fps;
}
/*get valid coordinate*/
static int gc3003_sen_get_valid_coord_func(void* arg, int* x, int* y)
{
    *x = SENSOR_VALID_OFFSET_X;
    *y = SENSOR_VALID_OFFSET_Y;
    return 0;
}
/*get bus type*/
static enum sensor_bus_type gc3003_sen_get_bus_type_func(void* arg)
{
    return SENSOR_BUS_TYPE;
}
static int get_ae_fast_default(struct ae_fast_struct* ae_fast)
{
    *ae_fast = ae_fast_default;
    return 0;
}

/*get self definded params*/
static int gc3003_sen_get_parameter_func(void* arg, int param, void* value)
{
    int ret = 0;
    struct gc3003_priv* priv = arg;

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
            *((int*)value) = DUAL_SYNC_PWM_FREQ_FIXED;
            break;

        case GET_MAX_FPS:
            *((int*)value) = MAX_FPS;
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
static int gc3003_sen_set_power_on_func(void* arg)
{
    struct gc3003_priv* priv = arg;

    pr_err("%s %d\n", __func__, __LINE__);

    if (priv->gpio_pwdn >= 0) {
        gpio_direction_output(priv->gpio_pwdn, !SENSOR_PWDN_LEVEL);
    }

    if (priv->gpio_reset >= 0) {
        pr_err("%s %d\n", __func__, __LINE__);
        gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
        msleep(50);
        gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
        msleep(50);
        gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
        msleep(100);
    }

    // aec_parms_init(priv);

    return 0;
}
/*sensor power off*/
static int gc3003_sen_set_power_off_func(void* arg)
{
    struct gc3003_priv* priv = arg;

    if (priv->gpio_pwdn >= 0) {
        gpio_direction_output(priv->gpio_pwdn, SENSOR_PWDN_LEVEL);
    }

    if (priv->gpio_reset >= 0) {
        gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
    }

    return 0;
}

static int gc3003_fps_to_vts(struct gc3003_priv* priv, const int fps)
{
    /*
     * max_exp_time = frame_length = R{0xd41,0xd42}
     * */

    int vts;
    int fps_tmp = 0;
    unsigned int csi_pwm_freq = fps * 100;
    pr_debug("%s\n", __func__);

    if (fps > MAX_FPS) {
        pr_err(
            "%s MAX_FPS:%d, so set to %d fps fail\n", __func__, MAX_FPS, fps);
        return -EINVAL;
    }

    if (!priv->aec_parms.reg_frame_hts) {
        return 0;
    }

    if (priv->master_or_slave == SINGLE_MODE
        || priv->master_or_slave == MASTER_MODE) {

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

        vts = priv->aec_parms.pclk_freq / priv->aec_parms.reg_frame_hts;
        vts = vts * 100;
        vts = vts / fps_tmp;
        pr_err("vts1 %d\n", vts);
    } else { // SLAVER_MODE
        /*
         * release_v0.5_GC3003_2304x1296_24M_25fps_2lane_20220916.txt
         * 双目这个序列的帧率不准确，最大帧率没达到25fps
         */
        switch (fps) {
            case 12:
                csi_pwm_freq = 1250;
                break;

            case 14:
                csi_pwm_freq = 1428;
                break;

            default:
                csi_pwm_freq = fps * 100;
                break;
        }

        if (fps == MAX_FPS)
            fps_tmp = csi_pwm_freq;
        else
            fps_tmp = csi_pwm_freq * 1000000 / (1000000 - csi_pwm_freq * 2);

        vts = priv->aec_parms.pclk_freq / priv->aec_parms.reg_frame_hts;
        vts = vts * 100;
        vts = vts / fps_tmp;
    }

    return vts;
}

/*set sensor fps*/
static int gc3003_sen_set_fps_func(void* arg, const int fps)
{
    int tmp;
    struct gc3003_priv* priv = arg;

    tmp = gc3003_fps_to_vts(priv, fps);

    if (tmp > 0) {
        priv->fps_info.to_fps_value = tmp;
        priv->fps_info.to_fps = fps;
    } else {
        return -EINVAL;
    }

    return 0;
}

static void gc3003_set_fps_async(struct gc3003_priv* priv)
{
    struct i2c_client* client = priv->client;

    if (priv->fps_info.to_fps != priv->fps_info.current_fps) {
        int value = priv->fps_info.to_fps_value;

        gc3003_write(client, 0x0d41, (value >> 8) & 0x3f);
        gc3003_write(client, 0x0d42, value & 0xff);
        priv->fps_info.current_fps = priv->fps_info.to_fps;
    }
}

static AK_ISP_SENSOR_CB gc3003_cb = {
    .sensor_init_func = gc3003_sen_init_func,
    .sensor_read_reg_func = gc3003_sen_read_reg_func,
    .sensor_write_reg_func = gc3003_sen_write_reg_func,
    .sensor_read_id_func = gc3003_sen_read_id_func,
    .sensor_update_a_gain_func = gc3003_sen_update_a_gain_func,
    .sensor_update_d_gain_func = gc3003_sen_update_d_gain_func,
    .sensor_updata_exp_time_func = gc3003_sen_updata_exp_time_func,
    .sensor_timer_func = gc3003_sen_timer_func,
    .sensor_set_standby_in_func = gc3003_sen_set_standby_in_func,
    .sensor_set_standby_out_func = gc3003_sen_set_standby_out_func,
    .sensor_probe_id_func = gc3003_sen_probe_id_func,
    .sensor_get_resolution_func = gc3003_sen_get_resolution_func,
    .sensor_get_mclk_func = gc3003_sen_get_mclk_func,
    .sensor_get_fps_func = gc3003_sen_get_fps_func,
    .sensor_get_valid_coordinate_func = gc3003_sen_get_valid_coord_func,
    .sensor_get_bus_type_func = gc3003_sen_get_bus_type_func,
    .sensor_get_parameter_func = gc3003_sen_get_parameter_func,
    .sensor_set_power_on_func = gc3003_sen_set_power_on_func,
    .sensor_set_power_off_func = gc3003_sen_set_power_off_func,
    .sensor_set_fps_func = gc3003_sen_set_fps_func,
};

static int sensor_id_func(void) { return gc3003_sen_read_id_func(NULL); }

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
