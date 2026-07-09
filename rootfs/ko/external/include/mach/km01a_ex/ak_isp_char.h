#ifndef __AK_ISP_CHAR_H__
#define __AK_ISP_CHAR_H__

/#include "../km01a_ex/ak_isp_param.h"

#if 0
#define AKISP_MAGIC 'I'
#define AK_ISP_VP_GET_PWL		 	_IOR(AKISP_MAGIC,  1, int)
#define AK_ISP_VP_SET_PWL		 	_IOR(AKISP_MAGIC,  2, int)
#define AK_ISP_GET_RAWWDRCMB	 	_IOR(AKISP_MAGIC,  3, int)
#define AK_ISP_SET_RAWWDRCMB	 	_IOR(AKISP_MAGIC,  4, int)
#define AK_ISP_GET_RAWWDR_HIST	 	_IOR(AKISP_MAGIC,  5, int)
#define AK_ISP_VP_GET_BLC           	_IOR(AKISP_MAGIC,  6, int)
#define AK_ISP_VP_SET_BLC           	_IOW(AKISP_MAGIC,  7, int)
#define AK_ISP_VP_GET_LSC           	_IOR(AKISP_MAGIC,  7, int)
#define AK_ISP_VP_SET_LSC           	_IOW(AKISP_MAGIC,  8, int)
#define AK_ISP_VP_GET_MESH_LSC      	_IOR(AKISP_MAGIC,  9, int)
#define AK_ISP_VP_SET_MESH_LSC      	_IOW(AKISP_MAGIC,  10, int)
#define AK_ISP_SET_RAW_G_HIST         _IOW(AKISP_MAGIC,  11, int)
#define AK_ISP_GET_RAW_G_HIST         _IOR(AKISP_MAGIC,  12, int)
#define AK_ISP_GET_RAW_HIST_STAT    _IOR(AKISP_MAGIC,  13, int)
#define AK_ISP_VP_GET_DRC           	_IOR(AKISP_MAGIC,  14, int)
#define AK_ISP_VP_SET_DRC           	_IOW(AKISP_MAGIC,  15, int)
#define AK_ISP_VP_GET_RAW_LUT       	_IOR(AKISP_MAGIC,  16, int)
#define AK_ISP_VP_SET_RAW_LUT       	_IOW(AKISP_MAGIC,  17, int)
#define AK_ISP_VP_GET_GB            	_IOR(AKISP_MAGIC,  18, int)
#define AK_ISP_VP_SET_GB            	_IOW(AKISP_MAGIC,  19, int)
#define AK_ISP_GET_DPC           	_IOR(AKISP_MAGIC,  20, int)
#define AK_ISP_SET_DPC           	_IOW(AKISP_MAGIC,  21, int)
#define AK_ISP_VP_GET_RAW_NR1    	_IOR(AKISP_MAGIC,  22, int)
#define AK_ISP_VP_SET_RAW_NR1    	_IOW(AKISP_MAGIC,  23, int)
#define AK_ISP_VP_GET_DEMO       	_IOR(AKISP_MAGIC,  24, int)
#define AK_ISP_VP_SET_DEMO       	_IOW(AKISP_MAGIC,  25, int)
#define AK_ISP_GET_CCM              _IOR(AKISP_MAGIC,  26, int)
#define AK_ISP_SET_CCM              _IOW(AKISP_MAGIC,  27, int) 
#define AK_ISP_SET_RGB_HIST         _IOW(AKISP_MAGIC,  28, int)
#define AK_ISP_GET_RGB_HIST         _IOR(AKISP_MAGIC,  29, int) 
#define AK_ISP_GET_RGB_HIST_STAT    _IOR(AKISP_MAGIC,  30, int) 
#define AK_ISP_GET_RGB_GAMMA        _IOR(AKISP_MAGIC,  31, int)
#define AK_ISP_SET_RGB_GAMMA        _IOW(AKISP_MAGIC,  32, int)
#define AK_ISP_SET_RGB2YUV          _IOW(AKISP_MAGIC,  33, int)
#define AK_ISP_GET_RGB2YUV          _IOR(AKISP_MAGIC,  34, int)
#define AK_ISP_SET_Y_HIST           _IOW(AKISP_MAGIC,  35, int)
#define AK_ISP_GET_Y_HIST           _IOR(AKISP_MAGIC,  36, int)
#define AK_ISP_GET_Y_HIST_STAT      _IOR(AKISP_MAGIC,  37, int)
#define AK_ISP_SET_CONTRAST         _IOW(AKISP_MAGIC,  38, int)
#define AK_ISP_GET_CONTRAST         _IOR(AKISP_MAGIC,  39, int)
#define AK_ISP_SET_Y_GAMMA          _IOW(AKISP_MAGIC,  40, int)
#define AK_ISP_GET_Y_GAMMA          _IOR(AKISP_MAGIC,  41, int)
#define AK_ISP_SET_HDR              _IOW(AKISP_MAGIC,  42, int)
#define AK_ISP_GET_HDR              _IOR(AKISP_MAGIC,  43, int)
#define AK_ISP_SET_SAT              _IOW(AKISP_MAGIC,  44, int)
#define AK_ISP_GET_SAT              _IOR(AKISP_MAGIC,  45, int)
#define AK_ISP_SET_LCE              _IOR(AKISP_MAGIC,  46, int)
#define AK_ISP_GET_LCE              _IOR(AKISP_MAGIC,  47, int)
#define AK_ISP_SET_3D_NR            _IOW(AKISP_MAGIC,  48, int)
#define AK_ISP_GET_3D_NR            _IOR(AKISP_MAGIC,  49, int)
#define AK_ISP_SET_3D_NR_REF        _IOW(AKISP_MAGIC,  50, int)
#define AK_ISP_GET_3D_NR_REF        _IOR(AKISP_MAGIC,  51, int)
#define AK_ISP_SET_SHARP            _IOW(AKISP_MAGIC,  52, int)
#define AK_ISP_GET_SHARP            _IOR(AKISP_MAGIC,  53, int)
#define AK_ISP_SET_SHARP_EX         _IOW(AKISP_MAGIC,  54, int)
#define AK_ISP_GET_SHARP_EX         _IOR(AKISP_MAGIC,  55, int)
#define AK_ISP_GET_FCS              _IOR(AKISP_MAGIC,  56, int)
#define AK_ISP_SET_FCS              _IOW(AKISP_MAGIC,  57, int)
#define AK_ISP_SET_Y_NR2            _IOW(AKISP_MAGIC,  58, int)
#define AK_ISP_GET_Y_NR2            _IOR(AKISP_MAGIC,  59, int)
#define AK_ISP_SET_UVNR             _IOW(AKISP_MAGIC,  60, int)
#define AK_ISP_GET_UVNR             _IOR(AKISP_MAGIC,  61, int)
#define AK_ISP_SET_HUE              _IOW(AKISP_MAGIC,  62, int)
#define AK_ISP_GET_HUE              _IOR(AKISP_MAGIC,  63, int)
#define AK_ISP_SET_YUV_EFFECT       _IOW(AKISP_MAGIC,  64, int)
#define AK_ISP_GET_YUV_EFFECT       _IOR(AKISP_MAGIC,  65, int)


