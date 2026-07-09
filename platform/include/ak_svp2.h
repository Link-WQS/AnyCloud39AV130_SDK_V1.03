#ifndef __AK_SVP2__
#define __AK_SVP2__


#include "ak_common.h"


enum ak_svp2_error_type {
    ERROR_SVP2_ALREADY_OPEN = ( MODULE_ID_SVP2 << 24 ) + 0,
    ERROR_SVP2_HANDLE_ID_ERROR,
    ERROR_SVP2_HANDLE_ERROR,
    ERROR_SVP2_NOT_CREATE,
    ERROR_SVP2_SET_PARAM,
    ERROR_SVP2_GET_PARAM,
    ERROR_SVP2_ALREADY_INIT,
    ERROR_SVP2_ALREADY_DEINIT,
    ERROR_SVP2_HANDLE_ID_NOT_CREATE,
    ERROR_SVP2_GET_DMA_FD_FAILED,
    ERROR_SVP2_DESTORY_CHN,
    ERROR_SVP2_CREATE_CHN,
    ERROR_SVP2_PROCESSING,
    ERROR_SVP2_CREATE_POOL,
    ERROR_SVP2_DESTROY_POOL,
    //ERROR_TYPE_DMA_MALLOC_FAILED,
};

typedef enum
{
    AK_SVP2_DAY_MODE = 0,        /* 白天模式 */
    AK_SVP2_NIDHT_MODE           /* 夜间模式 */
}AK_SVP2_ENV_MODE_E;

typedef enum
{
    AK_SVP2_HUMAN_SHAPE = 0,             /* human shape 人形 */
    AK_SVP2_HUMAN_FACE,                  /* human face 人脸 */
    AK_SVP2_HUMAN_SHAPE_AND_FACE,        /* human shape & face 人形&人脸 */
    AK_SVP2_PET,                         /* pets detect 宠物 */
    AK_SVP2_VEHICLE,                     /* vehicle detect 车辆 */
    AK_SVP2_NON_MOTOR_VEHICLE,           /* non motor vehicle detect 非机动车 */
    AK_SVP2_HUMAN_GESTURE,               /* 手势检测 */
    AK_SVP2_PACKAGE,                     /* 包裹检测 */
    AK_SVP2_FIRE,                        /* 火焰检测 */
    AK_SVP2_SMOKE,                       /* 烟雾检测 */
    AK_SVP2_TARGET_TYPE_MAX
}AK_SVP2_TARGET_TYPE_E;

typedef enum
{
    AK_SVP2_MODEL_HD_180P = 0,              /* 人形 人脸检测 320*180    */     
    AK_SVP2_MODEL_HD_360P,                  /* 人形 人脸检测 640*360   */  
    AK_SVP2_MODEL_FACE_DT_180P,             /* 横向人脸检测 320*180*  */  
    AK_SVP2_MODEL_FACE_RECOGNIZE,           /* 人脸识别 224*224 *    */  
    AK_SVP2_MODEL_FAS,                      /* 活体检测 224*224 *    */ 
    AK_SVP2_MODEL_0X0A000008,               /* 人形 人脸检测 640*384 5  */
    AK_SVP2_MODEL_0X0A000009,               /* 宠物 人形检测 416*256 6 */
    AK_SVP2_MODEL_0X0A00000A,               /* 人形 车辆检测 640*384 7 */
    AK_SVP2_MODEL_0X0A00000B,               /* 人形 车辆检测 320*192 8 */
    AK_SVP2_MODEL_0X0A00000C,               /* 宠物 人形检测 320*192 9 */
    AK_SVP2_MODEL_0X0A00000D,               /* 人形 人脸检测 320*192 10 */
    AK_SVP2_MODEL_0X0A00000E,               /* 宠物 人形检测 640*384 11 */
    AK_SVP2_MODEL_0X0A00000F,               /* 人形 机动车 非机动车检测 640*384 12*/
    AK_SVP2_MODEL_0X0A000010,               /* 横向人形人脸检测 320*180 13*/
    AK_SVP2_MODEL_0X0A000011,               /* 上半身 人脸 320*180 14*/
    AK_SVP2_MODEL_0X0A000007,               /* 手势检测 640*384 15*/
    AK_SVP2_MODEL_0X0A010002,               /* 手势识别 64*64 16*/
    AK_SVP2_MODEL_0X0A000012,               /* 人车非 640*384 17 ok*/
    AK_SVP2_MODEL_0X0A000013,               /* 上半身 人脸 640*360 18  */
    AK_SVP2_MODEL_0X0A000014,               /* 包裹检测 640*384 19*/
    AK_SVP2_MODEL_0X0A000015,               /* 横向人脸 上半身人形检测 320*180 20*/
    AK_SVP2_MODEL_0X0A010003,               /* 上半身人形活体检测 112*112 21*/
    AK_SVP2_MODEL_0X0A020002,               /* 头肩识别 112*112 22*/
    AK_SVP2_MODEL_0X0A010004,               /* 手势识别 64*64 多类别 23*/
    AK_SVP2_MODEL_0X0A010005,               /* 人脸角度分类 224*224 24*/
    AK_SVP2_MODEL_0X0A010006,               /* 活体分类     224*224 25*/
    AK_SVP2_MODEL_0X0A000016,               /* 上半身 人脸 车辆(暂未启用) 640*384 26*/
    AK_SVP2_MODEL_0X0A000017,               /* 上半身 人脸 320*180 27*/
    AK_SVP2_MODEL_0X0A000018,               /* 上半身 人脸 车辆(暂未启用) 320*192 28*/
    AK_SVP2_MODEL_0X0A00001D = 31,          /* 火焰检测 320*192 31*/
    AK_SVP2_MODEL_0X0A00001C = 32,          /* 手势检测 320*192 32*/
    AK_SVP2_MODEL_0x0A00001E = 33,          /* 包裹检测 320*192 33*/
    AK_SVP2_MODEL_0x0A000020 = 34,          /* 人车非检测 320*192 34*/
    AK_SVP2_MODEL_0X0A000023 = 37,          /* 人形 人脸检测 480*288 37*/
    AK_SVP2_MODEL_0X0A000024 = 38,          /* 上半身 人脸检测 480*288 38*/
    AK_SVP2_MODEL_0X0A000025 = 39,          /* 手势检测 320*192 39*/
    AK_SVP2_MODEL_0X0A000026 = 40,          /* 人车非检测 480*288 40*/
    AK_SVP2_MODEL_0X0A000027 = 41,          /* 宠物 人形检测 480*288 41*/
    AK_SVP2_MODEL_0X0A000028 = 42,          /* 手势检测 480*288 42*/
    AK_SVP2_MODEL_0X0A000029 = 43,          /* 人 宠 火焰 手势检测 480*288 43*/
    AK_SVP2_MODEL_0X0A00002A = 44,          /* 人 车 非机动车 火焰检测 480*288 44*/
    AK_SVP2_MODEL_0X0A00002B = 45,          /* 人 车 非机动车 火焰检测 320*192 45*/
    AK_SVP2_MODEL_0X0A00002C = 46,          /* 人 宠 火焰 手势检测 320*192 46*/
    AK_SVP2_MODEL_0X0A00002D = 47,          /* 手势检测 320*192 47*/
    AK_SVP2_MODEL_0X0A000033 = 53,          /* 宠物检测 320*192 53*/
    AK_SVP2_MODEL_MAX
}AK_SVP2_MODEL_E;

