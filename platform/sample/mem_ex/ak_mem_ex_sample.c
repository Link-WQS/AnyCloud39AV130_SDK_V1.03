#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#ifdef AK_RTOS
#include <kernel.h>
#define __STATIC__  static
#else
#define __STATIC__
#endif

#include "ak_common.h"
#include "ak_common_video.h"
#include "ak_log.h"
#include "ak_mem_ex.h"
#include "ak_vi.h"
#include "ak_mem.h"

#define LEN_HINT         512
#define LEN_OPTION_SHORT 512

static char *pc_prog_name = NULL;

/* ltr support or not*/
static int ltr_en = 0;
/* roi support or not*/
static int roi_en = 0;


static char ac_option_hint[  ][ LEN_HINT ] = {
    /*操作提示数组*/
    /*help*/
    "help info" ,
    /* ltr support or not*/
    "[NUM] 1 support ltr ,0 not support ltr, default 0" ,
    /* roi support or not*/
    "[NUM] 1 support roi ,0 not support roi, default 0" ,
    "" ,
};

static struct option option_long[ ] = {
    /*"help info"*/
    { "help"            , no_argument       , NULL , 'h' } ,
    /*"[NUM] 1 support ltr ,0 not support ltr, default 0"*/
    { "ltr_en"          , required_argument , NULL , 'l' } ,
    /*"[NUM] 1 support roi ,0 not support roi, default 0"*/
    { "roi_en"          , required_argument , NULL , 'r' } ,
    {0, 0, 0, 0}
 };

/*
 * usage: help str
 * name[IN]: sample name
 * return: void
 */
static void usage(const char * name)
{
    ak_print_normal(MODULE_ID_APP, "Usage: %s -l ltr_en -r roi_en\n", name);
    ak_print_notice(MODULE_ID_APP, "e.g: %s -l 1 -r 0\n", name);
}


/*
 * help_hint: use the -h --help option.Print option of help information
 * pc_prog_name[IN]: pc_prog_name
 * return: 0
 */
static int help_hint(char *pc_prog_name)
{
    int i;

    printf("%s\n" , pc_prog_name);
    for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++) {
        if( option_long[ i ].val != 0 ) {
            printf("\t--%-16s -%c %s\n" , option_long[ i ].name , \
                option_long[ i ].val , ac_option_hint[ i ]);
        }
    }

    usage(pc_prog_name);

    printf("\n\n");
    return 0;
}

/*
 * get_option_short: fill the stort option string.
 * return: option short string addr.
 */
static char *get_option_short( struct option *p_option, int i_num_option, \
    char *pc_option_short, int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    for( i = 0 ; i < i_num_option ; i ++ ) {
        if( ( c_option = p_option[ i ].val ) == 0 ) {
            continue;
        }
        switch( p_option[ i ].has_arg ){
        case no_argument:
            i_offset += snprintf( pc_option_short + i_offset , \
                i_len_option - i_offset , "%c" , c_option );
            break;
        case required_argument:
            i_offset += snprintf( pc_option_short + i_offset , \
                i_len_option - i_offset , "%c:" , c_option );
            break;
        case optional_argument:
            i_offset += snprintf( pc_option_short + i_offset , \
                i_len_option - i_offset , "%c::" , c_option );
            break;
        }
    }
    return pc_option_short;
}

/*
 * parse_option: parse_option.
 * return: c_flag
 */
static int parse_option( int argc, char **argv )
{
    int i_option;
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = 1;
    pc_prog_name = argv[ 0 ];

    get_option_short( option_long, i_array_num , ac_option_short , \
        LEN_OPTION_SHORT );
    while((i_option = getopt_long(argc , argv , ac_option_short , \
        option_long , NULL)) > 0) {
        switch(i_option) {
        case 'h' :  //help
            help_hint(argv[0]);
            c_flag = 0;
            goto parse_option_end;
        case 'l' :  //ltr_en
            ltr_en = atoi(optarg);
            break;
        case 'r' :  //roi_en
            roi_en = atoi(optarg);
            break;
        default :
            help_hint(argv[0]);
            c_flag = AK_FALSE;
            goto parse_option_end;
        }
    }
parse_option_end:
    return c_flag;
}


/**
 * Preconditions:
 * your main video progress must stop
 */
#ifdef AK_RTOS
static int mem_ex_sample(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    /*
     * sdk init first
     */
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_APP, "*********************************\n");
    ak_print_normal(MODULE_ID_APP, "*** app_mem_ex version: %s ***\n", \
        ak_app_mem_ex_get_version());
    ak_print_normal(MODULE_ID_APP, "*********************************\n");

    if (0 == parse_option(argc, argv))
    {
        return 0;
    }

    ak_print_normal(MODULE_ID_APP, "app_mem_dma test start.\n");

    /*
     * step 1: set  vi policy
     */
    struct vi_policy vi_info = {0};

    vi_info.crop_width = 1920;
    vi_info.crop_height = 1080;
    vi_info.main_max_width = 2304;
    vi_info.main_max_height = 1296;
    vi_info.main_frame_depth = 2;
    vi_info.sub_max_width = 640;
    vi_info.sub_max_height = 360;
    vi_info.sub_frame_depth = 2;
    vi_info.thr_max_width = 320;
    vi_info.thr_max_height = 180;
    vi_info.thr_frame_depth = 2;
    vi_info.info = NULL;

    ak_app_mem_ex_dma_pool_set_vi_policy(&vi_info);

    /*
     * step 2: set  venc policy
     */
    struct venc_policy venc_info = {0};

