#ifndef __AK_VICAP_APP_H__
#define __AK_VICAP_APP_H__

/*
 *
 * VICAP_APP_SET_LINKS : 用于绑定多个节点组成一个pipeline, 
 *                  例如vicap-》isp-》pp是一个pipeline
 * VICAP_APP_GET_ID : 好的这个vicap的ID号
 */
enum ak_vicap_app_cmd {
    VICAP_APP_SET_LINKS     = 0x100,
    VICAP_APP_SET_MAX_DMA_SIZE,
    VICAP_APP_SET_SENSOR_FPS,
    VICAP_APP_SET_BINING_MODE,
    VICAP_APP_SET_SENSOR_FPS_DIRECT,
    VICAP_APP_SET_LVICAP_CTRL_FPS,
    VICAP_APP_SET_STANDBY_NIGHT_INFO,

    VICAP_APP_GET_PAD_ID    = 0x200,
    VICAP_APP_GET_SENSOR_ID,
    VICAP_APP_GET_MAX_EXP_FOR_FPS,
    VICAP_APP_GET_SENSOR_FPS,
    VICAP_APP_GET_BINING_INFO,
    VICAP_APP_GET_SENSOR_MAX_FPS,
    VICAP_APP_GET_SENSOR_AE_CFG,
    VICAP_APP_GET_ONE_LINE_CYCLE,
    VICAP_APP_GET_HBLIANK_CYCLE,
};

struct ak_vicap_get_sensor_id {
    AK_S32 sensor_id;
};

struct ak_vicap_sensor_fps {
    AK_U32 fps;
};

struct ak_vicap_lvicap_ctrl_fps {
    AK_U32 fps;
};

enum ak_mipi_switch_app_cmd {
    MIPI_SWITCH_APP_SET_UPDATE     = 0x100,

    MIPI_SWITCH_APP_GET_STATUS    = 0x200,
};

struct ak_mipi_switch_app_update {
    AK_S32 switch_id;
    AK_S32 member_id;
};

typedef struct isp_night_info {
    unsigned int night_mode;   //1->nigth, 0->day
    unsigned int led_ctl_mode; //1->pwm控制,0->gpio控制
    unsigned int pin;          //GPIO控制,pin为GPIO，如果PWM控制，为PWM ID（0～4）
} isp_night_info_t;


#endif
