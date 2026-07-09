#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <error.h>
#include <errno.h>
#include "ak_common.h"
#include "ak_log.h"

#include "ak_rtsp_server.h"
#include "ak_circular_buffer.h"
#include "aas_ini_parser.h"

#define RTSP_URL_DEV "dev"

#define RTSP_URL_CHN0 "vs0"
#define RTSP_URL_CHN1 "vs1"
#define RTSP_URL_PREFIX "vs"
#define RTSP_MAX_CLIENT 8
typedef struct
{
    int iUsed;
    int iStreamType;    //0:实时流; 1:录像回放
    int iDev;
    int iChn;
    int iHandleV;
    int iHandleA;
}rtsp_stream_handle;
rtsp_stream_handle stStreamHandle[RTSP_MAX_CLIENT] = {0};
int iRecordStartTime = 0;
int rtsp_get_idle_handle()
{
    int nRet = -1;
    for (int i = 0; i < RTSP_MAX_CLIENT; i++)
    {
        if (stStreamHandle[i].iUsed == 0)
        {
            nRet = i;
            break;
        }
    }
    return nRet;
}

int aa_ls_get_audio_enc_param(rtsp_audio_enc_param* param);

int aa_ls_get_video_enc_param(int dev, int chn, rtsp_video_enc_param* param);

//0:main stream ; 1:sub stream ; 2:audio stream
int aa_ls_reg_stream_client(int nDevId,  int stream_id, int *pnHandle,char *pReaderName);

int aa_ls_unreg_stream_client(int nDevId, int stream_id, int nHandle);

int aa_ls_force_i_frame(int dev, int chn);

int aa_ls_get_frame(int nDevId, int stream_id, int nHandle, rtsp_frame_info *pFrame);

//错误返回码回调
int rtsp_server_get_error_code_cb(rtsp_error_type_e eErrorCode)
{
    printf("eErrorCode:%d", eErrorCode);
    return 0;
}

/**
 * rtsp_server_get_dev_param_cb: rtsp_server_get_dev_param_cb
 * @eParamType[IN]: eParamType
 * @pParam[IN]: pParam
 * return: 0 - success; otherwise error code;
 */
int rtsp_server_get_dev_param_cb(rtsp_param_type_e eParamType, rtsp_dev_param* pParam)
{
    switch (eParamType)
    {
    case RTSP_NET_ADDR_IPV4:
        //ak_net_get_ip("eth0", pParam->cAddrIPv4);
        sprintf(pParam->cAddrIPv4,"172.16.2.43");
        break;

    case RTSP_NET_ADDR_IPV6:
        //ak_net_get_ipv6("eth0", pParam->cAddrIPv6);
        break;
    default:
        break;
    }

    return 0;
}

/**
 * rtsp_server_get_video_param_cb: rtsp_server_get_video_param_cb
 * @pSourceToken[IN]: pSourceToken
 * @pParam[IN]: pParam
 * return: 0 - success; otherwise error code;
 */
int rtsp_server_get_video_param_cb(const char* pSourceToken, rtsp_video_enc_param* pParam)
{
    printf("pSourceToken:%s\n",pSourceToken);
    int dev = 0;
//    char* pDev = strstr(pSourceToken,RTSP_URL_DEV);
//    if(pDev)
//    {
//        dev = atoi(pDev + strlen(RTSP_URL_DEV));
//        printf("url dev:%d \n",dev);
//    }
    char* pStart = strstr(pSourceToken, RTSP_URL_PREFIX);
    if (pStart != NULL)
    {
        int iChn = atoi(pStart + strlen(RTSP_URL_PREFIX));
        //aa_video_enc_param param = { 0 };
        
        aa_ls_get_video_enc_param(dev, iChn, pParam);
        printf("eCodeType:%d\n",pParam->eCodeType);
        return 0;
    }

    return 0;
}
//获取音频参数回调
int rtsp_server_get_audio_param_cb(const char* pSourceToken, rtsp_audio_enc_param* pParam)
{
    //vs
    char* pStart = strstr(pSourceToken, RTSP_URL_PREFIX);
    if (pStart != NULL)
    {
        int iChn = atoi(pStart + strlen(RTSP_URL_PREFIX));
        //aa_ls_get_audio_enc_param(0, iChn,pParam);
        
        printf("iChn:%d ", iChn);
        return aa_ls_get_audio_enc_param(pParam);
    }

    return -1;
}

/**
 * rtsp_server_accept_client_cb: rtsp_server_accept_client_cb
 * @pClientIP[IN]: pClientIP
 * @iClientPort[IN]: iClientPort
 * return: 0 - success; otherwise error code;
 */
int rtsp_server_accept_client_cb(char* pClientIP, int iClientPort)
{
    printf("pClientIP:%s iClientPort=%d ", pClientIP, iClientPort);
    return 0;
}

/**
 * rtsp_server_play_cb: rtsp_server_play_cb
 * @pSourceToken[IN]: pSourceToken
 * return: 0 - success; otherwise error code;
 */
