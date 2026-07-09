#ifndef _AK_VENC_H_
#define _AK_VENC_H_
#include <stdbool.h>
#include "ak_common_video.h"

enum ak_venc_error_type
{
    ERROR_VENC_INIT_LIB_ERROR = (MODULE_ID_VENC << 24) + 0,
    ERROR_VENC_OPEN_LIB_ERROR,
    ERROR_VENC_OPEN_TOO_MANY,
    ERROR_VENC_INVALID_PARAM,
    ERROR_VENC_USER_NULL,
    ERROR_VENC_PARAM_CANNOT_DYNAMIC_SET,
    ERROR_VENC_ENCODE_FAILED,
    ERROR_VENC_DEV_OPEN_FAILED,
    ERROR_VENC_CREATE_CHN_FAILED,
    ERROR_VENC_DELETE_CHN_FAILED,
    ERROR_VENC_QUERY_DMA_INFO_FAILED,
    ERROR_VENC_SET_DMA_BLOCK_FAILED,
    ERROR_VENC_DEL_DMA_BLOCK_FAILED,
    ERROR_VENC_GET_STREAM_FAILED,
    ERROR_VENC_RELEASE_STREAM_FAILED,
    ERROR_VENC_CLEAN_STREAM_FAILED,
    ERROR_VENC_SET_JPEG_SLICE_FAILED,
    ERROR_VENC_DMA_NO_ENOUGH,
    ERROR_VENC_SET_ATTR_ERROR,
};

/* h.264 / h.265 encode control define */
enum bitrate_ctrl_mode {
    BR_MODE_CBR,
    BR_MODE_VBR,
    BR_MODE_CONST_QP,
    BR_MODE_LOW_LATENCY,
    BR_MODE_AVBR,
    BR_MODE_TYPE_TOTAL
};

enum profile_mode {
    PROFILE_MAIN,
    PROFILE_HIGH,
    PROFILE_BASE,
    PROFILE_C_BASE,
    PROFILE_HEVC_MAIN,
    PROFILE_HEVC_MAIN_STILL,
    PROFILE_HEVC_MAIN_INTRA,
    PROFILE_JPEG
};

enum chroma_mode {
    CHROMA_MONO,
    CHROMA_4_2_0,
    CHROMA_4_2_2
};

enum jpeg_quant_table_level {
    JPEG_QLEVEL_DEFAULT,
    JPEG_QLEVEL_HIGHEST,
    JPEG_QLEVEL_HIGH,
    JPEG_QLEVEL_LOW
};

enum smart_mode {
    SMART_DISABLE,
    SMART_LTR,
    SMART_CHANGING_GOPLEN,
    SMART_SKIP_FRAME
};

enum Gdr_mode {
    GDR_OFF = 0x0,
    GDR_VERTICAL = 0x02,
    GDR_HORIZONTAL = 0x02 | 0x01,
};

enum EncLevel{
    HEVC_AVC_LEVEL_1   = 10,
    AVC_LEVEL_1_1      = 11, //only for avc
    AVC_LEVEL_1_2      = 12, //only for avc
    AVC_LEVEL_1_3      = 13, //only for avc
    HEVC_AVC_LEVEL_2   = 20,
    HEVC_AVC_LEVEL_2_1 = 21,
    AVC_LEVEL_2_2      = 22, //only for avc
    HEVC_AVC_LEVEL_3   = 30,
    HEVC_AVC_LEVEL_3_1 = 31,
    AVC_LEVEL_3_2      = 32, //only for avc
    HEVC_AVC_LEVEL_4   = 40,
    HEVC_AVC_LEVEL_4_1 = 41,
    AVC_LEVEL_4_2      = 42, //only for avc
    HEVC_AVC_LEVEL_5   = 50,
    HEVC_AVC_LEVEL_5_1 = 51,
    HEVC_AVC_LEVEL_5_2 = 52,
    HEVC_AVC_LEVEL_6   = 60,
    HEVC_AVC_LEVEL_6_1 = 61,
    HEVC_AVC_LEVEL_6_2 = 62
};
enum venc_function
{
    stream_isize_adjustment = 1,
    bit_ratecontrol_slide_rule = 2,
    stream_state = 4,
    bit_ratecontrol_learning = 8,
    quality_Amendatory = 16,
    skip_mode = 32,
    calculte_QPtableRef = 64,
    frame_Resize = 128,
    application_of_NotifySceneChange = 256,
    gop_icontrol = 512,
};

