/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_vi_raw_sample.c
* Description: This is a simple example to show how get raw data.
* Notes: Before running please insmod sensor_xxx.ko ak_isp.ko at first.
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
#include "ak_common_graphics.h"
#include "ak_vi.h"
#include "ak_mem.h"
#include "ak_log.h"

#ifdef AK_RTOS
#include <kernel.h>
#define __STATIC__  static
#else
#define __STATIC__
#endif

#define LEN_HINT         512
#define LEN_OPTION_SHORT 512

#ifdef FAST_LAUNCH
#define DEF_FRAME_DEPTH     2
#else
#define DEF_FRAME_DEPTH     3
#endif


#define RES_GROUP   13

static int frame_num = 10;
static int channel_num = 0;
static char *isp_path = "";
static char *save_path = "/mnt/";

static int main_res_id = 4;
static int sub_res_id = 0;
static int crop_id = 4;
static RECTANGLE_S res = {0}; 
static VI_CHN_ATTR_EX chn_attr = {0};
static VI_CHN_ATTR_EX chn_attr_sub = {0};
static int dev_cnt = 1;
static int save_file_flag = 1;



/* support resolution list */

static RECTANGLE_S  res_group[RES_GROUP]=
{
 {640, 360},            /* 640*360 */
 {640, 480},            /*   VGA   */
 {1280,720},            /*   720P  */
 {960, 1080},           /*   1080i */
 {1920,1080},           /*   1080P */
 {2304,1296},
 {2560,1440},
 {2560,1920},
 {2688,1520},
 {2880,1620},
 {2880,2160},
};


/*     ***********************************
    ***********************************
    *
    use this demo
    must follow this:
    1. make sure the driver is insmode;
    2. mount the T card;
    3. the file path is exit;
    *
    ***********************************
    ***********************************
*/

static char ac_option_hint[  ][ LEN_HINT ] = {
    "           打印帮助信息" ,
    "[NUM]      采样帧数[0 1000]" ,
    "[PATH]     ISP config file 保存路径, 默认为空,需要填写, (dev_cnt:1  isp_xxx.conf, dev_cnt:2  isp_xxx_dual.conf)" ,
    "[PATH]     采集视频帧保存目录路径, 默认为/mnt/" ,
    "[NUM]      device count [1 - 2], default 1" ,
    "[NUM]      save file flag, 0 not save, otherwise save, default 1" ,
};

static struct option option_long[ ] = {
    { "help"            , no_argument       , NULL , 'h' } , //"       打印帮助信息" ,
    { "NUM"             , required_argument , NULL , 'n' } , //"        [NUM]  采集帧数" ,
    { "isp_cfg"         , required_argument , NULL , 'f' } , //"[CONFIG] ISP config file 路径" ,
    { "path"            , required_argument , NULL , 'p' } , //"[PATH] 数据帧保存路径" ,
    { "dev_cnt"         , required_argument , NULL , 'd' } , //"[NUM]       device count [1 - 2], default 1",
    //"[NUM]       save file flag, 0 not save, otherwise save, default 1",
    { "save_file_flag"  , required_argument , NULL , 'a' } , 

    {NULL    , 0, 0, 0}
};

static void usage(const char * name)
{
    printf(" %s -n [frame_num] -f [isp_config_path] -p [frame_save_path]\n", name);

    printf("\t[isp_config_path]:     default is null \n");
    printf("\t[frame_save_path]:     default is /mnt/ \n");

    printf("eg: %s -n 1000 -f isp_xxx.conf -p vi_yuv/\n", name);
}

/*
 * help_hint: use the -h --help option.Print option of help information
 * return: 0
 */
static int help_hint( char *pc_prog_name )
{
    int i;


    printf("%s\n" , pc_prog_name);
    for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++)
    {
        if(option_long[i].name != NULL)
            printf("\t--%-16s -%c %s\n" , option_long[ i ].name , option_long[ i ].val , ac_option_hint[ i ]);
    }

    usage(pc_prog_name);

    printf("\n\n");
    return 0;
}

