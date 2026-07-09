/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_vibe_sample.c
* Description: This is a simple example to show how the vibe module working.
* Notes: Before running this example, please insmode ak_isp.ko sensor_xxx.ko at first.
* History: V1.0.0
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "ak_common.h"
#include "ak_log.h"
#include "ak_vpss.h"
#include "ak_vi.h"
#include "ak_vibe.h"

#define LEN_HINT         512
#define LEN_OPTION_SHORT 512

static char *pc_prog_name = NULL;
static int run_time = 20;
static char *isp_path = "";
static int distance = 200;
static int mode = 0;
static int size_min = 30;
static int size_max = 2400;

static int width = 1920;
static int height = 1080;
static int subwidth = 640;
static int subheight = 360;


static void *vibe_handle = NULL;


static char ac_option_hint[  ][ LEN_HINT ] = {                                         //操作提示数组
    "help info" ,
    "[NUM] run time in seconds, default 20s" ,
    "[PATH] isp cfg file path" ,
    "" ,
};

static struct option option_long[ ] = {
    { "help"            , no_argument       , NULL , 'h' } ,    //"help info"
    { "time"            , required_argument , NULL , 't' } ,    //"[NUM] run time in seconds, default 20s"
    { "cfg"             , required_argument , NULL , 'f' } ,    //"[PATH] isp cfg path"
    {0, 0, 0, 0}
 };

static void usage(const char * name)
{
    ak_print_normal(MODULE_ID_APP, "Usage: %s -t time -f isp_cfg_path\n", name);
    ak_print_notice(MODULE_ID_APP, "e.g: %s -t 30 -f /etc/config/isp*.conf\n", name);
}


/*
 * help_hint: use the -h --help option.Print option of help information
 * return: 0
 */
static int help_hint(char *pc_prog_name)
{
    int i;

    printf("%s\n" , pc_prog_name);
    for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++) {
        if( option_long[ i ].val != 0 ) {
            printf("\t--%-16s -%c %s\n" , option_long[ i ].name , option_long[ i ].val , ac_option_hint[ i ]);
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
static char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short, int i_len_option )
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
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c" , c_option );
            break;
        case required_argument:
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c:" , c_option );
            break;
        case optional_argument:
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c::" , c_option );
            break;
        }
    }
    return pc_option_short;
}

static int parse_option( int argc, char **argv )
{
    int i_option;
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = 1;
    pc_prog_name = argv[ 0 ];

    get_option_short( option_long, i_array_num , ac_option_short , LEN_OPTION_SHORT );
    while((i_option = getopt_long(argc , argv , ac_option_short , option_long , NULL)) > 0) {
        switch(i_option) {
        case 'h' :  //help
            help_hint(argv[0]);
            c_flag = 0;
            goto parse_option_end;
        case 't' :  //time
            run_time = atoi(optarg);
            break;
        case 'f' :  //isp cfg path
            isp_path = optarg;
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

static int start_vi(void)
{
    int ret = -1;

    /*
     * step 0: global value initialize
     */


    /* open vi flow */

    /*
     * step 1: open video input device
     */
    ret = ak_vi_open(VIDEO_DEV0);
    if (AK_SUCCESS != ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device open failed\n");
        return ret;
    }

    /*
     * step 2: load isp config
     */
    ret = ak_vi_load_sensor_cfg(VIDEO_DEV0, isp_path);
    if (AK_SUCCESS != ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device load isp cfg [%s] failed!\n", isp_path);
        return ret;
    }

    /*
     * step 3: get sensor support max resolution
     */
    RECTANGLE_S res = {0};				//max sensor resolution
    VI_DEV_ATTR	dev_attr;
    memset(&dev_attr, 0, sizeof(VI_DEV_ATTR));
    dev_attr.dev_id = VIDEO_DEV0;
    dev_attr.crop.left = 0;
    dev_attr.crop.top = 0;
    dev_attr.crop.width = width;
    dev_attr.crop.height = height;
    dev_attr.max_width = width;
    dev_attr.max_height = height;
    dev_attr.sub_max_width = subwidth;
    dev_attr.sub_max_height = subheight;

    ret = ak_vi_get_sensor_resolution(VIDEO_DEV0, &res);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, "Can't get dev[%d]resolution\n",VIDEO_DEV0);
        ak_vi_close(VIDEO_DEV0);
        return ret;
    } else {
        ak_print_normal_ex(MODULE_ID_APP, "get dev res w:[%d]h:[%d]\n",res.width, res.height);
        dev_attr.crop.width = res.width;
        dev_attr.crop.height = res.height;
    }

    /*
     * step 4: set vi working parameters
     * default parameters: 25fps, day mode, auto frame-control
     */
    ret = ak_vi_set_dev_attr(VIDEO_DEV0, &dev_attr);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device set device attribute failed!\n");
        ak_vi_close(VIDEO_DEV0);
        return ret;
    }

    /*
     * step 5: set main channel attribute
     */
    VI_CHN_ATTR chn_attr = {0};
    chn_attr.chn_id = VIDEO_CHN0;
    chn_attr.res.width = width;
    chn_attr.res.height = height;
    chn_attr.frame_rate= 0;
    chn_attr.frame_depth = 3;
    ret = ak_vi_set_chn_attr(VIDEO_CHN0, &chn_attr);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device set channel [%d] attribute failed!\n", VIDEO_CHN0);
        ak_vi_close(VIDEO_DEV0);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP, "vi device main sub channel attribute\n");


    /*
     * step 5: set sub channel attribute
     */
    VI_CHN_ATTR chn_attr_sub;
    chn_attr_sub.chn_id = VIDEO_CHN1;
    chn_attr_sub.res.width = subwidth;
    chn_attr_sub.res.height = subheight;
    chn_attr_sub.frame_rate= 0;
    chn_attr_sub.frame_depth = 3;
    ret = ak_vi_set_chn_attr(VIDEO_CHN1, &chn_attr_sub);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device set channel [%d] attribute failed!\n", VIDEO_CHN1);
        ak_vi_close(VIDEO_DEV0);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP, "vi device set sub channel attribute\n");

    VI_CHN_ATTR chn_attr_td;
    chn_attr_td.chn_id = VIDEO_CHN16;
    chn_attr_td.res.width = subwidth / 2;
    chn_attr_td.res.height = subheight / 2;
    chn_attr_td.frame_rate= 0;
    chn_attr_td.frame_depth = 3;
    ret = ak_vi_set_chn_attr(VIDEO_CHN16, &chn_attr_td);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device set channel [%d] attribute failed!\n", VIDEO_CHN16);
        ak_vi_close(VIDEO_DEV0);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP, "vi device set 3rd channel attribute\n");

    /*
     * step 6: enable vi device
     */
    ret = ak_vi_enable_dev(VIDEO_DEV0);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device enable device  failed!\n");
        ak_vi_close(VIDEO_DEV0);
        return ret;
    }

    /*
     * step 7: enable vi main channel
     */
    ret = ak_vi_enable_chn(VIDEO_CHN0);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP, "vi channel[%d] enable failed!\n",VIDEO_CHN0);
        ak_vi_close(VIDEO_DEV0);
        return ret;
    }

    ret = ak_vi_enable_chn(VIDEO_CHN1);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP, "vi channel[%d] enable failed!\n",VIDEO_CHN1);
        ak_vi_close(VIDEO_DEV0);
        return ret;
    }

    ret = ak_vi_enable_chn(VIDEO_CHN16);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP, "vi channel[%d] enable failed!\n",VIDEO_CHN16);
        ak_vi_close(VIDEO_DEV0);
        return ret;
    }

    return AK_SUCCESS;
}