struct venc_roi_param {
    int  enable;    //1 enable, 0 disable
    long top;
    long bottom;
    long left;
    long right;
    long delta_qp;
};

struct venc_param {
    unsigned short                width;
    unsigned short                height;
    unsigned short                fps;
    unsigned short                goplen;
    unsigned short                target_kbps;
    unsigned short                max_kbps;
    enum profile_mode             profile;
    enum bitrate_ctrl_mode        br_mode;

    //recommend Dynamic bit rate parameter[0,100]
    unsigned short                initqp;
    //recommend Dynamic bit rate parameter[20,25]
    unsigned short                minqp;
    //recommend Dynamic bit rate parameter[45,50]
    unsigned short                maxqp;

    enum jpeg_quant_table_level   jpeg_qlevel;
    enum chroma_mode              chroma_mode;

    //encode output type, h264 or jpeg or h265
    enum encode_output_type       enc_out_type;
    unsigned int                  max_picture_size;
    enum EncLevel                 enc_level;

    //0:disable smart, 1:mode of LTR, 2:mode of changing GOP length
    enum smart_mode               smart_mode;
    unsigned short                smart_goplen;       //smart goplen
    unsigned short                smart_quality;      //smart quality
    unsigned short                smart_static_value; //smart static value
    enum Gdr_mode				  gdr_mode;
};

typedef enum CUBLKSize
{
    CUBSize32x32 = 0x00,		/*!< CU size 32x32, just for HEVC */
    CUBSize16x16 = 0x01, 		/*!< CU size 16x16, just for HEVC */
    CUBSize8x8   = 0x02, /*!< CU size 8x8, just for HEVC */
}AK_CUBSize;

struct venc_rc_param
{
    AK_CUBSize			CUBlockSize;
    unsigned short		minqp;
    unsigned short		maxqp;
    short				IPDelta;
    unsigned int		maxPictureISize;
    unsigned int		maxPicturePSize;
    unsigned int		maxPictureBSize;
    bool				enable_MMA;
    bool				enable_SRD;
    unsigned short		SRD_threshold;
};

struct venc_stat {
    unsigned short fps;
    unsigned short goplen;
    unsigned short kbps;
    unsigned short max_picture_size;
};

struct cur_qp_param
{
    int cur_qp;
};

struct rect {
    unsigned int width;
    unsigned int height;
    int x;
    int y;
};

struct venc_crop_cfg {
    bool enable;
    struct rect c_rect;
};

#define AK_VENC_MAX_DMA_BLOCK_NUM	20
typedef struct ak_venc_req_dma_block{
    unsigned int block_num;
    unsigned int block_size[AK_VENC_MAX_DMA_BLOCK_NUM];
}AK_VENC_REQ_DMA_BLOCK;

typedef enum
{
    AK_VENC_SPEC_PARAM_START_TYPE,
    AK_VENC_MAX_REORDER_PICS_NUM,
    AK_VENC_SPEC_SPS,
    AK_VENC_SPEC_PARAM_END_TYPE
}AK_VENC_SPEC_PARAM_TYPE;

struct hevc_spec_param
{
    char max_num_reorder_pics;
};

typedef enum ak_ColourDescription
{
  AK_VENC_COLOUR_DESC_RESERVED = 0,
  AK_VENC_COLOUR_DESC_UNSPECIFIED = 1,
  AK_VENC_COLOUR_DESC_BT_470_NTSC = 2,
  AK_VENC_COLOUR_DESC_BT_601_NTSC = 3,
  AK_VENC_COLOUR_DESC_BT_601_PAK = 4,
  AK_VENC_COLOUR_DESC_BT_709 = 5,
  AK_VENC_COLOUR_DESC_BT_2020 = 6,
  AK_VENC_COLOUR_DESC_SMPTE_240M = 7,
  AK_VENC_COLOUR_DESC_SMPTE_ST_428 = 8,
  AK_VENC_COLOUR_DESC_SMPTE_RP_431 = 9,
  AK_VENC_COLOUR_DESC_SMPTE_EG_432 = 10,
  AK_VENC_COLOUR_DESC_EBU_3213 = 11,
  AK_VENC_COLOUR_DESC_GENERIC_FILM = 12,
  AK_VENC_COLOUR_DESC_MAX_ENUM = 13,
}AK_VENC_EColourDescription;

