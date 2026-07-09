#ifndef __AK_ISP_PARAM_H__
#define __AK_ISP_PARAM_H__
#define ISP_DRV_LIB_VER     "isp_drv_lib-V0.1.0"

#include "ak_types.h"
/*--------------------------------IQT_struct--------------------------------*/
#pragma pack(1)
typedef enum ak_isp_op_type{
    OP_TYPE_MANUAL = 0,
    OP_TYPE_AUTO = 1,
    OP_TYPE_BUTT,
}AK_ISP_OP_TYPE;
#pragma pack()  

#pragma pack(1)
typedef enum ak_isp_rgb2yuv_model {
    BT601_MODEL = 0,
    BT709_MODEL = 1,
    BT2020_MODEL = 2,
}AK_ISP_RGB2YUV_MODEL;
#pragma pack()

#pragma pack(1)
typedef enum ak_isp_awb_status{
    AWB_STATUS_PAUSE = 0,
    AWB_STATUS_NORMAL,
}AK_ISP_AWB_STATUS;
 #pragma pack()
 
#pragma pack(1)
typedef enum ak_isp_awb_algo_type{
    AWB_ALGO_DEFAULT = 0,
    AWB_ALGO_AVERAGE_WEIGHT = 1,
    AWB_ALGO_BLOCK_ASSIST,
    AWB_ALGO_GW,
    AWB_ALGO_BUTT,
}AK_ISP_AWB_ALGO_TYPE;
#pragma pack()


#pragma pack(1)
typedef struct ak_isp_misc_attr { 
    AK_U16   hsyn_pol;
    AK_U16   vsync_pol;
    AK_U16   pclk_pol;
    AK_U16   test_pattern_en;
    //只用了这部分
    AK_U8    test_pattern_cfg;
    AK_U8    cfa_mode;
    AK_U8    inputdataw;
    AK_U8    yuv_swing;
    AK_U8    yuv_formula;
    //只用了这部分
    AK_U16   one_line_cycle;
    AK_U16   hblank_cycle;
    AK_U16   vflip;
    AK_U16   hflip;
    AK_U16   frame_start_delay_en;
    AK_U16   frame_start_delay_num;
    AK_U16   frame_start_edge;

    AK_U16   mipi_virtual_channel_sel; 
    AK_U16   mipi_bps_virtual_channel_sel; 
    AK_U16   mipi_hsync_str_sel;
    AK_U16   mipi_hsync_end_sel;
    AK_U16   mipi_auto_decoder_pix;
} AK_ISP_MISC_ATTR;  
#pragma pack()
typedef struct ak_isp_wb_gain {
    AK_U16	r_gain;
    AK_U16	g_gain;
    AK_U16	b_gain;
    AK_S16  r_offset;
    AK_S16  g_offset;
    AK_S16  b_offset;
}AK_ISP_WB_GAIN;

typedef struct ae_fast_struct {
    AK_S32 sensor_exp_time;
    AK_S32 sensor_a_gain;
    AK_S32 sensor_d_gain;
    AK_S32 isp_d_gain;
    AK_ISP_WB_GAIN wb;
}AK_ISP_AE_FAST;
/*--------------------------------isptool_struct--------------------------------*/
#pragma pack(1)
typedef struct ak_isp_pwl_decom_attr{
	AK_U8  pwl_decom_enable;
    AK_U16 piece_th[4];    //[0, 4095]
    AK_U16 piece_a[5];    //[0, 4095]
    AK_U8  piece_b[5];    //[0, 7]
}AK_ISP_PWL_DECOM_ATTR;
#pragma pack()

typedef struct ak_isp_wdr{
    AK_U8   wdr_enable;
    AK_U8   wdr_debug_mode;
    AK_U8   wdr_long_bits;          // [10, 16]
    AK_U8   wdr_short_bits;         // [10, 16]
    AK_U8   wdr_mix_th;             //[0, 255]
    AK_U8   wdr_mix_slop;           //6bits
    AK_U8   wdr_short_nr_th;        //[0, 255]
    AK_U8   wdr_short_nr_str;       //[0, 15]
    AK_U8   wdr_md_long_wgt;        //[0, 255]
    AK_U8   wdr_md_th;              //[0, 255]
    AK_U8   wdr_md_slop;            //[0, 255]
    AK_U8   wdr_md_spr_th;          //[0, 255]
    AK_U8   wdr_md_spr_slop;        //[0, 15]
    AK_U8   wdr_forcelong_th;       //[0, 255]
    AK_U8   wdr_forcelong_slop;     //[0, 255]
    AK_U8   wdr_pre_gain;           //[0, 255]
    AK_U16  wdr_hist_enable;
    AK_U16  wdr_gain_L;             //12bits 
    AK_S16  wdr_r_offset;           //[-2048, 2047] 
    AK_S16  wdr_g_offset;           //[-2048, 2047]
    AK_S16  wdr_b_offset;           //[-2048, 2047]

    AK_U8   wdr_pwl_long;           // 1 bit
    AK_U8   wdr_pwl_short;          // 1 bit
    AK_U16  piece_th[4];            //[0, 4095]
    AK_U16  piece_a[5];             //[0, 4095]
    AK_U8   piece_b[5];             //[0, 7]
}AK_ISP_WDR_ATTR;
#pragma pack(1)
typedef struct ak_isp_raw_g_hist_attr{
    AK_U8  raw_hist_enable;
    AK_U8  raw_rshift;   //[0, 15]
    AK_U8  zone_weight[8][16];
}AK_ISP_RAW_G_HIST_ATTR;
#pragma pack()

typedef struct ak_isp_rgb_hist_attr{
    AK_U8  rgb_hist_enable;
    AK_U8  zone_weight[8][16];
}AK_ISP_RGB_HIST_ATTR;

#pragma pack(1)
typedef struct ak_isp_yuv_hist_attr
{
    AK_U16  yuv_hist_enable;
    AK_U8  zone_weight[8][16];
}AK_ISP_YUV_HIST_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_blc {
    AK_U16    black_level_enable; //使能位
    AK_U16    bl_r_a;            //[ 0,1023]
    AK_S16    bl_r_offset;       //[-2048,2047]
    AK_U16    bl_gr_a;           //[ 0,1023]
    AK_S16    bl_gr_offset;      //[-2048,2047]
    AK_U16    bl_gb_a;           //[ 0,1023]
    AK_S16    bl_gb_offset;      //[-2048,2047]
    AK_U16    bl_b_a;            //[ 0,1023]
    AK_S16    bl_b_offset;       //[-2048,2047]
}AK_ISP_BLC;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_blc_attr {
    AK_ISP_OP_TYPE  blc_mode;              //0联动模式，1手动模式
    AK_ISP_BLC      manual_blc;
    AK_ISP_BLC      linkage_blc[12];
}AK_ISP_BLC_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct {
    AK_U16 coef_b[10];     //[0,255]
    AK_U16 coef_c[10];     //[0,1023]
}lens_coef;
#pragma pack()