/*get option short function */
static char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short,
                        int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    for( i = 0 ; i < i_num_option ; i ++ )
    {
        c_option = p_option[ i ].val;
        switch( p_option[ i ].has_arg )
        {
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

/* param parse function*/
static int parse_option( int argc, char **argv )
{
    int i_option;
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = 1;
    optind = 0;

    get_option_short( option_long, i_array_num , ac_option_short , LEN_OPTION_SHORT );
    while((i_option = getopt_long(argc , argv , ac_option_short , option_long , NULL)) > 0)
    {
        switch(i_option)
        {
        case 'h' :                              //help
            help_hint( argv[ 0 ] );
            c_flag = 0;
            goto parse_option_end;
        case 'n' :                              //frame count
            frame_num = atoi( optarg );
            break;
        case 'f' :                             //path
            isp_path = optarg;
            break;
        case 'p' :                             //path
            save_path = optarg;
            break;
        case 'd' :
            dev_cnt = atoi(optarg);
            break;
        case 'a' :
            save_file_flag = atoi(optarg);
            break;
        default:
            help_hint( argv[ 0 ] );
            c_flag = 0;
            goto parse_option_end;
        }
    }
parse_option_end:
    return c_flag;
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

/*
 * save_raw_data - use to save raw data to file
 * path[IN]: pointer to saving directory
 * index[IN]: frame number index
 * frame[IN]: pointer to yuv data, include main and sub channel data
 * attr[IN]:  vi channel attribute.
 * res[IN]: vi sensor resolution
 */
static void save_raw_data(int chn_id, const char *path, int index,
        struct video_input_frame *frame, VI_CHN_ATTR_EX *attr, RECTANGLE_S *res)
{
    FILE *fd = NULL;
    unsigned int len = 0;
    unsigned char *buf = NULL;
    struct ak_date date;
    char time_str[32] = {0};
    char file_path[255] = {0};
    int dev_id;
    int data_type = attr->data_type;

    if ((VIDEO_CHN0 == chn_id) || (VIDEO_CHN1 == chn_id) || (VIDEO_CHN16 == chn_id))
    {
        dev_id = 0;
    }
    else if ((VIDEO_CHN2 == chn_id) || (VIDEO_CHN3 == chn_id) || (VIDEO_CHN17 == chn_id))
    {
        dev_id = 1;
    }
    else if ((VIDEO_CHN4 == chn_id) || (VIDEO_CHN5 == chn_id) || (VIDEO_CHN18 == chn_id))
    {
        dev_id = 2;
    }
    else 
        dev_id = 3;

    ak_print_normal(MODULE_ID_APP, "saving frame, index=%d\n", index);

    /* construct file name */
    ak_get_localdate(&date);
    ak_date_to_string(&date, time_str);

    sprintf(file_path, "%sdev%d_%s_%d_%dx%d.raw", path, dev_id, time_str, index,
            res->width, res->height);

    /*
     * open appointed file to save raw data
     * save  raw here
     */
    if(data_type == VI_DATA_TYPE_RAW)
    {
        fd = fopen(file_path, "w+b");
        if (fd) {
            buf = frame->vi_frame.data;
            len = frame->vi_frame.len;
            do {
                len -= fwrite(buf+frame->vi_frame.len-len, 1, len, fd);
            } while (len != 0);

            fclose(fd);
        } else {
            ak_print_normal(MODULE_ID_APP,    \
                    "open file [%s] failed!!\n", file_path);
        }
    }    
}
/* end of save_raw_data */

static int start_vi(int dev_id, int dual_cis_mode)
{
    /*
     * step 0: global value initialize
     */
    int ret = -1;                                //return value
    int width = res_group[main_res_id].width;
    int height = res_group[main_res_id].height;
    int subwidth = res_group[sub_res_id].width;
    int subheight = res_group[sub_res_id].height;

    int crop_width = res_group[crop_id].width;
    int crop_height = res_group[crop_id].height;

    int chn_main_id = VIDEO_CHN0;
    int chn_sub_id = VIDEO_CHN1;

    if (dev_id  == VIDEO_DEV1)
    {
        chn_main_id = VIDEO_CHN2;
        chn_sub_id = VIDEO_CHN3;
    }
    else if (dev_id  == VIDEO_DEV2)
    {
        chn_main_id = VIDEO_CHN4;
        chn_sub_id = VIDEO_CHN5;
    }
    else if (dev_id  == VIDEO_DEV3)
    {
        chn_main_id = VIDEO_CHN6;
        chn_sub_id = VIDEO_CHN7;
    }


    /* open vi flow */

    /*
     * step 1: open video input device
     */
    ret = ak_vi_open(dev_id);
    if (AK_SUCCESS != ret) {
        ak_print_error_ex(MODULE_ID_APP,    \
                "vi device %d open failed\n", dev_id);
        return ret;
    }

    /* default vi ae para */
#ifdef AK_RTOS
#ifndef __CHIP_AK37E_SERIES
#include "ak_vpss.h"
    struct vpss_isp_ae_init_info ae_init_info;
    ae_init_info.a_gain = 256;
    ae_init_info.d_gain = 256;
    ae_init_info.exp_time = 2154;
    ae_init_info.isp_d_gain = 256;
    ak_vpss_set_ae_init_info(dev_id, &ae_init_info);
#endif
#endif


#ifndef __CHIP_AK37E_SERIES
    /*
     * step 2: load isp config
     */
    ret = ak_vi_load_sensor_cfg(dev_id, isp_path);
    if (AK_SUCCESS != ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device %d load isp cfg [%s] failed!\n", dev_id, isp_path);
        return ret;
    }
#endif
    /*
     * step 3: get sensor support max resolution
     */
                   //max sensor resolution
    VI_DEV_ATTR    dev_attr;
    memset(&dev_attr, 0, sizeof(VI_DEV_ATTR));
    dev_attr.dev_id = dev_id;
    /* init the interface mode according to the input param*/
    if(dual_cis_mode != 0)
        dev_attr.interf_mode = VI_INTF_DUAL_MIPI_2;
    dev_attr.crop.left = 0;
    dev_attr.crop.top = 0;
    dev_attr.crop.width = crop_width;
    dev_attr.crop.height = crop_height;
    dev_attr.max_width = width;
    dev_attr.max_height = height;
    dev_attr.sub_max_width = subwidth;
    dev_attr.sub_max_height = subheight;

    /* get sensor resolution */
    ret = ak_vi_get_sensor_resolution(dev_id, &res);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP,     \
                "Can't get dev[%d]resolution\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    } else {
        ak_print_normal_ex(MODULE_ID_APP,    \
            "get dev res w:[%d]h:[%d]\n",res.width, res.height);
        dev_attr.crop.width = res.width;
        dev_attr.crop.height = res.height;
        dev_attr.max_width = res.width;
        dev_attr.max_height = res.height;
    }


    /*
     * step 4: set vi device working parameters
     * default parameters: 25fps, day mode
     */
    ret = ak_vi_set_dev_attr(dev_id, &dev_attr);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP,     \
                "vi device %d set device attribute failed!\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    }

    /*
     * step 5: set main channel attribute
     */

    memset(&chn_attr, 0, sizeof(VI_CHN_ATTR_EX));
    chn_attr.chn_id = chn_main_id;
    chn_attr.res.width = res.width;
    chn_attr.res.height = res.height;
    chn_attr.frame_depth = DEF_FRAME_DEPTH;
    /*disable frame control*/
    chn_attr.frame_rate = 0;
    chn_attr.data_type = VI_DATA_TYPE_RAW;
    ret = ak_vi_set_chn_attr_ex(chn_main_id, &chn_attr);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP,        \
        "vi device %d set channel [%d] attribute failed!\n", dev_id, chn_main_id);
        ak_vi_close(dev_id);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP,           \
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
    chn_attr_sub.frame_rate = 0;
    chn_attr_sub.data_type = VI_DATA_TYPE_RAW;
    ret = ak_vi_set_chn_attr_ex(chn_sub_id, &chn_attr_sub);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP,           \
            "vi device %d set channel [%d] attribute failed!\n", dev_id, chn_sub_id);
        ak_vi_close(dev_id);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP,           \
        "vi device %d set sub channel attribute\n", dev_id);

