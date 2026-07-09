/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_dual_cis_sample.c
* Description: This is a simple example to show
               how the dual video module and the stitch module working.
* Notes: Before running please insmod sensor_xxx.ko ak_isp.ko
         ak_venc_adapter.ko ak_venc_bridge.ko at first.
* History: V1.0.0
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "ak_common.h"
#include "ak_log.h"
#include "ak_common_video.h"
#include "ak_venc.h"
#include "ak_thread.h"
#include "ak_mem.h"
#include "ak_vi.h"
#include "ak_ai.h"
#include "ak_aenc.h"
#include "aenc_mp3.h"
#include "aenc_aac.h"
#include "aenc_amr.h"

#include "ak_video.h"
#include "ak_rtsp_server.h"
#include "ak_circular_buffer.h"
#include "aas_ini_parser.h"

#ifdef AK_RTOS
#include <kernel.h>
#define THREAD_PRIO 90
#define __STATIC__  static
#else
#define THREAD_PRIO -1
#define __STATIC__
#endif

/* video resolution num */
#define DE_VIDEO_SIZE_MAX    7

/* this is length for parse */
#define LEN_HINT                512
#define LEN_OPTION_SHORT        512

#define MAX_ENC_NUM   4

#ifndef __CHIP_AK37E_SERIES
#define THIRD_CHN_SUPPORT   1
#define DEF_FRAME_DEPTH     2
#else
#define THIRD_CHN_SUPPORT   0
#define DEF_FRAME_DEPTH     3
#endif


#define VIDEO_STITCH 0
#define MAX_SENSOR	4

struct audio_param
{
    int code_type ;     //  = 4
    int channel_num ;   //= 1
    int sample_bits ;   //= 16
    int sample_rate ;   //= 16000
};

struct venc_info
{
    int venc_handle;
    struct venc_param ve_param;
    int dev_id;
    int chn_id;
};

//audio
static int cycle = 1;
static int dev_sel = DEV_ADC;
static int ai_handle_id = 0;
static int frame_interval = 32;
static int aenc_handle_id = -1;
int flag = -1;    /*get aenc stream flag */


//static int target_kbps = 1024;

#define MAX_DEV_CNT 4
#define MAX_DEV_STREAM 2 //每个设备最多流数

typedef struct _rtsp_info
{
    int dev_cnt ;
    int single_dev;//输出的dev 0/1/2,-1为所有设备
    int stitch_mode ;//0(none) / 1(vertical) / 2(horizontal)
    int stitch_num; //拼接镜头数
    ak_pthread_t venc_stream_th[MAX_DEV_CNT][MAX_DEV_STREAM];
    char isp_conf[MAX_DEV_CNT][128];
    struct venc_info venc_info[MAX_DEV_CNT][MAX_DEV_STREAM];

    //ring buffer handle
    CIRBUF_HANDLE ring_buffer_video[MAX_DEV_CNT][MAX_DEV_STREAM];//0:main stream; 1:sub stream;
    ak_mutex_t lock_buffer_video[MAX_DEV_CNT][MAX_DEV_STREAM];

    CIRBUF_HANDLE ring_buffer_audio;
    ak_mutex_t lock_buffer_audio;

    //[audio]
    struct audio_param audio;
}rtsp_info_t;

rtsp_info_t rtsp_param= {0};

static char *pc_prog_name = NULL;                      //demo名称

static char *cfg_path     = "/etc/config/rtsp.ini";
int audio_enable = 1;

//static ak_mutex_t refresh_flag_lock;
static VI_CHN_ATTR_EX chn_attr = {0};
static VI_CHN_ATTR_EX chn_attr_sub = {0};
//static int encoder_type[MAX_SENSOR] = {0};
static int encode_mode = 0;


/* this is the message to print */
static char ac_option_hint[  ][ LEN_HINT ] = {                                         //操作提示数组
    "打印帮助信息" ,
    "配置文件路径",
    "",
};

/* opt for print the message */
static struct option option_long[ ] = {
    { "help"                , no_argument       , NULL , 'h' } ,      //"打印帮助信息" ,
    { "cfg_path"            , required_argument , NULL , 'i' } ,      //"配置文件路径" ,
    { 0                     , 0                 , 0    , 0   } ,
};


int rtsp_server_init();

/*
* get_option_short: fill the stort option string.
* return: option short string addr.
*/
static char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short, int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    /* get the option */
    for( i = 0 ; i < i_num_option ; i ++ )
    {
        if( ( c_option = p_option[ i ].val ) == 0 )
        {
            continue;
        }

        switch( p_option[ i ].has_arg )
        {
        case no_argument:
            /* if no argument, set the offset for default */
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c" , c_option );
            break;
        case required_argument:
            /* required argument offset calculate */
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c:" , c_option );
            break;
        case optional_argument:
            /* calculate the option offset */
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c::" , c_option );
            break;
        }
    }
    return pc_option_short;
}

static void usage(const char * name)
{
    printf("eg.: %s -i /etc/config/rtsp.ini\n", name);
    printf("rtsp cmd:rtsp://ip/554:dev[x]/vs[x]\n" );
}

/* if opt is not supported, print the help message */
static int help_hint(void)
{
    int i;

    printf("%s\n" , pc_prog_name);
    /* parse the all supported option */
    for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++)
    {
        if( option_long[ i ].val != 0 )
            printf("\t--%-16s -%c %s\n" ,
                                                option_long[ i ].name , option_long[ i ].val , ac_option_hint[ i ]);
    }
    usage(pc_prog_name);
    printf("\n\n");
    return AK_SUCCESS;
}

/* parse the option from cmd line */
static int parse_option( int argc, char **argv )
{
    int i_option;

    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ; /* get the option num*/
    char c_flag = AK_TRUE;
    pc_prog_name = argv[ 0 ];   /* get the option */

    optind = 0;
    get_option_short( option_long, i_array_num , ac_option_short , LEN_OPTION_SHORT );  /* parse the cmd line input */
    while((i_option = getopt_long(argc , argv , ac_option_short , option_long , NULL)) > 0)
    {
        switch(i_option)
        {
        case 'h' :                               //help
            help_hint();
            c_flag = AK_FALSE;
            goto parse_option_end;
        case 'i' :                              //rtsp ini file path
            cfg_path = optarg ;
            break;      
        default :
            help_hint();
            c_flag = AK_FALSE;
            goto parse_option_end;
        }
    }
parse_option_end:
    return c_flag;
}

/**
 * aa_ls_get_audio_enc_param: aa_ls_get_audio_enc_param
 * @param[IN]: audio enc param
 * return: 0 - success; otherwise error code;
 */
int aa_ls_get_audio_enc_param(rtsp_audio_enc_param* param)
{
    if(param == NULL || audio_enable == 0)
    {
        return -1;
    }

    param->iChannelNum = rtsp_param.audio.channel_num;
    param->eCodeType = rtsp_param.audio.code_type;
    param->iSampleBits = rtsp_param.audio.sample_bits;
    param->iSampleRate = rtsp_param.audio.sample_rate;

    return 0;
}

/**
 * aa_ls_get_video_enc_param: aa_ls_get_video_enc_param
 * @dev[IN]: dev
 * @chn[IN]: chn
 * @param[IN]: video enc param
 * return: 0 - success; otherwise error code;
 */
int aa_ls_get_video_enc_param(int dev, int chn, rtsp_video_enc_param* param)
{
    if(param == NULL || chn >= 2 || chn < 0)
    {
        return -1;
    }

    if(rtsp_param.stitch_mode == 1)
    {
        param->iHeight = rtsp_param.venc_info[dev][chn].ve_param.height * rtsp_param.stitch_num;
        param->iWidth = rtsp_param.venc_info[dev][chn].ve_param.width;

    }
    if(rtsp_param.stitch_mode == 2)
    {
        param->iHeight = rtsp_param.venc_info[dev][chn].ve_param.height;
        param->iWidth = rtsp_param.venc_info[dev][chn].ve_param.width * rtsp_param.stitch_num;

    }
    else
    {
        param->iHeight = rtsp_param.venc_info[dev][chn].ve_param.height;
        param->iWidth = rtsp_param.venc_info[dev][chn].ve_param.width;
    }

    param->eCodeType = rtsp_param.venc_info[dev][chn].ve_param.enc_out_type;
    param->iBps = rtsp_param.venc_info[dev][chn].ve_param.max_kbps;
    param->iFps = rtsp_param.venc_info[dev][chn].ve_param.fps;


    return 0;
}