#pragma pack(1)
typedef struct ak_isp_lsc_attr {
    AK_U8         lsc_mode;
    AK_U8         linkage_strength[13];   //[0，1023]
    //the reference point of lens correction
    AK_U16        lsc_enable;
    AK_U16        xref;        //[0,4096]
    AK_U16        yref;        //[0,4096]
    AK_U16        lsc_shift;   //[0，15]
    lens_coef   lsc_r_coef;
    lens_coef   lsc_gr_coef;
    lens_coef   lsc_gb_coef;
    lens_coef   lsc_b_coef;
    //the range of ten segment
    AK_U16        range[10];   //[0，1023]
}AK_ISP_LSC_ATTR;
#pragma pack()

typedef struct ak_isp_mesh_lsc_attr{
    AK_U16   mesh_lsc_enable;
    AK_U8    mesh_lsc_mode;
    AK_U8    linkage_strength[13];   
    AK_U16  mesh_color_temp[3];
    AK_U16   gain_tbl_R[3][25][33]; //[0, 4095]
    AK_U16   gain_tbl_G[3][25][33];
    AK_U16   gain_tbl_B[3][25][33];
}AK_ISP_MESH_LSC_ATTR;

#pragma pack(1)
typedef struct ak_isp_drc{
    AK_U8    drc_enable;
    AK_U8    drc_rshift;             //data right-shift when drc disable [0, 7]
    AK_U8    drc_pre_shift;          //data left-shift before drc[0, 2]
    AK_U8    drc_pwl_compress;        //[0, 1]
    AK_U8    drc_lum_wgt_tbl[8];     //[0, 255]      
    AK_U8    drc_weight_k;           //[0,15]
    AK_U8    drc_he_distributor[6];  //[0, 63]
    AK_U8    drc_spatialfilter_k;    //[0,5]
    AK_U16   drc_clip_th;            //[0, 65535]
    AK_U16   drc_str;            //[0,1023]
    AK_U16   drc_str_low;        //[0,1023]
    AK_U16   drc_global_lut[8];      //[0, 65535]
    AK_U16   drc_global_lut_key[8];
    AK_U16   drc_global_lut_key_value[8];      //10bit
    //AK_U16   drc_lut[8][16][6];     //[0, 1023]
    //AK_U64   drc_areastat[8][16][7]; //18bits
    //AK_U16   drc_arealumi[8][16];       
}AK_ISP_DRC;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_drc_attr{
    AK_ISP_OP_TYPE  drc_mode;             //nr1 模式，自动或者联动模式
    AK_ISP_DRC      manual_drc;
    AK_ISP_DRC      linkage_drc[12];       //联动参??
}AK_ISP_DRC_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_raw_lut_attr {
    AK_U16    raw_gamma_enable;
    AK_U16    r_key[16];
    AK_U16    g_key[16];
    AK_U16    b_key[16];
    AK_U16    raw_r[129];      //10bit
    AK_U16    raw_g[129];      //10bit
    AK_U16    raw_b[129];      //10bit
}AK_ISP_RAW_LUT_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_rgb_gamma {
    AK_U16    rgb_gamma_enable;
    AK_U16    r_gamma[129];   //10bit
    AK_U16    g_gamma[129];   //10bit
    AK_U16    b_gamma [129];  //10bit
    AK_U16    r_key[16];
    AK_U16    g_key[16];
    AK_U16    b_key[16];
}AK_ISP_RGB_GAMMA;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_rgb_gamma_attr {
    AK_U16     gain_th1;
    AK_U16     gain_th2;
    AK_ISP_RGB_GAMMA  linkage_rgb_gamma[3];
}AK_ISP_RGB_GAMMA_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_y_gamma_attr {
    AK_U16    ygamma_enable;
    AK_U8      ygamma_uv_adjust_enable;
    AK_U8      ygamma_uv_adjust_level;
    AK_U16     ygamma[129];    //10bit
    AK_U16     ygamma_key[16]; //曲线的关键点
    AK_U16     ygamma_cnoise_yth1;   //Ygamma色差抑制门限值
    AK_U16     ygamma_cnoise_yth2;   //Ygamma色差抑制门限值
    //AK_U16   ygamma_cnoise_slop;
    AK_U16     ygamma_cnoise_gain ;  //UV调整系数计算参数
}AK_ISP_Y_GAMMA_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_nr1 {
    AK_U16   nr1_k;                 //[0,15]
    AK_U8    nr1_enable;            //ä½¿èƒ½ä½
    AK_U8    nr_keepth;
    AK_U8    nr_keepstr;
    AK_U8    nr_lum_str[32]; 

    AK_U8    de_enhance_enable;
    AK_U8    de_range;
    AK_U8    de_str;
    AK_U8    de_lum_wgt[32]; 
    
    AK_U16   nr1_calc_g_k;
    AK_U16   nr1_calc_r_k;
    AK_U16   nr1_calc_b_k;
    //AK_U16 nr1_lc_lut[17];        //10bit
    AK_U16   nr1_weight_rtbl[17];   //10bit
    AK_U16   nr1_weight_gtbl[17];   //10bit
    AK_U16   nr1_weight_btbl[17];   //10bit
}AK_ISP_NR1;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_nr1_attr {
    AK_ISP_OP_TYPE  nr1_mode;             //nr1 模式，自动或者联动模式
    AK_ISP_NR1  manual_nr1;
    AK_ISP_NR1  linkage_nr1[12];       //联动参数
}AK_ISP_NR1_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_nr2 {
    AK_U8      nr2_enable;
    AK_U8      nr2_k;                 //[0,15]
    AK_U8      ynr_y_dpc_enable;
    AK_U8      ynr_y_black_dpc_enable;
    AK_U8      ynr_y_white_dpc_enable;
    AK_U8      ynr_keep_th;
    AK_U8      ynr_keep_str;
    AK_U8      ynr_localstr[16];
    AK_U8      ynr_dpc_model;
    AK_U8      ynr_edge1nr_str;
    AK_U8      ynr_edge1nr_range;
    AK_U8      ynr_edge2nr_str;
    AK_U8      ynr_edge2nr_range;
    AK_U16     ynr_intensity1;//[0, 15],default:15
    AK_U16     ynr_intensity2;//[0, 15],default:15
    AK_U16     nr2_calc_y_k;
    AK_U16     nr2_weight_tbl[17];    //10bit
    AK_U16     ynr_dpc_th;
}AK_ISP_NR2;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_nr2_attr {
    AK_ISP_OP_TYPE    nr2_mode;         //手动或者联动模式
    AK_ISP_NR2  manual_nr2;
    AK_ISP_NR2  linkage_nr2[12];
}AK_ISP_NR2_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_uvnr {
    AK_U8    uvnr_enable;            //使能位
    AK_U8    uvnr_k; 
    AK_U16   uvnr_calc_k;
    AK_U16   uvnr_weight_tbl[17];    //10bit
}AK_ISP_UVNR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_uvnr_attr {
    AK_ISP_OP_TYPE  uvnr_mode;       //手动或者联动模式
    AK_ISP_UVNR  manual_uvnr;
    AK_ISP_UVNR  linkage_uvnr[12];
}AK_ISP_UVNR_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_3d_nr {
    AK_U8        tnr_enable;         //default:1(GUI edit)
    AK_U8		 tnr_ref_rc_enable;
    AK_U8        tnr_refFrame_format;    //参考帧压缩格式
    //AK_U16     t_mf_slop; //[0,255] //65536/(t_mf_th2-t_mf_th1)
    AK_U8        ynr_DPCStr;                 //[0, 127]
    AK_U8        ynr_DPCCap;                 //[0, 127]
    AK_U8        ynr_DPCDiffGain[8];         //[0,7]
    AK_U8        ynr_k;          //[0,15]
    AK_U8        ynr_lum_str[32]; //[0, 15]
    AK_U8        ylp_k1;         //[0, 15],default:7(GUI edit)
    AK_U8        ylp_k2;         //[0, 15],default:0(GUI edit)
    AK_U8        yDiffUVWgt;     //[0, 15]
    AK_U8        yDiffEdgeWgt;   //[0, 15]
    AK_U8        t_y_low1;       //[0,127]
    AK_U8        t_y_low2;       //[0,127]
    AK_U8        t_y_k1;         //[0, 127],default:120(GUI edit)
    AK_U8        t_y_k2;         //[0, 127],default:120(GUI edit)
    AK_U8        t_y_ex_k;       //[0, 15]
    AK_U8        t_y_kslop1;     //[0, 127],default:32(GUI edit)
    AK_U8        t_y_kslop2;     //[0, 127]
    AK_U8        t_y_src_choose_k_th;        //[0, 127]
    //AK_U8      t_y_src_choose_slop;        //[0, 63]
    AK_U8        t_y_update_th;  

    AK_U8        uvnr_k;         //[0, 15],default:8(GUI edit)
    AK_U8        uvlp_k1;        //[0, 15],default:7(GUI edit)
    AK_U8        uvlp_k2;        //[0, 15],default:0(GUI edit)
    AK_U8        t_uv_diff_scale;//[0, 15]
    AK_U8        t_uv_k_low1;    //[0, 127]
    AK_U8        t_uv_k_low2;    //[0, 127]
    AK_U8        t_uv_k1;        //[0, 127],default:120(GUI edit)
    AK_U8        t_uv_k2;        //[0, 127]
    AK_U8        t_uv_ex_k;      //[0, 15],
    AK_U8        t_uv_kslop1;        //[0, 127],default:32(GUI edit)
    AK_U8        t_uv_kslop2;        //[0, 127],default:32(GUI edit)
    AK_U8        t_uv_src_choose_k_th; //[0, 127]
    //AK_U8      t_uv_src_choose_slop; //[0, 63]
	AK_U8		 t_ref_rc_bit;
	AK_S8		 t_ref_rc_low_limit;
	AK_S8		 t_ref_rc_hihg_limit;
    AK_U8        mfactor_k_th;       //[0, 127]
    AK_U8        mfactor_slop;       //[0, 15]
    //AK_U16     sharp_factor_max;       //[0, 15]     
    
    AK_U8        motion_stat_th;         //[0, 15]
    AK_U8        sfactor_stat_th;        //[0, 3]
    AK_U8        motion_filter;          //[0,15]
    AK_U8        motion_area_expand;     //[0, 1]
    AK_U8        lumStr[32];             //[0, 15]
    //AK_U8      motion_flag[36][64];//运动检测输出
    AK_U16       t_mf_th1;   //[0, 8191],default:300(GUI edit)
    AK_U16       t_mf_th2;   //[0, 8191],default:500(GUI edit)
    AK_U16       ynr_calc_k;     //[0,65535](GUI edit)
    AK_U16       ynr_weight_tbl[17];//ynr_strength(GUI edit)
    AK_U16       t_y_th1;        //[0, 511],default:48(GUI edit)
    AK_U16       t_y_th2;        //[0, 511],default:16(GUI edit)
    AK_U16       t_uv_th1;       //[0, 511],default:48(GUI edit)
    AK_U16       t_uv_th2;       //[0, 511],default:16(GUI edit)
    AK_U16       md_th;          //[0, 65535]
    AK_U16       weight4_7X5;    //[0,8695]
    AK_U16      tnr_motion_flag_th;
}AK_ISP_3D_NR;
#pragma pack()

