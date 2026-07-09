#ifndef __SENSOR_CMD_H__
#define __SENSOR_CMD_H__

/*this is the include file for sensor subdev*/
#if defined(CONFIG_MACH_KM01A)
#include "mach/km01a/ak_isp_ioctl.h"

#define SWITCH_INFO_PATH                 "/etc/config/switch_info.bin"
#else
#define SWITCH_INFO_PATH                 "/etc/config/switch_info.bin"
#endif

#define AKSENSOR_MAGIC 'S'

/***********************************************
 * CMD for structv4l2_subdev_core_ops -->ioctl
 ***********************************************/
/*set cmd*/
#define AK_SENSOR_SET_INIT  _IOW(AKSENSOR_MAGIC,  1, AK_ISP_SENSOR_INIT_PARA)

/*get cmd*/
#define AK_SENSOR_GET_MAX_EXP_FOR_FPS   \
        _IOWR(AKSENSOR_MAGIC, 0x100, struct sensor_max_exp_for_fps)
#define AK_SENSOR_GET_SLAVE_MAX_EXP_FOR_FPS   \
        _IOWR(AKSENSOR_MAGIC, 0x101, struct sensor_slave_max_exp_for_fps)
#define AK_SENSOR_GET_AE_AWB_CONVERT_COEF   \
        _IOWR(AKSENSOR_MAGIC, 0x102, struct sensor_ae_awb_dynamic_map_policy_t)


/***********************************************
 * CMD for struct v4l2_ctrl_config
 ***********************************************/
enum sensor_cmd {
    SENSOR_GET_ID = 0x910000 | 0x1,
};

struct sensor_max_exp_for_fps {
    int fps;
    int max_exp;
};
struct sensor_slave_max_exp_for_fps {
    int fps;
    int max_exp;
};

#if defined(CONFIG_MACH_KM01A)
typedef struct adc_value_region
{
    int less_adc_value;
    int top_adc_value;
}adc_value_region_t;

typedef struct adc_ae_para_table
{
    adc_value_region_t adc_region;
    unsigned int ae_para;
}adc_ae_para_table_t;





//mis_chn info
//ae
//venc_conf
typedef struct header
{
    unsigned int check_sum;
    unsigned int file_size;
    unsigned int version;
    unsigned int reserved;
}header_info;
#elif defined(CONFIG_MACH_AK3918AV130)

typedef struct adc_value_region
{
    int less_adc_value;
    int top_adc_value;
}adc_value_region_t;

typedef struct isp_switch_info
{
    unsigned int first_sensor_idex;
    unsigned int reserved;
}isp_switch_info_t;


typedef struct adc_ae_para_table
{
    adc_value_region_t adc_region;
    struct ae_fast_struct ae_para;
}adc_ae_para_table_t;




typedef struct
{
    /* data */
    unsigned int ae_convert_mul;
    unsigned int ae_convert_div;
}ae_convert_coef_t;

/* 单通道系数结构体 */
typedef struct
{
    unsigned int mul;
    unsigned int div;
} awb_channel_coef_t;

/* AWB 总系数结构体 (包含 R/G/B 三通道) */
typedef struct
{
    awb_channel_coef_t r;  /* R 通道系数 */
    awb_channel_coef_t g;  /* G 通道系数 */
    awb_channel_coef_t b;  /* B 通道系数 */
} awb_convert_coef_t;

typedef struct sensor_ae_awb_dynamic_map_policy_t
{
    /* data */
    ae_convert_coef_t ae_convert_coef_group[4];
    int ae_convert_group_num;

    awb_convert_coef_t awb_convert_coef;

    char dynamic_map_enable;
}ae_awb_dynamic_map_policy_t;



//mis_chn info
//ae
//venc_conf
typedef struct header
{
    unsigned int check_sum;
    unsigned int file_size;
    unsigned int version;
    unsigned int reserved;
}header_info;
#endif
#endif
/*end of file*/