//0:main stream ; 1:sub stream ; 2:audio stream
int aa_ls_reg_stream_client(int nDevId,  int stream_id, int *pnHandle,char *pReaderName)
{
    
    printf("aa_ls_reg_stream_client nDevId=%d,stream_id=%d pReaderName=%s\n",nDevId,stream_id,pReaderName);
    if(stream_id < 0 || stream_id >=3)
    {
        ak_print_error_ex(MODULE_ID_APP, "stream_id = %d error!\n",stream_id);
        return -1;
    }
    if(stream_id == 2)
    {
        printf("nDevId=%d,stream_id=%d  rtsp_param.ring_buffer_audio=%p\n",nDevId,stream_id,rtsp_param.ring_buffer_audio);
        *pnHandle = cirbuf_reader_new(rtsp_param.ring_buffer_audio,pReaderName);
    }
    else
    {
        printf("nDevId=%d,stream_id=%d\n",nDevId,stream_id);
        *pnHandle = cirbuf_reader_new(rtsp_param.ring_buffer_video[nDevId][stream_id],pReaderName);
    }
//    char client[16] = {0};
//    sprintf(client,"client%d",stream_id);
    return 0;
}

/**
 * aa_ls_unreg_stream_client: aa_ls_unreg_stream_client
 * @nDevId[IN]: dev
 * @stream_id[IN]: stream_id
 * @nHandle[IN]: nHandle
 * return: 0 - success; otherwise error code;
 */
int aa_ls_unreg_stream_client(int nDevId, int stream_id, int nHandle)
{
    if(stream_id < 0 || stream_id >=3)
    {
        ak_print_error_ex(MODULE_ID_APP, "stream_id = %d error!\n",stream_id);
        return -1;
    }
    
    if(stream_id == 2)
    {
        return cirbuf_reader_delete(rtsp_param.ring_buffer_audio, nHandle);
    }
    else
    {
        return cirbuf_reader_delete(rtsp_param.ring_buffer_video[nDevId][stream_id], nHandle);
    }

}

/**
 * aa_ls_force_i_frame: aa_ls_force_i_frame
 * @dev[IN]: dev
 * @chn[IN]: chn
 * return: 0 - success; otherwise error code;
 */
int aa_ls_force_i_frame(int dev, int chn)
{
    if(chn >= 2 || chn < 0)
    {
        return -1;
    }

    return ak_venc_request_idr(rtsp_param.venc_info[dev][chn].venc_handle);
}

/**
 * aa_ls_get_frame: aa_ls_get_frame
 * @nDevId[IN]: dev
 * @stream_id[IN]: stream_id
 * @nHandle[IN]: nHandle
 * @pFrame[OUT]: pFrame
 * return: 0 - success; otherwise error code;
 */
int aa_ls_get_frame(int nDevId, int stream_id, int nHandle, rtsp_frame_info *pFrame)
{
    if(stream_id < 0 || stream_id >=3)
    {
        ak_print_error_ex(MODULE_ID_APP, "stream_id = %d error!\n",stream_id);
        return -1;
    }
    int data_len = 0;
    if(stream_id == 2)
    {
            ak_thread_mutex_lock(&rtsp_param.lock_buffer_audio);
            int result = 0;
            unsigned char *data = NULL;
            int ret = cirbuf_get_buff_data(rtsp_param.ring_buffer_audio, nHandle, &data,&data_len, 0);
            if( ret < 0)
            {
                pFrame->uiDataLen = 0;
                pFrame->pData = NULL;
                
                ak_thread_mutex_unlock(&rtsp_param.lock_buffer_audio);
                return ret;
            }
        
            memcpy(pFrame,data,sizeof(rtsp_frame_info));
            pFrame->pData = data + sizeof(rtsp_frame_info);
        //    ak_print_error_ex(MODULE_ID_APP, "stream_id=%d nHandle=%d pFrame->pData=%p pFrame->iCodeType=%d,
        //pFrame->uiDataLen=%d,pFrame->iWidth=%d,pFrame->iHeight=%d u64Time=%d\n",
        //        stream_id,nHandle,pFrame->pData,pFrame->iCodeType,pFrame->uiDataLen,pFrame->iWidth,
        //pFrame->iHeight,pFrame->u64Time);
            ak_thread_mutex_unlock(&rtsp_param.lock_buffer_audio);

    }
    else
    {
        ak_thread_mutex_lock(&rtsp_param.lock_buffer_video[nDevId][stream_id]);
        int result = 0;
        unsigned char *data = NULL;
        int ret = cirbuf_get_buff_data(rtsp_param.ring_buffer_video[nDevId][stream_id], nHandle, &data,&data_len, 0);
        if( ret < 0)
        {
            pFrame->uiDataLen = 0;
            pFrame->pData = NULL;
            
            ak_thread_mutex_unlock(&rtsp_param.lock_buffer_video[nDevId][stream_id]);
            return ret;
        }
    
        memcpy(pFrame,data,sizeof(rtsp_frame_info));
        pFrame->pData = data + sizeof(rtsp_frame_info);
    //    ak_print_error_ex(MODULE_ID_APP, "stream_id=%d nHandle=%d pFrame->pData=%p pFrame->iCodeType=%d,
    //pFrame->uiDataLen=%d,pFrame->iWidth=%d,pFrame->iHeight=%d u64Time=%d\n",
    //        stream_id,nHandle,pFrame->pData,pFrame->iCodeType,pFrame->uiDataLen,pFrame->iWidth,
    //pFrame->iHeight,pFrame->u64Time);
        ak_thread_mutex_unlock(&rtsp_param.lock_buffer_video[nDevId][stream_id]);

    }

    return 0;
}

int rtsp_deinit_param_multi()
{

    return 0;
}

/**
 * rtsp_init_param_multi: rtsp_init_param_multi
 * @void
 * return: 0 - success; otherwise error code;
 */