#pragma pack(1)
typedef struct  ak_isp_3d_nr_attr {
    AK_U8    tnr_debug_output;   //[0,1]
    AK_U8   main_tnr_enable;
    AK_ISP_OP_TYPE  _3d_nr_mode;
    AK_ISP_3D_NR    manual_3d_nr;
    AK_ISP_3D_NR    linkage_3d_nr[12];
}AK_ISP_3D_NR_ATTR;
#pragma pack()

typedef struct ak_isp_3d_nr_ref_attr {
    AK_U64   yaddr_3d;
    AK_U32   ysize_3d;
    AK_U64   uaddr_3d;
    AK_U32   usize_3d;
    AK_U64   vaddr_3d;
    AK_U32   vsize_3d;
}AK_ISP_3D_NR_REF_ATTR;


typedef struct ak_isp_3d_nr_ivref_add_stat {
    AK_U64   yaddr_head;
    AK_U64   yaddr_tail;
    AK_U64   uaddr_head;
	AK_U64   uaddr_tail;
	AK_U64   vaddr_head;
    AK_U64   vaddr_tail;
}AK_ISP_3D_NR_IVREF_ADD_STAT;


#pragma pack(1)
typedef struct  ak_isp_3d_nr_stat_info {
    AK_U16       MD_stat_max;
    AK_U16       MD_stat[36][64];        
    AK_U32       MD_level;
}AK_ISP_3D_NR_STAT_INFO;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_3d_nr_rer_size_stat_info{
    AK_U32  ref_size_statis[3];
}AK_ISP_3D_NR_REF_SIZE_STAT_INFO;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_gb {
    AK_U16    gb_enable;        //使能位
    AK_U8     gb_en_th;         //[0,255]
    AK_U8     gb_kstep;         //[0,15]
    AK_U16    gb_threshold;     //[0,1023
} AK_ISP_GB;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_gb_attr {
    AK_ISP_OP_TYPE   gb_mode;      
    AK_ISP_GB  manual_gb;
    AK_ISP_GB  linkage_gb[12];
} AK_ISP_GB_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_demo_attr {
    AK_U16  dmmosa_enable;
    AK_U16   dm_rg_thre;     //[0 1023]
    AK_U16   dm_bg_thre;     //[0 1023]
    AK_U16   dm_hf_th1;      //[0, 1023]
    AK_U16   dm_hf_th2;      //[0, 1023]
    
    AK_U16   dm_hv_th;       //方向判别系数
    AK_U8    dm_rg_gain;     //[0 255]
    AK_U8    dm_bg_gain;     //[0 255]
    AK_U8    dm_gr_gain;     //[0 255]
    AK_U8    dm_gb_gain;     //[0 255]
}AK_ISP_DEMO_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_ccm {
    AK_U8    cc_enable;
    AK_U8   cc_sat_tbl[9];       //[0, 15]   
    AK_S16    ccm[3][3];        //[-2048, 2047]
}AK_ISP_CCM;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_auto_ccm {
    AK_U16   cc_color_temp;
    AK_ISP_CCM      auto_ccm_para;
}AK_ISP_AUTO_CCM;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_ccm_ctr {  
   AK_U16    cc_start_gain;
   AK_U16    cc_min_saturation;
   AK_U16    cc_adjust_slop;    
}AK_ISP_CCM_CTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_manual_ccm_attr {     
    AK_ISP_CCM_CTR manual_ccm_ctrl;
    AK_ISP_CCM     manual_ccm_para; 
}AK_ISP_MANUAL_CCM_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_auto_ccm_attr{    
    AK_U8  color_matrix_num;
    AK_ISP_CCM_CTR  auto_ccm_ctrl;
    AK_ISP_AUTO_CCM auto_ccm_para[10];
}AK_ISP_AUTO_CCM_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_ccm_attr {
    AK_ISP_OP_TYPE          cc_mode;        //颜色校正矩阵联动或者手动
    AK_ISP_MANUAL_CCM_ATTR  manual_ccm;
    AK_ISP_AUTO_CCM_ATTR    auto_ccm;       //四个联动矩阵
}AK_ISP_CCM_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_hdr {
    AK_U8   hdr_enable;
    AK_U8   hdr_uv_adjust_enable;         //uv调整使能
    AK_U16   hdr_th1;      //0-1023
    AK_U16   hdr_th2;      //0-1023
    AK_U16   hdr_th3;      //0-1023
    AK_U16   hdr_th4;      //0-1023
    AK_U16   hdr_th5;      //0-1023

    //AK_U16 wdr_light_weight;

    AK_U16   area_tb1[65];     //曲线 10bit
    AK_U16   area_tb2[65];     //曲线 10bit
    AK_U16   area_tb3[65];     //曲线 10bit
    AK_U16   area_tb4[65];     //曲线 10bit
    AK_U16   area_tb5[65];     //曲线 10bit
    AK_U16   area_tb6[65];     //曲线 10bit

    AK_U16   area1_key[16];
    AK_U16   area2_key[16];
    AK_U16   area3_key[16];
    AK_U16   area4_key[16];
    AK_U16   area5_key[16];
    AK_U16   area6_key[16];  
    
    //AK_U8   hdr_spatialfilt_pos;
    
    AK_U16    hdr_cnoise_suppress_yth1;   //色彩噪声亮度阈值1
    AK_U16    hdr_cnoise_suppress_yth2;   //色彩噪声亮度阈值2
    AK_U16    hdr_cnoise_suppress_gain;   //色差抑制
    AK_U8     hdr_cnoise_suppress_slop;   //抑制斜率
    AK_U8     hdr_uv_adjust_level;        //uv调整程度, [0,31]
    AK_U8     hdr_spatialfilt_k;//[0,5]
}AK_ISP_HDR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_hdr_attr {
    AK_ISP_OP_TYPE hdr_mode;              //模式选择，手动或者联动
    AK_ISP_HDR manual_hdr;
    AK_ISP_HDR linkage_hdr[12];
}AK_ISP_HDR_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_lce {
    AK_U8    lce_enable;
    AK_U8    lce_uv_adjust_en;
    AK_U8    lce_uv_adjust_level;
    AK_U8   lce_he_str;
    AK_U8    lce_he_str_low;
    AK_U8    lce_lumFilter_k; //[0,5]
    AK_U8    lce_spatFilter_k;//[0,5]
    AK_U8    lce_tempFilter_k;//[0,15]
    AK_U8    lce_strength[4][8];
}AK_ISP_LCE;
#pragma pack()