typedef enum ak_TransferCharacteristics
{
  AK_VENC_TRANSFER_CHARAC_BT_709 = 1,
  AK_VENC_TRANSFER_CHARAC_UNSPECIFIED = 2,
  AK_VENC_TRANSFER_CHARAC_BT_470_SYSTEM_M = 3,
  AK_VENC_TRANSFER_CHARAC_BT_470_SYSTEM_B = 4,
  AK_VENC_TRANSFER_CHARAC_BT_601 = 5,
  AK_VENC_TRANSFER_CHARAC_SMPTE_240M = 6,
  AK_VENC_TRANSFER_CHARAC_LINEAR = 7,
  AK_VENC_TRANSFER_CHARAC_LOG = 8,
  AK_VENC_TRANSFER_CHARAC_LOG_EXTENDED = 9,
  AK_VENC_TRANSFER_CHARAC_IEC_61966_2_4 = 10,
  AK_VENC_TRANSFER_CHARAC_BT_1361 = 11,
  AK_VENC_TRANSFER_CHARAC_IEC_61966_2_1 = 12,
  AK_VENC_TRANSFER_CHARAC_BT_2020_10B = 13,
  AK_VENC_TRANSFER_CHARAC_BT_2020_12B = 14,
  AK_VENC_TRANSFER_CHARAC_BT_2100_PQ = 15,
  AK_VENC_TRANSFER_CHARAC_SMPTE_428 = 16,
  AK_VENC_TRANSFER_CHARAC_BT_2100_HLG = 17,
  AK_VENC_TRANSFER_CHARAC_MAX_ENUM = 18,
}AK_VENC_ETransferCharacteristics;

typedef enum ak_ColourMatrixCoefficients
{
  AK_VENC_COLOUR_MAT_COEFF_GBR = 1,
  AK_VENC_COLOUR_MAT_COEFF_BT_709 = 2,
  AK_VENC_COLOUR_MAT_COEFF_UNSPECIFIED = 3,
  AK_VENC_COLOUR_MAT_COEFF_USFCC_CFR = 4,
  AK_VENC_COLOUR_MAT_COEFF_BT_601_625 = 5,
  AK_VENC_COLOUR_MAT_COEFF_BT_601_525 = 6,
  AK_VENC_COLOUR_MAT_COEFF_BT_SMPTE_240M = 7,
  AK_VENC_COLOUR_MAT_COEFF_BT_YCGCO = 8,
  AK_VENC_COLOUR_MAT_COEFF_BT_2100_YCBCR = 9,
  AK_VENC_COLOUR_MAT_COEFF_BT_2020_CLS = 10,
  AK_VENC_COLOUR_MAT_COEFF_SMPTE_2085 = 11,
  AK_VENC_COLOUR_MAT_COEFF_CHROMA_DERIVED_NCLS = 12,
  AK_VENC_COLOUR_MAT_COEFF_CHROMA_DERIVED_CLS = 13,
  AK_VENC_COLOUR_MAT_COEFF_BT_2100_ICTCP = 14,
  AK_VENC_COLOUR_MAT_COEFF_MAX_ENUM = 15,
}AK_VENC_EColourMatrixCoefficients;

struct avc_spec_param
{

};

struct sps_spec_param
{
    struct avc_spec_param tAvcParm;
    unsigned int video_signal_type_present_flag;
    unsigned int video_format;
    unsigned int video_full_range_flag;
    unsigned int colour_description_present_flag;
    /*!< Indicates the chromaticity coordinates of the source primaries in terms of the CIE 1931 definition. */
    AK_VENC_EColourDescription eColourDescription; 
    /*!< Specifies the reference opto-electronic transfer characteristic function */
    AK_VENC_ETransferCharacteristics eTransferCharacteristics; 
    /*!< Specifies the matrix coefficients used in deriving luma and chroma signals from RGB */
    AK_VENC_EColourMatrixCoefficients eColourMatrixCoeffs; 
};

