#ifndef __AK_VPSS_STRUCT_KM01A_H__
#define __AK_VPSS_STRUCT_KM01A_H__


#define VPSS_MD_DIMENSION_H_MAX     64
#define VPSS_MD_DIMENSION_V_MAX     36

#define VPSS_OD_AF_STATICS_MAX      5
#define VPSS_OD_RGB_HIST_MAX        256

#define VPSS_MASK_AREA_MAX	        4


#pragma pack(1)

enum vpss_isp_op_type{
    VPSS_OP_TYPE_MANUAL = 0,
    VPSS_OP_TYPE_AUTO = 1,
    VPSS_OP_TYPE_BUTT,
};

struct vpss_md_info {
    unsigned short stat[36][64];
};
#if 0
struct vpss_od_info {
    unsigned int af_statics[8][16][5];
    //unsigned int rgb_hist[256];
    //unsigned long long rgb_total;
};
#endif
struct vpss_af_stat_info {
   unsigned int  af_statics[8][16][5];
   unsigned char af_local_highlight[64][128];
};



struct vpss_af_attr {
    unsigned short  af_stat_enable;
    unsigned char af_spotlight_lum_th;	  //[0 ,255]
    unsigned char af_spotlight_edge_th;   //[0 ,255]
    unsigned char af_spotlight_edge_gain; //[0, 15]
    unsigned char af_clip_th;			  //[0, 63]
    unsigned char af_peak_ctrl;		      //[0, 255]
    unsigned char af_rshift;			  //[0, 15]
};


struct vpss_isp_exp_type {
    enum vpss_isp_op_type   exp_type;
};

struct vpss_isp_ae_run_info {
    unsigned char	current_calc_avg_lumi;				//现在的计算出的亮度值
    unsigned char	current_calc_avg_compensation_lumi; 	//经过曝光补偿后的亮度值

    unsigned char	current_target_lumi;				 //白天黑夜的标记
    unsigned char	is_stable;

    unsigned int  current_a_gain;						//模拟增益的值
    unsigned int  current_d_gain;						//数字增益的值
    unsigned int  current_isp_d_gain;					//isp数字增益的值
    unsigned int  current_exp_time;					//曝光时间的值

    unsigned int  current_a_gain_step;				//现在的模拟增益的步长
    unsigned int  current_d_gain_step;				//数字增益的步长
    unsigned int  current_isp_d_gain_step;				//isp数字增益的步长
    unsigned int  current_exp_time_step;

    //AK_ISP_AE_ROUTE_ATTR		ae_route_para;
};


struct vpss_isp_ae_init_info {
    long  a_gain;				//ģ�������ֵ
    long  d_gain;				//���������ֵ
    long  isp_d_gain;			//isp���������ֵ
    long  exp_time;				//�ع�ʱ���ֵ
};

#if 0
enum vpss_isp_win_weight_type {
   VPSS_AE_CENTER_WGHT= 0,
   VPSS_AE_AVERAGE = 1,
   VPSS_AE_SPOT ,
   VPSS_AE_WGHT_END ,
};

struct vpss_isp_weight {
   unsigned short	zone_weight[8][16];
};

struct vpss_isp_weight_attr {
    enum vpss_isp_win_weight_type   ae_win_weight_type;
    struct vpss_isp_weight      	win_weight[4];
};

struct vpss_isp_ex_zweight_attr {
    unsigned char	win_weight[8][16];
    unsigned char	ex_zone_weight_str;
};
#endif

struct vpss_isp_ae_ex_convergence_attr {
    unsigned char    ae_ex_convergence_en;
    unsigned short   control_zone1;
    unsigned short   control_zone2;
    unsigned short   control_zone3;
    unsigned short   control_zone4;
    unsigned short   control_step1;
    unsigned short   control_step2;
    unsigned short   control_step3;
    unsigned short   control_step4;
    unsigned short   control_step_max;
};

enum vpss_isp_ae_strgy_model {
    VPSS_AE_EXP_HIGHLIGHT_PRIOR = 0,
    VPSS_AE_EXP_LOWLIGHT_PRIOR,
    VPSS_AE_STRGY_MODEL_BUFF,
};

struct vpss_isp_ae_strgy_attr {
    unsigned int	hist_weight[16];
};

