#ifndef _AK_ISP_DRV_H_
#define _AK_ISP_DRV_H_
//#define ISP_DRV_LIB_VER     "isp_drv_lib-V0.3.00"

#define DISABLE 0
#define ENABLE  1
#define BUFFER_NUM 4    /*每个输出通道最大buffer数*/
#define MASK_36BIT 0xfffffffff
#define MASK_32BIT 0xffffffff
#define MASK_28BIT 0xfffffff
#define MASK_24BIT 0xffffff
#define MASK_20BIT 0xfffff
#define MASK_19BIT 0x7ffff
#define MASK_18BIT 0x3ffff
#define MASK_16BIT 0xffff
#define MASK_15BIT 0x7fff
#define MASK_14BIT 0x3fff
#define MASK_12BIT 0xfff
#define MASK_11BIT 0x1ff
#define MASK_10BIT 0x3ff
#define MASK_9BIT 0x1ff
#define MASK_8BIT  0xff
#define MASK_7BIT  0x7f
#define MASK_5BIT  0x1f
#define MASK_3BIT  0x7

#define ENV_NUMBER 12
typedef unsigned long T_U64;
typedef unsigned int T_U32;


#if 0
enum buffer_id {
    BUFFER_ONE,
    BUFFER_TWO,
    BUFFER_THREE,
    BUFFER_FOUR,    
};

enum chn_output_format {
    /*YUV422*/
    CHN_OUTPUT_FMT_IYUV = 0,    /*y..u..v..*/
    CHN_OUTPUT_FMT_YV12,        /*y..v..u..*/
    CHN_OUTPUT_FMT_NV12,        /*y..uv..*/
    CHN_OUTPUT_FMT_NV21,        /*y..vu..*/

    /*YUV422*/
    CHN_OUTPUT_FMT_YV16,        /*y..v..u..*/
    CHN_OUTPUT_FMT_NV16,        /*y..uv..*/
    CHN_OUTPUT_FMT_NV61,        /*y..vu..*/

    /*YUV444*/
    CHN_OUTPUT_FMT_YV24,        /*y..v..u..*/
    CHN_OUTPUT_FMT_NV24,        /*y..uv..*/
    CHN_OUTPUT_FMT_NV42,        /*y..vu..*/
    CHN_OUTPUT_FMT_YVU444,      /*y..v..u..*/
    CHN_OUTPUT_FMT_YUV444,      /*y..u..v..*/
    CHN_OUTPUT_FMT_VYU444,      /*v..y..u..*/

    /*RGB888*/
    CHN_OUTPUT_FMT_RGB888,      /*r..g..b..*/
    CHN_OUTPUT_FMT_BGR888_PACKED,       /*bgr..*/
    CHN_OUTPUT_FMT_RGB888_PACKED,       /*rgb..*/
    CHN_OUTPUT_FMT_RGB888_LINE,         /*r0-0..r0-n
                                          g0-0..g0-n
                                          b0-0..b0-n
                                          r1-0..r1-n
                                          g1-0..g1-n
                                          b1-0..b1-n
                                          ......
                                          */

    CHN_OUTPUT_FMT_DATASTREAM,

    CHN_OUTPUT_FMT_NUM,
};


enum data_format {
    BAYER_RAW_DATA = 0,
    BAYER_RAW_PWL,
    BAYER_RAW_2FWDR,
    YUV444_DATA,
    YUV422_DATA,
    YUV420_DATA,
    JPEG_COMPRESS_DATA,
};

typedef enum ak_isp_op_type{
    OP_TYPE_MANUAL = 0,
    OP_TYPE_AUTO = 1,
    OP_TYPE_BUTT,
}AK_ISP_OP_TYPE;

typedef enum ak_isp_rgb2yuv_model {
    BT601_MODEL = 0,
    BT709_MODEL = 1,
    BT2020_MODEL = 2,
}AK_ISP_RGB2YUV_MODEL;  

enum yuv422_interface {
    YUV422_ITF_BT601 = 0,
    YUV422_ITF_BT656,
    YUV422_ITF_BT1120,
};  /*bt601/656/1120*/

enum yuv422_bt656_1120_frame_start {
    YUV422_BT656_1120_FS_FALLING = 0,/*下降沿*/
    YUV422_BT656_1120_FS_RISING,    /*上升沿*/
    YUV422_BT656_1120_FS_DEDGE,     /*双边沿*/
};  /*下降沿、上升沿、边沿(隔行时)*/


enum raw_seq {
    RAW_SEQ_R = 0,
    RAW_SEQ_B,
    RAW_SEQ_Gr,
    RAW_SEQ_Gb,
};

typedef struct profile_ctrl_attr{
    unsigned char profile_mode;
    unsigned char profile_halt;
    unsigned char profile_load;
    unsigned char profile_select_ID;
    unsigned char profile_state;
    unsigned char profile_locked_ID;
}AK_ISP_PROFILE_ATTR;

typedef struct dma_ctrl_attr{
    unsigned char axi_awb_outstanding;
    unsigned char axi_arr_outstanding;
    unsigned char dma_buffer_safe_level_upload;
    unsigned char dma_buffer_safe_level_download;
    unsigned char axiqos;
    unsigned char axiboundary;
    unsigned char axiqos_ref_frame;
    unsigned char axiboundary_ref_frame;
}AK_ISP_DMA_CTRL_ATTR;


#if 0
struct dvp_attr {
    signed char vsync_pol;  /*0-low active,  1-high active*/
    signed char href_pol;   /*0-low active,  1-high active*/
    //isp_upload_stream_sel_cfg，配置为dvp或mipi_attr后自动选定一种作为 data stream的数据源头。
    int one_line_cycle;     /*16bits*/
    signed char frame_start_edge;   /*0-vsync falling is frame starting, 1-vsync rising is frame starting*/
    signed char frame_start_delay_enable;   /*0-disable, 1-enable*/
    int hblank_cycle;       /*16bits*/
    signed char frame_start_delay_2line_num;    /*6bits*/
    signed char hsync_adjust_enable;    /*0-disable, 1-enable*/
    signed char vsync_filter_enable;    /*0-disable, 1-enable*/

    enum yuv422_interface itf;  /*bt601/656/1120*/
    enum yuv422_bt656_1120_frame_start fstart;  /*下降沿、上升沿、边沿(隔行时)*/
    signed char yuv422_bt656_1120_capture_odd_frame_enable; /*0-disable, 1-enable*/
    signed char yuv422_bt656_1120_capture_even_frame_enable;    /*0-disable, 1-enable*/
    signed char yuv422_sdr_ddr_sel; /*0-sdr单沿，1-ddr双沿*/
    signed char yuv422_error_correct_enable;    /*0-disable, 1-enalbe*/ /*纠正F/V/H Code*/
    signed char yuv422_ddr_data_start;  /*0-posedge, 1-negedge*/ /*双沿时起始位置为上升沿还是下降沿*/
};

struct mipi_attr {
    signed char packed_or_pixel_mode;   /*0-packed, 1-pixel*/
    //isp_upload_stream_sel_cfg，配置为dvp或mipi_attr后自动选定一种作为 data stream的数据源头。
    signed char virtual_channel_id;     /*4bits*/
    signed char bypass_virtual_channel_enable;  /*0-disable, 1-enable*/
    signed char hsync_start_sel;    /*0-CSI Controller输入的hsync_start，1-CSI Controller输入的信号生成hsync_start*/
    signed char hsync_end_sel;      /*0-CSI Controller输入的hsync_end，1-CSI Controller输入的信号生成hsync_end*/
    signed char decoder_di_enable;  /*0-ISP配置的数据格式和位宽参数，1-CSI2控制器传递的Data Type进行解码*/
};

struct dvp_mipi_common_attr {
    signed char data_width; /*8/10/12bits*/
    enum src_data_format    data_format;
    enum raw_seq        raw_seq;    /*only raw validable*/
    signed char go_upload_enable;   /*0-donot upload, 1-upload*/
    signed char go_yuv_enable;      /*0-donot go yuv, 1-go yuv*/ /* only for input yuv */
    signed char video_mode; /*0-single mode, 1-continue mode*/
    signed char dual_sensor_sel;    /*0-sensor0, 1-sensor1. only dual mode is validable*/
    /*
     * 有几个在top的寄存器：
     * csi_mipi_dvp_sel_cfg: dvp还是mipi接口
     * isp_pp_dual_sensor_mode_cfg: 单目还是双目
     * cis_pclk_pol: dvp时pclk极性
     */
    signed char test_pattern;           /*0~8*/
    signed char test_pattern_enable;    /*0-disable, 1-enable*/
    signed char uv_switch_enable;       /*0-default, 1-switch*/
    signed char uv_sub128_enable;       /*0-default, 1-sub128*/
    signed char line_end_cnt_enable;    /*0-disable, 1-enable*/
    signed char line_end_cnt;           /*6bits*/
};
#endif

struct input_data_format {
    enum data_format df;
    int data_width;
};
typedef struct ak_isp_common_attr
{   
    unsigned short res_width_cfg;
    unsigned short res_height_cfg;
    unsigned short res_width0_cfg;
    unsigned short res_height0_cfg;
    unsigned short raw_bayer_align;
    unsigned char  raw_pixel_bits;
    unsigned char  yuv_formula;
    unsigned char  yuv_swing;
    unsigned char  test_pattern_mode;
}AK_ISP_COMMON_ATTR;

typedef struct ak_isp_vtc_attr
{
    unsigned short vtc_pixel_rate_max;
    unsigned short vtc_hbp_min;
    unsigned short vtc_hfp_min;
    unsigned short vtc_hblank_min;
    unsigned short vtc_hpitch_min;
    unsigned short vtc_vbp_cyc_min;
    unsigned short vtc_vfp_cyc_min;
    unsigned short vtc_vbp_us_min;
    unsigned short vtc_vfp_us_min;
    unsigned short vtc_vpitch_us_min;
    unsigned short vtc_phase;
}AK_ISP_VTC_ATTR;

typedef struct ak_isp_pwl{
    unsigned short piece_th[4]; //[0, 4095]
    unsigned short piece_a[5];  //[0, 4095]
    unsigned char  piece_b[5];  //[0, 7]
}AK_ISP_PWL_ATTR;

typedef struct ak_isp_rawwdrcmb{
    unsigned char   wdr_enable;
    unsigned char   wdr_debug_mode;
    unsigned char   wdr_pwl_long;           // 1 bit
    unsigned char   wdr_pwl_short;          // 1 bit
    unsigned char   wdr_long_bits;          // [10, 16]
    unsigned char   wdr_short_bits;         // [10, 16]
    unsigned char   wdr_mix_th;             //[0, 255]
    unsigned char   wdr_mix_slop;           //6bits
    unsigned char   wdr_short_nr_th;        //[0, 255]
    unsigned char   wdr_short_nr_str;       //[0, 15]
    unsigned char   wdr_md_long_wgt;        //[0, 255]
    unsigned char   wdr_md_th;              //[0, 255]
    unsigned char   wdr_md_slop;            //[0, 255]
    unsigned char   wdr_md_spr_th;          //[0, 255]
    unsigned char   wdr_md_spr_slop;        //[0, 15]
    unsigned char   wdr_forcelong_th;       //[0, 255]
    unsigned char   wdr_forcelong_slop;     //[0, 255]
    unsigned char   wdr_pre_gain;           //[0, 255]
    unsigned short  wdr_hist_enable;
    //unsigned short  wdr_gain_L;               //12bits 
    signed short    wdr_r_offset;           //[-2048, 2047] 
    signed short    wdr_g_offset;           //[-2048, 2047]
    signed short    wdr_b_offset;           //[-2048, 2047]
}AK_ISP_RAWWDRCMB_ATTR;

typedef struct ak_isp_rawwdrcomb_hist_stat_info{
    unsigned int    wdr_hist_short[256];
    unsigned int    wdr_hist_long[256];
}AK_ISP_RAWWDRCOMB_HIST_STAT_INFO;

typedef struct ak_isp_blc {
    unsigned short   black_level_enable;    //使能位
    unsigned short   bl_r_a;            //[ 0,1023]
    unsigned short   bl_gr_a;           //[ 0,1023]
    unsigned short   bl_gb_a;           //[ 0,1023]
    unsigned short   bl_b_a;            //[ 0,1023]
    signed short     bl_r_offset;       //[-2048,2047]
    signed short     bl_gr_offset;      //[-2048,2047]
    signed short     bl_gb_offset;      //[-2048,2047]
    signed short     bl_b_offset;       //[-2048,2047]
}AK_ISP_BLC;

