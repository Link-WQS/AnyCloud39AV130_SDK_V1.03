/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_ps_ir_sample.c
* Description: This is a simple example to show \
* how the photosensitive ircut and irled module working.
* Notes:Before running this example, please insmode ak_isp.ko \
* sensor_xxx.ko at first.
* History: V1.0.0
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#include "ak_common.h"
#include "ak_vi.h"
#include "ak_mem.h"
#include "ak_log.h"
#include "ak_vpss.h"
#include "ak_drv.h"
#include "ak_thread.h"
#include "ak_ps_ir.h"
#include "ak_its.h"


#define LEN_HINT         512
#define LEN_OPTION_SHORT 512

int chn_index = 0;
char *isp_path = NULL;
VI_CHN_ATTR chn_attr = {0};
int ps_mode = HARDWARE_PHOTOSENSITIVE;
int day_night_mode = SET_AUTO_MODE;
static char *pc_prog_name = NULL;
int ain_high_level = 0;
int ain_low_level = 0;
char *ps_ir_path = NULL;


static char ac_option_hint[  ][ LEN_HINT ] = {
    /*操作提示数组*/
    "help info" ,
    "[PATH] isp cfg file path" ,
    "[NUM] [0,1]ps_mode, 0:hardware ps,1 no hardware ps" ,
    "[NUM] [0, 2] day_night_mode, 0:day,1:night,2:auto" ,
    "[NUM]  ( DEFAULT: 900 )" ,
    "[NUM]  ( DEFAULT: 400 )" ,
    "[PATH] ps ir cfg file path" ,
    "" ,
};

static struct option option_long[ ] = {
    /*"help info"*/
    { "help"            , no_argument       , NULL , 'h' } ,
    /*"[PATH] isp cfg path"*/
    { "isp_cfg"         , required_argument , NULL , 'f' } ,
    /*"[NUM] [0,1]ps_mode, 0:hardware ps,1 no hardware ps"*/
    { "ps_mode"         , required_argument , NULL , 'p' } ,
    /*"[NUM] [0, 2] day_night_mode, 0:day,1:night,2:auto"*/
    { "day_night_mode"  , required_argument , NULL , 'm' } ,
    /*"[NUM]  ( DEFAULT: 900 )"*/
    { "ain_high_level"  , required_argument , NULL , 'X' } ,
    /*"[NUM]  ( DEFAULT: 400 )"*/
    { "ain_low_level"   , required_argument , NULL , 'Y' } ,
    /*"[PATH] ps ir cfg file path"*/
    { "ps_ir_cfg"       , required_argument , NULL , 'Z' } ,
    {0, 0, 0, 0}
 };


void usage(const char * name)
{
    ak_print_normal(MODULE_ID_APP, "usage: %s -f isp config path -p ps_mode "\
        "-m day_night_mode -Z ps ir cfg path!\n", name);
    ak_print_normal(MODULE_ID_APP, "eg.: %s -f /etc/config/isp_xxx.conf "\
        "-p 0 -m 2 -Z /etc/config/ps_ir.conf\n", name);
    ak_print_normal(MODULE_ID_APP, "ps_mode : 0:hardware ps,"\
        "1 no hardware ps, default 0\n");
    ak_print_normal(MODULE_ID_APP, "day_night_mode : "\
        "0:day,1:night,2:auto, default 2\n");
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
static char *get_option_short( struct option *p_option, \
    int i_num_option, char *pc_option_short, int i_len_option )
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
        case 'f' :  //isp cfg path
            isp_path = optarg;
            break;
        case 'p' :  //ps_mode
            ps_mode = atoi(optarg);
            break;
        case 'm' :  //day_night_mode
            day_night_mode = atoi(optarg);
            break;
        case 'X' :  //ain_high_level
            ain_high_level = atoi(optarg);
            break;
        case 'Y' :  //ain_low_level
            ain_low_level = atoi(optarg);
            break;
        case 'Z' :  //ps ir conf path
            ps_ir_path = optarg;
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
 * brief: start vi
 * @void
 * return: 0 success, otherwise error code;
 * notes:
 */
static int start_vi(void)
{
    int ret = -1;

    /*
     * step 0: global value initialize
     */
    int width = 1920;
    int height = 1080;
    int subwidth = 640;
    int subheight = 360;

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
        ak_print_error_ex(MODULE_ID_APP, \
            "vi device load isp cfg [%s] failed!\n", isp_path);
        return ret;
    }

    /*
     * step 3: get sensor support max resolution
     */
    RECTANGLE_S res;				//max sensor resolution
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
        ak_print_error_ex(MODULE_ID_APP, \
            "Can't get dev[%d]resolution\n",VIDEO_DEV0);
        ak_vi_close(VIDEO_DEV0);
        return ret;
    } else {
        ak_print_normal_ex(MODULE_ID_APP, \
            "get dev res w:[%d]h:[%d]\n",res.width, res.height);
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

    memset(&chn_attr, 0, sizeof(VI_CHN_ATTR));
    chn_attr.chn_id = VIDEO_CHN0;
    chn_attr.res.width = width;
    chn_attr.res.height = height;
    chn_attr.frame_rate= 0;
    chn_attr.frame_depth = 3;
    ret = ak_vi_set_chn_attr(VIDEO_CHN0, &chn_attr);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, \
            "vi device set channel [%d] attribute failed!\n", VIDEO_CHN0);
        ak_vi_close(VIDEO_DEV0);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP, "vi device main sub channel attribute\n");


    /*
     * step 5: set sub channel attribute
     */
    VI_CHN_ATTR chn_attr_sub;
    memset(&chn_attr_sub, 0, sizeof(VI_CHN_ATTR));
    chn_attr_sub.chn_id = VIDEO_CHN1;
    chn_attr_sub.res.width = subwidth;
    chn_attr_sub.res.height = subheight;
    chn_attr_sub.frame_rate= 0;
    chn_attr_sub.frame_depth = 3;
    ret = ak_vi_set_chn_attr(VIDEO_CHN1, &chn_attr_sub);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, \
            "vi device set channel [%d] attribute failed!\n", VIDEO_CHN1);
        ak_vi_close(VIDEO_DEV0);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP, "vi device set sub channel attribute\n");

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

    /*
     * step 7: enable vi sub channel
     */
    ret = ak_vi_enable_chn(VIDEO_CHN1);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP, "vi channel[%d] enable failed!\n",VIDEO_CHN1);
        ak_vi_close(VIDEO_DEV0);
        return ret;
    }

    return AK_SUCCESS;
}
/* end of start_vi */