struct vpss_isp_ae_route_node_attr {
    unsigned int   exp_time;
    unsigned int   again;
    unsigned int   dgain;
    unsigned int   isp_dgain;
};

struct vpss_isp_ae_route_attr{
    unsigned int   ae_route_en;
    unsigned int   ae_route_node_num;
    struct vpss_isp_ae_route_node_attr  ae_route_node[10];
};


struct vpss_isp_raw_g_hist_attr{
    unsigned char  raw_hist_enable;
    unsigned char  raw_rshift;	//[0, 15]
    unsigned char  zone_weight[8][16];
};


struct vpss_isp_yuv_hist_attr{
    unsigned short  yuv_hist_enable;
    unsigned char  zone_weight[8][16];
};


struct vpss_isp_me_attr {
    unsigned int exp_time;
    unsigned int a_gain;
    unsigned int d_gain;
    unsigned int isp_d_gain;
};



enum vpss_isp_antiflicker_model
{
    VPSS_ANTIFLICKER_NORMAL_MODEL = 0,
    VPSS_ANTIFLICKER_AUTO_MODEL = 1,
    VPSS_ANTIFLICKER_AUTO_BUTT
};

enum vpss_isp_antiflicker_frequency
{
    VPSS_FLICKER_FREQUENCE_50HZ = 0,
    VPSS_FLICKER_FREQUENCE_60HZ = 1,
};

struct vpss_isp_antiflicker_attr
{
    unsigned char anti_flicker_en;
    enum vpss_isp_antiflicker_frequency frequency;
    enum vpss_isp_antiflicker_model     anti_flicker_model;
};

struct vpss_isp_subantiflicker_attr
{
    unsigned char  anti_subflicker_en;
    unsigned char  anti_flicker_target_lumi;
  unsigned char  anti_flicker_start_exp;//[1,100]
};

enum vpss_isp_merge_model
{
    vpss_COND_INDEPENDENT_MODEL = 0,
    vpss_NORMAL_WEIGHTED_MODEL = 1,
    vpss_MERGE_MODEL_BUTT
};


struct vpss_isp_ae_attr {
    unsigned int   exp_time_max;			//曝光时间的最大值
    unsigned int   a_gain_max;			//模拟增益的最大值
    unsigned int   d_gain_max;				//数字增益的最大值
    unsigned int   isp_d_gain_max;			//isp数字增益的最大值
    unsigned short exp_time_min;			//曝光时间的最小值
    unsigned short a_gain_min;				//模拟增益的最小值
    unsigned short d_gain_min;			//数字增益的最小值
    unsigned short isp_d_gain_min;			//isp数字增益的最小
    unsigned short blacklight_rate_max;
    unsigned short blacklight_rate_min;
    unsigned int   linesper500ms;
    unsigned char  exp_stable_range;		//稳定范围
    unsigned char  exp_hold_range;
    unsigned char  exp_speed;
    unsigned char  exp_lumi_filter_para;
    unsigned char  target_lumiance[12]; 		//目标亮度
   // unsigned int	 hist_weight[16];
    unsigned char  blacklight_compensation_en;
    unsigned char  blacklight_detect_scope;
    enum vpss_isp_merge_model  merge_model;
    unsigned char  merge_weight;
    signed char    merge_lumi_diff[12];

    //AK_ISP_WIN_WEIGHT_TYPE  ae_win_weight_type;
    //AK_ISP_WEIGHT_ATTR	win_weight[4];
    struct vpss_isp_raw_g_hist_attr         raw_g_hist_attr;
    struct vpss_isp_yuv_hist_attr           yuv_hist_attr;
    unsigned short envi_gain_range[12][2];
    struct vpss_isp_ae_ex_convergence_attr  ae_ex_convergence_para;
    enum vpss_isp_ae_strgy_model            ae_strgy_type;
    struct vpss_isp_ae_strgy_attr           ae_strgy_para[3];
    struct vpss_isp_ae_route_attr           exp_route_para;
    struct vpss_isp_antiflicker_attr	    ae_antiflicker_para;
    struct vpss_isp_subantiflicker_attr     ae_subantiflickerpara;
};