struct venc_spec_param{
    AK_VENC_SPEC_PARAM_TYPE spec_param_type;
    union 
    {
        struct hevc_spec_param h265_spec_param_reorder;
        struct sps_spec_param sps_spec_param_recorder;
    };
};


struct rate_ctrl_param
{
    unsigned short uSliceQP;
};

struct venc_rc_status
{
    unsigned int uSumCplx; // Sum of LCU complexity
    enum video_frame_type sliceType; // slice type of next frame
};

enum rc_type
{
    INTERNAL = 0x00,  // Use internal Rate Ctrl of encoder lib.
    EXTERNAL = 0x01,  // Use external Rate Ctrl of user.
};

/* ak venc_get_req_dma_block:
 *      get the required dma block info according to the encoder param
 * @param[IN]	:    basic param  of video encorder
 * @rc_param[IN]:	 rate control param of video encorder, if not used, input NULL
 * @dma_block[OUT]:	 dma_block requred according to the encoder param
 * return: AK_SUCCESS if success , others error code.
 */
int ak_venc_get_req_dma_block(const struct venc_param *param,
    const struct venc_rc_param *rc_param, AK_VENC_REQ_DMA_BLOCK *dma_block);

/**
 * ak_venc_get_version - get venc version
 * return: version string
 */
const char* ak_venc_get_version(void);

/**
 * ak_venc_get_enc_lib_version - get venc encode lib version
 * return: version string
 */
const char* ak_venc_get_enc_lib_version(void);

/**
 * ak_venc_open - open encoder and set encode param
 * @param[IN]: encode param
 * @handle_id[OUT]: handle id
 * return: AK_SUCCESS success , others error code.
 */
int ak_venc_open(const struct venc_param *param, int *handle_id);

/**
 * ak_venc_open_ex - open encoder with extened settings
 *                   and set encode and rate control param
 * @param[IN]: 	encode param
 * @rc_param[IN]: encode rate control param
 * @handle_id[OUT]: handle id
 * return: AK_SUCCESS success , others error code.
 */
int ak_venc_open_ex(const struct venc_param *param,
            const struct venc_rc_param *rc_param, int *handle_id);

/**
 * ak_venc_encode_frame - encode single frame
 * @handle_id[IN]: handle id return by ak_venc_open
 * @frame[IN]: frame which you want to encode
 * @frame_len[IN]: lenght of frame
 * @mdinfo[IN]: md info array
 * @stream[OUT]: encode output buffer address
 * return: 0 success , others error code.
 */
int ak_venc_encode_frame(int handle_id, const unsigned char *frame,
        unsigned int frame_len, void *mdinfo, struct video_stream *stream);

/**
 * ak_venc_set_jpeg_slice - jpeg slice encode header
 * @handle_id[IN]: handle id return by ak_venc_open
 * @firstSliceHeight[IN]: yuv frame slice height
 * return: 0 success , others error code.
 */
int ak_venc_set_jpeg_slice(int handle_id, int picHeight,int firstSliceHeight);

/**
 * ak_venc_jpeg_slice_encode - jpeg slice encoder
 * @handle_id[IN]: handle id return by ak_venc_open
 * @frame[IN]: frame which you want to encode
 * @frame_len[IN]: lenght of frame
 * @slice_idx[IN]: slice index
 * @slice_height[IN]: each slice frame height
 * @stream[OUT]: encode output buffer address
 * return: 0 success , others error code.
 */
int ak_venc_jpeg_slice_encode(int handle_id, const unsigned char *frame,
                              unsigned int frame_len, int slice_idx,
                              int slice_height,struct video_stream *stream);

/**
 * ak_venc_release_stream - release stream resource
 * @handle_id[IN]: handle id return by ak_venc_open
 * @stream[IN]: stream return by ak_venc_encode_frame()
 * return: 0 success , others error code.
 * notes:
 */
int ak_venc_release_stream(int handle_id, struct video_stream *stream);

