#ifndef __AK_VIDEO_PRIV_CMD_INTERNAL_H__
#define __AK_VIDEO_PRIV_CMD_INTERNAL_H__

struct isp_timing_info;

#define FLIP_OFFSET     0
#define MIRROR_OFFSET   1

typedef int (*SET_ISP_MISC)(void *arg, struct isp_timing_info *isp_timing);

/*
 * priv_cmd_internal
 * @GET_INTERFACE:  get interface that sensor output
 * @GET_IO_LEVEL:   get io level that sensor support
 * @GET_MIPI_LANES: mipi is active. get mipi lanes
 * @GET_MIPI_MHZ:   mipi is active. get mipi MBPS
 * @GET_RESET_GPIO: get reset pin for sensor
 * @GET_PWDN_GPIO:  get power_down pin for sesnor
 * @GET AE_FASE_DEFAULT:    get default ae config for the sensor
 * @GET_SCAN_METHOD:get scanning method for sensor
 * @SET_ISP_MISC_CALLBACK:  set callback for misc timing
 * @SET_FPS_DIRECT: set sensor change to fps now
 * @SET_MASTER:     dual is active. set sensor to master
 * @SET_SLAVER:     dual is active. set sensor to slaver
 */
#ifdef CONFIG_MACH_KM01A
enum priv_cmd_internal {
    GET_SENSOR_CB = 0x301,
    GET_SENSOR_ID,          //struct priv_sensor_id
    GET_MAX_EXP_FOR_FPS,    //struct priv_max_exp_for_fps

    GET_INTERFACE = 0x2000,
    GET_IO_LEVEL,
    GET_MIPI_LANES_SWAP,
    GET_MIPI_LANES,
    GET_MIPI_MHZ,
    GET_RESET_GPIO,
    GET_PWDN_GPIO,
    GET_AE_FAST_DEFAULT,
    GET_SCAN_METHOD,        //scaning method
    GET_VSYNC_FS_EDGE,      //Vsync Frame start edge
    GET_VCODE_FS_EDGE,      //Vcode Frame start edge
    GET_DUAL_SYNC_MODE,     //enum dual_sync_mode
    GET_MAX_FPS,            //get the Sensor MAX_FPS
    GET_RAW_FORMAT,         //get raw bayer seq from cis
    GET_FLIP_DROP_FRAMES,  //get the frame number need to drop after cis fliped or mirrored function
    GET_BINING_MAX,
    GET_PWM_ENABLE_POINT,
    GET_STEPS_OF_1LINE_EXP,//get how many step of 1 line exp time
    GET_MAX_EXP, // get the Sensor max exp time
    GET_MAX_AGAIN, // get the Sensor max a_gain
    GET_MAX_DGAIN, // get the Sensor max d_gain
    GET_ONE_LINE_CYCLE,
    GET_HBLIANK_CYCLE,
    GET_EXP_BINING2NORMAL,
    GET_AOV_MASTER_MODE,

    SET_ISP_MISC_CALLBACK = 0x3000,
    SET_FPS_DIRECT,
    SET_MASTER,/*for dual sensor*/
    SET_SLAVER,/*for dual sensor*/
    SET_FLIP_MIRROR,
    SET_NORMAL_MODE,
    SET_ISP_DONE_CALL_CIS_WORK,
    SET_FLIP_MIRROR_FLAG,
    SET_FAST_AE,
    SET_INIT_FPS,
};
#else
enum priv_cmd_internal {
    GET_SENSOR_CB = 0x301,
    GET_INTERFACE = 0x2000,
    GET_IO_LEVEL,
    GET_MIPI_LANES,
    GET_MIPI_MHZ,
    GET_RESET_GPIO,
    GET_PWDN_GPIO,
    GET_AE_FAST_DEFAULT,
    GET_SCAN_METHOD,        //scaning method
    GET_VSYNC_FS_EDGE,      //Vsync Frame start edge
    GET_VCODE_FS_EDGE,      //Vcode Frame start edge
    GET_DUAL_SYNC_MODE,     //enum dual_sync_mode
    GET_MAX_FPS,            //get the Sensor MAX_FPS
    GET_RAW_BAYER_SEQ,  //get raw bayer seq from cis
    GET_FLIP_DROP_FRAMES,  //get the frame number need to drop after cis fliped or mirrored function
    GET_REAL_HEIGHT,        //get real ouput line
    GET_BINING_MAX,
    GET_AOV_MASTER_MODE,
    GET_STEPS_OF_1LINE_EXP,//get how many step of 1 line exp time
    GET_SYNC_DROP_FRAME,    //GCxx CIS Using forced sync requires dropping frames.
    SET_ISP_MISC_CALLBACK = 0x3000,
    SET_FPS_DIRECT,
    SET_MASTER,/*for dual sensor*/
    SET_SLAVER,/*for dual sensor*/
    SET_FLIP_MIRROR,
    SET_ISP_DONE_CALL_CIS_WORK,
    SET_FLIP_MIRROR_FLAG,
    SET_FAST_AE,//ae
    SET_NOMAL_MODE,
    SET_INIT_FPS,
};
#endif