struct vpss_awb_blk_avg_stat_info
{
    unsigned int wb_Ravg_stat[16][16];
    unsigned int wb_Gavg_stat[16][16];
    unsigned int wb_Bavg_stat[16][16];
};

struct vpss_awb_blk_wp_stat_info
{
    unsigned int wp_r_stat[16][16];
    unsigned int wp_g_stat[16][16];
    unsigned int wp_b_stat[16][16];
    unsigned int wp_cnt_statics[16][16];
};

struct vpss_isp_awb_run_info{
    unsigned short	 r_gain;
    unsigned short	 g_gain;
    unsigned short	 b_gain;
    signed short   r_offset;
    signed short   g_offset;
    signed short   b_offset;
    unsigned char    is_stable;
    unsigned char	 current_colortemp_index;
    unsigned short	 colortemp_stable_cnt[10];
    unsigned short current_colortemp;
};


struct vpss_isp_awb_stat_info {
    //白平衡统计结果
    unsigned int   total_R[10]; //10个色温每一个色温下的R分量像素值
    unsigned int   total_G[10]; //10个色温每一个色温下的G分量像素值
    unsigned int   total_B[10]; //10个色温每一个色温下的B分量像素值
    unsigned int   total_cnt[10];	//在白平衡统计参数范围内的像素数量值

    struct vpss_awb_blk_avg_stat_info   wb_blk_avg_stat;
    struct vpss_awb_blk_wp_stat_info    wb_blk_wp_stat;
    struct vpss_isp_awb_run_info        run_info;
};

struct vpss_isp_wb_type_attr {
    enum vpss_isp_op_type wb_type;
};


struct vpss_isp_mwb_attr {
    unsigned short	r_gain;
    unsigned short	g_gain;
    unsigned short	b_gain;
    signed short  r_offset;
    signed short  g_offset;
    signed short  b_offset;
};

enum vpss_isp_awb_algo_type{
    VPSS_AWB_ALGO_DEFAULT = 0,
    VPSS_AWB_ALGO_AVERAGE_WEIGHT = 1,
    VPSS_AWB_ALGO_BLOCK_ASSIST,
    VPSS_AWB_ALGO_GW,
    VPSS_AWB_ALGO_BUTT,
};

typedef struct vpss_isp_awb_stat_calib_para{
    unsigned short	 gr_low[10];			//gr_low[i]<=gr_high[i]
    unsigned short	 gr_high[10];
    unsigned short	 gb_low[10];			//gb_low[i]<=gb_high[i]
    unsigned short	 gb_high[10];
    unsigned short	 rb_low[10];		   //rb_low[i]<=rb_high[i]
    unsigned short	 rb_high[10];
}VPSS_ISP_AWB_STAT_CALIB_PARA;

struct vpss_isp_awb_attr {
    unsigned char	 g_weight[16];
    unsigned char	 y_low; 				//y_low<=y_high
    unsigned char	 y_high;
    unsigned char	 err_est;
    unsigned char 	 awb_iso_track_enable;
    unsigned char	 colortemp_envi[10];
    VPSS_ISP_AWB_STAT_CALIB_PARA awb_stat_calib_para[12];
    unsigned char	 merge_benchmarks;
      unsigned short merge_weight[9];
    unsigned short auto_wb_step;                //白平衡步长计算
    unsigned short total_cnt_thresh;            //像素个数阈值
    unsigned short colortemp_stable_cnt_thresh; //稳定帧数，多少帧一样认为环境色温改变

    enum vpss_isp_awb_algo_type alg_type;
};


struct vpss_fps_level {
    int high_fps;
    int high_fps_exp_time;
    int high_fps_to_lower_fps_gain;

    int mid_fps;                    //set 0 means no use middle fps
    int mid_fps_exp_time;           //set 0 means no use middle fps
    int mid_fps_to_low_fps_gain;    //set 0 means no use middle fps

    int low_fps;
    int low_fps_exp_time;
    int low_fps_to_higher_fps_gain;
};


struct vpss_isp_hdr {
    unsigned char   hdr_enable;
    unsigned char   hdr_uv_adjust_enable;		 //uv调整使能
    unsigned short	hdr_th1;	  //0-1023
    unsigned short	hdr_th2;	  //0-1023
    unsigned short	hdr_th3;	  //0-1023
    unsigned short	hdr_th4;	  //0-1023
    unsigned short	hdr_th5;	  //0-1023

