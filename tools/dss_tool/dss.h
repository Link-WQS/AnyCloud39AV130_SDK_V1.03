#ifndef __DSS_H__
#define __DSS_H__

#include "ak_types.h"

// 视频设备最大数量
#define DSS_VIDEO_MAX_NUM       4
// 每个视频设备的最大通道数量
#define DSS_VIDEO_CHN_NUM       3
// 视频编码器最大数量
#define DSS_VENC_MAX_NUM        8
// ISP最大数量
#define DSS_ISP_MAX_NUM         8

// 硬件相关定义
// IRCUT最大数量
#define DSS_IRCUT_MAX_NUM       4
// LED最大数量
#define DSS_LED_MAX_NUM         4
// 硬光敏传感器最大数量
#define DSS_HW_LUMEN_SENSOR_MAX_NUM       4
// NPU最大数量
#define DSS_NPU_MAX_NUM         4
// 路径最大长度
#define DSS_MAX_PATH_LEN        64
// AIN最大数量
#define DSS_MAX_AIN_NUM		2

// 参数无效值
#define DSS_INVALID            (-1)

#define DSS_VERSION		1
/*
@brief: dss_gpio - GPIO硬件参数结构体
@details: 用于配置通用输入输出引脚的参数
@param nb: GPIO引脚编号
@param value: 引脚值，例如开灯时的引脚设置
*/
struct dss_gpio {
    int nb;      // GPIO引脚编号
    int value;   // 引脚值
};

/*
@brief: dss_pwm - PWM参数结构体
@details: 用于PWM控制参数，可能用于LED亮度调节
@param pwm_id: PWM编号
@param duty_ns: 脉冲宽度(纳秒)
@param period_ns: PWM周期(纳秒)
*/
struct dss_pwm {
    int pwm_id;     // PWM编号
    int duty_ns;    // 脉冲宽度(纳秒)
    int period_ns;  // PWM周期(纳秒)
};

/*
@brief: dss_hw_ircut - IRCUT硬件配置结构体
@details: 用于配置IRCUT滤光片切换硬件参数，可以是单线或双线IRCUT
@param ircut_a_gpio: IRCUT_A控制GPIO
@param ircut_b_gpio: IRCUT_B控制GPIO，单线IRCUT需设置ircut_b_gpio.nb = -1
*/
struct dss_hw_ircut {
    struct dss_gpio ircut_a_gpio;  // IRCUT_A控制GPIO
    struct dss_gpio ircut_b_gpio;  // IRCUT_B控制GPIO
};

/*
@brief: dss_hw_led - 补光灯硬件参数结构体
@details: 用于配置补光灯硬件参数，支持GPIO和PWM两种控制方式
@param ircut_led_type: 红外补光灯控制方式，0-GPIO，1-PWM
@param irled_gpio: GPIO控制的红外补光灯参数
@param irled_pwm: PWM控制的红外补光灯参数
@param whiteled_type: 白光补光灯控制方式，0-GPIO，1-PWM
@param whiteled_gpio: GPIO控制的白光补光灯参数
@param whiteled_pwm: PWM控制的白光补光灯参数
*/
struct dss_hw_led {
    int ircut_led_type;  // 红外补光灯控制方式
    union {
        struct dss_gpio irled_gpio;  // GPIO控制的红外补光灯
        struct dss_pwm  irled_pwm;   // PWM控制的红外补光灯
    };

    int whiteled_type;   // 白光补光灯控制方式
    union {
        struct dss_gpio whiteled_gpio;  // GPIO控制的白光补光灯
        struct dss_pwm  whiteled_pwm;   // PWM控制的白光补光灯
    };
};

/*
@brief: dss_hw_lumen_sensor - 硬光敏传感器结构体
@details: 用于配置硬光敏传感器参数，通过AIN读取光敏电阻值
@param ain_index: AIN输入通道索引，0或1
@param daynight_threshold: 昼夜切换阈值，单位与AIN值相同
*/
struct dss_hw_lumen_sensor {
    int ain_index;           // AIN输入通道索引
    int daynight_threshold;  // 昼夜切换阈值
};

