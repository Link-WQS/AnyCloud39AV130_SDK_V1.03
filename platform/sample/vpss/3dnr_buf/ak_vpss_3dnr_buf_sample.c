/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_vpss_3dnr_buf_sample.c
* Description: This is a simple example to show how to control 3dnr buf module working.
* Notes: Before running this example, please insmode ak_isp.ko sensor_xxx.ko at first.
* History: V1.0.0
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>

#include "ak_common.h"
#include "ak_log.h"
#include "ak_vpss.h"
#include "ak_vi.h"

#ifdef AK_RTOS
#include <kernel.h>
#endif

#define LEN_HINT         512
#define LEN_OPTION_SHORT 512

static char *pc_prog_name = NULL;
static int run_time = 30;
static char *isp_path = "";
static unsigned int g_buf_size = 0;
static unsigned int g_default_size[VIDEO_DEV_MUX] = {0};
static int dev_cnt = 1;

static char ac_option_hint[  ][ LEN_HINT ] = {
    /*鎿嶄綔鎻愮ず鏁扮粍**/
    "help info" ,
    "[NUM] run time in seconds, default 30s" ,
    "[PATH] isp cfg file path, (dev_cnt:1  isp_xxx.conf, dev_cnt:2  isp_xxx_dual.conf)" ,
    "[NUM] 3dnr buf size, KBytes" ,
    "[NUM] device count [1 - 4], default 1" ,
    "" ,
};

static struct option option_long[ ] = {
    /*"help info"*/
    { "help"      , no_argument       , NULL , 'h' } ,
    /*"[NUM] run time in seconds, default 20s"*/
    { "time"      , required_argument , NULL , 't' } ,
    /*"[PATH] isp cfg path"*/
    { "cfg"       , required_argument , NULL , 'f' } ,
    /*"[NUM] 3dnr buf size, KBytes"*/
    { "3dnr_buf_size"      , required_argument , NULL , 's' } ,
    /*"[NUM] device count [1 - 2], default 1"*/
    { "dev_cnt"   , required_argument , NULL , 'c' } ,
    {0, 0, 0, 0}
 };

static void usage(const char * name)
{
    ak_print_normal(MODULE_ID_APP, "Usage: %s -t time -f isp_cfg_path -s 3dnr_buf_size\n", name);
    ak_print_notice(MODULE_ID_APP, "e.g: %s -t 30 -f /etc/config/isp*.conf -s 2200\n", name);
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
    optind = 0;

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
        case 's' :
            g_buf_size = atoi(optarg);
            break;
        case 'c' :
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
 * brief: start vi
 * @void
 * return: 0 success, otherwise error code;
 * notes:
 */
static int start_vi(int dev_id,int dual_cis_mode)
{
    int ret = -1;

    /*
     * step 0: global value initialize
     */
    int width = 1920;
    int height = 1080;
#ifdef __CHIP_AK39EV200_SERIES
    width = 1280;
    height = 720;
#endif
    int subwidth = 640;
    int subheight = 360;
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

    /*
     * step 2: load isp config
     */
    ret = ak_vi_load_sensor_cfg(dev_id, isp_path);
    if (AK_SUCCESS != ret) {
        ak_print_error_ex(MODULE_ID_APP,
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
        ak_print_error_ex(MODULE_ID_APP, "Can't get dev[%d]resolution\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    } else {
        ak_print_normal_ex(MODULE_ID_APP, \
            "get dev res w:[%d]h:[%d]\n",res.width, res.height);
        dev_attr.crop.width = res.width;
        dev_attr.crop.height = res.height;
    }


    /* 3dnr buf default size = 1.25 * yuv(dev crop resolution) + 64 */
    g_default_size[dev_id] = (dev_attr.crop.width * dev_attr.crop.height * 3 / 2 * 5 / 4) + 64;


    /* call ak_vi_set_dev_3DNR_memsize after ak_vi_load_sensor_cfg, before ak_vi_set_dev_attr.*/
    ak_vi_set_dev_3DNR_memsize(dev_id, g_buf_size);

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
    VI_CHN_ATTR chn_attr = {0};
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
            "vi device %d set channel [%d] attribute failed!\n", dev_id, chn_main_id);
        ak_vi_close(dev_id);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP, \
        "vi device %d main sub channel attribute\n", dev_id);


    /*
     * step 6: set sub channel attribute
     */
    VI_CHN_ATTR chn_attr_sub;
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
            "vi device %d set channel [%d] attribute failed!\n", dev_id, chn_sub_id);
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

    /*
     * step 9: enable vi sub channel
     */
    ret = ak_vi_enable_chn(chn_sub_id);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP, \
            "vi channel[%d] enable failed!\n", chn_sub_id);
        ak_vi_close(dev_id);
        return ret;
    }

    return AK_SUCCESS;
}
/* end of start_vi*/