    //unsigned short wdr_light_weight;

    unsigned short	area_tb1[65];	  //曲线 10bit
    unsigned short	area_tb2[65];	  //曲线 10bit
    unsigned short	area_tb3[65];	  //曲线 10bit
    unsigned short	area_tb4[65];	  //曲线 10bit
    unsigned short	area_tb5[65];	  //曲线 10bit
    unsigned short	area_tb6[65];	  //曲线 10bit

    unsigned short	area1_key[16];
    unsigned short	area2_key[16];
    unsigned short	area3_key[16];
    unsigned short	area4_key[16];
    unsigned short	area5_key[16];
    unsigned short	area6_key[16];


    //unsigned char	 hdr_spatialfilt_pos;

    unsigned short	 hdr_cnoise_suppress_yth1;	 //色彩噪声亮度阈值1
    unsigned short	 hdr_cnoise_suppress_yth2;	 //色彩噪声亮度阈值2
    unsigned short	 hdr_cnoise_suppress_gain;	 //色差抑制
    unsigned char	 hdr_cnoise_suppress_slop;	 //抑制斜率
    unsigned char	 hdr_uv_adjust_level;		 //uv调整程度, [0,31]
    unsigned char	 hdr_spatialfilt_k;//[0,5]
};

struct vpss_wdr_attr {
    enum vpss_isp_op_type wdr_mode;              //模式选择，手动或者联�?
    struct vpss_isp_hdr manual_wdr;
    struct vpss_isp_hdr linkage_wdr[12];
};


struct vpss_isp_3d_nr {
    unsigned char	    tnr_enable; 		//default:1(GUI edit)
    unsigned char		tnr_ref_rc_enable;
    unsigned char		tnr_refFrame_format;	//参考帧压缩格式
    //unsigned short		t_mf_slop; //[0,255] //65536/(t_mf_th2-t_mf_th1)
    unsigned char 		ynr_DPCStr;	                //[0, 127]
    unsigned char 		ynr_DPCCap;	                //[0, 127]
    unsigned char 		ynr_DPCDiffGain[8];	        //[0,7]
    unsigned char		ynr_k;			//[0,15]
    unsigned char		ynr_lum_str[32]; //[0, 15]
    unsigned char		ylp_k1; 		//[0, 15],default:7(GUI edit)
    unsigned char		ylp_k2; 		//[0, 15],default:0(GUI edit)
    unsigned char 		yDiffUVWgt;		//[0, 15]
    unsigned char 		yDiffEdgeWgt;	//[0, 15]
    unsigned char		t_y_low1;		//[0,127]
    unsigned char		t_y_low2;		//[0,127]
    unsigned char		t_y_k1; 		//[0, 127],default:120(GUI edit)
    unsigned char		t_y_k2; 		//[0, 127],default:120(GUI edit)
    unsigned char		t_y_ex_k;		//[0, 15]
    unsigned char		t_y_kslop1;		//[0, 127],default:32(GUI edit)
    unsigned char		t_y_kslop2; 	//[0, 127]
    unsigned char		t_y_src_choose_k_th;		//[0, 127]
    //unsigned char		t_y_src_choose_slop;		//[0, 63]
    unsigned char		t_y_update_th;


    unsigned char		uvnr_k; 		//[0, 15],default:8(GUI edit)
    unsigned char		uvlp_k1;		//[0, 15],default:7(GUI edit)
    unsigned char		uvlp_k2;		//[0, 15],default:0(GUI edit)
    unsigned char		t_uv_diff_scale;//[0, 15]
    unsigned char		t_uv_k_low1; 	//[0, 127]
    unsigned char		t_uv_k_low2;	//[0, 127]
    unsigned char		t_uv_k1;		//[0, 127],default:120(GUI edit)
    unsigned char		t_uv_k2;		//[0, 127]
    unsigned char		t_uv_ex_k;		//[0, 15],
    unsigned char		t_uv_kslop1; 		//[0, 127],default:32(GUI edit)
    unsigned char		t_uv_kslop2;		//[0, 127],default:32(GUI edit)
    unsigned char		t_uv_src_choose_k_th; //[0, 127]
    //unsigned char		t_uv_src_choose_slop; //[0, 63]
    unsigned char		t_ref_rc_bit;
    signed char		    t_ref_rc_low_limit;
    signed char		    t_ref_rc_hihg_limit;
    unsigned char		mfactor_k_th; 		//[0, 127]
    unsigned char		mfactor_slop;		//[0, 15]
    //unsigned short		sharp_factor_max;		//[0, 15]