typedef struct ak_isp_lce_hist_stat_info{
    AK_U32     lce_hist_stat[4][8][8];     
}AK_ISP_LCE_HIST_STAT_INFO;

#pragma pack(1)
typedef struct ak_isp_lce_attr {
    AK_ISP_OP_TYPE  lce_mode;            
    AK_ISP_LCE      manual_lce;
    AK_ISP_LCE      linkage_lce[12];
}AK_ISP_LCE_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_sharp {
    AK_U8    ysharp_enable;              //[0,1]
    AK_U8    ysharp_nr_enable;           //[0,1]
    AK_U8   edge_enable;
    AK_U8    sharp_method;               //[0,3]
    AK_U8    mf_hpf_k;                   //[0,127]
    AK_U8    mf_hpf_shift;               //[0,15]
    AK_U8    hf_hpf_k;                   //[0,127]
    AK_U8    hf_hpf_shift;               //[0,15]
    AK_U8    ysharp_nr_k;  
    AK_U8   ysharp_nr_keepTh;
    AK_U8   ysharp_nr_keepStr;
    AK_U8   ysharp_mfacotr_sup;
    AK_U8   ysharp_freqctrl;
    //AK_U8  ysharp_reduce_factor[36][64];
    AK_U8   ysharp_lumWgt[32];
    AK_U8    ysharpStr[16];
    AK_U8   edgeStr[32];
    AK_U8   edgeFiltStr;
    AK_U8   edgeWhiteGain;
    AK_U8   edgeBlackGain;
    AK_U8   edgeWideWgt;
    AK_U8   edge45DWgt;
    AK_U8   weakEdgeCtrl;
    AK_U8   localShootSupTh;        //[0,31]
    AK_U8   localShootSupSlop;          //[0,15] default
    AK_U8   ovShootSupStr;              //[0, 31]
    AK_U8   udShootSupStr;              //[0, 31]
    AK_S8     UVGain[4];
    AK_S8     UVArea[4][4];
    
    AK_U16   ysharp_nr_calc_k;
    AK_U16   ysharp_nrweight_tbl[17];
    AK_U16  EdgeGainMax; //[0, 511]
    AK_S16   MF_HPF_LUT[256];            //[-256,255]
    AK_S16   HF_HPF_LUT[256];            //[-256,255]
    AK_U16   MF_LUT_KEY[16];
    AK_U16   HF_LUT_KEY[16];
}AK_ISP_SHARP;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_sharp_attr{
    AK_U8    sharp_debug_output;         //[0,3]
    AK_ISP_OP_TYPE  ysharp_mode;
    AK_ISP_SHARP    manual_sharp_attr;
    AK_ISP_SHARP    linkage_sharp_attr[12];
}AK_ISP_SHARP_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_sharp_ex_attr {
    AK_S8  mf_HPF[6];            
                                 //M13,M14,M15,
                                 //M11,M12,M14,
                                 //M10,M11,M13,
    AK_S8  hf_HPF[3];        
                                 //M22 M21,M22,
                                 //M21,M20, M21,
}AK_ISP_SHARP_EX_ATTR;
#pragma pack()

