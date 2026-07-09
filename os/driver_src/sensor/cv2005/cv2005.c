/*
 * cv2005 Camera Driver
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
//#include "cv2005_1920x1080.h"

/*
 *
SENSOR_PWDN_LEVEL:  the io level when sensor power down
SENSOR_RESET_LEVEL: the io level when sensor reset
SENSOR_I2C_ADDR:    the sensor slave address, 7 bits width.
                    the i2c bus left shift 1bits.
SENSOR_ID:  the sensor ID.
SENSOR_MCLK:    mclk, unit: MHz.24MHz or 27MHz mclk for most sensors.
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
#define SENSOR_PWDN_LEVEL		0
#define SENSOR_RESET_LEVEL		0
static int SENSOR_I2C_ADDR = 0x35;	        //write:0x6a, read:0x6b
#define ACTUAL_SENSOR_ID		0x2005
#define SENSOR_ID				0x2005    // V == 5, C52003 == CV2005
#define SENSOR_MCLK				24
#define SENSOR_REGADDR_BYTE		2
#define SENSOR_DATA_BYTE		1
#define SENSOR_OUTPUT_WIDTH		1920
#define SENSOR_OUTPUT_HEIGHT	1080
#define SENSOR_REAL_HEIGHT    1080 //sensor实际输出行数
#define SENSOR_VALID_OFFSET_X	0
#define SENSOR_VALID_OFFSET_Y	0
#define SENSOR_BUS_TYPE			BUS_TYPE_RAW
#define SENSOR_IO_INTERFACE		MIPI_INTERFACE
#define SENSOR_IO_LEVEL			IO_LEVEL_1V8
#define MIPI_MHZ				390
#define MIPI_LANES				2
//#define MAX_FPS					30
static int MAX_FPS = 20;

#if defined(CONFIG_MACH_AK39EV330) || defined(CONFIG_MACH_AK37D) /*H3B&H3D*/
#define EXP_EFFECT_FRAMES		1	/*adjust exp_time for 1/2 frames*/
#define A_GAIN_EFFECT_FRAMES	0	/*adjust a_gain for every frames*/
#define D_GAIN_EFFECT_FRAMES	0	/*adjust d_gain for every frames*/
#else /*H322&322L*/
#define EXP_EFFECT_FRAMES    1 /*adjust exp_time for 1/2 frames*/
#define A_GAIN_EFFECT_FRAMES 1 /*adjust a_gain for 1/2 frames*/
#define D_GAIN_EFFECT_FRAMES 1 /*adjust d_gain for 1/2 frames*/
#endif

#define DELAY_FLAG		(0xffff)

#define ID_REG_H    (0x3003)
#define ID_REG_L    (0x3002)
#define VTS_REG_H   (0x301E)
#define VTS_REG_M   (0x301D)
#define VTS_REG_L   (0x301C)
#define HTS_REG_H   (0x3021)
#define HTS_REG_L   (0x3020)

#define EXP_REG_H   (0x304A)
#define EXP_REG_M   (0x3049)
#define EXP_REG_L   (0x3048)

/*v330 flip no set*/
#define FLIP_REG        (0x3028)

#define GAIN_EN_REG     (0x3109)
#define AGAIN_REG       (0x3118)
#define DGAIN_REG_H     (0x311D)
#define DGAIN_REG_L     (0x311C)

#define FPS_EN      (1)
#define EXP_EN      (1)
#define AGAIN_EN    (1)
// #if defined(CONFIG_MACH_AK39EV330) || defined(CONFIG_MACH_AK37D) /*H3B&H3D*/
// #define FLIP_EN     (0)		/*v330 flip no set*/
// #else /*H322&322L*/
#define FLIP_EN     (1)
// #endif

#define FLIP_DROP_FRAMES (3) // need to drop frame number after fliped & mirrored

#define EXP_DECREASE_LINES (2) // used to calculate the maximum exposure value
#define EXP_MIN        (10)
//#define ONE_LINE_EXP_OF_STEPS (2)
//#define MAX_EXP_OF_FPS30    (1175) //单位为曝光行
//#define MAX_A_GAIN_X        (1000)  //(32*(2000/64))
//#define MAX_D_GAIN_X        (1)
//#define ONE_LINE_CYCLE      (9040)
//#define HBLIANK_CYCLE       (120)

/*
 * Struct
 */
struct regval_list {
    u8 reg_num;
    u8 value;
};

struct cv2005_win_size {
    char			*name;
    u32				width;
    u32				height;
    const struct regval_list	*regs;
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
    // int rb_rows;
};

/*
 * the host callbacks for sensor
 * @isp_timing_cb_info: isp timing callback.
 * 	a few sensor need the callback when fps had changed
 */
struct host_callbacks {
    struct isp_timing_cb_info isp_timing_cb_info;
};

/*
 * aec paramters. some sensors need record paramters.
 */
struct cv2005_aec_parms {
    int curr_again_level;
    int curr_again_10x;
    int target_exp_ctrl;
    int curr_2x_dgain;
    int curr_corse_gain;

    int reg_frame_hts;
    int reg_frame_vts;
    int pclk_freq;
    int calc_vts_tmp;
};

static const struct v4l2_ctrl_ops cv2005_ctrl_ops;
static const struct v4l2_ctrl_config config_sensor_get_id = {
    .ops	= &cv2005_ctrl_ops,
    .id		= SENSOR_GET_ID,
    .name	= "get sensor id",
    .type	= V4L2_CTRL_TYPE_INTEGER,
    .min	= 0,
    .max	= 0xfffffff,
    .step	= 1,
    .def	= 0,
    .flags	= V4L2_CTRL_FLAG_VOLATILE,/*set V4L2_CTRL_FLAG_VOLATILE avoid get cached*/
};

struct dual_sensor_attr {
    int init_count;
};

/*
 * the sensor struct
 * @client: point to i2c client
 * @subdev: v4l2 subdev struct
 * @hdl: 	v4l2 control handler
 * @cfmt_code:
 * @win:	default window
 * @gpio_reset:	gpio number for reset pin
 * @gpio_pwdn:	gpio number for power down pin
 * @cb_info:	sensor callbacks
 * @fps_info:	fps informations
 * @hcb:		host callbacks
 * @aec_parms:	aec paramters
 */
struct cv2005_priv {
    struct list_head list;
    struct i2c_client *client;
    struct v4l2_subdev		subdev;
    struct v4l2_ctrl_handler	hdl;
    u32	cfmt_code;
    const struct cv2005_win_size	*win;

    int gpio_reset;
    int gpio_pwdn;
    struct sensor_cb_info cb_info;
    struct sensor_fps_info fps_info;

    struct host_callbacks hcb;
    struct cv2005_aec_parms aec_parms;

    AK_ISP_SENSOR_INIT_PARA para;

    enum sensor_master_or_slave_mode master_or_slave;
    enum raw_seq raw_bayer_seq;
    struct dual_sensor_attr dual;
    struct v4l2_ctrl* ctrl_sensor_get_id;
};

/*default aec of fast boot*/
// static struct ae_fast_struct ae_fast_default = {
//     .sensor_exp_time = 2152,
//     .sensor_a_gain	= 256,
//     .sensor_d_gain	= 256,
//     .isp_d_gain		= 256,
//     {
//         .r_gain = 392,
//         .g_gain = 256,
//         .b_gain = 473,
//         .r_offset = 0,
//         .g_offset = 0,
//         .b_offset = 0
//     }
// };
static LIST_HEAD(privs_list);