typedef struct ak_isp_blc_attr {
    AK_ISP_OP_TYPE  blc_mode;              //0联动模式，1手动模式
    AK_ISP_BLC      manual_blc;
    AK_ISP_BLC      linkage_blc[12];
}AK_ISP_BLC_ATTR;

typedef struct {
    unsigned short coef_b[10];    //[0,255]
    unsigned short coef_c[10];    //[0,1023]
}lens_coef;

typedef struct ak_isp_lsc_attr {
    unsigned short       lsc_enable;
    unsigned char        lsc_mode;
    unsigned char        linkage_strength[13];   //[0，1023]
    //the reference point of lens correction
    unsigned short       xref;        //[0,4096]
    unsigned short       yref;        //[0,4096]
    unsigned short       lsc_shift;   //[0，15]
    lens_coef   lsc_r_coef;
    lens_coef   lsc_gr_coef;
    lens_coef   lsc_gb_coef;
    lens_coef   lsc_b_coef;
    //the range of ten segment
    unsigned short       range[10];   //[0，1023]
}AK_ISP_LSC_ATTR;

typedef struct ak_isp_mesh_lsc_attr{
    unsigned short   mesh_lsc_enable;
    unsigned char    mesh_lsc_mode;
    unsigned char    linkage_strength[13];   
    unsigned short   mesh_color_temp[3];
    unsigned short   gain_tbl_R[3][25][33]; //[0, 4095]
    unsigned short   gain_tbl_G[3][25][33];
    unsigned short   gain_tbl_B[3][25][33];
}AK_ISP_MESH_LSC_ATTR;


typedef struct ak_isp_mesh_lsc_blk_attr{
    unsigned char   mesh_lsc_blkW;                // [0, 255]
    unsigned char   mesh_lsc_blkH;                // [0, 255]
    unsigned char   mesh_lsc_revW;                // [0, 255]
    unsigned char   mesh_lsc_revH;                // [0, 255]
    unsigned char   mesh_lsc_blkW_cnt;
    unsigned char   mesh_lsc_blkH_cnt;
}AK_ISP_MESH_LSC_BLK_ATTR;


typedef struct ak_isp_raw_g_hist_attr{
    unsigned char  raw_hist_enable;
    unsigned char  raw_rshift;  //[0, 15]
    unsigned char  zone_weight[8][16];
}AK_ISP_RAW_G_HIST_ATTR;

typedef struct ak_isp_raw_g_hist_blk_attr{
    unsigned short rawstat_blkW;
    unsigned short rawstat_blkH;
    unsigned char  rawstat_blkW_cnt;
    unsigned char  rawstat_blkH_cnt;
}AK_ISP_RAW_G_HIST_BLK_ATTR;

typedef struct ak_isp_drc{
    unsigned char   drc_enable;
    unsigned char   drc_rshift;             //data right-shift when drc disable [0, 7]
    unsigned char   drc_pre_shift;          //data left-shift before drc[0, 2]
    unsigned char   drc_pwl_compress;       //[0, 1]
    unsigned char   drc_lum_wgt_tbl[8];     //[0, 255]      
    unsigned char   drc_weight_k;           //[0,15]
    unsigned char   drc_he_distributor[6];  //[0, 63]
    unsigned char   drc_spatialfilter_k;    //[0,5]


    unsigned short  drc_clip_th;            //[0, 65535]
    unsigned short  drc_str;            //[0,1023]
    unsigned short  drc_str_low;        //[0,1023]
    unsigned short  drc_global_lut[8];      //[0, 65535]
    unsigned short  drc_global_lut_key[8];      //[0, 65535]
    //unsigned short  drc_lut[8][16][6];        //[0, 1023]
    //unsigned long     drc_areastat[8][16][7]; //18bits
    //unsigned short  drc_arealumi[8][16];      
}AK_ISP_DRC;

typedef struct ak_isp_drc_attr{
    AK_ISP_OP_TYPE  drc_mode;             //nr1 模式，自动或者联动模式
    AK_ISP_DRC      manual_drc;
    AK_ISP_DRC      linkage_drc[12];       //联动参??
}AK_ISP_DRC_ATTR;

typedef struct ak_isp_drc_blk_attr{
    unsigned char   drc_blkW_cnt;
    unsigned char   drc_blkH_cnt;
    unsigned char   drc_reverseH_shift;     //[0,15]
    unsigned char   drc_reverseW_shift;     //[0,15]
    unsigned short  drc_reverseH_g;         //[0,511]
    unsigned short  drc_reverseW_g;         //[0,511]
    unsigned short  drc_blkW;               //[0,512]
    unsigned short  drc_blkH;               //[0,512]
    unsigned short  drc_blkcnt_rev;         //[0, 1023]
}AK_ISP_DRC_BLK_ATTR;

typedef struct ak_isp_raw_lut_attr {
    unsigned short   raw_gamma_enable;
    unsigned short   r_key[16];
    unsigned short   g_key[16];
    unsigned short   b_key[16];
    unsigned short   raw_r[129];      //10bit
    unsigned short   raw_g[129];      //10bit
    unsigned short   raw_b[129];      //10bit
}AK_ISP_RAW_LUT_ATTR;

typedef struct ak_isp_rgb_hist_attr{
    unsigned char  rgb_hist_enable;
    unsigned char  zone_weight[8][16];
}AK_ISP_RGB_HIST_ATTR;

typedef struct ak_isp_rgb_hist_blk_attr{
    unsigned short rgbstat_blkW;
    unsigned short rgbstat_blkH;
    unsigned char  rgbstat_blkW_cnt;
    unsigned char  rgbstat_blkH_cnt;
}AK_ISP_RGB_HIST_BLK_ATTR;

typedef struct ak_isp_rgb_gamma {
    unsigned short   rgb_gamma_enable;
    unsigned short   r_gamma[129];   //10bit
    unsigned short   g_gamma[129];   //10bit
    unsigned short   b_gamma [129];  //10bit
    unsigned short   r_key[16];
    unsigned short   g_key[16];
    unsigned short   b_key[16];
}AK_ISP_RGB_GAMMA;

typedef struct ak_isp_rgb_gamma_attr {
    unsigned short    gain_th1;
    unsigned short    gain_th2;
    AK_ISP_RGB_GAMMA  linkage_rgb_gamma[3];
}AK_ISP_RGB_GAMMA_ATTR;


typedef struct ak_isp_yuv_hist_attr
{
    unsigned short  yuv_hist_enable;
    unsigned char  zone_weight[8][16];
}AK_ISP_YUV_HIST_ATTR;

typedef struct ak_isp_yuv_hist_blk_attr{
    unsigned short yuvstat_blkW;
    unsigned short yuvstat_blkH;
    unsigned char  yuvstat_blkW_cnt;
    unsigned char  yuvstat_blkH_cnt;
}AK_ISP_YUV_HIST_BLK_ATTR;


typedef struct ak_isp_y_gamma_attr {
    unsigned short    ygamma_enable;
    unsigned char     ygamma_uv_adjust_enable;
    unsigned char     ygamma_uv_adjust_level;
    unsigned short    ygamma[129];    //10bit
    unsigned short    ygamma_key[16]; //曲线的关键点
    unsigned short    ygamma_cnoise_yth1;   //Ygamma色差抑制门限值
    unsigned short    ygamma_cnoise_yth2;   //Ygamma色差抑制门限值
    //unsigned short      ygamma_cnoise_slop;
    unsigned short    ygamma_cnoise_gain ;  //UV调整系数计算参数
}AK_ISP_Y_GAMMA_ATTR;

typedef struct ak_isp_lce {
    unsigned char   lce_enable;
    unsigned char   lce_uv_adjust_en;
    unsigned char   lce_uv_adjust_level;
    unsigned char   lce_he_str;
    unsigned char   lce_he_str_low;
    unsigned char   lce_lumFilter_k; //[0,5]
    unsigned char   lce_spatFilter_k;//[0,5]
    unsigned char   lce_tempFilter_k;//[0,15]
    unsigned char   lce_strength[4][8];
}AK_ISP_LCE;

typedef struct ak_isp_lce_blk_attr
{
    unsigned short    lce_blkW;     
    unsigned short    lce_blkH;
    unsigned char     lce_reverseW_g;
    unsigned char     lce_reverseH_g; 
    unsigned char     lce_blkW_cnt;     
    unsigned char     lce_blkH_cnt;
    unsigned char     lce_reverseW_shift;       
    unsigned char     lce_reverseH_shift; 
}AK_ISP_LCE_BLK_ATTR;

typedef struct ak_isp_lce_hist_stat_info{
    unsigned int     lce_hist_stat[4][8][8];      
}AK_ISP_LCE_HIST_STAT_INFO;

typedef struct ak_isp_lce_attr {
    AK_ISP_OP_TYPE  lce_mode;            
    AK_ISP_LCE      manual_lce;
    AK_ISP_LCE      linkage_lce[12];
}AK_ISP_LCE_ATTR;

typedef struct ak_isp_nr1 {
    unsigned short  nr1_k;                 //[0,15]
    unsigned char   nr1_enable;            //ä½¿èƒ½ä½
    unsigned char   nr_keepth;
    unsigned char   nr_keepstr;
    unsigned char   nr_lum_str[32];
    

    unsigned char   de_enhance_enable;
    unsigned char   de_range;
    unsigned char   de_str;
    unsigned char   de_lum_wgt[32];
    
    
    unsigned short  nr1_calc_g_k;
    unsigned short  nr1_calc_r_k;
    unsigned short  nr1_calc_b_k;
    //unsigned short    nr1_lc_lut[17];        //10bit
    unsigned short  nr1_weight_rtbl[17];   //10bit
    unsigned short  nr1_weight_gtbl[17];   //10bit
    unsigned short  nr1_weight_btbl[17];   //10bit
}AK_ISP_NR1;

typedef struct ak_isp_nr1_attr {
    AK_ISP_OP_TYPE  nr1_mode;             //nr1 模式，自动或者联动模式
    AK_ISP_NR1  manual_nr1;
    AK_ISP_NR1  linkage_nr1[12];       //联动参数
}AK_ISP_NR1_ATTR;


typedef struct ak_isp_nr2 {
    unsigned char     nr2_enable;
    unsigned char     nr2_k;                 //[0,15]
    unsigned char     ynr_y_dpc_enable;
    unsigned char     ynr_y_black_dpc_enable;
    unsigned char     ynr_y_white_dpc_enable;
    unsigned char     ynr_keep_th;
    unsigned char     ynr_keep_str;
    unsigned char     ynr_localstr[16];
    unsigned char     ynr_dpc_model;
    unsigned char     ynr_edge1nr_str;
    unsigned char     ynr_edge1nr_range;
    unsigned char     ynr_edge2nr_str;
    unsigned char     ynr_edge2nr_range;
    unsigned short    ynr_intensity1;//[0, 15],default:15
    unsigned short    ynr_intensity2;//[0, 15],default:15
    unsigned short    nr2_calc_y_k;
    unsigned short    nr2_weight_tbl[17];    //10bit
    unsigned short    ynr_dpc_th;

}AK_ISP_NR2;

typedef struct ak_isp_uvnr {
    unsigned char   uvnr_enable;            //使能位
    unsigned char   uvnr_k; 
    unsigned short  uvnr_calc_k;
    unsigned short  uvnr_weight_tbl[17];    //10bit
}AK_ISP_UVNR;

typedef struct ak_isp_nr2_attr {
    AK_ISP_OP_TYPE    nr2_mode;         //手动或者联动模式
    AK_ISP_NR2  manual_nr2;
    AK_ISP_NR2  linkage_nr2[12];
}AK_ISP_NR2_ATTR;

typedef struct ak_isp_uvnr_attr {
    AK_ISP_OP_TYPE  uvnr_mode;       //手动或者联动模式
    AK_ISP_UVNR  manual_uvnr;
    AK_ISP_UVNR  linkage_uvnr[12];
}AK_ISP_UVNR_ATTR;

typedef struct ak_isp_3d_nr_ref_attr {
    unsigned int    yaddr_3d;
    unsigned int    ysize_3d;
    unsigned int    uaddr_3d;
    unsigned int    usize_3d;
    unsigned int    vaddr_3d;
    unsigned int    vsize_3d;
}AK_ISP_3D_NR_REF_ATTR;

