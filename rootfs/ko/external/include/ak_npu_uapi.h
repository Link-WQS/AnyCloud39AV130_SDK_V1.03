#ifndef __AK_NPU_H__
#define __AK_NPU_H__

#ifndef AK_FAILED
#define AK_FAILED (-1)
#endif

#ifndef AK_SUCCESS
#define AK_SUCCESS (0)
#endif

#define NPU_VERSION_MAXLEN (10)

enum {
    ERROR_NPU_NO_ERROR            = 0,
    ERROR_NPU_POINTER_NULL        = 0x100,
    ERROR_NPU_INVALID_ARG,
    ERROR_NPU_MALLOC_FAILED,
    ERROR_NPU_NOT_INIT,
    ERROR_NPU_FILE_READ_FAILED,
    ERROR_NPU_HANDLE_ERROR,
    ERROR_NPU_SET_PARAM,
    ERROR_NPU_GET_PARAM,
    ERROR_NPU_ALREADY_INIT,
    ERROR_NPU_ALREADY_DEINIT,
    ERROR_NPU_DMA_MALLOC_FAILED,
};

#define AK_NNA_HD_MAX_NUM (200)
#define AK_NNA_MAX_FR_NUM (1000)
#define AK_NNA_PATH_MAXLEN (128)

typedef enum {
    AK_NNA_DAY_MODE = 0,        /* 白天模式 */
    AK_NNA_NIGHT_MODE            /* 夜间模式 */
} AK_NNA_ENV_MODE_E;

typedef enum {
    AK_NNA_HUMAN_SHAPE = 0,             /* human shape 人形 */
    AK_NNA_HUMAN_FACE,                  /* human face 人脸 */
    AK_NNA_HUMAN_SHAPE_AND_FACE,        /* human shape & face 人形&人脸 */
    AK_NNA_PET,                         /* pets detect 宠物 */
    AK_NNA_VEHICLE,                     /* vehicle detect 车辆 */
    AK_NNA_NON_MOTOR_VEHICLE,           /* non motor vehicle detect 非机动车 */
    AK_NNA_HUMAN_GESTURE,               /* 手势检测 */
    AK_NNA_PACKAGE,                     /* 包裹检测 */
    AK_NNA_FIRE,                        /* 火焰检测 */
    AK_NNA_SMOKE,                       /* 烟雾检测 */

} AK_NNA_TARGET_TYPE_E;