enum interface {
    DVP_INTERFACE   = 0,
    MIPI_INTERFACE,
    DVP_INTERFACE_BT656,
    DVP_INTERFACE_BT1120,
};

enum io_level {
    IO_LEVEL_3V3 = 0,
    IO_LEVEL_2V5,
    IO_LEVEL_1V8,
};

enum scan_method {
    SCAN_METHOD_INTERLACED = 0, //interlaced
    SCAN_METHOD_PROGRESSIVE,    //progressive
};

/*
 * DVP_VSYNC_FS_EDGE_INVALID : configure Frame_start  by ISP_CONF file ,
 *      now only support bt601
 * DVP_VSYNC_FS_RISING : bt601 recognize Vsync_rising_edge as ISP_frame_start
 * DVP_VSYNC_FS_FALLING : bt601 recognize Vsync_falling_edge as ISP_frame_start
 */
enum vsync_frame_start_edge {
    DVP_VSYNC_FS_EDGE_INVALID = 0,
    DVP_VSYNC_FS_RISING,
    DVP_VSYNC_FS_FALLING,
};

/*
 * DVP_VCODE_FS_EDGE_INVALID : want to configure Frame_start by ISP_CONF file,
 *      but now must set RISING or FALLING
 * DVP_VCODE_FS_RISING : bt656 & bt1120 recognize frame_end_V_code
 *      as ISP_frame_start
 * DVP_VCODE_FS_FALLING : bt656 & bt1120 recognize frame_start_V_code
 *      as ISP_frame_start
 */
enum vcode_frame_start_edge {
    DVP_VCODE_FS_EDGE_INVALID = 0,
    DVP_VCODE_FS_RISING,
    DVP_VCODE_FS_FALLING,
};

/*
 * the value from dts 'dual-cis-none-mipi-switch'
 * or transfer parameters when load ak_isp.ko
 */
enum mulit_cis_hw_mode {
    MULTI_CIS_DEFAULT = 0, /* 100N/300L dual cis + MIPI switch or KM01X dual cis**/
    MULTI_CIS_NONE_MIPI_SWITCH,// 100N/300L dual cis + none-MIPI switch eg:H63P
                               // or KM01/300L triple cis + none-MIPI_Switch
};



enum raw_seque {
    RAW_SEQUE_R     = 0,
    RAW_SEQUE_B,
    RAW_SEQUE_GR,
    RAW_SEQUE_GB,
};

enum dual_sync_mode {
    DUAL_SYNC_BY_EFSYNC = 0,    //default
    DUAL_SYNC_BY_PWDN,          //mis2008 as we know
    DUAL_SYNC_PWM_FREQ_FIXED,   //gc2063 as we know
    /* the PWM duty cycle is 50%,other synchronous logic is equal to DUAL_SYNC_BY_EFSYNC
     * hardware design is that using a single CIS_PWM pin to trigger two sensors.
     * cv2003 as we know 
     * */
    DUAL_SYNC_TRIG_ON_BOTH_EDGES,
};

enum frame_event {
    EVENT_FRAME_LACK,
    EVENT_FRAME_TARGET,
    EVENT_FRAME_ALLDONE,
};

enum pwm_enable_point {
    ENABLE_POINT_SET_FPS = 0, // default
    ENABLE_POINT_SENSOR_INIT, // sc
    ENABLE_POINT_SENSOR_INIT_ALL,
};

struct isp_timing_info {
    int oneline;
    int fsden;
    int hblank;
    int fsdnum;
    int pclk;
};

struct isp_timing_cb_info {
    void *isp_timing_arg;
    SET_ISP_MISC set_isp_timing;
};
#endif
