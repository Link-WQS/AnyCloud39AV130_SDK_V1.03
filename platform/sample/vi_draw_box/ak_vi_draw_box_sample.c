/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_vi_draw_box_sample.c
* Description: This is a simple example to show how the vi draw box module working.
* Notes:Before running this example, please insmode ak_isp.ko sensor_xxx.ko at first.
* History: V1.0.0
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <regex.h>

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
#define LEN_DATA         256

struct regloop {
    regex_t regex_t_use ;
    char *pc_pattern;
    int i_flags;
    int i_offset ;
} ;


static char *pc_prog_name = NULL;
static char *save_path = "/mnt/";
static char *isp_path = "";
struct ak_vi_box_group box_info = {0};
static int dev_cnt = 1;
static int channel = 0;

static char ac_option_hint[  ][ LEN_HINT ] = {                                         //操作提示数组
    "help info" ,
    "[NUM] channel, 0 main, 1 sub, default 0" ,
    "[left] [top] [width] [height] [enable] [color_id] [line_width] rect0 info, default disable" ,
    "[left] [top] [width] [height] [enable] [color_id] [line_width] rect1 info, default disable" ,
    "[left] [top] [width] [height] [enable] [color_id] [line_width] rect2 info, default disable" ,
    "[left] [top] [width] [height] [enable] [color_id] [line_width] rect3 info, default disable" ,
    "[PATH] isp cfg file path, (dev_cnt:1  isp_xxx.conf, dev_cnt:2  isp_xxx_dual.conf)" ,
    "[PATH] path to save yuv file, default /mnt/" ,
    "[NUM] device count [1 - 2], default 1" ,
    "[NUM] draw frame num ,-1 draw forever, 0 not draw, >0 draw frame num, default -1" ,
    "" ,
};

static struct option option_long[ ] = {
    //"help info"
    { "help"      , no_argument       , NULL , 'h' } ,
    //"[NUM] channel, 0 main, 1 sub"
    { "channel"   , required_argument , NULL , 'a' } ,
    //" [start_x] [end_x] [start_y] [end_y] [enable] [color_id] [line_width] rect0 info, default disable"
    { "rect0"     , required_argument , NULL , 'A' } ,
    //" [start_x] [end_x] [start_y] [end_y] [enable] [color_id] [line_width] rect1 info, default disable"
    { "rect1"     , required_argument , NULL , 'B' } ,
    //" [start_x] [end_x] [start_y] [end_y] [enable] [color_id] [line_width] rect2 info, default disable"
    { "rect2"     , required_argument , NULL , 'C' } ,
    //" [start_x] [end_x] [start_y] [end_y] [enable] [color_id] [line_width] rect3 info, default disable"
    { "rect3"     , required_argument , NULL , 'D' } ,
    //"[PATH] isp cfg path"
    { "cfg"       , required_argument , NULL , 'f' } ,
    //"[PATH] path to save yuv file, default /mnt/"
    { "path"      , required_argument , NULL , 'p' } ,
    //"[NUM] device count [1 - 2], default 1"
    { "dev_cnt"   , required_argument , NULL , 'c' } ,
    //"[NUM] draw frame num ,-1 draw forever, 0 not draw, >0 draw frame num, default -1"
    { "draw_frame_num"   , required_argument , NULL , 'n' } ,
    {0, 0, 0, 0}
 };

static void usage(const char * name)
{
    ak_print_normal(MODULE_ID_APP,
        "Usage: %s -a channel -A rect0 -B rect1 -C rect2 -D rect3  -f isp_cfg_path -p savepath\n", name);
    ak_print_notice(MODULE_ID_APP,
        "e.g: %s -a 0 -A 100,10,100,250,1,0,1 -B 500,10,100,60,1,1,1 -C 150,200,100,100,1,2,2 -D 500,200,100,100,1,0,4 -f /etc/config/isp*.conf -p /mnt/ -n 3\n", name);
}