    //unsigned short      tnr_motion_flag_th;
    unsigned char 		motion_stat_th;	        //[0, 15]
    unsigned char 		sfactor_stat_th;	    //[0, 3]
    unsigned char 		motion_filter;			//[0,15]
    unsigned char 		motion_area_expand;     //[0, 1]
    unsigned char 		lumStr[32];	            //[0, 15]
    //unsigned char 		motion_flag[36][64];//运动检测输出
    unsigned short		t_mf_th1;	//[0, 8191],default:300(GUI edit)
    unsigned short		t_mf_th2;	//[0, 8191],default:500(GUI edit)
    unsigned short		ynr_calc_k;		//[0,65535](GUI edit)
    unsigned short		ynr_weight_tbl[17];//ynr_strength(GUI edit)
    unsigned short		t_y_th1;		//[0, 511],default:48(GUI edit)
    unsigned short		t_y_th2;		//[0, 511],default:16(GUI edit)
    unsigned short		t_uv_th1;		//[0, 511],default:48(GUI edit)
    unsigned short		t_uv_th2;		//[0, 511],default:16(GUI edit)
    unsigned short		md_th;			//[0, 65535]
    unsigned short      weight4_7X5;    //[0,8695]
    unsigned short      tnr_motion_flag_th;
};

struct vpss_isp_3d_nr_attr {
    unsigned char 	tnr_debug_output;	//[0,1]
    unsigned char   main_tnr_enable;
    enum vpss_isp_op_type  _3d_nr_mode;
    struct vpss_isp_3d_nr manual_3d_nr;
    struct vpss_isp_3d_nr linkage_3d_nr[12];
};


struct vpss_lens_coef{
    unsigned short coef_b[10];    //[0,255]
    unsigned short coef_c[10];    //[0,1023]
};

struct vpss_lsc_attr {
    unsigned char		 lsc_mode;
    unsigned char	     linkage_strength[13];   //[0，1023]
    unsigned short		 lsc_enable;
    //the reference point of lens correction
    unsigned short		 xref;		  //[0,4096]
    unsigned short		 yref;		  //[0,4096]
    unsigned short		 lsc_shift;   //[0，15]
    struct vpss_lens_coef   lsc_r_coef;
    struct vpss_lens_coef   lsc_gr_coef;
    struct vpss_lens_coef   lsc_gb_coef;
    struct vpss_lens_coef   lsc_b_coef;
    //the range of ten segment
    //the range of ten segment
    unsigned short		 range[10];   //[0，1023]
};

struct vpss_yuv_effect_attr {
    unsigned char   yuv_out_enable;
    unsigned char	dark_margin_en;   //黑边使能
    unsigned char	y_a;	 // [0, 255]
    signed  char    y_b;	 //[-128, 127]
    signed short    uv_a;    //[-256, 255]
    signed short    uv_b;    //[-256, 255]
};

struct vpss_rgb_gamma {
    unsigned short	 rgb_gamma_enable;
    unsigned short	 r_gamma[129];	 //10bit
    unsigned short	 g_gamma[129];	 //10bit
    unsigned short	 b_gamma [129];  //10bit
    unsigned short	 r_key[16];
    unsigned short	 g_key[16];
    unsigned short	 b_key[16];
};

struct vpss_rgb_gamma_attr {
    unsigned short	  gain_th1;
    unsigned short	  gain_th2;
    struct vpss_rgb_gamma   linkage_rgb_gamma[3];
};

struct vpss_contrast {
     unsigned char      contrast_enable;
     unsigned short     y_contrast;	//[0,511]
     signed short       y_shift; 	//[0, 511]
};

struct vpss_auto_contrast {
    unsigned char   contrast_enable;
    unsigned char	shift_max;			//[0, 127]
    unsigned short	dark_pixel_area;	//[0, 511]
    unsigned short	dark_pixel_rate;	//[1, 256]

};