static int rtsp_init_param_multi()
{
    AAS_INI_HANDLE ini = aas_ini_load(cfg_path);
    if (ini == NULL)
    {
        printf( "out: cannot parse file: %s\n", cfg_path);
        return -1;
    }
    const char *res_conf = NULL;

    rtsp_param.stitch_mode = aas_ini_get_int( ini, "camera","mode", 0);
    printf( "rtsp_param.stitch_mode: %d\n", rtsp_param.stitch_mode);

    
    rtsp_param.dev_cnt = aas_ini_get_int( ini, "camera","sensor_cnt", 1);

    if(rtsp_param.dev_cnt > MAX_DEV_CNT)
    {
        printf("sonsor_cnt %d error largest than MAX_DEV_CNT(%d)\n",rtsp_param.dev_cnt,MAX_DEV_CNT);
        aas_ini_free(ini);
        return -1;
    }
    printf( "rtsp_param.dev_cnt: %d\n", rtsp_param.dev_cnt);

    rtsp_param.stitch_num = aas_ini_get_int( ini, "stitch","stitch_num", 0);
    printf( "rtsp_param.stitch_num: %d\n", rtsp_param.stitch_num);

    if(rtsp_param.stitch_mode == 0)
    {
        for(int i = 0;i < rtsp_param.dev_cnt; i++)
        {
            snprintf(rtsp_param.isp_conf[i], sizeof(rtsp_param.isp_conf[i]), aas_ini_get_string(ini, "single_cfg","isp_conf", ""));
            printf( "rtsp_param.isp_conf[%d]: %s\n", i,rtsp_param.isp_conf[i]);  
        }
    }
    else
    {
        for(int i = 0;i < rtsp_param.stitch_num; i++)
        {
            char conf[16] = {0};
            sprintf(conf,"isp_conf%d",i);
            res_conf = aas_ini_get_string(ini, "stitch",conf, NULL);
            if(res_conf == NULL)
            {
                printf("error isp_conf %s is NULL.\n",conf);
                aas_ini_free(ini);
                return -1;
            }
            snprintf(rtsp_param.isp_conf[i], sizeof(rtsp_param.isp_conf[i]), res_conf);
            printf( "rtsp_param.isp_conf[%d]: %s\n", i,rtsp_param.isp_conf[i]);
        }


    }


    rtsp_param.single_dev = aas_ini_get_int( ini, "single_cfg","single_dev", 0);
    printf( "rtsp_param.single_dev: %d\n", rtsp_param.single_dev);

    int for_cnt = 0;
    if(rtsp_param.stitch_mode == 0)
    {
        for_cnt = rtsp_param.dev_cnt;
    }
    else
        for_cnt = rtsp_param.stitch_num;

    for(int i = 0;i < for_cnt; i++)
    {
        struct venc_param *vs = &rtsp_param.venc_info[i][0].ve_param;
        
        const char *code = aas_ini_get_string( ini, "video0","code_type", "h.264");
        if(strcmp(code,"h.264") == 0)
        {
            vs->enc_out_type = 0;
            vs->profile = PROFILE_MAIN;
        }
        else if(strcmp(code,"h.265") == 0)
        {
            vs->enc_out_type = 2;
            vs->profile = PROFILE_HEVC_MAIN;
        }
        else
        {
            vs->enc_out_type = 1;
            vs->profile = PROFILE_JPEG;
        }
        
        vs->width = aas_ini_get_int(ini, "video0","width", 1920);
        vs->height = aas_ini_get_int(ini, "video0","height", 1080);
        vs->fps = aas_ini_get_int(ini, "video0","fps", 20);
        vs->target_kbps  = aas_ini_get_int(ini, "video0","target_kbps", 1024);
        
        vs->goplen = vs->fps * 5;
        vs->max_kbps = vs->target_kbps + 512;
        vs->minqp       = 25;          //qp set
        vs->maxqp       = 50;          //qp max value
        vs->initqp      = (vs->minqp + vs->maxqp)/2;    //qp value
        vs->jpeg_qlevel = JPEG_QLEVEL_DEFAULT;     //jpeg qlevel
        vs->chroma_mode = CHROMA_4_2_0;            //chroma mode
        vs->max_picture_size = 0;                  //0 means default
        vs->enc_level        = 30;                 //enc level
        vs->smart_mode       = 0;                  //smart mode set
        vs->smart_goplen     = 100;                //smart mode value
        vs->smart_quality    = 50;                 //quality
        vs->smart_static_value = 0;                //value
        
        struct venc_param *vs1 = &rtsp_param.venc_info[i][1].ve_param;
        code = aas_ini_get_string( ini, "video1","code_type", "h.264");
        if(strcmp(code,"h.264") == 0)
        {
            vs1->enc_out_type = 0;
            vs1->profile = PROFILE_MAIN;
        }
        else if(strcmp(code,"h.265") == 0)
        {
            vs1->enc_out_type = 2;
            vs1->profile = PROFILE_HEVC_MAIN;
        }
        else
        {
            vs1->enc_out_type = 1;
            vs1->profile = PROFILE_JPEG;
        }
        
        vs1->width = aas_ini_get_int(ini, "video1","width", 640);
        vs1->height = aas_ini_get_int(ini, "video1","height",360);
        vs1->fps = aas_ini_get_int(ini, "video1","fps", 20);
        vs1->target_kbps = aas_ini_get_int(ini, "video1","target_kbps", 512);
        
        vs1->goplen = vs1->fps * 5;
        vs1->max_kbps = vs1->target_kbps + 512;
        vs1->minqp               = 20;           //qp set
        vs1->maxqp               = 48;           //qp max value
        vs1->initqp                  = 50;       //qp value
        vs1->jpeg_qlevel         = JPEG_QLEVEL_DEFAULT;  //jpeg qlevel
        vs1->chroma_mode         = CHROMA_4_2_0;         //chroma mode
        vs1->max_picture_size    = 0;                    //0 means default
        vs1->enc_level           = 30;                   //enc level
        vs1->smart_mode          = 0;                    //smart mode set
        vs1->smart_goplen        = 100;                  //smart mode value
        vs1->smart_quality       = 50;                   //quality
        vs1->smart_static_value  = 0;                    //value
        
    }

    const char *audio = aas_ini_get_string( ini, "audio","code_type", "aac");

    if(strcmp(audio,"g711a") == 0)
    {
        rtsp_param.audio.code_type = 17;
    }
    else if(strcmp(audio,"g711u") == 0)
    {
        rtsp_param.audio.code_type = 18;
    }
    else //aac
    {
        rtsp_param.audio.code_type = 4;
    }

    rtsp_param.audio.sample_rate = aas_ini_get_int(ini, "audio","sample_rate", 8000);
    rtsp_param.audio.channel_num = 1;
    rtsp_param.audio.sample_bits = 16;
    printf("rtsp_param.audio.code_type=%d,rtsp_param.audio.sample_rate=%d,rtsp_param.audio.sample_bits=%d\n",
        rtsp_param.audio.code_type,rtsp_param.audio.sample_rate,rtsp_param.audio.sample_bits);
    aas_ini_free(ini);
    printf("fun=%s line=%d debug here.\n",__FUNCTION__,__LINE__);

    if(rtsp_param.stitch_mode == 0)
    {

        printf("fun=%s line=%d debug here.\n",__FUNCTION__,__LINE__);
        int i = rtsp_param.single_dev;
        ak_thread_mutex_init(&rtsp_param.lock_buffer_video[0][0], NULL);
        ak_thread_mutex_init(&rtsp_param.lock_buffer_video[0][1], NULL);
    
        //main stream ring buffer
        rtsp_param.ring_buffer_video[0][0] = cirbuf_new(2* 1024 *1024);
    
        //sub stream ring buffer
        rtsp_param.ring_buffer_video[0][1] = cirbuf_new(512 *1024);
        printf("fun=%s line=%d debug here.(%p %p)\n",__FUNCTION__,__LINE__,rtsp_param.ring_buffer_video[0][0],
            rtsp_param.ring_buffer_video[0][1]);

    }
    else
    {
        ak_thread_mutex_init(&rtsp_param.lock_buffer_video[0][0], NULL);
        ak_thread_mutex_init(&rtsp_param.lock_buffer_video[0][1], NULL);
        
        //main stream ring buffer
        rtsp_param.ring_buffer_video[0][0] = cirbuf_new(2* 1024 *1024 * rtsp_param.stitch_num);
        
        //sub stream ring buffer
        rtsp_param.ring_buffer_video[0][1] = cirbuf_new(512 *1024 * rtsp_param.stitch_num);

    }
    printf("fun=%s line=%d debug here.\n",__FUNCTION__,__LINE__);

    ak_thread_mutex_init(&rtsp_param.lock_buffer_audio, NULL);

    //audio stream ring buffer
    rtsp_param.ring_buffer_audio = cirbuf_new(256 *1024);
    
    printf("fun=%s line=%d debug here.rtsp_param.ring_buffer_audio=%p\n",__FUNCTION__,__LINE__,
        rtsp_param.ring_buffer_audio);
    

    return 0;
}
/* end of rtsp_ini_param */