#ifndef AK_RTOS
/*
    init_regloop: 初始化一个正则表达式的循环读取结构体
    @p_regloop[IN]:循环结构体
    @pc_pattern[IN]:正则表达式
    @i_flags[IN]:标志
    return: 成功:AK_SUCCESS 失败:AK_FAILED
*/
int init_regloop( struct regloop *p_regloop , char *pc_pattern , int i_flags )
{
    int i_status = 0;

    memset( p_regloop , 0 , sizeof( struct regloop ) );

    if ( ( pc_pattern != NULL ) &&
         ( ( i_status = regcomp( &p_regloop->regex_t_use , pc_pattern , i_flags ) ) == REG_NOERROR ) ) {
        p_regloop->pc_pattern = pc_pattern;
    }
    else {
        return AK_FALSE;
    }
    return AK_TRUE;
}

/*
    free_regloop: 释放一个正则表达式的循环读取结构体的正则指针
    @p_regloop[IN]:循环结构体
    return: 0
*/
int free_regloop( struct regloop *p_regloop )
{
    if( p_regloop->pc_pattern != NULL ) {
        regfree( &p_regloop->regex_t_use );
    }
    return 0;
}

/*
    reset_regloop: 重置一个正则循环结构体
    @p_regloop[IN]:循环结构体
    return: 0
*/
int reset_regloop( struct regloop *p_regloop )
{
    if( p_regloop != NULL ) {
        p_regloop->i_offset = 0;
    }
    return 0;
}

/*
    match_regloop: 重置一个正则循环结构体
    @p_regloop[IN]:循环结构体
    @pc_buff[IN]:输入的字符串
    @pc_res[OUT]:每次获取的正则匹配结果
    @i_len_res[IN]:匹配结果指针的缓冲区长度
    return: 正则匹配结果长度
*/
int match_regloop( struct regloop *p_regloop , char *pc_buff , char *pc_res , int i_len_res )
{
    int i_len_match = 0, i_len_cpy = 0;
    regmatch_t regmatch_t_res ;
    char *pc_now = pc_buff + p_regloop->i_offset ;

    pc_res[ 0 ] = 0;
    if ( regexec( &p_regloop->regex_t_use , pc_now , 1 , &regmatch_t_res , 0 ) == 0 ) {
        if ( ( i_len_match = regmatch_t_res.rm_eo - regmatch_t_res.rm_so ) <= 0 ) {                 //匹配到的字符串长度为0
            return 0;
        }

        if ( i_len_res > i_len_match ) {
            i_len_cpy = i_len_match;
        }
        else {
            i_len_cpy = i_len_res - 1;
        }
        memcpy( pc_res , pc_now + regmatch_t_res.rm_so , i_len_cpy ) ;
        pc_res[ i_len_cpy ] = 0 ;
        p_regloop->i_offset += regmatch_t_res.rm_eo;
    }
    return i_len_cpy ;
}
#endif

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
#ifndef AK_RTOS
    int i_times;
    struct regloop regloop_num;
    char ac_data[ LEN_DATA ];
#endif
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = 1;
    pc_prog_name = argv[ 0 ];
    optind = 0;

#ifndef AK_RTOS
    init_regloop( &regloop_num, "[0-9]+", REG_EXTENDED | REG_NEWLINE );   //初始化一个匹配16进制数字正则表达式
#else
    int left, top, width, height, enable, color_id, line_width;
