/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_vpss_od_sample.c
* Description: This is a simple example to show how the vpss od module working.
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
static int run_time = 20;
static char *save_path = "/mnt/";
static char *isp_path = "";
static int dev_cnt = 1;

static char ac_option_hint[  ][ LEN_HINT ] = {
    /*鎿嶄綔鎻愮ず鏁扮粍**/
    "help info" ,
    "[NUM] run time in seconds, default 20s" ,
    "[PATH] isp cfg file path, (dev_cnt:1  isp_xxx.conf, dev_cnt:2  isp_xxx_dual.conf)" ,
    "[PATH] path to save md info file, default /mnt/" ,
    "[NUM] device count [1 - 2], default 1" ,
    "" ,
};

static struct option option_long[ ] = {
    /*"help info"*/
    { "help"      , no_argument       , NULL , 'h' } ,
    /*"[NUM] run time in seconds, default 20s"*/
    { "time"      , required_argument , NULL , 't' } ,
    /*"[PATH] isp cfg path"*/
    { "cfg"       , required_argument , NULL , 'f' } ,
    /*"[PATH] path to save yuv file, default /mnt/"*/
    { "path"      , required_argument , NULL , 'p' } ,
    /*"[NUM] device count [1 - 2], default 1"*/
    { "dev_cnt"   , required_argument , NULL , 'c' } ,
    {0, 0, 0, 0}
 };

static void usage(const char * name)
{
    ak_print_normal(MODULE_ID_APP, "Usage: %s -t time -f isp_cfg_path -p savepath\n", name);
    ak_print_notice(MODULE_ID_APP, "e.g: %s -t 30 -f /etc/config/isp*.conf -p /mnt/\n", name);
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
        case 'p' :  //path
            save_path = optarg;
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
        ak_print_error_ex(MODULE_ID_APP, "Can't get dev[%d]resolution\n", dev_id);
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
        ak_print_error_ex(MODULE_ID_APP,\
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

/**
 * brief: save od info
 * @void
 * return: void;
 * notes:
 */
static void od_info_save(void)
{
    struct ak_timeval start_tv;
    struct ak_timeval end_tv;
    struct vpss_od_info od = {{0}};
    int i, j, m, n, t;
    struct ak_date date;
    char time_str[32] = {0};
    char file_path[255] = {0};
    FILE *fd[2];

    ak_get_localdate(&date);
    ak_date_to_string(&date, time_str);
    sprintf(file_path, "%sdev0_od_info_%s.txt", save_path, time_str);

    fd[0] = fopen(file_path, "w+");

    if (NULL == fd[0])
    {
        ak_print_normal(MODULE_ID_APP, "dev0 open od info file failed!!\n");
        return;
    }

    if (dev_cnt > 1)
    {
        sprintf(file_path, "%sdev1_od_info_%s.txt", save_path, time_str);
        fd[1] = fopen(file_path, "w+");

        if (NULL == fd[1])
        {
            ak_print_normal(MODULE_ID_APP, "dev1 open md info file failed!!\n");
            return;
        }
    }

    ak_get_ostime(&start_tv);

    do {
        for (i=0; i<dev_cnt; i++)
        {
            ak_vpss_od_get(i, &od);

            fprintf(fd[i],"ak_vpss_od_get starts!\n");
            fprintf(fd[i],"af: ");
            fprintf(fd[i],"AK_MPI_ISP_GetOdInfo starts!\n");

            fprintf(fd[i],"af: \n");
            for(m=0; m<8; m++)
            {
                fprintf(fd[i],"%d: \n", m);
                for(n=0; n<4; n++)
                {
                    for(t=0; t<5; t++)
                    {
                        fprintf(fd[i], "%d ", od.af_statics[m][n*4][t]);
                    }
                    fprintf(fd[i], "\n");
                }
                fprintf(fd[i], "\n");
                fprintf(fd[i], "\n");
            }

            fprintf(fd[i], "\n");

            fprintf(fd[i],"ak_vpss_od_get ends!\n");
        }

        ak_get_ostime(&end_tv);
        /* run time is over and exit */
        if (ak_diff_ms_time(&end_tv, &start_tv) > (run_time * 1000))
            break;
        else
            ak_sleep_ms(100);

    } while(1);

    fclose(fd[0]);

    if (dev_cnt > 1)
        fclose(fd[1]);

    ak_print_normal_ex(MODULE_ID_APP, "od info save over\n");
}

/*
 * check_dir: check whether the 'path' was exist.
 * path[IN]: pointer to the path which will be checking.
 * return: 1 on exist, 0 is not.
 */
static int check_dir(const char *path)
{
    struct stat buf = {0};

    if (NULL == path)
        return 0;

    stat(path, &buf);
    if (S_ISDIR(buf.st_mode)) {
        return 1;
    } else {
        return 0;
    }
}


/**
 * Preconditions:
 * your main video progress must stop
 */
#ifdef AK_RTOS
static int vqss_od_sample(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_APP, "**************************************\n");
    ak_print_normal(MODULE_ID_APP, "** vpss version: %s **\n", \
        ak_vpss_get_version());
    ak_print_normal(MODULE_ID_APP, "**************************************\n");

    if (0 == parse_option(argc, argv))
    {
        return 0;
    }

    if(dev_cnt < 1 || dev_cnt > 2)
    {
        ak_print_error_ex(MODULE_ID_APP,"dev_cnt error! use default value 1.\n");
        dev_cnt = 1;
    }

    /*check the data save path */
    if(0 == check_dir(save_path))
    {
        ak_print_error_ex(MODULE_ID_APP, "save path is not existed!\n");
        return 0;
    }

    ak_print_normal(MODULE_ID_APP, "od test start.\n");

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
        * step 2: od main function
        */
    od_info_save();

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
    ak_print_normal(MODULE_ID_APP, "exit vpss od sample\n");

    return 0;
}

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(vqss_od_sample, ak_vpss_od_sample, vpss od sample)
#endif

/* end of file */
