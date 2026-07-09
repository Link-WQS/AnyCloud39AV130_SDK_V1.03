#ifndef _AK_ISP_DRV_H_
#define _AK_ISP_DRV_H_
#define ISP_DRV_LIB_VER 	"isp_drv_lib-V0.2.47"


#define DISABLE    0
#define ENABLE     1
#define BUFFER_NUM 4 /*每个输出通道最大buffer数*/

enum pp_channel {
    PP_MAIN_CHAN = 0,
    PP_SUB_CHAN,
    PP_CH3_CHAN,
    PP_CHAN_NUM,
};

#if 1
enum ak_yuv420_type {
    YUV420_PLANAR = 0,
    YUV420_SEMI_PLANAR,
};
#endif

enum buffer_id {
    BUFFER_ONE,
    BUFFER_TWO,
    BUFFER_THREE,
    BUFFER_FOUR,
};

enum data_format {
    BAYER_RAW_DATA = 0,
    YUV422_DATA,
    JPEG_COMPRESS_DATA,
};

typedef enum ak_isp_op_type {
    OP_TYPE_MANUAL = 0,
    OP_TYPE_AUTO = 1,
    OP_TYPE_BUTT,
} AK_ISP_OP_TYPE;

typedef enum ak_isp_rgb2yuv_model {
    BT601_MODEL = 0,
    BT709_MODEL = 1,
} AK_ISP_RGB2YUV_MODEL;

enum yuv422_interface {
    YUV422_ITF_BT601 = 0,
    YUV422_ITF_BT656,
    YUV422_ITF_BT1120,
}; /*bt601/656/1120*/

enum yuv422_bt656_1120_frame_start {
    YUV422_BT656_1120_FS_FALLING = 0, /*下降沿*/
    YUV422_BT656_1120_FS_RISING, /*上升沿*/
    YUV422_BT656_1120_FS_DEDGE, /*双边沿*/
}; /*下降沿、上升沿、边沿(隔行时)*/

enum chn_frame_ctrl {
    CHN_FCTRL_CLOSE = 0,
    CHN_FCTRL_SINGE_CAPTURE,
    CHN_FCTRL_CONTINUE_CAPTURE,
}; /*关闭通道，单帧采集，循环采集*/

enum src_data_format {
    DATA_FMT_RAW = 0,
    DATA_FMT_YUV422,
    DATA_FMT_BYTE_STREAM,
    DATA_FMT_YUV420, /*only mipi*/
};

enum src_type {
    SRC_TYPE_DATA_STREAM = 0,
    SRC_TYPE_YUV420,
    SRC_TYPE_YUV422,
};

enum raw_seq {
    RAW_SEQ_R = 0,
    RAW_SEQ_B,
    RAW_SEQ_Gr,
    RAW_SEQ_Gb,
};



enum chn_output_format {
    /*YUV422*/
    CHN_OUTPUT_FMT_IYUV = 0, /*y..u..v..*/
    CHN_OUTPUT_FMT_YV12, /*y..v..u..*/
    CHN_OUTPUT_FMT_NV12, /*y..uv..*/
    CHN_OUTPUT_FMT_NV21, /*y..vu..*/

    /*YUV422*/
    CHN_OUTPUT_FMT_YV16, /*y..v..u..*/
    CHN_OUTPUT_FMT_NV16, /*y..uv..*/
    CHN_OUTPUT_FMT_NV61, /*y..vu..*/

    /*YUV444*/
    CHN_OUTPUT_FMT_YV24, /*y..v..u..*/
    CHN_OUTPUT_FMT_NV24, /*y..uv..*/
    CHN_OUTPUT_FMT_NV42, /*y..vu..*/
    CHN_OUTPUT_FMT_YVU444, /*y..v..u..*/
    CHN_OUTPUT_FMT_YUV444, /*y..u..v..*/
    CHN_OUTPUT_FMT_VYU444, /*v..y..u..*/

    /*RGB888*/
    CHN_OUTPUT_FMT_RGB888, /*r..g..b..*/
    CHN_OUTPUT_FMT_BGR888_PACKED, /*bgr..*/
    CHN_OUTPUT_FMT_RGB888_PACKED, /*rgb..*/
    CHN_OUTPUT_FMT_RGB888_LINE, /*r0-0..r0-n
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

enum margin_format_ctrl{
    MARGIN_FORMAT_CTRL_NORMAL = 0,
    MARGIN_FORMAT_CTRL_DARK  = 1,
    MARGIN_FORMAT_CTRL_COPY  = 2,
};

struct dvp_attr {
    signed char vsync_pol; /*0-low active,  1-high active*/
    signed char href_pol; /*0-low active,  1-high active*/
    // isp_upload_stream_sel_cfg，配置为dvp或mipi_attr后自动选定一种作为 data
    // stream的数据源头。
    int one_line_cycle; /*16bits*/
    signed char frame_start_edge; /*0-vsync falling is frame starting, 1-vsync
                                     rising is frame starting*/
    signed char frame_start_delay_enable; /*0-disable, 1-enable*/
    int hblank_cycle; /*16bits*/
    signed char frame_start_delay_2line_num; /*6bits*/
    signed char hsync_adjust_enable; /*0-disable, 1-enable*/
    signed char vsync_filter_enable; /*0-disable, 1-enable*/

    enum yuv422_interface itf; /*bt601/656/1120*/
    enum yuv422_bt656_1120_frame_start fstart; /*下降沿、上升沿、边沿(隔行时)*/
    signed char
        yuv422_bt656_1120_capture_odd_frame_enable; /*0-disable, 1-enable*/
    signed char
        yuv422_bt656_1120_capture_even_frame_enable; /*0-disable, 1-enable*/
    signed char yuv422_sdr_ddr_sel; /*0-sdr单沿，1-ddr双沿*/
    signed char yuv422_error_correct_enable;
        /*0-disable, 1-enalbe*/ /*纠正F/V/H Code*/
    signed char yuv422_ddr_data_start;
        /*0-posedge, 1-negedge*/ /*双沿时起始位置为上升沿还是下降沿*/
};

struct mipi_attr {
    signed char packed_or_pixel_mode; /*0-packed, 1-pixel*/
    // isp_upload_stream_sel_cfg，配置为dvp或mipi_attr后自动选定一种作为 data
    // stream的数据源头。
    signed char virtual_channel_id; /*4bits*/
    signed char bypass_virtual_channel_enable; /*0-disable, 1-enable*/
    signed char hsync_start_sel; /*0-CSI Controller输入的hsync_start，1-CSI
                                    Controller输入的信号生成hsync_start*/
    signed char hsync_end_sel; /*0-CSI Controller输入的hsync_end，1-CSI
                                  Controller输入的信号生成hsync_end*/
    signed char decoder_di_enable; /*0-ISP配置的数据格式和位宽参数，1-CSI2控制器传递的Data
                                      Type进行解码*/
};

struct dvp_mipi_common_attr {
    signed char data_width; /*8/10/12bits*/
    enum src_data_format data_format;
    enum raw_seq raw_seq; /*only raw validable*/
    signed char go_upload_enable; /*0-donot upload, 1-upload*/
    signed char go_yuv_enable;
        /*0-donot go yuv, 1-go yuv*/ /* only for input yuv */
    signed char video_mode; /*0-single mode, 1-continue mode*/
    signed char
        dual_sensor_sel; /*0-sensor0, 1-sensor1. only dual mode is validable*/
    /*
     * 有几个在top的寄存器：
     * csi_mipi_dvp_sel_cfg: dvp还是mipi接口
     * isp_pp_dual_sensor_mode_cfg: 单目还是双目
     * cis_pclk_pol: dvp时pclk极性
     */
    signed char test_pattern; /*0~8*/
    signed char test_pattern_enable; /*0-disable, 1-enable*/
    signed char uv_switch_enable; /*0-default, 1-switch*/
    signed char uv_sub128_enable; /*0-default, 1-sub128*/
    signed char line_end_cnt_enable; /*0-disable, 1-enable*/
    signed char line_end_cnt; /*6bits*/
};
#if 1   //del pp
struct pp_src_attr {
    // pp_src_frame_stamp,暂时没有发现需要写
    signed char src_frame_stamp_enable; /*0-disable, 1-enable*/
    signed char src_enable; /*源总开关*/
    enum src_type src_type; /*data stream, yuv420, yuv422*/
    signed char src_hflip_enable; /*0-disable, 1-enable*/
    signed char src_vflip_enable; /*0-disable, 1-enable*/
    signed char
        src_color_swing_yuv; /*0-full swing(0~255), 1-studio swing(16~223)*/
    signed char src_color_matrix; /*0-bt601, 1-bt709*/
    signed short src_frame_rate_x10; /*10bits*/ /*注意: 真实帧率*10倍*/
};

struct pp_chn_attr {
    signed char chn_buffer_disable[BUFFER_NUM];
    signed char chn_buffer_enable[BUFFER_NUM];
    signed short chn_slice_height; /*12bits*/
    /*
     * data_stream,
     * yuv420p: 1)y..u..v.., 2)v..u..y.., 3)v..y..v..,
     * yuv420sp: 1)y..uv.., 2)vu..y..,
     * yuv422,
     * yuv444,
     * rgb888,
     * luma
     */
    enum chn_output_format output_format;
    signed short y_stride;
    signed short u_stride;
    signed short v_stride;
    signed char chn_slice_enable; /*0-frame mode, 1-slice mode*/
    signed short chn_frame_rate_x10; /*10bits*/ /*真实帧率*10*/
    signed char chn_frame_skip_enable; /*0-disable, 1-enable*/
    signed char chn_buffer_disable_when_done_enable; /*0-disable, 1-enable*/
    enum chn_frame_ctrl chn_frame_ctrl; /*关闭通道，单帧采集，循环采集*/
};

typedef struct {
    unsigned int strX;
    unsigned int strY;
    unsigned int endX;
    unsigned int endY;
    unsigned int width;
    unsigned int height;
} AK_ISP_PP_WINDOW;

// Draw模块图层模式
typedef enum {
    PP_DRAW_OFF = 0, //关闭
    PP_DRAW_MOSAIC = 1, //马赛克
    PP_DRAW_RECTANGLE = 2, //矩形
    PP_DRAW_BOX = 3 //矩形框
} AK_ISP_PP_DRAW_MODE;

// OSD图层模式
typedef enum {
    PP_OSD_OFF = 0, //关闭
    PP_OSD_MONO = 1, //黑白
    PP_OSD_COLOR = 2, //索引
} AK_ISP_PP_OSD_MODE;

// OSD索引宽度
typedef enum {
    PP_OSD_1BIT = 0,
    PP_OSD_2BIT = 1,
    PP_OSD_4BIT = 2,
} AK_ISP_PP_OSD_SIZE;
#endif
struct input_data_format {
    enum data_format df;
    int data_width;
};

typedef struct ak_isp_blc {
    unsigned short black_level_enable; //使能位
    unsigned short bl_r_a; //[ 0,1023]
    unsigned short bl_gr_a; //[ 0,1023]
    unsigned short bl_gb_a; //[ 0,1023]
    unsigned short bl_b_a; //[ 0,1023]
    signed short bl_r_offset; //[-2048,2047]
    signed short bl_gr_offset; //[-2048,2047]
    signed short bl_gb_offset; //[-2048,2047]
    signed short bl_b_offset; //[-2048,2047]
} AK_ISP_BLC;

typedef struct ak_isp_blc_attr {
    AK_ISP_OP_TYPE blc_mode; // 0联动模式，1手动模式
    AK_ISP_BLC manual_blc;
    AK_ISP_BLC linkage_blc[16];
} AK_ISP_BLC_ATTR;

typedef struct {
    unsigned short coef_b[10]; //[0,255]
    unsigned short coef_c[10]; //[0,1023]
} lens_coef;

typedef struct ak_isp_lsc_attr {
    unsigned short enable;
    // the reference point of lens correction
    unsigned short xref; //[0,4096]
    unsigned short yref; //[0,4096]
    unsigned short lsc_shift; //[0，15]
    lens_coef lsc_r_coef;
    lens_coef lsc_gr_coef;
    lens_coef lsc_gb_coef;
    lens_coef lsc_b_coef;
    // the range of ten segment
    unsigned short range[10]; //[0，1023]
    unsigned short lsc_mode;
    unsigned char linkage_strength[17]; //[0，1023]
} AK_ISP_LSC_ATTR;

typedef struct ak_isp_raw_lut_attr {
    unsigned short raw_gamma_enable;
    unsigned short raw_rgain;
    unsigned short raw_ggain;
    unsigned short raw_bgain;
    unsigned short raw_rgain_rev;
    unsigned short raw_ggain_rev;
    unsigned short raw_bgain_rev; // 1
    unsigned short r_key[16];
    unsigned short g_key[16];
    unsigned short b_key[16];
    unsigned short raw_r[129]; // 10bit
    unsigned short raw_g[129]; // 10bit
    unsigned short raw_b[129]; // 10bit
} AK_ISP_RAW_LUT_ATTR;

typedef struct ak_isp_rgb_gamma {
    unsigned short r_gamma[129]; // 10bit
    unsigned short g_gamma[129]; // 10bit
    unsigned short b_gamma[129]; // 10bit
    unsigned short r_key[16];
    unsigned short g_key[16];
    unsigned short b_key[16];
    unsigned short rgb_gamma_enable;
} AK_ISP_RGB_GAMMA;

