#ifndef __AK_VPSS_H__
#define __AK_VPSS_H__

#include "ak_vpss_struct.h"



/********************** public *******************************/

/**
 * ak_vpss_get_version - get vpss version
 * return: version string
 */
const char *ak_vpss_get_version(void);

/**
 * ak_vpss_get_ispsdk_version - get ispsdk version
 * return: version string
 * notes:
 */
const char* ak_vpss_get_ispsdk_version(void);


/********************** md *******************************/

/**
 * ak_vpss_md_get_stat: get motion detection stat params
 * @dev[IN]: dev id
 * @md[OUT]: md params
 * return: 0 success, otherwise error code
 * notes:
 */
int ak_vpss_md_get_stat(int dev, struct vpss_md_info *md);

/********************** od *******************************/

/**
 * ak_vpss_od_get: get occlusion detection params
 * @dev[IN]: dev id
 * @od[OUT]: od params
 * return: 0 success, otherwise error code
 * notes:
 */
int ak_vpss_od_get(int dev, struct vpss_od_info *od);


/********************** af stat *******************************/

/**
 * ak_vpss_af_get_stat: get af stat info
 * @dev[IN]: dev id
 * @af_stat[OUT]: af_stat info
 * return: 0 success, otherwise error code
 * notes:
 */
int ak_vpss_af_get_stat(int dev, struct vpss_af_stat_info *af_stat);

/**
 * ak_vpss_af_set_attr: set af attr
 * @dev[IN]: dev id
 * @af_stat[IN]: af_attr
 * return: 0 success, otherwise error code;
 * notes:
 */
int ak_vpss_af_set_attr(int dev, struct vpss_af_attr *af_attr);

/**
 * ak_vpss_af_get_attr: set af attr
 * @dev[IN]: dev id
 * @af_stat[OUT]: af_attr
 * return: 0 success, otherwise error code;
 * notes:
 */
int ak_vpss_af_get_attr(int dev, struct vpss_af_attr *af_attr);


/********************** effect *******************************/

/**
 * ak_vpss_effect_get - get isp effect param.
 * @dev[IN]: dev id
 * @type[IN]:   effect type name
 * @value[OUT]: effect value(s)
 * return: 0 - success; otherwise error code
 */
int ak_vpss_effect_get(int dev, enum vpss_effect_type type, int *value);

/**
 * ak_vpss_effect_set - set isp effect param.
 * @dev[IN]: dev id
 * @type[IN]:   effect type name
 * @value[IN]: effect value
 * return: 0 - success; otherwise error code
 */
int ak_vpss_effect_set(int dev, enum vpss_effect_type type, const int value);



/********************** mask *******************************/

/**
 * ak_vpss_mask_set_area: set main & sub channel mask area
 * @dev[IN]: dev id
 * @area[IN]: main channel mask area paramters
 * return: 0 success, other error code
 * notes:
 */
int ak_vpss_mask_set_area(int dev, const struct vpss_mask_area_info *area);

/**
 * ak_vpss_mask_get_area: get main & sub channel mask area
 * @dev[IN]: dev id
 * @area[OUT]: main channel mask area paramters
 * return: 0 success, other error code
 * notes:
 */
int ak_vpss_mask_get_area(int dev, struct vpss_mask_area_info *area);

/**
 * ak_vpss_mask_set_color: set main & sub channel mask color
 * @dev[IN]: dev id
 * @color[IN]: main & sub channel mask color paramters
 * return: 0 success, other error code
 * notes:
 */
int ak_vpss_mask_set_color(int dev, const struct vpss_mask_color_info *color);

/**
 * ak_vpss_mask_get_color: get main & sub channel mask color
 * @dev[IN]: dev id
 * @color[OUT]: main & sub channel mask color paramters
 * return: 0 success, other error code
 * notes:
 */
int ak_vpss_mask_get_color(int dev, struct vpss_mask_color_info *color);


/**
 * ak_vpss_get_fps_ctrl_stat: get fps ctrl stat
 * @dev[IN]: device id
 * @stat[OUT]:gain stat
 * @need_fps[OUT]:need fps
 * return: 0 success, otherwise error code
 * notes:for switch day high light and low light. use with ak_vpss_change_sensor_fps
 */
int ak_vpss_get_fps_ctrl_stat(int dev, enum vpss_gain_stat *stat, int *need_fps);