static AK_ISP_SENSOR_CB cv2005_cb;

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
 * dual CSI + dual Switch :    addr0=0xxx   addr1=0xxx
 *              sensor0_switch=1   sensor1_switch=1;
 * Single CSI
 * CSI +  Switch :  sensor0_switch=1;
 *             or sensor1_switch=1
 * CSI +  dual Switch :  addr0=0xxx   sensor1_switch=1;
 *                or addr1=0xxx sensor1_switch=1;
 */
#define SENSOR_I2C_ADDR0 (0x35)
#define SENSOR_I2C_ADDR1 (0x36)
static int addr0 = SENSOR_I2C_ADDR0, addr1 = SENSOR_I2C_ADDR1;
static int sensor0_switch = 0;
static int sensor1_switch = 0;

module_param(MAX_FPS, int, 0644);

module_param(dvp, int, 0644);
module_param(addr0, int, 0644);
module_param(addr1, int, 0644);
module_param(sensor0_switch, int, 0644);
module_param(sensor1_switch, int, 0644);
module_param(SENSOR_I2C_ADDR, int, 0644);
/*
 * check_id - check hardware sensor ID whether it meets this driver
 * 0-no check, force to meet
 * others-check, if not meet return fail
 */
static int check_id = 0;
module_param(check_id, int, 0644);

static void cv2005_set_fps_async(struct cv2005_priv *priv);
//static int __cv2005_sensor_probe_id_func(struct i2c_client *client);  //use IIC bus
static int cv2005_sensor_read_id_func(void *arg);   //no use IIC bus
static int cv2005_sensor_get_resolution_func(void *arg, int *width, int *height);
static int cv2005_sensor_set_fps_func(void *arg, const int fps);
static int cv2005_fps_to_vts(struct cv2005_priv *priv, const int fps);
static int call_sensor_sys_init(void);
static int call_sensor_sys_deinit(void);
static int cv2005_read(const struct i2c_client *client, int reg);
static int cv2005_write(struct i2c_client* client, int reg, int value);

static u32 cv2005_codes[] = {
    MEDIA_BUS_FMT_YUYV8_2X8,
    MEDIA_BUS_FMT_UYVY8_2X8,
    MEDIA_BUS_FMT_RGB565_2X8_BE,
    MEDIA_BUS_FMT_RGB565_2X8_LE,
};

/*
 * General functions
 */
static struct cv2005_priv *to_cv2005(const struct i2c_client *client)
{
    return container_of(i2c_get_clientdata(client), struct cv2005_priv,
                subdev);
}

static struct v4l2_subdev *ctrl_to_sd(struct v4l2_ctrl *ctrl)
{
    return &container_of(ctrl->handler, struct cv2005_priv, hdl)->subdev;
}

/*
 * XXXX_s_stream -
 * set steaming enable/disable
 * soc_camera_ops functions
 *
 * @sd:				subdev
 * @enable:			enable flags
 */
static int cv2005_s_stream(struct v4l2_subdev *sd, int enable)
{
    return 0;
}

static int cv2005_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
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
 * @ctrl:			pointer to ctrl
 */
static int cv2005_s_ctrl(struct v4l2_ctrl *ctrl)
{
    return 0;
}

/*
 * XXXX_s_power -
 * set power operation
 * soc_camera_ops functions
 *
 * @sd:			subdev
 * @on:			power flags
 */
static int cv2005_core_s_power(struct v4l2_subdev *sd, int on)
{
    return 0;
}

/*
 * XXXX_get_sensor_id -
 * get sensor ID
 * private callback functions
 *
 * @ctrl:			pointer to ctrl
 */
static int cv2005_get_sensor_id(struct v4l2_control *ctrl)
{
    ctrl->value = cv2005_sensor_read_id_func(NULL);   //no use IIC bus
    return 0;
}

/*
 * XXXX_get_sensor_id -
 * get sensor callback
 * private callback functions
 *
 * @sd:				subdev
 * @ctrl:			pointer to ctrl
 */
static int cv2005_get_sensor_cb(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct cv2005_priv *priv = to_cv2005(client);

    ctrl->value = (int)&priv->cb_info;
    return 0;
}

static int cv2005_get_max_exp_for_fps(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct cv2005_priv *priv = to_cv2005(client);
    int fps = ctrl->value;
    int vts = cv2005_fps_to_vts(priv, fps);

    ctrl->value = vts - EXP_DECREASE_LINES;
    return 0;
}

// static int cv2005_get_cis_bayer_raw_seq(
//     struct v4l2_subdev* sd, struct v4l2_control* ctrl)
// {
//     struct i2c_client* client = v4l2_get_subdevdata(sd);
//     struct cv2005_priv* priv = to_cv2005(client);
// #if 0
//     int value = cv2005_read(client, FLIP_REG);

//     if (value&1) {
//         priv->raw_bayer_seq = RAW_SEQ_B;
//     }
//     else {
//         priv->raw_bayer_seq = RAW_SEQ_R;
//     }
// #endif
//     ctrl->value = priv->raw_bayer_seq;

//     pr_err("%s raw_bayer_seq:%d\n", __func__, priv->raw_bayer_seq);

//     return 0;
// }
/*
 * XXXX_core_g_ctrl -
 * get control
 * core functions
 *
 * @sd:				subdev
 * @ctrl:			pointer to ctrl
 */
static int cv2005_core_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int ret;

    switch (ctrl->id) {
        case GET_SENSOR_ID:
            ret = cv2005_get_sensor_id(ctrl);
            break;

        case GET_SENSOR_CB:
            ret = cv2005_get_sensor_cb(sd, ctrl);
            break;

        case GET_MAX_EXP_FOR_FPS:
            ret = cv2005_get_max_exp_for_fps(sd, ctrl);
            break;

        // case GET_RAW_BAYER_SEQ:
        //     ret = cv2005_get_cis_bayer_raw_seq(sd, ctrl);
        //     break;

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
 * @sd:				subdev
 * @ctrl:			pointer to ctrl
 */
static int cv2005_set_isp_timing_cb(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct cv2005_priv *priv = to_cv2005(client);
    struct isp_timing_cb_info *isp_timing_cb_info = (void *)ctrl->value;

    memcpy(&priv->hcb.isp_timing_cb_info,
            isp_timing_cb_info,
            sizeof(struct isp_timing_cb_info));
    return 0;
}

/*
 * XXXX_set_fps_direct -
 * set sensor fps directly, synchronously.
 * private callback functions
 *
 * @sd:				subdev
 * @ctrl:			pointer to ctrl
 */
static int cv2005_set_fps_direct(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct cv2005_priv *priv = to_cv2005(client);
    int fps = ctrl->value;

    cv2005_sensor_set_fps_func(priv, fps);
    cv2005_set_fps_async(priv);
    return 0;
}

static int cv2005_set_master_or_slave(struct v4l2_subdev* sd, int is_master)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct cv2005_priv* priv = to_cv2005(client);

    pr_debug("%s is_master:%d\n", __func__, is_master);

    if (is_master)
        priv->master_or_slave = MASTER_MODE;
    else
        priv->master_or_slave = SLAVER_MODE;

    return 0;
}