/*
@brief: ak_isp_3dnr_buf - 3D降噪缓冲区配置结构体
@details: 用于配置3D降噪功能的缓冲区参数，有两种配置方式
@param isp_3dnr_total_len: 方式1 - 总缓冲区长度配置
@param isp_3dnr_y_len: 方式2 - Y分量缓冲区长度配置
@param isp_3dnr_u_len: 方式2 - U分量缓冲区长度配置
@param isp_3dnr_v_len: 方式2 - V分量缓冲区长度配置
*/
struct ak_isp_3dnr_buf{
    int isp_3dnr_total_len;  // 总缓冲区长度(KB)
    int isp_3dnr_y_len;      // Y分量缓冲区长度
    int isp_3dnr_u_len;      // U分量缓冲区长度
    int isp_3dnr_v_len;      // V分量缓冲区长度
};

/*
@brief: qs_crop_info - 图像裁剪参数结构体
@details: 用于配置图像裁剪区域参数
@param left: 裁剪区域左上角X坐标
@param top: 裁剪区域左上角Y坐标
@param width: 裁剪区域宽度
@param height: 裁剪区域高度
*/
struct qs_crop_info {
    int left;    // 裁剪区域左上角X坐标
    int top;     // 裁剪区域左上角Y坐标
    int width;   // 裁剪区域宽度
    int height;  // 裁剪区域高度
};

/*
@brief: fastae_info - 快速自动曝光参数结构体
@details: 用于配置快速自动曝光功能参数
@param bining_mode: 输出窗口选择模式，0-大窗(正常输出)，1-小窗(输出窗口较小)
@param fastae_mode: 快速收敛策略
    0: 不使用AE表(一般用于小窗模式)
    1: 仅使用软光敏(一般用于大窗模式)
    2: 仅使用硬光敏(一般用于大窗模式)
    3: 先使用硬光敏，再使用软件快起收敛
@param ae_stable_range: AE收敛稳定范围，亮度容忍范围
@param max_run_frames: 快起AE收敛最大帧数，超过此帧数则退出
@param night_mode_light_select: 夜间模式灯光选择
    0: 昼夜切换，使用红外灯
    1: 昼夜切换，使用白光灯
    2: 始终日间模式，灯光关闭
    3: 始终夜间模式，灯光开启
    4: 始终日间模式，灯光开启
@param daynight_threshold: 昼夜切换阈值数组，索引0为IRCUT白天状态，索引1为IRCUT夜间状态
@param daynight_master_id: 昼夜判断主控设备ID
    -1: 使用自身
    0-7: 使用软光敏ID
    8-15: 使用硬光敏ID
@param ae_table_file: AE表文件路径
*/
struct fastae_info {
    int bining_mode;              // 输出窗口选择模式
    int fastae_mode;              // 快速收敛策略
    int ae_stable_range;          // AE收敛稳定范围
    int max_run_frames;           // 快起AE收敛最大帧数
    int night_mode_light_select;  // 夜间模式灯光选择
    int daynight_threshold[2];    // 昼夜切换阈值
    int daynight_master_id;       // 昼夜判断主控设备ID
    char ae_table_file[DSS_MAX_PATH_LEN];  // AE表文件路径
};

/*
@brief: ak_qs_vi_dev_info - 视频输入设备参数配置结构体
@details: 用于配置视频输入设备相关参数
@param dev_id: 视频设备ID
@param enable: 视频设备采集使能
@param isp_id: ISP ID
@param isp_path: ISP配置文件路径
@param crop: 全局裁剪参数
@param frame_rate: 多目帧采样比例设置，-1表示不采集
@param mirror_en: 画面镜像使能，0-不反转，1-反转
@param flip_en: 画面上下翻转使能，0-不反转，1-反转
@param isp_3dnr_info: 3D降噪缓冲区配置
@param ircut_group_id: IRCUT组ID，-1表示缺省
@param led_group_id: LED组ID，-1表示缺省
@param lumen_group_id: 光敏传感器组ID，-1表示缺省
@param fastae_info: 快速自动曝光参数
*/
struct ak_qs_vi_dev_info{
    int dev_id;                          // 视频设备ID
    int enable;                          // 视频设备采集使能
    int isp_id;                          // ISP ID
    char isp_path[DSS_MAX_PATH_LEN];     // ISP配置文件路径
    