typedef enum {
    AK_NNA_MODEL_HD_180P = 0,              /* 人形 人脸检测 320*180 */
    AK_NNA_MODEL_HD_360P,                  /* 人形 人脸检测 640*360 */
    AK_NNA_MODEL_FACE_DT_180P,             /* 横向人脸检测 320*180*/
    AK_NNA_MODEL_FACE_RECOGNIZE,           /* 人脸识别 224*224 */
    AK_NNA_MODEL_FAS,                      /* 活体检测 224*224 */
    AK_NNA_MODEL_0X0A000008,               /* 人形 人脸检测 640*384 5 */
    AK_NNA_MODEL_0X0A000009,               /* 宠物 人形检测 416*256 6 */
    AK_NNA_MODEL_0X0A00000A,               /* 人形 车辆检测 640*384 7 */
    AK_NNA_MODEL_0X0A00000B,               /* 人形 车辆检测 320*192 8 */
    AK_NNA_MODEL_0X0A00000C,               /* 宠物 人形检测 320*192 9 */
    AK_NNA_MODEL_0X0A00000D,               /* 人形 人脸检测 320*192 10 */
    AK_NNA_MODEL_0X0A00000E,               /* 宠物 人形检测 640*384 11 */
    AK_NNA_MODEL_0X0A00000F,               /* 人形 机动车 非机动车检测 640*384 12*/
    AK_NNA_MODEL_0X0A000010,               /* 横向人形人脸检测 320*180 13*/
    AK_NNA_MODEL_0X0A000011,               /* 上半身 人脸 320*180 14*/
    AK_NNA_MODEL_0X0A000007,               /* 手势检测 640*384 15*/
    AK_NNA_MODEL_0X0A010002,               /* 手势识别 64*64 16*/
    AK_NNA_MODEL_0X0A000012,               /* 人车非 640*384 17*/
    AK_NNA_MODEL_0X0A000013,               /* 上半身 人脸 640*360 18*/
    AK_NNA_MODEL_0X0A000014,               /* 包裹检测 640*384 19*/
    AK_NNA_MODEL_0X0A000015,               /* 横向人脸 上半身人形检测 320*180 20*/
    AK_NNA_MODEL_0X0A010003,               /* 上半身人形活体检测 112*112 21*/
    AK_NNA_MODEL_0X0A020002,               /* 头肩识别 112*112 22*/
    AK_NNA_MODEL_0X0A010004,               /* 手势识别 64*64 多类别 23*/
    AK_NNA_MODEL_0X0A010005,               /* 人脸角度分类 224*224 24*/
    AK_NNA_MODEL_0X0A010006,               /* 活体分类     224*224 25*/
    AK_NNA_MODEL_0X0A000016,               /* 上半身 人脸 车辆(暂未启用) 640*384 26*/
    AK_NNA_MODEL_0X0A000017,               /* 上半身 人脸 320*180 27*/
    AK_NNA_MODEL_0X0A000018,               /* 上半身 人脸 车辆(暂未启用) 320*192 28*/
    AK_NNA_MODEL_0X0A00001D = 31,          /* 火焰检测 320*192 31*/
    AK_NNA_MODEL_0X0A00001C = 32,          /* 手势检测 320*192 32*/
    AK_NNA_MODEL_0X0A00001E = 33,          /* 包裹检测 320*192 33*/
    AK_NNA_MODEL_0X0A000020 = 34,          /* 人车非检测 320*192 34*/
    AK_NNA_MODEL_0X0A000023 = 37,          /* 人形 人脸检测 480*288 37*/
    AK_NNA_MODEL_0X0A000024 = 38,          /* 上半身 人脸检测 480*288 38*/
    AK_NNA_MODEL_0X0A000025 = 39,          /* 手势检测 320*192 39*/
    AK_NNA_MODEL_0X0A000026 = 40,          /* 人车非检测检测 480*288 40*/
    AK_NNA_MODEL_0X0A000027 = 41,          /* 宠物 人形检测 480*288 41*/
    AK_NNA_MODEL_0X0A000028 = 42,          /* 手势检测 480*288 42*/
    AK_NNA_MODEL_0X0A000029 = 43,          /* 人 宠 火焰 手势检测 480*288 43*/
    AK_NNA_MODEL_0X0A00002A = 44,          /* 人 车 非机动车 火焰检测 480*288 44*/
    AK_NNA_MODEL_0X0A00002B = 45,          /* 人 车 非机动车 火焰检测 320*192 45*/
    AK_NNA_MODEL_0X0A00002C = 46,          /* 人 宠 火焰 手势检测 320*192 46*/
    AK_NNA_MODEL_0X0A00002D = 47,          /* 手势检测 320*192 47*/
    AK_NNA_MODEL_0X0A000033 = 53,          /* 宠物检测 320*192 53*/
    AK_NNA_MODEL_MAX,
} AK_NNA_MODEL_E;

typedef struct AK_NNA_HD_PARAM_S {
    int classify_threshold;
    int IoU_threshold;
    AK_NNA_TARGET_TYPE_E target_type;    /*目标检测类型*/
} AK_NNA_HD_PARAM_T;

typedef struct AK_NNA_FAS_PARAM_S {
    int confidence_threshold;
} AK_NNA_FAS_PARAM_T;

typedef struct AK_NNA_FR_PARAM_S {
    int cos_distance_threshold;
} AK_NNA_FR_PARAM_T;

typedef struct AK_NNA_GESTURE_RE_PARAM_S {
    int confidence_threshold;
} AK_NNA_GESTURE_RE_PARAM_T;

typedef struct AK_NNA_MODEL_PARAM_S {
    union {
        AK_NNA_HD_PARAM_T hd_param;
        AK_NNA_FAS_PARAM_T fas_param;
        AK_NNA_FR_PARAM_T fr_param;
        AK_NNA_GESTURE_RE_PARAM_T gesture_param;
    } param;
} AK_NNA_MODEL_PARAM_T;

typedef struct AK_NNA_MODEL_ATTR_S {
    AK_NNA_ENV_MODE_E    env_mode;    /*环境模式，AK3918AV100不支持*/
    AK_NNA_MODEL_E       model_type;  /*检测模型类型*/
    AK_NNA_MODEL_PARAM_T model_param;
} AK_NNA_MODEL_ATTR_T;