/**
 * ak_vpss_change_sensor_fps: change sensor fps
 * @dev[IN]: device id
 * @need_fps[IN]:need fps out by ak_vpss_get_fps_ctrl_stat
 * return:  0 success, otherwise error code
 * notes:for switch day high light and low light. use with ak_vpss_get_fps_ctrl_stat
 */
int ak_vpss_change_sensor_fps(int dev, int need_fps);

/**
 * ak_vpss_set_fps_level: set sensor fps level
 * @dev[IN]: device id
 * @fps_level[IN]:fps_level params
 * return:  0 success, otherwise error code
 * notes:
 */
int ak_vpss_set_fps_level(int dev, struct vpss_fps_level *fps_level);

/**
 * ak_vpss_get_fps_level: get sensor fps level
 * @dev[IN]: device id
 * @fps_level[OUT]:fps_level params
 * return:  0 success, otherwise error code
 * notes:
 */
int ak_vpss_get_fps_level(int dev, struct vpss_fps_level *fps_level);

/**
 * ak_vpss_get_sensor_fps: get sensor framerate
 * @dev[IN]: device id
 * @fps[OUT]: fps, sensor framerate
 * return: 0 success, otherwise error code
 * notes:
 */
int ak_vpss_get_sensor_fps(int dev, int *fps);


/**
 * ak_vpss_get_ae_run_info: get ae run info
 * @dev[IN]: device id
 * @ae_run_info[OUT]: ae run info
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_ae_run_info(int dev, struct vpss_isp_ae_run_info *ae_run_info);


/**
 * ak_vpss_set_ae_init_info: set ae init info
 * @dev[IN]: device id
 * @ae_init_info[IN]: ae init info
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_ae_init_info(int dev, struct vpss_isp_ae_init_info *ae_init_info);


/**
 * ak_vpss_get_sensor_ae_info: get sensor fast ae info
 * @dev[IN]: device id
 * @ae_init_info[OUT]: ae info
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_sensor_ae_info(int dev, struct vpss_isp_ae_init_info *ae_init_info);

/**
 * ak_vpss_get_ae_attr: get AE attr
 * @dev[IN]: device id
 * @ae_attr[OUT]: AE attr info
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_ae_attr(int dev, struct vpss_isp_ae_attr *ae_attr);

/**
 * ak_vpss_set_ae_attr: set AE attr
 * @dev[IN]: device id
 * @ae_attr[IN]: AE attr info
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_ae_attr(int dev, const struct vpss_isp_ae_attr *ae_attr);

/**
 * ak_vpss_set_ae_convergence_rate: set AE convergence rate
 * @dev[IN]: device id
 * @value[IN]: param of convergence rate
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_ae_convergence_rate(int dev, unsigned long value);

/**
 * ak_vpss_get_ae_convergence_rate: get AE convergence rate
 * @dev[IN]: device id
 * @value[OUT]: param of convergence rate
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_ae_convergence_rate(int dev, unsigned long *value);

/**
 * ak_vpss_check_ae_stable: check ae is stable or not
 * @dev[IN]: device id
 * @stable[OUT]: 1 stable, 0 not stable
 * return: 0 success, otherwise error code;
 */
int ak_vpss_check_ae_stable(int dev, int *stable);

/**
 * ak_vpss_get_me_attr: get ME attr
 * @dev[IN]: device id
 * @me_attr[OUT]: ME attr info
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_me_attr(int dev, struct vpss_isp_me_attr *me_attr);

/**
 * ak_vpss_set_me_attr: set ME attr
 * @dev[IN]: device id
 * @me_attr[IN]: ME attr info
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_me_attr(int dev, const struct vpss_isp_me_attr *me_attr);

/*
 * ak_vpss_set_ae_suspend - set ae suspend flag
 * dev[IN]: dev id
 * ae_suspend_flag[IN]: 1 suspend, 0 resume
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_ae_suspend(int dev,  int ae_suspend_flag);

/**
 * ak_vpss_get_awb_stat_info: get awb stat info
 * @dev[IN]: device id
 * @awb_stat_info[OUT]: awb stat info
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_awb_stat_info(int dev, struct vpss_isp_awb_stat_info *awb_stat_info);

/**
 * ak_vpss_get_cur_lumi: get current lum factor
 * @dev[IN]: device id
 * @lumi[OUT]: lum factor
 * return: 0 success, otherwise error code;
 * notes:
 */