/*
    设置各种音频处理参数结构体为默认值，默认值是从ak_audio_config.h抄的。
    参数仅作参考，实际参数要根据不同产品独立设定。
    1：NEAR(ai), ak_audio_nr_attr
    2：NEAR(ai), ak_audio_agc_attr
    3：NEAR(ai), ak_audio_aec_attr
    4：NEAR(ai), ak_audio_aslc_attr
    5：FAR(ao),  ak_audio_nr_attr
    6：FAR(ao),  ak_audio_aslc_attr
    7：NEAR(ai), ak_ai_set_eq_attr
    8：FAR(ao),  ak_ai_set_eq_attr
*/
static void setup_default_audio_argument(void *audio_args, char args_type)
{
    struct ak_audio_nr_attr     default_ai_nr_attr      = {-40, 0, 1};
    struct ak_audio_agc_attr    default_ai_agc_attr     = {24576, 4, 0, 80, 0, 1};
    struct ak_audio_aec_attr    default_ai_aec_attr     = {0, 1024, 1024, 0, 512, 1, 0};
    struct ak_audio_aslc_attr   default_ai_aslc_attr    = {32768, 0, 0};
    struct ak_audio_eq_attr     default_ai_eq_attr      = {
    0,
    10,
    {50, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {717, 717, 717, 717, 717, 717, 717, 717, 717, 717},
    {TYPE_HPF, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, \
    TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1},
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    struct ak_audio_nr_attr     default_ao_nr_attr      = {0, 0, 0};
    struct ak_audio_aslc_attr   default_ao_aslc_attr    = {32768, 0, 0};
    struct ak_audio_eq_attr     default_ao_eq_attr      = {
    0,
    10,
    {50, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {717, 717, 717, 717, 717, 717, 717, 717, 717, 717},
    {TYPE_HPF, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, \
    TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1},
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    switch (args_type) {
    case 1:
        *(struct ak_audio_nr_attr*)audio_args = default_ai_nr_attr;
        break;
    case 2:
        *(struct ak_audio_agc_attr*)audio_args = default_ai_agc_attr;
        break;
    case 3:
        *(struct ak_audio_aec_attr*)audio_args = default_ai_aec_attr;
        break;
    case 4:
        *(struct ak_audio_aslc_attr*)audio_args = default_ai_aslc_attr;
        break;
    case 5:
        *(struct ak_audio_nr_attr*)audio_args = default_ao_nr_attr;
        break;
    case 6:
        *(struct ak_audio_aslc_attr*)audio_args = default_ao_aslc_attr;
        break;
    case 7:
        *(struct ak_audio_eq_attr*)audio_args = default_ai_eq_attr;
        break;
    case 8:
        *(struct ak_audio_eq_attr*)audio_args = default_ao_eq_attr;
        break;
    default:
        break;
    }

    return;
}

/**
 * init_ai: init_ai
 * @audio[IN]: audio param
 * return: 0 - success; otherwise error code;
 */
static int init_ai(struct audio_param *audio)
{
    struct ak_audio_in_param param;
    memset(&param, 0, sizeof(struct ak_audio_in_param));
    ak_print_notice_ex(MODULE_ID_APP, "sample_rate=%d\n", audio->sample_rate);
    param.pcm_data_attr.sample_rate = audio->sample_rate;              // set sample rate
    param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;    // sample bits only support 16 bit
    param.pcm_data_attr.channel_num = audio->channel_num;              // channel number
    param.dev_id = dev_sel;                                     //DEV_ADC;

    if (ak_ai_open(&param, &ai_handle_id)) {
        audio_enable = 0;
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_open failed\n");
        return AK_FAILED;
    }

    /* set source, source include mic and linein */
    if (DEV_ADC == dev_sel) {
        if (ak_ai_set_source(ai_handle_id, AI_SOURCE_MIC)) {
            ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_source failed\n");
            return AK_FAILED;
        }

        /* enable_nr, nr only support 8000 or 16000 sample rate */
        struct ak_audio_nr_attr nr_attr;
        setup_default_audio_argument(&nr_attr, 1);
        if (audio->sample_rate != AK_AUDIO_SAMPLE_RATE_8000 && audio->sample_rate != AK_AUDIO_SAMPLE_RATE_16000) {
            ak_print_warning_ex(MODULE_ID_APP, "ak_ai_set_nr_attr only support sample rate 8000 or 16000\n");
        } else {
            if (ak_ai_set_nr_attr(ai_handle_id, &nr_attr)) {
                ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_nr_attr failed\n");
                return AK_FAILED;
            }
        }

        /*enable_agc, agc only support 8000 or 16000 sample rate */
        struct ak_audio_agc_attr agc_attr;
        setup_default_audio_argument(&agc_attr, 2);
        if (audio->sample_rate != AK_AUDIO_SAMPLE_RATE_8000 && audio->sample_rate != AK_AUDIO_SAMPLE_RATE_16000) {
            ak_print_warning_ex(MODULE_ID_APP, "ak_ai_set_agc_attr only support sample rate 8000 or 16000\n");
        } else {
            if (ak_ai_set_agc_attr(ai_handle_id, &agc_attr)) {
                ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_agc_attr failed\n");
                return AK_FAILED;
            }
        }

        /*enable_aec, aec only support 8000 or 16000 sample rate, aec will real open when ai and ao all open */
        struct ak_audio_aec_attr aec_attr;
        setup_default_audio_argument(&aec_attr, 3);
        if (audio->sample_rate != AK_AUDIO_SAMPLE_RATE_8000 && audio->sample_rate != AK_AUDIO_SAMPLE_RATE_16000) {
            ak_print_warning_ex(MODULE_ID_APP, "ak_ai_set_aec_attr only support sample rate 8000 or 16000\n");
        } else {
            if (ak_ai_set_aec_attr(ai_handle_id, &aec_attr)) {
                ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_aec_attr failed\n");
                return AK_FAILED;
            }
        }
    }

    if (DEV_PDM == dev_sel) {
        ak_print_warning_ex(MODULE_ID_APP, "ak_ai_set_gain no support PDM\n");
    } else {
        if (ak_ai_set_gain(ai_handle_id, 5)) {
            ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_gain failed\n");
            return AK_FAILED;
        }
    }

    struct ak_audio_aslc_attr ai_aslc_attr;
    setup_default_audio_argument(&ai_aslc_attr, 4);
    if (ak_ai_set_aslc_attr(ai_handle_id, &ai_aslc_attr)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_aslc_attr failed\n");
        return AK_FAILED;
    }

    struct ak_audio_eq_attr ai_eq_attr;
    setup_default_audio_argument(&ai_eq_attr, 7);
    if (ak_ai_set_eq_attr(ai_handle_id, &ai_eq_attr)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_eq_attr failed\n");
        return AK_FAILED;
    }

    int frame_len = 0;
    if (ak_ai_get_frame_length(ai_handle_id, &frame_len)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_frame_length failed\n");
        return AK_FAILED;
    }
    frame_interval = ak_audio_len_to_interval(&param.pcm_data_attr, frame_len);

    return AK_SUCCESS;
}

/**
 * destroy_ai: destroy_ai
 * @void
 * return: void
 */
static void destroy_ai(void)
{
    if (-1 != ai_handle_id) {
        /* ai close */
        ak_ai_close(ai_handle_id);
        ai_handle_id = -1;
    }
}

/**
 * init_aenc: init_aenc
 * @audio[IN]: audio param
 * return: 0 - success; otherwise error code;
 */
static int init_aenc(struct audio_param *audio)
{
    /* open audio encode */
    struct aenc_param aenc_param;
    aenc_param.type = audio->code_type;
    aenc_param.aenc_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
    aenc_param.aenc_data_attr.channel_num = audio->channel_num;
    aenc_param.aenc_data_attr.sample_rate = audio->sample_rate;

    aenc_encode_s stencode = {0};
    stencode.enType = audio->code_type;
    switch (stencode.enType) {
        case AK_AUDIO_TYPE_MP3:
            printf("AK_AUDIO_TYPE_MP3\n");
            stencode.pfnEncOpen = AENC_OpenMp3;
            stencode.pfnEncVersion = AENC_Mp3_GetVersionInfo;
            if (ak_aenc_registerencoder(&stencode)) {
                ak_print_error_ex(MODULE_ID_APP, "ak_aenc_registerencoder failed\n");
                return AK_FAILED;
            }
            break;
        case AK_AUDIO_TYPE_AAC:
            printf("AK_AUDIO_TYPE_AAC\n");
            /*set aac param*/
            stencode.pfnEncOpen = AENC_OpenAac;
            stencode.pfnEncVersion = AENC_Aac_GetVersionInfo;
            if (ak_aenc_registerencoder(&stencode)) {
                ak_print_error_ex(MODULE_ID_APP, "ak_aenc_registerencoder failed\n");
                return AK_FAILED;
            }
            break;
        case AK_AUDIO_TYPE_AMR:
            printf("AK_AUDIO_TYPE_AMR\n");
            stencode.pfnEncOpen = AENC_OpenAmr;
            stencode.pfnEncVersion = AENC_Amr_GetVersionInfo;
            if (ak_aenc_registerencoder(&stencode)) {
                ak_print_error_ex(MODULE_ID_APP, "ak_aenc_registerencoder failed\n");
                return AK_FAILED;
            }
            break;
        case AK_AUDIO_TYPE_PCM_ALAW:
        case AK_AUDIO_TYPE_PCM_ULAW:
            break;
        default:
            break;
    }

    if (ak_aenc_open(&aenc_param, &aenc_handle_id)) {
        audio_enable = 0;
        ak_print_error_ex(MODULE_ID_APP, "ak_aenc_open failed\n");
        return AK_FAILED;
    }

    if (aenc_param.type == AK_AUDIO_TYPE_AAC) {
        struct aenc_attr attr;
        attr.aac_head = AENC_AAC_SAVE_FRAME_HEAD;
        if (ak_aenc_set_attr(aenc_handle_id, &attr)) {
            audio_enable = 0;
            ak_print_error_ex(MODULE_ID_APP, "ak_aenc_set_attr failed\n");
            return AK_FAILED;
        }
    }
    return AK_SUCCESS;
}

/**
 * destroy_aenc: destroy_aenc
 * @void
 * return: void
 */
static void destroy_aenc(void)
{
    if (-1 != aenc_handle_id) {
        /* aenc close */
        ak_aenc_close(aenc_handle_id);
        aenc_handle_id = -1;
    }
}

/**
 * audio_get_stream: audio_get_stream
 * @arg[IN]: arg
 * return: 0 - success; otherwise error code;
 */
static void* audio_get_stream(void *arg)
{
    ak_thread_set_name("[aenc stream]attachment");
    int chn = (int)arg; //venc handle
    struct audio_stream stream = {0};
    int ret = -1;
    int sleep_time = frame_interval == 0 ? 16: (frame_interval / 2);
    ak_print_normal_ex(MODULE_ID_APP, "sleep_time=%d\n", sleep_time);

    while(1 == flag) {
        ret = ak_aenc_get_stream(aenc_handle_id, &stream, 0);
        if (ret)
        {
            if (ERROR_AENC_NO_DATA == ret) {
                ak_sleep_ms(sleep_time);
                continue;
            } else {
                ak_print_error_ex(MODULE_ID_APP, "ak_aenc_get_stream error ret=%d\n", ret);
                break;
            }
        }
        else
        {
            if(stream.data && stream.len)
            {
                //写入ring buffer
                rtsp_frame_info frame_info = {0};
                frame_info.u64Time = stream.ts;
                frame_info.uiSeqNo = stream.seq_no;
                frame_info.uiDataLen = stream.len;
                frame_info.pData = stream.data;
                frame_info.iSampleBits  = rtsp_param.audio.sample_bits;
                frame_info.iSampleRate  = rtsp_param.audio.sample_rate;
                frame_info.iChannelNum = rtsp_param.audio.channel_num;
                frame_info.iCodeType = rtsp_param.audio.code_type;
                ak_thread_mutex_lock(&rtsp_param.lock_buffer_audio);

                int frame_len = sizeof(rtsp_frame_info);
                char *buf_tmp = cirbuf_get_write_buf(rtsp_param.ring_buffer_audio,stream.len + frame_len);
                memcpy(buf_tmp,&frame_info,frame_len);
                memcpy(buf_tmp + frame_len,stream.data,stream.len);
                cirbuf_put_write_buf(rtsp_param.ring_buffer_audio,stream.len + frame_len);
                ak_thread_mutex_unlock(&rtsp_param.lock_buffer_audio);

            }
            else
            {
                continue;
            }
        }
        ak_aenc_release_stream(aenc_handle_id, &stream);
    }

    ak_print_notice_ex(MODULE_ID_APP, "%s exit\n", __func__);
    ak_thread_exit();
    return NULL;
}

/**
 * audio_get_frame_encode: audio_get_frame_encode
 * @arg[IN]: arg
 * return: 0 - success; otherwise error code;
 */
static void* audio_get_frame_encode(void *arg)
{
    ak_thread_set_name("[aenc frame]attachment");

    struct frame ai_frame = {0};
    unsigned long long start_ts = 0;// use to save capture start time
    unsigned long long end_ts = 0;  // the last frame time

    int ret = AK_FAILED;
    int sleep_time = frame_interval == 0 ? 16: (frame_interval / 2);
    ak_print_normal_ex(MODULE_ID_APP, "sleep_time=%d\n", sleep_time);

    ret = ak_ai_start_capture(ai_handle_id);
    if (ret) {
        ak_print_error(MODULE_ID_APP, "*** ak_ai_start_capture failed. ***\n");
        return NULL;
    }

    while (1) {
        ret = ak_ai_get_frame(ai_handle_id, &ai_frame, 0);
        if (ret) {
            if (ERROR_AI_NO_DATA == ret) {
                ak_sleep_ms(sleep_time);
                continue;
            } else {
                ak_print_error_ex(MODULE_ID_APP, "ak_ai_get_frame error ret=%d\n", ret);
                flag = 0;
                break;
            }
        }
        ret = ak_aenc_send_frame(aenc_handle_id, &ai_frame, 0);
        if (ret) {
            ak_ai_release_frame(ai_handle_id, &ai_frame);
            ak_print_error_ex(MODULE_ID_APP, "send frame error\n");
            flag = 0;
            break;
        }

        if (0 == start_ts) {
            start_ts = ai_frame.ts;
        }

        end_ts = ai_frame.ts;
        ak_ai_release_frame(ai_handle_id, &ai_frame);

    }
    ak_ai_stop_capture(ai_handle_id);
    ak_print_notice_ex(MODULE_ID_APP, "get_frame_encode exit\n");
    ak_thread_exit();
    return NULL;
}
/* end of func */


/**
 * start_vi: start_vi
 * @dev_id[IN]: dev_id
 * return: 0 - success; otherwise error code;
 */
static int start_vi(int dev_id, int dual_cis_mode)
{
    /*
     * step 0: global value initialize
     */
    int ret = -1;                                //return value

    unsigned int width = rtsp_param.venc_info[dev_id][0].ve_param.width;
    unsigned int height = rtsp_param.venc_info[dev_id][0].ve_param.height;
    unsigned int subwidth = rtsp_param.venc_info[dev_id][1].ve_param.width;;
    unsigned int subheight = rtsp_param.venc_info[dev_id][1].ve_param.height;

    int chn_main_id = VIDEO_CHN0;
    int chn_sub_id = VIDEO_CHN1;

    if (dev_id == VIDEO_DEV1){
        chn_main_id = VIDEO_CHN2;
        chn_sub_id = VIDEO_CHN3;
    }
    else if(dev_id == VIDEO_DEV2){
        chn_main_id = VIDEO_CHN4;
        chn_sub_id = VIDEO_CHN5;
    }

    /* open vi flow */

    /*
     * step 1: open video input device
     */
    ret = ak_vi_open(dev_id);
    if (AK_SUCCESS != ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device %d open failed\n", dev_id);
        return ret;
    }

#ifndef __CHIP_AK37E_SERIES
    /*
     * step 2: load isp config
     */
    ret = ak_vi_load_sensor_cfg(dev_id, rtsp_param.isp_conf[dev_id]);
    if (AK_SUCCESS != ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device %d load isp cfg [%s] failed!\n", dev_id, rtsp_param.isp_conf[dev_id]);
        return ret;
    }
#endif
    /*
     * step 3: get sensor support max resolution
     */
    RECTANGLE_S res;                //max sensor resolution
    VI_DEV_ATTR    dev_attr;
    memset(&dev_attr, 0, sizeof(VI_DEV_ATTR));
    dev_attr.dev_id = dev_id;
    dev_attr.crop.left = 0;
    dev_attr.crop.top = 0;
    dev_attr.crop.width = width;
    dev_attr.crop.height = height;
    dev_attr.max_width = width;
    dev_attr.max_height = height;
    dev_attr.sub_max_width = subwidth;
    dev_attr.sub_max_height = subheight;
    /* init the interface mode according to the input param */
    if(dual_cis_mode != 0)
        dev_attr.interf_mode = VI_INTF_DUAL_MIPI_2;

    dev_attr.frame_rate = rtsp_param.venc_info[dev_id][0].ve_param.fps;

    if(dev_id == VIDEO_DEV0 && rtsp_param.stitch_mode != DISABLED_STITCH )
    {
        //dev_attr.frame_rate = 0;
        dev_attr.interf_mode = VI_INTF_DUAL_MIPI_2;
    }

    /* get sensor resolution */
    ret = ak_vi_get_sensor_resolution(dev_id, &res);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP,
             "Can't get dev[%d]resolution\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    } else {
        ak_print_normal_ex(MODULE_ID_APP,
            "get dev res w:[%d]h:[%d]\n",res.width, res.height);
        dev_attr.crop.width = res.width;
        dev_attr.crop.height = res.height;
    }

    /*
     * step 4: set vi device working parameters
     * default parameters: 25fps, day mode
     */
    ret = ak_vi_set_dev_attr(dev_id, &dev_attr);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP,
            "vi device %d set device attribute failed!\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    }

    /*
     * step 5: set main channel attribute
     */

    memset(&chn_attr, 0, sizeof(VI_CHN_ATTR_EX));
    chn_attr.chn_id = chn_main_id;
    chn_attr.res.width = width;
    chn_attr.res.height = height;
    chn_attr.frame_depth = DEF_FRAME_DEPTH;
    /*disable frame control*/
    chn_attr.frame_rate = rtsp_param.venc_info[dev_id][0].ve_param.fps;
    chn_attr.mode = FRAME_MODE;

    if(rtsp_param.stitch_mode == VERTICAL_STITCH && encode_mode == 2)
    {
        chn_attr.mode = SLICE_MODE;
        chn_attr.frame_depth = 3;
    }

    ret = ak_vi_set_chn_attr_ex(chn_main_id, &chn_attr);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP,
            "vi device %d set channel [%d] attribute failed!\n", dev_id, chn_main_id);
        ak_vi_close(dev_id);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP,
        "vi device %d main sub channel attribute\n", dev_id);

#ifndef __CHIP_AK37E_SERIES
    /*
     * step 6: set sub channel attribute
     */

    memset(&chn_attr_sub, 0, sizeof(VI_CHN_ATTR_EX));
    chn_attr_sub.chn_id = chn_sub_id;
    chn_attr_sub.res.width = subwidth;
    chn_attr_sub.res.height = subheight;
    chn_attr_sub.frame_depth = DEF_FRAME_DEPTH;
    /*disable frame control*/
    chn_attr_sub.frame_rate = rtsp_param.venc_info[dev_id][1].ve_param.fps;

    ret = ak_vi_set_chn_attr_ex(chn_sub_id, &chn_attr_sub);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP,
            "vi device %d set channel [%d] attribute failed!\n", dev_id, chn_sub_id);
        ak_vi_close(dev_id);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP,
        "vi device %d set sub channel attribute\n", dev_id);