typedef struct ak_isp_3d_nr {
    unsigned char       tnr_enable;         //default:1(GUI edit)
    unsigned char       tnr_refFrame_format;    //参考帧压缩格式
    //unsigned short        t_mf_slop; //[0,255] //65536/(t_mf_th2-t_mf_th1)
    unsigned char       ynr_DPCStr;                 //[0, 127]
    unsigned char       ynr_DPCCap;                 //[0, 127]
    unsigned char       ynr_DPCDiffGain[8];         //[0,7]
    unsigned char       ynr_k;          //[0,15]
    unsigned char       ynr_lum_str[32]; //[0, 15]
    unsigned char       ylp_k1;         //[0, 15],default:7(GUI edit)
    unsigned char       ylp_k2;         //[0, 15],default:0(GUI edit)
    unsigned char       yDiffUVWgt;     //[0, 15]
    unsigned char       yDiffEdgeWgt;   //[0, 15]
    unsigned char       t_y_low1;       //[0,127]
    unsigned char       t_y_low2;       //[0,127]
    unsigned char       t_y_k1;         //[0, 127],default:120(GUI edit)
    unsigned char       t_y_k2;         //[0, 127],default:120(GUI edit)
    unsigned char       t_y_ex_k;       //[0, 15]
    unsigned char       t_y_kslop1;     //[0, 127],default:32(GUI edit)
    unsigned char       t_y_kslop2;     //[0, 127]
    unsigned char       t_y_src_choose_k_th;        //[0, 127]
    //unsigned char     t_y_src_choose_slop;        //[0, 63]
    unsigned char       t_y_update_th;
    

    unsigned char       uvnr_k;         //[0, 15],default:8(GUI edit)
    unsigned char       uvlp_k1;        //[0, 15],default:7(GUI edit)
    unsigned char       uvlp_k2;        //[0, 15],default:0(GUI edit)
    unsigned char       t_uv_diff_scale;//[0, 15]
    unsigned char       t_uv_k_low1;    //[0, 127]
    unsigned char       t_uv_k_low2;    //[0, 127]
    unsigned char       t_uv_k1;        //[0, 127],default:120(GUI edit)
    unsigned char       t_uv_k2;        //[0, 127]
    unsigned char       t_uv_ex_k;      //[0, 15],
    unsigned char       t_uv_kslop1;        //[0, 127],default:32(GUI edit)
    unsigned char       t_uv_kslop2;        //[0, 127],default:32(GUI edit)
    unsigned char       t_uv_src_choose_k_th; //[0, 127]
    //unsigned char     t_uv_src_choose_slop; //[0, 63]

    unsigned char       mfactor_k_th;       //[0, 127]
    unsigned char       mfactor_slop;       //[0, 15]
    //unsigned short        sharp_factor_max;       //[0, 15]
      
    
    //
    unsigned char       motion_stat_th;         //[0, 15]
    unsigned char       sfactor_stat_th;        //[0, 3]
    unsigned char       motion_filter;          //[0,15]
    unsigned char       motion_area_expand;     //[0, 1]
    unsigned char       lumStr[32];             //[0, 15]
    //unsigned char         motion_flag[36][64];//运动检测输出
    unsigned short      t_mf_th1;   //[0, 8191],default:300(GUI edit)
    unsigned short      t_mf_th2;   //[0, 8191],default:500(GUI edit)
    unsigned short      ynr_calc_k;     //[0,65535](GUI edit)
    unsigned short      ynr_weight_tbl[17];//ynr_strength(GUI edit)
    unsigned short      t_y_th1;        //[0, 511],default:48(GUI edit)
    unsigned short      t_y_th2;        //[0, 511],default:16(GUI edit)
    unsigned short      t_uv_th1;       //[0, 511],default:48(GUI edit)
    unsigned short      t_uv_th2;       //[0, 511],default:16(GUI edit)
    unsigned short      md_th;          //[0, 65535]
    unsigned short      weight4_7X5;    //[0,8695]
    unsigned short      tnr_motion_flag_th;
}AK_ISP_3D_NR;

typedef struct  ak_isp_3d_nr_attr {
    unsigned char   tnr_debug_output;   //[0,1]
    unsigned char   main_tnr_enable;
    AK_ISP_OP_TYPE  _3d_nr_mode;
    AK_ISP_3D_NR    manual_3d_nr;
    AK_ISP_3D_NR    linkage_3d_nr[12];
}AK_ISP_3D_NR_ATTR;

typedef struct  ak_isp_3d_nr_stat_info {
    unsigned short      MD_stat_max;
    unsigned short      MD_stat[36][64];        
    unsigned int        MD_level;
}AK_ISP_3D_NR_STAT_INFO;

typedef struct ak_isp_3d_nr_blk_attr {
    unsigned char _3d_nr_blkW;
    unsigned char _3d_nr_blkH;  
    unsigned char _3d_nr_last_blkW;
    unsigned char _3d_nr_last_blkH; 
    unsigned char _3d_nr_blkW_cnt; //[0,63]
    unsigned char _3d_nr_blkH_cnt; //[0,63]
    unsigned short _3d_nr_revW; //[0,511]
    unsigned short _3d_nr_revH; //[0,511]
}AK_ISP_3D_NR_BLK_ATTR;


typedef struct ak_isp_gb {
    unsigned short   gb_enable;        //使能位
    unsigned char    gb_en_th;         //[0,255]
    unsigned char    gb_kstep;         //[0,15]
    unsigned short   gb_threshold;     //[0,1023
} AK_ISP_GB;

typedef struct ak_isp_gb_attr {
    AK_ISP_OP_TYPE   gb_mode;      
    AK_ISP_GB  manual_gb;
    AK_ISP_GB  linkage_gb[12];
} AK_ISP_GB_ATTR;

typedef struct ak_isp_demo_attr {
    unsigned short  dmmosa_enable;
    unsigned short  dm_rg_thre;     //[0 1023]
    unsigned short  dm_bg_thre;     //[0 1023]
    unsigned short  dm_hf_th1;      //[0, 1023]
    unsigned short  dm_hf_th2;      //[0, 1023]
    
    unsigned short  dm_hv_th;       //方向判别系数
    unsigned char   dm_rg_gain;     //[0 255]
    unsigned char   dm_bg_gain;     //[0 255]
    unsigned char   dm_gr_gain;     //[0 255]
    unsigned char   dm_gb_gain;     //[0 255]
}AK_ISP_DEMO_ATTR;

typedef struct ak_isp_ccm {
    unsigned char   cc_enable;
    unsigned char   cc_sat_tbl[9];      //[0, 15]   
    signed short    ccm[3][3];         //[-2048, 2047]
}AK_ISP_CCM;

typedef struct ak_isp_auto_ccm {
    unsigned short  cc_color_temp;
    AK_ISP_CCM      auto_ccm_para;
}AK_ISP_AUTO_CCM;


typedef struct ak_isp_ccm_ctr {  
   unsigned short   cc_start_gain;
   unsigned short   cc_min_saturation;
   unsigned short   cc_adjust_slop;    
}AK_ISP_CCM_CTR;

typedef struct ak_isp_manual_ccm_attr {     
    AK_ISP_CCM_CTR manual_ccm_ctrl;
    AK_ISP_CCM     manual_ccm_para; 
}AK_ISP_MANUAL_CCM_ATTR;

typedef struct ak_isp_auto_ccm_attr{    
    unsigned char  color_matrix_num;
    AK_ISP_CCM_CTR  auto_ccm_ctrl;
    AK_ISP_AUTO_CCM auto_ccm_para[10];
}AK_ISP_AUTO_CCM_ATTR;

typedef struct ak_isp_ccm_attr {
    AK_ISP_OP_TYPE          cc_mode;        //颜色校正矩阵联动或者手动
    AK_ISP_MANUAL_CCM_ATTR  manual_ccm;
    AK_ISP_AUTO_CCM_ATTR    auto_ccm;       //四个联动矩阵
}AK_ISP_CCM_ATTR;

typedef struct ak_isp_hdr {
    unsigned char   hdr_enable;
    unsigned char   hdr_uv_adjust_enable;        //uv调整使能
    unsigned short  hdr_th1;      //0-1023
    unsigned short  hdr_th2;      //0-1023
    unsigned short  hdr_th3;      //0-1023
    unsigned short  hdr_th4;      //0-1023
    unsigned short  hdr_th5;      //0-1023

    //unsigned short wdr_light_weight;

    unsigned short  area_tb1[65];     //曲线 10bit
    unsigned short  area_tb2[65];     //曲线 10bit
    unsigned short  area_tb3[65];     //曲线 10bit
    unsigned short  area_tb4[65];     //曲线 10bit
    unsigned short  area_tb5[65];     //曲线 10bit
    unsigned short  area_tb6[65];     //曲线 10bit

    unsigned short  area1_key[16];
    unsigned short  area2_key[16];
    unsigned short  area3_key[16];
    unsigned short  area4_key[16];
    unsigned short  area5_key[16];
    unsigned short  area6_key[16];
    
    
    //unsigned char  hdr_spatialfilt_pos;
    
    unsigned short   hdr_cnoise_suppress_yth1;   //色彩噪声亮度阈值1
    unsigned short   hdr_cnoise_suppress_yth2;   //色彩噪声亮度阈值2
    unsigned short   hdr_cnoise_suppress_gain;   //色差抑制
    unsigned char    hdr_cnoise_suppress_slop;   //抑制斜率
    unsigned char    hdr_uv_adjust_level;        //uv调整程度, [0,31]
    unsigned char    hdr_spatialfilt_k;//[0,5]
}AK_ISP_HDR;

typedef struct ak_isp_hdr_attr {
    AK_ISP_OP_TYPE hdr_mode;              //模式选择，手动或者联动
    AK_ISP_HDR manual_hdr;
    AK_ISP_HDR linkage_hdr[12];
}AK_ISP_HDR_ATTR;

typedef struct ak_isp_hdr_blk_attr {
     unsigned short     hdr_blkW;
     unsigned short     hdr_blkH;
     unsigned short     hdr_reverseW_g;       //[0,511]
     unsigned short     hdr_reverseH_g;       //[0,511]
     unsigned short     hdr_weight_g;         //[0,511]
     
     unsigned char      hdr_blkW_cnt;
     unsigned char      hdr_blkH_cnt;
     unsigned char      hdr_reverseW_shift;   //[0,15]
     unsigned char      hdr_reverseH_shift;   //[0,15]
     unsigned char      hdr_weight_shift;     //[0,15]
     unsigned char      hdr_weight_k;         //[0,15]

}AK_ISP_HDR_BLK_ATTR;

typedef struct ak_isp_sharp {
    unsigned char   ysharp_enable;              //[0,1]
    unsigned char   ysharp_nr_enable;           //[0,1]
    unsigned char   edge_enable;
    unsigned char   sharp_method;               //[0,3]
    unsigned char   mf_hpf_k;                   //[0,127]
    unsigned char   mf_hpf_shift;               //[0,15]
    unsigned char   hf_hpf_k;                   //[0,127]
    unsigned char   hf_hpf_shift;               //[0,15]
    unsigned char   ysharp_nr_k;  
    unsigned char   ysharp_nr_keepTh;
    unsigned char   ysharp_nr_keepStr;
    unsigned char   ysharp_mfacotr_sup;
    unsigned char   ysharp_freqctrl;
    //unsigned char ysharp_reduce_factor[36][64];
    unsigned char   ysharp_lumWgt[32];
    unsigned char   ysharpStr[16];
    unsigned char   edgeStr[32];
    unsigned char   edgeFiltStr;
    unsigned char   edgeWhiteGain;
    unsigned char   edgeBlackGain;
    unsigned char   edgeWideWgt;
    unsigned char   edge45DWgt;
    unsigned char   weakEdgeCtrl;
    unsigned char   localShootSupTh;           //[0,31]
    unsigned char   localShootSupSlop;         //[0,15] default
    unsigned char   ovShootSupStr;             //[0, 31]
    unsigned char   udShootSupStr;             //[0, 31]
    signed char     UVGain[4];
    signed char     UVArea[4][4];
    
    unsigned short  ysharp_nr_calc_k;
    unsigned short  ysharp_nrweight_tbl[17];
    unsigned short  EdgeGainMax; //[0, 511]
    signed short    MF_HPF_LUT[256];            //[-256,255]
    signed short    HF_HPF_LUT[256];            //[-256,255]
    unsigned short  MF_LUT_KEY[16];
    unsigned short  HF_LUT_KEY[16];
}AK_ISP_SHARP;

typedef struct ak_isp_sharp_attr{
    unsigned char   sharp_debug_output;         //[0,3]
    AK_ISP_OP_TYPE  ysharp_mode;
    AK_ISP_SHARP    manual_sharp_attr;
    AK_ISP_SHARP    linkage_sharp_attr[12];
}AK_ISP_SHARP_ATTR;

