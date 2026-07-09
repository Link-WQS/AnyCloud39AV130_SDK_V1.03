#ifndef __AK_ISP_APP_H__
#define __AK_ISP_APP_H__

/*
 *
 * ISP_APP_SET_LINKS : 用于绑定多个节点组成一个pipeline, 
 *                  例如vicap-》isp-》pp是一个pipeline
 * ISP_APP_GET_ID : 好的这个vicap的ID号
 */
 
//帧率控制结构体
/*typedef  struct ak_isp_frame_rate_attr {
    AK_U32   hight_light_frame_rate ;
    AK_U32   hight_light_max_exp_time ;
    AK_U32   hight_light_to_low_light_gain;
    AK_U32   low_light_frame_rate;
    AK_U32   low_light_max_exp_time;
    AK_U32   low_light_to_hight_light_gain;
}AK_ISP_FRAME_RATE_ATTR;*/

typedef struct sensor_reg {
    AK_U16 reg_addr;
    AK_U16 value;
}AK_ISP_SENSOR_REG;

typedef int (*daynight_mode_cb_t)(int dev_id, int exp_div_lumi, int cur_isp_mode);
typedef int (*fps_reset_cb_t)(int dev_id);
typedef void* (*malloc_cb_t)(unsigned long sz);
typedef void (*free_cb_t)(void* ptr);
typedef int (*ae_table_load_cb_t)(int dev_id,unsigned int *data, int len);
typedef int (*ae_table_update_cb_t)(int dev_id,unsigned int *data,int len,int offset, unsigned int ae_value);

struct app_fast_status {
    char cur_lumi;
    char is_stable;
    char conv_frames;//曝光收敛帧数
    struct ae_fast_struct cur_ae;
};

struct app_isp_fast_ae_cfg
{
    char bining_mode; /* 小窗模式使能标志 */
    unsigned int init_ae; /* 初始ae参数，可由上层设定或软光敏提供*/
    unsigned int init_awb; /* 初始awb参数 = (r_gain<<16)|(b_gain)*/
    char max_run_frames; /* 快启收敛运行最大帧数 */
    char ae_stable_range; /* ae收敛阈值，= abs(目标亮度-当前亮度)*/
    /* 软光敏白天夜视切换回调，硬光敏时可不注册，执行ircut和补光灯控制*/
    /*note: 白天模式，返回0；夜视开补光灯，返回1,；夜彩不开补光灯，返回2 */
    daynight_mode_cb_t daynight_cb;
    /*快启收敛完成后，需切回常规帧率，小窗模式时还需切回常规分辨率*/
    fps_reset_cb_t fps_reset_cb; 
    malloc_cb_t malloc_cb;
    free_cb_t free_cb;
    ae_table_load_cb_t ae_table_load_cb;
    ae_table_update_cb_t ae_table_update_cb;
    int reserved;
};

enum ak_isp_app_cmd {
    ISP_APP_SET_LINKS     = 0x100,
    ISP_APP_SET_MAX_DMA_SIZE,
    ISP_APP_SET_AE_FAST,
    ISP_APP_SET_FAST_AE_INIT,

    ISP_APP_GET_PAD_ID    = 0x200,
    ISP_APP_GET_AE_FAST,
    ISP_APP_GET_FAST_AE_STATUS,
};

typedef  struct ak_isp_tnr_buffer_size {
    AK_U32 y_plane;
    AK_U32 u_plane;
    AK_U32 v_plane;
}AK_ISP_TNR_BUF_SIZE;

#endif
