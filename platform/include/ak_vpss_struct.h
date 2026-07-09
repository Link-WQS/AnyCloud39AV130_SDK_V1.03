#ifndef __AK_VPSS_STRUCT_H__
#define __AK_VPSS_STRUCT_H__

#if defined  __CHIP_AK3918AV130_SERIES
#include "ak_vpss_struct_av130.h"
#elif defined __CHIP_KM01A_SERIES
#include "ak_vpss_struct_km01a.h"
#endif

#define NIGHT_ARRAY_NUM		        5
#define DAY_ARRAY_NUM		        10

enum mode_state {
    STATE_DAY,
    STATE_NIGHT,
};


enum vpss_mask_color_type {
    VPSS_MASK_ORIGINAL_COLOR = 0,	/* masked with original color*/
    VPSS_MASK_MOSAIC_VIDEO,			/* masked with mosaic video data*/
};


enum vpss_effect_type {
    /* HUE to SHARP, value: [-50, 50], 0 means use the value in ISP config file */
    VPSS_EFFECT_HUE = 0x00,
    VPSS_EFFECT_BRIGHTNESS,
    VPSS_EFFECT_SATURATION,
    VPSS_EFFECT_CONTRAST,
    VPSS_EFFECT_SHARP,
    VPSS_EFFECT_WDR,

    VPSS_STYLE_ID,		//[0, 2]
    VPSS_POWER_HZ,		//50 or 60
    VPSS_FLIP_MIRROR    //[0, 3]
};

enum vpss_gain_stat {
    VPSS_GAIN_NO_CHANGE_STAT = 0,
    VPSS_GAIN_LOW_STAT,
    VPSS_GAIN_HIGH_STAT,
    VPSS_GAIN_MID_STAT,
};


struct vpss_isp_sensor_reg_info {
    unsigned short reg_addr;
    unsigned short value;
};



struct vpss_mask_masic_attr{
    unsigned short	mosai_size_hor;
    unsigned short	mosai_size_vec;
};

struct vpss_mask_color_info {
    /* defined by enum vpss_mask_color_type*/
    unsigned char color_type;

    /* range [0, 0xff]. if 0xff mask video opaquely, then can not see image */
    unsigned char mk_alpha;

    /* range [0, 0xff] */
    unsigned char r_mk_color;
    /* range [0, 0xff] */
    unsigned char g_mk_color;
    /* range [0, 0xff] */
    unsigned char b_mk_color;

    struct vpss_mask_masic_attr masic_attr;
};

struct vpss_mask_area {
    unsigned short start_xpos;
    unsigned short end_xpos;
    unsigned short start_ypos;
    unsigned short end_ypos;
    unsigned char enable;
};

struct vpss_mask_area_info {
    struct vpss_mask_area main_mask[VPSS_MASK_AREA_MAX];
};


struct ak_auto_day_night_threshold {
    int day_to_night_lum;	// day to night lum value
    int night_to_day_lum;	// night to day lum value
    int night_cnt[NIGHT_ARRAY_NUM];	// awb night cnt array
    int day_cnt[DAY_ARRAY_NUM];	// awb day cnt array
    int lock_time;					// locke night status time
    int quick_switch_mode;			// quick switch mode
    int day2night_sleep_time;       // day change to night sleep time
    int night2day_sleep_time;       // night change to day sleep time
};

struct ak_soft_ps_attr {
    unsigned int    day_to_night_lum;	// day to night lum value
    unsigned int    night_to_day_lum;	// night to day lum value
    unsigned int    awb_night_dis_min;
    unsigned int    awb_night_dis_max;
    unsigned int    day_check_frame_num;
    unsigned int    night_check_frame_num;
    unsigned int    exp_stable_check_frame_num;
    unsigned int    stable_wait_timeout_ms;
    unsigned int    gain_r;
    int             offset_r;
    unsigned int    gain_b;
    int             offset_b;
    unsigned int    gain_rb;
    int             offset_rb;
    unsigned int    blk_check_flag;
    unsigned int    low_lumi_th;
    unsigned int    high_lumi_th;
    unsigned int    non_infrared_ratio_th;
    unsigned int    highlight_lumi_th;
    unsigned int    highlight_blk_cnt_max;
    unsigned int    lock_cnt_th;
    unsigned int    lock_time_ms;
    unsigned int    revised_dis_max_ratio;
};

struct ak_non_infrared_ratio_attr {
    unsigned int    awb_night_dis_max;
    unsigned int    gain_r;
    int             offset_r;
    unsigned int    gain_b;
    int             offset_b;
    unsigned int    gain_rb;
    int             offset_rb;
    unsigned int    low_lumi_th;
    unsigned int    high_lumi_th;
    unsigned int    non_infrared_ratio_th;
    unsigned int    revised_dis_max_ratio;
};

#endif

/* end of file */