typedef struct AK_NNA_CHN_ATTR_S {
    int model_num;
    AK_NNA_MODEL_ATTR_T *model_attr;
} AK_NNA_CHN_ATTR_T;

typedef enum {
    AK_NNA_IMG_YUV420SP = 0,          /* YUV420SP，NV12*/
    AK_NNA_IMG_RGB_LI                 /*RGB line-interlaced format */
}AK_NNA_IMG_TYPE_E;

typedef struct AK_NNA_POS_INFO_S {
    int left;
    int top;
    int width;
    int height;
} AK_NNA_POS_INFO_T;

typedef struct AK_NNA_IMG_INFO_S {
    int model_index;
    AK_NNA_IMG_TYPE_E img_type;
    int width;
    int height;
    AK_NNA_POS_INFO_T pos_info;
    unsigned long phy_addr;
    void *vir_addr;
    unsigned int fd;
} AK_NNA_IMG_INFO_T;

typedef struct AK_NNA_RECT_S { //coordinate values
    AK_NNA_TARGET_TYPE_E label;
    unsigned long left;       //left_top point, horizontal axis
    unsigned long top;        //left_top point, vertical axis
    unsigned long right;      //right_bottom point, horizontal axis
    unsigned long bottom;     //right_bottom point, vertical axis
    unsigned long score;      //score
} AK_NNA_RECT_T;

typedef struct AK_NNA_HD_RESULT_S {
    int total_num;
    AK_NNA_RECT_T target_boxes[AK_NNA_HD_MAX_NUM]; //coordinate values,坐标信息数组
} AK_NNA_HD_RESULT_T;

typedef struct AK_NNA_FAS_RESULT_S {
    unsigned int label;
    unsigned int confidence;
} AK_NNA_FAS_RESULT_T;

typedef struct AK_NNA_FR_RESULT_S {
    unsigned int data_len;                //feature len
    signed char data[AK_NNA_MAX_FR_NUM]; //feature values
} AK_NNA_FR_RESULT_T;

typedef struct AK_NNA_GESTURE_RE_RESULT_S {
    int label;
    int confidence;
} AK_NNA_GESTURE_RE_RESULT_T;

typedef struct AK_NNA_OUTPUT_S {
    union{
        AK_NNA_HD_RESULT_T hd_result;
        AK_NNA_FAS_RESULT_T fas_result;
        AK_NNA_FR_RESULT_T fr_result;
        AK_NNA_GESTURE_RE_RESULT_T gesture_result;
    } result;
    int model_type;
    int model_index;
} AK_NNA_OUTPUT_T;

typedef struct AK_NPU_BUFFER {
    void *pData; // virtual address of the buffer
    unsigned long pa;  // physical address of the buffer
    unsigned int size; // size of the buffer
} AK_NPU_BUFFER_T;

typedef struct AK_NNA_CONF_INFO_S {
    union {
        char conf[AK_NNA_PATH_MAXLEN];
        AK_NPU_BUFFER_T conf_buf;
    } conf_type;
    int conf_mode; // conf load mode, 0: path, 1: dma buffer
} AK_NNA_CONF_INFO_T;

typedef struct AK_NNA_CREATE_CHN_INFO_S {
    AK_NNA_CHN_ATTR_T attr;
    AK_NNA_CONF_INFO_T conf_info;
    void **npu_handle;
    void **nna_handle;
    unsigned long *channel_id;
    int *ret;
    unsigned int user_channel_id;
} AK_NNA_CREATE_CHN_INFO_T;

typedef struct AK_NNA_DESTROY_CHN_INFO_S {
    void *npu_handle;
    int *ret;
} AK_NNA_DESTROY_CHN_INFO_T;

typedef enum {
    AK_NNA_PROCESS,
    AK_NNA_PROCESS_DM,
    AK_NNA_PROCESS_POLLING,
    AK_NNA_PROCESS_BPP,
    AK_NNA_PROCESS_BPP_DM,
    AK_NNA_PROCESS_MAX
} AK_NNA_PROCESS_TYPE_E;