#endif
    /*
     * step 8: enable vi device
     */
    ret = ak_vi_enable_dev(dev_id);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP,        \
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
        ak_print_error_ex(MODULE_ID_APP,        \
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
        ak_print_error_ex(MODULE_ID_APP,        \
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

    if (dev_id == VIDEO_DEV1)
    {
        chn_main_id = VIDEO_CHN2;
        chn_sub_id = VIDEO_CHN3;
    }
    else if (dev_id == VIDEO_DEV2)
    {
        chn_main_id = VIDEO_CHN4;
        chn_sub_id = VIDEO_CHN5;
    }
    else if (dev_id == VIDEO_DEV3)
    {
        chn_main_id = VIDEO_CHN6;
        chn_sub_id = VIDEO_CHN7;
    }

    ak_vi_disable_chn(chn_main_id);

#ifndef __CHIP_AK37E_SERIES
    ak_vi_disable_chn(chn_sub_id);
#endif
    ak_vi_disable_dev(dev_id);
    ret = ak_vi_close(dev_id);

    return ret;
}

/*
 * vi_capture_loop: loop to get and release yuv, between get and release,
 *                  here we just save the frame to file, on your platform,
 *                  you can rewrite the save_function with your code.
 * vi_handle[IN]: pointer to vi handle, return by ak_vi_open()
 * number[IN]: save numbers
 * path[IN]:   save directory path, if NULL, will not save anymore.
 * attr[IN]:   vi main channel attribute.
 * attr_sub[IN]: vi sub channel resolution
 * res[IN]: vi sensor resolution
 */
static void vi_capture_loop(int dev_cnt, int number, const char *path,
        VI_CHN_ATTR_EX *attr, VI_CHN_ATTR_EX *attr_sub, RECTANGLE_S *res)
{
    int count = 0;
    int chn_id = 0;
    struct video_input_frame frame;

    ak_print_normal(MODULE_ID_APP, "capture start\n");

    /*
     * To get frame by loop
     */
    while (count <  number )
    {
        int cnt = 0;
        int chn_magic = 1;
        while ( cnt < dev_cnt)
        {
            memset(&frame, 0x00, sizeof(frame));
            if(channel_num < VIDEO_CHN_TOTAL)
                chn_magic = 2;

            chn_id = channel_num + cnt * chn_magic;

            /* to get frame according to the channel number */
            int ret = ak_vi_get_frame(chn_id, &frame);

            if (!ret) {
                /*
                 * Here, you can implement your code to use this frame.
                 * Notice, do not occupy it too long.
                 */
                ak_print_normal_ex(MODULE_ID_APP, "chn[%d][%d] yuv len: %u\n",chn_id, count,
                        frame.vi_frame.len);
                ak_print_normal_ex(MODULE_ID_APP, "[%d] phyaddr: %lu\n", count,
                        frame.phyaddr);

                if (save_file_flag)
                {
                    if ((chn_id == VIDEO_CHN0) || (chn_id == VIDEO_CHN2) 
                      || chn_id == VIDEO_CHN4 || chn_id == VIDEO_CHN6)
                        save_raw_data(chn_id, save_path, count, &frame, attr, res);
                    else if ((chn_id == VIDEO_CHN1) || (chn_id == VIDEO_CHN3)
                      || chn_id == VIDEO_CHN5 || chn_id == VIDEO_CHN7)
                        save_raw_data(chn_id, save_path, count, &frame, attr_sub, res);
                }
                /*
                 * in this context, this frame was useless,
                 * release frame data
                 */
                ak_vi_release_frame(chn_id, &frame);
            } else {

                /*
                 *    If getting too fast, it will have no data,
                 *    just take breath.
                 */
                ak_print_normal_ex(MODULE_ID_APP, "get frame failed!\n");
                ak_sleep_ms(10);
            }

            cnt++;
        }
            count++;
    }

    ak_print_normal(MODULE_ID_APP,"capture finish\n\n");
}
/* end of func */

/**
 * Preconditions:
 * 1??TF card is already mounted
 * 2??yuv_data is already created in /mnt
 * 3??ircut is already opened at day mode
 * 4??your main video progress must stop
 */
#ifdef AK_RTOS
static int vi_raw_sample(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    /* start the application */
    sdk_run_config config;
    memset(&config, 0, sizeof(config));
    config.mem_trace_flag = SDK_RUN_NORMAL;
    config.isp_tool_server_flag = 0;
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_APP, "*************************************\n");
    ak_print_normal(MODULE_ID_APP, "** vi demo version: %s **\n", ak_vi_get_version());
    ak_print_normal(MODULE_ID_APP, "*************************************\n");

    channel_num = VIDEO_CHN0;

    if( parse_option( argc, argv ) == 0 )
    {
        return 0;
    }
    /*check param validate*/
    if(frame_num < 0 || frame_num > 1000 ||channel_num < 0 
    || (channel_num > 1) 
    || strlen(save_path) == 0 )
    {
        ak_print_error_ex(MODULE_ID_APP,"INPUT param error!\n");
        help_hint( argv[ 0 ] );
        return 0;
    }

#ifndef __CHIP_AK37E_SERIES
    if (strlen(isp_path) == 0)
    {
        ak_print_error_ex(MODULE_ID_APP,"INPUT param error!\n");
        help_hint( argv[ 0 ] );
        return 0;
    }
#endif

    if(dev_cnt < 1 || dev_cnt > VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_APP,    \
            "dev_cnt error! use default value 1.\n");
        dev_cnt = 1;
    }

    /*check the data save path */
    if(check_dir(save_path) == 0 && save_file_flag)
    {
        ak_print_error_ex(MODULE_ID_APP, "save path is not existed!\n");
        return 0;
    }

    if(dev_cnt == 2 && main_res_id == 12)
    {
        ak_print_error_ex(MODULE_ID_APP,    \
            "dev_cnt == 2 not support res[2880][2160] !\n");
        help_hint( argv[ 0 ] );
        return 0;
    }

    int ret = -1;

    int dual_cis_mode = 0;
    if(dev_cnt > 1)
        dual_cis_mode = 1;
    /*
     * step 1: start vi
     */
    ret = start_vi(VIDEO_DEV0, dual_cis_mode);
    if (ret)
    {
        goto exit;
    }

    if (dev_cnt >1)
    {
        start_vi(VIDEO_DEV1, dual_cis_mode);
    }

    if (dev_cnt >2)
    {
        start_vi(VIDEO_DEV2, dual_cis_mode);
    }
    
    if (dev_cnt >3)
    {
        start_vi(VIDEO_DEV3, dual_cis_mode);
    }


    /*
     * step 2: start to capture and save yuv frames
     */
    vi_capture_loop(dev_cnt, frame_num, save_path, &chn_attr, &chn_attr_sub, &res);

    /*
     * step 3: release resource
     */
    stop_vi(VIDEO_DEV0);
    if (dev_cnt > 1)
    {
        stop_vi(VIDEO_DEV1);
    }
    if (dev_cnt > 2)
    {
        stop_vi(VIDEO_DEV2);
    }
    if (dev_cnt > 3)
    {
        stop_vi(VIDEO_DEV3);
    }


exit:
    /* exit */
    ak_sdk_exit();
    ak_print_normal(MODULE_ID_APP, "exit vi demo\n");
    return ret;
}
/* end of func */

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(vi_raw_sample, ak_vi_raw_sample, vi raw sample)
#endif

/* end of file */
