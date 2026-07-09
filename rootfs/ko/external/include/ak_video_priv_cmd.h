#ifndef __AK_VIDEO_PRIV_CMD_H__
#define __AK_VIDEO_PRIV_CMD_H__


#ifndef CONFIG_MACH_KM01A
/*this is the include file for video device*/
#include <linux/videodev2.h>
#define MD_HEADER_SIZE  128
#define MD_MAGIC        0xdba8f04e
#define MD_VERSION      0x0000

/*rawdata*/
#define RAWDATA_HEADER_SIZE     128
#define RAWDATA_HEADER_MAGIC    0x52415744/*RAWD*/


#ifdef CONFIG_SYS_FAST_LAUNCH
/*fast ae*/
#define FAST_AE_ENABLE
#endif

#define AE_TABLE_CNT (3)
#define SUB_TABLE_CNT (2)
#define AE_TABLE_SIZE (150)
#define MAX_LUMI_VALUE (255)

typedef struct ak_isp_fast_ae_info
{
    unsigned short a_gain;                         //a-gain
    unsigned short d_gain;                        //d-gain
    unsigned short isp_d_gain;                    //isp-d-gain
    unsigned short exp_time;                    //exposure time
    unsigned char is_default;
    unsigned char reserved[2];
}AK_ISP_FAST_AE_INFO;


enum ak_video_priv_cmd {
    /*SET commad start*/
    SET_CAPTURE_RAWDATA = 0x100,    //none arg, capture one raw frame
    SET_GLOBAL_CROP,    //struct priv_global_crop, set global crop
    SET_SLICE_NUM,      //struct priv_slice_num, set slice num per frame
    SET_DONE_MODE,      //struct priv_done_mode
    SET_BLOCK_NUM,      //struct priv_block_num, set block num per frame
    //struct priv_chn_crop, set chn crop but not to change output resolution
    SET_CHN_CROP,       
    SET_DUAL_RATIO,     //struct priv_dual_ratio, set dual sensors ratio
    //struct priv_dual_mode, set isp working with dual or single sensor(s)
    SET_DUAL_MODE,
    SET_DUAL_STITCH_MODE,   //struct priv_dual_stitch_mode, set stitch for dual
    //struct priv_chn_max_dma_size, set max dma size of the chn
    SET_CHN_MAX_DMA_SIZE,
    SET_CHN_BUFS_RELEASE,   //set the chn release memory of all buffers
    SET_CAPTURE_MODE,      //struct priv_capture_mode
    SET_QUICK_START_FLAG,      //struct priv_quick_start

    /*GET commad start*/
    GET_SENSOR_ID = 0x300,  //struct priv_sensor_id
    GET_PHYADDR,            //struct priv_phyaddr
    GET_MAX_EXP_FOR_FPS,    //struct priv_max_exp_for_fps
    GET_Y_DATA,
    GET_SENEOR_CUR_FPS,
    GET_SENSOR_RAW_SEQUE,
    GET_LIGHT_SENSOR_AE_TABLE,
    GET_CAPTURE_MODE,
    GET_YUV_VADDR,
};

enum rawdata_format {
    BAYER_RAWDATA = 0,
    YUV422_16B_DATA,
};

enum done_mode {
    FRAME_DONE_MODE = 0,
    SLICE_DONE_MODE,
};

/*
 * struct vb2_buffer->field
 * struct vb2_v4l2_buffer->field
 */
enum field_define {
    NORMAL_FRAME = 0,
    RAWDATA_FRAME,/*indicate is rawdata*/
    RAWDATA_3DNR_BUF,/*indicate is rawdata & using 3DNR buffer*/
};

enum stitch_mode {
    DISABLE_STITCH_MODE = 0,//no stitch
    VERTICAL_STITCH_MODE,//vertical stitch
    HORIZONTAL_STITCH_MODE,//horizontal stitch
};

struct priv_sensor_id {
    int type;
    int sensor_id;
};

struct priv_yuv_vaddr {
    int type;
    int yuv_vaddr;
};

struct priv_phyaddr {
    int type;
    unsigned int phyaddr;/*set index, then return phyaddr for index*/
};

struct priv_y_data {
    int type;
    unsigned char *vaddr;/*vaddr of YUV buf*/
    int len;
    int pos_start_x;
    int pos_start_y;
    int width;
    int height;
};

struct priv_max_exp_for_fps {
    int type;
    int fps;
    int max_exp;
};

struct priv_global_crop {
    int type;
    struct v4l2_crop crop;
};

struct priv_slice_num {
    int type;
    int slice_num;
};

struct priv_block_num {
    int type;
    int block_num;
};

struct priv_done_mode {
    int type;
    enum done_mode mode;
};

struct priv_chn_crop_attr {
    struct v4l2_crop crop;
    int step;
};

struct priv_chn_crop {
    int type;
    struct priv_chn_crop_attr crop;
};



struct priv_dual_ratio_attr {
    int ratio;
};

struct priv_capture_mode {
    int type;
    int capture_mode;
};

struct priv_quick_start {
    int type;
    int quick_start_flag;
};

struct priv_dual_ratio {
    int type;
    struct priv_dual_ratio_attr dual_ratio;
};

struct priv_dual_mode {
    int type;
    int dual_enable;//=0 single mode, others dual mode
};

#if defined(CONFIG_MACH_AK3918AV130) || defined(__CHIP_AK3918AV130_SERIES)
struct stitch_attr {
    enum stitch_mode stitch_mode;
    int stitch_num;
    int index;      //拼接图像内所处的位置，0开始
    int global_id;  //拼接后的码流标识，0开始。不区分主次三通道。应用自行定义
};
#else
struct stitch_attr {
    enum stitch_mode stitch_mode;
    int index;  //valid only when stitch is enable
};
#endif

struct priv_dual_stitch_mode {
    int type;
    struct stitch_attr stitch;
};

struct priv_chn_max_dma_size {
    int type;
    int max_dma_size;
};

/*
 * md header:
 * magic,
 * version,
 * width blocks num,
 * height blocks num,
 * md level,
 * .......(128BYTES header),
 * md data
 * */
struct md_header {
    /*total 128 BYTES*/
    unsigned int magic;//4bytes
    unsigned short version;//2bytes
    unsigned short width_block_num;//2bytes
    unsigned short height_block_num;//2bytes
    unsigned short block_size;//2bytes
    unsigned short md_level;//2bytes
    unsigned char reserved[114];//114bytes
};

/*
 * struct rawdata_header
 * @magic:      magic value
 * @header_size:    only the size of the header
 * @format:     input data format
 * @rawdata_size:   only the rawdata size exclude header
 * @bits_width:     bits width of a pixsel
 * @width:          width pixsel of the rawdata picture
 * @height:         height pixsel of the rawdata picture
 */
struct rawdata_header {
    unsigned int magic;
    int header_size;
    enum rawdata_format format;
    int rawdata_size;
    int bits_width;
    int width;
    int height;
};

typedef struct ak_isp_3d_nr_mmap_attr {
    unsigned int width;
    unsigned int height;
    unsigned int size_3d;
    unsigned int ysize_3d;
    unsigned int usize_3d;
    unsigned int vsize_3d;
}AK_ISP_3D_NR_MMAP_ATTR;

#endif
#endif
/*end of file*/
