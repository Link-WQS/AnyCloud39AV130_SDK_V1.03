#ifndef __AK_VENC_BRIDGE_H__
#define __AK_VENC_BRIDGE_H__

#include <linux/proc_fs.h>

#define NO_MISSING  (0)
#define MISSING     (1)

enum ENC_MODE {
    FRAME_MODE = 0,
    SLICE_MODE,
};

typedef enum AK_Bridge_Mode {
    AK_BRIDGE_BYPASS_MODE = 0,
    AK_BRIDGE_BLOCKING_MODE = 1,
    AK_BRIDGE_FULL_CONTROL_MODE = 2,
    AK_BRIDGE_HALT_MODE = 3
} AK_BridgeMode;

#if defined(CONFIG_MACH_KM01A) || defined(CONFIG_MACH_AK3918AV130)
enum BRIDGE_CHANNEL_STATE {
    BRIDGE_CHANNEL_STATE_BYPASS = 0,
    BRIDGE_CHANNEL_STATE_HALT,
    BRIDGE_CHANNEL_STATE_REMAP_ONLY,
    BRIDGE_CHANNEL_STATE_FULL_CONTROL_IDLE,
    BRIDGE_CHANNEL_STATE_FULL_CONTROL_WAIT,
    BRIDGE_CHANNEL_STATE_FULL_CONTROL_WORKING,
};

struct encoder_crop_info {
    bool encoder_crop_enable;
    int encoder_crop_x0;
    int encoder_crop_y0;
    int encoder_crop_width;
    int encoder_crop_height;
};
#endif

struct comm_src_image_frame_info {
    signed char sensor_index;
    signed char isp_channel_index;
    unsigned short frame_width;
    unsigned short frame_height;
    unsigned int frame_index;
    void *y_phyaddr;
    void *uv_phyaddr;
    void *y_viraddr;
    void *uv_viraddr;
    void *y_busaddr;
    void *uv_busaddr;

    struct timeval pts;
};

struct comm_src_image_slice_info {
    unsigned short block_width;
    unsigned short block_height;
    unsigned short slice_index;
    unsigned short num_of_slices;
    unsigned char num_of_blocks;
    unsigned char block_index;
};

/*
 * H322:
 * width_block_num = 32
 * height_block_num = 24
 * block_size = 2 (BYTES)
 */
struct md_info {
    signed char width_block_num;
    signed char height_block_num;
    signed char block_size;
    unsigned char *mdbuf;
};

struct comm_src_image_info {
    struct comm_src_image_frame_info img_info;
    struct comm_src_image_slice_info slice_info;
    struct md_info md;
    int fps_inverse_us;
#if defined(CONFIG_MACH_KM01A) || defined(CONFIG_MACH_AK3918AV130)
    unsigned char enc_work; //normal
    bool enc_jpeg; //h265 or h264 /jpeg
    bool enc_dual; //dual sensor
    bool enc_join; //dual sensor
    enum ENC_MODE enc_mode;
#endif
};

typedef enum encode_output_type {
    H264_ENC_TYPE = 0,
    MJPEG_ENC_TYPE,
    HEVC_ENC_TYPE
} ENCODE_OUTPUT_TYPE;

struct b2i_exception_slice_info {
    signed char sensor_index;
    signed char isp_channel_index;
    unsigned short slice_index;
    unsigned int frame_index;
    unsigned int drop_frame_num;
};

/*
 *  quick_start提前配置bridge需要的参数
 */
struct b2i_quick_start_info {
    AK_BridgeMode encode_mode[2];
    unsigned int width[2];
    unsigned int height[2];
    unsigned int crop_x0[2];
    unsigned int crop_y0[2];
};

typedef int (*B2I_CB_PP_CHN_INIT)(struct comm_src_image_info *info);
typedef int (*B2I_CB_GATE_INIT)(struct comm_src_image_info *info);
typedef int (*B2I_CB_FIRST_BLOCK_READY)(struct comm_src_image_info *info);
typedef int (*B2I_CB_CAPTURE_FINISH)(struct comm_src_image_info *info);
typedef int (*B2I_CB_EXCEPTION_SLICE)(struct b2i_exception_slice_info *info);
typedef int (*B2I_CB_RESEND_FRAME)(struct comm_src_image_info *info);
/*
 *  quick_start提前配置bridge参数
 */
typedef int (*B2I_CB_QUICK_START_INIT)(struct b2i_quick_start_info *info);