typedef struct ak_isp_rgb_gamma_attr {
    unsigned short gain_threshold;
    AK_ISP_RGB_GAMMA linkage_rgb_gamma[2];
} AK_ISP_RGB_GAMMA_ATTR;

typedef struct ak_isp_y_gamma_attr {
    unsigned short ygamma[129]; // 10bit
    unsigned short ygamma_key[16]; //曲线的关键点
    unsigned short ygamma_uv_adjust_enable;
    unsigned short ygamma_uv_adjust_level;
    unsigned short ygamma_cnoise_yth1; // Ygamma色差抑制门限值
    unsigned short ygamma_cnoise_yth2; // Ygamma色差抑制门限值
    unsigned short ygamma_cnoise_slop;
    unsigned short ygamma_cnoise_gain; // UV调整系数计算参数
} AK_ISP_Y_GAMMA_ATTR;

typedef struct ak_isp_lce {
    unsigned short lce_enable;
    unsigned short lce_uv_adjust_en;
    unsigned short lce_strength[4][8];
    unsigned short lce_hist_weight[8];
    unsigned short lce_uv_adjust_level;
    unsigned short lce_weight_k;
} AK_ISP_LCE;

typedef struct ak_isp_lce_ex_attr {
    unsigned short lce_blkcnt_rev; // 1/(blkH*blkW)
    unsigned short lce_blk_pixel_cnt8; // blkW*blkH/8    16bits
} AK_ISP_LCE_EX_ATTR;

typedef struct ak_isp_lce_attr {
    AK_ISP_OP_TYPE lce_mode;
    AK_ISP_LCE manual_lce;
    AK_ISP_LCE linkage_lce[16];
} AK_ISP_LCE_ATTR;
typedef struct ak_isp_nr1 {
    unsigned short nr1_enable; //ä½¿èƒ½ä½
    unsigned short nr1_k; //[0,15]
    unsigned short nr_keepth;
    unsigned short nr1_calc_g_k;
    unsigned short nr1_calc_r_k;
    unsigned short nr1_calc_b_k;
    unsigned short nr1_lc_lut[17]; // 10bit
    unsigned short nr1_lc_lut_key[16];
    unsigned short nr1_weight_rtbl[17]; // 10bit
    unsigned short nr1_weight_gtbl[17]; // 10bit
    unsigned short nr1_weight_btbl[17]; // 10bit
} AK_ISP_NR1;

typedef struct ak_isp_nr1_attr {
    AK_ISP_OP_TYPE nr1_mode; // nr1 模式，自动或者联动模式
    AK_ISP_NR1 manual_nr1;
    AK_ISP_NR1 linkage_nr1[16]; //联动参数
} AK_ISP_NR1_ATTR;

typedef struct ak_isp_nr2 {
    unsigned short nr2_enable;
    unsigned short nr2_pass2_enable;
    unsigned short nr2_k; //[0,15]
    unsigned short ynr_keep_th;
    unsigned short ynr_intensity1; //[0, 15],default:15
    unsigned short ynr_intensity2; //[0, 15],default:15
    unsigned short nr2_calc_y_k;
    unsigned short nr2_weight_tbl[17]; // 10bit

    unsigned short ynr_y_dpc_enable;
    unsigned short ynr_y_dpc_th;
    unsigned short ynr_y_dpc_5_5;
    unsigned short ynr_y_black_dpc_enable;
    unsigned short ynr_y_white_dpc_enable;
} AK_ISP_NR2;

typedef struct ak_isp_uvnr {
    unsigned short uvnr_enable; //使能位
    unsigned short uvnr_pass2_enable;
    unsigned short uvnr_k;
    unsigned short uvnr_calc_k;
    unsigned short uvnr_weight_rtbl[17]; // 10bit
} AK_ISP_UVNR;

typedef struct ak_isp_nr2_attr {
    AK_ISP_OP_TYPE nr2_mode; //手动或者联动模式
    AK_ISP_NR2 manual_nr2;
    AK_ISP_NR2 linkage_nr2[16];
} AK_ISP_NR2_ATTR;

typedef struct ak_isp_uvnr_attr {
    AK_ISP_OP_TYPE uvnr_mode; //手动或者联动模式
    AK_ISP_UVNR manual_uvnr;
    AK_ISP_UVNR linkage_uvnr[16];
} AK_ISP_UVNR_ATTR;

typedef struct ak_isp_3d_nr_ref_attr {
    unsigned int yaddr_3d;
    unsigned int ysize_3d;
    unsigned int uaddr_3d;
    unsigned int usize_3d;
    unsigned int vaddr_3d;
    unsigned int vsize_3d;
} AK_ISP_3D_NR_REF_ATTR;

typedef struct ak_isp_3d_nr {
    unsigned short tnr_enable; // default:1(GUI edit)
    unsigned short update_ref;
    unsigned short tnr_refFrame_format; //参考帧压缩格式

    unsigned short t_mf_th1; //[0, 8191],default:300(GUI edit)
    unsigned short t_mf_th2; //[0, 8191],default:500(GUI edit)
    unsigned short t_ex_mf; //[0,2047]
    // unsigned short        t_mf_slop; //[0,255] //65536/(t_mf_th2-t_mf_th1)
    unsigned short statScale[129]; //[0, 15],default:8(GUI edit)
    unsigned short statScale_key[16];

    unsigned short ynr_k; //[0,15]
    unsigned short ynr_calc_k; //[0,65535](GUI edit)
    unsigned short ynr_weight_tbl[17]; // ynr_strength(GUI edit)
    unsigned short ylp_k1; //[0, 15],default:7(GUI edit)
    unsigned short ylp_k2; //[0, 15],default:0(GUI edit)
    unsigned short t_y_th1; //[0, 511],default:48(GUI edit)
    unsigned short t_y_th2; //[0, 511],default:16(GUI edit)
    unsigned short t_y_low; //[0,127]
    unsigned short t_y_low2; //[0,127]
    unsigned short t_y_k1; //[0, 127],default:120(GUI edit)
    unsigned short t_y_k2; //[0, 127],default:120(GUI edit)
    unsigned short t_y_ex_k; //[0, 15]
    unsigned short t_y_kslop; //[0, 127],default:32(GUI edit)
    unsigned short t_y_kslop2; //[0, 127]
    unsigned short y_minstep; //[0-15],default:2
    unsigned short t_y_src_choose_k_th; //[0, 127]
    unsigned short t_y_uvassist;

    unsigned short uvnr_k; //[0, 15],default:8(GUI edit)
    unsigned short uvlp_k1; //[0, 15],default:7(GUI edit)
    unsigned short uvlp_k2; //[0, 15],default:0(GUI edit)
    unsigned short t_uv_th1; //[0, 511],default:48(GUI edit)
    unsigned short t_uv_th2; //[0, 511],default:16(GUI edit)
    unsigned short t_uv_diff_scale; //[0, 15]
    unsigned short t_uv_k_low; //[0, 127]
    unsigned short t_uv_k_low2; //[0, 127]
    unsigned short t_uv_k1; //[0, 127],default:120(GUI edit)
    unsigned short t_uv_k2; //[0, 127]
    unsigned short t_uv_ex_k; //[0, 15],
    unsigned short t_uv_kslop; //[0, 127],default:32(GUI edit)
    unsigned short t_uv_kslop2; //[0, 127],default:32(GUI edit)
    unsigned short t_uv_minstep; //[0, 15],
    unsigned short t_uv_src_choose_k_th; //[0, 127]

    unsigned short sharp_k_th; //[0, 127]
    unsigned short sharp_factor_slop; //[0, 15]
    unsigned short sharp_factor_max; //[0, 15]
    unsigned short motion_filter; //[0, 15]
    unsigned int md_th; //[0, 65535]  è¿åŠ¨æ£€æµ‹é˜ˆå€¼
                        //[0-127],default:0
    unsigned short tnr_debug_output; //[0,1]
    unsigned short tnr_motion_flag_th;
} AK_ISP_3D_NR;

typedef struct ak_isp_3d_nr_rc{
    unsigned short tnr_ref_rc_enable; 
    unsigned short tnr_refy_rc_bit;   
    unsigned short tnr_refuv_rc_bit;  
}AK_ISP_3D_NR_RC; // H322LS: Add

typedef struct ak_isp_3d_nr_attr {
    AK_ISP_OP_TYPE _3d_nr_mode;

    AK_ISP_3D_NR manual_3d_nr;
    AK_ISP_3D_NR linkage_3d_nr[16];

    AK_ISP_3D_NR_RC manual_3dnr_ref_rc;
    AK_ISP_3D_NR_RC linkage_3dnr_ref_rc[16];
} AK_ISP_3D_NR_ATTR;

typedef struct ak_isp_3d_nr_stat_info {
    unsigned short MD_stat_max;
    unsigned short MD_stat[24][32]; //运动检测分块输出lz0499 9_12
    unsigned int MD_level; //运动检测输出
} AK_ISP_3D_NR_STAT_INFO;

typedef struct ak_isp_gb {

    unsigned short gb_enable; //使能位
    unsigned short gb_en_th; //[0,255]
    unsigned short gb_kstep; //[0,15]
    unsigned short gb_threshold; //[0,1023
} AK_ISP_GB;

typedef struct ak_isp_gb_attr {
    AK_ISP_OP_TYPE gb_mode; //模式选择，手动或者联动
    AK_ISP_GB manual_gb;
    AK_ISP_GB linkage_gb[16];
} AK_ISP_GB_ATTR;

typedef struct ak_isp_demo_attr {
    unsigned short dm_HV_th; //方向判别系数
    unsigned short dm_rg_thre; //[0 1023]
    unsigned short dm_bg_thre; //[0 1023]
    unsigned short dm_hf_th1; //[0, 1023]
    unsigned short dm_hf_th2; //[0, 1023]

    unsigned short dm_rg_gain; //[0 255]
    unsigned short dm_bg_gain; //[0 255]
    unsigned short dm_gr_gain; //[0 255]
    unsigned short dm_gb_gain; //[0 255]
    // unsigned short  cfa_mode;
} AK_ISP_DEMO_ATTR;

typedef struct ak_isp_ccm {
    unsigned short cc_enable; //颜色校正使能
    unsigned short cc_cnoise_yth; //亮度控制增益
    unsigned short cc_cnoise_gain; //亮度控制增益
    unsigned short cc_cnoise_slop; //亮度控制增益
    signed short ccm[3][3]; //[-2048, 2047]
} AK_ISP_CCM;

typedef struct ak_isp_auto_ccm {
    unsigned short cc_color_temp;
    AK_ISP_CCM auto_ccm_para;
} AK_ISP_AUTO_CCM;

typedef struct ak_isp_ccm_ctr {
    unsigned short cc_start_gain;
    unsigned short cc_min_saturation;
    unsigned short cc_adjust_slop;
} AK_ISP_CCM_CTR;

typedef struct ak_isp_manual_ccm_attr {
    AK_ISP_CCM_CTR manual_ccm_ctrl;
    AK_ISP_CCM manual_ccm_para;
} AK_ISP_MANUAL_CCM_ATTR;

typedef struct ak_isp_auto_ccm_attr {
    unsigned short color_matrix_num;
    AK_ISP_CCM_CTR auto_ccm_ctrl;
    AK_ISP_AUTO_CCM auto_ccm_para[10];
} AK_ISP_AUTO_CCM_ATTR;

typedef struct ak_isp_ccm_attr {
    AK_ISP_OP_TYPE cc_mode; //颜色校正矩阵联动或者手动
    AK_ISP_MANUAL_CCM_ATTR manual_ccm;
    AK_ISP_AUTO_CCM_ATTR auto_ccm; //四个联动矩阵
} AK_ISP_CCM_ATTR;

typedef struct ak_isp_wdr {
    unsigned short hdr_uv_adjust_level; // uv调整程度, [0,31]
    unsigned short hdr_cnoise_suppress_slop; //抑制斜率
    unsigned short wdr_enable;

    unsigned short wdr_th1; // 0-1023
    unsigned short wdr_th2; // 0-1023
    unsigned short wdr_th3; // 0-1023
    unsigned short wdr_th4; // 0-1023
    unsigned short wdr_th5; // 0-1023

    // unsigned short wdr_light_weight;

    unsigned short area_tb1[65]; //曲线 10bit
    unsigned short area_tb2[65]; //曲线 10bit
    unsigned short area_tb3[65]; //曲线 10bit
    unsigned short area_tb4[65]; //曲线 10bit
    unsigned short area_tb5[65]; //曲线 10bit
    unsigned short area_tb6[65]; //曲线 10bit

    unsigned short area1_key[16];
    unsigned short area2_key[16];
    unsigned short area3_key[16];
    unsigned short area4_key[16];
    unsigned short area5_key[16];
    unsigned short area6_key[16];

    unsigned short hdr_uv_adjust_enable; // uv调整使能
    unsigned short hdr_cnoise_suppress_yth1; //色彩噪声亮度阈值1
    unsigned short hdr_cnoise_suppress_yth2; //色彩噪声亮度阈值2
    unsigned short hdr_cnoise_suppress_gain; //色差抑制
} AK_ISP_WDR;

typedef struct ak_isp_wdr_attr {
    AK_ISP_OP_TYPE wdr_mode; //模式选择，手动或者联动
    AK_ISP_WDR manual_wdr;
    AK_ISP_WDR linkage_wdr[16];
} AK_ISP_WDR_ATTR;

