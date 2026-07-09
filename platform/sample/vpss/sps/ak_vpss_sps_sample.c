/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_vpss_sps_sample.c
* Description: This is a simple example to show \
* how the soft photosensitive(old method), ircut and irled module working.
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
#include "ak_sps_ir.h"
#include "ak_its.h"


#define LEN_HINT         512
#define LEN_OPTION_SHORT 512


char *isp_path = NULL;
static char *pc_prog_name = NULL;
char *ps_ir_path = NULL;
static int dev_cnt = 1;
static int dev_id = 0;
static int g_day_to_night_lum = 15000;
static int g_night_to_day_lum = 2048;
static int g_day_cnt = 740000;
static int g_night_cnt = 200000;
static int g_lock_time = 15000;
static int g_day2night_sleep_time = 500;
static int g_night2day_sleep_time = 500;

static char ac_option_hint[  ][ LEN_HINT ] = {
    /*操作提示数组*/
    "help info" ,
    "[PATH] isp cfg file path" ,
    "[PATH] ps ir cfg file path" ,
    "[NUM] device count [1 - 2], default 1" ,
    "[NUM] device id, default 0" ,
    "[NUM] day_to_night_lum, default 15000",
    "[NUM] night_to_day_lum, default 2048",
    "[NUM] day_cnt, default 740000",
    "[NUM] night_cnt, default 200000",
    "[NUM] lock_time(ms), default 15000",
    "[NUM] day2night_sleep_time(ms), default 500",
    "[NUM] night2day_sleep_time(ms), default 500",
    "" ,
};

static struct option option_long[ ] = {
    /*"help info"*/
    { "help"            , no_argument       , NULL , 'h' } ,
    /*"[PATH] isp cfg path"*/
    { "isp_cfg"         , required_argument , NULL , 'f' } ,
    /*"[PATH] ps ir cfg file path"*/
    { "ps_ir_cfg"       , required_argument , NULL , 'Z' } ,
    //"[NUM] device count [1 - 2], default 1"
    { "dev_cnt"   , required_argument , NULL , 'c' } ,
    //"[NUM] device id, default 0"
    { "dev_id"   , required_argument , NULL , 'd' } ,
    //"[NUM] day_to_night_lum, default 0"
    { "day_to_night_lum"   , required_argument , NULL , 'l' } ,
    //"[NUM] night_to_day_lum, default 0"
    { "night_to_day_lum"   , required_argument , NULL , 'm' } ,
    //"[NUM] day_cnt, default 0"
    { "day_cnt"   , required_argument , NULL , 'x' } ,
    //"[NUM] night_cnt, default 0"
    { "night_cnt"   , required_argument , NULL , 'y' } ,
    //"[NUM] lock_time, default 0"
    { "lock_time"   , required_argument , NULL , 'k' } ,
    //"[NUM] day2night_sleep_time, default 0"
    { "day2night_sleep_time"   , required_argument , NULL , 's' } ,
    //"[NUM] night2day_sleep_time, default 0"
    { "night2day_sleep_time"   , required_argument , NULL , 't' } ,
    {0, 0, 0, 0}
 };


void usage(const char * name)
{
    ak_print_normal(MODULE_ID_APP, "usage: %s -f isp config path -Z ps ir cfg path!\n", name);
    ak_print_normal(MODULE_ID_APP, "eg.: %s -f /etc/config/isp_xxx.conf -Z /etc/ps_ir_xxx.conf\n", name);
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
        case 'Z' :  //ps ir conf path
            ps_ir_path = optarg;
            break;
        case 'c' :  //dev_cnt
            dev_cnt = atoi(optarg);
            break;
        case 'd' :  //dev_id
            dev_id = atoi(optarg);
            break;
        case 'l' :  //day_to_night_lum
            g_day_to_night_lum = atoi(optarg);
            break;
        case 'm' :  //night_to_day_lum
            g_night_to_day_lum = atoi(optarg);
            break;
        case 'x' :  //dat_cnt
            g_day_cnt = atoi(optarg);
            break;
        case 'y' :  //night_cnt
            g_night_cnt = atoi(optarg);
            break;
        case 'k' :  //lock_time
            g_lock_time = atoi(optarg);
            break;
        case 's' :  //day2night_sleep_time
            g_day2night_sleep_time = atoi(optarg);
            break;
        case 't' :  //night2day_sleep_time
            g_night2day_sleep_time = atoi(optarg);
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
 * start_vi - start_vi
 * dev_id[IN] : dev_id
 * dual_cis_mode[IN] : dual_cis_mode
 * return: 0 AK_SUCCESS, otherwise failed
 * notes:
 */
static int start_vi(int dev_id,int dual_cis_mode)
{
    int ret = -1;

    /*
     * step 0: global value initialize
     */
#if defined(__CHIP_AK39EV200_SERIES)
        int width = 1280;
        int height = 720;
#else
        int width = 1920;
        int height = 1080;
#endif

    int subwidth = 640;
    int subheight = 360;
    int chn_main_id = VIDEO_CHN0;
    int chn_sub_id = VIDEO_CHN1;
    VI_CHN_ATTR chn_attr = {0};
    VI_CHN_ATTR chn_attr_sub = {0};

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
        ak_print_error_ex(MODULE_ID_APP, \
            "vi device %d open failed\n", dev_id);
        return ret;
    }

    /*
     * step 2: load isp config
     */
    ret = ak_vi_load_sensor_cfg(dev_id, isp_path);
    if (AK_SUCCESS != ret) {
        ak_print_error_ex(MODULE_ID_APP, \
            "vi device %d load isp cfg [%s] failed!\n", dev_id, isp_path);
        return ret;
    }

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
        ak_print_normal_ex(MODULE_ID_APP, \
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
        ak_print_error_ex(MODULE_ID_APP, \
            "vi device %d set device attribute failed!\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    }

    /*
     * step 5: set main channel attribute
     */

    memset(&chn_attr, 0, sizeof(VI_CHN_ATTR));
    chn_attr.chn_id = chn_main_id;
    chn_attr.res.width = width;
    chn_attr.res.height = height;
    chn_attr.frame_depth = 3;
    /*disable frame control*/
    chn_attr.frame_rate = 0;
    ret = ak_vi_set_chn_attr(chn_main_id, &chn_attr);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, \
            "vi device %d set channel [%d] attribute failed!\n", \
            dev_id, chn_main_id);
        ak_vi_close(dev_id);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP, \
        "vi device %d main sub channel attribute\n", dev_id);


    /*
     * step 6: set sub channel attribute
     */

    memset(&chn_attr_sub, 0, sizeof(VI_CHN_ATTR));
    chn_attr_sub.chn_id = chn_sub_id;
    chn_attr_sub.res.width = subwidth;
    chn_attr_sub.res.height = subheight;
    chn_attr_sub.frame_depth = 3;
    /*disable frame control*/
    chn_attr_sub.frame_rate = 0;
    ret = ak_vi_set_chn_attr(chn_sub_id, &chn_attr_sub);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, \
            "vi device %d set channel [%d] attribute failed!\n", \
            dev_id, chn_sub_id);
        ak_vi_close(dev_id);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP, \
        "vi device %d set sub channel attribute\n", dev_id);


    /*
     * step 7: enable vi device
     */
    ret = ak_vi_enable_dev(dev_id);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, \
            "vi device %d enable device  failed!\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    }

    /*
     * step 8: enable vi main channel
     */
    ret = ak_vi_enable_chn(chn_main_id);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP, \
            "vi channel[%d] enable failed!\n", chn_main_id);
        ak_vi_close(dev_id);
        return ret;
    }

    ret = ak_vi_enable_chn(chn_sub_id);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP, \
            "vi channel[%d] enable failed!\n",chn_sub_id);
        ak_vi_close(dev_id);
        return ret;
    }

    return AK_SUCCESS;
}
/* end of start_vi */