#if defined(CONFIG_MACH_KM01A) || defined(CONFIG_MACH_AK3918AV130)
typedef struct {
    B2I_CB_PP_CHN_INIT          b2i_pp_chn_init;
    B2I_CB_GATE_INIT            b2i_gate_init;
    B2I_CB_CAPTURE_FINISH       b2i_capture_finish;
    B2I_CB_EXCEPTION_SLICE      b2i_exception_slice;
    B2I_CB_QUICK_START_INIT     b2i_quick_start_init;
} T_BRIDGE2ISP_CBS;
#else
typedef struct {
    B2I_CB_PP_CHN_INIT          b2i_pp_chn_init;
    B2I_CB_FIRST_BLOCK_READY    b2i_first_slice_ready;
    B2I_CB_CAPTURE_FINISH       b2i_capture_finish;
    B2I_CB_EXCEPTION_SLICE      b2i_exception_slice;
    B2I_CB_RESEND_FRAME         b2i_resend_frame;
} T_BRIDGE2ISP_CBS;
#endif

struct b2e_bind_isp_channel_info{
    signed char sensor_index;//sensor序号，[0,1]
    signed char isp_channel_index;//PP通道。[0,2]
};//绑定编码通道和isp输出的通道。

struct b2e_query_next_frame_info {
    int slice_index;//slice有效
};

struct b2e_start_info {
    int slice_index;//slice有效
    int frame_index;
    int missing;//NO_MISSING, MISSING
    enum ENC_MODE enc_mode;
#if defined(CONFIG_MACH_KM01A) || defined(CONFIG_MACH_AK3918AV130)
    //start encoder ,must set yuv addr
    unsigned int frame_y_vir_addr;
    unsigned int frame_uv_vir_addr;
#endif
};

struct b2e_finish_info {
    int slice_index;//slice有效
    int frame_index;
    int error;//0:success, others: error
    int hw_enc_time_us;
    int total_enc_time_us;
#if defined(CONFIG_MACH_KM01A) || defined(CONFIG_MACH_AK3918AV130)
    bool bReleaseFrame;
#endif
};

struct b2e_hw_finish_info {
    bool h264_265_finish;
    bool mjpeg_finish;
};

typedef int (*B2E_CB_ENCODE_START)(
        void *bridge_handle, struct b2e_start_info *start);
typedef int (*B2E_CB_ENCODE_FINISH)(
        void *bridge_handle, struct b2e_finish_info *finish);
typedef int (*B2E_CB_HW_ENCODE_FINISH)(struct b2e_hw_finish_info *finish);
typedef int (*B2E_CB_ENCODE_START_U2K)(
        void *bridge_handle, struct b2e_start_info *start);
typedef int (*B2E_CB_ENCODE_FINISH_U2K)(
        void *bridge_handle, struct b2e_finish_info *finish);
typedef void *(*B2E_CB_NEW_BRIDGE_CHANNEL)(void *encoder_handle);
typedef int (*B2E_CB_DEL_BRIDGE_CHANNEL)(void *bridge_handle);
typedef int (*B2E_CB_BIND_ISP_CHANNEL)(void *bindge_handle,
        ENCODE_OUTPUT_TYPE encode_format,
        struct b2e_bind_isp_channel_info *info);
typedef int (*B2E_CB_SET_BRIDGE_CHANNEL_FPS)(
        void *bridge_handle, int fps_inverse_us);
typedef int (*B2E_CB_GET_BRIDGE_CHANNEL_FPS)(void *bridge_handle);
typedef int (*B2E_CB_GET_NUMBER_OF_FRAMES)(void *bridge_handle);
typedef struct comm_src_image_info *(*B2E_CB_GET_NEXT_FRAME)(
        void *bridge_handle,
        struct b2e_query_next_frame_info *next);
typedef int (*B2E_CB_SET_BYPASS)(void *bridge_handle, int enable);
#if defined(CONFIG_MACH_KM01A) || defined(CONFIG_MACH_AK3918AV130)
typedef int (*B2E_CB_SET_BRIDGE_CHANNEL_CROP)(void *bridge_handle, struct encoder_crop_info *info);
typedef enum BRIDGE_CHANNEL_STATE (*B2E_CB_GET_BRIDGE_CHANNEL_STATE)(void *bridge_handle);
typedef void (*B2E_CB_GATE_SOFT_RESET)(ENCODE_OUTPUT_TYPE encode_format);
typedef int (*B2E_CB_GATE_INIT)(void *bridge_handle);
#endif

