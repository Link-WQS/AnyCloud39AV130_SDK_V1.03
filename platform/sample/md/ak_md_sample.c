/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_md_sample.c
* Description: This is a simple example to show how the md module working.
* Notes: Before running this example, please insmode ak_isp.ko \
* sensor_xxx.ko at first.
* History: V1.0.0
*/

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
#include "ak_log.h"
#include "ak_vpss.h"
#include "ak_vi.h"
#include "ak_md.h"

#define LEN_HINT         512
#define LEN_OPTION_SHORT 512

static char *pc_prog_name = NULL;
static int run_time = 20;
static int sensitivity = 80;
static int area_mode = 0;
static char *isp_path = "";
static int dev_cnt = 1;

static char ac_option_hint[  ][ LEN_HINT ] = {
    /*操作提示数组*/
    /*"help info"*/
    "help info" ,
    /*"run time"*/
    "[NUM] run time in seconds, default 20s" ,
    /*"sensitivity"*/
    "[NUM] [0, 100] sensitivity, default 80" ,
    /*"area_mode"*/
    "[NUM] [0,4]area_mode,0 whole image, 1 left half, "\
    "2 right half, 3 top half, 4 bottom half, default 0" ,
    /*"isp cfg file path"*/
    "[PATH] isp cfg file path" ,
    /*"device count"*/
    "[NUM] device count [1 - 2], default 1" ,
    "" ,
};

static struct option option_long[ ] = {
    /*"help info"*/
    { "help"            , no_argument       , NULL , 'h' } ,
    /*"[NUM] run time in seconds, default 20s"*/
    { "time"            , required_argument , NULL , 't' } ,
    /*"[NUM] [0, 100] sensitivity, default 80"*/
    { "sensitivity"     , required_argument , NULL , 's' } ,
    /*"[NUM] [0,4]area_mode,0 whole image, 1 left half,
          2 right half, 3 top half, 4 bottom half, default 0"*/
    { "area_mode"       , required_argument , NULL , 'a' } ,
    /*"[PATH] isp cfg path"*/
    { "cfg"             , required_argument , NULL , 'f' } ,
    /*"[NUM] device count [1 - 2], default 1"*/
    { "dev_cnt"         , required_argument , NULL , 'd' } ,
    {0, 0, 0, 0}
 };