struct vpss_contrast_attr {
    enum vpss_isp_op_type       contrast_mode;	//模式选择，手动或者联�?
    struct vpss_contrast        manual_contrast;
    struct vpss_auto_contrast	linkage_contrast[12];
};

struct vpss_lce {
    unsigned char	lce_enable;
    unsigned char	lce_uv_adjust_en;
    unsigned char	lce_uv_adjust_level;
    unsigned char   lce_he_str;
    unsigned char   lce_he_str_low;
    unsigned char   lce_lumFilter_k; //[0,5]
    unsigned char   lce_spatFilter_k;//[0,5]
    unsigned char   lce_tempFilter_k;//[0,15]
    unsigned char   lce_strength[4][8];
};

struct vpss_lce_attr {
    enum vpss_isp_op_type	lce_mode;
    struct vpss_lce		    manual_lce;
    struct vpss_lce		    linkage_lce[12];
};

struct vpss_y_gamma_attr {
    unsigned short    ygamma_enable;
    unsigned char	  ygamma_uv_adjust_enable;
    unsigned char	  ygamma_uv_adjust_level;
    unsigned short	  ygamma[129];	  //10bit
    unsigned short	  ygamma_key[16]; //曲线的关键点
    unsigned short	  ygamma_cnoise_yth1;	//Ygamma色差抑制门限值
    unsigned short	  ygamma_cnoise_yth2;	//Ygamma色差抑制门限值
    //unsigned short	  ygamma_cnoise_slop;
    unsigned short	  ygamma_cnoise_gain ;	//UV调整系数计算参数
};

/*
struct sharp_reduce_factor_attr {
    unsigned short sharp_reduce_factor[36][64];
};

struct vpss_sharp_reduce_factor_attr {
    struct sharp_reduce_factor_attr manual_attr;
    struct sharp_reduce_factor_attr linkage_attr[12];
};
*/


struct vpss_3d_nr_ref_size_stat_info{
    unsigned int  ref_size_statis[3];/* 0-y used size; 1-u used size; 2-v used size*/
};

#if 1
struct vpss_ccm {
    unsigned char    cc_enable;
    unsigned char    cc_sat_tbl[9];       //[0, 15]
    signed short ccm[3][3]; //[-2048, 2047]
};

struct vpss_auto_ccm {
    unsigned short cc_color_temp;
    struct vpss_ccm auto_ccm_para;
};

struct vpss_ccm_ctr {
    unsigned short cc_start_gain;
    unsigned short cc_min_saturation;
    unsigned short cc_adjust_slop;
};

struct vpss_manual_ccm_attr {
    struct vpss_ccm_ctr manual_ccm_ctrl;
    struct vpss_ccm manual_ccm_para;
};

struct vpss_auto_ccm_attr {
    unsigned char color_matrix_num;
    struct vpss_ccm_ctr auto_ccm_ctrl;
    struct vpss_auto_ccm auto_ccm_para[10];
};

struct vpss_ccm_attr {
    enum vpss_isp_op_type cc_mode; //颜色校正矩阵联动或者手动
    struct vpss_manual_ccm_attr manual_ccm;
    struct vpss_auto_ccm_attr auto_ccm; //四个联动矩阵
};

struct vpss_hue {
    unsigned char hue_sat_en; // hue使能
    signed char hue_lut_a[65]; //[-128, 127]
    signed char hue_lut_b[65]; //[-128, 127]
    unsigned char hue_lut_s[65]; //[0, 255]
};

struct vpss_auto_hue {
    unsigned short hue_color_temp;
    struct vpss_hue auto_hue_para;
};

struct vpss_auto_hue_attr {
    unsigned short hue_num;
    struct vpss_auto_hue auto_hue_para[10];
};

struct vpss_hue_attr {
    enum vpss_isp_op_type hue_mode; //联动或者手动
    struct vpss_hue manual_hue;
    struct vpss_auto_hue_attr auto_hue; //四个联动参数
};

struct vpss_gb {
    unsigned short gb_enable; //使能位
    unsigned char gb_en_th; //[0,255]
    unsigned char gb_kstep; //[0,15]
    unsigned short gb_threshold; //[0,1023
};

