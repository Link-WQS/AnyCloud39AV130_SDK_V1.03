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
#include "ak_common.h"
#include "ak_log.h"
#include "ak_common_video.h"
#include "ak_venc.h"
#include "ak_thread.h"
#include "ak_mem.h"
#include "ak_vi.h"
#include "ak_video.h"

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
#define DEF_FRAME_DEPTH     2
#endif

#define H264   0
#define H265   1
#define MJPEG  2

#define VIDEO_STITCH 0
#define MAX_SENSOR	4

struct venc_pair
{
    int venc_handle;
    char save_file[128];
    int en_type;
    int dev_id;
};

/* resolation define */
struct resolution_t {
    unsigned int width;
    unsigned int height;
    unsigned char str[20];
};

static char *pc_prog_name = NULL;                      //demo名称
static char *type         = "h264";                      //get the type input
static char *save_path    = "/mnt/video_encode";
static int main_res       = 4;
static int sub_res        = 0;
static int frame_num   = 1000;
static int chn_index = 0;
static char br_mode = BR_MODE_VBR;
//static char init_qp = 100;
static int target_kbps = 1024;
static char vi_fps = 20;
static int save_file_flag = 1;

#ifdef AK_RTOS
static char *cfg = "/etc/config/isp_5268_mipi.conf";
#else
static char *cfg = "/etc/config/isp_pr2000_dvp.conf";
#endif

//static ak_mutex_t refresh_flag_lock;
static ak_pthread_t venc_stream_th[MAX_SENSOR];
static int CIS_no = -1;
static int CIS_num = 0;
static VI_CHN_ATTR_EX chn_attr = {0};
static VI_CHN_ATTR_EX chn_attr_sub = {0};
static int encoder_type[MAX_SENSOR] = {0};
static struct venc_pair enc_pair[MAX_SENSOR] = {{-1}, {-1}, {-1}, {-1}};
static int stitch_mode = DISABLED_STITCH;
static int output_mode = 4;
static int CIS_no_flag[MAX_SENSOR] = {0};
static int encode_mode = 0;

struct dual_param{
    int  chn_index;
    int  frame_num;
    int  main_res;
    int  sub_res;
    int  target_kbps;
    char vi_fps;
    char br_mode;
    char save_path[128];
    char cfg[128];
    char type[128];
};

struct dual_param dual_param[MAX_SENSOR];

void load_param(struct dual_param *dual_param)
{
    dual_param->chn_index        = chn_index;
    dual_param->frame_num        = frame_num;
    dual_param->main_res         = main_res;
    dual_param->sub_res          = sub_res;
    dual_param->target_kbps      = target_kbps;
    memcpy(&dual_param->vi_fps,&vi_fps,sizeof(vi_fps));
    memcpy(&dual_param->br_mode,&br_mode,sizeof(br_mode));
    memcpy(dual_param->cfg,cfg,strlen(cfg)+1);
    memcpy(dual_param->save_path,save_path,strlen(save_path)+1);
    memcpy(dual_param->type,type,strlen(type)+1);
}

/* decoder resolution */
static struct resolution_t resolutions[DE_VIDEO_SIZE_MAX] = {
    {640,   360,   "DE_VIDEO_SIZE_360P"},
    {640,   480,   "DE_VIDEO_SIZE_VGA"},
    {1280,  720,   "DE_VIDEO_SIZE_720P"},
    {1280,  960,   "DE_VIDEO_SIZE_960P"},
    {1920,  1080,  "DE_VIDEO_SIZE_1080P"},
    {2304,  1296,  "DE_VIDEO_SIZE_1296P"},
    {2560,  1440,  "DE_VIDEO_SIZE_1440P"}
};