typedef struct ak_isp_wdr_ex_attr {
    unsigned short hdr_blkW;
    unsigned short hdr_blkH;
    unsigned short hdr_reverseW_g; //[0,511]
    unsigned short hdr_reverseW_shift; //[0,15];
    unsigned short hdr_reverseH_g; //[0,511]
    unsigned short hdr_reverseH_shift; //[0,15]
    unsigned short hdr_weight_g; //[0,511]
    unsigned short hdr_weight_shift; //[0,15]
    unsigned short hdr_weight_k; //[0,15]
} AK_ISP_WDR_EX_ATTR;

typedef struct ak_isp_sharp {

    unsigned short mf_hpf_k; //[0,127]
    unsigned short mf_hpf_shift; //[0,15]

    unsigned short hf_hpf_k; //[0,127]
    unsigned short hf_hpf_shift; //[0,15]

    unsigned short sharp_method; //[0,3]
    unsigned short sharp_debug_output; //[0,3]
    unsigned short sharp_skin_gain_weaken; //[0ï¼Œ3]

    unsigned short sharp_skin_gain_th; //[0, 255]
    unsigned short sharp_skin_detect_enable;

    unsigned short ysharp_enable; //[0,1]
    unsigned short ysharp_nr_enable; //[0,1]
    unsigned short ysharp_nr_k;
    unsigned short ysharp_nrweight_tbl[17];
    unsigned short ysharp_nr_calc_k;
    unsigned short sharp_reduce_factor[24][32];

    unsigned short edge_enable;
    unsigned short Vedge_k; //[0, 15]
    unsigned short Vedge_mix_th; //[0, 255]
    unsigned short Vedge_mix_slop; //[0, 15]
    unsigned short VedgaGainW_ob; //[0, 255]
    unsigned short VedgaGainW_k; //[0, 255]
    unsigned short VedgaGainB_ob; //[0, 255]
    unsigned short VedgaGainB_k; //[0, 255]

    unsigned short Hedge_k; //[0, 15]
    unsigned short Hedge_mix_th; //[0, 255]
    unsigned short Hedge_mix_slop; //[0, 15]
    unsigned short HedgaGainW_ob; //[0, 255]
    unsigned short HedgaGainW_k; //[0, 255]
    unsigned short HedgaGainB_ob; //[0, 255]
    unsigned short HedgaGainB_k; //[0, 255]

    unsigned short TLedge_k; //[0, 15]
    unsigned short TLedgaGainW_ob; //[0, 255]
    unsigned short TLedgaGainW_k; //[0, 255]
    unsigned short TLedgaGainB_ob; //[0, 255]
    unsigned short TLedgaGainB_k; //[0, 255]

    unsigned short TRedge_k; //[0, 15]
    unsigned short TRedgaGainW_ob; //[0, 255]
    unsigned short TRedgaGainW_k; //[0, 255]
    unsigned short TRedgaGainB_ob; //[0, 255]
    unsigned short TRedgaGainB_k; //[0, 255]

    unsigned short EdgeGainMax; //[0, 511]

    unsigned short Edge_mix_th; //[0, 255]
    unsigned short Edge_mix_slop; //[0, 15]

    signed short MF_HPF_LUT[256]; //[-256,255]
    signed short HF_HPF_LUT[256]; //[-256,255]
    unsigned short MF_LUT_KEY[16];
    unsigned short HF_LUT_KEY[16];
} AK_ISP_SHARP;

typedef struct ak_isp_sharp_attr {
    AK_ISP_OP_TYPE ysharp_mode;
    AK_ISP_SHARP manual_sharp_attr;
    AK_ISP_SHARP linkage_sharp_attr[16];
} AK_ISP_SHARP_ATTR;

typedef struct ak_isp_sharp_ex_attr {
    signed short mf_HPF[6]; //
                            // M13,M14,M15,
                            // M11,M12,M14,
                            // M10,M11,M13,
    signed short hf_HPF[3]; //
                            // M22 M21,M22,
                            // M21,M20, M21,
} AK_ISP_SHARP_EX_ATTR;

typedef struct {
    AK_ISP_SHARP sharp_tmp;
    int gain;
} AK_ISP_SHARP_INFO;

typedef struct ak_isp_saturation {
    unsigned short SE_enable; // 使能位
    unsigned short SE_th1; //[0, 1023]
    unsigned short SE_th2; //[0, 1023]
    unsigned short SE_th3; //[0, 1023]
    unsigned short SE_th4; //[0, 1023]
    unsigned short SE_scale_slop1; //[0, 255]
    unsigned short SE_scale_slop2; //[0, 255]
    unsigned short SE_scale1; //[0,255]
    unsigned short SE_scale2; //[0,255]
    unsigned short SE_scale3; //[0,255]
} AK_ISP_SATURATION;

typedef struct ak_isp_saturation_attr {
    AK_ISP_OP_TYPE SE_mode; //饱和度模式
    AK_ISP_SATURATION manual_sat;
    AK_ISP_SATURATION linkage_sat[16];
} AK_ISP_SATURATION_ATTR;

typedef struct ak_isp_contrast {
    unsigned short y_contrast; //[0,511]
    signed short y_shift; //[0, 511]
} AK_ISP_CONTRAST;

typedef struct ak_isp_auto_contrast {
    unsigned short dark_pixel_area; //[0, 511]
    unsigned short dark_pixel_rate; //[1, 256]
    unsigned short shift_max; //[0, 127]
} AK_ISP_AUTO_CONTRAST;

typedef struct ak_isp_contrast_ATTR {
    AK_ISP_OP_TYPE cc_mode; //模式选择，手动或者联动
    AK_ISP_CONTRAST manual_contrast;
    AK_ISP_AUTO_CONTRAST linkage_contrast[16];
} AK_ISP_CONTRAST_ATTR;

typedef struct ak_isp_fcs {
    unsigned short fcs_th; //[0, 255]
    unsigned short fcs_gain_slop; //[0,63]
    unsigned short fcs_enable; //使能位
    unsigned short fcs_uv_nr_enable; //使能位
    unsigned short fcs_uv_nr_th; //[0, 1023]
} AK_ISP_FCS;

typedef struct ak_isp_fcs_attr {
    AK_ISP_OP_TYPE fcs_mode; //模式选择，手动或者联动
    AK_ISP_FCS manual_fcs;
    AK_ISP_FCS linkage_fcs[16];
} AK_ISP_FCS_ATTR;

typedef struct ak_isp_hue {
    unsigned short hue_sat_en; // hue使能
    signed char hue_lut_a[65]; //[-128, 127]
    signed char hue_lut_b[65]; //[-128, 127]
    unsigned char hue_lut_s[65]; //[0, 255]
} AK_ISP_HUE;

typedef struct ak_isp_auto_hue {
    unsigned short hue_color_temp;
    AK_ISP_HUE auto_hue_para;
} AK_ISP_AUTO_HUE;

typedef struct ak_isp_auto_hue_attr {
    unsigned short hue_num;
    AK_ISP_AUTO_HUE auto_hue_para[10];
} AK_ISP_AUTO_HUE_ATTR;

typedef struct ak_isp_hue_attr {
    AK_ISP_OP_TYPE hue_mode; //联动或者手动
    AK_ISP_HUE manual_hue;
    AK_ISP_AUTO_HUE_ATTR auto_hue; //四个联动参数
} AK_ISP_HUE_ATTR;

typedef struct ak_isp_rgb2yuv_attr {
    AK_ISP_RGB2YUV_MODEL mode; // bt601 或者bt709
} AK_ISP_RGB2YUV_ATTR;

typedef struct ak_isp_effect_attr {
    unsigned short y_a; // [0, 255]
    signed short y_b; //[-128, 127]
    signed short uv_a; //[-256, 255]
    signed short uv_b; //[-256, 255]
    unsigned short dark_margin_en; //黑边或者边沿复制使能 0：Normal，1：Dark Margin，2：Copy Margin
} AK_ISP_EFFECT_ATTR;

typedef struct ak_isp_ddpc {
    unsigned short ddpc_enable; //动态坏点使能位
    unsigned short ddpc_th_base; // 10bit
    unsigned short ddpc_th_slop;
    unsigned short ddpc_strength;
    unsigned short white_dpc_enable; //白点消除使能位
    unsigned short black_dpc_enable; //黑点消除使能位
} AK_ISP_DDPC;

typedef struct ak_isp_ddpc_attr {
    AK_ISP_OP_TYPE ddpc_mode; //模式选择，手动或者联动
    AK_ISP_DDPC manual_ddpc;
    AK_ISP_DDPC linkage_ddpc[16];
} AK_ISP_DDPC_ATTR;

typedef struct ak_isp_af_attr {
    unsigned short af_win0_left; //[0, 4095]
    unsigned short af_win0_right; //[0, 4095]
    unsigned short af_win0_top; //[0, 4095]
    unsigned short af_win0_bottom; //[0, 4095]

    unsigned short af_win1_left; //[0, 4095]
    unsigned short af_win1_right; //[0, 4095]
    unsigned short af_win1_top; //[[0, 4095]
    unsigned short af_win1_bottom; //[0, 4095]

    unsigned short af_win2_left; //[0, 4095]
    unsigned short af_win2_right; //[0, 4095]
    unsigned short af_win2_top; //[0, 4095]
    unsigned short af_win2_bottom; //[0, 4095]

    unsigned short af_win3_left; //[0, 4095]
    unsigned short af_win3_right; //[0, 4095]
    unsigned short af_win3_top; //[0, 4095]
    unsigned short af_win3_bottom; //[0, 4095]

    unsigned short af_win4_left; //[0, 4095]
    unsigned short af_win4_right; //[0, 4095]
    unsigned short af_win4_top; //[0, 4095]
    unsigned short af_win4_bottom; //[0, 4095]

    unsigned short af_th; //[0, 128]
} AK_ISP_AF_ATTR;

typedef struct ak_isp_af_stat_info {
    unsigned int af_statics[5]; //统计结果
} AK_ISP_AF_STAT_INFO;

typedef struct ak_isp_3d_nr_rer_size_stat_info {
    unsigned int ref_size_statis[3];
} AK_ISP_3D_NR_REF_SIZE_STAT_INFO;

#if 0
typedef struct ak_isp_weight_attr {
   unsigned short   zone_weight[8][16];      //权重系数
}AK_ISP_WEIGHT_ATTR;
#endif

//////////////////////Auto White Balance//////////////////////
typedef enum ak_isp_awb_status {
    AWB_STATUS_PAUSE = 0,
    AWB_STATUS_NORMAL,
} AK_ISP_AWB_STATUS;

typedef enum ak_isp_awb_algo_type {
    AWB_ALGO_DEFAULT = 0,
    AWB_ALGO_AVERAGE_WEIGHT = 1,
    AWB_ALGO_BLOCK_ASSIST,
    AWB_ALGO_GW,
    AWB_ALGO_BUTT,
} AK_ISP_AWB_ALGO_TYPE;

typedef struct ak_isp_mwb_attr {
    unsigned short r_gain;
    unsigned short g_gain;
    unsigned short b_gain;
    signed short r_offset;
    signed short g_offset;
    signed short b_offset;
} AK_ISP_MWB_ATTR;

typedef struct ak_isp_awb_stat_calib_para {
    unsigned short gr_low[10]; // gr_low[i]<=gr_high[i]
    unsigned short gb_low[10]; // gb_low[i]<=gb_high[i]
    unsigned short gr_high[10];
    unsigned short gb_high[10];
    unsigned short rb_low[10]; // rb_low[i]<=rb_high[i]
    unsigned short rb_high[10];
} AK_ISP_AWB_STAT_CALIB_PARA;

typedef struct ak_isp_awb_attr {
    unsigned short g_weight[16];
    unsigned short y_low; // y_low<=y_high
    unsigned short y_high;
    unsigned short err_est;
    unsigned short awb_iso_track_enable;
    AK_ISP_AWB_STAT_CALIB_PARA awb_stat_calib_para[16];
#if 0
    unsigned short   gr_low[10];            //gr_low[i]<=gr_high[i]
    unsigned short   gb_low[10];            //gb_low[i]<=gb_high[i]
    unsigned short   gr_high[10];
    unsigned short   gb_high[10];
    unsigned short   rb_low[10];           //rb_low[i]<=rb_high[i]
    unsigned short   rb_high[10];
#endif
    // awb软件部分需要设置的参数
    unsigned short auto_wb_step; //白平衡步长计算
    unsigned short total_cnt_thresh; //像素个数阈值
    unsigned short
        colortemp_stable_cnt_thresh; //稳定帧数，多少帧一样认为环境色温改变
    unsigned short colortemp_envi[10];
    AK_ISP_AWB_ALGO_TYPE alg_type;
} AK_ISP_AWB_ATTR;

typedef struct ak_isp_awb_rgain_max {
    int rgain_max[10];
} AK_ISP_AWB_RGAIN_MAX;

typedef struct ak_isp_awb_rgain_min {
    int rgain_min[10];
} AK_ISP_AWB_RGAIN_MIN;

typedef struct ak_isp_awb_ggain_max {
    int ggain_max[10];
} AK_ISP_AWB_GGAIN_MAX;

typedef struct ak_isp_awb_ggain_min {
    int ggain_min[10];
} AK_ISP_AWB_GGAIN_MIN;

typedef struct ak_isp_awb_bgain_max {
    int bgain_max[10];
} AK_ISP_AWB_BGAIN_MAX;

typedef struct ak_isp_awb_bgain_min {
    int bgain_min[10];
} AK_ISP_AWB_BGAIN_MIN;

typedef struct ak_isp_awb_ration {
    unsigned int ratio[10];
} AK_ISP_AWB_RATIO;