#endif

    return  ret;
}
/* end of func */

static int stop_vi(int dev_id)
{
    int ret = -1;
    int chn_main_id = VIDEO_CHN0;
    int chn_sub_id = VIDEO_CHN1;

    if (dev_id == VIDEO_DEV1){
        chn_main_id = VIDEO_CHN2;
        chn_sub_id = VIDEO_CHN3;
    }
    else if(dev_id == VIDEO_DEV2){
        chn_main_id = VIDEO_CHN4;
        chn_sub_id = VIDEO_CHN5;
    }

    ak_vi_disable_chn(chn_main_id);

#ifndef __CHIP_AK37E_SERIES
    ak_vi_disable_chn(chn_sub_id);
#endif

    ak_vi_disable_dev(dev_id);
    ret = ak_vi_close(dev_id);

    return ret;
}

static int stop_stitch_chn(int chn_id)
{
    int ret = -1;
    ak_vi_disable_chn(chn_id);
    if(chn_id == VIDEO_CHN24)
    {
        ak_vi_disable_chn(VIDEO_CHN0);
        ak_vi_disable_chn(VIDEO_CHN2);
        if(rtsp_param.stitch_num == 3)
            ak_vi_disable_chn(VIDEO_CHN4);
    }
    else
    {
        ak_vi_disable_chn(VIDEO_CHN1);
        ak_vi_disable_chn(VIDEO_CHN3);
        if(rtsp_param.stitch_num == 3)
            ak_vi_disable_chn(VIDEO_CHN5);
    }

    ak_vi_destroy_stitch_chn(chn_id);

    ak_vi_disable_dev(VIDEO_DEV0);
    ak_vi_disable_dev(VIDEO_DEV1);
    if(rtsp_param.stitch_num == 3)
        ak_vi_disable_dev(VIDEO_DEV2);

    ak_vi_close(VIDEO_DEV0);
    ak_vi_close(VIDEO_DEV1);
    if(rtsp_param.stitch_num == 3)
        ret = ak_vi_close(VIDEO_DEV2);

    return ret;
}