static void stop_vi(void)
{
    ak_vi_disable_chn(VIDEO_CHN0);
    ak_vi_disable_chn(VIDEO_CHN1);
    ak_vi_disable_chn(VIDEO_CHN16);
    ak_vi_disable_dev(VIDEO_DEV0);
    ak_vi_close(VIDEO_DEV0);
}

static int vibe_test(void)
{
    int ret = AK_FAILED;
    struct ak_timeval start_tv;
    struct ak_timeval end_tv;
    VIBE_BOX boxes[VIBE_BOXES_CAP] = {{0}};
    struct video_input_frame frame = {0};

    ak_get_ostime(&start_tv);

    ak_print_normal(MODULE_ID_MD, "*** start to init vibe module.***\n");

    /*
    * step 1:vibe init
    */
    vibe_handle = ak_vibe_init();
    if (NULL == vibe_handle)
    {
        ak_print_normal(MODULE_ID_MD, "vibe init fail\n");
        return ret;
    }

    /*
    * step 2:set params
    */
    ak_vibe_set_mode(vibe_handle, mode);

    ak_vibe_set_floating_wadding_params(vibe_handle, 3, 2, 4, distance, 4, 6, 0);

    ak_vibe_set_motion_region_size(vibe_handle, size_min, size_max);


    /*
    * step 3:get result
    * you can create a new thread to do it
    */
    do {
        ret = ak_vi_get_frame(VIDEO_CHN16, &frame);

        if (AK_SUCCESS != ret)
        {
            ak_sleep_ms(10);
            continue;
        }

        ret = ak_vibe(vibe_handle, frame.vi_frame.data, subheight / 2, subwidth / 2, boxes);

        ak_vi_release_frame(VIDEO_CHN16, &frame);

        if (ret > 0)
        {
            ak_print_normal(MODULE_ID_MD, "box 0: [%d, %d, %d, %d]\n",
                boxes[0].left, boxes[0].top, boxes[0].right, boxes[0].bottom);
        }

        ak_get_ostime(&end_tv);
        /* run time is over and exit */
        if (ak_diff_ms_time(&end_tv, &start_tv) > (run_time * 1000))
            break;
        else
            ak_sleep_ms(100);

    } while(1);

    ret = AK_SUCCESS;

EXIT:

    /*
    * step 4:vibe destroy
    */
    ak_vibe_destroy(vibe_handle);
    ak_print_normal(MODULE_ID_MD, "*** vibe exit ***\n");

    return ret;
}



/**
 * Preconditions:
 * your main video progress must stop
 */
int main(int argc, char **argv)
{
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_APP, "***********************************************\n");
    ak_print_normal(MODULE_ID_APP, "***** vibe version: %s *****\n", ak_vibe_get_version());
    ak_print_normal(MODULE_ID_APP, "***********************************************\n");

    if (0 == parse_option(argc, argv))
    {
        return 0;
    }

    ak_print_normal(MODULE_ID_APP, "vibe test start.\n");

    /*
        * step 1: start vi
        */
    if (start_vi())
        goto exit;

    /*
        * step 2: vibe test main function
        */
    vibe_test();

exit:
    /*
     * step 3: release resource
     */
    stop_vi();

    ak_sdk_exit();
    ak_print_normal(MODULE_ID_APP, "exit vibe sample\n");

    return 0;
}

