#ifndef __AK_ISP_IOTRL_H__
#define __AK_ISP_IOTRL_H__

#include "ak_isp_param.h"

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

#if 0
/*
 * first power on, then probe sensor id
 * @RETURN:  <=0: fail; others: success
 * RETURN type: int
 */
// #define AK_ISP_PROBE_SENSOR_ID      _IOR(AKISP_MAGIC,  109, int)
// #define AK_ISP_SET_AE_INIT_INFO     _IOW(AKISP_MAGIC,  110, int)
// #define AK_ISP_GET_SENSOR_AE_INFO   _IOW(AKISP_MAGIC,  111, int)



// #define  AK_ISP_SET_MAIN_CHAN_MASK_INFO _IOR(AKISP_MAGIC,  112, int)
// #define  AK_ISP_GET_MAIN_CHAN_MASK_INFO _IOR(AKISP_MAGIC,  113, int)

// #define  AK_ISP_SET_SUB_CHAN_MASK_INFO  _IOR(AKISP_MAGIC,  114, int)
// #define  AK_ISP_GET_SUB_CHAN_MASK_INFO  _IOR(AKISP_MAGIC,  115, int)

// #define  AK_ISP_SET_MAIN_CHAN_DRAW_AREA _IOR(AKISP_MAGIC,  116, int)
// #define  AK_ISP_GET_MAIN_CHAN_DRAW_AREA _IOR(AKISP_MAGIC,  117, int)

// #define  AK_ISP_SET_SUB_CHAN_DRAW_AREA  _IOR(AKISP_MAGIC,  118, int)
// #define  AK_ISP_GET_SUB_CHAN_DRAW_AREA  _IOR(AKISP_MAGIC,  119, int)

// #define  AK_ISP_SET_MAIN_CHAN_DRAW_INFO _IOR(AKISP_MAGIC,  120, int)
// #define  AK_ISP_GET_MAIN_CHAN_DRAW_INFO _IOR(AKISP_MAGIC,  121, int)

// #define  AK_ISP_SET_SUB_CHAN_DRAW_INFO  _IOR(AKISP_MAGIC,  122, int)
// #define  AK_ISP_GET_SUB_CHAN_DRAW_INFO  _IOR(AKISP_MAGIC,  123, int)

// #define AK_ISP_SET_MAIN_CHAN_MASK_AREA  _IOW(AKISP_MAGIC,  124, int)
// #define AK_ISP_GET_MAIN_CHAN_MASK_AREA  _IOR(AKISP_MAGIC,  125, int)
// #define AK_ISP_SET_SUB_CHAN_MASK_AREA   _IOW(AKISP_MAGIC,  126, int)
// #define AK_ISP_GET_SUB_CHAN_MASK_AREA   _IOR(AKISP_MAGIC,  127, int)
        

// #define AK_ISP_SET_MASK_COLOR       _IOW(AKISP_MAGIC,  128, int)
// #define AK_ISP_GET_MASK_COLOR       _IOR(AKISP_MAGIC,  129, int) 
#endif

#define AK_ISP_VP_SET_WDR_COMB      _IOW(AKISP_MAGIC,  130, int)
#define AK_ISP_VP_GET_WDR_COMB      _IOR(AKISP_MAGIC,  131, int)

#define AK_ISP_GET_DMABUF_FD        _IOR(AKISP_MAGIC, 132, int)

#define AK_ISP_SET_TNR_BUF_SIZE        _IOR(AKISP_MAGIC, 133, int)



//同步AV100的
#define  AK_ISP_AE_WORK                 _IOR(AKISP_MAGIC,  160, int)
#define  AK_ISP_SET_ISP_MODE            _IOR(AKISP_MAGIC,  161, int)
#define  AK_ISP_SWITCH_SENSOR            _IOR(AKISP_MAGIC,  162, int)
#define  AK_ISP_REINIT_SENSOR            _IOR(AKISP_MAGIC,  163, int)
#define  AK_ISP_STANDBY_IN_SENSOR        _IOR(AKISP_MAGIC,  164, int)
#define  AK_ISP_STANDBY_OUT_SENSOR        _IOR(AKISP_MAGIC,  165, int)
#define  AK_ISP_SET_FPS_AND_PWM        _IOR(AKISP_MAGIC,  166, int)
#define  AK_ISP_STANDBY_NIGHT_INFO       _IOR(AKISP_MAGIC,  167, int)
#define  AK_ISP_STANDBY_SENSOR_FSYNC     _IOR(AKISP_MAGIC,  168, int)

//#define  AK_ISP_FAST_AE_INIT             _IOR(AKISP_MAGIC,  169, int)
//#define  AK_ISP_GET_FAST_STATUS          _IOR(AKISP_MAGIC,  172, int)
//#define  AK_ISP_GET_ONE_LINE_CYCLE       _IOR(AKISP_MAGIC,  173, int)
//#define  AK_ISP_GET_HBLIANK_CYCLE        _IOR(AKISP_MAGIC,  174, int)
#define  AK_ISP_GET_BINING_INFO          _IOR(AKISP_MAGIC,  175, int)
#define  AK_ISP_SET_NORMAL_MODE          _IOR(AKISP_MAGIC,  176, int)
#define  AK_ISP_GET_SENSOR_MAX_FPS       _IOR(AKISP_MAGIC,  177, int)
#define  AK_ISP_SET_SENSOR_FPS_DIRECT    _IOR(AKISP_MAGIC,  178, int)
#define  AK_ISP_SET_3DNR_FORMAT_TYPE     _IOR(AKISP_MAGIC,  179, int)


typedef struct isp_dmabuf_info {
    AK_U32 id;
    AK_S32 dmabuf_fd;
}ISP_DMABUF_ATTR;

/*get raw*/
#define RAWDATA_HEADER_SIZE     128
#define RAWDATA_HEADER_MAGIC    0x52415744/*RAWD*/
#define RAWDATA_IMAGE_WIDTH     ((AK_U64)isp->crop.c.width)
#define RAWDATA_IMAGE_HEIGHT    ((AK_U64)isp->crop.c.height)

/*
 * struct vb2_buffer->field
 * struct vb2_v4l2_buffer->field
 */
enum field_define {
    NORMAL_FRAME = 0,
    RAWDATA_FRAME,/*indicate is rawdata*/
    RAWDATA_3DNR_BUF,/*indicate is rawdata & using 3DNR buffer*/
};

enum rawdata_format {
    BAYER_RAWDATA = 0,
    YUV422_16B_DATA,
};

struct rawdata_header {
    unsigned int magic;
    int header_size;
    enum rawdata_format format;
    int rawdata_size;
    int bits_width;
    int width;
    int height;
};
/*get raw end*/

struct isp_flip_mirror_info {
    int flip_en;
    int mirror_en;
};

/* Blow is the params that user can adjust in real time */
typedef struct {
    int id;
    unsigned char data[128];
} AK_ISP_USER_PARAM;


struct bining_info
{
    unsigned int width;
    unsigned int height;
};

//////////////////////////////////////////////////////////////////////////////


#if 0
/*
struct isp_channel2_info {
    int width;
    int height;
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
*/
#endif

#endif