#define AK_ISP_SET_AE_ROUTE_ATTR   _IOR(AKISP_MAGIC,  66, int)
#define AK_ISP_GET_AE_ROUTE_ATTR   _IOR(AKISP_MAGIC,  67, int)
#define AK_ISP_GET_3D_NR_REF_SIZE  _IOR(AKISP_MAGIC,  68, int)
#define AK_ISP_SET_EXPOSURE_ATTR   _IOR(AKISP_MAGIC,  69, int)
#define AK_ISP_GET_EXPOSURE_ATTR   _IOR(AKISP_MAGIC,  70, int)
#define AK_ISP_GET_AWB_RUN_INFO    _IOR(AKISP_MAGIC,  71, int)
#define AK_ISP_GET_AE_STAT_INFO    _IOR(AKISP_MAGIC,  72, int)

#define AK_ISP_SET_FRAME_RATE       _IOW(AKISP_MAGIC,  73, int)
#define AK_ISP_GET_FRAME_RATE       _IOR(AKISP_MAGIC,  74, int)
#define AK_ISP_SET_AE               _IOW(AKISP_MAGIC,  75, int)
#define AK_ISP_GET_AE               _IOR(AKISP_MAGIC,  76, int)
#define AK_ISP_GET_AE_RUN_INFO      _IOR(AKISP_MAGIC,  77, int)
#define AK_ISP_SET_EX_ZWEIGHT       _IOR(AKISP_MAGIC,  78, int)
#define AK_ISP_GET_EX_ZWEIGHT       _IOR(AKISP_MAGIC,  79, int)
#define AK_ISP_SET_WDR_AE		_IOR(AKISP_MAGIC,  80, int)
#define AK_ISP_GET_WDR_AE		_IOR(AKISP_MAGIC,  81, int)