int ak_vpss_get_cur_lumi(int dev, int *lumi);

/**
 * ak_vpss_set_auto_day_night_param: set auto day or night switch threshold
 * @dev[IN]: device id
 * @param[IN]: input param threshold
 * return: 0 - success; otherwise error code;
 */
int ak_vpss_set_auto_day_night_param(int dev, struct ak_auto_day_night_threshold *param);

/**
 * ak_vpss_get_auto_day_night_level: get day or night
 * @dev[IN]: device id
 * @pre_ir_level[IN]: pre status, 0 day, 1 night
 * @new_level[OUT]: new status, 0 day, 1 night
 * return: 0 - success; otherwise error code;
 */
int ak_vpss_get_auto_day_night_level(int dev, int pre_ir_level, int *new_ir_level);

/**
 * ak_vpss_set_force_anti_flicker_flag: set force anti flicker flag
 * @dev[IN]: device id
 * @force_flag[IN]: force anti flicker flag
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_force_anti_flicker_flag(int dev, int force_flag);

/**
 * ak_vpss_get_force_anti_flicker_flag: get force anti flicker flag
 * @dev[IN]: device id
 * @force_flag[OUT]: force anti flicker flag
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_force_anti_flicker_flag(int dev, int *force_flag);

/**
 * ak_vpss_set_force_anti_flicker_flag_ex: set force anti flicker flag and sub_antiflicker_value
 * @dev[IN]: device id
 * @force_flag[IN]: force anti flicker flag
 * @sub_antiflicker_value[IN]: sub_antiflicker_value suggest [64-90]
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_force_anti_flicker_flag_ex(int dev, int force_flag, int sub_antiflicker_value);

/**
 * ak_vpss_get_force_anti_flicker_flag_ex: get force anti flicker flag and sub_antiflicker_value
 * @dev[IN]: device id
 * @force_flag[OUT]: force anti flicker flag
 * @sub_antiflicker_value[OUT]: sub_antiflicker_value
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_force_anti_flicker_flag_ex(int dev, int *force_flag, int *sub_antiflicker_value);

/**
 * ak_vpss_set_anti_flicker_strength: set anti flicker strength
 * @dev[IN]: device id
 * @strength[IN]: anti flicker strength [0-100], 0 means disable anti flicker
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_anti_flicker_strength(int dev, int strength);


/**
 * ak_vpss_get_anti_flicker_strength: get anti flicker strength
 * @dev[IN]: device id
 * @strength[OUT]: anti flicker strength
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_anti_flicker_strength(int dev, int *strength);

/**
 * ak_vpss_get_wdr_attr: get wdr attr
 * @dev[IN]: device id
 * @p_wdr[OUT]: wdr attr
 * return: 0 success, otherwise error code;
 * notes:
 */
int ak_vpss_get_wdr_attr(int dev, struct vpss_wdr_attr *p_wdr);

/**
 * ak_vpss_set_wdr_attr: get wdr attr
 * @dev[IN]: device id
 * @p_wdr[IN]: wdr attr
 * return: 0 success, otherwise error code;
 * notes:
 */
int ak_vpss_set_wdr_attr(int dev, struct vpss_wdr_attr *p_wdr);


