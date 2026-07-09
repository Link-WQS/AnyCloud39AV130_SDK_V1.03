#ifndef _RTSP_SERVER_H_
#define _RTSP_SERVER_H_

//#include "rtsp_cmd.h"


#if !defined(WIN32)
    #define __PACKED__        __attribute__ ((__packed__))
#else
    #define __PACKED__
#endif


//int rtsp_init_client (rtsp_client_t * client);
//int rtsp_deinit_client (rtsp_client_t * client);
//int rtsp_live_stream_start(rtsp_client_t* client);
//int rtsp_live_stream_stop(rtsp_client_t* client);
//int rtsp_live_stream_thread(void *pParam);

/**
 * rtsp_server_get_version - get rtsp server version
 * return: version string
 * notes:
 */
const char* rtsp_server_get_version(void);

/**
 * rtsp_server_init: rtsp_server_init
 * @void
 * return: 0 - success; otherwise error code;
 */
int rtsp_server_init();

/**
 * rtsp_server_deinit: rtsp_server_deinit
 * @void
 * return: 0 - success; otherwise error code;
 */
int rtsp_server_deinit();

typedef enum
{
    E_RTSP_CONNECT_LOST     = -1,   //异常断开
    E_RTSP_NO_VIDEO_PARAM   = 0,    //没有视频参数
    E_RTSP_NO_AUDIO_PARAM   = 1,    //没有音频参数
    E_RTSP_NO_VIDEA_DATA    = 2,    //
    E_RTSP_NO_AUDIO_DATA    = 3,
    E_RTSP_NO_PLAY_CB       = 4,
    E_RTSP_NO_TEARDOWN_CB   = 5,
}rtsp_error_type_e;

typedef enum
{
    RTSP_NET_ADDR_IPV4 = 0,
    RTSP_NET_ADDR_IPV6 = 1,
    RTSP_PARAM_TYPE_MAX,
}rtsp_param_type_e;

typedef enum
{
    RTSP_AUDIO_TYPE_G726 = 0,
    RTSP_AUDIO_TYPE_AAC = 4,
    RTSP_AUDIO_TYPE_PCM_ALAW = 17,
    RTSP_AUDIO_TYPE_PCM_ULAW = 18,
}rtsp_audio_type;
typedef enum
{
    RTSP_VIDEO_H264 = 0,    // h264
    RTSP_VIDEO_MJPEG,
    RTSP_VIDEO_H265,
    RTSP_VIDEO_MAX,
}rtsp_video_type;
struct _rtsp_video_enc_param
{
    rtsp_video_type eCodeType;
    int	            iWidth;
    int	            iHeight;
    int             iFps; //帧率
    int             iBps; //码率
}__PACKED__;

typedef struct _rtsp_video_enc_param rtsp_video_enc_param;

struct _rtsp_audio_enc_param
{
    rtsp_audio_type eCodeType;
    int             iChannelNum;    //通道数
    int             iSampleBits;
    int             iSampleRate;
}__PACKED__;

typedef struct _rtsp_audio_enc_param rtsp_audio_enc_param;

struct _rtsp_frame_info
{
    int iCodeType;
    int iFrameType; //0: p帧   1: I帧
    unsigned int uiDataLen;
    unsigned int uiSeqNo;
    unsigned long long u64Time;
    int iWidth;
    int iHeight;
    int iSampleRate;
    int iSampleBits;
    int iChannelNum;
    unsigned char* pData;
} __PACKED__;

typedef struct _rtsp_frame_info rtsp_frame_info;


struct _rtsp_init_param
{
    unsigned int iServerPort;   // 本地tcp监听端口:默认554
    char cUserName[64];         //用户名
    char cPassword[64];         //密码
    char cRecordUrlPrefix[16];  //实时流url的前缀
    char cLiveStearmUrlPrefix[16];  //录像url的前缀

}__PACKED__;

typedef struct _rtsp_init_param rtsp_init_param;


struct _rtsp_dev_param
{
    char cAddrIPv4[16];         //ipv4地址
    char cAddrIPv6[64];         //ipv6地址

}__PACKED__;

typedef struct _rtsp_dev_param rtsp_dev_param;


/**
 * rtsp_server_dev_param: rtsp_server_dev_param
 * @pInitParam[IN]: init param
 * return: 0 - success; otherwise error code;
 */
int rtsp_server_dev_param(rtsp_init_param* pInitParam);