typedef struct ak_isp_sharp_ex_attr {
    signed char  mf_HPF[6];         
                                 //M13,M14,M15,
                                 //M11,M12,M14,
                                 //M10,M11,M13,
    signed char  hf_HPF[3];     
                                 //M22 M21,M22,
                                 //M21,M20, M21,
}AK_ISP_SHARP_EX_ATTR;
typedef struct ak_isp_sharp_blk_attr {
    unsigned char  ysharp_blkW;
    unsigned char  ysharp_blkH;
    unsigned char  ysharp_blkW_cnt;
    unsigned char  ysharp_blkH_cnt;
    unsigned short ysharp_revW;
    unsigned short ysharp_revH;
}AK_ISP_SHARP_BLK_ATTR;


typedef struct {
    AK_ISP_SHARP    sharp_tmp;
    int gain;
}AK_ISP_SHARP_INFO;

typedef struct ak_isp_saturation {
    unsigned char    SE_enable;    // 使能位
    unsigned char    SE_scale1;       //[0,255]
    unsigned char    SE_scale2;       //[0,255]
    unsigned char    SE_scale3;       //[0,255]
    //unsigned char  SE_scale_slop1;  //[0,255]
    //unsigned char  SE_scale_slop2;  //[0,255]
    unsigned short   SE_th1;          //[0, 1023]
    unsigned short   SE_th2;          //[0, 1023]
    unsigned short   SE_th3;          //[0, 1023]
    unsigned short   SE_th4;          //[0, 1023]
}AK_ISP_SATURATION;

typedef struct  ak_isp_saturation_attr {
    AK_ISP_OP_TYPE      SE_mode;         //饱和度模式
    AK_ISP_SATURATION   manual_sat;
    AK_ISP_SATURATION   linkage_sat[12];
}AK_ISP_SATURATION_ATTR;

typedef struct  ak_isp_contrast {
     unsigned short  y_contrast;    //[0,511]
     signed short    y_shift;   //[0, 511]
}AK_ISP_CONTRAST;

typedef struct ak_isp_auto_contrast
{
    unsigned short  dark_pixel_area;    //[0, 511]
    unsigned char   dark_pixel_rate;    //[1, 256]
    unsigned char   shift_max;          //[0, 127]
}AK_ISP_AUTO_CONTRAST;

typedef struct  ak_isp_contrast_ATTR {
    AK_ISP_OP_TYPE  contrast_mode;  //模式选择，手动或者联动
    AK_ISP_CONTRAST manual_contrast;
    AK_ISP_AUTO_CONTRAST    linkage_contrast[12];
}AK_ISP_CONTRAST_ATTR;

typedef  struct ak_isp_fcs {
    unsigned char   fcs_enable;         //使能位
    unsigned char   fcs2_enable;
    unsigned char   fcs_th;         //[0, 255]
    unsigned char   fcs_gain_slop;  //[0,63]
    unsigned char   fcs2_c_th;  //[0, 127]
    unsigned char   fcs2_c_str; //[0, 31]
    unsigned char   fcs2_s_th;  //[0, 127]
    unsigned char   fcs2_s_str; //[0, 31]
    unsigned char   dm_fcs_purple_str;//[0, 31]
    unsigned char   dm_fcs_cyan_str;  //[0, 31]
    unsigned char   dm_fcs_chrom_k;  //[0, 31]
    unsigned char   dm_fcs_hfp_k;    //[0, 31]
    unsigned char   hue_satSupTh;   //[0, 15]
    unsigned char   hue_satSupStr;  //[0, 15]
    unsigned char   pfs_str;        //[0, 63]
    unsigned char   pfs_start_c;    //[0, 63]
    unsigned char   pfs_range;      //[0, 15]
    unsigned char   pfs_highlight_th;       //[0, 255]
    unsigned char   pfs_lowlight_th;        //[0, 255]
}AK_ISP_FCS;

typedef  struct ak_isp_fcs_attr {
    AK_ISP_OP_TYPE  fcs_mode;       //模式选择，手动或者联动
    AK_ISP_FCS  manual_fcs;
    AK_ISP_FCS  linkage_fcs[12];
}AK_ISP_FCS_ATTR;

typedef struct ak_isp_hue {
    unsigned char   hue_sat_en;         //hue使能
    //unsigned char     pfs_str_tbl[16];//[0, 63]
    signed char     hue_lut_a[65];  //[-128, 127]
    signed char     hue_lut_b[65];  //[-128, 127]
    unsigned char   hue_lut_s[65];  //[0, 255]
}AK_ISP_HUE;

typedef struct ak_isp_hue_high_blk_attr{
    unsigned char  hue_yuvhightlumi_blkW_cnt;
    unsigned char  hue_yuvhightlumi_blkH_cnt;
    unsigned short hue_yuvhightlumi_blkW;
    unsigned short hue_yuvhightlumi_blkH;
}AK_ISP_HUE_HIGH_BLK_ATTR;


typedef struct ak_isp_auto_hue{
    unsigned short    hue_color_temp; 
    AK_ISP_HUE        auto_hue_para;
}AK_ISP_AUTO_HUE;

typedef struct ak_isp_auto_hue_attr {
    unsigned short      hue_num;
    AK_ISP_AUTO_HUE     auto_hue_para[10]; 
}AK_ISP_AUTO_HUE_ATTR;

typedef struct ak_isp_hue_attr {
    AK_ISP_OP_TYPE          hue_mode;   //联动或者手动
    AK_ISP_HUE              manual_hue;
    AK_ISP_AUTO_HUE_ATTR    auto_hue;       //四个联动参数
}AK_ISP_HUE_ATTR;

typedef  struct ak_isp_rgb2yuv_attr {
    unsigned short rgb2yuv_enable;
    AK_ISP_RGB2YUV_MODEL  mode;                 //bt601 或者bt709
}AK_ISP_RGB2YUV_ATTR;

typedef struct ak_isp_effect_attr {
    unsigned char   yuv_out_enable;
    unsigned char   dark_margin_en;   //黑边使能
    unsigned char   y_a;     // [0, 255]
    signed  char    y_b;     //[-128, 127]
    signed short    uv_a;    //[-256, 255]
    signed short    uv_b;    //[-256, 255]
}AK_ISP_EFFECT_ATTR;

typedef struct ak_isp_ddpc {
    //unsigned char  ddpc_enable;       //动态坏点使能位
    unsigned char  white_dpc_enable;    //白点消除使能位
    unsigned char  black_dpc_enable;    //黑点消除使能位
    unsigned char  ddpc_th_slop;
    unsigned char  ddpc_strength;
    unsigned short ddpc_th_base;             //10bit
}AK_ISP_DDPC;

typedef struct ak_isp_ddpc_attr {
    AK_ISP_OP_TYPE ddpc_mode;            //模式选择，手动或者联动
    AK_ISP_DDPC manual_ddpc;
    AK_ISP_DDPC linkage_ddpc[12];
}AK_ISP_DDPC_ATTR;

typedef struct ak_isp_af_attr {
    unsigned short  af_stat_enable;
    unsigned char af_spotlight_lum_th;    //[0 ,255]
    unsigned char af_spotlight_edge_th;   //[0 ,255]
    unsigned char af_spotlight_edge_gain; //[0, 15]
    unsigned char af_clip_th;             //[0, 63]
    unsigned char af_peak_ctrl;           //[0, 255]
    unsigned char af_rshift;              //[0, 15]
}AK_ISP_AF_ATTR;


typedef struct ak_isp_af_stat_info {
   unsigned int  af_statics[8][16][5];         //统计结果
   unsigned char af_local_highlight[64][128];
}AK_ISP_AF_STAT_INFO;

typedef struct ak_isp_3d_nr_rer_size_stat_info{
    unsigned int  ref_size_statis[3];
}AK_ISP_3D_NR_REF_SIZE_STAT_INFO;

#if 0
typedef struct ak_isp_weight_attr {
   unsigned short   zone_weight[8][16];      //权重系数
}AK_ISP_WEIGHT_ATTR;
#endif

//////////////////////Auto White Balance//////////////////////
typedef enum ak_isp_awb_status{
    AWB_STATUS_PAUSE = 0,
    AWB_STATUS_NORMAL,
}AK_ISP_AWB_STATUS;

typedef enum ak_isp_awb_algo_type{
    AWB_ALGO_DEFAULT = 0,
    AWB_ALGO_AVERAGE_WEIGHT = 1,
    AWB_ALGO_BLOCK_ASSIST,
    AWB_ALGO_GW,
    AWB_ALGO_BUTT,
}AK_ISP_AWB_ALGO_TYPE;



typedef  struct  ak_isp_mwb_attr {
    unsigned short  r_gain;
    unsigned short  g_gain;
    unsigned short  b_gain;
    signed short    r_offset;
    signed short    g_offset;
    signed short    b_offset;
}AK_ISP_MWB_ATTR;

typedef struct ak_isp_awb_stat_calib_para{
    unsigned short   gr_low[10];            //gr_low[i]<=gr_high[i]
    unsigned short   gb_low[10];            //gb_low[i]<=gb_high[i]
    unsigned short   gr_high[10];
    unsigned short   gb_high[10];
    unsigned short   rb_low[10];           //rb_low[i]<=rb_high[i]
    unsigned short   rb_high[10];
}AK_ISP_AWB_STAT_CALIB_PARA;

typedef  struct ak_isp_awb_attr {
    
    unsigned char    g_weight[16];
    unsigned char    y_low;                 //y_low<=y_high
    unsigned char    y_high;
    unsigned char    err_est;
    unsigned char    awb_iso_track_enable;
    unsigned char    colortemp_envi[10];
    AK_ISP_AWB_STAT_CALIB_PARA awb_stat_calib_para[12];
    #if 0
    unsigned short   gr_low[10];            //gr_low[i]<=gr_high[i]
    unsigned short   gb_low[10];            //gb_low[i]<=gb_high[i]
    unsigned short   gr_high[10];
    unsigned short   gb_high[10];
    unsigned short   rb_low[10];           //rb_low[i]<=rb_high[i]
    unsigned short   rb_high[10];
    #endif

    //awb软件部分需要设置的参数
    unsigned short   auto_wb_step;                 //白平衡步长计算
    unsigned short   total_cnt_thresh;            //像素个数阈值
    unsigned short   colortemp_stable_cnt_thresh; //稳定帧数，多少帧一样认为环境色温改变
    
    AK_ISP_AWB_ALGO_TYPE alg_type;
}AK_ISP_AWB_ATTR;
typedef  struct ak_isp_awb_blk_attr {
    unsigned int  wb_blkW;
    unsigned int  wb_blkH;
    unsigned char wb_blkW_cnt;
    unsigned char wb_blkH_cnt;
}AK_ISP_AWB_BLK_ATTR;


typedef  struct  ak_isp_awb_ration {
    unsigned int  ratio[10];
}AK_ISP_AWB_RATIO;

typedef  struct  ak_isp_awb_ex_attr {
    unsigned char       awb_ex_ctrl_enable;
    AK_ISP_AWB_RATIO    prefer_rratio[12];
    AK_ISP_AWB_RATIO    prefer_bratio[12];
}AK_ISP_AWB_EX_ATTR;


typedef struct ak_isp_awb_blk_stat_info{
    unsigned int wb_Ravg_stat[16][16];
    unsigned int wb_Gavg_stat[16][16];
    unsigned int wb_Bavg_stat[16][16];
}AK_ISP_AWB_BLK_AVG_STAT_INFO;

typedef struct ak_isp_awb_wp_blk_stat_info{
    unsigned int wp_r_stat[16][16];
    unsigned int wp_g_stat[16][16];
    unsigned int wp_b_stat[16][16];
    unsigned int wp_cnt_statics[16][16];
}AK_ISP_AWB_BLK_WP_STAT_INFO;

typedef  struct ak_isp_awb_stat_info {
    //白平衡统计结果
    unsigned int   total_R[10];
    unsigned int   total_G[10];
    unsigned int   total_B[10];
    unsigned int   total_cnt[10];
    //经由自动白平衡算法算出的白平衡增益值
    AK_ISP_AWB_BLK_AVG_STAT_INFO    wb_blk_avg_stat;
    AK_ISP_AWB_BLK_WP_STAT_INFO     wb_blk_wp_stat;
    #if 0
    unsigned short   r_gain;
    unsigned short   g_gain;
    unsigned short   b_gain;
    signed short   r_offset;
    signed short   g_offset;
    signed short   b_offset;
    unsigned short   current_colortemp_index;     //环境色温标记，是参数随环境变化的色温指标。
    unsigned short   colortemp_stable_cnt[10];         //每一种色温稳定的帧数计数
    unsigned short current_colortemp;  
    AK_ISP_CCM     current_ccm; 
    #endif
    
}AK_ISP_AWB_STAT_INFO;

