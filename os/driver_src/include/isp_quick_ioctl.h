/**********************************************************************
 * @FILENAME isp_quick_ioctl.h
 * @BRIEF isp quick start device include files.
 * Copyright (C) 2021 Anyka Guangzhou
 * @DATE 2021-10-29
 * @REF
 **********************************************************************/

#ifndef __ISP_QUICK_IOCTL_H__
#define __ISP_QUICK_IOCTL_H__
#include "dss.h"
#include "ak_npu_uapi.h"

//camera command
#define CMD_ISP_QUICK_START 'Q'

#define IO_ISP_QUICK_START_UPDATE_AE_TABLE  _IO(CMD_ISP_QUICK_START, 1)
#define IO_ISP_QUICK_START_DSS_GET_VI_INFO     _IO(CMD_ISP_QUICK_START, 2)
#define IO_ISP_QUICK_START_DSS_SET_VI_INFO     _IO(CMD_ISP_QUICK_START, 3)
#define IO_ISP_QUICK_START_DSS_GET_VI_DEV_INFO     _IO(CMD_ISP_QUICK_START, 4)
#define IO_ISP_QUICK_START_DSS_SET_VI_DEV_INFO    _IO(CMD_ISP_QUICK_START, 5)
#define IO_ISP_QUICK_START_DSS_GET_CHN_ATTR    _IO(CMD_ISP_QUICK_START, 6)
#define IO_ISP_QUICK_START_DSS_SET_CHN_ATTR     _IO(CMD_ISP_QUICK_START, 7)
#define IO_ISP_QUICK_START_DSS_GET_VENC_ATTR    _IO(CMD_ISP_QUICK_START, 8)
#define IO_ISP_QUICK_START_DSS_SET_VENC_ATTR    _IO(CMD_ISP_QUICK_START, 9)
#define IO_ISP_QUICK_START_DSS_GET_VIDEO_INFO    _IO(CMD_ISP_QUICK_START, 10)
#define IO_ISP_QUICK_START_DSS_SET_VIDEO_INFO    _IO(CMD_ISP_QUICK_START, 11)
#define IO_ISP_QUICK_START_DSS_GET_ISP_INFO    _IO(CMD_ISP_QUICK_START, 12)
#define IO_ISP_QUICK_START_DSS_SET_ISP_INFO    _IO(CMD_ISP_QUICK_START, 13)
#define IO_ISP_QUICK_START_DSS_GET_AUDIO_ATTR     _IO(CMD_ISP_QUICK_START, 14)
#define IO_ISP_QUICK_START_DSS_SET_AUDIO_ATTR     _IO(CMD_ISP_QUICK_START, 15)
#define IO_ISP_QUICK_START_DSS_GET_NPU_ATTR     _IO(CMD_ISP_QUICK_START, 16)
#define IO_ISP_QUICK_START_DSS_SET_NPU_ATTR     _IO(CMD_ISP_QUICK_START, 17)
#define IO_ISP_QUICK_START_DSS_GET_HW_LUMEN_SENSOR_ATTR     _IO(CMD_ISP_QUICK_START, 18)
#define IO_ISP_QUICK_START_DSS_INFO_COMMIT     _IO(CMD_ISP_QUICK_START, 19)
#define IO_ISP_QUICK_START_GET_VI_DEV_STATUS_ATTR     _IO(CMD_ISP_QUICK_START, 20)
#define IO_ISP_SWITCH_ISP_CAPTURE_MODE      _IO(CMD_ISP_QUICK_START, 21)
#define IO_ISP_QUICK_START_SWITCH_SENSOR    _IO(CMD_ISP_QUICK_START, 22)
#define IO_ISP_QUICK_START_REINIT_SENSOR    _IO(CMD_ISP_QUICK_START, 23)
#define IO_ISP_QUICK_START_FIRST_RUN_SENSOR _IO(CMD_ISP_QUICK_START, 24)
#define IO_ISP_STANDBY_IN_SENSOR            _IO(CMD_ISP_QUICK_START, 25)
#define IO_ISP_STANDBY_OUT_SENSOR           _IO(CMD_ISP_QUICK_START, 26)
#define IO_ISP_STANDBY_NIGTH_INFO           _IO(CMD_ISP_QUICK_START, 27)
#define IO_ISP_STANDBY_SENSOR_FSYNC         _IO(CMD_ISP_QUICK_START, 28)
#define IO_ISP_SET_CAPTURE_MODE             _IO(CMD_ISP_QUICK_START, 29)
#define IO_QUICK_START_GET_SVP_INFO         _IO(CMD_ISP_QUICK_START, 30)
#define IO_QUICK_START_SET_SVP_CTRL         _IO(CMD_ISP_QUICK_START, 31)


struct qs_wb_gain {
    unsigned short r_gain;
    unsigned short g_gain;
    unsigned short b_gain;
    signed short r_offset;
    signed short g_offset;
    signed short b_offset;
};

typedef enum
{
    NIGHT_MODE_IRFEED_LIGHT,
    NIGHT_MODE_WHITE_LIGHT,
    DAY_MODE_FORCE_NO_LIGHT,
    NIGHT_MODE_FORCE_IRFEED_LIGHT,
    DAY_MODE_IRFEED_LIGHT,
}T_NIGHT_MODE_LIGHT_SELECT;

