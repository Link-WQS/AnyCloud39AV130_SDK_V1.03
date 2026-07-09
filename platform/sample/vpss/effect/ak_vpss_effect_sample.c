/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_vpss_effect_sample.c
* Description: This is a simple example to show how the vpss effect module working.
* Notes: Before running this example, please insmode ak_isp.ko \
* sensor_xxx.ko at first.
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
static int value = 0;
static char *typestr = "brightness";
static char *save_path = "/mnt/";
static char *isp_path = "";
static int dev_cnt = 1;

static char ac_option_hint[  ][ LEN_HINT ] = {
    /*鎿嶄綔鎻愮ず鏁扮粍**/
    "help info" ,
    "[STR] type to set, default brightness" ,
    "[NUM] flip_mirror[0, 3],other type [-50, 50] degree to set, default 0" ,
    "[PATH] isp cfg file path, (dev_cnt:1  isp_xxx.conf, dev_cnt:2  isp_xxx_dual.conf)" ,
    "[PATH] path to save yuv file, default /mnt/" ,
    "[NUM] device count [1 - 2], default 1" ,
    "" ,
};

static struct option option_long[ ] = {
    /*"help info"*/
    { "help"      , no_argument       , NULL , 'h' } ,
    /*"[STR] type to set, default brightness"*/
    { "type"      , required_argument , NULL , 't' } ,
    /*"[NUM] flip_mirror[0, 3],other type [-50, 50] degree to set, default 0"*/
    { "degree"    , required_argument , NULL , 'd' } ,
    /*"[PATH] isp cfg path"*/
    { "isp_cfg_path"       , required_argument , NULL , 'f' } ,
    /*"[PATH] path to save yuv file, default /mnt/"*/
    { "path"      , required_argument , NULL , 'p' } ,
    /*"[NUM] device count [1 - 2], default 1"*/
    { "dev_cnt"   , required_argument , NULL , 'c' } ,
    {0, 0, 0, 0}
 };