#endif
    get_option_short( option_long, i_array_num , ac_option_short , LEN_OPTION_SHORT );
    while((i_option = getopt_long(argc , argv , ac_option_short , option_long , NULL)) > 0) {
        switch(i_option) {
        case 'h' :  //help
            help_hint(argv[0]);
            c_flag = 0;
            goto parse_option_end;
        case 'A' :  //rect0
#ifndef AK_RTOS
            reset_regloop( &regloop_num );
            i_times = 0;
            while( match_regloop( &regloop_num, optarg, ac_data, LEN_DATA ) > 0 ){
                switch( i_times ) {
                    case 0:
                        box_info.box[0].left = atoi( ac_data ) ;//left
                        break;
                    case 1:
                        box_info.box[0].top = atoi( ac_data ) ;//top
                        break;
                    case 2:
                        box_info.box[0].width = atoi( ac_data ) ;//width
                        break;
                    case 3:
                        box_info.box[0].height = atoi( ac_data ) ;//height
                        break;
                    case 4:
                        box_info.box[0].enable = atoi( ac_data ) ;//enable
                        break;
                    case 5:
                        box_info.box[0].color_id = atoi( ac_data ) ;//color_id
                        break;
                    case 6:
                        box_info.box[0].line_width = atoi( ac_data ) ;//line_width
                        break;
                }
                i_times ++;
            }
#else
            sscanf(optarg, "%d,%d,%d,%d,%d,%d,%d", &left, &top,
                               &width, &height, &enable, &color_id, &line_width);
            box_info.box[0].left = left;
            box_info.box[0].top = top;
            box_info.box[0].width = width;
            box_info.box[0].height = height;
            box_info.box[0].enable = enable;
            box_info.box[0].color_id = color_id;
            box_info.box[0].line_width = line_width;
#endif
            break;
        case 'B' :  //rect1
#ifndef AK_RTOS
            reset_regloop( &regloop_num );
            i_times = 0;
            while( match_regloop( &regloop_num, optarg, ac_data, LEN_DATA ) > 0 ){
                switch( i_times ) {
                    case 0:
                        box_info.box[1].left = atoi( ac_data ) ;//start x
                        break;
                    case 1:
                        box_info.box[1].top  = atoi( ac_data ) ;//end x
                        break;
                    case 2:
                        box_info.box[1].width = atoi( ac_data ) ;//start y
                        break;
                    case 3:
                        box_info.box[1].height  = atoi( ac_data ) ;//end y
                        break;
                    case 4:
                        box_info.box[1].enable = atoi( ac_data ) ;//enable
                        break;
                    case 5:
                        box_info.box[1].color_id = atoi( ac_data ) ;//color_id
                        break;
                    case 6:
                        box_info.box[1].line_width = atoi( ac_data ) ;//line_width
                        break;
                }
                i_times ++;
            }
#else
            sscanf(optarg, "%d,%d,%d,%d,%d,%d,%d", &left, &top,
                               &width, &height, &enable, &color_id, &line_width);
            box_info.box[1].left = left;
            box_info.box[1].top = top;
            box_info.box[1].width = width;
            box_info.box[1].height = height;
            box_info.box[1].enable = enable;
            box_info.box[1].color_id = color_id;
            box_info.box[1].line_width = line_width;
#endif
            break;
        case 'C' :  //rect2
#ifndef AK_RTOS
            reset_regloop( &regloop_num );
            i_times = 0;
            while( match_regloop( &regloop_num, optarg, ac_data, LEN_DATA ) > 0 ){
                switch( i_times ) {
                    case 0:
                        box_info.box[2].left = atoi( ac_data ) ;//start x
                        break;
                    case 1:
                        box_info.box[2].top  = atoi( ac_data ) ;//end x
                        break;
                    case 2:
                        box_info.box[2].width = atoi( ac_data ) ;//start y
                        break;
                    case 3:
                        box_info.box[2].height  = atoi( ac_data ) ;//end y
                        break;
                    case 4:
                        box_info.box[2].enable = atoi( ac_data ) ;//enable
                        break;
                    case 5:
                        box_info.box[2].color_id = atoi( ac_data ) ;//color_id
                        break;
                    case 6:
                        box_info.box[2].line_width = atoi( ac_data ) ;//line_width
                        break;
                }
                i_times ++;
            }
#else
            sscanf(optarg, "%d,%d,%d,%d,%d,%d,%d", &left, &top,
                               &width, &height, &enable, &color_id, &line_width);
            box_info.box[2].left = left;
            box_info.box[2].top = top;
            box_info.box[2].width = width;
            box_info.box[2].height = height;
            box_info.box[2].enable = enable;
            box_info.box[2].color_id = color_id;
            box_info.box[2].line_width = line_width;
#endif
            break;
        case 'D' :  //rect3
#ifndef AK_RTOS
            reset_regloop( &regloop_num );
            i_times = 0;
            while( match_regloop( &regloop_num, optarg, ac_data, LEN_DATA ) > 0 ){
                switch( i_times ) {
                    case 0:
                        box_info.box[3].left = atoi( ac_data ) ;//start x
                        break;
                    case 1:
                        box_info.box[3].top  = atoi( ac_data ) ;//end x
                        break;
                    case 2:
                        box_info.box[3].width = atoi( ac_data ) ;//start y
                        break;
                    case 3:
                        box_info.box[3].height  = atoi( ac_data ) ;//end y
                        break;
                    case 4:
                        box_info.box[3].enable = atoi( ac_data ) ;//enable
                        break;
                    case 5:
                        box_info.box[3].color_id = atoi( ac_data ) ;//color_id
                        break;
                    case 6:
                        box_info.box[3].line_width = atoi( ac_data ) ;//line_width
                        break;
                }
                i_times ++;
            }
#else
            sscanf(optarg, "%d,%d,%d,%d,%d,%d,%d", &left, &top,
                               &width, &height, &enable, &color_id, &line_width);
            box_info.box[3].left = left;
            box_info.box[3].top = top;
            box_info.box[3].width = width;
            box_info.box[3].height = height;
            box_info.box[3].enable = enable;
            box_info.box[3].color_id = color_id;
            box_info.box[3].line_width = line_width;
#endif
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
        case 'a' :
            channel = atoi(optarg);
            break;
        case 'n' :
            box_info.draw_frame_num = atoi(optarg);
            break;
        default :
            help_hint(argv[0]);
            c_flag = AK_FALSE;
            goto parse_option_end;
        }
    }