struct qs_fast_ae_info {
    int sensor_exp_time;
    int sensor_a_gain;
    int sensor_d_gain;
    int isp_d_gain;
    struct qs_wb_gain wb;
    int dev_id;
    int reserved;
};


typedef struct isp_quick_start_night_info {
    unsigned int mode;   //1->nigth, 0->day
    unsigned int led_ctl_mode; //1->pwm控制,0->gpio控制
    unsigned int pin;          //GPIO控制,pin为GPIO，如果PWM控制，为PWM ID（0～4）
    unsigned int sensor_idex;
} isp_quick_start_night_info_t;

typedef struct isp_quick_start_switch_info {
    unsigned int capture_mode;//1->continue, 0->single
    unsigned int sensor_idex;
} isp_quick_start_switch_info_t;



struct ak_qs_ircut_led_status {
    struct dss_hw_ircut   ircut[DSS_IRCUT_MAX_NUM]; //ircut 硬件信息
    int ircut_a_curstate[DSS_IRCUT_MAX_NUM]; //当前控制ircut_a的gpio电平值
    int ircut_b_curstate[DSS_IRCUT_MAX_NUM]; //当前控制ircut_b的gpio电平值

    struct dss_hw_led     led[DSS_LED_MAX_NUM];//led 硬件信息
    int whiteled_curstate[DSS_LED_MAX_NUM]; //当前控制白光灯的gpio电平值 (PWM暂不支持)
    int irled_curstate[DSS_LED_MAX_NUM];    //当前控制红外灯的gpio电平值 (PWM暂不支持)

    unsigned int  cur_set_day_night_mode[DSS_VIDEO_MAX_NUM]; //白天/夜视

};

typedef struct ak_qs_vi_dev_info_t {
    struct ak_qs_ircut_led_status ircut_led_status_info;
} qs_vi_dev_info_t;
/**************************************/
/**************** dss.h ***************/
/**************************************/
typedef struct qs_vi_ioctl_info {
    struct ak_qs_vi_info qs_vi_info;           // 视频设备配置
} qs_vi_ioctl_info_t;

typedef struct qs_vi_dev_ioctl_info {
    struct ak_qs_vi_dev_info qs_vi_dev_info;           // 视频设备配置
} qs_vi_dev_ioctl_info_t;

typedef struct qs_vi_chn_ioctl_info {
    struct ak_qs_vi_chn_info qs_vi_chn_info;           // 视频设备配置
} qs_vi_chn_ioctl_info_t;

typedef struct qs_venc_ioctl_info {
    struct ak_qs_venc_info qs_venc_info;
} qs_venc_ioctl_info_t;

typedef struct qs_video_ioctl_info {
    struct ak_qs_video_info qs_video_info;
    int index;  //DSS_VENC_MAX_NUM        8
} qs_video_ioctl_info_t;

typedef struct qs_isp_ioctl_info {
    struct ak_qs_isp_info isp_info;
} qs_isp_ioctl_info_t;

typedef struct qs_hw_ircut_ioctl_info {
    struct dss_hw_ircut ircut;
    int index;  //DSS_IRCUT_MAX_NUM       4
} qs_hw_ircut_ioctl_info_t;

typedef struct qs_hw_led_ioctl_info {
    struct dss_hw_led led;
    int index;   //DSS_LED_MAX_NUM         4
} qs_led_ioctl_info_t;

typedef struct qs_hw_lumen_sensor_ioctl_info {
    struct dss_hw_lumen_sensor  hw_lumen_sensor;
    int index; //DSS_MAX_AIN_NUM       2
} qs_hw_lumen_sensor_ioctl_info_t;

typedef struct qs_audio_ioctl_info {
    struct ak_qs_audio_info audio_info;
} qs_audio_ioctl_info_t;

typedef struct qs_npu_ioctl_info {
    struct ak_qs_npu_info npu_info;
} qs_npu_ioctl_info_t;


// 定义最大历史帧数，防止用户空间结构体过大
#define SVP_PROCESS_MAX_FRAMES  32
#define SVP_MAX_TARGET_BOXES  64

typedef struct quick_start_svp_target_boxes { //coordinate values
    unsigned long left;       //left_top point, horizontal axis
    unsigned long top;        //left_top point, vertical axis
    unsigned long right;      //right_bottom point, horizontal axis
    unsigned long bottom;     //right_bottom point, vertical axis
    unsigned int label;
    unsigned int score;       //score
    unsigned int obj_id;      // 对象ID（用于人脸识别等场景）
    char obj_name[32];        // 对象名称（如具体人员姓名）
} quick_start_svp_target_boxes_t;


typedef struct quick_start_svp_result{
    int total_num;
    int frame_seq;              // 帧序列号
    unsigned long timestamp_ms; // 时间戳 (毫秒)
    quick_start_svp_target_boxes_t target_boxes[SVP_MAX_TARGET_BOXES];
}quick_start_svp_result_t;


struct quick_start_svp_info
{
    int chn_id;
    int total_frame;
    int bSvpExit;
    quick_start_svp_result_t results[SVP_PROCESS_MAX_FRAMES];  //每帧的检测结果
};




struct quick_start_ctrl_svp
{
    int dev_id;
    int chn_id;
    int svp_enable;
};

#define ERR_ENTRY_NOT_EXIST   40  /*Error index Invalid*/
#endif

/*end of file*/