/* this is the message to print */
static char ac_option_hint[  ][ LEN_HINT ] = {                                         //操作提示数组
    "打印帮助信息" ,
    "从vi获取的帧的数量, default 1000",
    "编码输出数据格式, [h264, h265, jpeg], 37e only support jpeg, 300L not support h264，default h264",
    "编码模式 0:cbr , 1:vbr, 2:CONST_QP, 4:AVBR (2,4 only av100 support), default 1:vbr",
    "目标码率, 单位:kbps, [大于 0], default 1024",
    "帧率, 大于 0，小于sensor实际帧率, default 30",
    "主通道分辨率, [0, 6], default 4",
    "次通道分辨率 [0, 3]. note:need smaller than main channel resolution , default 0",
    "编码数据保存成文件的目录, default: /mnt/video_encode",
    "vi 通道选择, [0-main, 1-sub], default 0",
    "isp conf file path",
    "[NUM] CIS对应的编号选择。-v 0:CIS0; -v 1:CIS1; -v 2:CIS2" ,
    "[NUM] save file flag, 0 not save, otherwise save, default 1" ,
    "[NUM] encode_mode, 0:user mode, 1:kernel mode(frame), 2:kernel mode(slice),default 0" ,
    "[NUM] stitch mode, 0 not stitch mode, 1 vertical stitch mode, 2 horizontal stitch mode, default 0" ,
    "[NUM] output_mode, [0: sensor0, 1: sensor1, 2: sensor2, 4: all sensor, default 4" ,
    "",
};

/* opt for print the message */
static struct option option_long[ ] = {
    { "help"              , no_argument       , NULL , 'h' } ,      //"打印帮助信息" ,
    { "frame-num"         , required_argument , NULL , 'n' } ,      //"从vi获取的帧的数量" ,
    { "data-output"       , required_argument , NULL , 'o' } ,      //"编码输出数据格式" ,
    { "br_mode"           , required_argument , NULL , 'b' } ,      //"编码br模式" ,
    { "target_kbps"       , required_argument , NULL , 't' } ,      //"目标码率" ,
    { "frame-rate"        , required_argument , NULL , 'r' } ,      //"帧率" ,
    { "main_res  "        , required_argument , NULL , 'm' } ,      //"主通道分辨率" ,
    { "sub_res  "         , required_argument , NULL , 's' } ,      //"次通道分辨率" ,
    { "save-path"         , required_argument , NULL , 'p' } ,      //"编码数据保存成文件的目录" ,
    { "channel"           , required_argument , NULL , 'a' } ,      //"vi 输出通道选择" ,
    { "isp conf path"     , required_argument , NULL , 'c' } ,      //"isp 配置文件或yuv数据源文件目录"
    { "CIS_no"            , required_argument , NULL , 'v' } ,      // CIS对应的编号选择
    { "save_file_flag"    , required_argument , NULL , 'f' } ,      //"保存标志",
    { "encode_mode"       , required_argument , NULL , 'e' } ,      //"编码模式",
    { "stitch_mode"       , required_argument , NULL , 'j' } ,      //"拼接模式",
    { "output_mode"       , required_argument , NULL , 'd' } ,      //"选择输出哪路码流"
    { 0                   , 0                 , 0    , 0   } ,
};

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
    printf("eg.: %s -v 0 -n 1000 -m 2 -s 1 -a 0 -o h264 -b 1 -t 1024 -r 30  -p /mnt/ -c /etc/config/isp_*.conf \
    -v 1 -n 1000 -m 2 -s 1 -a 0 -o h264 -b 1 -t 1024 -r 30 -p /mnt/ -c /etc/config/isp_*.conf\n", name);
    printf("encode data type -o:          h264,  h265,  or  jpeg\n");
    printf("resolution -m -s:     value 0 ~ 6\n");
    printf("                      0 - 640*360\n");
    printf("                      1 - 640*480\n");
    printf("                      2 - 1280*720\n");
    printf("                      3 - 1280*960\n");
    printf("                      4 - 1920*1080\n");
    printf("                      5 - 2304*1296\n");
    printf("                      6 - 2560*1440\n");
    printf("save path -p:       \n");
    printf("output channel -a:      0-main 1-sub \n");
    printf("isp config file -c:       \n");
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
        case 'h' :                                    //help
            help_hint();
            c_flag = AK_FALSE;
            goto parse_option_end;
        case 'n' :                                    //frame num
            frame_num = atoi( optarg );
            break;
        case 'o' :                                    //data file format
            type = optarg;
            break;
        case 'm' :                                    //main res
            main_res= atoi( optarg );
            break;
        case 's' :                                    //sub res
            sub_res = atoi( optarg );
            break;
        case 'p' :                                     //save path
            save_path = optarg;
            break;
        case 'c' :                                     //cfg file path
            cfg =  optarg;
            break;
        case 'a' :                                     //chn index
            chn_index =  atoi( optarg );
            break;
        case 'b' :                                     //br mode
            br_mode =  atoi( optarg );
            break;
        case 't' :                                     //target kbps
            target_kbps =  atoi( optarg );
            break;
        case 'r' :                                     //frame rate
            vi_fps =  atoi( optarg );
            break;
        case 'v' :                                     //CIS no
            CIS_no = atoi(optarg);
            CIS_num++;
            break;
        case 'f' :                                     //save file flag
            save_file_flag = atoi(optarg);
            break;
        case 'e' :                                     //encode mode
            encode_mode = atoi(optarg);                
            break;
        case 'j' :                                     //stitch mode
            stitch_mode = atoi(optarg);
            break;
        case 'd' :                                     //output mode
            output_mode = atoi(optarg);
            break;
        default :
            help_hint();
            c_flag = AK_FALSE;
            goto parse_option_end;
        }


        if(CIS_no == VIDEO_DEV0)
        {
            CIS_no_flag[VIDEO_DEV0] = AK_TRUE;
        }

        if(CIS_no == VIDEO_DEV1)
        {
            CIS_no_flag[VIDEO_DEV1] = AK_TRUE;
        }

        if(CIS_no == VIDEO_DEV2)
        {
            CIS_no_flag[VIDEO_DEV2] = AK_TRUE;
        }


        if(CIS_no_flag[VIDEO_DEV0] == AK_FALSE)
        {
            ak_print_error_ex(MODULE_ID_APP, "please input the -v 0 at first\n");
            help_hint();
            c_flag = AK_FALSE;
            goto parse_option_end;
        }

         //load sensor0 param
        if(CIS_no == VIDEO_DEV0 || stitch_mode != DISABLED_STITCH)
        {
            load_param(&dual_param[VIDEO_DEV0]);
        }
        //load sensor1 param
        if(CIS_no == VIDEO_DEV1 || stitch_mode != DISABLED_STITCH)
        {

            load_param(&dual_param[VIDEO_DEV1]);
        }
        //load sensor2 param
        if(CIS_no == VIDEO_DEV2 || stitch_mode != DISABLED_STITCH)
        {
            load_param(&dual_param[VIDEO_DEV2]);
        }

    }