    struct qs_crop_info crop;               // 全局裁剪参数
    int frame_rate;                      // 多目帧采样比例设置
    AK_U32 mirror_en;                    // 画面镜像使能
    AK_U32 flip_en;                      // 画面上下翻转使能
    struct ak_isp_3dnr_buf isp_3dnr_info; // 3D降噪缓冲区配置
    int ircut_group_id;                  // IRCUT组ID
    int led_group_id;                    // LED组ID
    int lumen_group_id;                  // 光敏传感器组ID
    struct fastae_info fastae_info;      // 快速自动曝光参数
};

/*
@brief: AK_QS_VI_RECTANGLE - 矩形区域结构体
@details: 用于描述矩形区域的宽高信息
@param width: 矩形宽度
@param height: 矩形高度
*/
typedef struct AK_QS_VI_RECTANGLE{
    int width;   // 矩形宽度
    int height;  // 矩形高度
} QS_RECTANGLE_S;

// 视频通道模式类型定义
typedef unsigned int DSS_VI_CHN_MODE_E;

/*
@brief: dss_stitch_attr - 图像拼接属性结构体
@details: 用于配置图像拼接相关参数
@param stitch_mode: 拼接模式
    0: 禁用拼接
    1: 垂直拼接
    2: 水平拼接
@param stitch_chn_id: 拼接通道ID
@param stitch_num: 拼接通道总数
@param stitch_index: 拼接图像中的位置索引，从0开始
@param stitch_global_id: 拼接后的码流标识，从0开始
*/
struct dss_stitch_attr {
    int stitch_mode;       // 拼接模式
    int stitch_chn_id;     // 拼接通道ID
    int stitch_num;        // 拼接通道总数
    int stitch_index;      // 拼接图像中的位置索引
    int stitch_global_id;  // 拼接后的码流标识
};

/*
@brief: dss_npu_attr - NPU属性结构体
@details: 用于配置NPU相关属性
@param npu_enable: NPU使能
@param npu_chn_id: NPU通道ID数组
*/
struct dss_npu_attr{
    int npu_enable;                    // NPU使能
    int npu_chn_id[DSS_NPU_MAX_NUM];   // NPU通道ID数组
};

/*
@brief: ak_qs_vi_chn_info - 视频输入通道参数配置结构体
@details: 用于配置视频输入通道相关参数
@param dev_id: 视频设备ID
@param chn_id: 通道ID
@param enable: 通道采集使能
@param frame_rate: 通道帧率
@param res: 输出分辨率
@param frame_depth: 帧深度(缓冲区数量)
@param data_type: 数据类型
    0: YUV420SP(NV12)
    1: YUV420P(I420)
    2: RGB
    3: RAW
    4: RGB线交织
@param mode: 通道模式，0-FRAME_MODE，1-SLICE_MODE
@param crop: 通道裁剪参数
@param max_res: 最大输出分辨率
@param stitch_attr: 拼接属性
@param npu_attr: NPU属性
*/
struct ak_qs_vi_chn_info{
    int dev_id;                    // 视频设备ID
    int chn_id;                    // 通道ID
    int enable;                    // 通道采集使能
    int frame_rate;                // 通道帧率
    QS_RECTANGLE_S res;               // 输出分辨率
    int frame_depth;               // 帧深度
    unsigned int data_type;        // 数据类型
    DSS_VI_CHN_MODE_E mode;        // 通道模式
    struct qs_crop_info crop;         // 通道裁剪参数
    QS_RECTANGLE_S max_res;           // 最大输出分辨率
    struct dss_stitch_attr stitch_attr;  // 拼接属性
    struct dss_npu_attr npu_attr;        // NPU属性
};