/**
 * ak_venc_clean_stream - clean encode channel stream frame
 * @handle_id[IN]: handle id return by ak_venc_open()
 * return: 0 success , others error code.
 */
int ak_venc_clean_stream(int handle_id);

/**
 * ak_venc_close - close video encode
 * @handle_id[IN]: handle id return by ak_venc_open()
 * return: 0 success , others error code.
 */
int ak_venc_close(int handle_id);

/**
 * ak_venc_set_attr - set venc params
 * @handle_id[IN]: handle id return by ak_venc_open
 * @param[IN]: param to set
 * return: 0 success , others error code.
 * notes:
 */
int ak_venc_set_attr(int handle_id, const struct venc_param *param);

/**
 * ak_venc_set_attr_ex - set venc params with extend param
 * @handle_id[IN]: handle id return by ak_venc_open
 * @param[IN]: 		basic video encoder param  to set
 * @rc_param[IN]	: rate control param of video encoder
 * return: 0 success , others error code.
 * notes:
 */
int ak_venc_set_attr_ex(int handle_id, const struct venc_param *param,
                                        struct venc_rc_param *rc_param);

/**
 * ak_venc_get_attr - get venc params
 * @handle_id[IN]: handle id return by ak_venc_open
 * @param[OUT]: params
 * return: 0 success , others error code.
 * notes:
 */
int ak_venc_get_attr(int handle_id, struct venc_param *param);

/**
 * ak_venc_get_attr_ex - get venc params
 * @handle_id[IN]	: handle id return by ak_venc_open
 * @param[OUT]		: basic video encoder params
 * @rc_param[OUT]	: rate control param of video encoder
 * return: 0 success , others error code.
 * notes:
 */
int ak_venc_get_attr_ex(int handle_id, struct venc_param *param,
                                struct venc_rc_param *rc_param);

/**
 * ak_venc_request_idr - request I frame
 * @handle_id[IN]: handle id return by ak_venc_open
 * return: 0 success , others error code.
 * notes:
 */
int ak_venc_request_idr(int handle_id);


/**
 * ak_venc_get_rate_stat - on stream-encode, get encode rate stat info
 * @handle_id[IN]: handle id return by ak_venc_open
 * @stat[OUT]: stream rate stat info
 * return: 0 success, others error code.
 * notes:
 */
int ak_venc_get_stat(int handle_id, struct venc_stat *stat);

/**
 * ak_venc_set_fps - set the venc chn fps
 * @handle_id[IN]: handle id return by ak_venc_open
 * @fps[IN]: frame rate
 * return: 0 success, others error code.
 * notes: if venc chn not work on kernel-kernel slice frame mode,
 *        that func couldn't work if called.
 */
int ak_venc_set_fps(int handle_id, int fps);

/**
 * ak_venc_get_crop - get the venc crop
 * @handle_id[IN]: handle id return by ak_venc_open
 * @crop[OUT]: venc crop
 * return: 0 success, others error code.
 * notes:
 */
int ak_venc_get_crop(int handle_id, struct venc_crop_cfg *crop);

/**
 * ak_venc_set_crop - set the venc crop
 * @handle_id[IN]: handle id return by ak_venc_open
 * @crop[IN]: venc crop
 * return: 0 success, others error code.
 * notes:
 */
int ak_venc_set_crop(int handle_id, const struct venc_crop_cfg *crop);
/**
 * ak_venc_set_function - set the venc chn function capability
 * @handle_id[IN]: handle id return by ak_venc_open
 * @function[IN]: function value bit
 * return: 0 success, others error code.
 * notes: diffrent mode work function to adjust some encoder capability
 */
int ak_venc_set_function(int handle_id,int function);

/**
 * ak_venc_get_function - get the venc chn function capability
 * @handle_id[IN]: handle id return by ak_venc_open
 * @pfunction[IN]: pfunction value bit point
 * return: 0 success, others error code.
 * notes: diffrent mode work function to adjust some encoder capability
 */
int ak_venc_get_function(int handle_id,int *pfunction);
/**
 * ak_venc_set_qp - set the venc qp value
 * @handle_id[IN]: handle id return by ak_venc_open
 * @minQP[IN]: stream rate stat info
 * @maxQP[IN]: stream rate stat info
 * return: 0 success, others error code.
 * notes: if venc opened in const qp mode, that func couldn't work if called.
 */