/**
 * stop_vi - stop_vi
 * dev_id[IN] : dev_id
 * return: 0 AK_SUCCESS, otherwise failed
 * notes:
 */
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
    ak_vi_disable_chn(chn_sub_id);

    ak_vi_disable_dev(dev_id);
    ret = ak_vi_close(dev_id);

    return ret;
}


/**
 * set_auto_day_night_param: set_auto_day_night_param
 * return: 0 success, -1 failed
 */
static int sps_set_auto_day_night_param(void)
{
    int i = 0;
    struct ak_auto_day_night_threshold threshold = {0};

    threshold.day_to_night_lum = g_day_to_night_lum;
    threshold.night_to_day_lum = g_night_to_day_lum;
    threshold.lock_time = g_lock_time;

    for (i = 0; i < NIGHT_ARRAY_NUM; i++) {
        threshold.night_cnt[i] = g_night_cnt;
    }

    for (i = 0; i < DAY_ARRAY_NUM; i++) {
        threshold.day_cnt[i] = g_day_cnt;
    }
    threshold.day_cnt[9] = 4000000;
    threshold.day2night_sleep_time = g_day2night_sleep_time;
    threshold.night2day_sleep_time = g_night2day_sleep_time;

    return ak_vpss_set_auto_day_night_param(dev_id, &threshold);
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

    ak_print_normal(MODULE_ID_APP, "*****************************************\n");
    ak_print_normal(MODULE_ID_APP, "** sps sample version: %s **\n", ak_vpss_get_version());
    ak_print_normal(MODULE_ID_APP, "*****************************************\n");

    if (0 == parse_option(argc, argv))
    {
        return 0;
    }

    if(dev_cnt < 1 || dev_cnt > 2)
    {
        ak_print_error_ex(MODULE_ID_APP,\
            "dev_cnt error! use default value 1.\n");
        dev_cnt = 1;
    }

    if(dev_id > dev_cnt - 1)
    {
        ak_print_error_ex(MODULE_ID_APP,\
            "dev_id error! use default value 0.\n");
        dev_id = 0;
    }

    ak_print_normal(MODULE_ID_APP, "sps test start.\n");

    int dual_cis_mode = 0;
    if(dev_cnt > 1)
        dual_cis_mode = 1;

    /*
        * step 1: start vi
        */
    if (start_vi(VIDEO_DEV0,dual_cis_mode))
        goto exit;

    if (dev_cnt > 1)
    {
        start_vi(VIDEO_DEV1,dual_cis_mode);
    }


    /*
     * step 2: start photosensitive switch
     */
    sps_set_auto_day_night_param();

    if (ak_sps_start(dev_id, ps_ir_path, dev_cnt))
        goto exit;


    struct video_input_frame  frame = {0};
    while (1)
    {
        ak_sleep_ms(20);
    }


exit:
    /*
     * step 4: release resource
     */
    ak_sps_stop();
    stop_vi(VIDEO_DEV0);
    if (dev_cnt > 1)
    {
        stop_vi(VIDEO_DEV1);
    }

    /* exit */
    ak_sdk_exit();
    ak_print_normal(MODULE_ID_APP, "exit sps sample\n");
    return 0;
}
/* end of file */