int rtsp_server_play_cb(char* pSourceToken)
{
    printf("play source token:%s",pSourceToken);
    int i = 0;
    int dev = 0;

//    char* pDev = strstr(pSourceToken,RTSP_URL_DEV);
//    if(pDev)
//    {
//        dev = atoi(pDev + strlen(RTSP_URL_DEV));
//        printf("url dev:%d \n",dev);
//    }

    char* pStart = strstr(pSourceToken, RTSP_URL_PREFIX);
    if (pStart != NULL)
    {
        int iChn = atoi(pStart + strlen(RTSP_URL_PREFIX));
        i = rtsp_get_idle_handle();
        if (i < 0)
        {
            printf("get idle handle error.");
            return -1;
        }
        char client[16] = {0};
        sprintf(client,"Vclient%d",i);
        int ret = aa_ls_reg_stream_client(dev, iChn, &stStreamHandle[i].iHandleV,client);
        if(ret)
        {
            return -1;
        }
        sprintf(client,"Aclient%d",i);
//        ret = aa_ls_reg_stream_client(dev, 2, &stStreamHandle[i].iHandleA,client);
//        if(ret)
//        {
//            aa_ls_unreg_stream_client(dev, stStreamHandle[i].iChn, stStreamHandle[i].iHandleV);
//            return -1;
//        }
        printf("iHandleV:%d iHandleA=%d",stStreamHandle[i].iHandleV,stStreamHandle[i].iHandleA);
        usleep(100);

        stStreamHandle[i].iUsed = 1;
        stStreamHandle[i].iStreamType = 0;
        printf("play source token:%d",__LINE__);
            
        aa_ls_force_i_frame(dev, iChn);
        usleep(100);
        printf("play source token:%d",__LINE__);
        stStreamHandle[i].iChn = iChn;
        stStreamHandle[i].iDev = dev;
        return i;
    }

    return 0;
}

/**
 * rtsp_server_teardown_cb: rtsp_server_teardown_cb
 * @iHandle[IN]: iHandle
 * return: 0 - success; otherwise error code;
 */
int rtsp_server_teardown_cb(int iHandle)
{
    printf("fun=%s handle = %d ,iStreamType=%d\n",__FUNCTION__,iHandle,stStreamHandle[iHandle].iStreamType);
    if (stStreamHandle[iHandle].iStreamType == 0)
    {
        printf("handle  %d ",iHandle);

        aa_ls_unreg_stream_client(stStreamHandle[iHandle].iDev, stStreamHandle[iHandle].iChn, stStreamHandle[iHandle].iHandleV);
        aa_ls_unreg_stream_client(stStreamHandle[iHandle].iDev, 2, stStreamHandle[iHandle].iHandleA);
        stStreamHandle[iHandle].iUsed = 0;
        stStreamHandle[iHandle].iStreamType = -1;
        return 0;
    }

    return 0;
}
//获取视频流媒体回调
int rtsp_server_get_video_cb(int iHandle, rtsp_frame_info* pVideo, void* pUserData)
{
//    printf("(%s %d)handle = %d ,iStreamType=%d\n",__FUNCTION__,__LINE__,iHandle,stStreamHandle[iHandle].iStreamType);
    if (stStreamHandle[iHandle].iStreamType == 0)
    {
        //ak_print_error_ex(MODULE_ID_APP, "handle = %d ,iStreamType=%d\n",iHandle,stStreamHandle[iHandle].iStreamType);
        int iVideoHandle = stStreamHandle[iHandle].iHandleV;
        int iDev = stStreamHandle[iHandle].iDev;
        int iChn = stStreamHandle[iHandle].iChn;
        return aa_ls_get_frame(iDev, iChn, iVideoHandle, pVideo);
    }
    return 0;
}


//获取音频流媒体回调
int rtsp_server_get_audio_cb(int iHandle, rtsp_frame_info* pAudio, void* pUserData)
{
    int iAudioHandle = stStreamHandle[iHandle].iHandleA;
    return aa_ls_get_frame(0, 2, iAudioHandle, pAudio);
}

/**
 * rtsp_server_init: rtsp_server_init
 * @void
 * return: 0 - success; otherwise error code;
 */
int rtsp_server_init()
{
    rtsp_server_init_param(NULL);

    //设置网络信息
    rtsp_server_set_dev_param_callback(rtsp_server_get_dev_param_cb);

    //设置获取视频参数回调
    rtsp_server_set_video_param_callback(rtsp_server_get_video_param_cb);

    //设置获取音频参数回调
    rtsp_server_set_audio_param_callback(rtsp_server_get_audio_param_cb);

    //客户端信息回调
    rtsp_server_set_accept_client_callback(rtsp_server_accept_client_cb);

    //play回调,返回点流的url
    rtsp_server_set_play_callback(rtsp_server_play_cb);

    //teardown回调,返回play的句柄
    rtsp_server_set_teardown_callback(rtsp_server_teardown_cb);

    //设置视频流媒体回调
    rtsp_server_set_video_callback(rtsp_server_get_video_cb, NULL);

    //设置音频流媒体回调
    rtsp_server_set_audio_callback(rtsp_server_get_audio_cb, NULL);

    //设置流媒体回调,包括视频和音频
    //rtsp_server_set_media_callback(rtsp_server_get_media_cb, NULL);

    //设置错误返回码回调
    rtsp_server_set_error_code_callback(rtsp_server_get_error_code_cb);

    //开启rtsp服务
    rtsp_server_start();

    return 0;
}

/**
 * rtsp_server_deinit: rtsp_server_deinit
 * @void
 * return: 0 - success; otherwise error code;
 */
int rtsp_server_deinit()
{
    rtsp_server_stop();
    return 0;
}