static int cv2005_set_flip_mirror(struct v4l2_subdev* sd, int enable)
{
    struct i2c_client* client = v4l2_get_subdevdata(sd);
    struct cv2005_priv* priv = to_cv2005(client);
    int value = 0, tmp = 0;
    int reg_0x3034 = 0, reg_0x3038 = 0;
    int flip_en = enable & (0x1 << FLIP_OFFSET);
    int mirror_en = enable & (0x1 << MIRROR_OFFSET);
    int off_x = 0, off_y = 0, off_y_fast = 0, off_x_fast = 0;

#if FLIP_EN
#define RGB_WIDTH_ST_REG_L  (0x3038)
#define RGB_HEIGHT_ST_REG_L (0x3034)

    // 保存初始寄存器值（只在第一次调用时读取）
    static int initial_3034 = -1;
    static int initial_3038 = -1;
    
    if (initial_3034 == -1 || initial_3038 == -1) {
        initial_3034 = cv2005_read(client, (RGB_HEIGHT_ST_REG_L));
        initial_3038 = cv2005_read(client, (RGB_WIDTH_ST_REG_L));
    }

    value = 0;
    if (flip_en) {
        value |= 0x2;
    }
    if (mirror_en) {
        value |= 0x1;
    }

    // 计算新的寄存器值
    if (flip_en && mirror_en) {
        reg_0x3034 = initial_3034 + 1;
        reg_0x3038 = initial_3038 + 1;
    } else if (flip_en) {
        reg_0x3034 = initial_3034 + 1;
        reg_0x3038 = initial_3038;
    } else if (mirror_en) {
        reg_0x3034 = initial_3034;
        reg_0x3038 = initial_3038 + 1;
    } else {
        // 都不设置时，恢复到初始值
        reg_0x3034 = initial_3034;
        reg_0x3038 = initial_3038;
    }

#if 0
    if (flip_en && mirror_en)
        priv->raw_bayer_seq = RAW_SEQ_B;
    else if (flip_en && !mirror_en)
        priv->raw_bayer_seq = RAW_SEQ_Gb;
    else if (!flip_en && mirror_en)
        priv->raw_bayer_seq = RAW_SEQ_Gr;
    else if (!flip_en && !mirror_en)
        priv->raw_bayer_seq = RAW_SEQ_R;
#endif

    /***
    序列为：RAW_SEQ_B或 RAW_SEQ_R
    ***/
    //priv->raw_bayer_seq = (value&0x1);
    //flip
    cv2005_write(client, (RGB_HEIGHT_ST_REG_L), reg_0x3034);
    cv2005_write(client, (RGB_WIDTH_ST_REG_L), reg_0x3038);
    cv2005_write(client, FLIP_REG, value);
#endif

    return 0;
}

/*
 * XXXX_core_s_ctrl -
 * set core s_ctrl
 * core functions
 *
 * @sd:				subdev
 * @ctrl:			pointer to ctrl
 */
static int cv2005_core_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
    int ret;

    switch (ctrl->id) {
        case SET_ISP_MISC_CALLBACK:
            ret = cv2005_set_isp_timing_cb(sd, ctrl);
            break;

        case SET_FPS_DIRECT:
            ret = cv2005_set_fps_direct(sd, ctrl);
            break;

        case SET_MASTER:
            ret = cv2005_set_master_or_slave(sd, 1);
            break;

        case SET_SLAVER: /*for dual sensor*/
            ret = cv2005_set_master_or_slave(sd, 0);
            break;

        case SET_FLIP_MIRROR:
            ret = cv2005_set_flip_mirror(sd, ctrl->value);
            break;

        default:
            pr_err("%s cmd:%d not support\n", __func__, ctrl->id);
            ret = -1;
            break;
    }

    return ret;
}

static int cv2005_store_initial_regs(struct cv2005_priv *priv, void *arg)
{
    struct i2c_client *client = priv->client;
    AK_ISP_SENSOR_INIT_PARA *sensor_para = arg;
    void *p_reg_info;
    int ret;

    p_reg_info = sensor_para->reg_info;
    sensor_para->reg_info =
        kmalloc(sizeof(AK_ISP_SENSOR_REG_INFO) * sensor_para->num, GFP_KERNEL);
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
                sizeof(AK_ISP_SENSOR_REG_INFO)*sensor_para->num, GFP_KERNEL);
        if (!priv->para.reg_info) {
            pr_err("%s alloc2 fail\n", __func__);
            goto alloc2_fail;
        }
        memcpy(priv->para.reg_info, sensor_para->reg_info,
                sizeof(AK_ISP_SENSOR_REG_INFO)*sensor_para->num);

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

static long cv2005_core_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct cv2005_priv *priv = to_cv2005(client);
    int ret = 0;

    pr_debug("%s\n", __func__);

    switch (cmd) {
        case AK_SENSOR_SET_INIT:
            cv2005_store_initial_regs(priv, arg);
            break;

        case AK_SENSOR_GET_MAX_EXP_FOR_FPS: {
            struct sensor_max_exp_for_fps *exp_for_fps = arg;
            struct v4l2_control ctrl;

            ctrl.value = exp_for_fps->fps;
            ret = cv2005_get_max_exp_for_fps(sd, &ctrl);
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
 * @sd:				subdev
 * @cfg:			pointer to pad config
 * @format:			return format
 */
static int cv2005_get_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_pad_config *cfg,
        struct v4l2_subdev_format *format)
{
    struct v4l2_mbus_framefmt *mf = &format->format;
    struct i2c_client  *client = v4l2_get_subdevdata(sd);
    struct cv2005_priv *priv = to_cv2005(client);
    static struct cv2005_win_size win = {
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

    mf->width	= priv->win->width;
    mf->height	= priv->win->height;
    mf->code	= priv->cfmt_code;

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
    mf->field	= V4L2_FIELD_NONE;

    return 0;
}

/*
 * XXXX_set_fmt -
 * set format
 * pad functions
 *
 * @sd:				subdev
 * @cfg:			pointer to pad config
 * @format:			format to set
 */
static int cv2005_set_fmt(struct v4l2_subdev *sd,
        struct v4l2_subdev_pad_config *cfg,
        struct v4l2_subdev_format *format)
{
    struct v4l2_mbus_framefmt *mf = &format->format;

    if (format->pad)
        return -EINVAL;

    mf->field	= V4L2_FIELD_NONE;

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
 * @sd:				subdev
 * @cfg:			pointer to pad config
 * @sel:			return selection
 */
static int cv2005_get_selection(struct v4l2_subdev *sd,
        struct v4l2_subdev_pad_config *cfg,
        struct v4l2_subdev_selection *sel)
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
 * @sd:				subdev
 * @cfg:			pointer to pad config
 * @code:			return code of bus type
 */
static int cv2005_enum_mbus_code(struct v4l2_subdev *sd,
        struct v4l2_subdev_pad_config *cfg,
        struct v4l2_subdev_mbus_code_enum *code)
{
    if (code->pad || code->index >= ARRAY_SIZE(cv2005_codes))
        return -EINVAL;

    code->code = MEDIA_BUS_FMT_YUYV8_2X8;

    return 0;
}

/*
 * XXXX_g_crop -
 * get crop
 * video functions
 *
 * @sd:				subdev
 * @a:				return crop
 */
static int cv2005_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
    a->c.left	= SENSOR_VALID_OFFSET_X;
    a->c.top	= SENSOR_VALID_OFFSET_Y;
    a->c.width	= SENSOR_OUTPUT_WIDTH;
    a->c.height	= SENSOR_OUTPUT_HEIGHT;
    a->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;

    return 0;
}

/*
 * XXXX_cropcap -
 * get crop capbilities
 * video functions
 *
 * @sd:				subdev
 * @a:				return crop capbilities
 */
static int cv2005_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
    cv2005_sensor_get_resolution_func(NULL,
        &a->bounds.width, &a->bounds.height);

    a->bounds.left			= SENSOR_VALID_OFFSET_X;
    a->bounds.top			= SENSOR_VALID_OFFSET_Y;
    a->defrect			    = a->bounds;
    a->type				    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    a->pixelaspect.numerator	= 1;
    a->pixelaspect.denominator	= 1;

    pr_err("%s bounds.width:%d, bounds.height:%d\n",
        __func__, a->bounds.width, a->bounds.height);

    return 0;
}

/*
 * XXXX_video_probe -
 * do some process in subdev probe
 * locale functions
 *
 * @sd:				subdev
 * @a:				return crop capbilities
 */
static int cv2005_video_probe(struct i2c_client *client)
{
    struct cv2005_priv *priv = to_cv2005(client);
    int ret;

    ret = v4l2_ctrl_handler_setup(&priv->hdl);

    return ret;
}

/*ctrl ops*/
static const struct v4l2_ctrl_ops cv2005_ctrl_ops = {
    .g_volatile_ctrl	= cv2005_g_volatile_ctrl,
    .s_ctrl				= cv2005_s_ctrl,
};

/*core ops*/
static struct v4l2_subdev_core_ops cv2005_subdev_core_ops = {
    .s_power	= cv2005_core_s_power,
    .g_ctrl		= cv2005_core_g_ctrl,
    .s_ctrl		= cv2005_core_s_ctrl,
    .ioctl		= cv2005_core_ioctl,
};

/*
 * XXXX_g_mbus_config -
 * get buf config
 * video functions
 *
 * @sd:				subdev
 * @cfg:			return config
 */
static int cv2005_g_mbus_config(struct v4l2_subdev *sd,
                struct v4l2_mbus_config *cfg)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    cfg->flags = V4L2_MBUS_PCLK_SAMPLE_RISING | V4L2_MBUS_MASTER |
        V4L2_MBUS_VSYNC_ACTIVE_HIGH | V4L2_MBUS_HSYNC_ACTIVE_HIGH |
        V4L2_MBUS_DATA_ACTIVE_HIGH;
    cfg->type = V4L2_MBUS_PARALLEL;
    cfg->flags = 0;

    return 0;
}

