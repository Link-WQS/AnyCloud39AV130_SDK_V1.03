/*
 * gc2063 Camera Driver
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
#include <linux/of.h>
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
static int SENSOR_I2C_ADDR = 0x3f;
#define SENSOR_ID             0x2063
#define SENSOR_MCLK           24
#define SENSOR_REGADDR_BYTE   1
#define SENSOR_DATA_BYTE      1
#define SENSOR_OUTPUT_WIDTH   1920
#define SENSOR_OUTPUT_HEIGHT  1080
#define SENSOR_REAL_HEIGHT    1080 //sensor实际输出行数
#define SENSOR_VALID_OFFSET_X 0
#define SENSOR_VALID_OFFSET_Y 0
#define SENSOR_BUS_TYPE       BUS_TYPE_RAW
//#define SENSOR_IO_INTERFACE       DVP_INTERFACE
static int SENSOR_IO_INTERFACE = MIPI_INTERFACE;
#define SENSOR_IO_LEVEL IO_LEVEL_1V8
#define MIPI_MBPS       594
#define MIPI_LANES      2
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

#define DELAY_FLAG (0xffff)
#define EXP_LIMITE (8)

/*
 * Struct
 */
struct regval_list {
    u8 reg_num;
    u8 value;
};

struct gc2063_win_size {
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
struct gc2063_aec_parms {
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
    int calc_vts_tmp;
};

static const struct v4l2_ctrl_ops gc2063_ctrl_ops;
static const struct v4l2_ctrl_config config_sensor_get_id = {
    .ops = &gc2063_ctrl_ops,
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
struct gc2063_priv {
    struct list_head list;
    struct i2c_client* client;
    struct v4l2_subdev subdev;
    struct v4l2_ctrl_handler hdl;
    u32 cfmt_code;
    const struct gc2063_win_size* win;

    int gpio_reset;
    int gpio_pwdn;
    struct sensor_cb_info cb_info;
    struct sensor_fps_info fps_info;

    struct host_callbacks hcb;
    struct gc2063_aec_parms aec_parms;

    AK_ISP_SENSOR_INIT_PARA para;

    struct v4l2_ctrl* ctrl_sensor_get_id;

    enum sensor_master_or_slave_mode master_or_slave;
    struct dual_sensor_attr dual;
    u32 sensor_sys_ignore;
};

static LIST_HEAD(privs_list);

static AK_ISP_SENSOR_CB gc2063_cb;

/*
 * check_id - check hardware sensor ID whether it meets this driver
 * 0-no check, force to meet
 * others-check, if not meet return fail
 */
static int check_id = 0;
static int dvp = 0;

/*
 * sensor_probe_once - with the purpose of using different CIS
 *                     to implement dual mode
 * '1' :when this global variable was set '1', the second probe will fail.
 *
 * Combination with the dts, when the dts description is as follow :
 * sensor0: camera@0x30{
 *    compatible = "anyka,sensorA"
 *              or "anyka,sensorB"
 *     ...
 * }
 * It means using different CIS to implement dual mode.
 * Driver only need to probe successfully once.
 */
static u32 sensor_probe_once = 0;
static const struct of_device_id gc2063_of_match[4];

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

static void gc2063_set_fps_async(struct gc2063_priv* priv);
static int __gc2063_sen_probe_id_func(
    struct i2c_client* client); // use IIC bus
static int gc2063_sen_read_id_func(void* arg); // no use IIC bus
static int gc2063_sen_get_resolution_func(
    void* arg, int* width, int* height);
static int gc2063_sen_set_fps_func(void* arg, const int fps);
static int gc2063_fps_to_vts(struct gc2063_priv* priv, const int fps);
static int call_sensor_sys_init(void);
static int call_sensor_sys_deinit(void);
static int gc2063_sen_probe_id_func(void* arg); // use IIC bus
static int gc2063_write(struct i2c_client* client, int reg, int value);

static u32 gc2063_codes[] = {
    MEDIA_BUS_FMT_YUYV8_2X8,
    MEDIA_BUS_FMT_UYVY8_2X8,
    MEDIA_BUS_FMT_RGB565_2X8_BE,
    MEDIA_BUS_FMT_RGB565_2X8_LE,
};

/*
 * General functions
 */
static struct gc2063_priv* to_gc2063(const struct i2c_client* client)
{
    return container_of(i2c_get_clientdata(client), struct gc2063_priv, subdev);
}

static struct v4l2_subdev* ctrl_to_sd(struct v4l2_ctrl* ctrl)
{
    return &container_of(ctrl->handler, struct gc2063_priv, hdl)->subdev;
}

/*
 * XXXX_s_stream -
 * set steaming enable/disable
 * soc_camera_ops functions
 *
 * @sd:             subdev
 * @enable:         enable flags
 */
static int gc2063_s_stream(struct v4l2_subdev* sd, int enable) { return 0; }

static int gc2063_g_volatile_ctrl(struct v4l2_ctrl* ctrl)
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
static int gc2063_s_ctrl(struct v4l2_ctrl* ctrl) { return 0; }

/*
 * XXXX_s_power -
 * set power operation
 * soc_camera_ops functions
 *
 * @sd:         subdev
 * @on:         power flags
 */
static int gc2063_core_s_power(struct v4l2_subdev* sd, int on) { return 0; }

/*
 * XXXX_get_sensor_id -
 * get sensor ID
 * private callback functions
 *
 * @ctrl:           pointer to ctrl
 */
static int gc2063_get_sensor_id(struct v4l2_control* ctrl)
{
    ctrl->value = gc2063_sen_read_id_func(NULL); // no use IIC bus
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
static int gc2063_get_sensor_cb(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc2063_priv* priv = to_gc2063(client);

    ctrl->value = (int)&priv->cb_info;
    return 0;
}

static int gc2063_get_max_exp_for_fps(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc2063_priv* priv = to_gc2063(client);
    int fps = ctrl->value;
    int vts = gc2063_fps_to_vts(priv, fps);

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
static int gc2063_core_g_ctrl(struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    int ret;

    switch (ctrl->id) {
        case GET_SENSOR_ID:
            ret = gc2063_get_sensor_id(ctrl);
            break;

        case GET_SENSOR_CB:
            ret = gc2063_get_sensor_cb(sd, ctrl);
            break;

        case GET_MAX_EXP_FOR_FPS:
            ret = gc2063_get_max_exp_for_fps(sd, ctrl);
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
static int gc2063_set_isp_timing_cb(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc2063_priv* priv = to_gc2063(client);
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
static int gc2063_set_fps_direct(
    struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc2063_priv* priv = to_gc2063(client);
    int fps = ctrl->value;

    gc2063_sen_set_fps_func(priv, fps);
    gc2063_set_fps_async(priv);
    return 0;
}

static int gc2063_set_master_or_slave(struct v4l2_subdev* sd, int is_master)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc2063_priv* priv = to_gc2063(client);

    pr_debug("%s is_master:%d\n", __func__, is_master);

    if (is_master)
        priv->master_or_slave = MASTER_MODE;
    else
        priv->master_or_slave = SLAVER_MODE;

    return 0;
}

static int gc2063_set_flip_mirror(struct v4l2_subdev* sd, int enable)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc2063_priv* priv = to_gc2063(client);
    int value = 0;
    int flip_en = enable & (0x1 << FLIP_OFFSET);
    int mirror_en = enable & (0x1 << MIRROR_OFFSET);

    if (flip_en)
        value |= 0x1 << 1;
    else
        value &= ~(0x1 << 1);

    if (mirror_en)
        value |= 0x1 << 0;
    else
        value &= ~(0x1 << 0);

    gc2063_write(client, 0x17, value);

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
static int gc2063_core_s_ctrl(struct v4l2_subdev* sd, struct v4l2_control* ctrl)
{
    int ret;

    switch (ctrl->id) {
        case SET_ISP_MISC_CALLBACK:
            ret = gc2063_set_isp_timing_cb(sd, ctrl);
            break;

        case SET_FPS_DIRECT:
            ret = gc2063_set_fps_direct(sd, ctrl);
            break;

        case SET_MASTER: /*for dual sensor*/
            ret = gc2063_set_master_or_slave(sd, 1);
            break;

        case SET_SLAVER: /*for dual sensor*/
            ret = gc2063_set_master_or_slave(sd, 0);
            break;

        case SET_FLIP_MIRROR:
            ret = gc2063_set_flip_mirror(sd, ctrl->value);
            break;

        default:
            pr_err("%s cmd:%d not support\n", __func__, ctrl->id);
            ret = -1;
            break;
    }

    return ret;
}

static int gc2063_store_initial_regs(struct gc2063_priv* priv, void* arg)
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

static long gc2063_core_ioctl(
    struct v4l2_subdev* sd, unsigned int cmd, void* arg)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc2063_priv* priv = to_gc2063(client);
    int ret = 0;

    pr_debug("%s\n", __func__);

    switch (cmd) {
        case AK_SENSOR_SET_INIT:
            gc2063_store_initial_regs(priv, arg);
            break;

        case AK_SENSOR_GET_MAX_EXP_FOR_FPS: {
            struct sensor_max_exp_for_fps* exp_for_fps = arg;
            struct v4l2_control ctrl;

            ctrl.value = exp_for_fps->fps;
            ret = gc2063_get_max_exp_for_fps(sd, &ctrl);
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
static int gc2063_get_fmt(struct v4l2_subdev* sd,
    struct v4l2_subdev_pad_config* cfg, struct v4l2_subdev_format* format)
{
    struct v4l2_mbus_framefmt* mf = &format->format;
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct gc2063_priv* priv = to_gc2063(client);
    static struct gc2063_win_size win = {
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
static int gc2063_set_fmt(struct v4l2_subdev* sd,
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
static int gc2063_get_selection(struct v4l2_subdev* sd,
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
static int gc2063_enum_mbus_code(struct v4l2_subdev* sd,
    struct v4l2_subdev_pad_config* cfg, struct v4l2_subdev_mbus_code_enum* code)
{
    if (code->pad || code->index >= ARRAY_SIZE(gc2063_codes))
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
static int gc2063_g_crop(struct v4l2_subdev* sd, struct v4l2_crop* a)
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
static int gc2063_cropcap(struct v4l2_subdev* sd, struct v4l2_cropcap* a)
{
    gc2063_sen_get_resolution_func(
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
static int gc2063_video_probe(struct i2c_client* client)
{
    struct gc2063_priv* priv = to_gc2063(client);
    int ret;

    ret = v4l2_ctrl_handler_setup(&priv->hdl);

    return ret;
}

/*ctrl ops*/
static const struct v4l2_ctrl_ops gc2063_ctrl_ops = {
    .g_volatile_ctrl = gc2063_g_volatile_ctrl,
    .s_ctrl = gc2063_s_ctrl,
};

/*core ops*/
static struct v4l2_subdev_core_ops gc2063_subdev_core_ops = {
    .s_power = gc2063_core_s_power,
    .g_ctrl = gc2063_core_g_ctrl,
    .s_ctrl = gc2063_core_s_ctrl,
    .ioctl = gc2063_core_ioctl,
};

/*
 * XXXX_g_mbus_config -
 * get buf config
 * video functions
 *
 * @sd:             subdev
 * @cfg:            return config
 */
static int gc2063_g_mbus_config(
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
static int gc2063_s_crop(struct v4l2_subdev* sd, const struct v4l2_crop* crop)
{
    printk(KERN_ERR "%s %d, left:%d, top:%d, width:%d, height:%d\n", __func__,
        __LINE__, crop->c.left, crop->c.top, crop->c.width, crop->c.height);
    return 0;
}

/*video ops*/
static struct v4l2_subdev_video_ops gc2063_subdev_video_ops = {
    .s_stream = gc2063_s_stream,
    .cropcap = gc2063_cropcap,
    .g_crop = gc2063_g_crop,
    .g_mbus_config = gc2063_g_mbus_config,
    .s_crop = gc2063_s_crop,
};

/*pad ops*/
static const struct v4l2_subdev_pad_ops gc2063_subdev_pad_ops = {
    .enum_mbus_code = gc2063_enum_mbus_code,
    .get_fmt = gc2063_get_fmt,
    .set_fmt = gc2063_set_fmt,
    .get_selection = gc2063_get_selection,
};

/*sensor driver subdev ops*/
static struct v4l2_subdev_ops gc2063_subdev_ops = {
    .core = &gc2063_subdev_core_ops,
    .video = &gc2063_subdev_video_ops,
    .pad = &gc2063_subdev_pad_ops,
};

/*
 * sensor_of_parse -
 * parse node of device
 *
 * @client:         pointor to i2c client
 * @priv:           sensor struct
 */
static int sensor_of_parse(struct i2c_client* client, struct gc2063_priv* priv)
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
static int gc2063_match(struct gc2063_priv* priv)
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
    struct gc2063_priv* t_priv = NULL;
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

void gc2063_probe_from_which_compatible(struct i2c_client* client,
    struct gc2063_priv* priv)
{
    const char *compatible = NULL;
    int of_match_index = 0;
    int cplen = 0;

    compatible = of_get_property(client->dev.of_node, "compatible", &cplen);

    for(of_match_index = 0;
        of_match_index < sizeof(gc2063_of_match)/sizeof(struct of_device_id);
        of_match_index++) {
        if (strcmp(compatible,
                gc2063_of_match[of_match_index].compatible) == 0) {
            if(of_match_index == 1) {//"anyka,sensorA"
                sensor_probe_once = true;
                break;
            } else if(of_match_index == 2){//"anyka,sensorB"
                sensor_probe_once = true;
                priv->sensor_sys_ignore = true;
                break;
            } else {
                break;
            }
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
static int gc2063_probe(
    struct i2c_client* client, const struct i2c_device_id* did)
{
    struct gc2063_priv* priv;
    struct i2c_adapter* adapter = to_i2c_adapter(client->dev.parent);
    int ret;


    pr_err("%s %s %s\n", __func__, __DATE__, __TIME__);

    if (dvp)
        SENSOR_IO_INTERFACE = DVP_INTERFACE;
    else
        SENSOR_IO_INTERFACE = MIPI_INTERFACE;

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_err(&adapter->dev, "gc2063: I2C-Adapter doesn't support SMBUS\n");
        return -EIO;
    }

    priv = devm_kzalloc(&client->dev, sizeof(struct gc2063_priv), GFP_KERNEL);
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
    priv->cb_info.cb = &gc2063_cb;
    priv->cb_info.arg = priv;
    priv->client = client;

    if (!client->dev.of_node) {
        dev_err(&client->dev, "Missing platform_data for driver\n");
        ret = -EINVAL;
        goto err_clk;
    }

    if (!gc2063_match(priv)) {
        ret = -ENODEV;
        goto err_clk;
    }
    //为了适配不同分辨率的双目
    if (sensor_probe_once) {
        dev_err(&client->dev,"gc2063 don't support probe again!\n");
        ret = -ENODEV;
        goto err_clk;
    }
    gc2063_probe_from_which_compatible(client, priv);


    /*subdev init*/
    v4l2_i2c_subdev_init(&priv->subdev, client, &gc2063_subdev_ops);
    priv->subdev.flags
        |= /*V4L2_SUBDEV_FL_HAS_EVENTS | */ V4L2_SUBDEV_FL_HAS_DEVNODE;
    v4l2_ctrl_handler_init(&priv->hdl, 2);
    v4l2_ctrl_new_std(&priv->hdl, &gc2063_ctrl_ops, V4L2_CID_VFLIP, 0, 1, 1, 0);
    v4l2_ctrl_new_std(&priv->hdl, &gc2063_ctrl_ops, V4L2_CID_HFLIP, 0, 1, 1, 0);

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

    ret = gc2063_video_probe(client);
    if (ret < 0) {
        dev_err(&client->dev, "gc2063 probe fail\n");
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

    dev_err(&adapter->dev, "gc2063 Probed success, subdev:%p\n", &priv->subdev);

    /*ak platform need export some informations from sensor*/
    if (!priv->sensor_sys_ignore)
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
static int gc2063_remove(struct i2c_client* client)
{
    struct gc2063_priv* priv = to_gc2063(client);

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
    if (!priv->sensor_sys_ignore)
        call_sensor_sys_deinit();

    sensor_probe_once = 0;
    return 0;
}

static const struct i2c_device_id gc2063_id[] = { { "gc2063", 0 }, {} };
MODULE_DEVICE_TABLE(i2c, gc2063_id);

static const struct of_device_id gc2063_of_match[4] = {
    /*donot changed compatible, must the same as dts*/
    {
        .compatible = "anyka,sensor0",
    },
    {
        .compatible = "anyka,sensorA",
    },
    {
        .compatible = "anyka,sensorB",
    },
    {},
};
MODULE_DEVICE_TABLE(of, gc2063_of_match);

static struct i2c_driver gc2063_i2c_driver = {
    .driver = {
        .name = "gc2063",
        .of_match_table = of_match_ptr(gc2063_of_match),
    },
    .probe    = gc2063_probe,
    .remove   = gc2063_remove,
    .id_table = gc2063_id,
};

module_i2c_driver(gc2063_i2c_driver);

MODULE_DESCRIPTION("SoC Camera driver for gc2063 sensor");
MODULE_AUTHOR("Anyka Microelectronic");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0.13");

/*sensor i2c write*/
static int __gc2063_write(const struct i2c_client* client, int reg, int value)
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

static int _gc2063_write(
    struct i2c_client* client, int reg, int value, int sensor_switch)
{
    struct i2c_client* client_p = client;
    unsigned short tmp_addr = client_p->addr;
    int ret = 0;

    if (sensor_switch == 1) {
        client_p->addr = SENSOR_I2C_ADDR0;
        __gc2063_write(client_p, reg, value);
        client_p->addr = SENSOR_I2C_ADDR1;
        ret = __gc2063_write(client_p, reg, value);
        client_p->addr = tmp_addr;
        return ret;
    } else {
        return __gc2063_write(client_p, reg, value);
    }
}

static int gc2063_write_cisx(struct gc2063_priv* t_priv, int reg, int value)
{
    switch (t_priv->subdev.devnode->num) {
        case 0:
            return _gc2063_write(t_priv->client, reg, value, sensor0_switch);
            break;

        case 1:
            return _gc2063_write(t_priv->client, reg, value, sensor1_switch);
            break;

        default:
            break;
    }
    return 0;
}

static int gc2063_write(struct i2c_client* client, int reg, int value)
{
    struct gc2063_priv* t_priv = NULL;
    struct i2c_client* client_p;

    list_for_each_entry(t_priv, &privs_list, list)
    {
        if (!t_priv || !t_priv->subdev.devnode)
            continue;

        if (t_priv->master_or_slave == SLAVER_MODE) {
            // Dual CSI
            if (client == t_priv->client) {
                return gc2063_write_cisx(t_priv, reg, value);
            }
        } else {
            // single CSI
            // when use Two_way_I2C + MIPI switch need write register twice.
            gc2063_write_cisx(t_priv, reg, value);
        }
    }
    return 0;
}

/*sensor i2c read*/
static int gc2063_read(const struct i2c_client* client, int reg)
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

#if 0
static void gc2063_dual_master_vts(struct gc2063_priv *priv, int cur_vts)
{
    struct i2c_client *client = priv->client;
    int value;

    if (priv->master_or_slave != MASTER_MODE)
        return;

    /*
     * gc2063主从模式时，主的帧率要稍微大于从的帧率。
     * 调试发现0x42原为0x65调整到0x6b就可以稳定同步，
     * 因此我们认为当前初始化序列时主比次大0x6b-0x65=6即可。
     */

    if (cur_vts <= 0) {
        value = gc2063_read(client, 0x41) << 8;
        value |= gc2063_read(client, 0x42) & 0xff;
    } else {
        value = cur_vts;
    }
    value += 6;
    gc2063_write(client, 0x41, value >> 8);
    gc2063_write(client, 0x42, value & 0xff);
}

static void gc2063_dual_init_func(struct gc2063_priv *priv,
        AK_ISP_SENSOR_INIT_PARA *para)
{
    /*
     * gc2053双目目前使用单次触发单次输出，但为了兼容单次触发连续输出，
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
                gc2063_write(client, preg_info->reg_addr, preg_info->value);

                preg_info++;
            }

            /*slaver*/
            gc2063_write(client,0xfe,0x00);
            gc2063_write(client,0x7f,0x09);
            gc2063_write(client,0x82,0x0a);
            gc2063_write(client,0x83,0x0b);
            gc2063_write(client,0x84,0x80);
            gc2063_write(client,0x85,0x51);
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
                gc2063_write(client, preg_info->reg_addr, preg_info->value);

                preg_info++;
            }

            /*master*/
            gc2063_write(client,0xfe,0x00);
            gc2063_write(client,0x7f,0x09);
            gc2063_write(client,0x82,0x01);
            gc2063_write(client,0x83,0x0c);
            gc2063_write(client,0x84,0x80);

            gc2063_dual_master_vts(priv, 0);
        }
    }
}
#endif

/*
 * The sensor must set callbacks
 *
 * */
static int gc2063_sen_init_func(
    void* arg, const AK_ISP_SENSOR_INIT_PARA* npara)
{
    int i;
    AK_ISP_SENSOR_REG_INFO* preg_info;
    struct gc2063_priv* priv = arg;
    struct i2c_client* client = priv->client;
    AK_ISP_SENSOR_INIT_PARA* para = &priv->para;

    gc2063_sen_probe_id_func(arg); // use IIC bus

    if (para->num <= 0 && npara)
        para = (void*)npara;

    preg_info = para->reg_info;
    for (i = 0; i < para->num; i++) {
#if 0
        {
            int value;

            value = gc2063_read(client, preg_info->reg_addr);
            pr_err("reg:%x read value:%x\n", preg_info->reg_addr, value);
        }
#endif
        gc2063_write(client, preg_info->reg_addr, preg_info->value);
#if 0
        {
            int value;

            value = gc2063_read(client, preg_info->reg_addr);
            pr_err("reg:%x write value:%x, read back value:%x\n",
                    preg_info->reg_addr, preg_info->value, value);
        }
#endif

        preg_info++;
    }

    priv->fps_info.current_fps = MAX_FPS;
    priv->fps_info.to_fps = priv->fps_info.current_fps;
    priv->fps_info.to_fps_value = 0;
    gc2063_write(client, 0xfe, 0x00);

    priv->aec_parms.reg_frame_vts
        = (gc2063_read(client, 0x41) & (0x3f)) << 8 | gc2063_read(client, 0x42);

    priv->aec_parms.reg_frame_hts
        = ((gc2063_read(client, 0x05) & (0x0f)) << 8 | gc2063_read(client, 0x06)) * 2;

    priv->aec_parms.pclk_freq = priv->aec_parms.reg_frame_hts
        * priv->aec_parms.reg_frame_vts * MAX_FPS;

    priv->aec_parms.calc_vts_tmp
        = (priv->aec_parms.reg_frame_vts * MAX_FPS * 100);

    pr_info("MarJ pclk %d vts %d hts %d calc_tmp %d\n",
    priv->aec_parms.pclk_freq, priv->aec_parms.reg_frame_vts,
    priv->aec_parms.reg_frame_hts, priv->aec_parms.calc_vts_tmp);

    return 0;
}

/*read sensor register*/
static int gc2063_sen_read_reg_func(void* arg, const int reg_addr)
{
    struct gc2063_priv* priv = arg;
    struct i2c_client* client = priv->client;

    return gc2063_read(client, reg_addr);
}
/*write sensor register*/
static int gc2063_sen_write_reg_func(
    void* arg, const int reg_addr, int value)
{
    struct gc2063_priv* priv = arg;
    struct i2c_client* client = priv->client;

    return gc2063_write(client, reg_addr, value);
}
/*read sensor register, NO i2c ops*/
static int gc2063_sen_read_id_func(void* arg) // no use IIC bus
{
    return SENSOR_ID;
}

static unsigned char regValTable[29][4] = {
    //0xb4  0xb3 0xb8 0xb9
    {0x00, 0x00,0x01,0x00},
    {0x00, 0x10,0x01,0x0c},
    {0x00, 0x20,0x01,0x1b},
    {0x00, 0x30,0x01,0x2c},
    {0x00, 0x40,0x01,0x3f},
    {0x00, 0x50,0x02,0x16},
    {0x00, 0x60,0x02,0x35},
    {0x00, 0x70,0x03,0x16},
    {0x00, 0x80,0x04,0x02},
    {0x00, 0x90,0x04,0x31},
    {0x00, 0xa0,0x05,0x32},
    {0x00, 0xb0,0x06,0x35},
    {0x00, 0xc0,0x08,0x04},
    {0x00, 0x5a,0x09,0x19},
    {0x00, 0x83,0x0b,0x0f},
    {0x00, 0x93,0x0d,0x12},
    {0x00, 0x84,0x10,0x00},
    {0x00, 0x94,0x12,0x3a},
    {0x01, 0x2c,0x1a,0x02},
    {0x01, 0x3c,0x1b,0x20},
    {0x00, 0x8c,0x20,0x0f},
    {0x00, 0x9c,0x26,0x07},
    {0x02, 0x64,0x36,0x21},
    {0x02, 0x74,0x37,0x3a},
    {0x00, 0xc6,0x3d,0x02},
    {0x00, 0xdc,0x3f,0x3f},
    {0x02, 0x85,0x3f,0x3f},
    {0x02, 0x95,0x3f,0x3f},
    {0x00, 0xce,0x3f,0x3f},
};

static unsigned int gainLevelTable[30] = {
   64 ,
   74 ,
   89 ,
   102,
   127,
   147,
   177,
   203,
   260,
   300,
   361,
   415,
   504,
   581,
   722,
   832,
   1027,
   1182,
   1408,
   1621,
   1990,
   2291,
   2850,
   3282,
   4048,
   5180,
   5500,
   6744,
   7073,
   0xffffffff,
};

static int gc2063_setgain(struct i2c_client* client, unsigned short iGain)
{
    u16 i;
    u16 total;
    unsigned int tol_dig_gain = 0;
    u16 iReg;

    iReg = iGain;

    total = sizeof(gainLevelTable) / sizeof(unsigned int);
    for (i = 0; i < total; i++) {
        if ((gainLevelTable[i] <= iReg) && (iReg < gainLevelTable[i + 1]))
            break;
    }
    gc2063_write(client, 0xfe, 0x00);
    gc2063_write(client, 0xb4, regValTable[i][0]);
    gc2063_write(client, 0xb3, regValTable[i][1]);
    gc2063_write(client, 0xb8, regValTable[i][2]);
    gc2063_write(client, 0xb9, regValTable[i][3]);
    tol_dig_gain = iReg * 64 / gainLevelTable[i];
    gc2063_write(client, 0xb1, (tol_dig_gain >> 6));
    gc2063_write(client, 0xb2, ((tol_dig_gain & 0x3f) << 2));
    return 0;
}

/*set sensor again*/
/*max gain: 4853*/
static int gc2063_sen_update_a_gain_func(
    void* arg, const unsigned int a_gain)
{
    struct gc2063_priv* priv = arg;
    struct i2c_client* client = priv->client;
    unsigned int tmp_a_gain;
    unsigned int tmp_d_gain;

    tmp_a_gain = a_gain >> 2;
    tmp_d_gain = 0;

    gc2063_setgain(client, tmp_a_gain);

    return A_GAIN_EFFECT_FRAMES;
}
/*set sensor dgain*/
static int gc2063_sen_update_d_gain_func(
    void* arg, const unsigned int d_gain)
{
    return D_GAIN_EFFECT_FRAMES;
}
/*set sensor exp time*/
/*max exp: 0x3fff*/
static int gc2063_sen_updata_exp_time_func(void* arg, unsigned int exp_time)
{
    struct gc2063_priv* priv = arg;
    struct i2c_client* client = priv->client;
    int const max_exp_ctrl = priv->fps_info.to_fps_value - EXP_LIMITE; // Vts-8

    if (exp_time < 1)
        exp_time = 1;
    if (exp_time > max_exp_ctrl)
        exp_time = max_exp_ctrl;
    // Update Shutter
    gc2063_write(client, 0x04, (exp_time)&0xff);
    gc2063_write(client, 0x03, (exp_time >> 8) & 0x1f);

    return EXP_EFFECT_FRAMES;
}
/*sensor timer*/
static int gc2063_sen_timer_func(void* arg) { return 0; }
/*standby in*/
static int gc2063_sen_set_standby_in_func(void* arg) { return 0; }
/*standby out*/
static int gc2063_sen_set_standby_out_func(void* arg) { return 0; }
/*low level read sensor ID, user i2c ops*/
static int gc2063_sen_probe_id_func(void* arg) // use IIC bus
{
    struct gc2063_priv* priv = arg;
    struct i2c_client* client = priv->client;
    int id;
    int value;

    value = gc2063_read(client, 0xf0);
    id = value << 8;

    value = gc2063_read(client, 0xf1);
    id |= value;

    printk(KERN_ERR "id:%x\n", id);

    if (id == 0x2053)
        return SENSOR_ID;

    return 0;
}
/*get resolution*/
static int gc2063_sen_get_resolution_func(void* arg, int* width, int* height)
{
    *width = SENSOR_OUTPUT_WIDTH;
    *height = SENSOR_OUTPUT_HEIGHT;
    return 0;
}
/*get mclk*/
static int gc2063_sen_get_mclk_func(void* arg) { return SENSOR_MCLK; }
/*get current fps*/
static int gc2063_sen_get_fps_func(void* arg)
{
    struct gc2063_priv* priv = arg;

    return priv->fps_info.current_fps;
}
/*get valid coordinate*/
static int gc2063_sen_get_valid_coord_func(void* arg, int* x, int* y)
{
    *x = SENSOR_VALID_OFFSET_X;
    *y = SENSOR_VALID_OFFSET_Y;
    return 0;
}
/*get bus type*/
static enum sensor_bus_type gc2063_sen_get_bus_type_func(void* arg)
{
    return SENSOR_BUS_TYPE;
}
/*get self definded params*/
static int gc2063_sen_get_parameter_func(void* arg, int param, void* value)
{
    int ret = 0;
    struct gc2063_priv* priv = arg;

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
static int gc2063_sen_set_power_on_func(void* arg)
{
    struct gc2063_priv* priv = arg;

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

    priv->dual.init_count = 0;
    priv->master_or_slave = SINGLE_MODE;

    return 0;
}
/*sensor power off*/
static int gc2063_sen_set_power_off_func(void* arg)
{
    struct gc2063_priv* priv = arg;

    if (priv->gpio_pwdn >= 0) {
        gpio_direction_output(priv->gpio_pwdn, SENSOR_PWDN_LEVEL);
    }

    if (priv->gpio_reset >= 0) {
        gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
    }

    return 0;
}

static int gc2063_fps_to_vts(struct gc2063_priv* priv, const int fps)
{
    unsigned int vts;
    unsigned int csi_pwm_freq = fps * 100;

    if (priv->master_or_slave == SINGLE_MODE
        || priv->master_or_slave == MASTER_MODE) {
        /*
         * dvp & mipi 参数一样的参数。
         *
         * 2053-h3-20200320-格科威给的新版本.txt
         *
         * window_size=1920*1080 mipi@2lane
         * mclk=27mhz,mipi_clk=594Mbps
         * pixel_line_total=2200,line_frame_total=1125
         * row_time=29.629us,frame_rate=30fps
         *
         * 按照格科威杨工所说，R{0x41,0x42}=一帧时间/一行时间。
         * 例如30帧时：R{0x41,0x42}=1000000/30/29.629=1125
         */
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
        /*vts = 1125 * 30 * 100 / vts;   row_time=29.629us */
        //vts = 3375000 / vts;
    } else { // SLAVER_MODE
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

        //=> f = fpwm * 1S / (1S-fpwm*300us)
        //目前测试Sensor帧率需要比PWM频率大11.5行的时间。
        if (fps == MAX_FPS)
            vts = csi_pwm_freq;
        else
            vts = csi_pwm_freq * 1000000 / (1000000 - csi_pwm_freq * 3);

        /*vts = 1273 * 30 * 100 / vts;   dual mode row_time=26.1us  (提频会出现竖纹，使用标准频率的序列)*/
        //vts = 3819000 / vts;
    }
    vts = (priv->aec_parms.calc_vts_tmp) / vts;

    return vts;
}

/*set sensor fps*/
static int gc2063_sen_set_fps_func(void* arg, const int fps)
{
    struct gc2063_priv* priv = arg;
    int tmp;

    tmp = gc2063_fps_to_vts(priv, fps);
    if (tmp > 0) {
        priv->fps_info.to_fps = fps;
        priv->fps_info.to_fps_value = tmp;
    } else {
        return -EINVAL;
    }

    return 0;
}

static void gc2063_set_fps_async(struct gc2063_priv* priv)
{
    struct i2c_client* client = priv->client;

    if (priv->fps_info.to_fps != priv->fps_info.current_fps) {
        gc2063_write(client, 0x41, priv->fps_info.to_fps_value >> 8);
        gc2063_write(client, 0x42, priv->fps_info.to_fps_value & 0xff);
        priv->fps_info.current_fps = priv->fps_info.to_fps;
        // gc2063_dual_master_vts(priv, priv->fps_info.to_fps_value);
    }
}

static AK_ISP_SENSOR_CB gc2063_cb = {
    .sensor_init_func = gc2063_sen_init_func,
    .sensor_read_reg_func = gc2063_sen_read_reg_func,
    .sensor_write_reg_func = gc2063_sen_write_reg_func,
    .sensor_read_id_func = gc2063_sen_read_id_func,
    .sensor_update_a_gain_func = gc2063_sen_update_a_gain_func,
    .sensor_update_d_gain_func = gc2063_sen_update_d_gain_func,
    .sensor_updata_exp_time_func = gc2063_sen_updata_exp_time_func,
    .sensor_timer_func = gc2063_sen_timer_func,
    .sensor_set_standby_in_func = gc2063_sen_set_standby_in_func,
    .sensor_set_standby_out_func = gc2063_sen_set_standby_out_func,
    .sensor_probe_id_func = gc2063_sen_probe_id_func,
    .sensor_get_resolution_func = gc2063_sen_get_resolution_func,
    .sensor_get_mclk_func = gc2063_sen_get_mclk_func,
    .sensor_get_fps_func = gc2063_sen_get_fps_func,
    .sensor_get_valid_coordinate_func = gc2063_sen_get_valid_coord_func,
    .sensor_get_bus_type_func = gc2063_sen_get_bus_type_func,
    .sensor_get_parameter_func = gc2063_sen_get_parameter_func,
    .sensor_set_power_on_func = gc2063_sen_set_power_on_func,
    .sensor_set_power_off_func = gc2063_sen_set_power_off_func,
    .sensor_set_fps_func = gc2063_sen_set_fps_func,
};

static int sensor_id_func(void) { return gc2063_sen_read_id_func(NULL); }

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