typedef struct  ak_isp_awb_fine_correct_blk_attr{
    unsigned char wbfc_blkW;
    unsigned char wbfc_blkH;
    unsigned char wbfc_revW;
    unsigned char wbfc_revH;
    unsigned char wbfc_blkW_cnt;
    unsigned char wbfc_blkH_cnt;
}AK_ISP_AWB_FINE_CORRECTION_BLK_ATTR;


typedef struct  ak_isp_awb_fine_gain_tbl{
    signed char  wb_R_fine_gain[17][17];
    signed char  wb_G_fine_gain[17][17];
    signed char  wb_B_fine_gain[17][17];
}AK_ISP_AWB_FINE_GAIN_TBL;


typedef struct ak_isp_awb_run_info{
    unsigned short   r_gain;
    unsigned short   g_gain;
    unsigned short   b_gain;
    signed short     r_offset;
    signed short     g_offset;
    signed short     b_offset;
    unsigned char    is_stable;
    unsigned char    current_colortemp_index;     //环境色温标记，是参数随环境变化的色温指标。
    unsigned short   colortemp_stable_cnt[10];         //每一种色温稳定的帧数计数
    unsigned short   current_colortemp;  
    AK_ISP_CCM       current_ccm;
}AK_ISP_AWB_RUN_INFO;


typedef  struct ak_isp_awb_calib_node_attr {
    unsigned short   cali_gr;
    unsigned short   cali_gb;                   
    unsigned short   cali_color_temp;  
}AK_ISP_AWB_CALIB_NODE_ATTR;

typedef  struct  ak_isp_awb_calib_attr {
    unsigned  char         awb_calib_node_num;
    AK_ISP_AWB_CALIB_NODE_ATTR  awb_calib_para[10];
}AK_ISP_AWB_CALIB_INFO;

typedef struct ak_isp_wb_gain {
    unsigned short  r_gain;
    unsigned short  g_gain;
    unsigned short  b_gain;
    signed short  r_offset;
    signed short  g_offset;
    signed short  b_offset;
}AK_ISP_WB_GAIN;

typedef struct ak_isp_wb_attr {
    unsigned char       awb_run_interval;
    unsigned char       wb_enable;
    unsigned char       wb_statis_enable;
    unsigned char       wb_fine_linkage_strength[13];
    unsigned short      wb_fine_correct_enable;
    AK_ISP_AWB_STATUS   wb_status;
    AK_ISP_OP_TYPE      wb_type;
    AK_ISP_MWB_ATTR     manual_wb_para;
    AK_ISP_AWB_ATTR     auto_wb_para;
}AK_ISP_WB_ATTR;



///////////////////AUTO Exposure//////////////////////
typedef enum ak_isp_ae_status{
    AE_STATUS_PAUSE = 0,
    AE_STATUS_NORMAL,
}AK_ISP_AE_STATUS;

typedef enum ak_isp_ae_strgy_model {
    AE_EXP_HIGHLIGHT_PRIOR = 0,
    AE_EXP_LOWLIGHT_PRIOR,
    AE_STRGY_MODEL_BUFF,
}AK_ISP_AE_STRGY_MODEL;

#if 0
typedef enum ak_isp_win_weight_type {
   AE_CENTER_WGHT= 0,        
   AE_AVERAGE = 1,    
   AE_SPOT ,    
   AE_WGHT_END ,    
}AK_ISP_WIN_WEIGHT_TYPE;

typedef struct ak_isp_weight_attr {
   unsigned short   zone_weight[8][16];        
}AK_ISP_WEIGHT_ATTR;
#endif

typedef struct ak_isp_wdr_ae_attr 
{
    unsigned short exp_ratio_type;
    unsigned short exp_ratio_LS;
    unsigned short exp_ratio_SVS;
    unsigned short exp_ratio_max;
    unsigned short exp_ratio_min;
    unsigned char  exp_ratio_speed;
    unsigned char  exp_highlight_ctrl;
}AK_ISP_WDR_AE_ATTR;

typedef struct ak_isp_raw_hist_stat_info {
    unsigned int   raw_g_hist[256];
    unsigned long long   raw_g_total;
}AK_ISP_RAW_HIST_STAT_INFO;


typedef struct ak_isp_rgb_hist_stat_info {
    unsigned int   rgb_hist[256];
    unsigned long long   rgb_total;
}AK_ISP_RGB_HIST_STAT_INFO;


typedef struct  ak_isp_yuv_hist_stat_info {
    unsigned int    y_hist[256];
    unsigned long long  y_total;
}AK_ISP_YUV_HIST_STAT_INFO;

typedef struct ak_isp_ae_strgy_attr {
    unsigned int    hist_weight[16];
}AK_ISP_AE_STRGY_ATTR;

typedef struct ak_isp_ae_route_node_attr {
    unsigned int   exp_time;                    
    unsigned int   again;               
    unsigned int   dgain; 
    unsigned int   isp_dgain; 
}AK_ISP_AE_ROUTE_NODE_ATTR; 

typedef struct ak_isp_ae_route_attr{
    unsigned char   ae_route_en;
    unsigned char   ae_route_node_num;
   AK_ISP_AE_ROUTE_NODE_ATTR  ae_route_node[10];
}AK_ISP_AE_ROUTE_ATTR;

typedef struct ak_isp_ae_ex_convergence_attr {
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
}AK_ISP_AE_EX_CONVER_ATTR;

typedef struct ak_isp_me_attr {
    unsigned int   exp_time;
    unsigned int   a_gain;
    unsigned int   d_gain;
    unsigned int   isp_d_gain;
}AK_ISP_ME_ATTR;

typedef struct ak_isp_ex_zone_weight_attr {
    unsigned char   win_weight[8][16];
    unsigned char   ex_zone_weight_str;
}AK_ISP_EX_ZONE_WEIGHT_ATTR;

typedef struct ak_isp_ae_attr {
    unsigned int   exp_time_max;            //曝光时间的最大值
    unsigned int   a_gain_max;          //模拟增益的最大值
    unsigned int   d_gain_max;              //数字增益的最大值
    unsigned int   isp_d_gain_max;          //isp数字增益的最大值
    unsigned short exp_time_min;            //曝光时间的最小值
    unsigned short a_gain_min;              //模拟增益的最小值
    unsigned short d_gain_min;          //数字增益的最小值
    unsigned short isp_d_gain_min;          //isp数字增益的最小
    unsigned short blacklight_rate_max;         
    unsigned short blacklight_rate_min; 
    unsigned short exp_step;                //用户曝光调整步长
    unsigned char  exp_stable_range;        //稳定范围
    unsigned char  exp_hold_range;
    unsigned char  exp_speed;
    unsigned char  anti_flicker_target_lumi;
    unsigned char  anti_flicker_start_exp;//[1,100]
    unsigned char  exp_lumi_filter_para;    
    unsigned char  target_lumiance[12];         //目标亮度
   // unsigned int   hist_weight[16];
    unsigned char  blacklight_compensation_en;
    unsigned char  blacklight_detect_scope;     

    //AK_ISP_WIN_WEIGHT_TYPE  ae_win_weight_type;
    //AK_ISP_WEIGHT_ATTR    win_weight[4];
    AK_ISP_RAW_G_HIST_ATTR  raw_g_hist_attr;
    AK_ISP_RGB_HIST_ATTR    rgb_hist_attr;
    AK_ISP_YUV_HIST_ATTR    yuv_hist_attr;
    unsigned short envi_gain_range[12][2];
    AK_ISP_AE_EX_CONVER_ATTR  ae_ex_convergence_para;
    AK_ISP_AE_STRGY_MODEL     ae_strgy_type;
    AK_ISP_AE_STRGY_ATTR      ae_strgy_para[3]; 
    AK_ISP_AE_ROUTE_ATTR      exp_route_para;
}AK_ISP_AE_ATTR;

typedef  struct ak_isp_ae_run_info {
    unsigned char   current_calc_avg_lumi;              //现在的计算出的亮度值
    unsigned char   current_calc_avg_compensation_lumi;     //经过曝光补偿后的亮度值

    unsigned char   current_target_lumi;                 //白天黑夜的标记
    unsigned char   is_stable;        

    unsigned int  current_a_gain;                       //模拟增益的值
    unsigned int  current_d_gain;                       //数字增益的值
    unsigned int  current_isp_d_gain;                   //isp数字增益的值
    unsigned int  current_exp_time;                 //曝光时间的值

    unsigned int  current_a_gain_step;              //现在的模拟增益的步长
    unsigned int  current_d_gain_step;              //数字增益的步长
    unsigned int  current_isp_d_gain_step;              //isp数字增益的步长
    unsigned int  current_exp_time_step; 
    //AK_ISP_AE_ROUTE_ATTR      ae_route_para;
}AK_ISP_AE_RUN_INFO;

typedef struct ak_ae_stat_info{
    AK_ISP_RAW_HIST_STAT_INFO   raw_hist_stat_para;    
    AK_ISP_RGB_HIST_STAT_INFO   rgb_hist_stat_para;     
    AK_ISP_YUV_HIST_STAT_INFO   yuv_hist_stat_para; 
    AK_ISP_RAWWDRCOMB_HIST_STAT_INFO raw_wdr_comb_stat_para;
}AK_ISP_AE_STAT_INFO;

typedef struct {
    int a_gain;
    int d_gain;
    int isp_d_gain;
    int exp_time;
    int effect_frame;
    int effect_gain;
}AK_ISP_AE_UPDATE_QUEUE;


typedef  struct  ak_isp_exposure_attr {
    AK_ISP_AE_STATUS  exp_status;
    unsigned char     ae_run_interval;
    AK_ISP_OP_TYPE    exp_type;
    AK_ISP_ME_ATTR    manual_exp_para;
    AK_ISP_AE_ATTR    auto_exp_para;
}AK_ISP_EXPOSURE_ATTR;
//帧率控制结构体
typedef  struct ak_isp_frame_rate_attr {
    unsigned int    hight_light_frame_rate ;
    unsigned int    hight_light_max_exp_time ;
    unsigned int    hight_light_to_low_light_gain;
    unsigned int    low_light_frame_rate;
    unsigned int    low_light_max_exp_time;
    unsigned int    low_light_to_hight_light_gain;
}AK_ISP_FRAME_RATE_ATTR;

typedef struct ak_isp_misc_attr {
    unsigned short  hsyn_pol;
    unsigned short  vsync_pol;
    unsigned short  pclk_pol;
    unsigned short  test_pattern_en;
    unsigned char   test_pattern_cfg;
    unsigned char   cfa_mode;
    unsigned char   inputdataw;
    unsigned char   yuv_swing;
    unsigned char   yuv_formula;
    unsigned short  one_line_cycle;
    unsigned short  hblank_cycle;
    unsigned short  vflip;
    unsigned short  hflip;
    unsigned short  frame_start_delay_en;
    unsigned short  frame_start_delay_num;
    unsigned short  frame_start_edge;

    unsigned short  mipi_virtual_channel_sel; 
    unsigned short  mipi_bps_virtual_channel_sel; 
    unsigned short  mipi_hsync_str_sel;
    unsigned short  mipi_hsync_end_sel;
    unsigned short  mipi_auto_decoder_pix;
} AK_ISP_MISC_ATTR;






typedef enum ak_isp_pclk_polar {
    POLAR_ERR   = 0,
    POLAR_RISING,
    POLAR_FALLING,
}AK_ISP_PCLK_POLAR;



typedef struct ak_isp_wb_gain_info {
    AK_ISP_WB_GAIN isp_wb_gain;
    char flg;

}ISP_WB_GAIN_INFO;

typedef struct ak_isp_ae_info_update
{
    AK_ISP_AE_INFO ae_info;
    char flg;

}AK_ISP_AE_INFO_UPDATE;
#if 0
struct ae_fast_struct {
    int sensor_exp_time;
    int sensor_a_gain;
    int sensor_d_gain;
    int isp_d_gain;
    AK_ISP_WB_GAIN wb;
};
#endif
struct mem_ptrs {
    T_U64 *pRegIspTop;
    T_U64 *pRegIspProfile;
    T_U64 *pRegConfDataFrag;
    T_U64 *pRegConfDataBlk;
    T_U64 *pRegStatDataFrag;
    T_U64 *pRegStatDataBlk;
    T_U64 *pRegIVDataFrag;
    T_U64 *pRegIVDataBlk;
};

void  ak_isp_ae_info(void *isp_struct,AK_ISP_AE_INFO *isp_ae_info);
int ak_isp_vi_start_capturing(void *isp_struct);