typedef struct {
    B2E_CB_ENCODE_START             b2e_encode_start;
    B2E_CB_ENCODE_FINISH            b2e_encode_finish;
    B2E_CB_HW_ENCODE_FINISH         b2e_hw_encode_finish;
    B2E_CB_ENCODE_START_U2K         b2e_encode_start_u2k;
    B2E_CB_ENCODE_FINISH_U2K        b2e_encode_finish_u2k;
    B2E_CB_NEW_BRIDGE_CHANNEL       b2e_new_bridge_channel;
    B2E_CB_DEL_BRIDGE_CHANNEL       b2e_del_bridge_channel;
    B2E_CB_BIND_ISP_CHANNEL         b2e_bind_isp_channel;
    B2E_CB_SET_BRIDGE_CHANNEL_FPS   b2e_set_encode_channel_fps;
    B2E_CB_GET_BRIDGE_CHANNEL_FPS   b2e_get_encode_channel_fps;
    B2E_CB_GET_NUMBER_OF_FRAMES     b2e_get_number_of_frames;
    B2E_CB_GET_NEXT_FRAME           b2e_get_next_frame;
    B2E_CB_SET_BYPASS               b2e_set_bypass;
#if defined(CONFIG_MACH_KM01A) || defined(CONFIG_MACH_AK3918AV130)
    B2E_CB_SET_BRIDGE_CHANNEL_CROP  b2e_set_bridge_channel_crop;
    B2E_CB_GET_BRIDGE_CHANNEL_STATE b2e_get_bridge_channel_state;
    B2E_CB_GATE_SOFT_RESET          b2e_gate_soft_reset;
    B2E_CB_GATE_INIT                b2e_gate_init;
#endif
} T_BRIDGE2ENCODER_CBS;

enum encoder_exception_type {
    ENCODER_EXCEPTION_CAPTURING_FAIL = 0,//采集异常
    ENCODER_EXCEPTION_ENCODING_CONFLICT,//编码和采集冲突
    ENCODER_EXCEPTION_SLICE_OVER_FLOW,
};

struct encoder_exception_info {
    enum encoder_exception_type type;
    signed char sensor_index;
    signed char isp_channel_index;
    unsigned short slice_index;
    unsigned int frame_index;
};

/****************************************************************************/
/*************************************venc EXPORT to bridge******************/
/****************************************************************************/
int ak_venc_encoder_force_end(void *encoder_handle);
/**
 *@brief: send new frame/slice to venc
 *@     frame/slice模式。Slice模式时表示第20行中断。中断调用要及时完成。
 *@param    [in] encoder_handle : the venc pointer
 *@param    [in] info : the frame info structure
 *@return: 0 : success, others : fail
 **/
int ak_venc_send_frame(void *encoder_handle, struct comm_src_image_info *info);

/**
 *@brief: send exception info to venc
 *@     采集异常，或者编码和采集冲突。中断调用要及时完成。
 *@param    [in] encoder_handle : the venc pointer
 *@param    [in] info : the exception info structure
 *@return: 0 : success, others : fail
 **/
int ak_venc_send_exception(
        void *encoder_handle, struct encoder_exception_info *info);

/**
 *@brief: register the callbacks from bridge to venc
 *@param    [in] bridge2isp_cb : the callbacks
 *@return: 0 : success, others : fail
 **/
int ak_venc_register_bridge_callbacks(T_BRIDGE2ENCODER_CBS *bridge2isp_cb);

/**
 *@brief: unregister the callbacks from bridge to venc
 *@param    none
 *@return: 0 : success, others : fail
 **/
int ak_venc_unregister_bridge_callbacks(void);

/**
 *@brief: reset venc hw
 *@param    none
 *@return: 0 : success, others : fail
 **/
int ak_venc_encoder_reset_hw(void);
/****************************************************************************/
/**************************************isp EXPORT to bridge******************/
/****************************************************************************/
/**
 *@brief: register the callbacks from bridge to isp
 *@param    [in] bridge2isp_cb : the callbacks
 *@return: 0 : success, others : fail
 **/
int ak_isp_register_bridge_callbacks(T_BRIDGE2ISP_CBS *bridge2isp_cb);

/**
 *@brief: unregister the callbacks from bridge to isp
 *@param    none
 *@return: 0 : success, others : fail
 **/
int ak_isp_unregister_bridge_callbacks(void);

/**
 *@brief: frame release
 *@param    [in] info : the frame info
 *@return: 0 : success, others : fail
 **/
int ak_isp_frame_release(struct comm_src_image_frame_info *info);

/**
 *@brief: get the root umap pointer
 *@param    none
 *@return: NULL : fail, > 0 : success
 **/
struct proc_dir_entry *umap_root_dir(void);

/**
 *@brief: get iss int state
 *@param    none
 *@return: NULL : fail, > 0 : success
 **/
int get_iss_int_state(unsigned int *state);

/**
 *@brief: get main channel capturing id
 *@param    none
 *@return: NULL : fail, > 0 : success
 **/
int ak_isp_get_main_channel_capturing_id(void);

#endif