/**
 * ak_vpss_get_weight_attr: get weight attr
 * @dev[IN]: device id
 * @weight_attr[OUT]: weight_attr info
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_weight_attr(int dev, struct vpss_isp_weight_attr *weight_attr);

/**
 * ak_vpss_set_weight_attr: set weight attr
 * @dev[IN]: device id
 * @weight_attr[IN]: weight_attr info
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_weight_attr(int dev, struct vpss_isp_weight_attr *weight_attr);

/**
 * ak_vpss_get_rgb_average: get rgb average value
 * @dev[IN]: device id
 * @r_avr[OUT]: r average value
 * @g_avr[OUT]: g average value
 * @b_avr[OUT]: b average value
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_rgb_average(int dev,
        unsigned int *r_avr, unsigned int *g_avr, unsigned int *b_avr);

/**
 * ak_vpss_get_wb_type: get wb type
 * @dev[IN]: device id
 * @wb_type[OUT]: wb type
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_wb_type(int dev, struct vpss_isp_wb_type_attr *wb_type);

/**
 * ak_vpss_set_wb_type: set wb type
 * @dev[IN]: device id
 * @wb_type[IN]: wb type
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_wb_type(int dev, const struct vpss_isp_wb_type_attr *wb_type);

/**
 * ak_vpss_get_3d_nr_attr: get 3D NR attr
 * @dev[IN]: device id
 * @nr_3d_attr[OUT]: 3D NR attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_3d_nr_attr(int dev, struct vpss_isp_3d_nr_attr *nr_3d_attr);

/**
 * ak_vpss_set_3d_nr_attr: set 3D NR attr
 * @dev[IN]: device id
 * @nr_3d_attr[IN]: 3D NR attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_3d_nr_attr(int dev, const struct vpss_isp_3d_nr_attr *nr_3d_attr);

/**
 * ak_vpss_get_mwb_attr: get mwb attr
 * @dev[IN]: device id
 * @mwb_attr[OUT]: mwb attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_mwb_attr(int dev, struct vpss_isp_mwb_attr *mwb_attr);

/**
 * ak_vpss_set_mwb_attr: set mwb attr
 * @dev[IN]: device id
 * @mwb_attr[IN]: mwb attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_mwb_attr(int dev, const struct vpss_isp_mwb_attr *mwb_attr);

/**
 * ak_vpss_get_awb_attr: get awb attr
 * @dev[IN]: device id
 * @awb_attr[OUT]: awb attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_awb_attr(int dev, struct vpss_isp_awb_attr *awb_attr);

/**
 * ak_vpss_set_awb_attr: set awb attr
 * @dev[IN]: device id
 * @awb_attr[IN]: awb attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_awb_attr(int dev, const struct vpss_isp_awb_attr *awb_attr);

/**
 * ak_vpss_get_exp_type: get exp type
 * @dev[IN]: device id
 * @exp_type[OUT]: exp type
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_exp_type(int dev, struct vpss_isp_exp_type *exp_type);

/**
 * ak_vpss_set_exp_type: set exp type
 * @dev[IN]: device id
 * @exp_type[IN]: exp type
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_exp_type(int dev, const struct vpss_isp_exp_type *exp_type);

/**
 * ak_vpss_get_sensor_reg: get sensor register info
 * @dev[IN]: device id
 * @sensor_reg_info[IN/OUT]: sensor register info
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_sensor_reg(int dev, struct vpss_isp_sensor_reg_info *sensor_reg_info);

/**
 * ak_vpss_get_lsc_attr: get lsc attr
 * @dev[IN]: device id
 * @lsc[OUT]: lsc attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_lsc_attr(int dev, struct vpss_lsc_attr *lsc);

/**
 * ak_vpss_set_lsc_attr: set lsc attr
 * @dev[IN]: device id
 * @lsc[IN]: lsc attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_lsc_attr(int dev, const struct vpss_lsc_attr *lsc);

/**
 * ak_vpss_get_ex_zweight_attr: get ex zweight attr
 * @dev[IN]: device id
 * @ex_zweight[OUT]: ex zweight attr info
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_ex_zweight_attr(int dev, struct vpss_isp_ex_zweight_attr *ex_zweight);

/**
 * ak_vpss_set_ex_zweight_attr: set ex zweight attr
 * @dev[IN]: device id
 * @ex_zweight[IN]: ex zweight attr info
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_ex_zweight_attr(int dev, struct vpss_isp_ex_zweight_attr *ex_zweight);

/**
 * ak_vpss_set_yuv_effect_attr: set yuv effect attr
 * @dev[IN]: device id
 * @yuv_effect[IN]: yuv_effect attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_yuv_effect_attr(int dev, const struct vpss_yuv_effect_attr *yuv_effect);

/**
 * ak_vpss_get_yuv_effect_attr: get yuv effect attr
 * @dev[IN]: device id
 * @yuv_effect[OUT]: yuv_effect attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_yuv_effect_attr(int dev, struct vpss_yuv_effect_attr *yuv_effect);

/**
 * ak_vpss_set_rgb_gamma_attr: set rgb gamma attr
 * @dev[IN]: device id
 * @rgb_gamma[IN]: rgb_gamma attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_rgb_gamma_attr(int dev, const struct vpss_rgb_gamma_attr *rgb_gamma);

/**
 * ak_vpss_get_rgb_gamma_attr: get rgb gamma attr
 * @dev[IN]: device id
 * @rgb_gamma[OUT]: rgb_gamma attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_rgb_gamma_attr(int dev, struct vpss_rgb_gamma_attr *rgb_gamma);

/**
 * ak_vpss_set_contrast_attr: set contrast attr
 * @dev[IN]: device id
 * @contrast[IN]: contrast attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_contrast_attr(int dev, const struct vpss_contrast_attr *contrast);

/**
 * ak_vpss_get_contrast_attr: get contrast attr
 * @dev[IN]: device id
 * @contrast[OUT]: contrast attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_contrast_attr(int dev, struct vpss_contrast_attr *contrast);

/**
 * ak_vpss_set_lce_attr: set lce attr
 * @dev[IN]: device id
 * @lce[IN]: lce attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_lce_attr(int dev, const struct vpss_lce_attr *lce);

/**
 * ak_vpss_get_lce_attr: get lce attr
 * @dev[IN]: device id
 * @lce[OUT]: lce attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_lce_attr(int dev, struct vpss_lce_attr *lce);

/**
 * ak_vpss_set_y_gamma_attr: set y_gamma attr
 * @dev[IN]: device id
 * @y_gamma[IN]: y_gamma attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_y_gamma_attr(int dev, const struct vpss_y_gamma_attr *y_gamma);

/**
 * ak_vpss_get_y_gamma_attr: set y_gamma attr
 * @dev[IN]: device id
 * @y_gamma[OUT]: y_gamma attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_y_gamma_attr(int dev, struct vpss_y_gamma_attr *y_gamma);

/**
 * ak_vpss_set_raw_lut_attr: set RAW gamma attr
 * @dev[IN]: device id
 * @raw_lut[IN]: raw_lut attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_raw_lut_attr(int dev, struct vpss_raw_lut_attr *raw_lut);

/**
 * ak_vpss_get_raw_lut_attr: get RAW lut attr
 * @dev[IN]: device id
 * @raw_lut[OUT]: raw_lut attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_raw_lut_attr(int dev, struct vpss_raw_lut_attr *raw_lut);

/**
 * ak_vpss_set_ccm_attr - set isp ccm param.
 * @dev[IN]: dev id
 * @ccm_attr[IN]: ccm_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_set_ccm_attr(int dev, struct vpss_ccm_attr *ccm_attr);

/**
 * ak_vpss_get_ccm_attr - get isp ccm param.
 * @dev[IN]: dev id
 * @ccm_attr[OUT]: ccm_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_get_ccm_attr(int dev, struct vpss_ccm_attr *ccm_attr);

/**
 * ak_vpss_set_hue_attr - set isp hue param.
 * @dev[IN]: dev id
 * @hue_attr[IN]: hue_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_set_hue_attr(int dev, struct vpss_hue_attr *hue_attr);

/**
 * ak_vpss_get_hue_attr - get isp hue param.
 * @dev[IN]: dev id
 * @hue_attr[OUT]: hue_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_get_hue_attr(int dev, struct vpss_hue_attr *hue_attr);

/**
 * ak_vpss_set_gb_attr - set isp gb param.
 * @dev[IN]: dev id
 * @gb_attr[IN]: gb_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_set_gb_attr(int dev, struct vpss_gb_attr *gb_attr);

/**
 * ak_vpss_get_gb_attr - get isp gb param.
 * @dev[IN]: dev id
 * @gb_attr[OUT]: gb_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_get_gb_attr(int dev, struct vpss_gb_attr *gb_attr);

/**
 * ak_vpss_set_ddpc_attr - set isp dpc param.
 * @dev[IN]: dev id
 * @ddpc_attr[IN]: ddpc_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_set_ddpc_attr(int dev, struct vpss_ddpc_attr *ddpc_attr);

/**
 * ak_vpss_get_ddpc_attr - get isp dpc param.
 * @dev[IN]: dev id
 * @ddpc_attr[OUT]: ddpc_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_get_ddpc_attr(int dev, struct vpss_ddpc_attr *ddpc_attr);

/**
 * ak_vpss_set_nr1_attr - set isp nr1 param.
 * @dev[IN]: dev id
 * @nr1_attr[IN]: nr1_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_set_nr1_attr(int dev, struct vpss_nr1_attr *nr1_attr);

/**
 * ak_vpss_get_nr1_attr - get isp nr1 param.
 * @dev[IN]: dev id
 * @nr1_attr[OUT]: nr1_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_get_nr1_attr(int dev, struct vpss_nr1_attr *nr1_attr);

/**
 * ak_vpss_set_nr2_attr - set isp nr2 param.
 * @dev[IN]: dev id
 * @nr2_attr[IN]: nr2_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_set_nr2_attr(int dev, struct vpss_nr2_attr *nr2_attr);

/**
 * ak_vpss_get_nr2_attr - get isp nr2 param.
 * @dev[IN]: dev id
 * @nr2_attr[OUT]: nr2_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_get_nr2_attr(int dev, struct vpss_nr2_attr *nr2_attr);

/**
 * ak_vpss_set_uvnr_attr - set isp uvnr param.
 * @dev[IN]: dev id
 * @uvnr_attr[IN]: uvnr_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_set_uvnr_attr(int dev, struct vpss_uvnr_attr *uvnr_attr);

/**
 * ak_vpss_get_uvnr_attr - get isp uvnr param.
 * @dev[IN]: dev id
 * @uvnr_attr[OUT]: uvnr_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_get_uvnr_attr(int dev, struct vpss_uvnr_attr *uvnr_attr);

/**
 * ak_vpss_set_sharp_attr - set isp sharp param.
 * @dev[IN]: dev id
 * @sharp_attr[IN]: sharp_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_set_sharp_attr(int dev, struct vpss_sharp_attr *sharp_attr);

/**
 * ak_vpss_get_sharp_attr - get isp sharp param.
 * @dev[IN]: dev id
 * @sharp_attr[OUT]: sharp_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_get_sharp_attr(int dev, struct vpss_sharp_attr *sharp_attr);

/**
 * ak_vpss_set_fcs_attr - set isp fcs param.
 * @dev[IN]: dev id
 * @fcs_attr[IN]: fcs_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_set_fcs_attr(int dev, struct vpss_fcs_attr *fcs_attr);

/**
 * ak_vpss_get_fcs_attr - get isp fcs param.
 * @dev[IN]: dev id
 * @fcs_attr[OUT]: fcs_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_get_fcs_attr(int dev, struct vpss_fcs_attr *fcs_attr);


/**
 * ak_vpss_set_saturation_attr - set isp saturation param.
 * @dev[IN]: dev id
 * @sat_attr[IN]: sat_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_set_saturation_attr(int dev, struct vpss_saturation_attr *sat_attr);

/**
 * ak_vpss_get_saturation_attr - get isp saturation param.
 * @dev[IN]: dev id
 * @sat_attr[OUT]: sat_attr
 * return: 0 - success; otherwise error code
 */