/*
 * XXXX_s_crop -
 * set crop
 * video functions
 *
 * @sd:				subdev
 * @crop:			crop to set
 */
static int cv2005_s_crop(struct v4l2_subdev *sd, const struct v4l2_crop *crop)
{
    printk(KERN_INFO "%s %d, left:%d, top:%d, width:%d, height:%d\n",
            __func__, __LINE__, crop->c.left, crop->c.top, crop->c.width, crop->c.height);
    return 0;
}

/*video ops*/
static struct v4l2_subdev_video_ops cv2005_subdev_video_ops = {
    .s_stream	    = cv2005_s_stream,
    .cropcap	    = cv2005_cropcap,
    .g_crop		    = cv2005_g_crop,
    .g_mbus_config	= cv2005_g_mbus_config,
    .s_crop		    = cv2005_s_crop,
};

/*pad ops*/
static const struct v4l2_subdev_pad_ops cv2005_subdev_pad_ops = {
    .enum_mbus_code = cv2005_enum_mbus_code,
    .get_fmt	    = cv2005_get_fmt,
    .set_fmt	    = cv2005_set_fmt,
    .get_selection	= cv2005_get_selection,
};

/*sensor driver subdev ops*/
static struct v4l2_subdev_ops cv2005_subdev_ops = {
    .core	= &cv2005_subdev_core_ops,
    .video	= &cv2005_subdev_video_ops,
    .pad	= &cv2005_subdev_pad_ops,
};

/*
 * sensor_of_parse -
 * parse node of device
 *
 * @client:			pointor to i2c client
 * @priv:			sensor struct
 */
