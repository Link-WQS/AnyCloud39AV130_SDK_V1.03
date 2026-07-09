#ifndef __AK_VIDEO_APP_H__
#define __AK_VIDEO_APP_H__

#define PAD_ID_INVAID   (-1)

/*
 * vicap isp pp都会用到这个结构体来组成pipeline
 */

/*所有set &get param都用到的通用结构体*/
struct param_struct {
    AK_U32 reserved[10];
    AK_U32 cmd;
    AK_U32 arg; // U64因为可能传递用户空间指针
};

struct ak_video_app_link {
    AK_S32 prev_pad_id;
    AK_S32 next_pad_id;
};

struct current_pad_id {
    AK_S32 pad_id;
};

//TODO 中间件有使用到，但是驱动还未支持的结构体
enum ak_video_priv_cmd {
    /*SET commad start*/
    // SET_CAPTURE_RAWDATA = 0x100,    //none arg, capture one raw frame
    // SET_GLOBAL_CROP,    //struct priv_global_crop, set global crop
    // SET_SLICE_NUM,      //struct priv_slice_num, set slice num per frame
    // SET_DONE_MODE,      //struct priv_done_mode
    // SET_BLOCK_NUM,      //struct priv_block_num, set block num per frame
    // //struct priv_chn_crop, set chn crop but not to change output resolution
    // SET_CHN_CROP,
    SET_DUAL_RATIO,     //struct priv_dual_ratio, set dual sensors ratio
    //struct priv_dual_mode, set isp working with dual or single sensor(s)
    SET_DUAL_MODE,
    SET_DUAL_STITCH_MODE,   //struct priv_dual_stitch_mode, set stitch for dual
    // //struct priv_chn_max_dma_size, set max dma size of the chn
    // SET_CHN_MAX_DMA_SIZE,
    // SET_CHN_BUFS_RELEASE,   //set the chn release memory of all buffers

    // /*GET commad start*/
    // GET_SENSOR_ID = 0x300,  //struct priv_sensor_id
    // GET_PHYADDR,            //struct priv_phyaddr
    // GET_MAX_EXP_FOR_FPS,    //struct priv_max_exp_for_fps
};

struct priv_dual_mode {
    int type;
    int dual_enable;//=0 single mode, others dual mode
};

struct priv_dual_ratio_attr {
    int ratio;
};

struct priv_dual_ratio {
    int type;
    struct priv_dual_ratio_attr dual_ratio;
};

#endif