static int enable_vi(int dev_id)
{
    int chn_main_id = VIDEO_CHN0;
    int chn_sub_id = VIDEO_CHN1;

    if (dev_id == VIDEO_DEV1)
    {
        chn_main_id = VIDEO_CHN2;
        chn_sub_id = VIDEO_CHN3;
    }
    else if(dev_id == VIDEO_DEV2)
    {
        chn_main_id = VIDEO_CHN4;
        chn_sub_id = VIDEO_CHN5;
    }
    /*
     * step 8: enable vi device
     */
     
    printf("fun=%s line=%d debug here.\n",__FUNCTION__,__LINE__);
    int ret = ak_vi_enable_dev(dev_id);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP,
            "vi device %d enable device  failed!\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    }
    printf("fun=%s line=%d debug here.\n",__FUNCTION__,__LINE__);

    /*
     * step 9: enable vi main channel
     */
    ret = ak_vi_enable_chn(chn_main_id);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP,
            "vi channel[%d] enable failed!\n", chn_main_id);
        ak_vi_close(dev_id);
        return ret;
    }
    printf("fun=%s line=%d debug here.\n",__FUNCTION__,__LINE__);

#ifndef __CHIP_AK37E_SERIES
    /*
     * step 10: enable vi sub channel
     */
    ret = ak_vi_enable_chn(chn_sub_id);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP,
            "vi channel[%d] enable failed!\n",chn_sub_id);
        ak_vi_close(dev_id);
        return ret;
    }
    
    printf("fun=%s line=%d debug here.\n",__FUNCTION__,__LINE__);
#endif

    return ret;
}
static int disable_vi(int dev_id)
{
    ak_vi_close(dev_id);

    return 0;
}