#pragma pack(1)
typedef  struct ak_isp_fcs {
    AK_U8    fcs_enable;         //使能位
    AK_U8    fcs2_enable;
    AK_U8    fcs_th;         //[0, 255]
    AK_U8    fcs_gain_slop;  //[0,63]
    AK_U8    fcs2_c_th;   //[0, 127]
    AK_U8    fcs2_c_str;  //[0, 31]
    AK_U8    fcs2_s_th;   //[0, 127]
    AK_U8    fcs2_s_str;  //[0, 31]
    AK_U8    dm_fcs_purple_str;//[0, 31]
    AK_U8    dm_fcs_cyan_str;  //[0, 31]
    AK_U8    dm_fcs_chrom_k;   //[0, 31]
    AK_U8    dm_fcs_hfp_k;     //[0, 31]
    AK_U8    hue_satSupTh;  //[0, 15]
    AK_U8    hue_satSupStr; //[0, 15]
    AK_U8    pfs_str;       //[0, 63]
    AK_U8    pfs_start_c;   //[0, 63]
    AK_U8    pfs_range;     //[0, 15]
    AK_U8    pfs_highlight_th;      //[0, 255]
    AK_U8    pfs_lowlight_th;       //[0, 255]
}AK_ISP_FCS;
#pragma pack()

#pragma pack(1)
typedef  struct ak_isp_fcs_attr {
    AK_ISP_OP_TYPE  fcs_mode;       //模式选择，手动或者联动
    AK_ISP_FCS  manual_fcs;
    AK_ISP_FCS  linkage_fcs[12];
}AK_ISP_FCS_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_hue {
    AK_U8    hue_sat_en;         //hue使能
    //AK_U8  pfs_str_tbl[16];//[0, 63]
    AK_S8    hue_lut_a[65]; //[-128, 127]
    AK_S8    hue_lut_b[65]; //[-128, 127]
    AK_U8    hue_lut_s[65];  //[0, 255]
}AK_ISP_HUE;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_auto_hue{
    AK_U16     hue_color_temp; 
    AK_ISP_HUE        auto_hue_para;
}AK_ISP_AUTO_HUE;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_auto_hue_attr {
    AK_U16       hue_num;
    AK_ISP_AUTO_HUE     auto_hue_para[10]; 
}AK_ISP_AUTO_HUE_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_hue_attr {
    AK_ISP_OP_TYPE          hue_mode;   //联动或者手动
    AK_ISP_HUE              manual_hue;
    AK_ISP_AUTO_HUE_ATTR    auto_hue;       //四个联动参数
}AK_ISP_HUE_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct  ak_isp_contrast {
    AK_U8   contrast_enable;
    AK_U16  y_contrast; //[0,511]
    AK_S16  y_shift;  //[0, 511]
}AK_ISP_CONTRAST;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_auto_contrast
{
    AK_U8   contrast_enable;
    AK_U8   shift_max;          //[0, 127]
    AK_U16  dark_pixel_area;    //[0, 511]
    AK_U16  dark_pixel_rate;    //[1, 256]  
}AK_ISP_AUTO_CONTRAST;
#pragma pack()

#pragma pack(1)
typedef struct  ak_isp_contrast_ATTR {
    AK_ISP_OP_TYPE  contrast_mode;  //模式选择，手动或者联动
    AK_ISP_CONTRAST manual_contrast;
    AK_ISP_AUTO_CONTRAST    linkage_contrast[12];
}AK_ISP_CONTRAST_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_saturation {
    AK_U8     SE_enable;    // 使能位
    AK_U8     SE_scale1;       //[0,255]
    AK_U8     SE_scale2;       //[0,255]
    AK_U8     SE_scale3;       //[0,255]
    //AK_U8   SE_scale_slop1;  //[0,255]
    //AK_U8   SE_scale_slop2;  //[0,255]
    AK_U16    SE_th1;          //[0, 1023]
    AK_U16    SE_th2;          //[0, 1023]
    AK_U16    SE_th3;          //[0, 1023]
    AK_U16    SE_th4;          //[0, 1023]
}AK_ISP_SATURATION;
#pragma pack()

#pragma pack(1)
typedef struct  ak_isp_saturation_attr {
    AK_ISP_OP_TYPE      SE_mode;         //饱和度模式
    AK_ISP_SATURATION   manual_sat;
    AK_ISP_SATURATION   linkage_sat[12];
}AK_ISP_SATURATION_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_effect_attr {
    AK_U8   yuv_out_enable;
    AK_U8    dark_margin_en;   //黑边使能
    AK_U8    y_a;     // [0, 255]
    AK_S8    y_b;     //[-128, 127]
    AK_S16    uv_a;    //[-256, 255]
    AK_S16    uv_b;    //[-256, 255]
}AK_ISP_EFFECT_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_ddpc {
    //AK_U8  ddpc_enable;        //动态坏点使能位
    AK_U8  white_dpc_enable; //白点消除使能位
    AK_U8  black_dpc_enable; //黑点消除使能位
    AK_U8  ddpc_th_slop;
    AK_U8  ddpc_strength;
    AK_U16 ddpc_th_base;          //10bit
}AK_ISP_DDPC;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_ddpc_attr {
    AK_ISP_OP_TYPE ddpc_mode;            //模式选择，手动或者联动
    AK_ISP_DDPC manual_ddpc;
    AK_ISP_DDPC linkage_ddpc[12];
}AK_ISP_DDPC_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_af_attr {
    AK_U16  af_stat_enable;
    AK_U8 af_spotlight_lum_th;     //[0 ,255]
    AK_U8 af_spotlight_edge_th;   //[0 ,255]
    AK_U8 af_spotlight_edge_gain; //[0, 15]
    AK_U8 af_clip_th;              //[0, 63]
    AK_U8 af_peak_ctrl;            //[0, 255]
    AK_U8 af_rshift;           //[0, 15]
}AK_ISP_AF_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_af_stat_info {
   AK_U32  af_statics[8][16][5];            //统计结果
   AK_U8 af_local_highlight[64][128];
}AK_ISP_AF_STAT_INFO;
#pragma pack()