typedef struct AK_SVP2_HD_PARAM_S
{
    int      classify_threshold;
    int      IoU_threshold;
    AK_SVP2_TARGET_TYPE_E  target_type;      /*目标检测类型*/
}AK_SVP2_HD_PARAM_T;

typedef struct AK_SVP2_FAS_PARAM_S
{
    int confidence_threshold;
}AK_SVP2_FAS_PARAM_T;

typedef struct AK_SVP2_FR_PARAM_S
{
    int cos_distance_threshold;
}AK_SVP2_FR_PARAM_T;

typedef struct AK_SVP2_GESTURE_RE_PARAM_S
{
    int confidence_threshold;

}AK_SVP2_GESTURE_RE_PARAM_T;

typedef struct AK_SVP2_MODEL_PARAM_S
{
    union
    {
        AK_SVP2_HD_PARAM_T hd_param;
        AK_SVP2_FAS_PARAM_T fas_param;
        AK_SVP2_FR_PARAM_T fr_param;
        AK_SVP2_GESTURE_RE_PARAM_T gesture_param;
    }param;
}AK_SVP2_MODEL_PARAM_T;

typedef struct AK_SVP2_MODEL_ATTR_S
{
    AK_SVP2_ENV_MODE_E       env_mode;         /*环境模式，AK3918AV100不支持*/
    AK_SVP2_MODEL_E          model_type;       /*检测模型类型*/
    AK_SVP2_MODEL_PARAM_T    model_param;
}AK_SVP2_MODEL_ATTR_T;

typedef struct AK_SVP2_CHN_ATTR_S
{
    int                     model_num;
    AK_SVP2_MODEL_ATTR_T    model_attr[];
}AK_SVP2_CHN_ATTR_T;

typedef int (*AK_SVP2_CUS_READ_FUNC)(void *dst_buf, int buf_len, const char *path);

typedef enum
{
    AK_SVP2_IMG_YUV420SP = 0,            /* YUV420SP，NV12*/
    AK_SVP2_IMG_RGB_LI                    /*RGB line-interlaced format */
}AK_SVP2_IMG_TYPE_E;

typedef struct AK_SVP2_POS_INFO_S
{
    int left;
    int top;
    int width;
    int height;
} AK_SVP2_POS_INFO_T;

typedef struct AK_SVP2_IMG_INFO_S
{
    int                  model_index;
    AK_SVP2_IMG_TYPE_E   img_type;
    int                  width;
    int                  height;
    AK_SVP2_POS_INFO_T   pos_info;
    unsigned long        phy_addr;
    void                 *vir_addr;
} AK_SVP2_IMG_INFO_T;