/**
 * brief: stop vi
 * @void
 * return: void;
 * notes:
 */
static void stop_vi(void)
{
    ak_vi_disable_chn(VIDEO_CHN0);
    ak_vi_disable_chn(VIDEO_CHN1);
    ak_vi_disable_dev(VIDEO_DEV0);
    ak_vi_close(VIDEO_DEV0);
}

/**
 * set_auto_day_night_param: set_auto_day_night_param
 * return: 0 success, -1 failed
 */
static int ps_set_auto_day_night_param(void)
{
    int i = 0;
    struct ak_auto_day_night_threshold threshold;

    threshold.day_to_night_lum = 6400;
    threshold.night_to_day_lum = 2048;
    threshold.lock_time = 15000;

    for (i = 0; i < NIGHT_ARRAY_NUM; i++) {
        threshold.night_cnt[i] = 1200;
    }

    for (i = 0; i < DAY_ARRAY_NUM; i++) {
        threshold.day_cnt[i] = 50000;
    }
    threshold.day2night_sleep_time = 500;
    threshold.night2day_sleep_time = 2000;

    return ak_vpss_set_auto_day_night_param(VIDEO_DEV0, &threshold);
}

/**
 * Preconditions:
 * your main video progress must stop
 */
int main(int argc, char **argv)
{
    /* start the application */
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );
    ak_its_start(8765);

    ak_print_normal(MODULE_ID_APP, "*****************************************\n");
    ak_print_normal(MODULE_ID_APP, "** ps sample version: %s **\n", ak_ps_get_version());
    ak_print_normal(MODULE_ID_APP, "*****************************************\n");

    if (0 == parse_option(argc, argv))
    {
        return 0;
    }

    if (ps_mode > AUTO_PHOTOSENSITIVE)
    {
        ak_print_error_ex(MODULE_ID_VPSS, "ps_mode:%d error!\n", ps_mode);
        return 0;
    }

    if (day_night_mode > SET_AUTO_MODE)
    {
        ak_print_error_ex(MODULE_ID_VPSS, "day_night_mode:%d error!\n", day_night_mode);
        return 0;
    }

    /*
     * step 1: start vi
     */
    if (start_vi())
        goto exit;


    /*
     * step 2: start photosensitive switch
     */

    if (AUTO_PHOTOSENSITIVE == ps_mode)
    {
        ps_set_auto_day_night_param();
    }


    if (ak_ps_start(VIDEO_DEV0, ps_mode, day_night_mode, ps_ir_path))
        goto exit;

    ak_ps_set_ain_th(ain_high_level, ain_low_level);

    struct video_input_frame  frame = {0};
    while (1)
    {
        ak_vi_get_frame(0, &frame);
        ak_vi_release_frame(0, &frame);
        ak_sleep_ms(20);
    }


exit:
    /*
     * step 4: release resource
     */
    ak_ps_stop();
    stop_vi();

    /* exit */
    ak_sdk_exit();
    ak_print_normal(MODULE_ID_APP, "exit ps sample\n");
    return 0;
}
/* end of file */