#pragma pack(1)
typedef  struct  ak_isp_mwb_attr {
    AK_U16   r_gain;
    AK_U16   g_gain;
    AK_U16   b_gain;
    AK_S16    r_offset;
    AK_S16    g_offset;
    AK_S16    b_offset;
}AK_ISP_MWB_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_awb_stat_calib_para{
    AK_U16    gr_low[10];            //gr_low[i]<=gr_high[i]
    AK_U16    gr_high[10];
    AK_U16    gb_low[10];            //gb_low[i]<=gb_high[i]
    AK_U16    gb_high[10];
    AK_U16    rb_low[10];           //rb_low[i]<=rb_high[i]
    AK_U16    rb_high[10];
}AK_ISP_AWB_STAT_CALIB_PARA;
#pragma pack()

#pragma pack(1)
typedef  struct ak_isp_awb_attr {
    
    AK_U8     g_weight[16];
    AK_U8     y_low;                 //y_low<=y_high
    AK_U8     y_high;
    AK_U8     err_est;
    AK_U8     awb_iso_track_enable;
    AK_U8     colortemp_envi[10];
    AK_ISP_AWB_STAT_CALIB_PARA awb_stat_calib_para[12];
	AK_U8	  merge_benchmarks;
	AK_U16	  merge_weight[9];
    //awb软件部分需要设置的参数
    AK_U16    auto_wb_step;                 //白平衡步长计算
    AK_U16    total_cnt_thresh;            //像素个数阈值
    AK_U16    colortemp_stable_cnt_thresh; //稳定帧数，多少帧一样认为环境色温改变
    
    AK_ISP_AWB_ALGO_TYPE alg_type;
}AK_ISP_AWB_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_wb_attr {
    AK_U8        awb_run_interval;
    AK_U8       wb_enable;
    AK_U8       wb_statis_enable;
    AK_U8        wb_fine_linkage_strength[13];
    AK_U16       wb_fine_correct_enable;
    AK_ISP_AWB_STATUS   wb_status;
    AK_ISP_OP_TYPE      wb_type;
    AK_ISP_MWB_ATTR     manual_wb_para;
    AK_ISP_AWB_ATTR     auto_wb_para;
}AK_ISP_WB_ATTR;
#pragma pack()

#pragma pack(1)
typedef  struct ak_isp_awb_calib_node_attr {
    AK_U16    cali_rg;
    AK_U16    cali_bg;
    AK_U16    cali_color_temp;
}AK_ISP_AWB_CALIB_NODE_ATTR;
#pragma pack()

#pragma pack(1)
typedef  struct  ak_isp_awb_calib_attr {
    AK_U8           awb_calib_node_num;
    AK_ISP_AWB_CALIB_NODE_ATTR  awb_calib_para[10];
}AK_ISP_AWB_CALIB_INFO;
#pragma pack()

#pragma pack(1)
typedef  struct  ak_isp_awb_ration {
    AK_U32  ratio[10];
}AK_ISP_AWB_RATIO;
#pragma pack()

#pragma pack(1)
typedef  struct  ak_isp_awb_ex_attr {
    AK_U8        awb_ex_ctrl_enable;
    AK_ISP_AWB_RATIO    prefer_rratio[12];
    AK_ISP_AWB_RATIO    prefer_bratio[12];
}AK_ISP_AWB_EX_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_awb_blk_stat_info{
    AK_U32 wb_Ravg_stat[16][16];
    AK_U32 wb_Gavg_stat[16][16];
    AK_U32 wb_Bavg_stat[16][16];
}AK_ISP_AWB_BLK_AVG_STAT_INFO;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_awb_wp_blk_stat_info{
    AK_U32 wp_r_stat[16][16];
    AK_U32 wp_g_stat[16][16];
    AK_U32 wp_b_stat[16][16];
    AK_U32 wp_cnt_statics[16][16];
}AK_ISP_AWB_BLK_WP_STAT_INFO;
#pragma pack()

#pragma pack(1)
typedef  struct ak_isp_awb_stat_info {
    //白平衡统计结果
    AK_U32   total_R[10];
    AK_U32   total_G[10];
    AK_U32   total_B[10];
    AK_U32   total_cnt[10];
    //经由自动白平衡算法算出的白平衡增益值
    AK_ISP_AWB_BLK_AVG_STAT_INFO    wb_blk_avg_stat;
    AK_ISP_AWB_BLK_WP_STAT_INFO     wb_blk_wp_stat;
    #if 0
    AK_U16    r_gain;
    AK_U16    g_gain;
    AK_U16    b_gain;
    AK_S16   r_offset;
    AK_S16   g_offset;
    AK_S16   b_offset;
    AK_U16    current_colortemp_index;     //环境色温标记，是参数随环境变化的色温指标。
    AK_U16    colortemp_stable_cnt[10];         //每一种色温稳定的帧数计数
    AK_U16 current_colortemp;  
    AK_ISP_CCM     current_ccm; 
    #endif  
}AK_ISP_AWB_STAT_INFO;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_awb_run_info{
    AK_U16    r_gain;
    AK_U16    g_gain;
    AK_U16    b_gain;
    AK_S16    r_offset;
    AK_S16    g_offset;
    AK_S16    b_offset;
    AK_U8     is_stable;
    AK_U8     current_colortemp_index;     //环境色温标记，是参数随环境变化的色温指标。
    AK_U16    colortemp_stable_cnt[10];         //每一种色温稳定的帧数计数
    AK_U16   current_colortemp;  
    AK_ISP_CCM       current_ccm;
}AK_ISP_AWB_RUN_INFO;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_ex_zone_weight_attr {
    AK_U8    win_weight[8][16];
    AK_U8    ex_zone_weight_str;
}AK_ISP_EX_ZONE_WEIGHT_ATTR;
#pragma pack()

///////////////////AUTO Exposure//////////////////////
#pragma pack(1)
typedef enum ak_isp_ae_status{
    AE_STATUS_PAUSE = 0,
    AE_STATUS_NORMAL,
}AK_ISP_AE_STATUS;
#pragma pack()

#pragma pack(1)
typedef enum ak_isp_ae_strgy_model {
    AE_EXP_HIGHLIGHT_PRIOR = 0,
    AE_EXP_LOWLIGHT_PRIOR,
    AE_STRGY_MODEL_BUFF,
}AK_ISP_AE_STRGY_MODEL;
#pragma pack() 