/**
 * brief: stop vi
 * @void
 * return: void;
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

static int reduce_ae_gain_max(int dev, int ratio)
{
    struct vpss_isp_ae_attr  ae_para = {0};
    struct vpss_isp_ae_attr  suppresion_ae_para = {0};
    ak_vpss_get_ae_attr(dev, &ae_para);
    
    
    unsigned int again = ae_para.a_gain_max;
    unsigned int dgain = ae_para.d_gain_max;
    unsigned int isp_dgain = ae_para.isp_d_gain_max;
    
    unsigned long long Allgain = again * dgain * isp_dgain;
    unsigned long long AllGain_Suppresion = Allgain * ratio / 100;
    
    /* reduce ispdgain max first */
    unsigned int ispdgain_new_max = AllGain_Suppresion/(again*dgain);
    if(ispdgain_new_max < 256)
        ispdgain_new_max = 256;

    /* reduce dgain max then */
    unsigned int dgain_new_max = AllGain_Suppresion/(again*ispdgain_new_max);
    if(dgain_new_max < 256)
        dgain_new_max = 256;

    /* reduce again max last */
    unsigned int again_new_max = AllGain_Suppresion/(dgain_new_max*ispdgain_new_max);
    if(again_new_max < 256)
        again_new_max = 256;
    
    
    memcpy(&suppresion_ae_para,&ae_para,sizeof(struct vpss_isp_ae_attr)); 
    suppresion_ae_para.a_gain_max = again_new_max;
    suppresion_ae_para.d_gain_max = dgain_new_max;
    suppresion_ae_para.isp_d_gain_max = ispdgain_new_max;

    ak_print_normal(1, "dev %d old ae gain max : %u, %u, %u. new ae gain max : %u, %u, %u.\n", 
                dev, again, dgain, isp_dgain, again_new_max, dgain_new_max, ispdgain_new_max);

    return ak_vpss_set_ae_attr(dev, &suppresion_ae_para);
}


/**
 * brief: 3dnr buf control
 * @void
 * return: void;
 * notes:
 */
static void buf_control(void)
{
    struct ak_timeval start_tv;
    struct ak_timeval end_tv;
    int i = 0;
    int buflen[VIDEO_DEV_MUX] = {0};
    struct vpss_3d_nr_ref_size_stat_info ref_size = {0};
    unsigned int total = 0;
    int ratio = 0;

    for (i=0; i<dev_cnt; i++)
    {
        ak_vi_get_dev_3DNR_memsize(i, &buflen[i]);

        if (0 == buflen)
        {
            /* if ak_vi_get_dev_3DNR_memsize return 0, use default size */
            buflen[i] = g_default_size[i];
        }
        else
        {
            /* ak_vi_get_dev_3DNR_memsize return value is KBytes, convert to Bytes*/
            buflen[i] *= 1024;
        }
    }

    ak_get_ostime(&start_tv);

    while (1)
    {
        for (i=0; i<dev_cnt; i++)
        {
            /* get 3dnr buf used stat */
            ak_vpss_get_3d_nr_ref_size_stat(i, &ref_size);

            total = ref_size.ref_size_statis[0] + ref_size.ref_size_statis[1] + ref_size.ref_size_statis[2];
            ratio = total * 100 / buflen[i];
            
            ak_print_normal(1, "dev %d ref_size : %u, %u, %u. total : %u. buf size : %u. ratio : %u. \n", 
                i, ref_size.ref_size_statis[0], 
                ref_size.ref_size_statis[1], 
                ref_size.ref_size_statis[2],
                total, buflen[i], ratio);

            /* while 3dnr buf used exceed 95% (for example), reduce ae gain max*/
            while (ratio > 95)
            {
                ak_print_normal(1, "dev %d reduce_ae_gain_max.\n", i);

                /* reduce ae gain max to 75% (for example)*/
                reduce_ae_gain_max(i, 75);

                ak_sleep_ms(100);

                /* get 3dnr buf used stat */
                ak_vpss_get_3d_nr_ref_size_stat(i, &ref_size);

                total = ref_size.ref_size_statis[0] + ref_size.ref_size_statis[1] + ref_size.ref_size_statis[2];
                ratio = total * 100 / buflen[i];

                ak_print_normal(1, "dev %d ref_size : %u, %u, %u. total : %u. buf size : %u. ratio : %u. \n", 
                        i, ref_size.ref_size_statis[0], 
                        ref_size.ref_size_statis[1], 
                        ref_size.ref_size_statis[2],
                        total, buflen[i], ratio);
            }
        }
        
        ak_get_ostime(&end_tv);
        /* run time is over and exit */
        if (ak_diff_ms_time(&end_tv, &start_tv) > (run_time * 1000))
            break;
        else
            ak_sleep_ms(1000);
    }

    ak_print_normal_ex(MODULE_ID_APP, "buf_control over\n");
}


/**
 * Preconditions:
 * your main video progress must stop
 */
#ifdef AK_RTOS
static int vpss_3dnr_buf_sample(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_APP, "*************************************\n");
    ak_print_normal(MODULE_ID_APP, "** vpss version: %s **\n", \
        ak_vpss_get_version());
    ak_print_normal(MODULE_ID_APP, "*************************************\n");

    if (0 == parse_option(argc, argv))
    {
        return 0;
    }

    if(dev_cnt < 1 || dev_cnt > VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_APP,"dev_cnt error! use default value 1.\n");
        dev_cnt = 1;
    }

    ak_print_normal(MODULE_ID_APP, "3dnr buf test start.\n");


    int dual_cis_mode = 0;
    if(dev_cnt > 1)
        dual_cis_mode = 1;

    int i = 0;
    
    /*
        * step 1: start vi
        */
    for (i=0; i<dev_cnt; i++)
    {
        if (start_vi(i, dual_cis_mode))
            goto exit;
    }


    /*
        * step 2: md main function
        */
    buf_control();

exit:
    /*
     * step 3: release resource
     */
    for (i=0; i<dev_cnt; i++)
    {
        stop_vi(i);
    }
    
    ak_sdk_exit();
    ak_print_normal(MODULE_ID_APP, "exit vpss 3dnr buf sample\n");

    return 0;
}

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(vpss_3dnr_buf_sample, ak_vpss_3dnr_buf_sample, vpss 3dnr buf sample)
#endif

/* end of file */