#define AK_ISP_SET_WB              _IOR(AKISP_MAGIC,  82, int)
#define AK_ISP_GET_WB              _IOR(AKISP_MAGIC,  83, int)
#define AK_ISP_SET_WB_FINE         _IOR(AKISP_MAGIC,  84, int)
#define AK_ISP_GET_WB_FINE         _IOR(AKISP_MAGIC,  85, int)
#define AK_ISP_SET_AWB_CALIB_INFO  _IOR(AKISP_MAGIC,  86, int)
#define AK_ISP_GET_AWB_CALIB_INFO  _IOR(AKISP_MAGIC,  87, int)
#define AK_ISP_GET_AWB_STAT_INFO   _IOR(AKISP_MAGIC,  88, int)
#define AK_ISP_GET_AWB_BLK_STAT_INFO    _IOR(AKISP_MAGIC,  89, int)
#define AK_ISP_SET_AWB_EX           _IOW(AKISP_MAGIC,  90, int)
#define AK_ISP_GET_AWB_EX           _IOR(AKISP_MAGIC,  91, int)
#define AK_ISP_SET_AF               _IOW(AKISP_MAGIC,  92, int)
#define AK_ISP_GET_AF               _IOR(AKISP_MAGIC,  93, int)
#define AK_ISP_GET_AF_STAT          _IOR(AKISP_MAGIC,  94, int)


#define AK_ISP_SET_MISC_ATTR        _IOW(AKISP_MAGIC,  95, int)
#define AK_ISP_GET_MISC_ATTR        _IOR(AKISP_MAGIC,  96, int)
        
  
#define AK_ISP_INIT_SENSOR_DEV      _IOW(AKISP_MAGIC,  97, int)
#define AK_ISP_SET_3D_NR_PHYADDR    _IOW(AKISP_MAGIC,  98, int)
#define AK_ISP_SET_SENSOR_REG       _IOW(AKISP_MAGIC,  99, int)
#define AK_ISP_GET_SENSOR_REG       _IOR(AKISP_MAGIC,  100, int)
#define AK_ISP_SET_USER_PARAMS      _IOW(AKISP_MAGIC,  101, int)
#define AK_ISP_GET_SENSOR_ID        _IOR(AKISP_MAGIC,  102, int)
#define AK_ISP_SET_ISP_CAPTURING    _IOW(AKISP_MAGIC,  103, int)
#define AK_ISP_SET_FLIP_MIRROR      _IOW(AKISP_MAGIC,  104, int)
#define AK_ISP_SET_SENSOR_FPS       _IOW(AKISP_MAGIC,  105, int)
#define AK_ISP_GET_SENSOR_FPS       _IOR(AKISP_MAGIC,  106, int)
         
#define  AK_ISP_GET_WORK_SCENE      _IOR(AKISP_MAGIC,  107, int)
#define AK_ISP_GET_3D_NR_STAT_INFO  _IOR(AKISP_MAGIC,  108, int)


/*
 * first power on, then probe sensor id
 * @RETURN:  <=0: fail; others: success
 * RETURN type: int
 */
#define AK_ISP_PROBE_SENSOR_ID      _IOR(AKISP_MAGIC,  109, int)
#define AK_ISP_SET_AE_INIT_INFO     _IOW(AKISP_MAGIC,  110, int)
#define AK_ISP_GET_SENSOR_AE_INFO   _IOW(AKISP_MAGIC,  111, int)



#define  AK_ISP_SET_MAIN_CHAN_MASK_INFO _IOR(AKISP_MAGIC,  112, int)
#define  AK_ISP_GET_MAIN_CHAN_MASK_INFO _IOR(AKISP_MAGIC,  113, int)