typedef struct ak_isp_awb_ex_attr {
    int awb_ex_ctrl_enable;
    AK_ISP_AWB_RGAIN_MAX rgain_max[16];
    AK_ISP_AWB_RGAIN_MIN rgain_min[16];
    AK_ISP_AWB_GGAIN_MAX ggain_max[16];
    AK_ISP_AWB_GGAIN_MIN ggain_min[16];
    AK_ISP_AWB_BGAIN_MAX bgain_max[16];
    AK_ISP_AWB_BGAIN_MIN bgain_min[16];
    AK_ISP_AWB_RATIO prefer_rratio[16];
    AK_ISP_AWB_RATIO prefer_bratio[16];
} AK_ISP_AWB_EX_ATTR;

#if 0
typedef  struct  ak_isp_awb_ctrl {
    int rgain_max;
    int rgain_min;
    int ggain_max;
    int ggain_min;
    int bgain_max;
    int bgain_min;
    int rgain_ex;
    int bgain_ex;
}AK_ISP_AWB_CTRL;

typedef  struct  ak_isp_awb_ex_attr {
    int awb_ex_ctrl_enable;
    AK_ISP_AWB_CTRL awb_ctrl[10];
}AK_ISP_AWB_EX_ATTR;
#endif

typedef struct ak_isp_awb_block_stat_info {
    unsigned int r_awb_block_stat[24][32];
    unsigned int g_awb_block_stat[24][32];
    unsigned int b_awb_block_stat[24][32];
} AK_ISP_AWB_BLOCK_STAT_INFO;

typedef struct ak_isp_awb_stat_info {
    //白平衡统计结果
    unsigned int total_R[10];
    unsigned int total_G[10];
    unsigned int total_B[10];
    unsigned int total_cnt[10];
    //经由自动白平衡算法算出的白平衡增益值
    AK_ISP_AWB_BLOCK_STAT_INFO wb_block_stat_info;
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

} AK_ISP_AWB_STAT_INFO;

typedef struct ak_isp_awb_run_info {
    unsigned short r_gain;
    unsigned short g_gain;
    unsigned short b_gain;
    signed short r_offset;
    signed short g_offset;
    signed short b_offset;
    unsigned char is_stable;
    unsigned short
        current_colortemp_index; //环境色温标记，是参数随环境变化的色温指标。
    unsigned short colortemp_stable_cnt[10]; //每一种色温稳定的帧数计数
    unsigned short current_colortemp;
    AK_ISP_CCM current_ccm;
} AK_ISP_AWB_RUN_INFO;

typedef struct ak_isp_awb_calib_node_attr {
    unsigned short cali_gr;
    unsigned short cali_gb;
    unsigned short cali_color_temp;
} AK_ISP_AWB_CALIB_NODE_ATTR;

typedef struct ak_isp_awb_calib_attr {
    unsigned int awb_calib_node_num;
    AK_ISP_AWB_CALIB_NODE_ATTR awb_calib_para[10];
} AK_ISP_AWB_CALIB_INFO;

typedef struct ak_isp_wb_gain {
    unsigned short r_gain;
    unsigned short g_gain;
    unsigned short b_gain;
    signed short r_offset;
    signed short g_offset;
    signed short b_offset;
} AK_ISP_WB_GAIN;


typedef struct ak_isp_wb_attr {
    AK_ISP_AWB_STATUS wb_status;
    unsigned char awb_run_interval;
    AK_ISP_OP_TYPE wb_type;
    AK_ISP_MWB_ATTR manual_wb_para;
    AK_ISP_AWB_ATTR auto_wb_para;
} AK_ISP_WB_ATTR;

///////////////////AUTO Exposure//////////////////////
typedef enum ak_isp_ae_status {
    AE_STATUS_PAUSE = 0,
    AE_STATUS_NORMAL,
} AK_ISP_AE_STATUS;

typedef enum ak_isp_ae_strgy_model {
    AE_EXP_HIGHLIGHT_PRIOR = 0,
    AE_EXP_LOWLIGHT_PRIOR,
    AE_STRGY_MODEL_BUFF,
} AK_ISP_AE_STRGY_MODEL;

typedef enum ak_isp_win_weight_type {
    AE_CENTER_WGHT = 0,
    AE_AVERAGE = 1,
    AE_SPOT,
    AE_WGHT_END,
} AK_ISP_WIN_WEIGHT_TYPE;

typedef struct ak_isp_weight_attr {
    unsigned short zone_weight[8][16];
} AK_ISP_WEIGHT_ATTR;

typedef struct ak_isp_raw_hist_stat_info {
    unsigned int raw_g_hist[256];
    unsigned int raw_g_total;
} AK_ISP_RAW_HIST_STAT_INFO;

typedef struct ak_isp_rgb_hist_stat_info {
    unsigned int rgb_hist[256];
    unsigned int rgb_total;
} AK_ISP_RGB_HIST_STAT_INFO;

typedef struct ak_isp_yuv_hist_stat_info {
    unsigned int y_hist[256];
    unsigned int y_total;
} AK_ISP_YUV_HIST_STAT_INFO;

typedef struct ak_isp_ae_strgy_attr {
    unsigned int hist_weight[16];
} AK_ISP_AE_STRGY_ATTR;

typedef struct ak_isp_ae_route_node_attr {
    unsigned int exp_time;
    unsigned int again;
    unsigned int dgain;
    unsigned int isp_dgain;
} AK_ISP_AE_ROUTE_NODE_ATTR;

typedef struct ak_isp_ae_route_attr {
    unsigned int ae_route_en;
    unsigned int ae_route_node_num;
    AK_ISP_AE_ROUTE_NODE_ATTR ae_route_node[16];
} AK_ISP_AE_ROUTE_ATTR;

typedef struct ak_isp_ae_ex_convergence_attr {
    unsigned int ae_ex_convergence_en;
    unsigned int control_zone1;
    unsigned int control_zone2;
    unsigned int control_zone3;
    unsigned int control_zone4;
    unsigned int control_step1;
    unsigned int control_step2;
    unsigned int control_step3;
    unsigned int control_step4;
    unsigned int control_step_max;
} AK_ISP_AE_EX_CONVER_ATTR;

typedef struct ak_isp_me_attr {
    unsigned int exp_time;
    unsigned int a_gain;
    unsigned int d_gain;
    unsigned int isp_d_gain;
} AK_ISP_ME_ATTR;

typedef struct ak_isp_ex_zone_weight_attr {
    unsigned short win_weight[8][16];
    unsigned short ex_zone_weight_str;
} AK_ISP_EX_ZONE_WEIGHT_ATTR;

typedef struct ak_isp_ae_attr {
    unsigned int exp_time_max; //曝光时间的最大值
    unsigned int exp_time_min; //曝光时间的最小值
    unsigned int d_gain_max; //数字增益的最大值
    unsigned int d_gain_min; //数字增益的最小值
    unsigned int isp_d_gain_min; // isp数字增益的最小
    unsigned int isp_d_gain_max; // isp数字增益的最大值
    unsigned int a_gain_max; //模拟增益的最大值
    unsigned int a_gain_min; //模拟增益的最小值
    unsigned int exp_step; //用户曝光调整步长
    unsigned int exp_stable_range; //稳定范围
    unsigned int exp_hold_range;
    unsigned int exp_speed;
    unsigned int anti_flicker_target_lumi;
    unsigned int anti_flicker_start_exp; //[1,100]
    unsigned int exp_lumi_filter_para;
    unsigned int target_lumiance[16]; //目标亮度
    unsigned int envi_gain_range[16][2];
    // unsigned int   hist_weight[16];
    unsigned int blacklight_compensation_en;
    unsigned int blacklight_detect_scope;
    unsigned int blacklight_rate_max;
    unsigned int blacklight_rate_min;
    AK_ISP_WIN_WEIGHT_TYPE ae_win_weight_type;
    AK_ISP_WEIGHT_ATTR win_weight[4];
    AK_ISP_AE_EX_CONVER_ATTR ae_ex_convergence_para;
    AK_ISP_AE_STRGY_MODEL ae_strgy_type;
    AK_ISP_AE_STRGY_ATTR ae_strgy_para[3];
    AK_ISP_AE_ROUTE_ATTR exp_route_para;
} AK_ISP_AE_ATTR;

typedef struct ak_isp_ae_run_info {
    unsigned char current_calc_avg_lumi; //现在的计算出的亮度值
    unsigned char current_calc_avg_compensation_lumi; //经过曝光补偿后的亮度值

    unsigned char current_target_lumi; //白天黑夜的标记
    unsigned char is_stable;

    int current_a_gain; //模拟增益的值
    int current_d_gain; //数字增益的值
    int current_isp_d_gain; // isp数字增益的值
    int current_exp_time; //曝光时间的值

    unsigned int current_a_gain_step; //现在的模拟增益的步长
    unsigned int current_d_gain_step; //数字增益的步长
    unsigned int current_isp_d_gain_step; // isp数字增益的步长
    unsigned int current_exp_time_step;
    // AK_ISP_AE_ROUTE_ATTR      ae_route_para;
} AK_ISP_AE_RUN_INFO;

typedef struct ak_ae_stat_info {
    AK_ISP_RAW_HIST_STAT_INFO raw_hist_stat_para;
    AK_ISP_RGB_HIST_STAT_INFO rgb_hist_stat_para;
    AK_ISP_YUV_HIST_STAT_INFO yuv_hist_stat_para;
} AK_ISP_AE_STAT_INFO;

typedef struct {
    int a_gain;
    int d_gain;
    int isp_d_gain;
    int exp_time;
    int effect_frame;
    int effect_gain;
} AK_ISP_AE_UPDATE_QUEUE;

typedef struct ak_isp_exposure_attr {
    AK_ISP_AE_STATUS exp_status;
    unsigned char ae_run_interval;
    AK_ISP_OP_TYPE exp_type;
    AK_ISP_ME_ATTR manual_exp_para;
    AK_ISP_AE_ATTR auto_exp_para;
} AK_ISP_EXPOSURE_ATTR;
//帧率控制结构体
typedef struct ak_isp_frame_rate_attr {
    unsigned int hight_light_frame_rate;
    unsigned int hight_light_max_exp_time;
    unsigned int hight_light_to_low_light_gain;
    unsigned int low_light_frame_rate;
    unsigned int low_light_max_exp_time;
    unsigned int low_light_to_hight_light_gain;
} AK_ISP_FRAME_RATE_ATTR;

typedef struct ak_isp_pp_frame_ctrl {
    unsigned short pp_frame_rate;
    unsigned char pp_frame_skip_en;
    unsigned char pp_buffer_disable_when_done;
    unsigned short pp_frame_ctrl;
} AK_ISP_PP_FRAME_CTRL_ATTR;

typedef struct ak_isp_misc_attr {
    unsigned short hsyn_pol;
    unsigned short vsync_pol;
    unsigned short pclk_pol;
    unsigned short test_pattern_en;
    unsigned short test_pattern_cfg;
    unsigned short cfa_mode;
    unsigned short inputdataw;
    unsigned short one_line_cycle;
    unsigned short hblank_cycle;
    unsigned short vflip;
    unsigned short hflip;
    unsigned short frame_start_delay_en;
    unsigned short frame_start_delay_num;
    unsigned short frame_start_edge;

    unsigned short mipi_virtual_channel_sel;
    unsigned short mipi_bps_virtual_channel_sel;
    unsigned short mipi_hsync_str_sel;
    unsigned short mipi_hsync_end_sel;
    unsigned short mipi_auto_decoder_pix;
} AK_ISP_MISC_ATTR;

typedef struct ak_isp_mask_area {
    unsigned short fgc_cfg;
    unsigned short aux_cfg;
#if 0   //del pp
    AK_ISP_PP_DRAW_MODE mode;
    AK_ISP_PP_WINDOW win;
#endif
} AK_ISP_MASK_AREA;

typedef struct ak_isp_mask_masic_attr {
    unsigned short mosai_size_hor;
    unsigned short mosai_size_vec;
} AK_ISP_MASK_MASIC_ATTR;

typedef struct ak_isp_axi0_attr {
    unsigned short axi_download_outstand;
    unsigned short axi_download_boundry;
    unsigned short axi_upload_outstand;
    unsigned short axi_upload_boundry;
    unsigned short axi_upoad_response;
    unsigned short axi_crossbar_gnt_mode;
} AK_ISP_AXI0_ATTR;

typedef struct ak_isp_axi1_attr {
    unsigned short axi_crossbar_aw_qos;
    unsigned short axi_crossbar_ar_qos;
} AK_ISP_AXI1_ATTR;
typedef struct ak_isp_stride_attr {
    unsigned short y_stride;
    unsigned short u_stride;
    unsigned short v_stride;
} AK_ISP_STRIDE_ATTR;
typedef struct ak_isp_main_chan_mask_area_attr {
    AK_ISP_MASK_AREA mask_area[14];
    AK_ISP_MASK_MASIC_ATTR mask_masic_attr;
} AK_ISP_MAIN_CHAN_MASK_AREA_ATTR;

typedef struct ak_isp_sub_chan_mask_area_attr {
    AK_ISP_MASK_AREA mask_area[14];
    AK_ISP_MASK_MASIC_ATTR mask_masic_attr;
} AK_ISP_SUB_CHAN_MASK_AREA_ATTR;

typedef struct ak_isp_mask_color_attr {
    unsigned char color_type; // 0:指定遮挡色；1：
    unsigned char mk_alpha;
    unsigned char y_mk_color;
    unsigned char u_mk_color;
    unsigned char v_mk_color;
} AK_ISP_MASK_COLOR_ATTR;