int ak_venc_set_qp(int handle_id, int minQP, int maxQP);

/**
 * ak_venc_reset_qp - reset the venc qp value
 * @handle_id[IN]: handle id return by ak_venc_open
 * return: 0 success, others error code.
 * notes: if venc opened in const qp mode, that func couldn't work if called.
 *        And that func will make the qp back to the value when opening.
 */
int ak_venc_reset_qp(int handle_id);

/**
 * ak_venc_set_stream_buff - set the venc stream buff size
 * @handle_id[IN]: handle id return by ak_venc_open
 * @size[IN]     : size to set，unit is byte.
 * return: 0 success, others error code.
 * notes: if venc opened in const qp mode, that func couldn't work if called.
 *        And that func will make the qp back to the value when opening.
 */
int ak_venc_set_stream_buff(int handle_id, int size);

/**
 * ak_venc_set_iframe_param - set the venc iframe size
 * @handle_id[IN]: handle id return by ak_venc_open
 * @minQP[IN]: Iframe stat info
 * @maxQP[IN]: Iframe stat info
 * @minframeSize[IN]: min frame KByte size
 * @maxframeSize[IN]: max frame KByte size
 * return: 0 success, others error code.
 */
int ak_venc_set_iframe_param(int handle_id, int minQP, int maxQP,
                                int minframeSize, int maxframeSize);

/**
 * ak_venc_get_iframe_param - get the venc iframe size
 * @handle_id[IN]: handle id return by ak_venc_open
 * @minQP[OUT]: Iframe stat info
 * @maxQP[OUT]: Iframe stat info
 * @minframeSize[OUT]: min frame KByte size
 * @maxframeSize[OUT]: max frame KByte size
 * return: 0 success, others error code.
 */
int ak_venc_get_iframe_param(int handle_id, int *minQP, int *maxQP,
                                int *minframeSize, int *maxframeSize);

/**
 * ak_venc_set_ROI_Mdinfo - set the ROI param to night mode NR
 *                          (ROI的功能用于减轻夜视图像中的噪声)
 * @handle_id[IN]: handle id return by ak_venc_open
 * @Threshold[IN]: threshold of mdinfo value
 * @Quality[IN]:  max qp delta (Q0 format)
 * @RecoverStep[IN]: recover step of qp delta (Q2)
 * @WaitIntra[IN]: wait num of frames to enable Intra, 0-disable, 1-enable
 * @WaitSkip[IN]: wait num of frames to enable skip, 0-disable, 1-enable
 * return: 0 success, others error code.
 */
int ak_venc_set_ROI_Mdinfo(int handle_id, int Threshold, int Quality,
                            int RecoverStep, int WaitIntra, int WaitSkip);

/**
 * ak_venc_set_ROI_Mdinfo_Param - set the venc ROI param
 *                              (only support AK3918EV200L)
 * @handle_id[IN]: handle id return by ak_venc_open
 * @top[IN]: the top value of ROI region
 * @left[IN]: the left value of ROI region
 * @bottom[IN]: the bottom value of ROI region
 * @right[IN]: the right value of ROI region
 * @DeltaQP[IN]: increment of the ROI region,
 *               the smaller the value, the stronger the effect
 * return: 0 success, others error code.
 */
int ak_venc_set_ROI_Mdinfo_Param(int handle_id, int top, int left,
                                    int bottom,  int right, int DeltaQP);

/**
 * ak_venc_set_jpeg_slice - set the jpeg slice encode config
 * @handle_id[IN]: handle id return by ak_venc_open
 * @picHeight[IN]:total frame height
 * @firstSliceHeight[IN]: first Slice Height
 * return: 0 success, others error code.
 */
int ak_venc_set_jpeg_slice(int handle_id, int picHeight,int firstSliceHeight );

/**
 * ak_venc_jpeg_slice_encode - jpeg slice encode
 * @handle_id[IN]: handle id return by ak_venc_open
 * @frame[IN]: slice frame which you want to encode
 * @frame_len[IN]: lenght of slice frame
 * @slice_idx[IN]: index of slice number
 * @slice_height[IN]: height of slice frame
 * @stream[OUT]: encode output buffer address
 * return: 0 success , others error code.
 */
