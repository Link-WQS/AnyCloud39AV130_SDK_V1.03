/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_video_sample.c
* Description: This is a simple example to show how the video module working.
* Notes: Before running please insmod sensor_xxx.ko ak_isp.ko ak_venc_adapter.ko ak_venc_bridge.ko at first.
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
#include "list.h"

#ifdef AK_RTOS
#include <kernel.h>
#define THREAD_PRIO 90
#define __STATIC__  static
#else
#define THREAD_PRIO -1
#define __STATIC__
#endif

/* video resolution num */
#define DE_VIDEO_SIZE_MAX    8

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

typedef struct stream_node{
    struct video_stream stream;
    struct list_head list;
}stream_list;

static char *pc_prog_name = NULL;                      //demo名称
static char *type         = "h264";                      //get the type input
static char *save_path    = "/mnt/video_encode";
static int main_res       = 4;
static int sub_res        = 0;
static int frame_num   = 1000;
static char chn_index = 0;
static char br_mode = BR_MODE_VBR;
//static char init_qp = 100;
static int target_kbps = 1024;
static char vi_fps = 20;
static int save_file_flag = 1;
int buf_size_3dnr = 0;

#ifdef AK_RTOS
typedef struct {
    unsigned long total;
    unsigned long used;
    unsigned long free;
} mem_info_t;
extern int32_t umm_info_get(mem_info_t *info);
#define SAFE_REMAINING_MEMEOY   3*1024*1024

static char *cfg = "/etc/config/isp_5268_mipi.conf";
#else
static char *cfg = "/etc/config/isp_pr2000_dvp.conf";
#endif
ak_mutex_t refresh_flag_lock;
static ak_pthread_t venc_stream_th[2];
static ak_pthread_t save_th[2];
static int dev_cnt = 1;
static VI_CHN_ATTR_EX chn_attr = {0};
static VI_CHN_ATTR_EX chn_attr_sub = {0};
static int encoder_type = 0;
static struct venc_pair enc_pair[2] = {{-1}, {-1}};
static int encode_mode = 0;
struct list_head *p_head[2];
ak_mutex_t stream_lock[2];

/* decoder resolution */
static struct resolution_t resolutions[DE_VIDEO_SIZE_MAX] = {
    {640,   360,   "DE_VIDEO_SIZE_360P"},
    {640,   480,   "DE_VIDEO_SIZE_VGA"},
    {1280,  720,   "DE_VIDEO_SIZE_720P"},
    {960,  1080,   "DE_VIDEO_SIZE_960"},
    {1920,  1080,  "DE_VIDEO_SIZE_1080P"},
    {2304,  1296,  "DE_VIDEO_SIZE_1296P"},
    {2560,  1440,  "DE_VIDEO_SIZE_1440P"},
    {2560,  1920,  "DE_VIDEO_SIZE_1920P"}
};

/* this is the message to print */
static char ac_option_hint[  ][ LEN_HINT ] = {             //操作提示数组
    "打印帮助信息" ,
    "从vi获取的帧的数量",
    "编码输出数据格式, [h264, h265, jpeg], 37e only support jpeg, 300L not support h264 ",
    //"编码质量(qp) [0, 100]",
    "编码模式 0:cbr , 1:vbr, 2:CONST_QP, 4:AVBR (2,4 only av100 and av130 support)",
    "目标码率, 单位:kbps, [大于 0]",
    "帧率, 大于 0，小于sensor实际帧率",
    "主通道分辨率, [0, 7], 37e[0, 1], 300L[0, 4]",
    "次通道分辨率 [0, 3]. note:need smaller than main channel resolution",
    "编码数据保存成文件的目录",
    "vi 通道选择, [0-main, 1-sub]",
    "isp conf file path, (dev_cnt:1  isp_xxx.conf, dev_cnt:2  isp_xxx_dual.conf)",
    "[NUM] device count [1 - 2], default 1" ,
    "[NUM] encode_mode, 0:user mode, 1:kernel mode(frame), 2:kernel mode(slice),default 0" ,
    "[NUM] save file flag, 0 not save, otherwise save, default 1" ,
    "[NUM] 3dnr buf size, KB" ,
    "",
};