typedef struct AK_NNA_PROCESS_INFO_S {
    void *npu_handle;
    AK_NNA_IMG_INFO_T input;
    AK_NNA_PROCESS_TYPE_E type;
    AK_NNA_OUTPUT_T *output;
    int *ret;
} AK_NNA_PROCESS_INFO_T;

typedef struct AK_NNA_GET_PARAM_INFO_S {
    void *npu_handle;
    AK_NNA_MODEL_PARAM_T *model_param;
    int *ret;
} AK_NNA_GET_PARAM_INFO_T;

typedef struct AK_NNA_SET_PARAM_INFO_T {
    void *npu_handle;
    AK_NNA_MODEL_PARAM_T model_param;
    int *ret;
} AK_NNA_SET_PARAM_INFO_T;

typedef struct AK_NNE_BLOB_INFO_S {
    void *VirAddr;
    unsigned long PhyAddr;
    unsigned long size;
    unsigned long w;
    unsigned long h;
    unsigned long c;
    unsigned long scale;
    unsigned long data_type; //T_U8/T_S8/...
    unsigned long data_arrange_type;
    unsigned long memory_type; //[0:dma_memory,will not copy] [1:normal_memory,will copy]
} AK_NNE_BLOB_INFO_T;

#define AK_NNE_MAX_BLOB_CNT 12
typedef struct AK_NNE_BLOB_VECTOR_S {
    unsigned long blob_cnt;
    AK_NNE_BLOB_INFO_T array_blobs[AK_NNE_MAX_BLOB_CNT];
} AK_NNE_BLOB_VECTOR_T;

typedef struct AK_NNE_CREATE_CH_INFO_S {
    AK_NPU_BUFFER_T model;
    char model_path[AK_NNA_PATH_MAXLEN];
    void **nne_net_handle;
    unsigned long *channel_id;
    int *ret;
} AK_NNE_CREATE_CH_INFO_T;

typedef struct AK_NNE_DESTROY_CH_INFO_S {
    void *nne_net_handle;
    int *ret;
} AK_NNE_DESTROY_CH_INFO_T;

typedef enum {
    AK_NNE_RUN,
    AK_NNE_RUN_BPP,
    AK_NNE_RUN_BPP_DM,
    AK_NNE_RUN_INTERRUPT,
    AK_NNE_RUN_INTERRUPT_DM,
    AK_NNE_RUN_MAX
} AK_NNE_RUN_TYPE_E;

typedef struct AK_NNE_RUN_INFO_S {
    void *nne_net_handle;
    AK_NNE_BLOB_VECTOR_T input_blobs;
    AK_NNE_RUN_TYPE_E type;
    AK_NNE_BLOB_VECTOR_T *output_blobs;
    unsigned int *output_fds;
    int *ret;
} AK_NNE_RUN_INFO_T;


typedef struct AK_NNE_RUN_INTERRUPT_INFO_S {
    void *nne_net_handle;
    AK_NNE_BLOB_VECTOR_T input_blobs;
    AK_NNE_BLOB_VECTOR_T *output_blobs;
    unsigned int *output_fds;
    int *ret;
} AK_NNE_RUN_INTERRUPT_INFO_T;

typedef struct AK_NNE_MODEL_HEADER_S //12*(32bit)
{
    unsigned long model_parse_version;
    unsigned long header_size;
    unsigned long network_size;
    unsigned long param_size;
    unsigned long model_size;
    unsigned long model_label;
    unsigned long v_abcd;
    unsigned long reserved1;
    unsigned long reserved2;
    unsigned long reserved3;
    unsigned long reserved4;
    unsigned long reserved5;
} AK_NNE_MODEL_HEADER_T;

typedef struct AK_NNE_GET_MODEL_HEADER_INFO_S {
    void *nne_net_handle;
    AK_NNE_MODEL_HEADER_T *header;
    int *ret;
} AK_NNE_GET_MODEL_HEADER_INFO_T;

typedef struct {
    unsigned long free_size;        // 空闲大小
    unsigned long used_size;        // 已使用大小
} AK_MEM_DMA_POOL_STATUS_T;

/* IOCTL */
#define NPU_IOC_MAGIC           'N'
#define AKNPU_IO(nr)            _IOC(_IOC_NONE, NPU_IOC_MAGIC, nr, 0)
#define AKNPU_IORWn(nr, size)    _IOWR(NPU_IOC_MAGIC, nr, size)