typedef struct AK_SVP2_RECT_S        //coordinate values, 坐标值
{
    AK_SVP2_TARGET_TYPE_E   label;
    unsigned long           left;     //left_top point, horizontal axis, 左上角x值
    unsigned long           top;     //left_top point, vertical axis,   左上角y值
    unsigned long           right;     //right_bottom point, horizontal axis 右下角x值
    unsigned long           bottom;     //right_bottom point, vertical axis   右下角y值
    unsigned long           score;          // 识别出来的区域的得分
}AK_SVP2_RECT_T;

typedef struct AK_SVP2_HD_RESULT_S
{
    int total_num;
    AK_SVP2_RECT_T *target_boxes;        //coordinate values,坐标信息数组
}AK_SVP2_HD_RESULT_T;

typedef struct AK_SVP2_FAS_RESULT_S
{
    unsigned int label;
    unsigned int confidence;
}AK_SVP2_FAS_RESULT_T;

typedef struct AK_SVP2_FR_RESULT_S
{
    unsigned int data_len;           //feature len;
    signed char *data;               //feature values;
}AK_SVP2_FR_RESULT_T;

typedef struct AK_SVP2_GESTURE_RE_RESULT_S
{
    int label;
    int confidence;
}AK_SVP2_GESTURE_RE_RESULT_T;


typedef struct AK_SVP2_OUTPUT_S
{
    union{
        AK_SVP2_HD_RESULT_T hd_result;
        AK_SVP2_FAS_RESULT_T fas_result;
        AK_SVP2_FR_RESULT_T fr_result;
        AK_SVP2_GESTURE_RE_RESULT_T gesture_result;
    }result;
    int  model_type;
    int  model_index;
}AK_SVP2_OUTPUT_T;

typedef struct AK_SVP2_BUFFER
{
    void          *pData;  // virtual address of the buffer
    unsigned long pa;      // physical address of the buffer
    unsigned int  size;    // size of the buffer
}AK_SVP2_BUFFER_T;

typedef struct AK_SVP2_CONF_INFO_S
{
    union
    {
        char *conf;
        AK_SVP2_BUFFER_T *conf_buf;
    } conf_type;
    int conf_mode;   // conf load mode, 0: path, 1: buffer
    AK_SVP2_CUS_READ_FUNC pReadFunc;
}AK_SVP2_CONF_INFO;

/**
 * ak_svp2_get_version - get version
 * return: version string
 */
const char* ak_svp2_get_version(void);

/**
 * ak_svp2_create_chn - setup the svp channel
 * @handle_id[IN]   :   the svp handle id
 * @attr[IN]        :   Chn attr to set
 * @conf_info[IN]   :   model file info
 * return AK_SUCCESS is successful,  error code otherwise.
 */
int ak_svp2_create_chn(int handle_id, const AK_SVP2_CHN_ATTR_T *attr, AK_SVP2_CONF_INFO *conf_info);

/**
 * ak_svp2_destroy_chn - close the svp channel
 * @handle_id[IN]   :   the svp handle id
 * return AK_SUCCESS is successful,  error code otherwise.
 */
int ak_svp2_destroy_chn(int handle_id);

/**
 * ak_svp2_process - start chn to work
 * @handle_id[IN]    :   the svp handle id
 * @input[IN]        :   input tensor
 * @output[OUT]       :   output tensor
 * return AK_SUCCESS is successful,  error code otherwise.
 */
int ak_svp2_process(int handle_id,  AK_SVP2_IMG_INFO_T *input,  AK_SVP2_OUTPUT_T *output);

/**
 * ak_svp2_release - release the output tensor
 * @output[IN]       :   output tensor
 * return AK_SUCCESS is successful,  error code otherwise.
 */
int ak_svp2_release(AK_SVP2_OUTPUT_T *output);

/**
 * ak_svp2_get_param - get model param from the dest chn
 * @handle_id[IN]    :   the svp handle id
 * @threshold[OUT]   :   model param info
 * return AK_SUCCESS is successful,  error code otherwise.
 */
int ak_svp2_get_param(int handle_id, AK_SVP2_MODEL_PARAM_T *model_param);

/**
 * ak_svp2_set_param - set model param to the dest chn
 * @handle_id[IN]     :   the svp handle id
 * @model_param[IN]   :   model param info
 * return AK_SUCCESS is successful,  error code otherwise.
 */
int ak_svp2_set_param(int handle_id, AK_SVP2_MODEL_PARAM_T model_param);

/**
 * ak_svp2_create_share_mem - create share memory
 * @pool_size[IN]     :   share memory size
 * return AK_SUCCESS is successful,  error code otherwise.
 */
int ak_svp2_create_share_mem(unsigned long pool_size);

/**
 * ak_svp2_create_share_mem - destroy share memory
 * return AK_SUCCESS is successful,  error code otherwise.
 */
int ak_svp2_destroy_share_mem(void);

#endif