parse_option_end:
#ifndef AK_RTOS
    free_regloop( &regloop_num );
#endif
    return c_flag;
}
/* end of func */

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
    int width = 640;
    int height = 360;
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
        ak_print_error_ex(MODULE_ID_APP,    \
            "vi device %d open failed\n", dev_id);
        return ret;
    }

    /*
     * step 2: load isp config
     */
    ret = ak_vi_load_sensor_cfg(dev_id, isp_path);
    if (AK_SUCCESS != ret) {
        ak_print_error_ex(MODULE_ID_APP,        \
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
        ak_print_error_ex(MODULE_ID_APP,    \
            "Can't get dev[%d]resolution\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    } else {
        ak_print_normal_ex(MODULE_ID_APP,   \
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
        ak_print_error_ex(MODULE_ID_APP,    \
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
        ak_print_error_ex(MODULE_ID_APP,    \
            "vi device %d set channel [%d] attribute failed!\n", dev_id, chn_main_id);
        ak_vi_close(dev_id);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP,       \
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
        ak_print_error_ex(MODULE_ID_APP,    \
            "vi device %d set channel [%d] attribute failed!\n", dev_id, chn_sub_id);
        ak_vi_close(dev_id);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP,       \
        "vi device %d set sub channel attribute\n", dev_id);



    /*
     * step 7: enable vi device
     */
    ret = ak_vi_enable_dev(dev_id);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP,    \
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
        ak_print_error_ex(MODULE_ID_APP,    \
            "vi channel[%d] enable failed!\n", chn_main_id);
        ak_vi_close(dev_id);
        return ret;
    }

    ret = ak_vi_enable_chn(chn_sub_id);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP,    \
            "vi channel[%d] enable failed!\n",chn_sub_id);
        ak_vi_close(dev_id);
        return ret;
    }

    return AK_SUCCESS;
}
/* end of func */

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
    int i = 0;

    for (i = 0; i < 8; i++)
    {
        ret = ak_vi_get_frame(channel, &frame);
        if (!ret)
        {
            ak_get_localdate(&date);
            ak_date_to_string(&date, time_str);

            if (VIDEO_CHN0 == channel)
                sprintf(file_path, "%sdev0_box_main%s_640x360_%d.yuv", save_path, time_str, i);
            else
                sprintf(file_path, "%sdev0_box_sub%s_640x360_%d.yuv", save_path, time_str, i);


            fd = fopen(file_path, "w+b");
            if (fd)
            {
                unsigned char *buf = frame.vi_frame.data;
                unsigned int len = frame.vi_frame.len;
                ak_print_normal(MODULE_ID_APP, "yuv len: %u\n", len);
                do
                {
                    len -= fwrite(buf + frame.vi_frame.len - len, 1, len, fd);
                } while (len != 0);
            }
            else
            {
                ak_print_normal(MODULE_ID_APP, "open YUV file failed!!\n");
            }

            if (fd)
                fclose(fd);

            ak_vi_release_frame(channel, &frame);
        }
    }

    if (dev_cnt > 1)
    {
        for (i = 0; i < 8; i++)
        {
            ret = ak_vi_get_frame(channel+2, &frame);
            if (!ret)
            {
                ak_get_localdate(&date);
                ak_date_to_string(&date, time_str);

                if (VIDEO_CHN2 == channel+2)
                    sprintf(file_path, "%sdev1_box_main%s_640x360_%d.yuv", save_path, time_str, i);
                else
                    sprintf(file_path, "%sdev1_box_sub%s_640x360_%d.yuv", save_path, time_str, i);


                fd = fopen(file_path, "w+b");
                if (fd)
                {
                    unsigned char *buf = frame.vi_frame.data;
                    unsigned int len = frame.vi_frame.len;
                    ak_print_normal(MODULE_ID_APP, "yuv len: %u\n", len);
                    do
                    {
                        len -= fwrite(buf + frame.vi_frame.len - len, 1, len, fd);
                    } while (len != 0);
                }
                else
                {
                    ak_print_normal(MODULE_ID_APP, "open YUV file failed!!\n");
                }

                if (fd)
                    fclose(fd);

                ak_vi_release_frame(channel+2, &frame);
            }
        }
    }
}