parse_option_end:
    return c_flag;
}
/* end of func */

static int start_vi(int dev_id)
{
    /*
     * step 0: global value initialize
     */
    int ret = -1;                                //return value
    unsigned int width = resolutions[dual_param[dev_id].main_res].width;
    unsigned int height = resolutions[dual_param[dev_id].main_res].height;
    unsigned int subwidth = resolutions[dual_param[dev_id].sub_res].width;;
    unsigned int subheight = resolutions[dual_param[dev_id].sub_res].height;
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
    ret = ak_vi_load_sensor_cfg(dev_id, dual_param[dev_id].cfg);
    if (AK_SUCCESS != ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device %d load isp cfg [%s] failed!\n", dev_id, cfg);
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
    dev_attr.interf_mode = VI_INTF_DUAL_MIPI_2;
    dev_attr.crop.left = 0;
    dev_attr.crop.top = 0;
    dev_attr.crop.width = width;
    dev_attr.crop.height = height;
    dev_attr.max_width = width;
    dev_attr.max_height = height;
    dev_attr.sub_max_width = subwidth;
    dev_attr.sub_max_height = subheight;

    dev_attr.frame_rate = -1;

    if(dev_id == VIDEO_DEV0 &&
    ( stitch_mode != DISABLED_STITCH || output_mode == 0 || output_mode == 4))
    {
        dev_attr.frame_rate = 0;
    }

    if(dev_id == VIDEO_DEV1 &&
    ( stitch_mode != DISABLED_STITCH || output_mode == 1 || output_mode == 4))
    {
        dev_attr.frame_rate = 0;
    }
    if(dev_id == VIDEO_DEV2 &&
    ( stitch_mode != DISABLED_STITCH || output_mode == 2 || output_mode == 4))
    {
        dev_attr.frame_rate = 0;
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
    chn_attr.frame_rate = dual_param[dev_id].vi_fps;
    chn_attr.mode = FRAME_MODE;

#if defined(__CHIP_AK3918AV100_SERIES)
    if(stitch_mode == VERTICAL_STITCH && encode_mode == 2)
    {
        chn_attr.mode = SLICE_MODE;
        chn_attr.frame_depth = 3;
    }
#else
    if(encode_mode == 2)
    {
        chn_attr.mode = SLICE_MODE;
        chn_attr.frame_depth = 3;
    }
#endif

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
    chn_attr_sub.frame_rate = dual_param[dev_id].vi_fps;
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
        if(CIS_num == 3)
            ak_vi_disable_chn(VIDEO_CHN4);
    }
    else
    {
        ak_vi_disable_chn(VIDEO_CHN1);
        ak_vi_disable_chn(VIDEO_CHN3);
        if(CIS_num == 3)
            ak_vi_disable_chn(VIDEO_CHN5);
    }

    ak_vi_destroy_stitch_chn(chn_id);

    ak_vi_disable_dev(VIDEO_DEV0);
    ak_vi_disable_dev(VIDEO_DEV1);
    if(CIS_num == 3)
        ak_vi_disable_dev(VIDEO_DEV2);

    ak_vi_close(VIDEO_DEV0);
    ak_vi_close(VIDEO_DEV1);
    if(CIS_num == 3)
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
    int ret = ak_vi_enable_dev(dev_id);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP,
            "vi device %d enable device  failed!\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    }

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
#endif

    return ret;
}

static int start_stitch_chn(int chn_id, int stitch_mode)
{
    int ret=AK_FAILED;
    VI_STITCH_ATTR stitch_attr = {0};
    stitch_attr.mode = stitch_mode;
    stitch_attr.stitch_chn_num = CIS_num;

    if(chn_id %2 == 0)
    {
        stitch_attr.stitch_bind_chn[0]=VIDEO_CHN0;
        stitch_attr.stitch_bind_chn[1]=VIDEO_CHN2;
        if(CIS_num == 3)
            stitch_attr.stitch_bind_chn[2]=VIDEO_CHN4;

        //stitch main channel
        ret = ak_vi_create_stitch_chn(VIDEO_CHN24, NULL, &stitch_attr);
    }
    else
    {
        stitch_attr.stitch_bind_chn[0]=VIDEO_CHN1;
        stitch_attr.stitch_bind_chn[1]=VIDEO_CHN3;
        if(CIS_num == 3)
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
    ak_thread_set_name("venc_th");
    struct venc_pair *venc_th = (struct venc_pair *)arg; //venc handle

    int dev_id = venc_th->dev_id;

    FILE *save_fp = NULL;
    VI_CHN chn_id = dev_id*2 + dual_param[dev_id].chn_index;

    ak_print_normal(MODULE_ID_APP, "capture start\n");

    RECTANGLE_S res;
    res.width = resolutions[dual_param[dev_id].main_res].width;
    res.height = resolutions[dual_param[dev_id].main_res].height;
    if(dual_param[dev_id].chn_index)
    {
        res.width = resolutions[dual_param[dev_id].sub_res].width;
        res.height = resolutions[dual_param[dev_id].sub_res].height;
    }

    if(stitch_mode != DISABLED_STITCH)
    {
        if(stitch_mode == VERTICAL_STITCH){
            res.height = res.height * CIS_num;   //VERTICAL_STITCH
        }

        if(stitch_mode == HORIZONTAL_STITCH){
            res.width = res.width * CIS_num;   //HORIZONTAL_STITCH
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

    if (save_file_flag)
    {
        /* init the save file name and open it */
        memset(venc_th->save_file, 0x00, sizeof(venc_th->save_file));
        if (H264_ENC_TYPE == venc_th->en_type)
            sprintf(venc_th->save_file, "%s/h264_br%d_%dkbps_chn%d_%dx%d.str",
                dual_param[dev_id].save_path, dual_param[dev_id].br_mode,dual_param[dev_id].target_kbps,   \
                                                                            chn_id,res.width, res.height);
        else
            sprintf(venc_th->save_file, "%s/h265_br%d_%dkbps_chn%d_%dx%d.str",
                dual_param[dev_id].save_path, dual_param[dev_id].br_mode,dual_param[dev_id].target_kbps,    \
                                                                            chn_id, res.width, res.height);

        /* save file open */
        if (MJPEG_ENC_TYPE != venc_th->en_type)
        {
            save_fp = fopen(venc_th->save_file, "w+");
            if(save_fp == NULL)
            {
                ak_print_error_ex(MODULE_ID_APP,
                    "fopen failed, errno = %d.\n", errno);
                return NULL;
            }
        }
    }

    APP_CHN_S Schn;
    APP_CHN_S Dchn;
    Schn.mid = MODULE_ID_VI;
    Schn.oid = chn_id;
    Dchn.mid = MODULE_ID_VENC;
    Dchn.oid = venc_th->venc_handle;
    APP_BIND_PARAM param;
    param.frame_rate = dual_param[dev_id].vi_fps;
    param.frame_depth = 200;
    param.mode = USER_ENCODE_MODE;

    if(encode_mode != 0)
        param.mode = KERNEL_ENCODE_MODE;

    ak_print_normal_ex(MODULE_ID_APP, "Encoder [%d] bind chn!\n",Dchn.oid);
    int ret = ak_app_video_bind_chn(&Schn, &Dchn, &param);
    if(ret != AK_SUCCESS)
    {
        ak_print_error_ex(MODULE_ID_APP,
            "ak_app_video_bind_chn failed [%d]\n",ret);
        goto error_exit;
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
        goto error_exit;
    }

    int frame_c = dual_param[dev_id].frame_num;
    while (frame_c > 0)
    {
        memset(stream, 0, sizeof(struct video_stream));

        if (save_file_flag)
        {
            /* if save as jpeg data */
            if (MJPEG_ENC_TYPE == venc_th->en_type)
            {
                memset(venc_th->save_file, 0x00, sizeof(venc_th->save_file));
                sprintf(venc_th->save_file, "%s/chn%d_%dx%d_num_%d.jpeg",
                    dual_param[dev_id].save_path, chn_id, res.width, res.height,    \
                                            dual_param[dev_id].frame_num - frame_c);
                save_fp = fopen(venc_th->save_file, "w+");
            }
        }

        ret = ak_app_video_venc_get_stream(&Dchn, stream);
        if(ret == AK_SUCCESS)
        {
            //ak_print_normal_ex(MODULE_ID_APP, "ak_app_video_venc_get_stream success!\n");
            if(save_fp && save_file_flag)
                fwrite(stream->data, stream->len, 1, save_fp);
            ak_venc_release_stream(venc_th->venc_handle, stream);
            frame_c--;

            if (0 == (dual_param[dev_id].frame_num - frame_c) % 50)
                ak_print_normal_ex(MODULE_ID_VENC,
                    "encode num:%d\n", dual_param[dev_id].frame_num - frame_c);
        }
        else
        {
            //ak_print_error_ex(MODULE_ID_APP, "ak_app_video_venc_get_stream fail [%x]!\n",ret);
            ak_sleep_ms(10);
        }

        if (save_file_flag)
        {
            /* save as a jpeg picture */
            if (MJPEG_ENC_TYPE == venc_th->en_type)
            {
                if (NULL != save_fp)
                {
                    fclose(save_fp);
                    save_fp = NULL;
                }
            }
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

error_exit:
    if (save_file_flag)
    {
        /* finished */
        if (MJPEG_ENC_TYPE != venc_th->en_type)
        {
            if(save_fp)
                fclose(save_fp);
        }
    }

    return NULL;
}
/* end of func */

static int start_venc(int dev_id)
{
    int ret = -1;
    int width = resolutions[dual_param[dev_id].main_res].width;
    int height = resolutions[dual_param[dev_id].main_res].height;
    int subwidth = resolutions[dual_param[dev_id].sub_res].width;;
    int subheight = resolutions[dual_param[dev_id].sub_res].height;
    int handle_id[MAX_SENSOR] = {-1, -1, -1, -1};


    /* open venc */
    struct venc_param ve_param = {0};

    struct venc_rc_param rc_param = {0};
    rc_param.enable_MMA = 1;

    if (dual_param[dev_id].chn_index == 0)
    {
        ve_param.width  = width;            //resolution width
        ve_param.height = height;           //resolution height
    }
    else
    {
        ve_param.width  = subwidth;            //resolution width
        ve_param.height = subheight;           //resolution height
    }

    if(stitch_mode == VERTICAL_STITCH){
        ve_param.height += ve_param.height;      //stitch height
        if(CIS_num == 3 && dual_param[dev_id].chn_index == 0) //dev2 main chn
            ve_param.height += height;
        else if(CIS_num == 3 && dual_param[dev_id].chn_index == 1) //dev2 sub chn
            ve_param.height += subheight;
    }

    if(stitch_mode == HORIZONTAL_STITCH){
        ve_param.width += ve_param.width;      //stitch width
        if(CIS_num == 3 && dual_param[dev_id].chn_index == 0) //dev2 main chn
            ve_param.width += width; 
        else if(CIS_num == 3 && dual_param[dev_id].chn_index == 1) //dev2 sub chn
             ve_param.width += subheight; 
    }

    ve_param.fps    = dual_param[dev_id].vi_fps;               //fps set
    ve_param.goplen = 2*dual_param[dev_id].vi_fps;               //gop set
    ve_param.target_kbps = dual_param[dev_id].target_kbps;         //k bps
    ve_param.max_kbps    = 2048;        //max kbps
    ve_param.br_mode     = dual_param[dev_id].br_mode; //br mode
    ve_param.minqp       = 25;          //qp set
    ve_param.maxqp       = 50;          //qp max value
    ve_param.initqp       = (ve_param.minqp + ve_param.maxqp)/2;    //qp value
    ve_param.jpeg_qlevel = JPEG_QLEVEL_DEFAULT;     //jpeg qlevel
    ve_param.chroma_mode = CHROMA_4_2_0;            //chroma mode
    ve_param.max_picture_size = 0;                  //0 means default
    ve_param.enc_level        = 30;                 //enc level
    ve_param.smart_mode       = 0;                  //smart mode set
    ve_param.smart_goplen     = 100;                //smart mode value
    ve_param.smart_quality    = 50;                 //quality
    ve_param.smart_static_value = 0;                //value
    ve_param.enc_out_type = encoder_type[dev_id];           //enc type

    /* decode type is h264*/
    if(encoder_type[dev_id] == H264_ENC_TYPE)
    {
        /* profile type */
        ve_param.profile     = PROFILE_MAIN;

    }
    /*decode type is jpeg*/
    else if(encoder_type[dev_id] == MJPEG_ENC_TYPE)
    {
        /* jpeg enc profile */
        ve_param.profile = PROFILE_JPEG;
    }
    /*decode type is h265*/
    else
    {
        /* hevc profile */
        ve_param.profile = PROFILE_HEVC_MAIN;
    }

    ret = ak_venc_open_ex(&ve_param, &rc_param,  &handle_id[dev_id]);

    if (ret || (-1 == handle_id[dev_id]) )
    {
        ak_print_error_ex(MODULE_ID_APP, "dev%d open venc failed\n", dev_id);
        return ret;
    }

    if (ve_param.width > 1920 && ve_param.height > 1080 && ve_param.initqp > 60)
    {
        ak_venc_set_stream_buff(handle_id[dev_id], 1*1024*1024);
    }


    /* recode the file */
    //enc_pair.file = file;
    enc_pair[dev_id].venc_handle = handle_id[dev_id];
    enc_pair[dev_id].en_type     = encoder_type[dev_id];
    enc_pair[dev_id].dev_id = dev_id;

    /* create the venc thread */
    ak_thread_create(&venc_stream_th[dev_id], video_encode_from_vi_th, &enc_pair[dev_id],   \
                                                    ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);

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

    memset(dual_param, 0, sizeof(struct dual_param)*2);

    /* start to parse the opt */
    if( parse_option( argc, argv ) == AK_FALSE )
    {                                               //解释和配置选项
        return AK_FAILED;
    }
    /* param check */
    if(CIS_no < 0 || CIS_no >= MAX_SENSOR)
    {
        ak_print_error_ex(MODULE_ID_APP,
            "CIS_no error! please input the correct CIS_no (0 ~ 1)\n");
        return AK_FALSE;
    }
    /* param check */
    if(CIS_num == 3 && stitch_mode == HORIZONTAL_STITCH)
    {
        ak_print_error_ex(MODULE_ID_APP,"three sensor not support horizontal stitch mode\n");
        return AK_FALSE;
    }
    /* param check */
    if(CIS_no_flag[1] != AK_TRUE)
    {
        ak_print_error_ex(MODULE_ID_APP,
            "CIS_no error! please input -v 0 and -v 1\n");
        help_hint();
        return AK_FAILED;
    }
    /* param check */
    if(output_mode < 0 || output_mode > 4)
    {
         ak_print_error_ex(MODULE_ID_APP,
            "please input the correct output_mode param: 0,1,2\n");
        /* print the hint and notice */
        help_hint();
        return AK_FAILED;
    }

    /* param check */
    for(int dev_id = 0; dev_id < CIS_num; dev_id++)
    {

        if(dual_param[dev_id].frame_num <= 0)
        {
            ak_print_error_ex(MODULE_ID_APP, "frame num must be positive.\n");
            return AK_FAILED;
        }

        /* check the res */
        if (dual_param[dev_id].main_res < 0 || dual_param[dev_id].main_res > DE_VIDEO_SIZE_MAX-1
                || dual_param[dev_id].sub_res < 0 || dual_param[dev_id].sub_res > DE_VIDEO_SIZE_MAX-1
                    || dual_param[dev_id].main_res < dual_param[dev_id].sub_res)
        {
            ak_print_error_ex(MODULE_ID_APP,
                "resolution out of range, check it\n");
            return AK_FAILED;
        }

#if defined(__CHIP_AK3918AV100_SERIES) || defined(__CHIP_AK3918EV300L_SERIES)
        /* check the br mode */
        if (dual_param[dev_id].br_mode < BR_MODE_CBR || dual_param[dev_id].br_mode >= BR_MODE_TYPE_TOTAL    \
                                                     || BR_MODE_LOW_LATENCY == dual_param[dev_id].br_mode)
        {
            ak_print_error_ex(MODULE_ID_VENC,
                "br mode out of range, check it\n");
            return AK_FAILED;
        }
#else
        /* check the br mode */
        if (dual_param[dev_id].br_mode < BR_MODE_CBR || dual_param[dev_id].br_mode > BR_MODE_VBR)
        {
            ak_print_error_ex(MODULE_ID_VENC,
                "br mode out of range, check it\n");
            return AK_FAILED;
        }
#endif

        /* check the chn index value */
        if (dual_param[dev_id].chn_index < 0 || dual_param[dev_id].chn_index > 1)
        {
            ak_print_error_ex(MODULE_ID_APP, "please input the correct param\n");
            /* print the hint and notice */
            help_hint();
            return AK_FAILED;
        }

        /* check the chn fps value */
        if (dual_param[dev_id].vi_fps < 1 || dual_param[dev_id].vi_fps > 30)
        {
            ak_print_error_ex(MODULE_ID_VENC,
                "please input the correct vi_fps (1 ~ 30)\n");
            /* print the hint and notice */
            help_hint();
            return AK_FAILED;
        }

        /* check the chn index value */
        if (dual_param[dev_id].target_kbps < 1 || dual_param[dev_id].target_kbps > 2048)
        {
            ak_print_error_ex(MODULE_ID_VENC,
                "please input the correct target_kbps (1 ~ 2048)\n");
            /* print the hint and notice */
            help_hint();
            return AK_FAILED;
        }

        if (dual_param[dev_id].type == NULL)
        {
            ak_print_error_ex(MODULE_ID_APP, "please input the type\n");
            /* print the hint and notice */
            help_hint();
            return AK_FAILED;
        }

        /* get the type for input */
        if(!strcmp(dual_param[dev_id].type,"h264"))
        {
            /* h264 */
            ak_print_normal(MODULE_ID_APP, "encode type: h264\n");
            encoder_type[dev_id] = H264_ENC_TYPE;
        }
        else if(!strcmp(dual_param[dev_id].type,"h265"))
        {
            /* h265 */
            ak_print_normal(MODULE_ID_APP, "encode type:h265 \n");
            encoder_type[dev_id] = HEVC_ENC_TYPE;
        }
        else if(!strcmp(dual_param[dev_id].type,"jpeg"))
        {
            /* jpeg */
            ak_print_normal(MODULE_ID_APP, "encode type:jpeg\n");
            encoder_type[dev_id] = MJPEG_ENC_TYPE;
        }
        else
        {
            ak_print_normal_ex(MODULE_ID_APP,
                "unsupport video enccode input type [%s] \n", dual_param[dev_id].type);
            return AK_FAILED;
        }

#if defined(__CHIP_AK3918AV130_SERIES)
        if(encoder_type[dev_id] == HEVC_ENC_TYPE)
        {
            ak_print_error_ex(MODULE_ID_APP, "not support encode type:h265\n");
            return AK_FAILED;
        }
#endif

       if ((access(dual_param[dev_id].save_path, W_OK) != 0) && save_file_flag)
       {
            if (mkdir(dual_param[dev_id].save_path, 0755))
            {
                ak_print_error_ex(MODULE_ID_APP,
                    "mkdir: %s failed\n", dual_param[dev_id].save_path);
                return AK_FAILED;
            }
        }
    }
    /* get param from cmd line */
    int ret = -1;

    /* start vi */
    ret = start_vi(VIDEO_DEV0);
    ret = start_vi(VIDEO_DEV1);
    if(CIS_num == 3)
        ret = start_vi(VIDEO_DEV2); /* DEV2 */

    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP, "vi init failed!\n");
        goto erro;
    }

     /* create stitch chn */
    if(stitch_mode != DISABLED_STITCH)
    {
        ret = start_stitch_chn(chn_index, stitch_mode);
        if(ret != AK_SUCCESS)
            goto erro;
    }

    /* enable vi channel */
    ret = enable_vi(VIDEO_DEV0);
    ret = enable_vi(VIDEO_DEV1);
    if(CIS_num == 3)
        ret = enable_vi(VIDEO_DEV2); /* DEV2 */

    if(stitch_mode != DISABLED_STITCH)
    {
        if(chn_index %2 == 0)
        {
            /* enable stitch channel */
            ak_vi_enable_chn(VIDEO_CHN24);
        }
        else
        {
            /* enable stitch channel */
            ak_vi_enable_chn(VIDEO_CHN25);
        }

    }

    if (stitch_mode != DISABLED_STITCH)
    {
        /* stitch encode param from VIDEO_DEV0 */
        ret = start_venc(VIDEO_DEV0);
    }
    else
    {
        /* VIDEO_DEV0 start venc*/
        if(output_mode == 0 || output_mode == 4)
        {
            ret = start_venc(VIDEO_DEV0);
        }

        /* VIDEO_DEV1 start venc*/
        if(output_mode == 1 || output_mode == 4)
        {
            ret = start_venc(VIDEO_DEV1);
        }
        /* VIDEO_DEV2 start venc*/
        if((output_mode == 2 || output_mode == 4) && CIS_num == 3)
        {
            ret = start_venc(VIDEO_DEV2);
        }
    }

    if (ret)
    {
        goto erro;
    }

    if(stitch_mode != DISABLED_STITCH)
    {
        /* WAITER for the thread exit */
        ak_thread_join(venc_stream_th[0]);
    }
    else
    {
        /* WAITER for the thread exit */
         if(output_mode == 0 || output_mode == 4)
            ak_thread_join(venc_stream_th[0]);

        /* WAITER for the thread exit */
         if(output_mode == 1 || output_mode == 4)
            ak_thread_join(venc_stream_th[1]);
        /* WAITER for the thread exit */
         if((output_mode == 2 || output_mode == 4) && CIS_num == 3) /* DEV2 */
            ak_thread_join(venc_stream_th[2]);
    }

erro:
    /* close the video encoder and video decoder */
    if(stitch_mode != DISABLED_STITCH)
    {
        if(enc_pair[0].venc_handle != -1)
        {
            /* close the venc*/
            ak_venc_close(enc_pair[0].venc_handle);
            enc_pair[0].venc_handle = -1;
        }
    }
    else
    {   /* DEV0 */
        if(output_mode == 0 || output_mode == 4)
        {
            if(enc_pair[0].venc_handle != -1)
            {
                /* close the venc*/
                ak_venc_close(enc_pair[0].venc_handle);
                enc_pair[0].venc_handle = -1;
            }
        }
        /* DEV1 */
        if(output_mode == 1 || output_mode == 4)
        {
            if(enc_pair[1].venc_handle != -1)
            {
                /* close the venc*/
                ak_venc_close(enc_pair[1].venc_handle);
                enc_pair[1].venc_handle = -1;
            }
        }
        /* DEV2 */
        if((output_mode == 2 || output_mode == 4) && CIS_num == 3)
        {
            if(enc_pair[2].venc_handle != -1)
            {
                /* close the venc*/
                ak_venc_close(enc_pair[2].venc_handle);
                enc_pair[2].venc_handle = -1;
            }
        }
    }

    if(stitch_mode != DISABLED_STITCH)
    {
        if(chn_index %2 == 0)
        {
            /* disable stitch channel */
            stop_stitch_chn(VIDEO_CHN24);
        }
        else
        {
            /* disable stitch channel */
            stop_stitch_chn(VIDEO_CHN25);
        }
    }
    else
    {
        stop_vi(VIDEO_DEV0);
        stop_vi(VIDEO_DEV1);
        if(CIS_num == 3) /* DEV2 */
            stop_vi(VIDEO_DEV2);
    }


    ak_sdk_exit();

    return ret;
}
/* enf of func */

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(multi_CISs_sample, ak_multi_CISs_sample, Video Encode and Stitch Sample Program);
#endif

/* end of file */