static int sensor_of_parse(struct i2c_client *client, struct cv2005_priv *priv)
{
    struct device_node *np = client->dev.of_node;

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
 * @priv:			pointer to pirvate structure
 *
 * @RETURN:	0-match fail; 1-match success
 */
static int cv2005_match(struct cv2005_priv *priv)
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
    struct cv2005_priv* t_priv = NULL;
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
 * @client:			pointor to i2c client
 * @did:			driver ids
 */
static int cv2005_probe(struct i2c_client *client,
            const struct i2c_device_id *did)
{
    struct cv2005_priv	*priv;
    struct i2c_adapter	*adapter = to_i2c_adapter(client->dev.parent);
    int			ret;

    pr_info("%s %s %s\n", __func__, __DATE__, __TIME__);

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_err(&adapter->dev,
            "cv2005: I2C-Adapter doesn't support SMBUS\n");
        return -EIO;
    }

    priv = devm_kzalloc(&client->dev, sizeof(struct cv2005_priv), GFP_KERNEL);
    if (!priv) {
        dev_err(&adapter->dev,
            "Failed to allocate memory for private data!\n");
        return -ENOMEM;
    }

    /*parse of node*/
    sensor_of_parse(client, priv);

    /*
     * the current sensor slave address to client.
     * the address from dts may incorrect
     * */
    if(client->addr != SENSOR_I2C_ADDR0 && client->addr != SENSOR_I2C_ADDR1)
        client->addr = SENSOR_I2C_ADDR;
    priv->cb_info.cb = &cv2005_cb;
    priv->cb_info.arg = priv;
    priv->client = client;

    if (!client->dev.of_node) {
        dev_err(&client->dev, "Missing platform_data for driver\n");
        ret = -EINVAL;
        goto err_clk;
    }

    if (!cv2005_match(priv)) {
        ret = -ENODEV;
        goto err_clk;
    }

    /*subdev init*/
    v4l2_i2c_subdev_init(&priv->subdev, client, &cv2005_subdev_ops);
    priv->subdev.flags |= /*V4L2_SUBDEV_FL_HAS_EVENTS | */V4L2_SUBDEV_FL_HAS_DEVNODE;
    v4l2_ctrl_handler_init(&priv->hdl, 2);
    v4l2_ctrl_new_std(&priv->hdl, &cv2005_ctrl_ops,
            V4L2_CID_VFLIP, 0, 1, 1, 0);
    v4l2_ctrl_new_std(&priv->hdl, &cv2005_ctrl_ops,
            V4L2_CID_HFLIP, 0, 1, 1, 0);

    /*private ctrl*/
    priv->ctrl_sensor_get_id = v4l2_ctrl_new_custom(&priv->hdl, &config_sensor_get_id, NULL);
    if (!priv->ctrl_sensor_get_id)
        pr_err("creat ctrl_sensor_get_id fail\n");

    priv->subdev.ctrl_handler = &priv->hdl;
    if (priv->hdl.error) {
        dev_err(&client->dev, "Hdl error\n");
        ret = priv->hdl.error;
        goto err_clk;
    }

    ret = cv2005_video_probe(client);
    if (ret < 0) {
        dev_err(&client->dev, "cv2005 probe fail\n");
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

    dev_info(&adapter->dev, "cv2005 Probed success, subdev:%p\n", &priv->subdev);

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
 * @client:			pointor to i2c client
 */
static int cv2005_remove(struct i2c_client *client)
{
    struct cv2005_priv       *priv = to_cv2005(client);

    pr_info("%s %d\n", __func__, __LINE__);

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

static const struct i2c_device_id cv2005_id[] = {
    { "cv2005", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, cv2005_id);

static const struct of_device_id cv2005_of_match[] = {
    /*donot changed compatible, must the same as dts*/
    {.compatible = "anyka,sensor0", },
    {},
};
MODULE_DEVICE_TABLE(of, cv2005_of_match);

static struct i2c_driver cv2005_i2c_driver = {
    .driver = {
        .name = "cv2005",
        .of_match_table = of_match_ptr(cv2005_of_match),
    },
    .probe    = cv2005_probe,
    .remove   = cv2005_remove,
    .id_table = cv2005_id,
};

module_i2c_driver(cv2005_i2c_driver);

MODULE_DESCRIPTION("SoC Camera driver for cv2005 sensor");
MODULE_AUTHOR("Anyka Microelectronic");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("1.0.01");
/***
v1.0.02:更新翻转逻辑修复flip&mirror异常


***/


/*sensor i2c write*/
static int __cv2005_write(const struct i2c_client* client, int reg, int value)
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

static int _cv2005_write(
    struct i2c_client* client, int reg, int value, int sensor_switch)
{
    struct i2c_client* client_p = client;
    unsigned short tmp_addr = client_p->addr;
    int ret = 0;

    if (sensor_switch == 1) {
        client_p->addr = SENSOR_I2C_ADDR0;
        __cv2005_write(client_p, reg, value);
        client_p->addr = SENSOR_I2C_ADDR1;
        ret = __cv2005_write(client_p, reg, value);
        client_p->addr = tmp_addr;
        return ret;
    } else {
        return __cv2005_write(client_p, reg, value);
    }
}

static int cv2005_write_cisx(struct cv2005_priv* t_priv, int reg, int value)
{
    switch (t_priv->subdev.devnode->num) {
        case 0:
            return _cv2005_write(t_priv->client, reg, value, sensor0_switch);
            break;

        case 1:
            return _cv2005_write(t_priv->client, reg, value, sensor1_switch);
            break;

        default:
            break;
    }
    return 0;
}

static int cv2005_write(struct i2c_client* client, int reg, int value)
{
    struct cv2005_priv* t_priv = NULL;
    //struct i2c_client* client_p;

    list_for_each_entry(t_priv, &privs_list, list)
    {
        if (!t_priv || !t_priv->subdev.devnode)
            continue;

        if (t_priv->master_or_slave == SLAVER_MODE) {
            // Dual CSI
            if (client == t_priv->client) {
                return cv2005_write_cisx(t_priv, reg, value);
            }
        } else {
            // single CSI
            // when use Two_way_I2C + MIPI switch need write register twice.
            cv2005_write_cisx(t_priv, reg, value);
        }
    }
    return 0;
}

/*sensor i2c read*/
static int cv2005_read(const struct i2c_client *client, int reg)
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

static void cv2005_agc_mode(struct cv2005_priv *priv)
{
    int value;
    struct i2c_client *client = priv->client;

    value = cv2005_read(client, GAIN_EN_REG);
    value |= 0x1;
    cv2005_write(client, GAIN_EN_REG, value);

    // value = 0x32;
    // cv2005_write(client, BLC_REG, value);
}

/*
 * The sensor must set callbacks
 *
 * */
static int cv2005_sensor_init_func(void *arg, const AK_ISP_SENSOR_INIT_PARA *npara)
{
    int i;
    AK_ISP_SENSOR_REG_INFO *preg_info;
    struct cv2005_priv *priv = arg;
    struct i2c_client *client = priv->client;
    AK_ISP_SENSOR_INIT_PARA *para = &priv->para;

    if (para->num <= 0 && npara)
        para = (void *)npara;

    preg_info = para->reg_info;
    // preg_info = (void*)cv2005_1920x1080_30fps;
    // para->num = sizeof(cv2005_1920x1080_30fps) / sizeof(cv2005_1920x1080_30fps[0]) / 2;

    for (i = 0; i < para->num; i++) {
#if 0
        {
            int value;

            value = cv2005_read(client, preg_info->reg_addr);
            pr_err("reg:%x read value:%x\n", preg_info->reg_addr, value);
        }
#endif
        cv2005_write(client, preg_info->reg_addr, preg_info->value);
#if 0
        {
            int value;

            value = cv2005_read(client, preg_info->reg_addr);
            pr_err("reg:%x write value:%x, read back value:%x\n",
                    preg_info->reg_addr, preg_info->value, value);
        }
#endif

        preg_info++;
    }

    cv2005_agc_mode(priv);

    priv->aec_parms.reg_frame_vts
        = ((cv2005_read(client, VTS_REG_H) & 0xf) << 16 \
            |cv2005_read(client, VTS_REG_M) << 8 \
            | cv2005_read(client, VTS_REG_L));
    priv->fps_info.to_fps = priv->aec_parms.reg_frame_vts;
    priv->fps_info.to_fps_value = priv->aec_parms.reg_frame_vts;
    // pr_err("0x320e:%02d(0x%04x)", (cv2005_read(client, 0x320e)&0x7f), (cv2005_read(client, 0x320e)&0x7f));
    // pr_err("0x320f:%02d(0x%04x)", (cv2005_read(client, 0x320f)&0xff), (cv2005_read(client, 0x320f)&0xff));
    // pr_err("priv->fps_info.to_fps_value:%02d(0x%04x)", priv->fps_info.to_fps_value, priv->fps_info.to_fps_value);

    priv->aec_parms.reg_frame_hts
        = (cv2005_read(client, HTS_REG_H) << 8 \
            | cv2005_read(client, HTS_REG_L));
    // pr_err("0x320c:%02d(0x%04x)", (cv2005_read(client, 0x320c)&0xff), (cv2005_read(client, 0x320c)&0xff));
    // pr_err("0x320d:%02d(0x%04x)", (cv2005_read(client, 0x320d)&0xff), (cv2005_read(client, 0x320d)&0xff));
    // pr_err("priv->fps_info.default_hts:%02d(0x%04x)", priv->fps_info.default_hts, priv->fps_info.default_hts);

    priv->aec_parms.pclk_freq = \
        priv->aec_parms.reg_frame_hts \
        * priv->aec_parms.reg_frame_vts * MAX_FPS;

    priv->aec_parms.calc_vts_tmp = \
        (priv->fps_info.to_fps_value * MAX_FPS * 100);

    pr_info("pclk %d vts %d hts %d calc_tmp %d\n",
        priv->aec_parms.pclk_freq, priv->aec_parms.reg_frame_vts,
        priv->aec_parms.reg_frame_hts, priv->aec_parms.calc_vts_tmp);

    priv->fps_info.current_fps = MAX_FPS;
    priv->fps_info.to_fps = priv->fps_info.current_fps;
    priv->fps_info.to_fps_value = priv->aec_parms.reg_frame_vts;
    priv->fps_info.reg_fps_value = priv->aec_parms.reg_frame_vts;

    return 0;
}

/*read sensor register*/
static int cv2005_sensor_read_reg_func(void *arg, const int reg_addr)
{
    struct cv2005_priv *priv = arg;
    struct i2c_client *client = priv->client;

    return cv2005_read(client, reg_addr);
}
/*write sensor register*/
static int cv2005_sensor_write_reg_func(void *arg, const int reg_addr, int value)
{
    struct cv2005_priv *priv = arg;
    struct i2c_client *client = priv->client;

    return cv2005_write(client, reg_addr, value);
}
/*read sensor register, NO i2c ops*/
static int cv2005_sensor_read_id_func(void *arg)   //no use IIC bus
{
    return SENSOR_ID;
}

static int aec_parms_init(struct cv2005_priv *priv)
{
    priv->aec_parms.curr_again_level = 0;
    priv->aec_parms.curr_again_10x = -1;
    priv->aec_parms.target_exp_ctrl = 0;
    priv->aec_parms.curr_2x_dgain = 0;
    priv->aec_parms.curr_corse_gain = -1;

    priv->aec_parms.reg_frame_hts = 0;
    priv->aec_parms.reg_frame_vts = 0;
    priv->aec_parms.pclk_freq = 0;
    priv->aec_parms.calc_vts_tmp = 0;

    return 0;
}

//#if AGAIN_EN
#if 1
//#define cv2005_REG_SPLIT_GAIN_EN 0x3109
// #define cv2005_REG_SPLIT_DGAIN_L 0x311C
// #define cv2005_REG_SPLIT_DGAIN_H 0x311D
// #define cv2005_REG_SPLIT_AGAIN   0x3118

static const int internal_multiple_table[241] = \
{
    256, 264, 274, 283, 293, 304, 314, 326, 337, 349, 361, 374, 387, 401, 415, 429, 444, 460, 476, 493, 510, 528, 547, 566, 586, 607, 
    628, 650, 673, 697, 721, 746, 773, 800, 828, 857, 887, 918, 951, 984, 1019, 1054, 1092, 1130, 1170, 1211, 1253, 1297, 1343, 1390, 
    1439, 1490, 1542, 1596, 1652, 1710, 1771, 1833, 1897, 1964, 2033, 2104, 2178, 2255, 2334, 2416, 2501, 2589, 2680, 2774, 2872, 2973,
    3077, 3185, 3297, 3413, 3533, 3657, 3786, 3919, 4057, 4199, 4347, 4500, 4658, 4822, 4991, 5167, 5348, 5536, 5731, 5932, 6141, 6356, 
    6580, 6811, 7050, 7298, 7555, 7820, 8095, 8379, 8674, 8979, 9294, 9621, 9959, 10309, 10671, 11046, 11435, 11836, 12252, 12683, 13129, 
    13590, 14068, 14562, 15074, 15604, 16152, 16720, 17307, 17915, 18545, 19197, 19871, 20570, 21293, 22041, 22816, 23617, 24447, 25306,
    26196, 27116, 28069, 29056, 30077, 31134, 32228, 33361, 34533, 35747, 37003, 38303, 39649, 41043, 42485, 43978, 45523, 47123, 48779, 
    50494, 52268, 54105, 56006, 57974, 60012, 62121, 64304, 66564, 68903, 71324, 73831, 76425, 79111, 81891, 84769, 87748, 90832, 94024, 
    97328, 100748, 104289, 107954, 111748, 115675, 119740, 123948, 128303, 132812/* MAX = 519*256(�?)  */, 0x7FFFFFFF,
};

static unsigned char internal_again_table[81]  = \
{
	0,9,17,25,33,41,48,55,62,68,75,81,87,93,98,104,109,114,119,123,128,
	132,136,140,144,148,152,155,159,162,165,168,171,174,177,180,182,185,
	187,189,192,194,196,198,200,202,204,206,207,209,210,212,214,215,216,
	218,219,220,221,223,224,225,226,227,228,229,230,231,232,232,233,234,
	235,235,236,237,237,238,239,239,240,
};

static unsigned short internal_dgain_table[100] = \
{
	66,68,70,73,75,78,81,84,87,90,93,96,99,103,106,110,114,118,122,126,131,
	136,140,145,150,156,161,167,173,179,185,191,198,205,212,220,228,236,244,
	252,261,270,280,290,300,310,321,333,344,357,369,382,395,409,424,439,454,
	470,486,504,521,540,559,578,598,620,641,664,687,711,736,762,789,817,845,
	875,906,938,971,1005,1040,1077,1114,1154,1194,1236,1280,1325,1371,1419,
	1469,1521,1574,1630,1687,1746,1807,1871,1937,2005
};

static int cv2005_internal_set_gain(struct i2c_client *sd, int value)
{
    int ret = 0;
    unsigned char again = value & 0xff;
    again = (again > 180) ? 180:again;

    pr_debug("%s: line:%d again %d\n", __func__, __LINE__, again);

    if(again <= 80)	//80
    {
        ret = cv2005_write(sd,AGAIN_REG,internal_again_table[again]);
        ret |= cv2005_write(sd,DGAIN_REG_H,0);

        if(again == 50 || again == 54 || again == 58 ||	again == 69 \
            || again == 70 || 	again == 73 || again == 79)
                ret |= cv2005_write(sd,DGAIN_REG_L,65);
        else if((again == 76))
            ret |= cv2005_write(sd,DGAIN_REG_L,66);
        else
            ret |= cv2005_write(sd,DGAIN_REG_L,64);
    }
    else
    {
        ret = cv2005_write(sd,DGAIN_REG_L, \
                (unsigned char)(internal_dgain_table[again - sizeof(internal_again_table)/sizeof(internal_again_table[0])]&0xFF));
        ret |= cv2005_write(sd,DGAIN_REG_H, \
                (unsigned char)(internal_dgain_table[again - sizeof(internal_again_table)/sizeof(internal_again_table[0])]>>8));
    }

    return ret;
}
#endif

/*set sensor again*/
static int cv2005_sensor_update_a_gain_func(void *arg, const unsigned int a_gain)
{
    /*
     * max again < 16 times = 4096
     * */
    struct cv2005_priv *priv = arg;
    struct i2c_client *client = priv->client;
    //unsigned int corse_gain, fine_gain;
    int i,ret,reg_vale;
       //Ankya平台 256 代表sensor的 X1 倍增益.
    unsigned int gain;
    unsigned int again_reg,again_real,dgain_want,dgain_reg,dgain_real;
    unsigned int sensor_again_value,sensor_dgain_value_low,sensor_dgain_value_high;
	gain = a_gain * 4; //精度扩张到1024.
    // sensor_gain = gain/1024;

    /////again_real = 256/(256-reg)
    /////dgain_real = reg / 64

#if AGAIN_EN
   if(gain <= 32*1024) {
        again_reg = 256 - 256/(gain/1024);       //=241, 假如是17倍.
        again_real = 256*1024/(256-again_reg);   //=16 * 1024
        dgain_want = gain * 1024 /again_real;    //=1024    16X1024x1024/16
        dgain_reg  = dgain_want * 64 /1024 ;     //=64        1024x1024x64/1024
    } else {
        again_reg = 0xf8;                  //假如是48倍.
        again_real = 32 * 1024;                   //32*1024
        dgain_want = gain * 1024 /again_real ;      //48 * 1024 / (32*1024) * 1024 = 1536.
        dgain_reg  = dgain_want * 64 /1024 ;      //1536*64/1024 = 96.
    }

    if (dgain_reg < 64){
        dgain_reg = 64;
        // dgain_real = 1;
    }

    sensor_again_value = again_reg;
    sensor_dgain_value_low = dgain_reg & 0xff;
    sensor_dgain_value_high = (dgain_reg >> 8) & 0xff;
    pr_info("again: %d, dgain_l: %d, dgain_h: %d\n", sensor_again_value, sensor_dgain_value_low, sensor_dgain_value_high);
    ret = cv2005_write(client,AGAIN_REG,sensor_again_value & 0xFF);
    ret |= cv2005_write(client,DGAIN_REG_L,sensor_dgain_value_low & 0xFF);
    ret |= cv2005_write(client,DGAIN_REG_H,sensor_dgain_value_high & 0xFF);

    if(ret){
        //错误返回
        return 0;
    }
#endif

    return A_GAIN_EFFECT_FRAMES;
}

/*set sensor dgain*/
static int cv2005_sensor_update_d_gain_func(void *arg, const unsigned int d_gain)
{
    return 0;
}

static int __set_reg_exp_time(struct cv2005_priv *priv, unsigned int exp_time)
{
    int ret = 0;
    struct i2c_client *client = priv->client;
    int exp_h,exp_m,exp_l;
    struct v4l2_control ctrl_max_exp;
    int max_exp = priv->fps_info.current_fps;

    if (exp_time < EXP_MIN)
        exp_time = EXP_MIN;
    if (exp_time > (65535))
        exp_time = 65535; // 2^16

    //CV2005 do not need 2N.
    // exp_time /=2; 
    // exp_time *= 2;

    pr_debug("%s, exp:%d\n", __func__, exp_time);

    exp_l = ((exp_time)) & 0xff;
    exp_m = (exp_time >> 8) & 0xff;
    exp_h = (exp_time >> 16) & 0xf;

#if EXP_EN
    cv2005_write(client, EXP_REG_H, exp_h);
    cv2005_write(client, EXP_REG_M, exp_m);
    cv2005_write(client, EXP_REG_L, exp_l);
#endif
    //priv->aec_parms.target_exp_ctrl = exp_time;

    return ret;
}

static int __set_reg_frame_vts(struct cv2005_priv *priv, int vts)
{
    int ret = 0;
    struct i2c_client *client = priv->client;
    if (vts < 8)
        vts = 8;
    pr_debug("%s: vts:%d \n", __func__, vts);

#if FPS_EN
    cv2005_write(client, VTS_REG_H, ((vts >> 16) & 0xf));
    cv2005_write(client, VTS_REG_M, (vts >> 8));
    cv2005_write(client, VTS_REG_L, (vts & 0xff));
#endif

    priv->fps_info.to_fps_value = vts;
    priv->fps_info.reg_fps_value = vts;

    return ret;
}

static int __get_reg_frame_vts(struct cv2005_priv *priv)
{
    struct i2c_client *client = priv->client;

    return ((cv2005_read(client, VTS_REG_H) & 0xf) << 16 \
            | (cv2005_read(client, VTS_REG_M)) << 8 \
            | cv2005_read(client, VTS_REG_L));
}

static void update_frame_vts_and_exp_ctrl(struct cv2005_priv *priv)
{
    //struct i2c_client *client = priv->client;

    /*
     * exp_time=1 mean half line exptime
     * max exp_time = {R0x304a,R0x3049,R0x3048} - 2
     *
     * now:
     * at max_exptime_30fps
     * 	{R0x304a,R0x3049,R0x3048} - 2
     * 	= 0x4b0 - 10
     *  = 1200 - 10
     *  = 2400 - 10
     *  = 2390
     */

    int const cur_frame_vts     = __get_reg_frame_vts(priv);

    int const max_exp_ctrl      = priv->fps_info.to_fps_value - EXP_DECREASE_LINES;   //Vts-5

    //int const target_frame_vts  = (priv->fps_info.to_fps_value);
    int target_frame_vts  = (priv->fps_info.to_fps_value);

    int target_exp_ctrl = (priv->aec_parms.target_exp_ctrl > max_exp_ctrl) ? \
                            max_exp_ctrl : priv->aec_parms.target_exp_ctrl ;

    int target_exp_reg_value;

    // target_frame_vts = target_frame_vts / 2 * 2;
    /**注意：建议FRAME_LENGTH设置成偶数(2N).  SHUTTER0没有奇偶数的限制。**/
    if ((target_frame_vts%2)!=0)
        target_frame_vts += 1;
    target_exp_reg_value = target_frame_vts - target_exp_ctrl;
    target_exp_reg_value = target_exp_reg_value / 2 * 2;

    if ((target_frame_vts <= 0) || (target_exp_reg_value <= 0))
        return;

    pr_debug("%s, target_frame_vts:%d, target_exp_ctrl:%d, target_exp_reg_value:%d\n", \
        __func__, target_frame_vts, target_exp_ctrl, target_exp_reg_value);

    if (target_frame_vts >= cur_frame_vts) {
        if (target_frame_vts != cur_frame_vts)
            __set_reg_frame_vts(priv, target_frame_vts);

        __set_reg_exp_time(priv, target_exp_reg_value);

    } else {
        __set_reg_frame_vts(priv, target_frame_vts);

        __set_reg_exp_time(priv, target_exp_reg_value);
    }
}

/*set sensor exp time*/
static int cv2005_sensor_updata_exp_time_func(void *arg, unsigned int exp_time)
{
    struct cv2005_priv *priv = arg;
    //struct i2c_client *client = priv->client;

    priv->aec_parms.target_exp_ctrl = exp_time;
    update_frame_vts_and_exp_ctrl(priv);

    return EXP_EFFECT_FRAMES;
}
/*sensor timer*/
static int cv2005_sensor_timer_func(void *arg)
{
    return 0;
}
/*standby in*/
static int cv2005_sensor_set_standby_in_func(void *arg)
{
    return 0;
}
/*standby out*/
static int cv2005_sensor_set_standby_out_func(void *arg)
{
    return 0;
}
/*low level read sensor ID, user i2c ops*/
static int cv2005_sensor_probe_id_func(void *arg)  //use IIC bus
{
    struct cv2005_priv *priv = arg;
    struct i2c_client *client = priv->client;
    int id;
    int value;

    value = cv2005_read(client, ID_REG_H);
    id = value << 8;

    value = cv2005_read(client, ID_REG_L);
    id |= value;

    pr_err("id:%x\n", id);

    if (id == ACTUAL_SENSOR_ID)
        return SENSOR_ID;


    return 0;
}
/*get resolution*/
static int cv2005_sensor_get_resolution_func(void *arg, int *width, int *height)
{
    *width = SENSOR_OUTPUT_WIDTH;
    *height = SENSOR_OUTPUT_HEIGHT;
    return 0;
}
/*get mclk*/
static int cv2005_sensor_get_mclk_func(void *arg)
{
    return SENSOR_MCLK;
}
/*get current fps*/
static int cv2005_sensor_get_fps_func(void *arg)
{
    struct cv2005_priv *priv = arg;

    return priv->fps_info.current_fps;
}
/*get valid coordinate*/
static int cv2005_sensor_get_valid_coordinate_func(void *arg, int *x, int *y)
{
    *x = SENSOR_VALID_OFFSET_X;
    *y = SENSOR_VALID_OFFSET_Y;
    return 0;
}
/*get bus type*/
static enum sensor_bus_type cv2005_sensor_get_bus_type_func(void *arg)
{
    return SENSOR_BUS_TYPE;
}
// static int get_ae_fast_default(struct ae_fast_struct *ae_fast)
// {
//     *ae_fast = ae_fast_default;
//     return 0;
// }
/*get self definded params*/
static int cv2005_sensor_get_parameter_func(void *arg, int param, void *value)
{
    int ret = 0;
    struct cv2005_priv *priv = arg;

    switch (param) {
        case GET_INTERFACE:
            *((int *)value) = MIPI_INTERFACE;
            break;

        case GET_IO_LEVEL:
            *((int *)value) = SENSOR_IO_LEVEL;
            break;

        case GET_MIPI_MHZ:
            *((int *)value) = MIPI_MHZ;
            break;

        case GET_MIPI_LANES:
            *((int *)value) = MIPI_LANES;
            break;

        case GET_RESET_GPIO:
            *((int *)value) = priv->gpio_reset;
            break;

        case GET_PWDN_GPIO:
            *((int *)value) = priv->gpio_pwdn;
            break;

        // case GET_AE_FAST_DEFAULT:
        //     get_ae_fast_default(value);
        //     break;

        // case GET_SCAN_METHOD:
        //     ret = -1;
        //     break;

        // case GET_DUAL_SYNC_MODE:
        //     //*((int*)value) = DUAL_SYNC_PWM_FREQ_FIXED;
        //     *((int*)value) =  DUAL_SYNC_BY_EFSYNC;
        //     break;

        case GET_MAX_FPS:
            *((int*)value) = MAX_FPS;
            break;

        case GET_FLIP_DROP_FRAMES:
            *((int*)value) = FLIP_DROP_FRAMES;
            break;

        case GET_REAL_HEIGHT:
            *((int*)value) = SENSOR_REAL_HEIGHT;
            break;
        
        // case GET_HBLIANK_CYCLE:
        //     *((int*)value) = HBLIANK_CYCLE;
        //     break;

        // case GET_STEPS_OF_1LINE_EXP:
        //     *((int*)value) = ONE_LINE_EXP_OF_STEPS;

        // case GET_MAX_EXP:
        //     *((int*)value) = MAX_EXP_OF_FPS30;
        //     break;

        // case GET_MAX_AGAIN:
        //     *((int*)value) = MAX_A_GAIN_X;
        //     break;

        // case GET_MAX_DGAIN:
        //     *((int*)value) = MAX_D_GAIN_X;
        //     break;

        // case GET_ONE_LINE_CYCLE:
        //     *((int*)value) = ONE_LINE_CYCLE;

        default:
            pr_err("%s param:%d not support\n", __func__, param);
            ret = -1;
            break;
    }

    return ret;
}

/*sensor power on*/
static int cv2005_sensor_set_power_on_func(void *arg)
{
    struct cv2005_priv *priv = arg;
    struct i2c_client *client = priv->client;

    if (priv->gpio_pwdn >= 0) {
        gpio_direction_output(priv->gpio_pwdn, !SENSOR_PWDN_LEVEL);
    }

    if (priv->gpio_reset >= 0) {
        //gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
        //msleep(50);
        gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
        msleep(5);
        gpio_direction_output(priv->gpio_reset, !SENSOR_RESET_LEVEL);
        //msleep(100);
    }

    aec_parms_init(priv);

    priv->dual.init_count = 0;
    priv->master_or_slave = SINGLE_MODE;

    return 0;
}

/*sensor power off*/
static int cv2005_sensor_set_power_off_func(void *arg)
{
    struct cv2005_priv *priv = arg;
    struct i2c_client *client = priv->client;

    if (priv->gpio_pwdn >= 0) {
        //gpio_direction_input(priv->gpio_pwdn);
        gpio_direction_output(priv->gpio_pwdn, SENSOR_PWDN_LEVEL);
    }

    if (priv->gpio_reset >= 0) {
        //gpio_direction_input(priv->gpio_reset);
        gpio_direction_output(priv->gpio_reset, SENSOR_RESET_LEVEL);
    }

    return 0;
}

/*set sensor fps*/
static int cv2005_fps_to_vts(struct cv2005_priv *priv, const int fps)
{

    /*
     * cv2005:
     * CV2005_linear_1920_1080_25fps_AD10_2lane_396Mbps_FrameLength2400_LineLength660_MCLK24Mhz.cleaned.ini
     *
     * mipi:
     * HTS=(0x320c,0x320d)=0x834=2100
     * VTS=(0x320e[6:0],0x320f)=0x4b0=1200‬ at 30fps
     * fps = pclk / HTS / VTS
     *
     * pclk = HTS*VTS*fps
     *      = 2100*1200*30fps
     *      = 75.6MHz
     *
     * so :
     * vts = pclk / HTS / fps
     *     = (75600000UL) / (2100) / fps
     *     = (36000*100)  / (fps*100)
     *     = (3600000UL)  / vts;   //vts=fps*100
     *
     */

    int vts = 0;
    int fps_tmp;

    pr_debug("%s %d fps:%d default_hts:%d tmp:%d\n", \
        __func__, __LINE__, fps, \
        priv->aec_parms.reg_frame_hts, \
        priv->aec_parms.calc_vts_tmp);
    pr_debug("%s %d pclk_freq:%d\n", __func__, __LINE__, priv->aec_parms.pclk_freq);

    switch (fps) {
   /*     case 12:
            fps_tmp = 1250;
            break;

        case 14:
            fps_tmp = 1428;
            break;
*/
        default:
            fps_tmp = fps * 100;
            break;
    }

    vts = (priv->aec_parms.calc_vts_tmp) / fps_tmp;

    return vts;
}

/*set sensor fps*/
static int cv2005_sensor_set_fps_func(void *arg, const int fps)
{
    int tmp;
    struct cv2005_priv *priv = arg;

    tmp = cv2005_fps_to_vts(priv, fps);

    if (tmp > 0) {
        priv->fps_info.to_fps_value = tmp;
        priv->fps_info.to_fps = fps;
    }
    return 0;
}

static void cv2005_set_fps_async(struct cv2005_priv *priv)
{
    //struct i2c_client *client = priv->client;

    if (priv->fps_info.to_fps != priv->fps_info.current_fps) {
        update_frame_vts_and_exp_ctrl(priv);
        priv->fps_info.current_fps = priv->fps_info.to_fps;
    }
}

static AK_ISP_SENSOR_CB cv2005_cb = {
    .sensor_init_func					= cv2005_sensor_init_func,
    .sensor_read_reg_func				= cv2005_sensor_read_reg_func,
    .sensor_write_reg_func				= cv2005_sensor_write_reg_func,
    .sensor_read_id_func				= cv2005_sensor_read_id_func,
    .sensor_update_a_gain_func			= cv2005_sensor_update_a_gain_func,
    .sensor_update_d_gain_func			= cv2005_sensor_update_d_gain_func,
    .sensor_updata_exp_time_func		= cv2005_sensor_updata_exp_time_func,
    .sensor_timer_func					= cv2005_sensor_timer_func,
    .sensor_set_standby_in_func			= cv2005_sensor_set_standby_in_func,
    .sensor_set_standby_out_func		= cv2005_sensor_set_standby_out_func,
    .sensor_probe_id_func				= cv2005_sensor_probe_id_func,
    .sensor_get_resolution_func			= cv2005_sensor_get_resolution_func,
    .sensor_get_mclk_func				= cv2005_sensor_get_mclk_func,
    .sensor_get_fps_func				= cv2005_sensor_get_fps_func,
    .sensor_get_valid_coordinate_func	= cv2005_sensor_get_valid_coordinate_func,
    .sensor_get_bus_type_func			= cv2005_sensor_get_bus_type_func,
    .sensor_get_parameter_func			= cv2005_sensor_get_parameter_func,
    .sensor_set_power_on_func			= cv2005_sensor_set_power_on_func,
    .sensor_set_power_off_func			= cv2005_sensor_set_power_off_func,
    .sensor_set_fps_func				= cv2005_sensor_set_fps_func,
};

static int sensor_id_func(void)
{
    return cv2005_sensor_read_id_func(NULL);
}

static char *sensor_if_func(void)
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