// PP输入输出格式
typedef enum {
    PP_FORMAT_DATASTREAM = 0,
    PP_FORMAT_YUV420 = 1,
    PP_FORMAT_YUV422 = 2,
    PP_FORMAT_YUV444 = 3,
    PP_FORMAT_LUMA = 5,
    PP_FORMAT_RGB888 = 4,
    PP_FORMAT_RGB565 = 6 // 暂不支持
} PP_FORMAT_ATTR;

#if 1   //del pp
// YUV -> RGB矩阵
typedef enum {
    PP_YUV_MATRIX_BT601 = 0,
    PP_YUV_MATRIX_BT709 = 1,
    PP_YUV_MATRIX_BT2020 = 2
} PP_YUV_MATRIX;

// YUV -> RGB输入的yuv范围
typedef enum {
    PP_YUV_SWING_FULL = 0, // 0-255
    PP_YUV_SWING_STUDIO = 1 // 16-235, 16-240
} PP_YUV_SWING;

typedef struct pp_src_para_ctrl_attr {
    unsigned short src_frame_stamp;
    unsigned char src_frame_stamp_en;
    unsigned char src_enable;
    PP_FORMAT_ATTR src_format;
    unsigned char src_hflip;
    unsigned char src_vflip;
    PP_YUV_SWING src_yuv_swing; // 0x400 [14]
    PP_YUV_MATRIX src_yuv_matrix; // 0x400 [16:15]
    unsigned short src_frame_rate;
} PP_SRC_PARA_CTRL_ATTR;

typedef enum {
    PP_PLANER_FULL = 0,
    PP_PLANER_SEMI = 1,
    PP_PLANER_PACKED = 2
} PP_PLANER_ATTR;

typedef enum {
    PP_ENDIAN_VUY = 0,
    PP_ENDIAN_YUV = 1,
    PP_ENDIAN_VYU = 2
} PP_ENDIAN_ATTR;

typedef struct pp_slice_ctrl_attr {
    unsigned char slice_enable;
    unsigned int pp_slice_height;
    unsigned int pp_last_slice_height;
    PP_FORMAT_ATTR format; // 0x408 ~ 0x41C
    PP_PLANER_ATTR planer; // 0x408 ~ 0x41C
    PP_ENDIAN_ATTR endian;
} PP_SLICE_CTRL_ATTR;

typedef struct pp_state_attr {
    unsigned char pp_src_state;
    unsigned char pp_ch1_state;
    unsigned char pp_ch2_state;
    unsigned char pp_ch3_state;
    unsigned char pp_ch1_axi_state;
    unsigned char pp_ch2_axi_state;
    unsigned char pp_ch3_axi_state;
} PP_STATE_ATTR;
// enum for sub sample
typedef enum {
    SUBSMP_1X, /*no sub sample*/
    SUBSMP_2X, /*sub sample 1/2 * 1/2 */
    SUBSMP_4X, /*sub sample 1/4 * 1/4 */
    SUBSMP_8X /*sub sample 1/8 * 1/8 */
} T_SUBSMP_RTO;

typedef enum mask_num {
    MAIN_CHAN_ONE = 0,
    MAIN_CHAN_TWO,
    MAIN_CHAN_THREE,
    MAIN_CHAN_FOURE,
    SUB_CHAN_ONE,
    SUB_CHAN_TWO,
    SUB_CHAN_THREE,
    SUB_CHAN_FOURE,
} MASK_NUM;

typedef enum osd_channel {
    OSD_CHN0 = 0,
    OSD_CHN1,
    OSD_CHN2,
    OSD_CHN3,
    OSD_CHN_NUM
} OSD_CHANNEL;

typedef struct ak_isp_osd_mask_color_table_attr {
    unsigned int color_table[20];
} AK_ISP_OSD_MASK_COLOR_TABLE_ATTR;

typedef struct ak_isp_osd_context_attr {

#if 0
    OSD_CHANNEL chn;
    unsigned int *osd_context_addr;
    unsigned int osd_width;
    unsigned int osd_height;
    unsigned short start_xpos;
    unsigned short start_ypos;
    unsigned short alpha;
    unsigned char enable;
#endif
    OSD_CHANNEL chn;
    unsigned int* osd_context_addr;
    AK_ISP_PP_OSD_MODE mode;
    AK_ISP_PP_WINDOW win;
    AK_ISP_PP_OSD_SIZE size;
    unsigned int addr;
    unsigned int stride;
} AK_ISP_OSD_CONTEXT_ATTR;

typedef struct ak_main_chan_osd_attr {
    AK_ISP_OSD_CONTEXT_ATTR osd_attr[4];
} AK_MAIN_CHAN_OSD_ATTR;

typedef struct ak_sub_chan_osd_attr {
    AK_ISP_OSD_CONTEXT_ATTR osd_attr[4];
} AK_SUB_CHAN_OSD_ATTR;

typedef struct ak_isp_osd_mem_attr {
    OSD_CHANNEL chn;
    unsigned char* dma_paddr;
    unsigned char* dma_vaddr;
    unsigned int size;
} AK_ISP_OSD_MEM_ATTR;
#endif
typedef enum ak_isp_pclk_polar {
    POLAR_ERR = 0,
    POLAR_RISING,
    POLAR_FALLING,
} AK_ISP_PCLK_POLAR;

typedef struct ak_isp_ae_info {
    int a_gain; //模拟增益的值
    int d_gain; //数字增益的值
    int isp_d_gain; // isp数字增益的值
    int exp_time; //曝光时间的值

} AK_ISP_AE_INFO;

typedef struct ak_isp_wb_gain_info {
    AK_ISP_WB_GAIN isp_wb_gain;
    char flg;

} ISP_WB_GAIN_INFO;

typedef struct ak_isp_ae_info_update {
    AK_ISP_AE_INFO ae_info;
    char flg;

} AK_ISP_AE_INFO_UPDATE;

typedef struct ae_fast_struct {
    int sensor_exp_time;
    int sensor_a_gain;
    int sensor_d_gain;
    int isp_d_gain;
    AK_ISP_WB_GAIN wb;
} AK_ISP_AE_FAST;

struct mem_ptrs {
    unsigned int* reg_blkaddr1;
    unsigned int* reg_blkaddr2;
    unsigned int* reg_blkaddr3;
    unsigned int* reg_blkaddr4;
    unsigned int* reg_blkaddr5;
    unsigned int* mem_blkaddr1;
    unsigned int* mem_blkaddr2;
    unsigned int* mem_blkaddr3;
    unsigned int* mem_blkaddr4;
    unsigned int* mem_blkaddr5;
    unsigned int* mem_blkaddr6;
    unsigned int* mem_blkaddr7;
};

#if 1
typedef struct ak_isp_aec_algo {
    unsigned int  aec_status;				//自动曝光控制的状态，稳定或者不稳定
    unsigned int  aec_locked;			//自动曝光锁定与否
    unsigned char  exp_time_need_updata;	//需要被更新，减少iic读写次数
    unsigned char  a_gain_need_updata;		//需要被更新，减少iic读写次数
    unsigned char  d_gain_need_updata;		 //需要被更新，只有需要更新时才写
    unsigned char  isp_d_gain_need_updata;	//需要被更新
    int		ae_isp_d_gain;
    int		ae_effect_frame;
    int	rnode_num;
    int	rnode_exp_time[16+2];
    int	rnode_again[16+2];
    int	rnode_dgain[16+2];
    int	rnode_ispDgain[16+2];
    unsigned long	rnode_EV[(16+2)*4];
    //ev[0]: start EV; ev[1]: EV from rnode_exp_time[0] to rnode_exp_time[1];
    //ev[2]: EV frome rnode_again[0] to rnode_again[1]; ev[3]: EV from rnode_dgain[0] to rnode_dgain[1]
    //ev[4]:EV at rnode_exp_time[1]/rnode_again[1]/rnode_dgain[1]/rnode_ispDgain[1];
    //ev[5]: EV from rnode_exp_time[1] to rnode_exp_time[2]; .....

    AK_ISP_AE_UPDATE_QUEUE	ae_update_queue[4];
    int 				ae_last_gain_step;
    unsigned int	frame_idx;
}AK_ISP_AEC_ALGO;
#endif

/**
*@brief: ak_isp_ae_info
*/
void ak_isp_ae_info(void* isp_struct, AK_ISP_AE_INFO* isp_ae_info);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vi_start_capturing(void* isp_struct);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vi_capturing_one_prepare(void* isp_struct);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vi_capturing_one(void* isp_struct);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vi_stop_capturing(void* isp_struct);
/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vi_set_input_size(void* isp_struct, int width, int height);
/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vi_set_crop(void* isp_struct, int sx, int sy, int width, int height);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vi_get_crop(
    void* isp_struct, int* sx, int* sy, int* width, int* height);

#if 0   //del pp
/**
*@brief: ak_isp_ae_info
*/
int ak_isp_set_pp_main_chan_crop(
    void* isp_struct, int sx, int sy, int width, int height);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_get_pp_main_chan_crop(
    void* isp_struct, int* sx, int* sy, int* width, int* height);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_set_pp_sub_chan_crop(
    void* isp_struct, int sx, int sy, int width, int height);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_get_pp_sub_chan_crop(
    void* isp_struct, int* sx, int* sy, int* width, int* height);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_set_pp_ch3_chan_crop(
    void* isp_struct, int sx, int sy, int width, int height);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_get_pp_ch3_chan_crop(
    void* isp_struct, int* sx, int* sy, int* width, int* height);
#endif
/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vi_get_input_data_format(
    void* isp_struct, struct input_data_format* idf);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vi_set_misc_attr(void* isp_struct, AK_ISP_MISC_ATTR* p_misc);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vi_get_misc_attr(void* isp_struct, AK_ISP_MISC_ATTR* p_misc);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_set_flip_mirror(void* isp_struct, int flip_en, int mirror_en);

#if 0   //del pp
/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_set_main_channel_scale(void* isp_struct, int width, int height);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_get_main_channel_scale(void* isp_struct, int* width, int* height);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_set_sub_channel_scale(void* isp_struct, int width, int height);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_get_sub_channel_scale(void* isp_struct, int* width, int* height);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_set_ch3_channel_scale(void* isp_struct, int width, int height);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_get_ch3_channel_scale(void* isp_struct, int* width, int* height);
#endif
/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_set_target_lines(void* isp_struct, int lines);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_check_update_status(void* isp_struct);

/**
*@brief: ak_isp_check isp status
*/
int ak_isp_vo_check_isp_irq_status(void* isp_struct);

#if 0   //del pp
/**
*@brief: ak_isp_check pp status
*/
// int ak_isp_vo_check_pp_irq_status(void* isp_struct);
#endif
/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_clear_irq_status(void* isp_struct, int isp_bit, int pp_bit);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_enable_irq_status(void* isp_struct, int isp_bit, int pp_bit);

/**
*@brief: ak_isp_ae_info
*/
unsigned int ak_isp_vo_get_reg(void* isp_struct, int reg);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_set_cfg_reg(
    void* isp_struct, int regaddr, int value, int bitmask);

#if 0   //del pp
/**
*@brief: ak_isp_ae_info
*/
int ak_isp_enable_buffer_main(void* isp_struct);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_enable_buffer_sub(void* isp_struct);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_enable_buffer_ch3(void* isp_struct);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_enable_buffer(void* isp_struct, enum buffer_id id);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_enable_buffer_main(void* isp_struct, enum buffer_id id);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_enable_buffer_sub(void* isp_struct, enum buffer_id id);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_enable_buffer_ch3(void* isp_struct, enum buffer_id id);
#if 0
/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_disable_buffer(void* isp_struct, enum buffer_id id);
#endif