int ak_vpss_get_saturation_attr(int dev, struct vpss_saturation_attr *sat_attr);


/**
 * ak_vpss_set_sharp_reduce_factor: set sharp_reduce_factor
 * @dev[IN]: device id
 * @attr[IN]: sharp_reduce_factor attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_set_sharp_reduce_factor(int dev, struct vpss_sharp_reduce_factor_attr *attr);

/**
 * ak_vpss_get_sharp_reduce_factor: get sharp_reduce_factor
 * @dev[IN]: device id
 * @attr[OUT]: sharp_reduce_factor attr
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_sharp_reduce_factor(int dev, struct vpss_sharp_reduce_factor_attr *attr);


#if 0
/**
 * ak_vpss_get_awb_stable: get awb stable flag
 * @dev[IN]: device id
 * @exp_type[OUT]: awb stable flag
 * return: 0 success, otherwise error code;
 */
int ak_vpss_get_awb_stable(int dev, char *awb_stable_flag);
#endif

/**
 * brief: set flip mirror compare to default value
 * @dev[IN]: device id
 * @flip_en[IN]:flip enable or not , 0 disable (default value), 1 enable
 * @mirror_en[IN]:mirror enable or not , 0 disable (default value), 1 enable
 * return:  0 success, otherwise error code
 * notes:
 */