static void usage(const char * name)
{
    ak_print_normal(MODULE_ID_APP, \
        "Usage: %s -t time -s sensitivity -a area -f isp_cfg_path\n", name);
    ak_print_notice(MODULE_ID_APP, \
        "e.g: %s -t 30 -s 70 -a 0 -f /etc/config/isp*.conf\n", name);
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

static int parse_option( int argc, char **argv )
{
    int i_option;
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = 1;
    pc_prog_name = argv[ 0 ];

    optind = 0;
    get_option_short( option_long, i_array_num , ac_option_short , \
        LEN_OPTION_SHORT );
    while((i_option = getopt_long(argc , argv , ac_option_short , \
        option_long , NULL)) > 0) {
        switch(i_option) {
        case 'h' :  //help
            help_hint(argv[0]);
            c_flag = 0;
            goto parse_option_end;
        case 't' :  //time
            run_time = atoi(optarg);
            break;
        case 's' :  //sensitivity
            sensitivity = atoi(optarg);
            break;
        case 'a' :  //area_mode
            area_mode = atoi(optarg);
            break;
        case 'f' :  //isp cfg path
            isp_path = optarg;
            break;
        case 'd' :
            dev_cnt = atoi(optarg);
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
 * md_start - md_start
 * dev_id[IN] : dev_id
 * return: 0 AK_SUCCESS, otherwise failed
 * notes:
 */
static int md_start(int dev_id)
{
    int ret = AK_FAILED;

    /*
    * step 1:md init
    * default parameters: detect in 10 fps
    */
    if (AK_SUCCESS != ak_md_init(dev_id))
    {
        ak_print_error_ex(MODULE_ID_APP, "dev %d md init fail\n", dev_id);
        return ret;
    }

    ak_md_set_day_night_mode(dev_id, 1); //set day night mode; 1: day mode， 0：night mode

    /*
    * step 2:set sensitivity
    */
    if (AK_SUCCESS != ak_md_set_sensitivity(dev_id, sensitivity))
    {
        ak_print_error_ex(MODULE_ID_APP, \
            "dev %d set_sensitivity fail\n", dev_id);
        return ret;
    }

    /*
    * step 3:set area
    */
    char area_mask[VPSS_MD_DIMENSION_V_MAX * VPSS_MD_DIMENSION_H_MAX] = {0};
    int v, h;


    switch (area_mode)
    {
    case 0: //whole image

        break;
    case 1: //left half area
        for (v=0; v<VPSS_MD_DIMENSION_V_MAX; v++)
        {
            for(h=0; h<VPSS_MD_DIMENSION_H_MAX/2; h++)
            {
                area_mask[v * VPSS_MD_DIMENSION_H_MAX + h] = 1;
            }
        }
        break;
    case 2: //right half area
        for (v=0; v<VPSS_MD_DIMENSION_V_MAX; v++)
        {
            for(h=VPSS_MD_DIMENSION_H_MAX/2; h<VPSS_MD_DIMENSION_H_MAX; h++)
            {
                area_mask[v * VPSS_MD_DIMENSION_H_MAX + h] = 1;
            }
        }
        break;
    case 3: //top half area
        for (v=0; v<VPSS_MD_DIMENSION_V_MAX/2; v++)
        {
            for(h=0; h<VPSS_MD_DIMENSION_H_MAX; h++)
            {
                area_mask[v * VPSS_MD_DIMENSION_H_MAX + h] = 1;
            }
        }
        break;
    case 4: //bottom half area
        for (v=VPSS_MD_DIMENSION_V_MAX/2; v<VPSS_MD_DIMENSION_V_MAX; v++)
        {
            for(h=0; h<VPSS_MD_DIMENSION_H_MAX; h++)
            {
                area_mask[v * VPSS_MD_DIMENSION_H_MAX + h] = 1;
            }
        }
        break;
    default:
        ak_print_error_ex(MODULE_ID_APP, "detect_mode error! must be 0-4.\n");
        return ret;
        break;
    }

    if (area_mode > 0)
    {
        if (AK_SUCCESS != ak_md_set_area(dev_id, area_mask))
        {
            ak_print_error_ex(MODULE_ID_APP, \
                "dev %d set_area_sensitivity fail\n", dev_id);
            return ret;
        }
    }

    /*
    * step 4:md enable
    */
    if (AK_SUCCESS != ak_md_enable(dev_id, 1))
    {
        ak_print_error_ex(MODULE_ID_APP, "dev %d md_enable fail\n", dev_id);
        return ret;
    }

    return AK_SUCCESS;
}

/**
 * md_test - md test
 * return: 0 AK_SUCCESS, otherwise failed
 * notes:
 */
static int md_test(void)
{
    int ret = AK_FAILED;
    struct ak_timeval start_tv;
    struct ak_timeval end_tv;
    struct md_result_info ret_info[2] = {0};

    ak_get_ostime(&start_tv);
    ak_print_normal(MODULE_ID_APP, "*** start to init md module.***\n");

    if (AK_SUCCESS != md_start(VIDEO_DEV0))
    {
        goto EXIT;
    }

    if (dev_cnt > 1)
    {
        md_start(VIDEO_DEV1);
    }

    /*
    * step 5:get result
    * you can create a new thread to do it
    */
    do {
        if (AK_SUCCESS == ak_md_get_result(VIDEO_DEV0, &ret_info[0], 0))
        {
            if (ret_info[0].result)
                ak_print_normal(MODULE_ID_APP, \
                "dev0 detected moving, num: %d\n", ret_info[0].move_box_num);
        }

        if (dev_cnt > 1)
        {
            if (AK_SUCCESS == ak_md_get_result(VIDEO_DEV1, &ret_info[1], 0))
            {
                if (ret_info[1].result)
                    ak_print_normal(MODULE_ID_APP, \
                    "dev1 detected moving, num: %d\n", \
                    ret_info[1].move_box_num);
            }
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
    * step 6:md destroy
    */
    ak_md_destroy(VIDEO_DEV0);
    if (dev_cnt > 1)
        ak_md_destroy(VIDEO_DEV1);

    ak_print_normal(MODULE_ID_APP, "*** md exit ***\n");

    return ret;
}
/* end of md_test */


/**
 * Preconditions:
 * your main video progress must stop
 */
#ifdef AK_RTOS
static int md_sample(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_APP, "**************************************\n");
    ak_print_normal(MODULE_ID_APP, "*** md version: %s ***\n", \
        ak_md_get_version());
    ak_print_normal(MODULE_ID_APP, "**************************************\n");

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

    ak_print_normal(MODULE_ID_APP, "md test start.\n");

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
        * step 2: md test main function
        */
    md_test();

exit:
    /*
     * step 3: release resource
     */
    stop_vi(VIDEO_DEV0);
    if (dev_cnt > 1)
    {
        stop_vi(VIDEO_DEV1);
    }

    ak_sdk_exit();
    ak_print_normal(MODULE_ID_APP, "exit md sample\n");

    return 0;
}

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(md_sample, ak_md_sample, vi sample)
#endif

/* end of file */
