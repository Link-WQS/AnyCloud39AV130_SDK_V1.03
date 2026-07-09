#ifndef __IPI_PROTOCOL_H__
#define __IPI_PROTOCOL_H__

/**
 * ipi
 */

/*
对齐
*/
//#define IPI_PROTOCOL_ATTR          __attribute__((aligned(4)))
#define IPI_PROTOCOL_ATTR

/**
 * fastsys channel cmd
 */
#if 0   //fastsys属于系统基础, 在其他文件定义
/* cmd list */
#define IPI_FASTSYS_CMD_RUNNING     0x1
#define IPI_FASTSYS_CMD_DONE        0x2
#define IPI_FASTSYS_CMD_ION         0x3

#define IPI_MASTER_CMD_RUNNING      0x1
#define IPI_MASTER_CMD_FSDONE       0x2

struct IPI_PROTOCOL_ATTR ipip_ion_used_mem {
    void *base;
    unsigned int  size;
};
#endif

/**
 * isp channel cmd
 */
/* cmd list */
/* rtos to linux */
/* post fastae message */
#define IPI_FASTSYS_CMD_AE_FAST_PARAM       0x1
/* query frame information */
#define IPI_FASTSYS_CMD_POST_FRAMESINFO     0x2
/* get one frame */
#define IPI_FASTSYS_CMD_POST_ONEFRAME       0x3
/* post rtos parameter for display*/
#define IPI_FASTSYS_CMD_UMAP_PARAM          0x4
/*
    IPI_FASTSYS_CMD_POST_FAST_PARAM2 参数的放置
    | 长度(4 bytes) | 数据 n bytes | 长度(4 bytes) | 数据 n bytes |
    - 存放结构体数据长, 4byte
    - struct ipip_vi_dev_status ir_led;
    - 存放ae_status, 4byte
    - struct ipip_fast_ae_status ae_status * n;
*/
#define IPI_FASTSYS_CMD_POST_FAST_PARAM2    0x5

/* linux to rtos */
/* request fastae command from linux */
#define IPI_MASTER_CMD_GET_AE_FAST_PARAM    0x1
/* query frame information */
#define IPI_MASTER_CMD_QUERY_FRAMESINFO     0x2
/* get one frame */
#define IPI_MASTER_CMD_GET_ONEFRAME         0x3
/* release one frame */
#define IPI_MASTER_CMD_RELEASE_ONEFRAME     0x4
/* all down */
#define IPI_MASTER_CMD_ALL_DONE             0x5
/* get rtos parameter for display */
#define IPI_MASTER_CMD_GET_UMAP_PARAM       0x6
/* request fastae command from linux */
#define IPI_MASTER_CMD_GET_FAST_PARAM2      0x7

//
struct IPI_PROTOCOL_ATTR ipip_gpio {
    int gpio;       // GPIO引脚编号
    int value;      // 使能有效值
};

struct IPI_PROTOCOL_ATTR ipip_pwm {
    int pwm_id;     // PWM编号
    int duty_ns;    // 脉冲宽度(纳秒)
    int period_ns;  // PWM周期(纳秒)
};

struct IPI_PROTOCOL_ATTR ipip_ircut {
    struct ipip_gpio ircut_a_gpio;  // IRCUT_A控制GPIO
    struct ipip_gpio ircut_b_gpio;  // IRCUT_B控制GPIO
};

struct IPI_PROTOCOL_ATTR ipip_led {
    int ircut_led_type;  // 红外补光灯控制方式
    union {
        struct ipip_gpio irled_gpio;  // GPIO控制的红外补光灯
        struct ipip_pwm  irled_pwm;   // PWM控制的红外补光灯
    };

    int whiteled_type;   // 白光补光灯控制方式
    union {
        struct ipip_gpio whiteled_gpio;  // GPIO控制的白光补光灯
        struct ipip_pwm  whiteled_pwm;   // PWM控制的白光补光灯
    };
};

#define IPIP_IRCUT_MAX_NUM          4
#define IPIP_LED_MAX_NUM            4
#define IPIP_DEV_MAX_NUM            4

struct IPI_PROTOCOL_ATTR ipip_vi_dev_status {
    struct ipip_ircut   ircut[IPIP_IRCUT_MAX_NUM];
    int ircut_a_curstate[IPIP_IRCUT_MAX_NUM];
    int ircut_b_curstate[IPIP_IRCUT_MAX_NUM];

    struct ipip_led     led[IPIP_LED_MAX_NUM];
    int whiteled_curstate[IPIP_LED_MAX_NUM];
    int irled_curstate[IPIP_LED_MAX_NUM];
};

//
struct IPI_PROTOCOL_ATTR ipip_awb_gain {
    unsigned short  r_gain;
    unsigned short  g_gain;
    unsigned short  b_gain;
    signed short    r_offset;
    signed short    g_offset;
    signed short    b_offset;
};

struct IPI_PROTOCOL_ATTR ipip_fast_ae {
    unsigned int sensor_exp_time;
    unsigned int sensor_a_gain;
    unsigned int sensor_d_gain;
    unsigned int isp_d_gain;
    struct ipip_awb_gain    wb;
};

struct IPI_PROTOCOL_ATTR ipip_fast_ae_status {
    int fastae_conv_frames;//收敛帧数
    int fastae_max_frames;//最大收敛帧数
    int fastae_is_timeout;//超过fastae_max_frames还没收敛成功
    int current_day_night_mode;//day night mode
    int isp_sub_file_index;//sub file index
    unsigned int current_lumi;//当前亮度
    unsigned int target_lumi;//目标s亮度
    struct ipip_fast_ae ae;//快速收敛AE参数

};
struct IPI_PROTOCOL_ATTR ipip_fast_ae_info {
    int fast_valid_ae_num;
    struct ipip_fast_ae_status fast_ae[IPIP_DEV_MAX_NUM];
};


struct IPI_PROTOCOL_ATTR ipip_query_framsinfo {
    unsigned int frame_cnt;
    unsigned int isp_load_mode;
};

struct IPI_PROTOCOL_ATTR ipip_isp_frame{
    int dev_id;
    unsigned int attr;
#define IPIP_ISP_FRAME_ATTR_ERR              (0x1 << 0)
    unsigned int seq;
    unsigned long timestramp;
    unsigned long length;
    unsigned long phy_addr;
};

struct IPI_PROTOCOL_ATTR ipip_fastsys_param {
    unsigned char dss_data[3072];
};


/**
 * venc channel cmd
 */

/* linux to rtos */
/* get stream */
#define IPI_MASTER_VENC_GET_STREAM_MSG          0x1

/* rtos to linux */
#define IPI_FASTSYS_VENC_POST_STREAM_MSG        (IPI_MASTER_VENC_GET_STREAM_MSG | 0x80)

struct IPI_PROTOCOL_ATTR ipip_stream_message {
    unsigned long phy_addr;
    unsigned long used_bytes;
    unsigned long size;
};

/**/
#endif