int ak_vpss_switch_flip_mirror(int dev, int flip_en, int mirror_en);

/**
 * brief: get flip mirror compare to default value
 * @dev[IN]: device id
 * @flip_en[OUT]:flip enable or not , 0 disable (equal to default value), \
 * 1 enable (not equal to default value)
 * @mirror_en[OUT]:mirror enable or not , 0 disable (equal to default value), \
 * 1 enable (not equal to default value)
 * return:  0 success, otherwise error code
 * notes:
 */
int ak_vpss_get_flip_mirror(int dev, int *flip_en, int *mirror_en);

/**
 * ak_vpss_set_soft_ps_param: set soft ps switch threshold
 * @dev[IN]: device id
 * @param[IN]: input param threshold
 * return: 0 - success; otherwise error code;
 */
int ak_vpss_set_soft_ps_param(int dev, struct ak_soft_ps_attr *param);


/**
 * ak_vpss_get_soft_ps_level: get day or night
 * @dev[IN]: device id
 * @pre_ir_level[IN]: pre status, 0 day, 1 night
 * @new_level[OUT]: new status, 0 day, 1 night
 * return: 0 - success; otherwise error code;
 */
int ak_vpss_get_soft_ps_level(int dev, int pre_ir_level, int *new_ir_level);


/**
 * ak_vpss_get_soft_wh_level: get day or night
 * @dev[IN]: device id
 * @pre_ir_level[IN]: pre status, 0 day, 1 white led night
 * @new_level[OUT]: new status, 0 day, 1 white led night
 * return: 0 - success; otherwise error code;
 */