/* opt for print the message */
static struct option option_long[ ] = {
    { "help"              , no_argument       , NULL , 'h' } ,      //"打印帮助信息" ,
    { "frame-num"         , required_argument , NULL , 'n' } ,      //"从vi获取的帧的数量" ,
    { "data-output"       , required_argument , NULL , 'o' } ,      //"编码输出数据格式" ,
    //{ "qp"                , required_argument , NULL , 'q' } ,      //"编码质量" ,
    { "br_mode"           , required_argument , NULL , 'b' } ,      //"编码br模式" ,
    { "target_kbps"       , required_argument , NULL , 't' } ,      //"目标码率" ,
    { "frame-rate"        , required_argument , NULL , 'r' } ,      //"帧率" ,
    { "main_res  "        , required_argument , NULL , 'm' } ,      //"主通道分辨率" ,
    { "sub_res  "         , required_argument , NULL , 's' } ,      //"次通道分辨率" ,
    { "save-path"         , required_argument , NULL , 'p' } ,      //"编码数据保存成文件的目录" ,
    { "channel"           , required_argument , NULL , 'a' } ,      //"vi 输出通道选择" ,
    { "isp_conf_path"     , required_argument , NULL , 'c' } ,      //"isp 配置文件或yuv数据源文件目录"
    { "dev_cnt"           , required_argument , NULL , 'v' } ,
    { "encode_mode"           , required_argument , NULL , 'e' } ,
    { "save_file_flag"  , required_argument , NULL , 'f' } ,        //"是否保存文件",
    { "buf_size_3dnr"     , required_argument , NULL , 'd' } ,      //3dnr buf size
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
    printf("eg.: %s -n 1000 -o h264 -b 1 -t 1024 -r 20 -m 4 -s 1 -p /mnt/video -a 0 -c /etc/config/isp_xx.conf -e 0\n", 
                                                                                                                name);
    printf("encode data type -o:          h264,  h265,  or  jpeg\n");
    printf("resolution -m -s:     value 0 ~ 7\n");
    printf("                      0 - 640*360\n");
    printf("                      1 - 640*480\n");
    printf("                      2 - 1280*720\n");
    printf("                      3 - 960*1080\n");
    printf("                      4 - 1920*1080\n");
    printf("                      5 - 2304*1296\n");
    printf("                      6 - 2560*1440\n");
    printf("                      7 - 2560*1920\n");
    printf("save path -p:       \n");
    printf("output channel -a:      0-main 1-sub \n");
    printf("isp config file -c:       \n");
    printf("encode mode  -e:       0-user mode, 1-kernel mode\n");
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
        case 'n' :                                   //data file path
            frame_num = atoi( optarg );
            break;
        case 'o' :                                   //data_file format
            type = optarg;
            break;
        case 'm' :                                   //decode num
            main_res= atoi( optarg );
            break;
        case 's' :                                   //decode num
            sub_res = atoi( optarg );
            break;
        case 'p' :                                   //decode num
            save_path = optarg;
            break;
        case 'c' :                                   //decode num
            cfg =  optarg;
            break;
        case 'a' :                                   //decode num
            chn_index =  atoi( optarg );
            break;
        //case 'q' :                                 //encode mode
            //init_qp =  atoi( optarg );
            //break;
        case 'b' :                                  //encode mode
            br_mode =  atoi( optarg );
            break;
        case 't' :                                  //target kbps
            target_kbps =  atoi( optarg );
            break;
        case 'r' :                                  //frame rate
            vi_fps =  atoi( optarg );
            break;
        case 'v' :
            dev_cnt = atoi(optarg);
            break;
        case 'e' :
            encode_mode = atoi(optarg);
            break;
        case 'f' :
            save_file_flag = atoi(optarg);
            break;
        case 'd' :
            buf_size_3dnr = atoi(optarg);            //3dnr_buf_size 
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

static int start_vi(int dev_id,int dual_cis_mode)
{
    /*
     * step 0: global value initialize
     */
    int ret = -1;                                //return value
    unsigned int width = resolutions[main_res].width;
    unsigned int height = resolutions[main_res].height;
    unsigned int subwidth = resolutions[sub_res].width;;
    unsigned int subheight = resolutions[sub_res].height;
    int chn_main_id = VIDEO_CHN0;
    int chn_sub_id = VIDEO_CHN1;

    if (dev_id)
    {
        chn_main_id = VIDEO_CHN2;
        chn_sub_id = VIDEO_CHN3;
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
    ret = ak_vi_load_sensor_cfg(dev_id, cfg);
    if (AK_SUCCESS != ret) {
        ak_print_error_ex(MODULE_ID_APP,
            "vi device %d load isp cfg [%s] failed!\n", dev_id, cfg);
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
    if(dual_cis_mode != 0)
        dev_attr.interf_mode = VI_INTF_DUAL_MIPI_2;
    dev_attr.crop.left = 0;
    dev_attr.crop.top = 0;
    dev_attr.crop.width = width;
    dev_attr.crop.height = height;
    dev_attr.max_width = width;
    dev_attr.max_height = height;
    dev_attr.sub_max_width = subwidth;
    dev_attr.sub_max_height = subheight;

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

    /* set 3dnr buf size */
    if(buf_size_3dnr){
        ret = ak_vi_set_dev_3DNR_memsize(dev_id, buf_size_3dnr); //KB
        if (ret) {
            ak_print_error_ex(MODULE_ID_APP,    "set 3dnr buf size %d failed!\n", dev_id, buf_size_3dnr);
        }
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
    chn_attr.frame_rate = vi_fps;
    if(encode_mode == 0 || encode_mode == 1)
        chn_attr.mode = FRAME_MODE;
    else
    {
        chn_attr.mode = SLICE_MODE;
        chn_attr.frame_depth = 4;
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
    chn_attr_sub.frame_rate = vi_fps;
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

    /*
     * step 8: enable vi device
     */
    ret = ak_vi_enable_dev(dev_id);
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
    return 0;
}
/* end of func */

static int stop_vi(int dev_id)
{
    int ret = -1;
    int chn_main_id = VIDEO_CHN0;
    int chn_sub_id = VIDEO_CHN1;

    if (dev_id)
    {
        chn_main_id = VIDEO_CHN2;
        chn_sub_id = VIDEO_CHN3;
    }

    ak_vi_disable_chn(chn_main_id);

#ifndef __CHIP_AK37E_SERIES
    ak_vi_disable_chn(chn_sub_id);
#endif

    ak_vi_disable_dev(dev_id);
    ret = ak_vi_close(dev_id);

    return ret;
}


/* stream data write to file */
static int stream_buf_out(struct list_head *frame_list, FILE *file, int dev_id)
{
    stream_list *pos = NULL;

    if (!list_empty(frame_list)) {
        /* get one frame from the frame list*/
        pos = list_first_entry(frame_list, stream_list, list);
        {
            /*delete the pos from the frame list*/
            ak_thread_mutex_lock(&stream_lock[dev_id]);
            list_del(&pos->list);
            ak_thread_mutex_unlock(&stream_lock[dev_id]);

            if(file)
                fwrite((char*)pos->stream.data, 1, pos->stream.len, file);
            ak_mem_free(pos->stream.data);
            ak_mem_free(pos);
        }
    }
    return AK_SUCCESS;
}


static void *save_file_th(void *arg)
{
    ak_thread_set_name("save_file_th");

    struct venc_pair *venc_th = (struct venc_pair *)arg;    // venc handle
    VI_CHN chn_id = (chn_index == 0) ?  VIDEO_CHN0 : VIDEO_CHN1;
    int dev_id = venc_th->dev_id;

    if (venc_th->dev_id)
        chn_id += 2;

    FILE *save_fp = NULL;
    int ret = -1;
    RECTANGLE_S res;

    res.width = resolutions[main_res].width;
    res.height = resolutions[main_res].height;
    if(chn_index)
    {
        res.width = resolutions[sub_res].width;
        res.height = resolutions[sub_res].height;
    }

    if (save_file_flag && MJPEG_ENC_TYPE != venc_th->en_type)
    {
        /* init the save file name and open it */
        memset(venc_th->save_file, 0x00, sizeof(venc_th->save_file));
        if (H264_ENC_TYPE == venc_th->en_type)
            sprintf(venc_th->save_file, "%s/h264_br%d_%dkbps_chn%d_%dx%d.str",
                save_path, br_mode,target_kbps,chn_id,res.width, res.height);
        else
            sprintf(venc_th->save_file, "%s/h265_br%d_%dkbps_chn%d_%dx%d.str",
                save_path, br_mode,target_kbps,chn_id, res.width, res.height);

        /* save file open */
        save_fp = fopen(venc_th->save_file, "w+");
        if(save_fp == NULL)
        {
            ak_print_error_ex(MODULE_ID_APP, "fopen failed, errno = %d.\n", errno);
            return NULL;
        }
    }

    int count = frame_num;
    while(count && save_file_flag)
    {
        ret = list_empty(p_head[dev_id]);       // return 1 when the list is empty
        if(!ret)                                // list is not empty
        {
            if(MJPEG_ENC_TYPE == venc_th->en_type)
            {
                sprintf(venc_th->save_file, "%s/chn%d_%dx%d_num_%d.jpeg",
                    save_path, chn_id, res.width, res.height, frame_num - count);
                save_fp = fopen(venc_th->save_file, "w+");
                if(save_fp == NULL)
                {
                    ak_print_error_ex(MODULE_ID_APP, "fopen failed, errno = %d.\n", errno);
                    return NULL;
                }
            }
            stream_buf_out(p_head[dev_id], save_fp, dev_id);
            if(save_fp && MJPEG_ENC_TYPE == venc_th->en_type)
            {
                fclose(save_fp);
            }
            count--;
        }
        else
        {
            ak_sleep_ms(10);
        }
    }

    ak_thread_mutex_destroy(&stream_lock[dev_id]);
    if(p_head[dev_id] != NULL)
        ak_mem_free(p_head[dev_id]);

    if(save_fp && MJPEG_ENC_TYPE != venc_th->en_type)
    {
        fclose(save_fp);
    }

    ak_thread_exit();

    return NULL;
}

/* get stream data into list*/
static int stream_buf_in(struct list_head *frame_list, struct video_stream *stream, int dev_id)
{
    stream_list *vs = ak_mem_alloc(MODULE_ID_APP, sizeof(stream_list));
    if(NULL == vs)
    {
        ak_print_error_ex(MODULE_ID_VENC, "Alloc vs failed!\n");
        return AK_FAILED;
    }
    memset(vs, 0, sizeof(stream_list));

    char *pdata = ak_mem_alloc(MODULE_ID_APP, stream->len);
    if(NULL == pdata )
    {
        ak_print_error_ex(MODULE_ID_VENC, "Alloc stream buffer failed!\n");
        free(vs);
        return AK_FAILED;
    }
    memcpy((unsigned char*)pdata, stream->data, stream->len);
    vs->stream.data = (unsigned char*)pdata;
    vs->stream.len = stream->len;

    ak_thread_mutex_lock(&stream_lock[dev_id]);
    list_add_tail(&vs->list, frame_list);
    ak_thread_mutex_unlock(&stream_lock[dev_id]);

    return AK_SUCCESS;
}


/* get the encode data from vi module */
static void *video_encode_from_vi_th(void *arg)
{
    ak_thread_set_name("venc_th");
    struct venc_pair *venc_th = (struct venc_pair *)arg; //venc handle

    VI_CHN chn_id = (chn_index == 0) ?  VIDEO_CHN0 : VIDEO_CHN1;

    if (venc_th->dev_id)
        chn_id += 2;

    int dev_id = venc_th->dev_id;
    ak_thread_mutex_init(&stream_lock[dev_id], NULL);

    ak_print_normal(MODULE_ID_APP, "capture start\n");

    APP_CHN_S Schn;
    APP_CHN_S Dchn;
    Schn.mid = MODULE_ID_VI;
    Schn.oid = chn_id;
    Dchn.mid = MODULE_ID_VENC;
    Dchn.oid = venc_th->venc_handle;
    APP_BIND_PARAM param;
    param.frame_rate = vi_fps;
    param.frame_depth = 200;
    if(encode_mode == 0)
        param.mode = USER_ENCODE_MODE;
    else
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

    ak_print_normal_ex(MODULE_ID_APP,
        "Encoder [%d] activate chn!\n",Dchn.oid);
    ret = ak_app_video_set_dst_chn_active(&Dchn, AK_TRUE);

    struct video_stream *stream = ak_mem_alloc(MODULE_ID_APP, sizeof(struct video_stream));
    if(NULL == stream)
    {
        ak_print_error_ex(MODULE_ID_APP,
            "Can't alloc memory for stream buffer!\n");
        return NULL;
    }

    int frame_c = frame_num;
    while (frame_c > 0)
    {
        memset(stream, 0, sizeof(struct video_stream));

        ret = ak_app_video_venc_get_stream(&Dchn, stream);
        if(ret == AK_SUCCESS)
        {
#ifdef AK_RTOS
            mem_info_t info = {0};
            /* Get ramfs space info */
            umm_info_get(&info);
            if(info.free > SAFE_REMAINING_MEMEOY)
#endif
            {
                //ak_print_normal_ex(MODULE_ID_APP, "ak_app_video_venc_get_stream success!\n");
                if(save_file_flag)
                {
                    stream_buf_in(p_head[dev_id], stream, dev_id);
                }
                ak_venc_release_stream(venc_th->venc_handle, stream);
                frame_c--;

                if (0 == (frame_num - frame_c) % 50)
                    ak_print_normal_ex(MODULE_ID_VENC,
                        "encode num:%d\n", frame_num - frame_c);
            }
#ifdef AK_RTOS
            else
                ak_venc_release_stream(venc_th->venc_handle, stream);
#endif
        }
        else
        {
            //ak_print_error_ex(MODULE_ID_APP, "ak_app_video_venc_get_stream fail [%x]!\n",ret);
            ak_sleep_ms(10);
        }
    }

    if(stream)
        ak_mem_free(stream);

    ak_print_normal_ex(MODULE_ID_APP, "Encoder [%d] disable chn!\n", Dchn.oid);
    ret = ak_app_video_set_dst_chn_active(&Dchn, AK_FALSE);

    ak_print_normal_ex(MODULE_ID_APP, "Encoder [%d] unbind chn!\n", Dchn.oid );
    ret = ak_app_video_unbind_chn(&Schn, &Dchn);
    if(ret != AK_SUCCESS)
    {
        ak_print_error_ex(MODULE_ID_APP,
            "ak_app_video_unbind_chn failed [%d]\n",ret);
    }

    ak_thread_exit();

    return NULL;
}
/* end of func */

static int start_venc(int dev_id)
{
    int ret = -1;
    int width = resolutions[main_res].width;
    int height = resolutions[main_res].height;
    int subwidth = resolutions[sub_res].width;;
    int subheight = resolutions[sub_res].height;
    int handle_id[2] = {-1, -1};

    /* open venc */
    struct venc_param ve_param = {0};

    struct venc_rc_param rc_param = {0};
    rc_param.enable_MMA = 1;

    if (chn_index == 0)
    {
        ve_param.width  = width;            //resolution width
        ve_param.height = height;           //resolution height
    }
    else
    {
        ve_param.width  = subwidth;            //resolution width
        ve_param.height = subheight;           //resolution height
    }

    ve_param.fps    = vi_fps;               //fps set
    ve_param.goplen = 2*vi_fps;               //gop set
    ve_param.target_kbps = target_kbps;         //k bps
    ve_param.max_kbps    = 2048;        //max kbps
    ve_param.br_mode     = br_mode; //br mode
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
    ve_param.enc_out_type = encoder_type;           //enc type

    /* decode type is h264*/
    if(encoder_type == H264_ENC_TYPE)
    {
        /* profile type */
        ve_param.profile = PROFILE_MAIN;

    }
    /*decode type is jpeg*/
    else if(encoder_type == MJPEG_ENC_TYPE)
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

    /* recode the file */
    //enc_pair.file = file;
    enc_pair[dev_id].venc_handle = handle_id[dev_id];
    enc_pair[dev_id].en_type     = encoder_type;
    enc_pair[dev_id].dev_id = dev_id;

    p_head[dev_id] = ak_mem_alloc(MODULE_ID_VENC, sizeof(struct list_head));
    if(NULL == p_head[dev_id]){
        ak_print_error_ex(MODULE_ID_VENC,
            "Alloc p_head[%d] failed!\n", dev_id);
        return -1;
    }
    INIT_LIST_HEAD(p_head[dev_id]);

    /* create the venc thread */
    ret = ak_thread_create(&venc_stream_th[dev_id], video_encode_from_vi_th, &enc_pair[dev_id], \
                                                         ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);
    if(ret != AK_SUCCESS)
        ak_thread_detach(venc_stream_th[dev_id]);

    ret = ak_thread_create(&save_th[dev_id], save_file_th, &enc_pair[dev_id],  \
                                                    ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);
    if(ret != AK_SUCCESS)
        ak_thread_detach(save_th[dev_id]);

    return 0;
}
/* end of func */

/**
 * note: read the appointed video file, decode, and then output to vo  to playing
 */
#ifdef AK_RTOS
static int video_sample(int argc, char **argv)
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

    /* start to parse the opt */
    if( parse_option( argc, argv ) == AK_FALSE )
    {                                               //解释和配置选项
        return AK_FAILED;
    }

    if(frame_num <= 0)
    {
        ak_print_error_ex(MODULE_ID_APP, "frame num must be positive.\n");
        return AK_FAILED;
    }

    if(buf_size_3dnr < 0)
    {
        ak_print_error_ex(MODULE_ID_APP, "3dnr buf size must be positive.\n");
        return AK_FAILED;
    }

    /* check the res */
    if (main_res < 0 || main_res > DE_VIDEO_SIZE_MAX-1
            || sub_res < 0 || sub_res > DE_VIDEO_SIZE_MAX-1
                || main_res < sub_res)
    {
        ak_print_error_ex(MODULE_ID_APP, "resolution out of range, check it\n");
        return AK_FAILED;
    }

#if defined(__CHIP_AK3918AV100_SERIES) || defined(__CHIP_AK3918EV300L_SERIES) || defined(__CHIP_AK3918AV130_SERIES)
    /* check the br mode */
    if (br_mode < BR_MODE_CBR || br_mode >= BR_MODE_TYPE_TOTAL || BR_MODE_LOW_LATENCY == br_mode)
    {
        ak_print_error_ex(MODULE_ID_VENC, "br mode out of range, check it\n");
        return AK_FAILED;
    }
#else
    /* check the br mode */
    if (br_mode < BR_MODE_CBR || br_mode > BR_MODE_VBR)
    {
        ak_print_error_ex(MODULE_ID_VENC, "br mode out of range, check it\n");
        return AK_FAILED;
    }
#endif
    /* get the type for output */
    if (type == NULL)
    {
        ak_print_error_ex(MODULE_ID_APP, "please input the type\n");
        /* print the hint and notice */
        help_hint();
        return AK_FAILED;
    }

    /* check the chn index value */
    if (chn_index < 0 || chn_index > 1)
    {
        ak_print_error_ex(MODULE_ID_APP, "please input the correct param\n");
        /* print the hint and notice */
        help_hint();
        return AK_FAILED;
    }

    /* check the chn fps value */
    if (vi_fps < 1 || vi_fps > 30)
    {
        ak_print_error_ex(MODULE_ID_VENC,
            "please input the correct vi_fps (1 ~ 30)\n");
        /* print the hint and notice */
        help_hint();
        return AK_FAILED;
    }

    /* check the chn index value */
    if (target_kbps < 1 || target_kbps > 2048)
    {
        ak_print_error_ex(MODULE_ID_VENC,
            "please input the correct target_kbps (1 ~ 2048)\n");
        /* print the hint and notice */
        help_hint();
        return AK_FAILED;
    }

    if(dev_cnt < 1 || dev_cnt > 2)
    {
        ak_print_error_ex(MODULE_ID_APP,
            "dev_cnt error! use default value 1.\n");
        dev_cnt = 1;
    }

    /* get the type for input */
    if(!strcmp(type,"h264"))
    {
        /* h264 */
        ak_print_normal(MODULE_ID_APP, "encode type: h264\n");
        encoder_type = H264_ENC_TYPE;
    }
    else if(!strcmp(type,"h265"))
    {
        /* h265 */
        ak_print_normal(MODULE_ID_APP, "encode type:h265 \n");
        encoder_type = HEVC_ENC_TYPE;
    }
    else if(!strcmp(type,"jpeg"))
    {
        /* jpeg */
        ak_print_normal(MODULE_ID_APP, "encode type:jpeg\n");
        encoder_type = MJPEG_ENC_TYPE;
    }
    else
    {
        ak_print_normal_ex(MODULE_ID_APP,
            "unsupport video enccode input type [%s] \n", type);
        return AK_FAILED;
    }

#if defined(__CHIP_AK3918AV130_SERIES)
    if(encoder_type == HEVC_ENC_TYPE)
    {
        ak_print_error_ex(MODULE_ID_APP, "not support encode type:h265\n");
        return AK_FAILED;
    }
#endif

    if ((access(save_path, W_OK) != 0) && save_file_flag)
    {
        if (mkdir(save_path, 0755))
        {
            ak_print_error_ex(MODULE_ID_APP, "mkdir: %s failed\n", save_path);
            return AK_FAILED;
        }
    }

    /* get param from cmd line */
    int ret = -1;
    int dual_cis_mode = 0;
    if(dev_cnt > 1)
        dual_cis_mode = 1;

    ret = start_vi(VIDEO_DEV0,dual_cis_mode);
    if (dev_cnt > 1)
    {
         ret |= start_vi(VIDEO_DEV1,dual_cis_mode);
    }

    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP, "vi init failed!\n");
        goto erro;
    }

    ret = start_venc(VIDEO_DEV0);
    if (ret)
    {
        goto erro;
    }

    if (dev_cnt > 1)
    {
        ret = start_venc(VIDEO_DEV1);
    }


    /* WAITER for the thread exit */
    ak_thread_join(venc_stream_th[0]);
    ak_thread_join(save_th[0]);
    if(dev_cnt > 1 && ret == AK_SUCCESS ){
        ak_thread_join(venc_stream_th[1]);
        ak_thread_join(save_th[1]);
    }

erro:
    /* close the video encoder and video decoder */
    if(enc_pair[0].venc_handle != -1)
    {
        /* close the venc*/
        ak_venc_close(enc_pair[0].venc_handle);
        enc_pair[0].venc_handle = -1;
    }

    if (dev_cnt > 1)
    {
        if(enc_pair[1].venc_handle != -1)
        {
            /* close the venc*/
            ak_venc_close(enc_pair[1].venc_handle);
            enc_pair[1].venc_handle = -1;
        }
    }

    stop_vi(VIDEO_DEV0);

    if (dev_cnt > 1)
    {
        stop_vi(VIDEO_DEV1);
    }

    ak_sdk_exit();

    return ret;
}
/* end of func */

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(video_sample, ak_video_sample, Video Encode Sample Program);
#endif

/* end of file */