// 获取当前时间(毫秒)
static uint64_t get_current_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static int start_stitch_chn(int chn_id, int stitch_mode)
{
    int ret=AK_FAILED;
    VI_STITCH_ATTR stitch_attr = {0};
    stitch_attr.mode = stitch_mode;
    stitch_attr.stitch_chn_num = rtsp_param.dev_cnt;

    if(chn_id %2 == 0)
    {
        stitch_attr.stitch_bind_chn[0]=VIDEO_CHN0;
        stitch_attr.stitch_bind_chn[1]=VIDEO_CHN2;
        if(rtsp_param.dev_cnt == 3)
            stitch_attr.stitch_bind_chn[2]=VIDEO_CHN4;

        //stitch main channel
        ret = ak_vi_create_stitch_chn(VIDEO_CHN24, NULL, &stitch_attr);
    }
    else
    {
        stitch_attr.stitch_bind_chn[0]=VIDEO_CHN1;
        stitch_attr.stitch_bind_chn[1]=VIDEO_CHN3;
        if(rtsp_param.dev_cnt == 3)
            stitch_attr.stitch_bind_chn[2]=VIDEO_CHN5;


        //stitch sub channel
        ret = ak_vi_create_stitch_chn(VIDEO_CHN25, NULL, &stitch_attr);
    }

    if(ret != AK_SUCCESS)
    {
        ak_print_error_ex(MODULE_ID_APP,
            "create stitch_chn [%d] failed!\n", chn_id);
    }
    return ret;
}


/* get the encode data from vi module */
static void *video_encode_from_vi_th(void *arg)
{
    struct venc_info *venc_th = (struct venc_info *)arg; //venc handle


    int dev_id = venc_th->dev_id;
    int chn = venc_th->chn_id;
    VI_CHN chn_id = dev_id * 2 + chn;
    char th[64] = {0};
    sprintf(th,"venc%d_th",chn);
    ak_thread_set_name(th);

    ak_print_normal(MODULE_ID_APP, "capture start\n");

    RECTANGLE_S res;

    res.width = rtsp_param.venc_info[dev_id][chn].ve_param.width;
    res.height = rtsp_param.venc_info[dev_id][chn].ve_param.height;

    if(rtsp_param.stitch_mode != DISABLED_STITCH)
    {
        if(rtsp_param.stitch_mode == VERTICAL_STITCH){
            res.height = res.height * rtsp_param.dev_cnt;   //VERTICAL_STITCH
        }

        if(rtsp_param.stitch_mode == HORIZONTAL_STITCH){
            res.width = res.width * rtsp_param.dev_cnt;   //HORIZONTAL_STITCH
        }

        if(chn_id %2 == 0)
        {
            chn_id = VIDEO_CHN24;
        }
        else
        {
            chn_id = VIDEO_CHN25;
        }
    }


    APP_CHN_S Schn;
    APP_CHN_S Dchn;
    Schn.mid = MODULE_ID_VI;
    Schn.oid = chn_id;
    Dchn.mid = MODULE_ID_VENC;
    Dchn.oid = venc_th->venc_handle;
    APP_BIND_PARAM param;
    param.frame_rate = venc_th->ve_param.fps;
    param.frame_depth = 200;
    param.mode = USER_ENCODE_MODE;
    if(chn_id == VIDEO_CHN24 && rtsp_param.stitch_mode == VERTICAL_STITCH && encode_mode != 0)
        param.mode = KERNEL_ENCODE_MODE;


    ak_print_normal_ex(MODULE_ID_APP, "Encoder [%d] bind chn!\n",Dchn.oid);
    int ret = ak_app_video_bind_chn(&Schn, &Dchn, &param);
    if(ret != AK_SUCCESS)
    {
        ak_print_error_ex(MODULE_ID_APP,
            "ak_app_video_bind_chn failed [%d]\n",ret);
        return NULL;
    }

    /* Force request I frame */
    ak_venc_request_idr(venc_th->venc_handle);

    ak_print_normal_ex(MODULE_ID_APP, "Encoder [%d] activate chn!\n",Dchn.oid);
    ret = ak_app_video_set_dst_chn_active(&Dchn, AK_TRUE);

    struct video_stream *stream = ak_mem_alloc(MODULE_ID_APP, sizeof(struct video_stream));
    if(NULL == stream)
    {
        ak_print_error_ex(MODULE_ID_APP,
            "Can't alloc memory for stream buffer!\n");
        return NULL;
    }
    int count = 0;
    int start_t = get_current_time_ms();
    while (1)
    {
        
        int end_t = get_current_time_ms();
        if(end_t - start_t > 2000)
        {
            ak_print_normal_ex(MODULE_ID_VENC, "encode fps:%d\n", count * 1000 / (end_t - start_t) );
            start_t = end_t;
            count = 0;
        }
        memset(stream, 0, sizeof(struct video_stream));
        ret = ak_app_video_venc_get_stream(&Dchn, stream);
        if(ret == AK_SUCCESS)
        {
            if(stream->len > 0)
            {
                //写入ring buffer
                rtsp_frame_info frame_info = {0};
                frame_info.iCodeType = venc_th->ve_param.enc_out_type;
                frame_info.iWidth  = venc_th->ve_param.width;
                frame_info.iHeight = venc_th->ve_param.height;
                frame_info.iFrameType = stream->frame_type;
                frame_info.u64Time = stream->ts;
                frame_info.uiSeqNo = stream->seq_no;
                frame_info.uiDataLen = stream->len;
                frame_info.pData = stream->data;
                ak_thread_mutex_lock(&rtsp_param.lock_buffer_video[dev_id][chn]);
                int frame_len = sizeof(rtsp_frame_info);
                char *buf_tmp = cirbuf_get_write_buf(rtsp_param.ring_buffer_video[dev_id][chn],stream->len + frame_len);
                memcpy(buf_tmp,&frame_info,frame_len);
                memcpy(buf_tmp + frame_len,stream->data,stream->len);
                cirbuf_put_write_buf(rtsp_param.ring_buffer_video[dev_id][chn],stream->len + frame_len);
                ak_thread_mutex_unlock(&rtsp_param.lock_buffer_video[dev_id][chn]);
                count++;
//              if (0 == count % 50)
//                  ak_print_normal_ex(MODULE_ID_VENC, "encode num:%d\n", count);
            }
            ak_venc_release_stream(venc_th->venc_handle, stream);

        }
        else
        {
            //ak_print_error_ex(MODULE_ID_APP, "ak_app_video_venc_get_stream fail [%x]!\n",ret);
            ak_sleep_ms(50);
        }
        
    }

    if(stream)
        ak_mem_free(stream);

    ak_print_normal_ex(MODULE_ID_APP, "Encoder [%d] disable chn!\n", Dchn.oid);
    ret = ak_app_video_set_dst_chn_active(&Dchn, AK_FALSE);

    ak_print_normal_ex(MODULE_ID_APP,
        "Encoder [%d] unbind chn!\n", Dchn.oid );
    ret = ak_app_video_unbind_chn(&Schn, &Dchn);
    if(ret != AK_SUCCESS)
    {
        ak_print_error_ex(MODULE_ID_APP,
            "ak_app_video_unbind_chn failed [%d]\n",ret);
    }

    return NULL;
}
/* end of func */


/**
 * start_venc: start_venc
 * @chn[IN]: chn
 * return: 0 - success; otherwise error code;
 */