int ak_venc_jpeg_slice_encode(int handle_id, const unsigned char *frame,
                        unsigned int frame_len, int slice_idx, int slice_height,
                        struct video_stream *stream);

/**
 * ak_venc_change_CUblocksize - brief Set the information
 *                               of CUsize [0:2] only for HEVC
 * @handle_id[IN]: handle id return by ak_venc_open
 * @size[IN]: CU size  defaut=0, 0:32x32 ; 1:16x16; 2:8x8 just for 265
 * return: 0 success, others error code.
 */
int ak_venc_change_CUblocksize(int handle_id, int size);

/**
 * ak_venc_strictlylimit_frameSize - Limit the size of frame strictly
 * @handle_id[IN]: handle id return by ak_venc_open
 * @maxsize[IN]: max frame size(unit is kbytes),
 *              0 <= maxsize(KB) * 1024 < Stream_Buffer Size,
 *              when maxsize==0, will disable frame size limit
 * return: 0 success, others error code.
 */
int ak_venc_strictlylimit_frameSize(int handle_id, int maxsize);

/**
 * ak_venc_set_spec_param - set special paramter to encoder;
 * @handle_id[IN]: handle id return by ak_venc_open
 * @spec_param[IN]: Specical paramter data;
 *                  just one type AK_VENC_MAX_REORDER_PICS_NUM;
 *                  max_num_reorder_pics should be at[0,1]
 * return: 0 success, others error code.
 */
int ak_venc_set_spec_param(int handle_id, const struct venc_spec_param *spec_param);

/**
 * ak_venc_get_spec_param - get special paramter to encoder;
 * @handle_id[IN]: handle id return by ak_venc_open
 * @spec_param[IN]: Specical paramter data;
 * return: 0 success, others error code.
 */
int ak_venc_get_spec_param(int handle_id, struct venc_spec_param *spec_param);
/**
 * ak_venc_clear_policy - clear all the venc policy for pre-alloc dma malloc
 * return: 0 success , others error code.
 */
int ak_venc_clear_policy(void);

/**
 * ak_venc_set_policy - set the venc policy for pre-alloc dma malloc
 * @policy_id	: policy id
 * @param[IN]: encode param
 * @rc_param[IN]: encode rate control param
 * @param[OUT]: handle id
 * return: 0 success , others error code.
 */
int ak_venc_set_policy(int policy_id, const struct venc_param* param,
                                        struct venc_rc_param *rc_param);

/**
 * ak_venc_get_policy_req_dma - get the required dma total length
 *                              according to the policy
 * @policy_id[IN]	: policy id
 * @*length[in]		: store the required lenght according to the policy_id
 * return: 0 success , others error code.
 */
int ak_venc_get_policy_req_dma(int policy_id, int *length);

/**
 * ak_venc_get_cur_qp - get current QP;
 * @handle_id[IN]: handle id return by ak_venc_open
 * @cur_qp_param[IN]: current qp;
 * return: 0 success, others error code.
 */
int ak_venc_get_cur_qp(int handle_id, struct cur_qp_param *cur_qp_param);

/**
 * ak_venc_set_rc_type - set rc type;
 * @handle_id[IN]: handle id return by ak_venc_open
 * @rc_type[IN]: rc type;
 * return: 0 success, others error code.
 */
int ak_venc_set_rc_type(int handle_id, enum rc_type rc_type);

/**
 * ak_venc_set_rc_param - set rc param;
 * @handle_id[IN]: handle id return by ak_venc_open
 * @rc_param_temp[IN]: rc param;
 * return: 0 success, others error code.
 */
int ak_venc_set_rc_param(int handle_id, struct rate_ctrl_param *rc_param);

/**
 * ak_venc_get_rc_stat - get rc stat;
 * @handle_id[IN]: handle id return by ak_venc_open
 * @get_rc_status[OUT]: rc stat;
 * return: 0 success, others error code.
 */
int ak_venc_get_rc_stat(int handle_id, struct venc_rc_status *rc_status);

#endif

/* end of file */