/* npu command */
#define IOC_NR_NPU_GET_VERSION  (0x10)
#define IOC_NR_NPU_INIT         (0x11)
#define IOC_NR_NPU_DEINIT       (0x12)
#define IOC_NR_NPU_HW_RESET     (0x13)

#define IOC_NPU_GET_VERSION     AKNPU_IORWn(IOC_NR_NPU_GET_VERSION, char*)
#define IOC_NPU_INIT            AKNPU_IORWn(IOC_NR_NPU_INIT, int)
#define IOC_NPU_DEINIT          AKNPU_IO(IOC_NR_NPU_DEINIT)
#define IOC_NPU_HW_RESET        AKNPU_IO(IOC_NR_NPU_HW_RESET)

/* svp2 command */
#define IOC_NR_NNA_CREATE_CHN     (0x20)
#define IOC_NR_NNA_DESTROY_CHN    (0x21)
#define IOC_NR_NNA_PROCESS        (0x22)
//#define IOC_NR_NNA_RELEASE      (0x23)
#define IOC_NR_NNA_GET_PARAM      (0x24)
#define IOC_NR_NNA_SET_PARAM      (0x25)
#define IOC_NR_NNA_DMA_MEM_SHARED (0x26)

#define IOC_NNA_CREATE_CHN      AKNPU_IORWn(IOC_NR_NNA_CREATE_CHN, AK_NNA_CREATE_CHN_INFO_T)
#define IOC_NNA_DESTROY_CHN     AKNPU_IORWn(IOC_NR_NNA_DESTROY_CHN, AK_NNA_DESTROY_CHN_INFO_T)
#define IOC_NNA_PROCESS         AKNPU_IORWn(IOC_NR_NNA_PROCESS, AK_NNA_PROCESS_INFO_T)
//#define IOC_NNA_RELEASE         AKNPU_IORWn(IOC_NR_NNA_RELEASE, AK_NNA_RELEASE_INFO_T)
#define IOC_NNA_GET_PARAM       AKNPU_IORWn(IOC_NR_NNA_GET_PARAM, AK_NNA_GET_PARAM_INFO_T)
#define IOC_NNA_SET_PARAM       AKNPU_IORWn(IOC_NR_NNA_SET_PARAM, AK_NNA_SET_PARAM_INFO_T)
#define IOC_NNA_DMA_MEM_SHARED  AKNPU_IORWn(IOC_NR_NNA_DMA_MEM_SHARED, int)

/* nne */
#define IOC_NR_NNE_CREATE_CH        (0x30)
#define IOC_NR_NNE_DESTROY_CH       (0x31)
#define IOC_NR_NNE_RUN              (0x32)
#define IOC_NR_NNE_GET_MODEL_HEADER (0x33)

#define IOC_NNE_CREATE_CH     AKNPU_IORWn(IOC_NR_NNE_CREATE_CH, AK_NNE_CREATE_CH_INFO_T)
#define IOC_NNE_DESTROY_CH    AKNPU_IORWn(IOC_NR_NNE_DESTROY_CH, AK_NNE_DESTROY_CH_INFO_T)
#define IOC_NNE_RUN           AKNPU_IORWn(IOC_NR_NNE_RUN, AK_NNE_RUN_INFO_T)
#define IOC_NNE_GET_MODEL_HEADER AKNPU_IORWn(IOC_NR_NNE_GET_MODEL_HEADER, AK_NNE_GET_MODEL_HEADER_INFO_T)

/* mem */
#define IOC_NR_MEM_DMA_POOL_INIT     (0x40)
#define IOC_NR_MEM_DMA_POOL_DESTROY  (0x41)
#define IOC_NR_MEM_DMA_POOL_STATUS   (0x42)

#define IOC_MEM_DMA_POOL_INIT        AKNPU_IORWn(IOC_NR_MEM_DMA_POOL_INIT, unsigned long)
#define IOC_MEM_DMA_POOL_DESTROY     AKNPU_IO(IOC_NR_MEM_DMA_POOL_DESTROY)
#define IOC_MEM_DMA_POOL_STATUS      AKNPU_IORWn(IOC_NR_MEM_DMA_POOL_STATUS, AK_MEM_DMA_POOL_STATUS_T)

#endif /* __AK_NPU_H__ */