struct vpss_gb_attr {
    enum vpss_isp_op_type gb_mode; //模式选择，手动或者联动
    struct vpss_gb manual_gb;
    struct vpss_gb linkage_gb[12];
};

struct vpss_ddpc {
    //unsigned char  ddpc_enable;        //动态坏点使能位
    unsigned char  white_dpc_enable; //白点消除使能位
    unsigned char  black_dpc_enable; //黑点消除使能位
    unsigned char  ddpc_th_slop;
    unsigned char  ddpc_strength;
    unsigned short ddpc_th_base;          //10bit
};

struct vpss_ddpc_attr {
    enum vpss_isp_op_type ddpc_mode; //模式选择，手动或者联动
    struct vpss_ddpc manual_ddpc;
    struct vpss_ddpc linkage_ddpc[12];
};
struct vpss_nr1 {
    unsigned short   nr1_k;                 //[0,15]
    unsigned char    nr1_enable;            //ä½¿èƒ½ä½
    unsigned char    nr_keepth;
    unsigned char    nr_keepstr;
    unsigned char    nr_lum_str[32];

    unsigned char    de_enhance_enable;
    unsigned char    de_range;
    unsigned char    de_str;
    unsigned char    de_lum_wgt[32];

    unsigned short   nr1_calc_g_k;
    unsigned short   nr1_calc_r_k;
    unsigned short   nr1_calc_b_k;
    //AK_U16 nr1_lc_lut[17];        //10bit
    unsigned short   nr1_weight_rtbl[17];   //10bit
    unsigned short   nr1_weight_gtbl[17];   //10bit
    unsigned short   nr1_weight_btbl[17];   //10bit
};

struct vpss_nr1_attr {
    enum vpss_isp_op_type nr1_mode; // nr1 模式，自动或者联动模式
    struct vpss_nr1 manual_nr1;
    struct vpss_nr1 linkage_nr1[12]; //联动参数
};

struct vpss_nr2 {
    unsigned char      nr2_enable;
    unsigned char      nr2_k;                 //[0,15]
    unsigned char      ynr_y_dpc_enable;
    unsigned char      ynr_y_black_dpc_enable;
    unsigned char      ynr_y_white_dpc_enable;
    unsigned char      ynr_keep_th;
    unsigned char      ynr_keep_str;
    unsigned char      ynr_localstr[16];
    unsigned char      ynr_dpc_model;
    unsigned char      ynr_edge1nr_str;
    unsigned char      ynr_edge1nr_range;
    unsigned char      ynr_edge2nr_str;
    unsigned char      ynr_edge2nr_range;
    unsigned short     ynr_intensity1;//[0, 15],default:15
    unsigned short     ynr_intensity2;//[0, 15],default:15
    unsigned short     nr2_calc_y_k;
    unsigned short     nr2_weight_tbl[17];    //10bit
    unsigned short     ynr_dpc_th;
};

struct vpss_nr2_attr {
    enum vpss_isp_op_type nr2_mode; //手动或者联动模式
    struct vpss_nr2 manual_nr2;
    struct vpss_nr2 linkage_nr2[12];
};

struct vpss_uvnr {
    unsigned char    uvnr_enable;            //使能位
    unsigned char    uvnr_k;
    unsigned short   uvnr_calc_k;
    unsigned short   uvnr_weight_tbl[17];    //10bit
};

struct vpss_uvnr_attr {
    enum vpss_isp_op_type uvnr_mode; //手动或者联动模式
    struct vpss_uvnr manual_uvnr;
    struct vpss_uvnr linkage_uvnr[12];
};