static void usage(const char * name)
{
    ak_print_normal(MODULE_ID_APP, \
        "Usage: %s -t type(brightness/hue/sharp/contrast/saturation/wdr/flip_mirror) "\
        "-d degree -f isp_cfg_path -p savepath\n", name);
    ak_print_notice(MODULE_ID_APP, "e.g: %s -t brightness -d 20 "\
        "-f /etc/config/isp*.conf -p /mnt/\n", name);
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

/*
 * parse_option: parse_option
 * return: c_flag
 */
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
        case 't' :  //type
            typestr = optarg;
            break;
        case 'd' :  //degree
            value = atoi(optarg);
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

/**
 * brief: save yuv data
 * @void
 * return: void;
 * notes:
 */
static void save_yuv_frame()
{
    int ret = -1;
    FILE *fd = NULL;
    struct ak_date date;
    char time_str[32] = {0};
    char file_path[255] = {0};
    struct video_input_frame frame = {{0}};

    ret = ak_vi_get_frame(VIDEO_CHN0, &frame);
    if (!ret) {
        ak_get_localdate(&date);
        ak_date_to_string(&date, time_str);
        sprintf(file_path, "%sdev0_effect_%s.yuv", save_path, time_str);

        fd = fopen(file_path, "w+b");
        if (fd) {
            unsigned char *buf = frame.vi_frame.data;
            unsigned int len = frame.vi_frame.len;
            ak_print_normal(MODULE_ID_APP, "yuv len: %u\n", len);
            do {
                len -= fwrite(buf+frame.vi_frame.len-len, 1, len, fd);
            } while (len != 0);
        } else {
            ak_print_normal(MODULE_ID_APP, "open YUV file failed!!\n");
        }

        if (fd)
            fclose(fd);

        ak_vi_release_frame(VIDEO_CHN0, &frame);
    }

    if (dev_cnt > 1)
    {
        ret = ak_vi_get_frame(VIDEO_CHN2, &frame);
        if (!ret) {
            ak_get_localdate(&date);
            ak_date_to_string(&date, time_str);
            sprintf(file_path, "%sdev1_effect_%s.yuv", save_path, time_str);

            fd = fopen(file_path, "w+b");
            if (fd) {
                unsigned char *buf = frame.vi_frame.data;
                unsigned int len = frame.vi_frame.len;
                ak_print_normal(MODULE_ID_APP, "yuv len: %u\n", len);
                do {
                    len -= fwrite(buf+frame.vi_frame.len-len, 1, len, fd);
                } while (len != 0);
            } else {
                ak_print_normal(MODULE_ID_APP, "open YUV file failed!!\n");
            }

            if (fd)
                fclose(fd);

            ak_vi_release_frame(VIDEO_CHN2, &frame);
        }
    }
}

/**
 * brief: effect main function
 * @void
 * return: void;
 * notes:
 */
static void effect_set(void)
{
    struct video_input_frame frame = {{0}};
    int i = 0;
    enum vpss_effect_type type;
    int drop_num = 40;
    int ret = -1;

    if (0 == strcmp(typestr, "brightness"))
        type = VPSS_EFFECT_BRIGHTNESS;
    else if (0 == strcmp(typestr, "hue"))
        type = VPSS_EFFECT_HUE;
    else if (0 == strcmp(typestr, "sharp"))
        type = VPSS_EFFECT_SHARP;
    else if (0 == strcmp(typestr, "contrast"))
        type = VPSS_EFFECT_CONTRAST;
    else if (0 == strcmp(typestr, "saturation"))
        type = VPSS_EFFECT_SATURATION;
    else if (0 == strcmp(typestr, "wdr"))
        type = VPSS_EFFECT_WDR;
    else if (0 == strcmp(typestr, "flip_mirror"))
        type = VPSS_FLIP_MIRROR;
    else {
        ak_print_error(MODULE_ID_APP, \
            "type string %s error ,default type brightness!\n", typestr);
        type = VPSS_EFFECT_BRIGHTNESS;
    }

    if (VPSS_FLIP_MIRROR == type)
    {
        ak_vpss_effect_set(VIDEO_DEV0, type, 0);
        if (dev_cnt > 1)
            ak_vpss_effect_set(VIDEO_DEV1, type, 0);
    }

    for (i = 0; i < drop_num; i++) {
        ak_sleep_ms(10);
        ret = ak_vi_get_frame(VIDEO_CHN0, &frame);
        if (!ret)
            /* the frame has used,release the frame data */
            ak_vi_release_frame(VIDEO_CHN0, &frame);

        if (dev_cnt > 1)
        {
            ret = ak_vi_get_frame(VIDEO_CHN2, &frame);
            if (!ret)
                /* the frame has used,release the frame data */
                ak_vi_release_frame(VIDEO_CHN2, &frame);
        }
    }

    save_yuv_frame();

    if (ak_vpss_effect_set(VIDEO_DEV0, type, value)) {
        ak_print_error(MODULE_ID_APP, "dev0 set effect fail\n");
    }

    if (dev_cnt > 1)
    {
        if (ak_vpss_effect_set(VIDEO_DEV1, type, value)) {
            ak_print_error(MODULE_ID_APP, "dev1 set effect fail\n");
        }
    }

     for (i = 0; i < drop_num; i++) {
        ak_sleep_ms(10);
        ret = ak_vi_get_frame(VIDEO_CHN0, &frame);
        if (!ret)
            /* the frame has used,release the frame data */
            ak_vi_release_frame(VIDEO_CHN0, &frame);

        if (dev_cnt > 1)
        {
            ret = ak_vi_get_frame(VIDEO_CHN2, &frame);
            if (!ret)
                /* the frame has used,release the frame data */
                ak_vi_release_frame(VIDEO_CHN2, &frame);
        }
    }

    save_yuv_frame();

    ak_print_normal_ex(MODULE_ID_APP, "effect set over\n");
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
static int vpss_effect_sample(int argc, char **argv)
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

    if ((0 == strcmp(typestr, "flip_mirror"))
        && (value > 3 || value < 0))
    {
        ak_print_error(MODULE_ID_APP, "degree error. must be [0, 3].\n");
        return 0;
    }
    else
    {
        if (value > 50 || value < -50)
        {
            ak_print_error(MODULE_ID_APP, "degree error. must be [-50, 50].\n");
            return 0;
        }
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

    ak_print_normal(MODULE_ID_APP, "effect test start.\n");

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
        * step 2: effect main function
        */
    effect_set();

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
    ak_print_normal(MODULE_ID_APP, "exit vpss effect sample\n");

    return 0;
}

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(vpss_effect_sample, ak_vpss_effect_sample, vpss effect sample)
#endif

/* end of file */