/*
@brief: ak_qs_venc_info - 视频编码器参数配置结构体
@details: 用于配置视频编码器相关参数
@param venc_id: 编码器ID
@param width: 编码宽度
@param height: 编码高度
@param fps: 帧率
@param goplen: GOP长度
@param target_kbps: 目标码率(Kbps)
@param max_kbps: 最大码率(Kbps)
@param profile: 编码profile
@param br_mode: 码率控制模式
@param initqp: 初始QP值
@param minqp: 最小QP值
@param maxqp: 最大QP值
@param jpeg_qlevel: JPEG质量等级
@param chroma_mode: 色度模式
@param enc_out_type: 编码输出类型，0-H.264，1-JPEG，2-H.265
@param max_picture_size: 最大图片大小
@param enc_level: 编码级别
@param smart_mode: 智能编码模式，0-禁用，1-LTR模式，2-改变GOP长度模式
@param smart_goplen: 智能GOP长度
@param smart_quality: 智能质量
@param smart_static_value: 智能静态值
@param gdr_mode: GDR模式
@param max_frame_size: 最大帧大小
*/
struct ak_qs_venc_info {
    int  venc_id;              // 编码器ID
    AK_U32 width;                // 编码宽度
    AK_U32 height;               // 编码高度
    AK_U32 fps;                  // 帧率
    AK_U32 goplen;               // GOP长度
    AK_U32 target_kbps;          // 目标码率(Kbps)
    AK_U32 max_kbps;             // 最大码率(Kbps)
    AK_U32 profile;              // 编码profile
    AK_U32 br_mode;              // 码率控制模式
    AK_U32 initqp;               // 初始QP值
    AK_U32 minqp;                // 最小QP值
    AK_U32 maxqp;                // 最大QP值
    AK_U32 jpeg_qlevel;          // JPEG质量等级
    AK_U32 chroma_mode;          // 色度模式
    AK_U32 enc_out_type;         // 编码输出类型
    AK_U32 max_picture_size;     // 最大图片大小
    AK_U32 enc_level;            // 编码级别
    AK_U32 smart_mode;           // 智能编码模式
    AK_U32 smart_goplen;         // 智能GOP长度
    AK_U32 smart_quality;        // 智能质量
    AK_U32 smart_static_value;   // 智能静态值
    AK_U32 gdr_mode;             // GDR模式
    AK_U32 max_frame_size;       // 最大帧大小
};

/*
@brief: ak_qs_video_info - 视频采集与编码绑定配置结构体
@details: 用于配置视频采集与编码器绑定关系
@param chn_id: 通道ID
@param venc_id: 编码器ID
@param enable:  流式编码使能
@param frame_rate: 编码器帧率
@param frame_depth: 帧深度
@param venc_encode_mode: 编码模式，0-用户模式，1-内核模式
*/
struct ak_qs_video_info{
    int chn_id;             // 通道ID
    int venc_id;            // 编码器ID
    int enable;	            // 流式编码使能
    int frame_rate;         // 编码器帧率
    int frame_depth;        // 帧深度
    int venc_encode_mode;   // 编码模式
};

/*
@brief: ak_qs_isp_info - ISP信息配置结构体
@details: 用于配置ISP相关信息
@param isp_id: ISP索引
@param isp_path: ISP配置文件路径
*/
struct ak_qs_isp_info{
    int isp_id;                         // ISP索引
    char isp_path[DSS_MAX_PATH_LEN];    // ISP配置文件路径
};

/*
@brief: ak_qs_vi_info - 视频输入参数配置结构体
@details: 用于配置视频输入相关参数
@param qs_vi_dev_info: 视频设备配置
@param qs_vi_chn_info: 视频通道配置数组
*/
struct ak_qs_vi_info{
    struct ak_qs_vi_dev_info qs_vi_dev_info;           // 视频设备配置
    struct ak_qs_vi_chn_info qs_vi_chn_info[DSS_VIDEO_CHN_NUM];  // 视频通道配置数组
};

/*
@brief: ak_qs_npu_info - NPU参数配置结构体
@details: 用于配置NPU相关信息
@param chn_id: 通道ID
@param model_type: 模型类型
@param model_path: 模型文件路径
*/
struct ak_qs_npu_info{
    int chn_id;                         // 通道ID
    int model_type;                     // 模型类型
    char model_path[DSS_MAX_PATH_LEN];  // 模型文件路径
};

/*
@brief: ak_qs_ai_info - 音频输入参数配置结构体
@details: 用于配置音频输入相关参数
@param id: 音频输入ID
@param sample_rate: 采样率
@param channels: 声道数
@param sample_bits: 采样位数
@param period_bytes: 周期字节数
@param periods: 周期数
@param dev_type: 设备类型
@param gain: 增益
@param data_type: 数据类型
@param reserved: 保留字段
*/
struct ak_qs_ai_info{
    int 		 id;              // 音频输入ID
    unsigned int sample_rate;     // 采样率
    unsigned int channels;        // 声道数
    unsigned int sample_bits;     // 采样位数
    unsigned int period_bytes;    // 周期字节数
    unsigned int periods;         // 周期数
    int          dev_type;        // 设备类型
    int          gain;            // 增益
    int          data_type;       // 数据类型
    unsigned int reserved;        // 保留字段
};