/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_disable_buffer_main(void* isp_struct, enum buffer_id id);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_disable_buffer_sub(void* isp_struct, enum buffer_id id);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_disable_buffer_ch3(void* isp_struct, enum buffer_id id);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_set_buffer_addr(void* isp_struct, enum buffer_id id,
    unsigned long yaddr_main_chan_addr, unsigned long yaddr_sub_chan_addr,
    unsigned long yaddr_sub_chan3_addr);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_set_main_buffer_addr(
    void* isp_struct, enum buffer_id id, unsigned long yaddr_main_chan_addr);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_set_sub_buffer_addr(
    void* isp_struct, enum buffer_id id, unsigned long yaddr_sub_chan_addr);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_set_ch3_buffer_addr(
    void* isp_struct, enum buffer_id id, unsigned long yaddr_chan3_addr);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_set_main_buffer_addr2(void* isp_struct, int buffer_index,
    unsigned int yaddr, unsigned int uaddr, unsigned int vaddr);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_set_sub_buffer_addr2(void* isp_struct, int buffer_index,
    unsigned int yaddr, unsigned int uaddr, unsigned int vaddr);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_set_ch3_buffer_addr2(void* isp_struct, int buffer_index,
    unsigned int yaddr, unsigned int uaddr, unsigned int vaddr);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_get_using_frame_main_buf_id(void* isp_struct);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_get_using_frame_sub_buf_id(void* isp_struct);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_get_using_frame_ch3_buf_id(void* isp_struct);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_get_using_frame_buf_hwid(void* isp_struct);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_set_ch1_frame_ctrl(
    void* isp_struct, AK_ISP_PP_FRAME_CTRL_ATTR* ch1_frame_ctrl);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_set_ch2_frame_ctrl(
    void* isp_struct, AK_ISP_PP_FRAME_CTRL_ATTR* ch2_frame_ctrl);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_set_ch3_frame_ctrl(
    void* isp_struct, AK_ISP_PP_FRAME_CTRL_ATTR* ch3_frame_ctrl);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_get_ch1_frame_ctrl(
    void* isp_struct, AK_ISP_PP_FRAME_CTRL_ATTR* ch1_frame_ctrl);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_get_ch2_frame_ctrl(
    void* isp_struct, AK_ISP_PP_FRAME_CTRL_ATTR* ch2_frame_ctrl);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_get_ch3_frame_ctrl(
    void* isp_struct, AK_ISP_PP_FRAME_CTRL_ATTR* ch3_frame_ctrl);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_get_ch1_slice_ctrl(
    void* isp_struct, PP_SLICE_CTRL_ATTR* ch1_slice_para);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_set_ch1_slice_ctrl(
    void* isp_struct, PP_SLICE_CTRL_ATTR* ch1_slice_para);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_get_ch2_slice_ctrl(
    void* isp_struct, PP_SLICE_CTRL_ATTR* ch2_slice_para);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_set_ch2_slice_ctrl(
    void* isp_struct, PP_SLICE_CTRL_ATTR* ch2_slice_para);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_get_ch3_slice_ctrl(
    void* isp_struct, PP_SLICE_CTRL_ATTR* ch3_slice_para);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_set_ch3_slice_ctrl(
    void* isp_struct, PP_SLICE_CTRL_ATTR* ch3_slice_para);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_set_pp_src_ctrl_attr(
    void* isp_struct, PP_SRC_PARA_CTRL_ATTR* pp_src_crl);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_get_pp_src_ctrl_attr(
    void* isp_struct, PP_SRC_PARA_CTRL_ATTR* pp_src_crl);

/**
*@brief: ak_isp_ae_info
*/
int isp_set_pp_frame_ctrl_single(
    void* isp_struct, int ch_frame_ctrl, int index);
#endif
/**
*@brief: ak_isp_set_pp_src_format
*/
int ak_isp_set_pp_src_format(void *isp_struct, PP_FORMAT_ATTR *src_format);

/**
*@brief: ak_isp_get_pp_src_ctrl_attr
*/
int ak_isp_get_pp_src_ctrl_attr(void *isp_struct, PP_FORMAT_ATTR *src_format);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_vo_update_setting(void* isp_struct);

/**
*@brief: ak_isp_ae_info
*/
int ak_isp_is_continuous(void* isp_struct);

// int ak_isp_vpp_set_osd(AK_ISP_OSD_ATTR *p_osd);

// int ak_isp_vpp_set_occlusion_attr(AK_ISP_OCCLUSION_ATTR *p_occ);
// int ak_isp_vpp_occlusion_color_attr(AK_ISP_OCCLUSION_COLOR *p_occ_color);

/**
 * @brief: set blc param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_blc:  blc param
 */
int ak_isp_vp_set_blc_attr(void* isp_struct, AK_ISP_BLC_ATTR* p_blc);

/**
 * @brief: get blc param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_blc:  blc param
 */
int ak_isp_vp_get_blc_attr(void* isp_struct, AK_ISP_BLC_ATTR* p_blc);

/**
 * @brief: set lsc param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_lsc:  lsc param
 */
int ak_isp_vp_set_lsc_attr(void* isp_struct, AK_ISP_LSC_ATTR* p_lsc);

/**
 * @brief: get lsc param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_lsc:  lsc param
 */
int ak_isp_vp_get_lsc_attr(void* isp_struct, AK_ISP_LSC_ATTR* p_lsc);

/**
 * @brief: set rgb gamma param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_rgb_gamma:  rgb gamma param
 */
int ak_isp_vp_set_rgb_gamma_attr(
    void* isp_struct, AK_ISP_RGB_GAMMA_ATTR* p_rgb_gamma);

/**
 * @brief: get rgb gamma param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_rgb_gamma:  rgb gamma param
 */
int ak_isp_vp_get_rgb_gamma_attr(
    void* isp_struct, AK_ISP_RGB_GAMMA_ATTR* p_rgb_gamma);

/**
 * @brief: set y gamma param
 * @author: lz
 * @date: 2016-8-26
 * @param [in] *p_y_gamma:  y gamma param
 */
int ak_isp_vp_set_y_gamma_attr(
    void* isp_struct, AK_ISP_Y_GAMMA_ATTR* p_y_gamma);

/**
 * @brief: get y gamma param
 * @author: lz
 * @date: 2016-8-26
 * @param [out] *p_y_gamma:  y gamma param
 */
int ak_isp_vp_get_y_gamma_attr(
    void* isp_struct, AK_ISP_Y_GAMMA_ATTR* p_y_gamma);

/**
 * @brief: set raw gamma param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_raw_lut:  raw gamma param
 */
int ak_isp_vp_set_raw_lut_attr(
    void* isp_struct, AK_ISP_RAW_LUT_ATTR* p_raw_lut);

/**
 * @brief: get raw gamma param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_raw_lut:  raw gamma param
 */
int ak_isp_vp_get_raw_lut_attr(
    void* isp_struct, AK_ISP_RAW_LUT_ATTR* p_raw_lut);

/**
 * @brief: set dpc param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_dpc:  dpc param
 */
int ak_isp_vp_set_dpc_attr(void* isp_struct, AK_ISP_DDPC_ATTR* p_dpc);

/**
 * @brief: get dpc param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_dpc:  dpc param
 */
int ak_isp_vp_get_dpc_attr(void* isp_struct, AK_ISP_DDPC_ATTR* p_dpc);

/**
 * @brief: set raw noise remove param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *nr1:  raw noise remove  param
 */
int ak_isp_vp_set_nr1_attr(void* isp_struct, AK_ISP_NR1_ATTR* p_nr1);

/**
 * @brief: get raw noise remove param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *nr1:  raw noise remove  param
 */
int ak_isp_vp_get_nr1_attr(void* isp_struct, AK_ISP_NR1_ATTR* p_nr1);

/**
 * @brief: set green balance param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_gb:  green balance param
 */
int ak_isp_vp_set_gb_attr(void* isp_struct, AK_ISP_GB_ATTR* p_gb);

/**
 * @brief: get green balance param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_gb:  green balance param
 */
int ak_isp_vp_get_gb_attr(void* isp_struct, AK_ISP_GB_ATTR* p_gb);

/**
 * @brief: set demosaic param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_demo:  demosaic param
 */
int ak_isp_vp_set_demo_attr(void* isp_struct, AK_ISP_DEMO_ATTR* p_demo);

/**
 * @brief: get demosaic param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_demo:  demosaic param
 */
int ak_isp_vp_get_demo_attr(void* isp_struct, AK_ISP_DEMO_ATTR* p_demo);

/**
 * @brief: set color correct param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_ccm:  color correct param
 */
int ak_isp_vp_set_ccm_attr(void* isp_struct, AK_ISP_CCM_ATTR* p_ccm);

/**
 * @brief: set color correct param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_ccm:  color correct param
 */
int ak_isp_vp_get_ccm_attr(void* isp_struct, AK_ISP_CCM_ATTR* p_ccm);

/**
 * @brief: set wdr  param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_wdr: wdr param
 */
int ak_isp_vp_set_wdr_attr(void* isp_struct, AK_ISP_WDR_ATTR* p_wdr);

/**
 * @brief: get wdr  param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_wdr: wdr param
 */
int ak_isp_vp_get_wdr_attr(void* isp_struct, AK_ISP_WDR_ATTR* p_wdr);

/**
 * @brief: set lce  param
 * @author: lizhi
 * @date: 2021-03-01
 * @param [in] *p_lce: lce param
 */
int ak_isp_vp_set_lce_attr(void* isp_struct, AK_ISP_LCE_ATTR* p_lce);

/**
 * @brief: get lce  param
 * @author: lizhi
 * @date: 2021-03-01
 * @param [out] *p_lce: lce param
 */
int ak_isp_vp_get_lce_attr(void* isp_struct, AK_ISP_LCE_ATTR* p_lce);

/**
 * @brief: set yuv noise remove param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_nr2: noise remove param
 */
int ak_isp_vp_set_nr2_attr(void* isp_struct, AK_ISP_NR2_ATTR* p_nr2);

/**
 * @brief: get yuv noise remove param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_nr2: noise remove param
 */
int ak_isp_vp_get_nr2_attr(void* isp_struct, AK_ISP_NR2_ATTR* p_nr2);

/**
 * @brief: set yuv noise remove param
 * @author: wyf
 * @date: 2020-1-06
 * @param [in] *p_uvnr: noise remove param
 */
int ak_isp_vp_set_uvnr_attr(void* isp_struct, AK_ISP_UVNR_ATTR* p_uvnr);

/**
 * @brief: get yuv noise remove param
 * @author: wyf
 * @date: 2020-1-06
 * @param [out] *p_uvnr: noise remove param
 */
int ak_isp_vp_get_uvnr_attr(void* isp_struct, AK_ISP_UVNR_ATTR* p_uvnr);

/**
 * @brief: set 3d noise remove param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_3d_nr: 3d noise remove param
 */
int ak_isp_vp_set_3d_nr_attr(void* isp_struct, AK_ISP_3D_NR_ATTR* p_3d_nr);

/**
 * @brief: get 3d noise remove param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_3d_nr: 3d noise remove param
 */
int ak_isp_vp_get_3d_nr_attr(void* isp_struct, AK_ISP_3D_NR_ATTR* p_3d_nr);

/**
 * @brief: set 3d noise remove reference param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_ref: 3d noise remove referenc param
 */
int ak_isp_vp_set_3d_nr_ref_addr(
    void* isp_struct, AK_ISP_3D_NR_REF_ATTR* p_ref);

/**
 * @brief: get 3d noise remove reference param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_ref: 3d noise remove referenc param
 */
int ak_isp_vp_get_3d_nr_ref_addr(
    void* isp_struct, AK_ISP_3D_NR_REF_ATTR* p_ref);

/**
 * @brief: get 3d noise remove statics param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_3d_nr_stat_info: 3d noise remove statics param
 */
int ak_isp_vp_get_3d_nr_stat_info(
    void* isp_struct, AK_ISP_3D_NR_STAT_INFO* p_3d_nr_stat_info);

/**
 * @brief: get 3d noise reference frame total Bytes
 * @author: lizhi
 * @date: 2021-6-16
 * @param [out] *p_3d_nr_stat_info: 3d noise reference frame total Bytes
 */
int ak_isp_vp_get_3d_nr_ref_size_stat_info(void* isp_struct,
    AK_ISP_3D_NR_REF_SIZE_STAT_INFO* p_3d_nr_ref_size_stat_info);

/**
 * @brief: set sharp param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_sharp: sharp param
 */
int ak_isp_vp_set_sharp_attr(void* isp_struct, AK_ISP_SHARP_ATTR* p_sharp);

/**
 * @brief: get sharp param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_sharp: sharp param
 */
int ak_isp_vp_get_sharp_attr(void* isp_struct, AK_ISP_SHARP_ATTR* p_sharp);

/**
 * @brief: set sharp other param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_sharp_ex: sharp other param
 */
int ak_isp_vp_set_sharp_ex_attr(
    void* isp_struct, AK_ISP_SHARP_EX_ATTR* p_sharp_ex);

/**
 * @brief: get sharp other param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_sharp_ex: sharp other param
 */
int ak_isp_vp_get_sharp_ex_attr(
    void* isp_struct, AK_ISP_SHARP_EX_ATTR* p_sharp_ex);

/**
 * @brief: set false color param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *fcs: false color param
 */
int ak_isp_vp_set_fcs_attr(void* isp_struct, AK_ISP_FCS_ATTR* p_fcs);

/**
 * @brief: get false color param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *fcs: false color param
 */
int ak_isp_vp_get_fcs_attr(void* isp_struct, AK_ISP_FCS_ATTR* p_fcs);

/**
 * @brief: set hue param
 * @author: lz
 * @date: 2016-8-26
 * @param [in] *p_hue:hue param
 */
int ak_isp_vp_set_hue_attr(void* isp_struct, AK_ISP_HUE_ATTR* p_hue);

/**
 * @brief: gethue param
 * @author: lz
 * @date: 2016-8-26
 * @param [in] *p_hue:hue param
 */
int ak_isp_vp_get_hue_attr(void* isp_struct, AK_ISP_HUE_ATTR* p_hue);

/**
 * @brief: set satruration param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_sat: satruration param
 */
int ak_isp_vp_set_saturation_attr(
    void* isp_struct, AK_ISP_SATURATION_ATTR* p_sat);

/**
 * @brief: get satruration param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_sat: satruration param
 */
int ak_isp_vp_get_saturation_attr(
    void* isp_struct, AK_ISP_SATURATION_ATTR* p_sat);