#define  AK_ISP_SET_SUB_CHAN_MASK_INFO  _IOR(AKISP_MAGIC,  114, int)
#define  AK_ISP_GET_SUB_CHAN_MASK_INFO  _IOR(AKISP_MAGIC,  115, int)

#define  AK_ISP_SET_MAIN_CHAN_DRAW_AREA _IOR(AKISP_MAGIC,  116, int)
#define  AK_ISP_GET_MAIN_CHAN_DRAW_AREA _IOR(AKISP_MAGIC,  117, int)

#define  AK_ISP_SET_SUB_CHAN_DRAW_AREA  _IOR(AKISP_MAGIC,  118, int)
#define  AK_ISP_GET_SUB_CHAN_DRAW_AREA  _IOR(AKISP_MAGIC,  119, int)

#define  AK_ISP_SET_MAIN_CHAN_DRAW_INFO _IOR(AKISP_MAGIC,  120, int)
#define  AK_ISP_GET_MAIN_CHAN_DRAW_INFO _IOR(AKISP_MAGIC,  121, int)

#define  AK_ISP_SET_SUB_CHAN_DRAW_INFO  _IOR(AKISP_MAGIC,  122, int)
#define  AK_ISP_GET_SUB_CHAN_DRAW_INFO  _IOR(AKISP_MAGIC,  123, int)

#define AK_ISP_SET_MAIN_CHAN_MASK_AREA  _IOW(AKISP_MAGIC,  124, int)
#define AK_ISP_GET_MAIN_CHAN_MASK_AREA  _IOR(AKISP_MAGIC,  125, int)
#define AK_ISP_SET_SUB_CHAN_MASK_AREA   _IOW(AKISP_MAGIC,  126, int)
#define AK_ISP_GET_SUB_CHAN_MASK_AREA   _IOR(AKISP_MAGIC,  127, int)
        

#define AK_ISP_SET_MASK_COLOR       _IOW(AKISP_MAGIC,  128, int)
#define AK_ISP_GET_MASK_COLOR       _IOR(AKISP_MAGIC,  129, int)  

#define AK_ISP_VP_SET_WDR_COMB      _IOW(AKISP_MAGIC,  130, int)
#define AK_ISP_VP_GET_WDR_COMB      _IOR(AKISP_MAGIC,  131, int)
#endif

/* Blow is the params that user can adjust in real time */
typedef struct {
    int id;
    unsigned char data[128];
} AK_ISP_USER_PARAM;

struct isp_zoom_info {
    int channel;
    int cut_xpos;
    int cut_ypos;
    int cut_width;
    int cut_height;
    int out_width;
    int out_height;
};

struct isp_channel2_info {
    int width;
    int height;
};

struct isp_mask_draw_area {
    unsigned short start_xpos;
    unsigned short end_xpos;
    unsigned short start_ypos;
    unsigned short end_ypos;
    unsigned char enable;
};

struct isp_mask_draw_area_info {
    struct isp_mask_draw_area win[14];
};

struct mask_masic_attr{
    unsigned short  mosai_size_hor;
    unsigned short  mosai_size_vec;
};

#define MASK_IMAGE_LAYER_BEGIN_INDEX 10
#define MASK_DRAW_IMAGE_LAYER_END_INDEX 14

struct isp_mask_draw_config {
    unsigned char color_type;   //0x0 = 不使用0x1 = 马赛克0x2 = 纯色矩形0x3 = 纯色矩形框
    //调色板索引，马赛克模式下是alpha值，都为5bit宽度 索引色下draw:16~18 mask:19
    unsigned short  fgc_cfg;
    unsigned short  aux_cfg;    //矩形边框线宽 4bit宽度 0~16
};

#define MASK_DRAW_LAYER_NUM 14

struct isp_mask_draw_info {
    struct isp_mask_draw_config isp_mask_draw_config[MASK_DRAW_LAYER_NUM];
    struct mask_masic_attr mask_masic_attr;
    int draw_rect_frame_num;
};

struct isp_gamma_info {
    int value;
};

struct isp_saturation_info {
    int value;
};

struct isp_brightness_info {
    int value;
};