#pragma pack(1)
typedef struct ak_isp_me_attr {
    AK_U32   exp_time;
    AK_U32   a_gain;
    AK_U32   d_gain;
    AK_U32   isp_d_gain;
}AK_ISP_ME_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_ae_ex_convergence_attr {
    AK_U8    ae_ex_convergence_en;
    AK_U16   control_zone1;
    AK_U16   control_zone2;  
    AK_U16   control_zone3;
    AK_U16   control_zone4;      
    AK_U16   control_step1; 
    AK_U16   control_step2; 
    AK_U16   control_step3; 
    AK_U16   control_step4; 
    AK_U16   control_step_max; 
}AK_ISP_AE_EX_CONVER_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_ae_strgy_attr {
    AK_U32   hist_weight[16];
}AK_ISP_AE_STRGY_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_ae_route_node_attr {
    AK_U32   exp_time;                   
    AK_U32   again;              
    AK_U32   dgain; 
    AK_U32   isp_dgain; 
}AK_ISP_AE_ROUTE_NODE_ATTR; 
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_ae_route_attr{
    AK_U8   ae_route_en;
    AK_U8   ae_route_node_num;
   AK_ISP_AE_ROUTE_NODE_ATTR  ae_route_node[10];
}AK_ISP_AE_ROUTE_ATTR;
#pragma pack()

#pragma pack(1)
typedef enum ak_isp_antiflicker_model
{
	ANTIFLICKER_NORMAL_MODEL = 0,
	ANTIFLICKER_AUTO_MODEL = 1,
	ANTIFLICKER_AUTO_BUTT
}AK_ISP_ANTIFLICKER_MODEL;
#pragma pack()

#pragma pack(1)
typedef enum ak_isp_antiflicker_frequency
{
	FLICKER_FREQUENCE_50HZ = 0,
	FLICKER_FREQUENCE_60HZ = 1,
}AK_ISP_ANTIFLICKER_FREQUENCY;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_antiflicker_attr
{
	AK_U8 anti_flicker_en;
	AK_ISP_ANTIFLICKER_FREQUENCY frequency;
	AK_ISP_ANTIFLICKER_MODEL anti_flicker_model;
}AK_ISP_ANTIFLICKER_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_subantiflicker_attr
{
	AK_U8  anti_subflicker_en;
	AK_U8  anti_flicker_target_lumi;
    AK_U8  anti_flicker_start_exp;//[1,100]
}AK_ISP_SUBANTIFLICKER_ATTR;
#pragma pack()

#pragma pack(1)
typedef enum ak_isp_merge_model
{
	COND_INDEPENDENT_MODEL = 0,
	NORMAL_WEIGHTED_MODEL = 1,
	MERGE_MODEL_BUTT
}AK_ISP_MERGE_MODEL;
#pragma pack()


#pragma pack(1)
typedef struct ak_isp_ae_attr {
    AK_U32   exp_time_max;           //曝光时间的最大值
    AK_U32   a_gain_max;         //模拟增益的最大值
    AK_U32   d_gain_max;             //数字增益的最大值
    AK_U32   isp_d_gain_max;         //isp数字增益的最大值
    AK_U16 exp_time_min;         //曝光时间的最小值
    AK_U16 a_gain_min;               //模拟增益的最小值
    AK_U16 d_gain_min;           //数字增益的最小值
    AK_U16 isp_d_gain_min;           //isp数字增益的最小
    AK_U16 blacklight_rate_max;      
    AK_U16 blacklight_rate_min; 
	AK_U32	linesper500ms;
    AK_U8  exp_stable_range;     //稳定范围
    AK_U8  exp_hold_range;
    AK_U8  exp_speed;
    
    AK_U8  exp_lumi_filter_para; 
    AK_U8  target_lumiance[12];      //目标亮度
   // AK_U32  hist_weight[16];
    AK_U8  blacklight_compensation_en;
    AK_U8  blacklight_detect_scope; 
	AK_ISP_MERGE_MODEL  merge_model;
	AK_U8  merge_weight;
	AK_S8  merge_lumi_diff[12];
    //AK_ISP_WIN_WEIGHT_TYPE  ae_win_weight_type;
    //AK_ISP_WEIGHT_ATTR    win_weight[4];
    AK_ISP_RAW_G_HIST_ATTR  raw_g_hist_attr;
    AK_ISP_YUV_HIST_ATTR    yuv_hist_attr;
    AK_U16 envi_gain_range[12][2];
    AK_ISP_AE_EX_CONVER_ATTR  ae_ex_convergence_para;
    AK_ISP_AE_STRGY_MODEL     ae_strgy_type;
    AK_ISP_AE_STRGY_ATTR      ae_strgy_para[3]; 
    AK_ISP_AE_ROUTE_ATTR      exp_route_para;
	AK_ISP_ANTIFLICKER_ATTR	  ae_antiflicker_para;
	AK_ISP_SUBANTIFLICKER_ATTR ae_subantiflickerpara;
}AK_ISP_AE_ATTR;
#pragma pack()

#pragma pack(1)
typedef  struct  ak_isp_exposure_attr {
    AK_ISP_AE_STATUS  exp_status;
    AK_U8      ae_run_interval;
    AK_ISP_OP_TYPE    exp_type;
    AK_ISP_ME_ATTR    manual_exp_para;
    AK_ISP_AE_ATTR    auto_exp_para;
}AK_ISP_EXPOSURE_ATTR;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_ae_run_info {
    AK_U8    current_calc_avg_lumi;              //现在的计算出的亮度值
    AK_U8    current_calc_avg_compensation_lumi;     //经过曝光补偿后的亮度值

    AK_U8    current_target_lumi;                 
    AK_U8    is_stable;        

    AK_U32  current_a_gain;                      //模拟增益的值
    AK_U32  current_d_gain;                      //数字增益的值
    AK_U32  current_isp_d_gain;                  //isp数字增益的值
    AK_U32  current_exp_time;                    //曝光时间的值

    AK_U32  current_a_gain_step;             //现在的模拟增益的步长
    AK_U32  current_d_gain_step;             //数字增益的步长
    AK_U32  current_isp_d_gain_step;             //isp数字增益的步长
    AK_U32  current_exp_time_step; 
    //AK_ISP_AE_ROUTE_ATTR      ae_route_para;
}AK_ISP_AE_RUN_INFO;
#pragma pack()

#pragma pack(1)
typedef struct ak_isp_raw_hist_stat_info {
    AK_U32   raw_g_hist[256];
    AK_U64   raw_g_total;
}AK_ISP_RAW_HIST_STAT_INFO;
#pragma pack()

typedef struct ak_isp_rgb_hist_stat_info {
    AK_U32   rgb_hist[256];
    AK_U64   rgb_total;
}AK_ISP_RGB_HIST_STAT_INFO;