#ifndef __CHIP_AK3918EV300L_SERIES
    /*main 1296p h264*/
    venc_info.video_chn_id = VIDEO_CHN_MAIN;
    venc_info.width = 2304;
    venc_info.height = 1296;
    venc_info.type = H264_ENC_TYPE;
    venc_info.smart_mode = ltr_en;
    venc_info.max_picture_size = 0;
    venc_info.roi_en = roi_en;
    venc_info.info = NULL;

    ak_app_mem_ex_dma_pool_set_venc_policy(&venc_info);

    /*main 1080p h264*/
    venc_info.video_chn_id = VIDEO_CHN_MAIN;
    venc_info.width = 1920;
    venc_info.height = 1080;
    venc_info.type = H264_ENC_TYPE;
    venc_info.smart_mode = ltr_en;
    venc_info.max_picture_size = 0;
    venc_info.roi_en = roi_en;
    venc_info.info = NULL;

    ak_app_mem_ex_dma_pool_set_venc_policy(&venc_info);

    /*main 720p h264*/
    venc_info.video_chn_id = VIDEO_CHN_MAIN;
    venc_info.width = 1280;
    venc_info.height = 720;
    venc_info.type = H264_ENC_TYPE;
    venc_info.smart_mode = ltr_en;
    venc_info.max_picture_size = 0;
    venc_info.roi_en = roi_en;
    venc_info.info = NULL;

    ak_app_mem_ex_dma_pool_set_venc_policy(&venc_info);
#endif

    /*main 1296p h265*/
    venc_info.video_chn_id = VIDEO_CHN_MAIN;
    venc_info.width = 2304;
    venc_info.height = 1296;
    venc_info.type = HEVC_ENC_TYPE;
    venc_info.smart_mode = ltr_en;
    venc_info.max_picture_size = 0;
    venc_info.roi_en = roi_en;
    venc_info.info = NULL;

    ak_app_mem_ex_dma_pool_set_venc_policy(&venc_info);

    /*main 1080p h265*/
    venc_info.video_chn_id = VIDEO_CHN_MAIN;
    venc_info.width = 1920;
    venc_info.height = 1080;
    venc_info.type = HEVC_ENC_TYPE;
    venc_info.smart_mode = ltr_en;
    venc_info.max_picture_size = 0;
    venc_info.roi_en = roi_en;
    venc_info.info = NULL;

    ak_app_mem_ex_dma_pool_set_venc_policy(&venc_info);

    /*main 720p h265*/
    venc_info.video_chn_id = VIDEO_CHN_MAIN;
    venc_info.width = 1280;
    venc_info.height = 720;
    venc_info.type = HEVC_ENC_TYPE;
    venc_info.smart_mode = ltr_en;
    venc_info.max_picture_size = 0;
    venc_info.roi_en = roi_en;
    venc_info.info = NULL;

    ak_app_mem_ex_dma_pool_set_venc_policy(&venc_info);

#ifndef __CHIP_AK3918EV300L_SERIES
    /*sub 360p h264*/
    venc_info.video_chn_id = VIDEO_CHN_SUB;
    venc_info.width = 640;
    venc_info.height = 360;
    venc_info.type = H264_ENC_TYPE;
    venc_info.smart_mode = ltr_en;
    venc_info.max_picture_size = 0;
    venc_info.roi_en = roi_en;
    venc_info.info = NULL;

    ak_app_mem_ex_dma_pool_set_venc_policy(&venc_info);
#endif

    /*sub 360p h265*/
    venc_info.video_chn_id = VIDEO_CHN_SUB;
    venc_info.width = 640;
    venc_info.height = 360;
    venc_info.type = HEVC_ENC_TYPE;
    venc_info.smart_mode = ltr_en;
    venc_info.max_picture_size = 0;
    venc_info.roi_en = roi_en;
    venc_info.info = NULL;

    ak_app_mem_ex_dma_pool_set_venc_policy(&venc_info);

    /*sub 360p jpeg*/
    venc_info.video_chn_id = VIDEO_CHN_SUB;
    venc_info.width = 640;
    venc_info.height = 360;
    venc_info.type = MJPEG_ENC_TYPE;
    venc_info.smart_mode = ltr_en;
    venc_info.max_picture_size = 0;
    venc_info.roi_en = roi_en;
    venc_info.info = NULL;

    ak_app_mem_ex_dma_pool_set_venc_policy(&venc_info);

    /*
     * step 3: active
     */
    ak_app_mem_ex_dma_pool_activate();

    unsigned long vi_free = 0;
    unsigned long venc_free = 0;
    ak_mem_dma_pool_get_free_mem(MODULE_ID_VI, &vi_free);
    ak_mem_dma_pool_get_free_mem(MODULE_ID_VENC, &venc_free);

    ak_print_normal(MODULE_ID_APP, "dma pool vi free %lu, venc free %lu\n", \
        vi_free, venc_free);

    /*
     * step 4: exit
     */
    ak_app_mem_ex_dma_pool_exit();
    ak_sdk_exit();
    ak_print_normal(MODULE_ID_APP, "exit app_mem_dma sample\n");

    return 0;
}/* end of main */

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(mem_ex_sample, ak_mem_ex_sample, vi sample)
#endif

/* end of file */