struct isp_contrast_info {
    int value;
};

struct isp_sharp_info {
    int value;
};

struct isp_power_line_freq_info {
    int value;
};

struct isp_flip_mirror_info {
    int flip_en;
    int mirror_en;
};

typedef enum osd_mask_color_region {
    COLOR_TABLE_OSD_REGION = 0,
    COLOR_TABLE_DRAW_REGION,
    COLOR_TABLE_MASK_REGION,
} OSD_MASK_COLOR_REGION;

struct isp_osd_color_table_attr {
    unsigned int osd_color_table[16];
    unsigned int draw_color_table[3];
    unsigned int mask_color_table;

    OSD_MASK_COLOR_REGION color_table_region;
};

typedef enum isp_osd_channel {
    ISP_OSD_CHN0 = 0,
    ISP_OSD_CHN1,
    ISP_OSD_CHN2,
    ISP_OSD_CHN3,
    ISP_OSD_CHN_NUM
} ISP_OSD_CHANNEL;

typedef enum isp_osd_mode {
    ISP_OSD_CLOSE_MODE = 0,
    ISP_OSD_MONO_MODE,
    ISP_OSD_COLOR_MODE,
} ISP_OSD_MODE;


struct isp_osd_context_attr {
    ISP_OSD_CHANNEL chn;
    unsigned char   *osd_context_addr;
    unsigned int    osd_width;
    unsigned int    osd_height;
    unsigned short  start_xpos;
    unsigned short  start_ypos;
    ISP_OSD_MODE    mode;
    unsigned int    size;
};

struct isp_osd_mem_attr {
    ISP_OSD_CHANNEL chn;
    unsigned char   *dma_paddr; 
    unsigned char   *dma_vaddr;
    unsigned int    size;
};

#define AK_ISP_USER_CID_SET_ZOOM            _IOW(AKISP_MAGIC,  0x00010000, int)
#define AK_ISP_USER_CID_SET_SUB_CHANNEL     _IOW(AKISP_MAGIC,  0x00010001, int)
#define AK_ISP_USER_CID_SET_OCCLUSION       _IOW(AKISP_MAGIC,  0x00010002, int)
#define AK_ISP_USER_CID_SET_OCCLUSION_COLOR _IOW(AKISP_MAGIC,  0x00010003, int)
#define AK_ISP_USER_CID_SET_GAMMA           _IOW(AKISP_MAGIC,  0x00010004, int)
#define AK_ISP_USER_CID_SET_SATURATION      _IOW(AKISP_MAGIC,  0x00010005, int)
#define AK_ISP_USER_CID_SET_BRIGHTNESS      _IOW(AKISP_MAGIC,  0x00010006, int)
#define AK_ISP_USER_CID_SET_CONTRAST        _IOW(AKISP_MAGIC,  0x00010007, int)
#define AK_ISP_USER_CID_SET_SHARPNESS       _IOW(AKISP_MAGIC,  0x00010008, int)

#define AK_ISP_USER_CID_SET_POWER_LINE_FREQUENCY \
            _IOW(AKISP_MAGIC,  0x00010009, int)
            
#define AK_ISP_USER_CID_SET_OSD_COLOR_TABLE_ATTR \
            _IOW(AKISP_MAGIC,  0x0001000a, int)
            
#define AK_ISP_USER_CID_SET_MAIN_CHANNEL_OSD_CONTEXT_ATTR \
            _IOW(AKISP_MAGIC,  0x0001000b, int)
            
#define AK_ISP_USER_CID_SET_SUB_CHANNEL_OSD_CONTEXT_ATTR \
            _IOW(AKISP_MAGIC,  0x0001000c, int)
            
#define AK_ISP_USER_CID_SET_MAIN_CHANNEL_OSD_MEM_ATTR \
            _IOW(AKISP_MAGIC,  0x0001000d, int)
            
#define AK_ISP_USER_CID_SET_SUB_CHANNEL_OSD_MEM_ATTR \
            _IOW(AKISP_MAGIC,  0x0001000e, int)

/*
*akisp_init
*/
int akisp_init(struct sensor_cb_info *cb0_info,
                struct sensor_cb_info *cb1_info);

/*
*akisp_exit
*/
void akisp_exit(void);

#endif