#pragma pack(1)
typedef struct  ak_isp_yuv_hist_stat_info {
    AK_U32   y_hist[256];
    AK_U64   y_total;
}AK_ISP_YUV_HIST_STAT_INFO;
#pragma pack()

typedef struct ak_isp_wdr_hist_stat_info{
    AK_U32   wdr_hist_short[256];
    AK_U32   wdr_hist_long[256];
}AK_ISP_WDR_HIST_STAT_INFO;

#pragma pack(1)
typedef struct ak_ae_stat_info{
    AK_ISP_RAW_HIST_STAT_INFO   raw_hist_stat_para;
    AK_ISP_YUV_HIST_STAT_INFO   yuv_hist_stat_para;
}AK_ISP_AE_STAT_INFO;
#pragma pack()
//////////////////////////end IQT //////////////////////////////////////////////////

/**blk**/
typedef struct ak_isp_raw_g_hist_blk_attr{
    AK_U16 rawstat_blkW;
    AK_U16 rawstat_blkH;
    AK_U8  rawstat_blkW_cnt;
    AK_U8  rawstat_blkH_cnt;
}AK_ISP_RAW_G_HIST_BLK_ATTR;

typedef struct ak_isp_mesh_lsc_blk_attr{
    AK_U8   mesh_lsc_blkW;                // [0, 255]
    AK_U8   mesh_lsc_blkH;                // [0, 255]
    AK_U8   mesh_lsc_revW;                // [0, 255]
    AK_U8   mesh_lsc_revH;                // [0, 255]
    AK_U8   mesh_lsc_blkW_cnt;
    AK_U8   mesh_lsc_blkH_cnt;
}AK_ISP_MESH_LSC_BLK_ATTR;

typedef struct ak_isp_drc_blk_attr{
    AK_U8   drc_blkW_cnt;
    AK_U8   drc_blkH_cnt;
    AK_U8   drc_reverseH_shift;     //[0,15]
    AK_U8   drc_reverseW_shift;     //[0,15]
    AK_U16  drc_reverseH_g;         //[0,511]
    AK_U16  drc_reverseW_g;         //[0,511]
    AK_U16  drc_blkW;               //[0,512]
    AK_U16  drc_blkH;               //[0,512]
    AK_U16  drc_blkcnt_rev;         //[0, 1023]
}AK_ISP_DRC_BLK_ATTR;

typedef  struct ak_isp_awb_blk_attr {
    AK_U32  wb_blkW;
    AK_U32  wb_blkH;
    AK_U8 wb_blkW_cnt;
    AK_U8 wb_blkH_cnt;
}AK_ISP_AWB_BLK_ATTR;

typedef struct  ak_isp_awb_fine_correct_blk_attr{
    AK_U8 wbfc_blkW;
    AK_U8 wbfc_blkH;
    AK_U8 wbfc_revW;
    AK_U8 wbfc_revH;
    AK_U8 wbfc_blkW_cnt;
    AK_U8 wbfc_blkH_cnt;
}AK_ISP_AWB_FINE_CORRECTION_BLK_ATTR;

typedef struct ak_isp_rgb_hist_blk_attr{
    AK_U16 rgbstat_blkW;
    AK_U16 rgbstat_blkH;
    AK_U8  rgbstat_blkW_cnt;
    AK_U8  rgbstat_blkH_cnt;
}AK_ISP_RGB_HIST_BLK_ATTR;

typedef struct ak_isp_yuv_hist_blk_attr{
    AK_U16 yuvstat_blkW;
    AK_U16 yuvstat_blkH;
    AK_U8  yuvstat_blkW_cnt;
    AK_U8  yuvstat_blkH_cnt;
}AK_ISP_YUV_HIST_BLK_ATTR;

typedef struct ak_isp_hdr_blk_attr {
     AK_U16     hdr_blkW;
     AK_U16     hdr_blkH;
     AK_U16     hdr_reverseW_g;       //[0,511]
     AK_U16     hdr_reverseH_g;       //[0,511]
     AK_U16     hdr_weight_g;         //[0,511]
     
     AK_U8      hdr_blkW_cnt;
     AK_U8      hdr_blkH_cnt;
     AK_U8      hdr_reverseW_shift;   //[0,15]
     AK_U8      hdr_reverseH_shift;   //[0,15]
     AK_U8      hdr_weight_shift;     //[0,15]
     AK_U8      hdr_weight_k;         //[0,15]
}AK_ISP_HDR_BLK_ATTR;

typedef struct ak_isp_lce_blk_attr
{
    AK_U16    lce_blkW;     
    AK_U16    lce_blkH;
    AK_U8     lce_reverseW_g;
    AK_U8     lce_reverseH_g; 
    AK_U8     lce_blkW_cnt;     
    AK_U8     lce_blkH_cnt;
    AK_U8     lce_reverseW_shift;       
    AK_U8     lce_reverseH_shift; 
}AK_ISP_LCE_BLK_ATTR;

typedef struct ak_isp_3d_nr_blk_attr {
    AK_U8 _3d_nr_blkW;
    AK_U8 _3d_nr_blkH;  
    AK_U8 _3d_nr_last_blkW;
    AK_U8 _3d_nr_last_blkH; 
    AK_U8 _3d_nr_blkW_cnt; //[0,63]
    AK_U8 _3d_nr_blkH_cnt; //[0,63]
    AK_U16 _3d_nr_revW; //[0,511]
    AK_U16 _3d_nr_revH; //[0,511]
}AK_ISP_3D_NR_BLK_ATTR;

typedef struct ak_isp_sharp_blk_attr {
    AK_U8  ysharp_blkW;
    AK_U8  ysharp_blkH;
    AK_U8  ysharp_blkW_cnt;
    AK_U8  ysharp_blkH_cnt;
    AK_U16 ysharp_revW;
    AK_U16 ysharp_revH;
}AK_ISP_SHARP_BLK_ATTR;

typedef struct ak_isp_hue_high_blk_attr{
    AK_U8  hue_yuvhightlumi_blkW_cnt;
    AK_U8  hue_yuvhightlumi_blkH_cnt;
    AK_U16 hue_yuvhightlumi_blkW;
    AK_U16 hue_yuvhightlumi_blkH;
}AK_ISP_HUE_HIGH_BLK_ATTR;

typedef struct ak_isp_frame_rate_attr {
    AK_U16  hight_light_frame_rate ;
    AK_U16  hight_light_max_exp_time ;
    AK_U16  hight_light_to_low_light_gain;
    AK_U16  mid_light_frame_rate;
    AK_U16  mid_light_max_exp_time;
    AK_U16  mid_light_to_low_light_gain;
    AK_U16  low_light_frame_rate;
    AK_U16  low_light_max_exp_time;
    AK_U16  low_light_to_hight_light_gain;
}AK_ISP_FRAME_RATE_ATTR;

#endif