int ak_isp_vi_capturing_one_prepare(void *isp_struct);
int ak_isp_vi_capturing_one(void *isp_struct);
int ak_isp_vi_stop_capturing(void *isp_struct);
int ak_isp_vi_set_input_size(void *isp_struct,int width, int height);
int ak_isp_vi_set_crop(void *isp_struct, int left, int top, int width, int height);
int ak_isp_vi_get_crop(void *isp_struct,int *width, int *height);

int ak_isp_vi_get_input_data_format(void *isp_struct, struct input_data_format *idf);
int ak_isp_vi_set_misc_attr(void *isp_struct,AK_ISP_MISC_ATTR *p_misc);
int ak_isp_vi_get_misc_attr(void *isp_struct,AK_ISP_MISC_ATTR *p_misc);

int ak_isp_set_flip_mirror(void *isp_struct,int flip_en, int mirror_en);



int ak_isp_vo_set_target_lines(void *isp_struct, int lines);
int ak_isp_vo_check_update_status(void *isp_struct);
int ak_isp_vo_check_irq_status (void *isp_struct);
int ak_isp_vo_clear_irq_status(void *isp_struct,int bit);
int ak_isp_vo_enable_irq_status(void *isp_struct,int bit);
unsigned int ak_isp_vo_get_reg(void *isp_struct, int reg);
int ak_isp_vo_set_cfg_reg(void *isp_struct,int regaddr, int value, int bitmask);

int ak_isp_enable_buffer_main(void *isp_struct);
int ak_isp_enable_buffer_sub(void *isp_struct);
int ak_isp_enable_buffer_ch3(void *isp_struct);
int ak_isp_vo_enable_buffer(void *isp_struct,enum buffer_id id);
int ak_isp_vo_enable_buffer_main(void *isp_struct,enum buffer_id id);
int ak_isp_vo_enable_buffer_sub(void *isp_struct,enum buffer_id id);
int  ak_isp_vo_enable_buffer_ch3(void *isp_struct,enum buffer_id  id);

int ak_isp_vo_disable_buffer(void *isp_struct,enum buffer_id id);
int ak_isp_vo_disable_buffer_main(void *isp_struct,enum buffer_id id);
int ak_isp_vo_disable_buffer_sub(void *isp_struct,enum buffer_id id);
int ak_isp_vo_disable_buffer_ch3(void *isp_struct,enum buffer_id id);
int ak_isp_vo_set_buffer_addr(void *isp_struct,enum buffer_id id,
        unsigned long yaddr_main_chan_addr, unsigned long yaddr_sub_chan_addr,unsigned long yaddr_sub_chan3_addr);
int ak_isp_vo_set_main_buffer_addr \
    (void *isp_struct,
     enum buffer_id  id,\
     unsigned long yaddr_main_chan_addr);
int ak_isp_vo_set_sub_buffer_addr \
    (void *isp_struct,
     enum buffer_id  id,\
     unsigned long yaddr_sub_chan_addr);
int ak_isp_vo_set_ch3_buffer_addr \
    (void *isp_struct,
     enum buffer_id  id,\
     unsigned long yaddr_chan3_addr);
int ak_isp_vo_set_main_buffer_addr2(void *isp_struct, int buffer_index,
    unsigned int yaddr, unsigned int uaddr, unsigned int vaddr);
int ak_isp_vo_set_sub_buffer_addr2(void *isp_struct, int buffer_index,
    unsigned int yaddr, unsigned int uaddr, unsigned int vaddr);
int ak_isp_vo_set_ch3_buffer_addr2(void *isp_struct, int buffer_index,
    unsigned int yaddr, unsigned int uaddr, unsigned int vaddr);
int ak_isp_vo_get_using_frame_main_buf_id(void *isp_struct);
int ak_isp_vo_get_using_frame_sub_buf_id(void *isp_struct);
int ak_isp_vo_get_using_frame_ch3_buf_id(void *isp_struct);
int ak_isp_vo_get_using_frame_buf_hwid(void *isp_struct);

int ak_isp_vo_update_setting(void *isp_struct);
int ak_isp_is_continuous(void *isp_struct);

//int ak_isp_vpp_set_osd(AK_ISP_OSD_ATTR *p_osd);

//int ak_isp_vpp_set_occlusion_attr(AK_ISP_OCCLUSION_ATTR *p_occ);
//int ak_isp_vpp_occlusion_color_attr(AK_ISP_OCCLUSION_COLOR *p_occ_color);

/**
 * @brief: set pwl param
 * @author: lizhi
 * @date: 2023-03-16
 * @param [in] *p_pwl:pwl param
 */
int ak_isp_vp_set_pwl_attr(void *isp_struct,AK_ISP_PWL_ATTR *p_pwl);
/**
 * @brief: get pwl param
 * @author: lizhi
 * @date: 2023-03-16
 * @param [in] *p_pwl:pwl param
 */
int ak_isp_vp_get_pwl_attr( void *isp_struct,AK_ISP_PWL_ATTR *p_pwl);

/**
 * @brief: set wdr_comb param
 * @author: lizhi
 * @date: 2023-03-16
 * @param [in] *p_wdr_comb:wdr_comb param
 */
int ak_isp_vp_set_wdr_comb_attr(void *isp_struct,AK_ISP_RAWWDRCMB_ATTR *p_wdr_comb);

/**
 * @brief: get wdr_comb param
 * @author: lizhi
 * @date: 2023-03-016
 * @param [in] *p_wdr_comb:wdr_comb param
 */
int ak_isp_get_vp_wdr_comb_attr(void *isp_struct,AK_ISP_RAWWDRCMB_ATTR *p_wdr_comb);

/**
 * @brief: set blc param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_blc:  blc param
 */
int ak_isp_vp_set_blc_attr(void *isp_struct,AK_ISP_BLC_ATTR *p_blc);

/**
 * @brief: get blc param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_blc:  blc param
 */
int ak_isp_vp_get_blc_attr(void *isp_struct,AK_ISP_BLC_ATTR *p_blc);

/**
 * @brief: set lsc param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_lsc:  lsc param
 */
int ak_isp_vp_set_lsc_attr(void *isp_struct,AK_ISP_LSC_ATTR *p_lsc);

/**
 * @brief: get lsc param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_lsc:  lsc param
 */
int ak_isp_vp_get_lsc_attr(void *isp_struct,AK_ISP_LSC_ATTR *p_lsc);

/**
 * @brief: set rgb gamma param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_rgb_gamma:  rgb gamma param
 */
int ak_isp_vp_set_mesh_lsc_attr(void *isp_struct,AK_ISP_MESH_LSC_ATTR *p_mesh_lsc);
/**
 * @brief: get mesh lsc param
 * @author: lz
 * @date: 2023-3-17
 * @param [out] *p_mesh_lsc: mesh  lsc param
 */
int ak_isp_vp_get_mesh_lsc_attr( void *isp_struct,AK_ISP_MESH_LSC_ATTR *p_mesh_lsc);


/**
 * @brief: set raw hist param
 * @author: lizhi
 * @date: 2023-03-17
 * @param [in] *p_raw_gstat:raw hist param
 */
int ak_isp_vp_set_raw_g_hist_attr(void *isp_struct,AK_ISP_RAW_G_HIST_ATTR *raw_g_hist_para);
/**
 * @brief: get raw hist  param
 * @author: lizhi
 * @date: 2023-3-17
 * @param [in] *p_raw_hist: raw hist param
 */
int ak_isp_vp_get_raw_g_hist_attr(void *isp_struct,AK_ISP_RAW_G_HIST_ATTR *p_raw_hist);

/**
 * @brief: set drc param
 * @author: lz
 * @date: 2023-3-17
 * @param [in] *p_drc:drc param
 */
int ak_isp_vp_set_drc_attr(void *isp_struct,AK_ISP_DRC_ATTR *p_drc);
/**
 * @brief: get drc param
 * @author: lz
 * @date: 2023-3-17
 * @param [out] *p_drc:drc param
 */
int ak_isp_vp_get_drc_attr( void *isp_struct,AK_ISP_DRC_ATTR *p_drc);

/**
 * @brief: set rgb gamma param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_rgb_gamma:  rgb gamma param
 */
int ak_isp_vp_set_rgb_gamma_attr(void *isp_struct,AK_ISP_RGB_GAMMA_ATTR *p_rgb_gamma);

/**
 * @brief: get rgb gamma param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_rgb_gamma:  rgb gamma param
 */
int ak_isp_vp_get_rgb_gamma_attr(void *isp_struct,AK_ISP_RGB_GAMMA_ATTR *p_rgb_gamma);

/**
 * @brief: set y gamma param
 * @author: lz
 * @date: 2016-8-26
 * @param [in] *p_y_gamma:  y gamma param
 */
/**
 * @brief: set rgb hist param
 * @author: lizhi
 * @date: 2023-03-17
 * @param [in] *p_rgb_hist:rgb hist param
 */
int ak_isp_vp_set_rgb_hist_attr(void *isp_struct,AK_ISP_RGB_HIST_ATTR *rgb_hist_para);
/**
 * @brief: get rgb hist  param
 * @author: lizhi
 * @date: 2023-3-17
 * @param [in] *p_rgb_hist: rgb hist param
 */
int ak_isp_vp_get_rgb_hist_attr(void *isp_struct,AK_ISP_RGB_HIST_ATTR *p_rgb_hist);
/**
 * @brief: set yuv hist param
 * @author: lizhi
 * @date: 2023-03-17
 * @param [in] *p_yuv_hist:yuv hist param
 */
int ak_isp_vp_set_yuv_hist_attr(void *isp_struct,AK_ISP_YUV_HIST_ATTR *yuv_hist_para);
/**
 * @brief: get yuv hist  param
 * @author: lizhi
 * @date: 2023-3-17
 * @param [in] *p_yuv_hist: yuv hist param
 */
int ak_isp_vp_get_yuv_hist_attr(void *isp_struct,AK_ISP_YUV_HIST_ATTR *p_yuv_hist);

/**
 * @brief: set y gamma param
 * @author: lz
 * @date: 2016-8-26
 * @param [in] *p_y_gamma:  y gamma param
 */
int ak_isp_vp_set_y_gamma_attr(void *isp_struct,AK_ISP_Y_GAMMA_ATTR *p_y_gamma);

/**
 * @brief: get y gamma param
 * @author: lz
 * @date: 2016-8-26
 * @param [out] *p_y_gamma:  y gamma param
 */
int ak_isp_vp_get_y_gamma_attr(void *isp_struct,AK_ISP_Y_GAMMA_ATTR *p_y_gamma);

/**
 * @brief: set raw gamma param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_raw_lut:  raw gamma param
 */
int ak_isp_vp_set_raw_lut_attr(void *isp_struct,AK_ISP_RAW_LUT_ATTR *p_raw_lut);

/**
 * @brief: get raw gamma param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_raw_lut:  raw gamma param
 */
int ak_isp_vp_get_raw_lut_attr(void *isp_struct,AK_ISP_RAW_LUT_ATTR *p_raw_lut);

/**
 * @brief: set dpc param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_dpc:  dpc param
 */
int ak_isp_vp_set_dpc_attr(void *isp_struct,AK_ISP_DDPC_ATTR *p_dpc);

/**
 * @brief: get dpc param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_dpc:  dpc param
 */
int ak_isp_vp_get_dpc_attr(void *isp_struct,AK_ISP_DDPC_ATTR *p_dpc);


/**
 * @brief: set raw noise remove param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *nr1:  raw noise remove  param
 */
int ak_isp_vp_set_nr1_attr(void *isp_struct,AK_ISP_NR1_ATTR *p_nr1);

/**
 * @brief: get raw noise remove param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *nr1:  raw noise remove  param
 */
int ak_isp_vp_get_nr1_attr(void *isp_struct,AK_ISP_NR1_ATTR *p_nr1);

/**
 * @brief: set green balance param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_gb:  green balance param
 */
int ak_isp_vp_set_gb_attr(void *isp_struct,AK_ISP_GB_ATTR *p_gb);

/**
 * @brief: get green balance param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_gb:  green balance param
 */
int ak_isp_vp_get_gb_attr(void *isp_struct,AK_ISP_GB_ATTR *p_gb);

/**
 * @brief: set demosaic param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_demo:  demosaic param
 */
int ak_isp_vp_set_demo_attr(void *isp_struct,AK_ISP_DEMO_ATTR *p_demo);

/**
 * @brief: get demosaic param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_demo:  demosaic param
 */
int ak_isp_vp_get_demo_attr(void *isp_struct,AK_ISP_DEMO_ATTR *p_demo);

/**
 * @brief: set color correct param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_ccm:  color correct param
 */
int ak_isp_vp_set_ccm_attr(void *isp_struct,AK_ISP_CCM_ATTR *p_ccm);