static int start_venc(int dev_id,int chn)
{
    int ret = -1;

    int width = rtsp_param.venc_info[dev_id][chn].ve_param.width;
    int height = rtsp_param.venc_info[dev_id][chn].ve_param.height;

    int handle_id = -1;

    /* open venc */
    struct venc_param ve_param = {0};

    memcpy(&ve_param,&rtsp_param.venc_info[dev_id][chn].ve_param,sizeof(struct venc_param));

    struct venc_rc_param rc_param = {0};
    rc_param.enable_MMA = 1;

    ve_param.width  = width;            //resolution width
    ve_param.height = height;           //resolution height

    if(rtsp_param.stitch_mode == VERTICAL_STITCH)
    {
        ve_param.height += ve_param.height;      //stitch height
        if(rtsp_param.stitch_num == 3 ) //dev2 main chn
            ve_param.height += height;
    }

    if(rtsp_param.stitch_mode == HORIZONTAL_STITCH){
        ve_param.width += ve_param.width;      //stitch width
        if(rtsp_param.stitch_num == 3 ) //dev2 main chn
            ve_param.width += width; 

    }

    ret = ak_venc_open_ex(&ve_param, &rc_param,  &handle_id);

    if (ret || (-1 == handle_id) )
    {
        ak_print_error_ex(MODULE_ID_APP, "dev%d open venc failed\n", dev_id);
        return ret;
    }

    if (ve_param.width > 1920 && ve_param.height > 1080 && ve_param.initqp > 60)
    {
        ak_venc_set_stream_buff(handle_id, 1*1024*1024);
    }
    
    rtsp_param.venc_info[dev_id][chn].venc_handle = handle_id;
    rtsp_param.venc_info[dev_id][chn].chn_id = chn;

    /* create the venc thread */
    ak_thread_create(&rtsp_param.venc_stream_th[dev_id][chn], video_encode_from_vi_th, 
        &(rtsp_param.venc_info[dev_id][chn]), ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);

    return 0;
}
/* end of func */

/**
 * note: output two separate stream mode video file, or output stitch mode video file
 */
#ifdef AK_RTOS
static int multi_CISs_sample(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    /* init sdk running */
    sdk_run_config config;
    memset(&config, 0, sizeof(config));             //init the struct
    config.mem_trace_flag = SDK_RUN_NORMAL;
    config.isp_tool_server_flag = 0;                //isp tool sever
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_APP, "*****************************************\n");
    ak_print_normal(MODULE_ID_APP, "** venc version: %s **\n", ak_venc_get_version());
    ak_print_normal(MODULE_ID_APP, "*****************************************\n");

    for(int i = 0;i < MAX_DEV_CNT;i++)
    {
        rtsp_param.venc_info[i][0].venc_handle = -1;
        rtsp_param.venc_info[i][1].venc_handle = -1;
        rtsp_param.venc_stream_th[i][0] = -1;
        rtsp_param.venc_stream_th[i][1] = -1;
    }

    /* start to parse the opt */
    if( parse_option( argc, argv ) == AK_FALSE )
    {                                               //解释和配置选项
        return AK_FAILED;
    }
/* get param from cmd line */
    int ret = -1;
    ret = rtsp_init_param_multi();
    if(ret)
    {
        ak_sdk_exit();
        ak_print_normal_ex(MODULE_ID_APP, "rtsp_init_param error. \n");
        return AK_FAILED;
    }
    
    /* param check */
    if(rtsp_param.stitch_num == 3 && rtsp_param.stitch_mode == HORIZONTAL_STITCH)
    {
        rtsp_deinit_param_multi();
        ak_print_error_ex(MODULE_ID_APP,"three sensor not support horizontal stitch mode\n");
        return AK_FALSE;
    }

    int dual_cis_mode = 0;
    if(rtsp_param.dev_cnt > 1)
        dual_cis_mode = 1;

    /* get param from cmd line */
    if(rtsp_param.stitch_mode != DISABLED_STITCH )
    {
        for(int i = 0;i < rtsp_param.stitch_num; i++)
        {
            ret = start_vi(i, dual_cis_mode);
            if(ret)
            {
                ak_print_error_ex(MODULE_ID_APP, "vi init failed!\n");
                goto erro;
            }
        }        
    }
    else
    {
        //ret = start_vi(rtsp_param.single_dev);
        //单目暂时只支持1个设备且是设备0
        ret = start_vi(0, dual_cis_mode);
        if(ret)
        {
            ak_print_error_ex(MODULE_ID_APP, "vi init failed!\n");
            goto erro;
        }
    }
    printf("fun=%s line=%d debug here.\n",__FUNCTION__,__LINE__);
     /* create stitch chn */
    if(rtsp_param.stitch_mode != DISABLED_STITCH)
    {
        
        printf("fun=%s line=%d debug here.\n",__FUNCTION__,__LINE__);
        ret = start_stitch_chn(0, rtsp_param.stitch_mode);
        if(ret != AK_SUCCESS)
            goto erro;
        ret = start_stitch_chn(1, rtsp_param.stitch_mode);
    }

    /* create vi */
    if(rtsp_param.stitch_mode != DISABLED_STITCH )
    {
        for(int i = 0;i < rtsp_param.stitch_num; i++)
        {
            enable_vi(i);
        }

        ak_vi_enable_chn(VIDEO_CHN24);
        ak_vi_enable_chn(VIDEO_CHN25);
    }
    else
    {
        //enable_vi(rtsp_param.single_dev);
        //单目暂时只支持1个设备且是设备0

        printf("fun=%s line=%d debug here. rtsp_param.single_dev=%d\n",__FUNCTION__,__LINE__,rtsp_param.single_dev);
        enable_vi(0);
    }

    /* create venc */
    printf("fun=%s line=%d debug here.\n",__FUNCTION__,__LINE__);

    if (rtsp_param.stitch_mode != DISABLED_STITCH)
    {
        /* stitch encode param from VIDEO_DEV0 */
    
        printf("fun=%s line=%d debug here.\n",__FUNCTION__,__LINE__);
        ret = start_venc(VIDEO_DEV0,0);
        ret = start_venc(VIDEO_DEV0,1);
    }
    else
    {
        /* VIDEO_DEV start venc*/
        //start_venc(rtsp_param.single_dev,0);
        //单目暂时只支持1个设备且是设备0

        ret = start_venc(0,0);
        if (ret)
        {
            goto erro;
        } 
        ret = start_venc(0,1);
        if (ret)
        {
            goto erro;
        }
    }

    signal (SIGPIPE, SIG_IGN);
    init_ai(&rtsp_param.audio);
    init_aenc(&rtsp_param.audio);

    ak_pthread_t encode_tid;
    ak_pthread_t get_tid;

    flag = 1;
    int chn = 2;
    ak_thread_create(&(get_tid), audio_get_stream, (void*)chn, ANYKA_THREAD_MIN_STACK_SIZE, 19);
    ak_thread_create(&(encode_tid), audio_get_frame_encode, NULL, ANYKA_THREAD_MIN_STACK_SIZE, 19);
    rtsp_server_init();


    /* WAITER for the thread exit */
    if(rtsp_param.venc_stream_th[0][0] != -1)
        ak_thread_join(rtsp_param.venc_stream_th[0][0]);
    
    if(rtsp_param.venc_stream_th[0][1] != -1)
        ak_thread_join(rtsp_param.venc_stream_th[0][1]);

    
    ak_thread_join(get_tid);
    ak_thread_join(encode_tid);

erro:

    for(int i = 0;i < rtsp_param.dev_cnt; i++)
    {
        if(rtsp_param.venc_info[i][0].venc_handle != -1)
        {
            ak_venc_close(rtsp_param.venc_info[i][0].venc_handle);
            rtsp_param.venc_info[i][0].venc_handle = -1;
        }
        if(rtsp_param.venc_info[i][1].venc_handle != -1)
        {
            ak_venc_close(rtsp_param.venc_info[i][1].venc_handle);
            rtsp_param.venc_info[i][1].venc_handle = -1;
        }
    }

    if(rtsp_param.stitch_mode != DISABLED_STITCH)
    {
        /* disable stitch channel */
        stop_stitch_chn(VIDEO_CHN24);
        stop_stitch_chn(VIDEO_CHN25);
    }
    else
    {
        disable_vi(0);
    }

    ak_sdk_exit();

    return ret;
}
/* end of func */

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(multi_CISs_sample, ak_multi_CISs_sample, Video Encode and Stitch Sample Program);
#endif

/* end of file */