/**
 * @brief: set contrast attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vp_set_contrast_attr(
    void* isp_struct, AK_ISP_CONTRAST_ATTR* p_contrast);

/**
 * @brief: set contrast attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vp_get_contrast_attr(
    void* isp_struct, AK_ISP_CONTRAST_ATTR* p_contrast);

/**
 * @brief: set_rgb2yuv_attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vp_set_rgb2yuv_attr(
    void* isp_struct, AK_ISP_RGB2YUV_ATTR* p_rgb2yuv_attr);

/**
 * @brief: get_rgb2yuv_attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vp_get_rgb2yuv_attr(
    void* isp_struct, AK_ISP_RGB2YUV_ATTR* p_rgb2yuv_attr);

/**
 * @brief: set_effect_attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vp_set_effect_attr(
    void* isp_struct, AK_ISP_EFFECT_ATTR* p_effect_attr);

/**
 * @brief: get_effect_attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vp_get_effect_attr(
    void* isp_struct, AK_ISP_EFFECT_ATTR* p_effect_attr);

#if 0   //del pp
/**
 * @brief: set_mask_color
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vpp_set_mask_color(void* isp_struct, AK_ISP_MASK_COLOR_ATTR* p_mask);

/**
 * @brief: get_mask_color
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vpp_get_mask_color(void* isp_struct, AK_ISP_MASK_COLOR_ATTR* p_mask);

/**
 * @brief: set_main_chan_mask_area_attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vpp_set_main_chan_mask_area_attr(
    void* isp_struct, AK_ISP_MAIN_CHAN_MASK_AREA_ATTR* p_mask);

/**
 * @brief: get_main_chan_mask_area_attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vpp_get_main_chan_mask_area_attr(
    void* isp_struct, AK_ISP_MAIN_CHAN_MASK_AREA_ATTR* p_mask);

/**
 * @brief: set_sub_chan_mask_area_attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vpp_set_sub_chan_mask_area_attr(
    void* isp_struct, AK_ISP_SUB_CHAN_MASK_AREA_ATTR* p_mask);

/**
 * @brief: get_sub_chan_mask_area_attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vpp_get_sub_chan_mask_area_attr(
    void* isp_struct, AK_ISP_SUB_CHAN_MASK_AREA_ATTR* p_mask);

/**
 * @brief: set_osd_mask_color_table_attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vpp_set_osd_mask_color_table_attr(
    void* isp_struct, AK_ISP_OSD_MASK_COLOR_TABLE_ATTR* p_isp_color_table);

/**
 * @brief: get_osd_mask_color_table_attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vpp_get_osd_mask_color_table_attr(
    void* isp_struct, AK_ISP_OSD_MASK_COLOR_TABLE_ATTR* p_isp_color_table);

/**
 * @brief: set_main_channel_osd_context_attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vpp_set_main_channel_osd_context_attr(
    void* isp_struct, AK_ISP_OSD_CONTEXT_ATTR* p_context);

/**
 * @brief: set_sub_channel_osd_context_attr
 * @param [out] *p_contrast: contrast attr
 */
int ak_isp_vpp_set_sub_channel_osd_context_attr(
    void* isp_struct, AK_ISP_OSD_CONTEXT_ATTR* p_context);

/**
 * @brief: set_main_channel_osd_mem_attr
 * @param [out] *p_mem: OSD_MEM_ATTR
 */
int ak_isp_vpp_set_main_channel_osd_mem_attr(
    void* isp_struct, AK_ISP_OSD_MEM_ATTR* p_mem);

/**
 * @brief: set_sub_channel_osd_mem_attr
 * @param [out] *p_mem: OSD_MEM_ATTR
 */
int ak_isp_vpp_set_sub_channel_osd_mem_attr(
    void* isp_struct, AK_ISP_OSD_MEM_ATTR* p_mem);
#endif
/**
 * @brief: set auto focus param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_af:  af  param
 */
int ak_isp_vp_set_af_attr(void* isp_struct, AK_ISP_AF_ATTR* p_af);

/**
 * @brief: get auto focus param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_af:  af  param
 */
int ak_isp_vp_get_af_attr(void* isp_struct, AK_ISP_AF_ATTR* p_af);

/**
 * @brief: get auto focus statics info param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_af_stat_info:  af_stat  param
 */
int ak_isp_vp_get_af_stat_info(
    void* isp_struct, AK_ISP_AF_STAT_INFO* p_af_stat_info);

/**
 * @brief: set white balance  param
 * @author: lizhi
 * @date: 2021-06-16
 * @param [in] *p_type:  white balance  param
 */
int ak_isp_vp_set_wb_attr(void* isp_struct, AK_ISP_WB_ATTR* p_wb_attr);

/**
 * @brief: get white balance  param
 * @author: lizhi
 * @date: 2021-06-16
 * @param [out] *p_type:  white balance  param
 */
int ak_isp_vp_get_wb_attr(void* isp_struct, AK_ISP_WB_ATTR* p_wb_attr);
#if 0
/**
 * @brief: set white balance  type
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_type:  white balance  type  param
 */
int ak_isp_vp_set_wb_type(void *isp_struct,AK_ISP_WB_TYPE_ATTR *p_type);
/**
 * @brief: get white balance  type
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_type:  white balance  type  param
 */
int ak_isp_vp_get_wb_type(void *isp_struct,AK_ISP_WB_TYPE_ATTR *p_type);

/**
 * @brief: set manual white balance
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_mwb:  manual white balance  type  param
 */
int ak_isp_vp_set_mwb_attr(void *isp_struct,AK_ISP_MWB_ATTR *p_mwb);

/**
 * @brief: get manual white balance
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_mwb:  manual white balance  type  param
 */
int ak_isp_vp_get_mwb_attr(void *isp_struct,AK_ISP_MWB_ATTR *p_mwb);

/**
 * @brief: set auto white balance
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_awb:  auto white balance  param
 */
int ak_isp_vp_set_awb_attr(void *isp_struct,AK_ISP_AWB_ATTR *p_awb);

/**
 * @brief: get auto white balance
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_awb:  auto white balance  param
 */
int ak_isp_vp_get_awb_attr(void *isp_struct,AK_ISP_AWB_ATTR *p_awb);

/**
 * @brief: set exp  type
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_exp_type:  exp type  param
 */
int ak_isp_vp_set_exp_type(void *isp_struct,AK_ISP_EXP_TYPE* p_exp_type);

/**
 * @brief: get exp  type
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_exp_type:  exp type  param
 */
int ak_isp_vp_get_exp_type( void *isp_struct,AK_ISP_EXP_TYPE* p_exp_type);

#endif

/**
 * @brief: set auto white balance ex attr
 * @author: wyf
 * @date: 2016-5-16
 * @param [in] *p_awb:  auto white balance  param
 */
int ak_isp_vp_set_awb_ex_attr(void* isp_struct, AK_ISP_AWB_EX_ATTR* p_awb);

/**
 * @brief: get auto white balance ex attr
 * @author: wyf
 * @date: 2016-5-16
 * @param [in] *p_awb:  auto white balance  param
 */
int ak_isp_vp_get_awb_ex_attr(void* isp_struct, AK_ISP_AWB_EX_ATTR* p_awb);

/**
 * @brief: set auto white balance calib attr
 * @author: lizhi
 * @date: 2021-06-16
 * @param [in] *p_awb_calib_attr:  auto white calib  param
 */
int ak_isp_vp_set_awb_calib_attr(
    void* isp_struct, AK_ISP_AWB_CALIB_INFO* p_awb_calib_attr);

/**
 * @brief: get auto white balance calib attr
 * @author: lizhi
 * @date: 2021-06-16
 * @param [out] *p_awb_calib_attr:  auto white calib  param
 */
int ak_isp_vp_get_awb_calib_attr(
    void* isp_struct, AK_ISP_AWB_CALIB_INFO* p_awb_calib_attr);

/**
 * @brief: get awb statics info
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_awb_stat_info:  awb statics info  param
 */
int ak_isp_vp_get_awb_stat_info(
    void* isp_struct, AK_ISP_AWB_STAT_INFO* p_awb_stat_info);

/**
 * @brief: get_awb_run_info
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_awb_stat_info:  awb statics info  param
 */
int ak_isp_vp_get_awb_run_info(
    void* isp_struct, AK_ISP_AWB_RUN_INFO* p_awb_run_info);

#if 0
/**
 * @brief: set auto exposure  param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_ae:  auto exposure  param
 */
int ak_isp_vp_set_ae_attr( void *isp_struct,AK_ISP_AE_ATTR *p_ae);

/**
 * @brief: get auto exposure  param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_ae:  auto exposure  param
 */
int ak_isp_vp_get_ae_attr(void *isp_struct,AK_ISP_AE_ATTR *p_ae);

/**
 * @brief: set manual exposure  param
 * @author: lizhi
 * @date: 2020-07-08
 * @param [in] *p_mae:  manual exposure  param
 */
int ak_isp_vp_set_me_attr(void *isp_struct,AK_ISP_ME_ATTR *p_me);

/**
 * @brief: get manual exposure  param
 * @author: lizhi
 * @date: 2020-06-01
 * @param [in] *p_mae:  manual exposure  param
 */
int  ak_isp_vp_get_me_attr(void *isp_struct,AK_ISP_ME_ATTR *p_me);

/**
 * @brief: get raw hist   running info
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_raw_hist_stat:  raw hist info  param
 */
int ak_isp_vp_get_raw_hist_stat_info(void *isp_struct,
            AK_ISP_RAW_HIST_STAT_INFO *p_raw_hist_stat);

/**
 * @brief: get rgb hist   running info
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_rgb_hist_stat:  rgb hist info  param
 */
int ak_isp_vp_get_rgb_hist_stat_info(void *isp_struct,
            AK_ISP_RGB_HIST_STAT_INFO *p_rgb_hist_stat);

/**
 * @brief: get rgb hist   running info
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_yuv_hist_stat:  yuv hist info  param
 */
int ak_isp_vp_get_yuv_hist_stat_info(void *isp_struct,
            AK_ISP_YUV_HIST_STAT_INFO *p_yuv_hist_stat);

/**
 * @brief: set zone weight param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [in] *p_weight:  weight  param
 */
int ak_isp_vp_set_zone_weight(void *isp_struct,AK_ISP_WEIGHT_ATTR *p_weight);
#endif
/**
 * @brief: set exposure param
 * @author: lizhi
 * @date: 2021-06-16
 * @param [in] *p_exposure_attr:  auto exposure  param
 */
int ak_isp_vp_set_exposure_attr(
    void* isp_struct, AK_ISP_EXPOSURE_ATTR* p_exposure_attr);

/**
 * @brief: get auto exposure  param
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_exposure_attr:  auto exposure  param
 */
int ak_isp_vp_get_exposure_attr(
    void* isp_struct, AK_ISP_EXPOSURE_ATTR* p_exposure_attr);

/**
 * @brief: set ex zone weight attribute
 * @author: wyf
 * @date: 2021-12-21
 * @param [in] *p_ex_zweight:  zone weight attibute
 */
int ak_isp_vp_set_ex_zweight_attr(
    void* isp_struct, AK_ISP_EX_ZONE_WEIGHT_ATTR* p_ex_zweight);
/**
 * @brief: get ex zone weight attribute
 * @author: wyf
 * @date: 2021-12-31
 * @param [in] *p_ex_zweight:  zone weight attibute
 */
int ak_isp_vp_get_ex_zweight_attr(
    void* isp_struct, AK_ISP_EX_ZONE_WEIGHT_ATTR* p_ex_zweight);
/**
 * @brief: set ae route param
 * @author: lizhi
 * @date: 2021-06-16
 * @param [in] *p_ae_route_attr:  auto exposure route param
 */
// int ak_isp_vp_set_ae_route_attr( void *isp_struct,AK_ISP_AE_ROUTE_ATTR*
// p_ae_route_attr);

/**
 * @brief: set ae route param
 * @author: lizhi
 * @date: 2021-06-16
 * @param [out] *p_ae_route_attr:  auto exposure route param
 */
// int ak_isp_vp_get_ae_route_attr(void *isp_struct,AK_ISP_AE_ROUTE_ATTR*
// p_ae_route_attr);

/**
 * @brief: set frame rate param
 * @author: lz
 * @date: 2016-8-29
 * @param [in] *p_frame_rate:  frame rate param
 */
int ak_isp_vp_set_frame_rate(
    void* isp_struct, AK_ISP_FRAME_RATE_ATTR* p_frame_rate);

/**
 * @brief: set frame rate param
 * @author: lz
 * @date: 2016-8-29
 * @param [in] *p_frame_rate:  frame rate param
 */
int ak_isp_vp_get_frame_rate(
    void* isp_struct, AK_ISP_FRAME_RATE_ATTR* p_frame_rate);

/**
 * @brief: get auto  exposure  running info
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_ae_stat:  auto  exposure  running info  param
 */
int ak_isp_vp_get_ae_run_info(
    void* isp_struct, AK_ISP_AE_RUN_INFO* p_ae_run_info);

/**
 * @brief: get auto  exposure  stat info
 * @author: xiepenghe
 * @date: 2016-5-06
 * @param [out] *p_ae_stat:  auto  exposure  stat info  param
 */
int ak_isp_vp_get_ae_stat_info(
    void* isp_struct, AK_ISP_AE_STAT_INFO* p_ae_stat_info);

/* Pclk_Polar not isp function, but described in PG */
AK_ISP_PCLK_POLAR ak_isp_get_pclk_polar(void* isp_struct);

/**
 * @brief: get bits width
 * @return: int
 */
int ak_isp_get_bits_width(void* isp_struct);

/**
 * @brief: set_isp_capturing
 * @return: int
 */
int ak_isp_set_isp_capturing(void* isp_struct, int resume);

/**
 * @brief: irq_work
 * @return: int
 */
int ak_isp_irq_work(void* isp_struct);

/**
 * @brief: ae_work
 * @return: int
 */
int ak_isp_ae_work(void* isp_struct);

/**
 * @brief: ae_work rt
 * @return: int
 */
int ak_isp_ae_work_rt(void* isp_struct);

/**
 * @brief: awb_work
 * @return: int
 */