int ak_vpss_get_soft_wh_level(int dev, int pre_ir_level, int *new_ir_level);


/**
 * ak_vpss_wait_exp_stable
 * @dev[IN]: device id
 * @exp_stable_check_frame_num[IN]: exp_stable_check_frame_num
 * return: 0 - success, otherwise error code;
 * notes:
 */
int ak_vpss_wait_exp_stable(int dev, unsigned int exp_stable_check_frame_num,unsigned int timeout_ms);

/**
 * ak_vpss_get_ev: get ev
 * @dev[IN]: device id
 * @ev[OUT]: ev value
 * return: 0 - success; otherwise error code;
 */
int ak_vpss_get_ev(int dev, unsigned int *ev);

/**
 * ak_vpss_get_rgb_dis: get rgb dis
 * @dev[IN]: device id
 * @gain_r[IN]: gain_r
 * @offset_r[IN]: offset_r
 * @gain_b[IN]: gain_b
 * @offset_b[IN]: offset_b
 * @dis[OUT]: dis value
 * return: 0 - success; otherwise error code;
 */
int ak_vpss_get_rgb_dis(int dev, unsigned int gain_r, int offset_r, unsigned int gain_b, int offset_b, int *dis);

/**
 * ak_vpss_get_blk_lumi: get block lumi
 * @dev[IN]: device id
 * @blk_lumi[OUT]: block lumi
 * return: 0 - success; otherwise error code;
 */
int ak_vpss_get_blk_lumi(int dev, struct vpss_blk_lumi_attr *blk_lumi);

/**
 * ak_vpss_get_non_infrared_ratio: get non infrared ratio
 * @dev[IN]: device id
 * @nir_attr[IN]:non_infrared_ratio_attr
 * @pnon_infrared_ratio[OUT]: non_infrared_ratio
 * return: 0 - success; otherwise error code;
 */
int ak_vpss_get_non_infrared_ratio(int dev,
    struct ak_non_infrared_ratio_attr *nir_attr, unsigned int *pnon_infrared_ratio);

/**
 * ak_vpss_get_3d_nr_ref_size_stat: get  3dnr buf used size
 * @dev[IN]: device id
 * @ref_size[OUT]: 3dnr buf y/u/v used size
 * return: 0 - success; otherwise error code;
 */
int ak_vpss_get_3d_nr_ref_size_stat(int dev, struct vpss_3d_nr_ref_size_stat_info *ref_size);

#if defined  __CHIP_AK3918AV130_SERIES

/**
 * brief: get 3dnr rc
 * @dev[IN]: device id
 * @rc_all[OUT]: 3dnr rc
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vpss_get_3dnr_rc_all(int dev, struct ak_vpss_3d_nr_rc *rc_all);

/**
 * brief: set 3dnr rc
 * @dev[IN]: device id
 * @rc[IN]: 3dnr rc
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vpss_set_3dnr_rc(int dev, enum vpss_isp_grp grp, struct vpss_isp_3d_nr_rc *rc);
#endif

#endif

/* end of file */