/**
 * @brief: set color correct param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_ccm:  color correct param
 */
int ak_isp_vp_get_ccm_attr(void *isp_struct,AK_ISP_CCM_ATTR *p_ccm);

/**
 * @brief: set wdr  param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_wdr: wdr param
 */
int ak_isp_vp_set_hdr_attr(void *isp_struct,AK_ISP_HDR_ATTR *p_hdr);

/**
 * @brief: get wdr  param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_wdr: wdr param
 */
int ak_isp_vp_get_hdr_attr(void *isp_struct,AK_ISP_HDR_ATTR *p_hdr);

/**
 * @brief: set lce  param
 * @author: lizhi
 * @date: 2021-03-01
 * @param [in] *p_lce: lce param
 */

int ak_isp_vp_set_lce_attr(void *isp_struct,AK_ISP_LCE_ATTR *p_lce);

/**
 * @brief: get lce  param
 * @author: lizhi
 * @date: 2021-03-01
 * @param [out] *p_lce: lce param
 */
int ak_isp_vp_get_lce_attr(void *isp_struct,AK_ISP_LCE_ATTR *p_lce);


/**
 * @brief: set yuv noise remove param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_nr2: noise remove param
 */
int ak_isp_vp_set_nr2_attr(void *isp_struct,AK_ISP_NR2_ATTR *p_nr2);

/**
 * @brief: get yuv noise remove param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_nr2: noise remove param
 */
int ak_isp_vp_get_nr2_attr(void *isp_struct,AK_ISP_NR2_ATTR *p_nr2);

/**
 * @brief: set yuv noise remove param
 * @author: wyf
 * @date: 2020-1-06
 * @param [in] *p_uvnr: noise remove param
 */
int ak_isp_vp_set_uvnr_attr(void *isp_struct,AK_ISP_UVNR_ATTR *p_uvnr);

/**
 * @brief: get yuv noise remove param
 * @author: wyf
 * @date: 2020-1-06
 * @param [out] *p_uvnr: noise remove param
 */
int ak_isp_vp_get_uvnr_attr(void *isp_struct,AK_ISP_UVNR_ATTR *p_uvnr);

/**
 * @brief: set 3d noise remove param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_3d_nr: 3d noise remove param
 */
int ak_isp_vp_set_3d_nr_attr(void *isp_struct,AK_ISP_3D_NR_ATTR *p_3d_nr);

/**
 * @brief: get 3d noise remove param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_3d_nr: 3d noise remove param
 */
int ak_isp_vp_get_3d_nr_attr(void *isp_struct,AK_ISP_3D_NR_ATTR *p_3d_nr);

/**
 * @brief: set 3d noise remove reference param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_ref: 3d noise remove referenc param
 */
int ak_isp_vp_set_3d_nr_ref_addr(void *isp_struct,AK_ISP_3D_NR_REF_ATTR *p_ref);

/**
 * @brief: get 3d noise remove reference param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_ref: 3d noise remove referenc param
 */
int ak_isp_vp_get_3d_nr_ref_addr(void *isp_struct,AK_ISP_3D_NR_REF_ATTR *p_ref);

/**
 * @brief: get 3d noise remove statics param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_3d_nr_stat_info: 3d noise remove statics param
 */
int ak_isp_vp_get_3d_nr_stat_info(void *isp_struct,AK_ISP_3D_NR_STAT_INFO * p_3d_nr_stat_info);

/**
 * @brief: get 3d noise reference frame total Bytes 
 * @author: lizhi
 * @date: 2021-6-16
 * @param [out] *p_3d_nr_stat_info: 3d noise reference frame total Bytes 
 */
int ak_isp_vp_get_3d_nr_ref_size_stat_info(void *isp_struct,
    AK_ISP_3D_NR_REF_SIZE_STAT_INFO * p_3d_nr_ref_size_stat_info);


/**
 * @brief: set sharp param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_sharp: sharp param
 */
int ak_isp_vp_set_sharp_attr(void *isp_struct,AK_ISP_SHARP_ATTR *p_sharp);

/**
 * @brief: get sharp param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_sharp: sharp param
 */
int ak_isp_vp_get_sharp_attr(void *isp_struct,AK_ISP_SHARP_ATTR* p_sharp);

/**
 * @brief: set sharp other param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_sharp_ex: sharp other param
 */
int ak_isp_vp_set_sharp_ex_attr(void *isp_struct,AK_ISP_SHARP_EX_ATTR *p_sharp_ex);

/**
 * @brief: get sharp other param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_sharp_ex: sharp other param
 */
int ak_isp_vp_get_sharp_ex_attr(void *isp_struct,AK_ISP_SHARP_EX_ATTR* p_sharp_ex);

/**
 * @brief: set false color param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *fcs: false color param
 */
int ak_isp_vp_set_fcs_attr(void *isp_struct,AK_ISP_FCS_ATTR *p_fcs);

/**
 * @brief: get false color param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *fcs: false color param
 */
int ak_isp_vp_get_fcs_attr(void *isp_struct,AK_ISP_FCS_ATTR *p_fcs);

/**
 * @brief: set hue param
 * @author: lz
 * @date: 2016-8-26
 * @param [in] *p_hue:hue param
 */
int ak_isp_vp_set_hue_attr(void *isp_struct,AK_ISP_HUE_ATTR *p_hue);

/**
 * @brief: gethue param
 * @author: lz
 * @date: 2016-8-26
 * @param [in] *p_hue:hue param
 */
int ak_isp_vp_get_hue_attr(void *isp_struct,AK_ISP_HUE_ATTR *p_hue);

/**
 * @brief: set satruration param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_sat: satruration param
 */
int ak_isp_vp_set_saturation_attr(void *isp_struct,AK_ISP_SATURATION_ATTR *p_sat);

/**
 * @brief: get satruration param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_sat: satruration param
 */
int ak_isp_vp_get_saturation_attr(void *isp_struct,AK_ISP_SATURATION_ATTR *p_sat);

/**
 * @brief: set contrast attr
 * @author: 
 * @date: 
 * @param [in] *p_contrast: contrast param
 */
int ak_isp_vp_set_contrast_attr(void *isp_struct,AK_ISP_CONTRAST_ATTR  *p_contrast);

/**
 * @brief: get contrast attr
 * @author: 
 * @date: 
 * @param [out] *p_contrast: contrast param
 */
int ak_isp_vp_get_contrast_attr(void *isp_struct,AK_ISP_CONTRAST_ATTR  *p_contrast);

/**
 * @brief: set rgb2yuv attr
 * @author: 
 * @date: 
 * @param [in] *p_rgb2yuv_attr: rgb2yuv param
 */
int ak_isp_vp_set_rgb2yuv_attr(void *isp_struct,AK_ISP_RGB2YUV_ATTR*p_rgb2yuv_attr);

/**
 * @brief: get rgb2yuv attr
 * @author: 
 * @date: 
 * @param [out] *p_rgb2yuv_attr: rgb2yuv param
 */
int ak_isp_vp_get_rgb2yuv_attr(void *isp_struct,AK_ISP_RGB2YUV_ATTR*p_rgb2yuv_attr);

/**
 * @brief: set effect attr
 * @author: 
 * @date: 
 * @param [in] *p_effect_attr: effect param
 */
int ak_isp_vp_set_effect_attr(void *isp_struct,AK_ISP_EFFECT_ATTR *p_effect_attr);

/**
 * @brief: get effect attr
 * @author: 
 * @date: 
 * @param [out] *p_effect_attr: effect param
 */
int ak_isp_vp_get_effect_attr(void *isp_struct,AK_ISP_EFFECT_ATTR *p_effect_attr);

/**
 * @brief: set auto focus param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_af:  af  param
 */
int ak_isp_vp_set_af_attr(void *isp_struct,AK_ISP_AF_ATTR *p_af);

/**
 * @brief: get auto focus param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_af:  af  param
 */
int ak_isp_vp_get_af_attr(void *isp_struct,AK_ISP_AF_ATTR *p_af);

/**
 * @brief: get auto focus statics info param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_af_stat_info:  af_stat  param
 */
int ak_isp_vp_get_af_stat_info(void *isp_struct,AK_ISP_AF_STAT_INFO *p_af_stat_info);



//------------------------AW---------------------------------------------
/**
 * @brief: set white balance  param
 * @author: lizhi
 * @date: 2021-06-16
 * @param [in] *p_type:  white balance  param
 */
int ak_isp_vp_set_wb_attr(void *isp_struct,AK_ISP_WB_ATTR *p_wb_attr);

/**
 * @brief: get white balance  param
 * @author: lizhi
 * @date: 2021-06-16
 * @param [out] *p_type:  white balance  param
 */
int ak_isp_vp_get_wb_attr(void *isp_struct,AK_ISP_WB_ATTR *p_wb_attr);

/**
 * @brief: set auto white balance ex attr
 * @author: wyf
 * @date: 2016-5-16
 * @param [in] *p_awb:  auto white balance  param
 */
int ak_isp_vp_set_awb_ex_attr(void *isp_struct,AK_ISP_AWB_EX_ATTR *p_awb);

/**
 * @brief: get auto white balance ex attr
 * @author: wyf
 * @date: 2016-5-16
 * @param [in] *p_awb:  auto white balance  param
 */
int ak_isp_vp_get_awb_ex_attr(void *isp_struct,AK_ISP_AWB_EX_ATTR *p_awb);

/**
 * @brief: set auto white balance calib attr
 * @author: lizhi
 * @date: 2021-06-16
 * @param [in] *p_awb_calib_attr:  auto white calib  param
 */
int ak_isp_vp_set_awb_calib_attr(void *isp_struct,AK_ISP_AWB_CALIB_INFO *p_awb_calib_attr);

/**
 * @brief: get auto white balance calib attr
 * @author: lizhi
 * @date: 2021-06-16
 * @param [out] *p_awb_calib_attr:  auto white calib  param
 */
int ak_isp_vp_get_awb_calib_attr(void *isp_struct,AK_ISP_AWB_CALIB_INFO *p_awb_calib_attr);


/**
 * @brief: get awb statics info
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_awb_stat_info:  awb statics info  param
 */
int ak_isp_vp_get_awb_stat_info(void *isp_struct,AK_ISP_AWB_STAT_INFO *p_awb_stat_info);

/**
 * @brief: get awb run info
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_awb_run_info:  awb run info
 */
int ak_isp_vp_get_awb_run_info(void *isp_struct,AK_ISP_AWB_RUN_INFO *p_awb_run_info);






//----------------------------------AE--------------------------------------------
/**
 * @brief: set wdr ae param
 * @author: 
 * @date: 
 * @param [in] *p_wdr_ae_attr: wdr exposure param
 */
int  ak_isp_vp_set_wdr_ae_attr(void *isp_struct,AK_ISP_WDR_AE_ATTR *p_wdr_ae_attr);

/**
 * @brief: get wdr ae param
 * @author: 
 * @date: 
 * @param [out] *p_wdr_ae_attr: wdr exposure param
 */
int  ak_isp_vp_get_wdr_ae_attr(void *isp_struct,AK_ISP_WDR_AE_ATTR *p_wdr_ae_attr);

/**
 * @brief: set exposure param
 * @author: lizhi
 * @date: 2021-06-16
 * @param [in] *p_exposure_attr:  auto exposure  param
 */
int ak_isp_vp_set_exposure_attr( void *isp_struct,AK_ISP_EXPOSURE_ATTR* p_exposure_attr);

/**
 * @brief: get auto exposure  param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_exposure_attr:  auto exposure  param
 */
int ak_isp_vp_get_exposure_attr(void *isp_struct,AK_ISP_EXPOSURE_ATTR *p_exposure_attr);

/**
 * @brief: set ex zone weight attribute
 * @author: wyf
 * @date: 2021-12-21
 * @param [in] *p_ex_zweight:  zone weight attibute 
 */
int  ak_isp_vp_set_ex_zweight_attr(void *isp_struct,AK_ISP_EX_ZONE_WEIGHT_ATTR* p_ex_zweight);
/**
 * @brief: get ex zone weight attribute
 * @author: wyf
 * @date: 2021-12-31
 * @param [in] *p_ex_zweight:  zone weight attibute  
 */
int  ak_isp_vp_get_ex_zweight_attr(void *isp_struct,AK_ISP_EX_ZONE_WEIGHT_ATTR* p_ex_zweight);
/**
 * @brief: set ae route param
 * @author: lizhi
 * @date: 2021-06-16
 * @param [in] *p_ae_route_attr:  auto exposure route param
 */
//int ak_isp_vp_set_ae_route_attr( void *isp_struct,AK_ISP_AE_ROUTE_ATTR* p_ae_route_attr);