/* argb8888 table */
static unsigned int rgb_color_table[3] =
{
    0xff0000ff, 0xff00ff00, 0xffff0000
};


/**
 * brief: box main function
 * @void
 * return: void;
 * notes:
 */
static void box_set(void)
{
    struct video_input_frame frame = {{0}};
    int i = 0;
    int j = 0;
    int drop_num = 40;
    int ret = -1;

    for (j=0; j<dev_cnt; j++)
    {
        ak_vi_set_box_color_table(j, rgb_color_table);
    }

    for (i = 0; i < drop_num; i++)
    {
        for (j=0; j<2*dev_cnt; j++)
        {
            ret = ak_vi_get_frame(j, &frame);
            if (!ret)
                /* the frame has used,release the frame data */
                ak_vi_release_frame(j, &frame);
        }
    }

    for (i=0; i<4; i++)
    {
        ak_print_normal(MODULE_ID_APP, "area %d, [%d, %d, %d, %d], enable:%d, color_id:%d, line_width:%d\n", i,
            box_info.box[i].left,
            box_info.box[i].top,
            box_info.box[i].width,
            box_info.box[i].height,
            box_info.box[i].enable,
            box_info.box[i].color_id,
            box_info.box[i].line_width);
    }


    if (ak_vi_draw_box(channel, &box_info))
    {
        ak_print_error(MODULE_ID_APP, "draw box fail\n");
    }

    if (dev_cnt > 1)
    {
        if (ak_vi_draw_box(channel+2, &box_info))
        {
            ak_print_error(MODULE_ID_APP, "draw box fail\n");
        }
    }


    save_yuv_frame();

    ak_print_normal_ex(MODULE_ID_APP, "box set over\n");
}


/**
 * Preconditions:
 * your main video progress must stop
 */
#ifdef AK_RTOS
static int box_sample(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_APP, "***********************************************\n");
    ak_print_normal(MODULE_ID_APP, "***** vi version: %s *****\n", ak_vi_get_version());
    ak_print_normal(MODULE_ID_APP, "***********************************************\n");

    box_info.draw_frame_num = -1;
    if (0 == parse_option(argc, argv))
    {
        return 0;
    }

    if(dev_cnt < 1 || dev_cnt > 2)
    {
        ak_print_error_ex(MODULE_ID_APP,    \
            "dev_cnt error! use default value 1.\n");
        dev_cnt = 1;
    }

    if(channel < 0 || channel > 1)
    {
        ak_print_error_ex(MODULE_ID_APP,    \
            "channel error! use default value 0.\n");
        channel = 0;
    }

    if (box_info.draw_frame_num < -1)
    {
        ak_print_error_ex(MODULE_ID_APP,    \
            "draw_frame_num error! use default value -1.\n");
        box_info.draw_frame_num = -1;
    }


    ak_print_normal(MODULE_ID_APP, "box test start.\n");

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
        * step 2: box main function
    */
    box_set();

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
    ak_print_normal(MODULE_ID_APP, "exit box sample\n");

    return 0;
}

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(box_sample, ak_box_sample, box sample)
#endif

/* end of file */