//用户名密码设置
int rtsp_server_init_param(rtsp_init_param* pInitParam);


//设置错误返回码回调
typedef int (*RTSP_SERVER_GET_ERROR_CODE_CALLBACK)(rtsp_error_type_e eErrorCode);
//设置错误返回码回调
int rtsp_server_set_error_code_callback(RTSP_SERVER_GET_ERROR_CODE_CALLBACK funGetErrorCodeCb);

//设置相关参数回调，如ip地址
typedef int (*RTSP_SERVER_GET_DEV_PARAM_CALLBACK)(rtsp_param_type_e eParamType,rtsp_dev_param* pDevParam);
//设置获取视频参数回调
int rtsp_server_set_dev_param_callback(RTSP_SERVER_GET_DEV_PARAM_CALLBACK funGetDevParamCb);



//视频参数回调
typedef int (*RTSP_SERVER_GET_VIDEO_PARAM_CALLBACK)(const char* pSourceToken, rtsp_video_enc_param* pParam);
//设置获取视频参数回调
int rtsp_server_set_video_param_callback(RTSP_SERVER_GET_VIDEO_PARAM_CALLBACK funGetVideoParamCb);

//音频参数回调
typedef int (*RTSP_SERVER_GET_AUDIO_PARAM_CALLBACK)(const char* pSourceToken, rtsp_audio_enc_param* pParam);
//设置获取音频参数回调
int rtsp_server_set_audio_param_callback(RTSP_SERVER_GET_AUDIO_PARAM_CALLBACK funGetAudioParamCb);

/****************************************
    接收客户回调
    非必须
    pClientIP:  连接客户的地址
    iClientPort:连接客户的端口
****************************************/
typedef int (*RTSP_SERVER_ACCEPT_CLIENT_CALLBACK)(char* pClientIP, int iClientPort);

/**
 * rtsp_server_set_accept_client_callback: rtsp_server_set_accept_client_callback
 * @funGetClientInfoCb[IN]: callback function
 * return: 0 - success; otherwise error code;
 */
int rtsp_server_set_accept_client_callback(RTSP_SERVER_ACCEPT_CLIENT_CALLBACK funGetClientInfoCb);


/****************************************
    pUrl:   客户点流的url
    收到play回调需要强制i帧，以免出图过慢
    return: 返回值为取流句柄，小于0失败
****************************************/
typedef int (*RTSP_SERVER_PLAY_CALLBACK)(char* pUrl);

/**
 * rtsp_server_set_play_callback: rtsp_server_set_play_callback
 * @funPlayCb[IN]: callback function
 * return: 0 - success; otherwise error code;
 */
int rtsp_server_set_play_callback(RTSP_SERVER_PLAY_CALLBACK funPlayCb);


/****************************************
    iHandle：   play callback返回的句柄，
                即取流句柄
****************************************/
typedef int (*RTSP_SERVER_TEARDOWN_CALLBACK)(int iHandle);

/**
 * rtsp_server_set_teardown_callback: rtsp_server_set_teardown_callback
 * @funTeardownCb[IN]: callback function
 * return: 0 - success; otherwise error code;
 */
int rtsp_server_set_teardown_callback(RTSP_SERVER_TEARDOWN_CALLBACK funTeardownCb);


//视频流媒体回调
typedef int (*RTSP_SERVER_GET_VIDEO_CALLBACK)(int iHandle, rtsp_frame_info*pVideo, void* pUserData);
//设置视频流媒体回调
int rtsp_server_set_video_callback(RTSP_SERVER_GET_VIDEO_CALLBACK funGetVideoCb, void* pUserData);


//音频流媒体回调
typedef int (*RTSP_SERVER_GET_AUDIO_CALLBACK)(int iHandle, rtsp_frame_info* pAudio, void* pUserData);
//设置流媒体回调
int rtsp_server_set_audio_callback(RTSP_SERVER_GET_AUDIO_CALLBACK funGetAudioCb, void* pUserData);

//音频流媒体回调
typedef int (*RTSP_SERVER_GET_MEDIA_CALLBACK)(int iHandle, rtsp_frame_info* pMedia, void* pUserData);
//设置流媒体回调
int rtsp_server_set_media_callback(RTSP_SERVER_GET_MEDIA_CALLBACK funGetMediaCb, void* pUserData);


//开启rtsp服务
int rtsp_server_start();

//关闭rtsp服务
int rtsp_server_stop();
//url设置(实时流/录像的前缀)


#endif