/**
 * @brief: set ae route param
 * @author: lizhi
 * @date: 2021-06-16
 * @param [out] *p_ae_route_attr:  auto exposure route param
 */
//int ak_isp_vp_get_ae_route_attr(void *isp_struct,AK_ISP_AE_ROUTE_ATTR* p_ae_route_attr);

/**
 * @brief: set frame rate param
 * @author: lz
 * @date: 2016-8-29
 * @param [in] *p_frame_rate:  frame rate param
 */
int ak_isp_vp_set_frame_rate(void *isp_struct, AK_ISP_FRAME_RATE_ATTR*p_frame_rate);

/**
 * @brief: set frame rate param
 * @author: lz
 * @date: 2016-8-29
 * @param [in] *p_frame_rate:  frame rate param
 */
int ak_isp_vp_get_frame_rate(void *isp_struct,AK_ISP_FRAME_RATE_ATTR*p_frame_rate);

/**
 * @brief: get auto  exposure  running info
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_ae_stat:  auto  exposure  running info  param
 */
int ak_isp_vp_get_ae_run_info(void *isp_struct,AK_ISP_AE_RUN_INFO*p_ae_run_info);

/**
 * @brief: get ae stat running info
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_ae_stat_info: auto exposure stat info param
 */
int ak_isp_vp_get_ae_stat_info(void *isp_struct,AK_ISP_AE_STAT_INFO*p_ae_stat_info);

/**
 * @brief: get hdr weight
 * @author: 
 * @date: 
 * @param [out] 
 */
int _get_hdr_weight(void *isp_struct);

/* Pclk_Polar not isp function, but described in PG */
AK_ISP_PCLK_POLAR ak_isp_get_pclk_polar(void *isp_struct);

/**
 * @brief: get bits width
 * @author: 
 * @date: 
 * @param [out] 
 */
int ak_isp_get_bits_width(void *isp_struct);

/**
 * @brief: set isp capturing
 * @author: 
 * @date: 
 * @param [in] resume
 */
int ak_isp_set_isp_capturing(void *isp_struct,int resume);

/**
 * @brief: update isp irq
 */
int ak_isp_irq_work(void *isp_struct);

/**
 * @brief: update isp ae
 */
int ak_isp_ae_work(void *isp_struct);

/**
 * @brief: update isp ae rt
 */
int ak_isp_ae_work_rt(void *isp_struct);

/**
 * @brief: update isp awb
 */
int ak_isp_awb_work(void *isp_struct);

/**
 * @brief: isp fast ae work
 */
int ak_isp_fast_ae_work(void *isp_struct,const int (*p)[2]);

/**
 * @brief: isp fast awb work
 */
int ak_isp_fast_awb_work(void *isp_struct);

/**
 * @brief: set awb work suspend
 * @author: 
 * @date: 
 * @param [in] ae_suspend_flag
 */
int ak_isp_set_awb_work_suspend(void *isp_struct, int ae_suspend_flag);


void *isp_struct2_module_switch_sensor(void *isp);

/**
 * @brief: isp2 module curr sensor
 */
int isp2_module_curr_sensor(void);

/**
 * @brief: isp2 module init
 */
int isp2_module_init(AK_ISP_FUNC_CB *cb, struct sensor_cb_info *sensor_cb,
        void *isp_base, void *pp_base, void *glue_base, void **isp_struct, int isp_id);

/**
 * @brief: isp2 module fini
 */
void isp2_module_fini(void *isp_struct);

/**
 * @brief: isp2 print reg table
 */
void  isp2_print_reg_table(void *isp_struct);

/**
 * @brief: get mem ptrs
 * @author: 
 * @date: 
 * @param [out] *ptrs :mem ptrs
 */
void ak_isp_get_mem_ptrs(void *isp_struct, struct mem_ptrs *ptrs);

/**
 * @brief: isp register sensor
 */
int ak_isp_register_sensor(void *sensor_info);

/**
 * @brief: isp get sensor
 */
void *ak_isp_get_sensor(int *index);

/**
 * @brief: isp remove all sensors
 */
void ak_isp_remove_all_sensors(void);

/**
 * @brief: isp set td
 */
int ak_isp_set_td(void *isp_struct);

/**
 * @brief: isp reload td
 */
int ak_isp_reload_td(void *isp_struct);

/**
 * @brief: isp sensor cb init
 */
int ak_isp_sensor_cb_init(void *isp_struct, AK_ISP_SENSOR_INIT_PARA *sensor_para);

/**
 * @brief: isp get scene
 */
enum scene ak_isp_get_scene(void *isp_struct);

/**
 * @brief: isp vpp mainchn osdmem useok
 */
int ak_isp_vpp_mainchn_osdmem_useok(void *isp_struct);

/**
 * @brief: isp vpp subchn osdmem useok
 */
int ak_isp_vpp_subchn_osdmem_useok(void *isp_struct);

/**
 * @brief: isp get mdinfo
 */
int ak_isp_get_mdinfo(void *isp_struct, AK_ISP_3D_NR_STAT_INFO **_3d_nr_stat_para,
                int *width_block_num, int *height_block_num, int *block_size);

/**
 * @brief: isp get md array max size
 */
int ak_isp_get_md_array_max_size(void *isp_struct,
                int *width_block_num, int *height_block_num, int *block_size);

/**
 * @brief: dual 3dnr work
 */
int _dual_3dnr_work(void *isp_struct);

/**
 * @brief: isp set ae fast struct default
 */
int ak_isp_set_ae_fast_struct_default(void *isp_struct, struct ae_fast_struct *ae_fast);

/**
 * @brief: isp set ae fast struct
 */
int ak_isp_set_ae_fast_struct(void *isp_struct, struct ae_fast_struct *ae_fast);

/**
 * @brief: isp set ae fast set ae
 */
int ak_isp_ae_fast_set_ae(void *isp_struct);

/**
 * @brief: isp ae fast set isp d gain
 */
int ak_isp_ae_fast_set_isp_d_gain(void *isp_struct);

/**
 * @brief: isp ae fast set wb
 */
int ak_isp_ae_fast_set_wb(void *isp_struct);

/**
 * @brief: isp set ae work suspend
 */
int ak_isp_set_ae_work_suspend(void *isp_struct, int ae_suspend_flag);

/**
 * @brief: isp vi start capturing on
 */
int ak_isp_vi_start_capturing_one(void *isp_struct, int wait);

/**
 * @brief: isp get version
 */
const char *ak_isp_get_version(void *isp_struct);

/*int ak_isp_get_current_block_done(void *isp_struct, enum pp_channel chn, int *slice, int *hwbuf);
int ak_isp_set_dvp_attr(void *isp_struct, struct dvp_attr *attr);
int ak_isp_get_dvp_attr(void *isp_struct, struct dvp_attr *attr);
int ak_isp_set_mipi_attr(void *isp_struct, struct mipi_attr *attr);
int ak_isp_get_mipi_attr(void *isp_struct, struct mipi_attr *attr);
int ak_isp_set_dvp_mipi_common_attr(void *isp_struct, struct dvp_mipi_common_attr *attr);
int ak_isp_get_dvp_mipi_common_attr(void *isp_struct, struct dvp_mipi_common_attr *attr);*/

/**
 * @brief: isp vo update regtable mode all
 */
int ak_isp_vo_update_regtable_mode_all(void *isp_struct);
#endif


typedef struct ak_isp_ae_info
{
    int  a_gain;                        //模拟增益的值
    int  d_gain;                        //数字增益的值
    int  isp_d_gain;                    //isp数字增益的值
    int  exp_time;                  //曝光时间的值
}AK_ISP_AE_INFO;


typedef void (*ISPDRV_CB_PRINTK)(char * format, ...);
typedef void (*ISPDRV_CB_MEMCPY)(void *dst, void *src, unsigned long sz);
typedef void (*ISPDRV_CB_MEMSET)(void *ptr, unsigned char value, unsigned long sz);
typedef void* (*ISPDRV_CB_MALLOC)(unsigned long sz);
typedef void (*ISPDRV_CB_FREE)(void *ptr);
typedef void* (*ISPDRV_CB_DMAMALLOC)(unsigned long sz, void *handle);
typedef void (*ISPDRV_CB_DMAFREE)(void *ptr, unsigned long sz, unsigned long handle);
typedef void (*ISPDRV_CB_MSLEEP)(int ms);
typedef void (*ISPDRV_CB_CACHE_INVALID)(void);
typedef unsigned long (*ISPDRV_CB_WORD_READ)(void *addr);
typedef void (*ISPDRV_CB_WORD_WRITE)(unsigned long value, void *addr);
typedef unsigned long (*ISPDRV_CB_DWORD_READ)(void *addr);
typedef void (*ISPDRV_CB_DWORD_WRITE)(unsigned long value, void *addr);

typedef struct {
    ISPDRV_CB_PRINTK cb_printk;
    ISPDRV_CB_MEMCPY cb_memcpy;
    ISPDRV_CB_MEMSET cb_memset;
    ISPDRV_CB_MALLOC cb_malloc;
    ISPDRV_CB_FREE   cb_free;
    ISPDRV_CB_DMAMALLOC cb_dmamalloc;
    ISPDRV_CB_DMAFREE cb_dmafree;
    ISPDRV_CB_MSLEEP cb_msleep;
    ISPDRV_CB_CACHE_INVALID cb_cache_invalid;
    ISPDRV_CB_WORD_READ cb_word_read;
    ISPDRV_CB_WORD_WRITE cb_word_write;
    ISPDRV_CB_DWORD_READ cb_dword_read;
    ISPDRV_CB_DWORD_WRITE cb_dword_write;
}AK_ISP_FUNC_CB;

typedef struct sensor_reg_info {
    unsigned short reg_addr;
    unsigned short value;
}AK_ISP_SENSOR_REG_INFO;

typedef struct sensor_init_para {
    unsigned short num;
    AK_ISP_SENSOR_REG_INFO *reg_info;
}AK_ISP_SENSOR_INIT_PARA;


typedef struct sensor_callback {
    int (*sensor_init_func)(void *arg, const AK_ISP_SENSOR_INIT_PARA *para);
    int (*sensor_read_reg_func)(void *arg, const int reg_addr);
    int (*sensor_write_reg_func)(void *arg, const int reg_addr, int value);
    int (*sensor_read_id_func)(void *arg);  //no use IIC bus
    int (*sensor_update_a_gain_func)(void *arg, const unsigned int a_gain);
    int (*sensor_update_d_gain_func)(void *arg, const unsigned int d_gain);
    int (*sensor_updata_exp_time_func)(void *arg, unsigned int exp_time);
    //int (*sensor_updata_exp_time_short_func)(void* arg, unsigned int exp_time);
    int (*sensor_timer_func)(void *arg);
    int (*sensor_set_standby_in_func)(void *arg);
    int (*sensor_set_standby_out_func)(void *arg);

    int (*sensor_probe_id_func)(void *arg); //use IIC bus
    int (*sensor_get_resolution_func)(void *arg, int *width, int *height);
    int (*sensor_get_mclk_func)(void *arg);
    int (*sensor_get_fps_func)(void *arg);
    int (*sensor_get_valid_coordinate_func)(void *arg, int *x, int *y);
    enum sensor_bus_type (*sensor_get_bus_type_func)(void *arg);
    int (*sensor_get_parameter_func)(void *arg, int param, void *value);

    int (*sensor_set_power_on_func)(void *arg);
    int (*sensor_set_power_off_func)(void *arg);
    int (*sensor_set_fps_func)(void *arg, const int fps);
    //int (*sensor_set_standby_in_func)(const int pwdn_pin, const int reset_pin);
    //int (*sensor_set_standby_out_func)(const int pwdn_pin, const int reset_pin);
    int (*sensor_get_fast_para_func)(void *arg, AK_ISP_AE_INFO *para);
    /*ONLY used for FastAOV*/
    int (*sensor_event_func)(void *arg, int evt);
    int (*sensor_fsync_func)(void *arg);
    int (*sensor_get_current_rb_rows_func)(void* arg);
    int (*sensor_get_aov_flag_func)(void* arg);
}AK_ISP_SENSOR_CB;

struct sensor_cb_info {
    AK_ISP_SENSOR_CB *cb;
    void *arg;
};


enum sensor_bus_type {
    BUS_TYPE_RAW,
    BUS_TYPE_YUV,
    BUS_TYPE_NUM
};

enum scene {
    SCENE_INDOOR = 0,
    SCENE_OUTDOOR
};





/*****************************PP***********************************/
/*****************************PP***********************************/
#include "ak_pp.h"

#endif