struct vpss_sharp {
    unsigned char    ysharp_enable;              //[0,1]
    unsigned char    ysharp_nr_enable;           //[0,1]
    unsigned char   edge_enable;
    unsigned char    sharp_method;               //[0,3]
    unsigned char    mf_hpf_k;                   //[0,127]
    unsigned char    mf_hpf_shift;               //[0,15]
    unsigned char    hf_hpf_k;                   //[0,127]
    unsigned char    hf_hpf_shift;               //[0,15]
    unsigned char    ysharp_nr_k;
    unsigned char   ysharp_nr_keepTh;
    unsigned char   ysharp_nr_keepStr;
    unsigned char   ysharp_mfacotr_sup;
    unsigned char   ysharp_freqctrl;
    //unsigned char  ysharp_reduce_factor[36][64];
    unsigned char   ysharp_lumWgt[32];
    unsigned char    ysharpStr[16];
    unsigned char   edgeStr[32];
    unsigned char   edgeFiltStr;
    unsigned char   edgeWhiteGain;
    unsigned char   edgeBlackGain;
    unsigned char   edgeWideWgt;
    unsigned char   edge45DWgt;
    unsigned char   weakEdgeCtrl;
    unsigned char   localShootSupTh;        //[0,31]
    unsigned char   localShootSupSlop;          //[0,15] default
    unsigned char   ovShootSupStr;              //[0, 31]
    unsigned char   udShootSupStr;              //[0, 31]
    signed char     UVGain[4];
    signed char     UVArea[4][4];

    unsigned short   ysharp_nr_calc_k;
    unsigned short   ysharp_nrweight_tbl[17];
    unsigned short  EdgeGainMax; //[0, 511]
    signed short   MF_HPF_LUT[256];            //[-256,255]
    signed short   HF_HPF_LUT[256];            //[-256,255]
    unsigned short   MF_LUT_KEY[16];
    unsigned short   HF_LUT_KEY[16];
};

struct vpss_sharp_attr{
    unsigned char    sharp_debug_output;         //[0,3]
    enum vpss_isp_op_type ysharp_mode;
    struct vpss_sharp manual_sharp_attr;
    struct vpss_sharp linkage_sharp_attr[12];
};

struct vpss_fcs {
    unsigned char    fcs_enable;         //使能位
    unsigned char    fcs2_enable;
    unsigned char    fcs_th;         //[0, 255]
    unsigned char    fcs_gain_slop;  //[0,63]
    unsigned char    fcs2_c_th;   //[0, 127]
    unsigned char    fcs2_c_str;  //[0, 31]
    unsigned char    fcs2_s_th;   //[0, 127]
    unsigned char    fcs2_s_str;  //[0, 31]
    unsigned char    dm_fcs_purple_str;//[0, 31]
    unsigned char    dm_fcs_cyan_str;  //[0, 31]
    unsigned char    dm_fcs_chrom_k;   //[0, 31]
    unsigned char    dm_fcs_hfp_k;     //[0, 31]
    unsigned char    hue_satSupTh;  //[0, 15]
    unsigned char    hue_satSupStr; //[0, 15]
    unsigned char    pfs_str;       //[0, 63]
    unsigned char    pfs_start_c;   //[0, 63]
    unsigned char    pfs_range;     //[0, 15]
    unsigned char    pfs_highlight_th;      //[0, 255]
    unsigned char    pfs_lowlight_th;       //[0, 255]
};

struct vpss_fcs_attr {
    enum vpss_isp_op_type fcs_mode; //模式选择，手动或者联动
    struct vpss_fcs manual_fcs;
    struct vpss_fcs linkage_fcs[12];
};

struct vpss_saturation {
    unsigned char     SE_enable;    // 使能位
    unsigned char     SE_scale1;       //[0,255]
    unsigned char     SE_scale2;       //[0,255]
    unsigned char     SE_scale3;       //[0,255]
    //unsigned char   SE_scale_slop1;  //[0,255]
    //unsigned char   SE_scale_slop2;  //[0,255]
    unsigned short    SE_th1;          //[0, 1023]
    unsigned short    SE_th2;          //[0, 1023]
    unsigned short    SE_th3;          //[0, 1023]
    unsigned short    SE_th4;          //[0, 1023]
};

struct vpss_saturation_attr {
    enum vpss_isp_op_type SE_mode; //饱和度模式
    struct vpss_saturation manual_sat;
    struct vpss_saturation linkage_sat[12];
};

struct vpss_raw_lut_attr {
    unsigned short    raw_gamma_enable;
    unsigned short    r_key[16];
    unsigned short    g_key[16];
    unsigned short    b_key[16];
    unsigned short    raw_r[129];      //10bit
    unsigned short    raw_g[129];      //10bit
    unsigned short    raw_b[129];      //10bit
};
#endif





struct vpss_blk_lumi_attr {
    unsigned int blk_lumi[16][16];
};
#pragma pack()


#endif

/* end of file */