/*
@brief: ak_ao_data_info - 音频输出数据信息结构体
@details: 用于配置音频输出数据信息
@param data: 音频数据指针
@param len: 音频数据长度
*/
struct ak_ao_data_info{
    unsigned int data_addr; // 音频数据指针
    unsigned int len;      // 音频数据长度
};

/*
@brief: ak_qs_ao_info - 音频输出参数配置结构体
@details: 用于配置音频输出相关参数
@param id: 音频输出ID
@param sample_rate: 采样率
@param channels: 声道数
@param sample_bits: 采样位数
@param period_bytes: 周期字节数
@param periods: 周期数
@param dev_type: 设备类型
@param gain: 增益
@param data_type: 数据类型
@param data_src: 数据来源
@param file_path: 音频文件路径
@param data_info: 音频数据信息
@param reserved: 保留字段
*/
struct ak_qs_ao_info{
    int 		 id;              // 音频输出ID
    unsigned int sample_rate;     // 采样率
    unsigned int channels;        // 声道数
    unsigned int sample_bits;     // 采样位数
    unsigned int period_bytes;    // 周期字节数
    unsigned int periods;         // 周期数
    int          dev_type;        // 设备类型
    int          gain;            // 增益
    int          data_type;       // 数据类型
    int          data_src;        // 数据来源
    union{
        char file_path[DSS_MAX_PATH_LEN];     // 音频文件路径
        struct ak_ao_data_info data_info;     // 音频数据信息
    };
    unsigned int reserved;        // 保留字段
};

/*
@brief: ak_qs_audio_info - 音频参数配置结构体
@details: 用于配置音频输入输出相关参数
@param ai_info: 音频输入信息
@param ao_info: 音频输出信息
*/
struct ak_qs_audio_info{
    struct ak_qs_ai_info ai_info;  // 音频输入信息
    struct ak_qs_ao_info ao_info;  // 音频输出信息
};

/*
@brief: ak_little_core_info - 小核参数结构体
@details: 用于配置小核相关参数
@param board_name: 板级配置名称，可通过此字符串匹配小核的板级配置
@param reserved: 保留字段
*/
struct ak_little_core_info {
    char board_name[32];        // 板级配置名称
    unsigned char reserved[32];  // 保留字段
};

/*
@brief: ak_qs_dss_info - 设备特定设置信息结构体
@details: 用于保存到dss.bin文件的完整配置信息
@param version: 配置版本号
@param qs_vi_info: 视频输入配置数组
@param qs_venc_info: 视频编码器配置数组
@param qs_video_info: 视频采集与编码绑定配置数组
@param isp_info: ISP信息配置数组
@param ircut: IRCUT硬件参数配置数组
@param led: LED硬件参数配置数组
@param hw_lumi_sensor: 硬光敏传感器配置数组
@param audio_info: 音频配置信息
@param npu_info: NPU配置数组
@param little_core_info: 小核参数
*/
struct ak_qs_dss_info {
    unsigned int version;                                  // 配置版本号
    struct ak_qs_vi_info qs_vi_info[DSS_VIDEO_MAX_NUM];        // 视频输入配置数组
    struct ak_qs_venc_info qs_venc_info[DSS_VENC_MAX_NUM];     // 视频编码器配置数组
    struct ak_qs_video_info qs_video_info[DSS_VENC_MAX_NUM];   // 视频采集与编码绑定配置数组
    struct ak_qs_isp_info isp_info[DSS_ISP_MAX_NUM];           // ISP信息配置数组
    struct dss_hw_ircut ircut[DSS_IRCUT_MAX_NUM];              // IRCUT硬件参数配置数组
    struct dss_hw_led led[DSS_LED_MAX_NUM];                    // LED硬件参数配置数组
    struct dss_hw_lumen_sensor hw_lumi_sensor[DSS_MAX_AIN_NUM];// 硬光敏传感器配置数组
    struct ak_qs_audio_info audio_info;                        // 音频配置信息
    struct ak_qs_npu_info npu_info[DSS_NPU_MAX_NUM];           // NPU配置数组
    struct ak_little_core_info little_core_info;               // 小核参数
};

#endif

/* 文件结束 */