int ak_isp_awb_work(void* isp_struct);

/**
 * @brief: fast_ae_work
 * @return: int
 */
int ak_isp_fast_ae_work(void* isp_struct, const int (*p)[2]);

/**
 * @brief: fast_awb_work
 * @return: int
 */
int ak_isp_fast_awb_work(void* isp_struct);

/**
 * @brief: set_awb_work_suspend
 * @return: int
 */
int ak_isp_set_awb_work_suspend(void* isp_struct, int ae_suspend_flag);

typedef void (*ISPDRV_CB_PRINTK)(char* format, ...);
typedef void (*ISPDRV_CB_MEMCPY)(void* dst, void* src, unsigned long sz);
typedef void (*ISPDRV_CB_MEMSET)(
    void* ptr, unsigned char value, unsigned long sz);
typedef void* (*ISPDRV_CB_MALLOC)(unsigned long sz);
typedef void (*ISPDRV_CB_FREE)(void* ptr);
typedef void* (*ISPDRV_CB_DMAMALLOC)(unsigned long sz, void* handle);
typedef void (*ISPDRV_CB_DMAFREE)(
    void* ptr, unsigned long sz, unsigned long handle);
typedef void (*ISPDRV_CB_MSLEEP)(int ms);
typedef void (*ISPDRV_CB_CACHE_INVALID)(void);
typedef unsigned long (*ISPDRV_CB_WORD_READ)(void* addr);
typedef void (*ISPDRV_CB_WORD_WRITE)(unsigned long value, void* addr);

typedef struct {
    ISPDRV_CB_PRINTK cb_printk;
    ISPDRV_CB_MEMCPY cb_memcpy;
    ISPDRV_CB_MEMSET cb_memset;
    ISPDRV_CB_MALLOC cb_malloc;
    ISPDRV_CB_FREE cb_free;
    ISPDRV_CB_DMAMALLOC cb_dmamalloc;
    ISPDRV_CB_DMAFREE cb_dmafree;
    ISPDRV_CB_MSLEEP cb_msleep;
    ISPDRV_CB_CACHE_INVALID cb_cache_invalid;
    ISPDRV_CB_WORD_READ cb_word_read;
    ISPDRV_CB_WORD_WRITE cb_word_write;
} AK_ISP_FUNC_CB;

typedef struct sensor_reg_info {
    unsigned short reg_addr;
    unsigned short value;
} AK_ISP_SENSOR_REG_INFO;

typedef struct sensor_init_para {
    unsigned short num;
    AK_ISP_SENSOR_REG_INFO* reg_info;
} AK_ISP_SENSOR_INIT_PARA;

enum sensor_bus_type { BUS_TYPE_RAW, BUS_TYPE_YUV, BUS_TYPE_NUM };

enum scene { SCENE_INDOOR = 0, SCENE_OUTDOOR };

typedef struct sensor_callback {
    int (*sensor_init_func)(void* arg, const AK_ISP_SENSOR_INIT_PARA* para);
    int (*sensor_read_reg_func)(void* arg, const int reg_addr);
    int (*sensor_write_reg_func)(void* arg, const int reg_addr, int value);
    int (*sensor_read_id_func)(void* arg); // no use IIC bus
    int (*sensor_update_a_gain_func)(void* arg, const unsigned int a_gain);
    int (*sensor_update_d_gain_func)(void* arg, const unsigned int d_gain);
    int (*sensor_updata_exp_time_func)(void* arg, unsigned int exp_time);
    int (*sensor_timer_func)(void* arg);
    int (*sensor_set_standby_in_func)(void* arg);
    int (*sensor_set_standby_out_func)(void* arg);
#if 0
    int (*sensor_init_func)(const int id, const AK_ISP_SENSOR_INIT_PARA *para);
    int (*sensor_read_reg_func)(const int id, const int reg_addr);
    int (*sensor_write_reg_func)(const int id, const int reg_addr, int value);
    int (*sensor_read_id_func)(const int id);   //no use IIC bus
    int (*sensor_update_a_gain_func)(const int id, const unsigned int a_gain);
    int (*sensor_update_d_gain_func)(const int id, const unsigned int d_gain);
    int (*sensor_updata_exp_time_func)(const int id, unsigned int exp_time);
    int (*sensor_timer_func)(const int id);
    int (*sensor_set_standby_in_func)(void);
    int (*sensor_set_standby_out_func)(void);
#endif

    int (*sensor_probe_id_func)(void* arg); // use IIC bus
    int (*sensor_get_resolution_func)(void* arg, int* width, int* height);
    int (*sensor_get_mclk_func)(void* arg);
    int (*sensor_get_fps_func)(void* arg);
    int (*sensor_get_valid_coordinate_func)(void* arg, int* x, int* y);
    enum sensor_bus_type (*sensor_get_bus_type_func)(void* arg);
    int (*sensor_get_parameter_func)(void* arg, int param, void* value);

    int (*sensor_set_power_on_func)(void* arg);
    int (*sensor_set_power_off_func)(void* arg);
    int (*sensor_set_fps_func)(void* arg, const int fps);
    // int (*sensor_set_standby_in_func)(const int pwdn_pin, const int
    // reset_pin); int (*sensor_set_standby_out_func)(const int pwdn_pin, const
    // int reset_pin);
    int (*sensor_get_fast_para_func)(void* arg, AK_ISP_AE_INFO* para);


    /*ONLY used for FastAOV*/
    int (*sensor_event_func)(void *arg, int evt);
    int (*sensor_fsync_func)(void *arg);
    int (*sensor_get_current_rb_rows_func)(void* arg);
    int (*sensor_get_aov_flag_func)(void* arg);
} AK_ISP_SENSOR_CB;

struct sensor_cb_info {
    AK_ISP_SENSOR_CB* cb;
    void* arg;
};

typedef struct reserve_param_info {
    int physical_port_id; //for isp->physical_port_id
    int res[3];
} AK_RESERVE_PRARAM_INFO;

/**
 * @brief: isp_struct2_module_switch_sensor
 * @return: void*
 */
void* isp_struct2_module_switch_sensor(void* isp);

/**
 * @brief: isp2_module_curr_sensor
 * @return: int
 */
int isp2_module_curr_sensor(void);

/**
 * @brief: isp2_module_init
 * @return: int
 */
int isp2_module_init(AK_ISP_FUNC_CB* cb, struct sensor_cb_info* sensor_cb,
    void* isp_base, void* pp_base, void** isp_struct, int isp_id);

/**
 * @brief: isp2_module_param_init
 * @return: int
 */
int isp2_module_init_ex(void **isp_struct,AK_RESERVE_PRARAM_INFO * reserve_param_point);

/**
 * @brief: isp2_module_fini
 * @return: void
 */
void isp2_module_fini(void* isp_struct);

/**
 * @brief: isp2_print_reg_table
 * @return: void
 */
void isp2_print_reg_table(void* isp_struct);

/**
 * @brief: ak_isp_get_mem_ptrs
 * @return: void
 */
void ak_isp_get_mem_ptrs(void* isp_struct, struct mem_ptrs* ptrs);

/**
 * @brief: ak_isp_register_sensor
 * @return: int
 */
int ak_isp_register_sensor(void* sensor_info);

/**
 * @brief: ak_isp_get_sensor
 * @return: int
 */
void* ak_isp_get_sensor(int* index);

/**
 * @brief: ak_isp_remove_all_sensors
 * @return: int
 */
void ak_isp_remove_all_sensors(void);

/**
 * @brief: ak_isp_set_td
 * @return: int
 */
int ak_isp_set_td(void* isp_struct);

/**
 * @brief: ak_isp_reload_td
 * @return: int
 */
int ak_isp_reload_td(void* isp_struct);

/**
 * @brief: ak_isp_sensor_cb_init
 * @return: int
 */
int ak_isp_sensor_cb_init(
    void* isp_struct, AK_ISP_SENSOR_INIT_PARA* sensor_para);

/**
 * @brief: ak_isp_get_scene
 */
enum scene ak_isp_get_scene(void* isp_struct);

/**
 * @brief: ak_isp_vpp_mainchn_osdmem_useok
 * @return: int
 */
int ak_isp_vpp_mainchn_osdmem_useok(void* isp_struct);

/**
 * @brief: ak_isp_vpp_subchn_osdmem_useok
 * @return: int
 */

int ak_isp_vpp_subchn_osdmem_useok(void* isp_struct);

/**
 * @brief: ak_isp_get_mdinfo
 * @return: int
 */

int ak_isp_get_mdinfo(void* isp_struct,
    AK_ISP_3D_NR_STAT_INFO** _3d_nr_stat_para, int* width_block_num,
    int* height_block_num, int* block_size);

/**
 * @brief: ak_isp_get_md_array_max_size
 * @return: int
 */
int ak_isp_get_md_array_max_size(void* isp_struct, int* width_block_num,
    int* height_block_num, int* block_size);

/**
 * @brief: _dual_3dnr_work
 * @return: int
 */
int _dual_3dnr_work(void* isp_struct);

/**
 * @brief: ak_isp_set_ae_fast_struct_default
 * @return: int
 */
int ak_isp_set_ae_fast_struct_default(
    void* isp_struct, struct ae_fast_struct* ae_fast);

/**
 * @brief: ak_isp_set_ae_fast_struct
 * @return: int
 */
int ak_isp_set_ae_fast_struct(void* isp_struct, struct ae_fast_struct* ae_fast);

/**
 * @brief: ak_isp_ae_fast_set_ae
 * @return: int
 */
int ak_isp_ae_fast_set_ae(void* isp_struct);

/**
 * @brief: ak_isp_ae_fast_set_isp_d_gain
 * @return: int
 */
int ak_isp_ae_fast_set_isp_d_gain(void* isp_struct);

/**
 * @brief: ak_isp_ae_fast_set_wb
 * @return: int
 */
int ak_isp_ae_fast_set_wb(void* isp_struct);

/**
 * @brief: ak_isp_set_ae_work_suspend
 * @return: int
 */
int ak_isp_set_ae_work_suspend(void* isp_struct, int ae_suspend_flag);

/**
 * @brief: ak_isp_vi_start_capturing_one
 * @return: int
 */
int ak_isp_vi_start_capturing_one(void* isp_struct, int wait);

/**
 * @brief: ak_isp_get_version
 * @return: int
 */
const char* ak_isp_get_version(void* isp_struct);

/**
 * @brief: ak_isp_get_current_block_done
 * @return: int
 */
int ak_isp_get_current_block_done(
    void* isp_struct, enum pp_channel chn, int* slice, int* hwbuf);

/**
 * @brief: ak_isp_set_dvp_attr
 * @return: int
 */
int ak_isp_set_dvp_attr(void* isp_struct, struct dvp_attr* attr);

/**
 * @brief: ak_isp_get_dvp_attr
 * @return: int
 */
int ak_isp_get_dvp_attr(void* isp_struct, struct dvp_attr* attr);

/**
 * @brief: ak_isp_set_mipi_attr
 * @return: int
 */
int ak_isp_set_mipi_attr(void* isp_struct, struct mipi_attr* attr);

/**
 * @brief: ak_isp_get_mipi_attr
 * @return: int
 */
int ak_isp_get_mipi_attr(void* isp_struct, struct mipi_attr* attr);

/**
 * @brief: ak_isp_set_dvp_mipi_common_attr
 * @return: int
 */
int ak_isp_set_dvp_mipi_common_attr(
    void* isp_struct, struct dvp_mipi_common_attr* attr);

    /**
 * @brief: ak_isp_get_dvp_mipi_common_attr
 * @return: int
 */
int ak_isp_get_dvp_mipi_common_attr(
    void* isp_struct, struct dvp_mipi_common_attr* attr);
#if 0   //del pp
/**
 * @brief: ak_isp_set_pp_src_attr
 * @return: int
 */
int ak_isp_set_pp_src_attr(void* isp_struct, struct pp_src_attr* attr);

/**
 * @brief: ak_isp_get_pp_src_attr
 * @return: int
 */
int ak_isp_get_pp_src_attr(void* isp_struct, struct pp_src_attr* attr);

/**
 * @brief: ak_isp_set_pp_chn_attr
 * @return: int
 */
int ak_isp_set_pp_stitch_mode(void *isp_struct, int chn_id, unsigned char join_mode,
    unsigned char join_fisrt,unsigned char join_last);
    
/**
 * @brief: ak_isp_set_pp_chn_attr
 * @return: int
 */
int ak_isp_set_pp_chn_attr(
    void* isp_struct, int chn_id, struct pp_chn_attr* attr);

/**
 * @brief: ak_isp_get_pp_chn_attr
 * @return: int
 */
int ak_isp_get_pp_chn_attr(
    void* isp_struct, int chn_id, struct pp_chn_attr* attr);
#endif
/**
 * @brief: ak_isp_vo_update_regtable_mode_all
 * @return: int
 */
int ak_isp_vo_update_regtable_mode_all(void* isp_struct);

#if 0   //del pp

/**
 * @brief: ak_isp_vo_disable_all_buffer_main
 * @return: int
 */
int  ak_isp_vo_disable_all_buffer_main(void *isp_struct);
/**
 * @brief: ak_isp_vo_disable_all_buffer_sub
 * @return: int
 */
int  ak_isp_vo_disable_all_buffer_sub(void *isp_struct);
/**
 * @brief: ak_isp_vo_disable_all_buffer_ch3
 * @return: int
 */
int  ak_isp_vo_disable_all_buffer_ch3(void *isp_struct);
#endif

#endif
