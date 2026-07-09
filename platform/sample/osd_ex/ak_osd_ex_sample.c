/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_osd_ex_sample.c
* Description: This is a simple example to show how the osd ex module working.
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
#include "ak_osd.h"
#include "ak_mem.h"
//#include "ak_osd_ex.h"
#include "ak_osd_ex.h"

#define LEN_HINT         512            //defaut len for opt
#define LEN_OPTION_SHORT 512
#define FONT_WIDTH    16                  //default font size
#define FONT_HEIGHT    16                  //default font size
#define OSD_STRING_FORMAT "CHN%dOSD%d"  //display string

static char *pc_prog_name = NULL;

static char *save_path = "/mnt/";       //yuv data save path
static char *isp_path = "";             //vi isp cfg file
#define LOGO_FILE_DIR         "/tmp/anyka.bmp"    //logo bmp file
static char *font_file = "/usr/local/ak_font_16.bin";   //default font file
static int dev_cnt = 1;
static int g_color_type = 0;
static int bit_width = 4;
static int g_font_color = 1;
static int g_bg_color = 0;
static int g_edge_color = 2;
static int font_interval[2] = {0, 0};
static int inverse_color_en = 0;

static char ac_option_hint[  ][ LEN_HINT ] = {
    /*操作提示数组*/
    /*help*/
    "help info" ,
    /*cfg*/
    "[PATH] isp cfg file path, (dev_cnt:1  isp_xxx.conf, dev_cnt:2  "\
    "isp_xxx_dual.conf)" ,
    /*path*/
    "[PATH] path to save yuv file, default /mnt/" ,
    /*font*/
    "[FONT FILE] font file, default /usr/local/ak_font_16.bin" ,
#ifndef __CHIP_AK39EV200_SERIES
    "[NUM] device count [1 - 2], default 1" ,
    /*color type*/
    "[NUM]	color type ,0 color, 1 mono, default 0" ,
    /*bit width*/
    "[NUM]	bit width 1, 2 ,4 , default 4" ,
    /*font color*/
    "[NUM]	font color, bit_width 1 [0, 1], bit_width 2 [0, 3], "\
    "bit_width 4 [0, 15], default 1" ,
    /*background color*/
    "[NUM]	background color, bit_width 1 [0, 1], bit_width 2 [0, 3], "\
    "bit_width 4 [0, 15], default 0" ,
    /*edge color*/
    "[NUM]	edge color, bit_width 2 [0, 3], bit_width 4 [0, 15], default 2" ,
    /*main chn font interval*/
    "[NUM] main chn font interval,  default 0" ,
    /*sub chn font interval*/
    "[NUM] sub chn font interval,  default 0" ,
    /*inverse color enable*/
    "[NUM] inverse color enable,  default 0" ,
#endif
    "" ,
};

/* opt define */
static struct option option_long[ ] = {
    /*"help info"*/
    { "help"      , no_argument       , NULL , 'h' } ,
    /*"[PATH] isp cfg path"*/
    { "cfg"       , required_argument , NULL , 'f' } ,
    /*"[PATH] path to save yuv file, default /mnt/"*/
    { "path"      , required_argument , NULL , 'p' } ,
    /*"[PATH] font file path"*/
    { "font"      , required_argument , NULL , 's' } ,
#ifndef __CHIP_AK39EV200_SERIES
    /*"[NUM] device count [1 - 2], default 1"*/
    { "dev_cnt"   , required_argument , NULL , 'd' } ,
    /*"color type ,0 color, 1 mono, default 0"*/
    { "color_type", required_argument , NULL , 'c' } ,
    /*"bit width 1, 2 ,4 , default 4"*/
    { "bit_width" , required_argument , NULL , 'b' } ,
    /*"[NUM]	font color, bit_width 1 [0, 1], bit_width 2 [0, 3],
        bit_width 4 [0, 15], default 1"*/
    { "font_color" , required_argument , NULL , 'i' } ,
    /*"[NUM]	background color, bit_width 1 [0, 1], bit_width 2 [0, 3],
        bit_width 4 [0, 15], default 0"*/
    { "bg_color" , required_argument , NULL , 'j' } ,
    /*"[NUM]	edge color, bit_width 2 [0, 3], bit_width 4 [0, 15], default 2"*/
    { "edge_color" , required_argument , NULL , 'k' } ,
    /*"[NUM] main font interval,  default 0"*/
    { "main_font_interval"   , required_argument , NULL , 'l' } ,
    /*"[NUM] sub font interval,  default 0"*/
    { "sub_font_interval"   , required_argument , NULL , 'm' } ,
    /*"[NUM] inverse color enable,  default 0"*/
    { "inverse_color_en"   , required_argument , NULL , 'n' } ,
#endif
    {0, 0, 0, 0}
 };

/* usage */
static void usage(const char * name)
{
    ak_print_normal(MODULE_ID_APP, \
        "Usage: %s -f isp_cfg_path -p savepath -s font_file\n", name);
    ak_print_notice(MODULE_ID_APP, \
        "e.g: %s -f /etc/config/isp*.conf -p /mnt/ -s /mnt/ak_font_16.bin\n", \
        name);
}


/*
 * help_hint: use the -h --help option.Print option of help information
 * return: 0
 */
static int help_hint(char *pc_prog_name)
{
    int i;

    printf("%s\n" , pc_prog_name);
    /* get opt */
    for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++) {
        if( option_long[ i ].val != 0 ) {
            printf("\t--%-16s -%c %s\n" , option_long[ i ].name , \
                option_long[ i ].val , ac_option_hint[ i ]);
        }
    }

    /* printf a example */
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

    /* get opt */
    for( i = 0 ; i < i_num_option ; i ++ ) {
        if( ( c_option = p_option[ i ].val ) == 0 ) {
            continue;
        }
        switch( p_option[ i ].has_arg ){
        case no_argument:
            /* if no argument, set the offset for default */
            i_offset += snprintf( pc_option_short + i_offset , \
                i_len_option - i_offset , "%c" , c_option );
            break;
        case required_argument:
            /* required argument offset calculate */
            i_offset += snprintf( pc_option_short + i_offset , \
               i_len_option - i_offset , "%c:" , c_option );
            break;
        case optional_argument:
            /* calculate the option offset */
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
    /* opt string */
    char ac_option_short[ LEN_OPTION_SHORT ];
    /* get the option num*/
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = 1;
    /* get the option */
    pc_prog_name = argv[ 0 ];

    optind = 0;
    /* parse the cmd line input */
    get_option_short( option_long, i_array_num , ac_option_short , \
        LEN_OPTION_SHORT );
    while((i_option = getopt_long(argc , argv , ac_option_short , \
        option_long , NULL)) > 0) {
        switch(i_option) {
        case 'h' :                   //help
            help_hint(argv[0]);
            c_flag = 0;
            goto parse_option_end;
        case 'f' :                   //isp cfg path
            isp_path = optarg;
            break;
        case 'p' :                   //path
            save_path = optarg;
            break;
        case 's' :                   //path
            font_file = optarg;
            break;
#ifndef __CHIP_AK39EV200_SERIES
        case 'd' :
            dev_cnt = atoi(optarg);
            break;
        case 'c' :              //color type
            g_color_type = atoi(optarg);
            break;
        case 'b' :              //bit width
            bit_width = atoi(optarg);
            break;
        case 'i' :              //font color
            g_font_color = atoi(optarg);
            break;
        case 'j' :              //bg color
            g_bg_color = atoi(optarg);
            break;
        case 'k' :              //edge color
            g_edge_color = atoi(optarg);
            break;
        case 'l' :
            font_interval[0] = atoi(optarg);
            break;
        case 'm' :
            font_interval[1] = atoi(optarg);
            break;
        case 'n' :
            inverse_color_en = atoi(optarg);
            break;
#endif
        default :
            help_hint(argv[0]);     //help message print
            c_flag = AK_FALSE;
            goto parse_option_end;
        }
    }
parse_option_end:
    return c_flag;
}

static int app_start_vi(int dev_id,int dual_cis_mode)
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
        ak_print_error_ex(MODULE_ID_APP,\
            "Can't get dev[%d]resolution\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    } else {
        ak_print_normal_ex(MODULE_ID_APP,
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
    chn_attr.chn_id = chn_main_id;
    chn_attr.res.width = width;
    chn_attr.res.height = height;
    chn_attr.frame_depth = 2;
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
    ak_print_normal_ex(MODULE_ID_APP,
        "vi device %d main sub channel attribute\n", dev_id);


    /*
     * step 6: set sub channel attribute
     */

    VI_CHN_ATTR chn_attr_sub;
    chn_attr_sub.chn_id = chn_sub_id;
    chn_attr_sub.res.width = subwidth;
    chn_attr_sub.res.height = subheight;
    chn_attr_sub.frame_depth = 2;
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
}/* end of app_start_vi*/

static int app_stop_vi(int dev_id)
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


static void save_yuv_frame(void)
{
    int ret = -1;
    FILE *fd = NULL;
    struct ak_date date;                        //time
    char time_str[32] = {0};
    char file_path[255] = {0};
    struct video_input_frame frame = {{0}};     //vi frame
    int i = 0;

    for (i=0; i<2*dev_cnt; i++)
    {
        /* vi frame get */
        ret = ak_vi_get_frame(i, &frame);
        if (!ret) {
            ak_get_localdate(&date);            //get the time
            ak_date_to_string(&date, time_str); //change digital to string

            if (VIDEO_CHN0 == i)
#ifndef __CHIP_AK39EV200_SERIES
                sprintf(file_path, "%sdev0_osd_main%s_1920x1080.yuv", \
                    save_path, time_str);        //save yuv file
#else
                sprintf(file_path, "%sdev0_osd_main%s_1280x720.yuv", \
                    save_path, time_str);
#endif
            else if (VIDEO_CHN1 == i)
                sprintf(file_path, "%sdev0_osd_sub%s_640x360.yuv",
                    save_path, time_str);
            else if (VIDEO_CHN2 == i)
#ifndef __CHIP_AK39EV200_SERIES
                sprintf(file_path, "%sdev1_osd_main%s_1920x1080.yuv", \
                    save_path, time_str);
#else
                sprintf(file_path, "%sdev1_osd_main%s_1280x720.yuv", \
                    save_path, time_str);
#endif
            else if (VIDEO_CHN3 == i)
                sprintf(file_path, "%sdev1_osd_sub%s_640x360.yuv", \
                    save_path, time_str);

            /* save the yuv data */
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

            ak_vi_release_frame(i, &frame);
        }
    }
}

/* argb8888 table */
static unsigned int rgb_color_tables[] =
{
    0x00000000, 0xffffffff, 0xff000000, 0xff0000ff,
    0xff00ff00, 0xffff0000, 0xffffff00, 0xffff00ff,
    0xff00ffff, 0xff786085, 0xff2c8aa0, 0xff68d535,
    0xff34aa5a, 0xff43e9ab, 0xff4b55a5, 0xff008080
};

static void app_osd_set(const char *file)
{
    struct video_input_frame frame = {{0}};
    int i = 0;
    int j = 0;
    int drop_num = 5;
    int ret = -1;
    int main_font_size = 32;
    int sub_font_size = 16;

    struct osd_attr attr = {0};
    attr.color_mode_index = AK_OSD_COLOR_FMT_RGB;   //rgb mode
    attr.color_table = rgb_color_tables;            //init rgb map table
    attr.region_num = 4;
#ifdef __CHIP_AK39EV200_SERIES
    attr.region_num = 3;
#endif
    attr.color_type = g_color_type;
    attr.bit_width = bit_width;

    /* osd ex test */
    ret = ak_osd_ex_set_font_file(16, file);
    if(ret)
    {
        return;
    }

    ak_osd_ex_init2(&attr);

    ak_osd_ex_set_color(g_font_color, g_bg_color);
    if (attr.bit_width >= 2)
    {
        ak_osd_ex_set_edge_color(g_edge_color);
    }

    ak_osd_ex_set_font_size(0, main_font_size);
    ak_osd_ex_set_font_size(1, sub_font_size);
    ak_osd_ex_set_font_size(2, main_font_size);
    ak_osd_ex_set_font_size(3, sub_font_size);

    int video_chn;
    int osd_chn;
    int osd_yoffset = 0;
    char osd_string[80];
    unsigned short us_osd_string[80];

    for (video_chn = 0; video_chn < 2*dev_cnt; video_chn++)
    {
        for (osd_chn = 0; osd_chn < attr.region_num; osd_chn++) {
            int width;
            int height;
            int max_w, max_h;

            /* get the rect max size */
            if (ak_osd_ex_get_max_rect(video_chn, &max_w, &max_h) < 0) {
                ak_print_error_ex(MODULE_ID_APP, \
                    "chn:%d ak_osd_get_max_rect fail!\n", video_chn);
                return;
            }

            /* get the string code */
            sprintf(osd_string, OSD_STRING_FORMAT, video_chn, osd_chn);
            width = strlen(osd_string) * \
                (FONT_WIDTH + font_interval[video_chn % 2]);
            height = FONT_HEIGHT * 2 + 2;

            width = (width > max_w) ? max_w : width;
            height = (height > max_h) ? max_h : height;

            int x = FONT_WIDTH * 4;
            int y = FONT_HEIGHT + (height + FONT_HEIGHT) * osd_chn;
#ifdef __CHIP_AK39EV200_SERIES
            y = 0;   // 0 means create canvas above image
            osd_yoffset = osd_chn*height;   // osd string display yoffset
#endif
            // printf("len %d w %d h %d x %d y %d\n", strlen(osd_string), width, height, x, y);
            if (ak_osd_ex_set_rect(video_chn, osd_chn, x, y,
                    width, height) < 0) {
                ak_print_error_ex(MODULE_ID_APP, \
                    "video_chn:%d osd_chn:%d ak_osd_set_rect fail!\n", \
                    video_chn, osd_chn);
                return;
            }

            for (int i = 0; i < strlen(osd_string); i++)
                us_osd_string[i] = osd_string[i];

            ak_osd_ex_set_font_interval(video_chn, osd_chn,\
                font_interval[video_chn % 2]);

            /* draw string */
            if (0 == inverse_color_en)
            {
                if (ak_osd_ex_draw_str(video_chn, osd_chn, 0, osd_yoffset,
                            us_osd_string, strlen(osd_string)) < 0) {
                    ak_print_error_ex(MODULE_ID_APP, \
                        "video_chn:%d osd_chn:%d ak_osd_draw_str fail!\n", \
                        video_chn, osd_chn);
                    return;
                }
            }
            else
            {
            #ifndef __CHIP_AK39EV200_SERIES
                int font_size = main_font_size / 2; /* number and english char is half width of font size*/
                if (VIDEO_CHN1 == video_chn || VIDEO_CHN3 == video_chn)
                {
                    font_size /= 2;
                }

                struct crop_info rect = {0};
                    rect.left = x;
                rect.top = y - 1;//osd top 1 line
                rect.width = strlen(osd_string) * font_size;
                rect.height = 1; //only need 1 line

                unsigned char *buf = (unsigned char *)ak_mem_alloc(MODULE_ID_APP, rect.width * rect.height);
                if (NULL == buf)
                {
                    ak_print_error_ex(MODULE_ID_APP, "buf malloc fail!\n");
                    return;
                }

                memset(buf, 0, rect.width * rect.height);


                /* get y data, need isp driver support */
                int ret = ak_vi_get_y_data(video_chn, &rect, buf, rect.width * rect.height);
                if(ret)
                    ak_print_error_ex(MODULE_ID_APP, "ak_vi_get_y_data fail!\n");



                struct font_ctrl_attr font_ctrl_attr = {0};
                font_ctrl_attr.channel = video_chn;
                font_ctrl_attr.osd_rect = osd_chn;
                font_ctrl_attr.xoffset = 0;
                font_ctrl_attr.yoffset = 0;
                font_ctrl_attr.disp_str = us_osd_string;
                font_ctrl_attr.str_len = strlen(osd_string);

                font_ctrl_attr.font_color = (struct font_color_ctrl *)ak_mem_alloc(MODULE_ID_APP,
                    strlen(osd_string)*sizeof(struct font_color_ctrl));

                if (NULL == font_ctrl_attr.font_color)
                {
                    ak_print_error_ex(MODULE_ID_APP, "font_ctrl_attr.font_color malloc fail!\n");
                    ak_mem_free(buf);
                    return;
                }

                for(int buf_num = 0; buf_num < strlen(osd_string); buf_num++)
                {
                    int index = buf_num * font_size;
                    int sum = 0;
                    int avg = 0;

                    for(int buf_w = 0; buf_w < font_size; buf_w++)
                    {
                        sum += buf[index + buf_w];
                    }

                    avg = sum / font_size;

                    if(avg <= 128) //backgroud is black
                    {
                        font_ctrl_attr.font_color[buf_num].font_color_id = 1; //set font color white
                    }
                    else if(avg > 128) //backgroud is white
                    {
                        font_ctrl_attr.font_color[buf_num].font_color_id = 2; //set font color black
                    }
                }

                /* draw string */
                if (ak_osd_ex_draw_str_ex(&font_ctrl_attr) < 0)
                {
                    ak_print_error_ex(MODULE_ID_APP, \
                        "video_chn:%d osd_chn:%d ak_osd_draw_str fail!\n", video_chn, osd_chn);

                    /* free font_color */
                    ak_mem_free(font_ctrl_attr.font_color);
                    /* free buf */
                    ak_mem_free(buf);
                    return;
                }

                /* free font_color */
                ak_mem_free(font_ctrl_attr.font_color);
                /* free buf */
                ak_mem_free(buf);
            #endif
            }
        }
    }

    /* drop frame */
    for (i = 0; i < drop_num; i++)
    {
        for (j = 0; j < 2*dev_cnt; j++)
        {
            ret = ak_vi_get_frame(j, &frame);
            if (!ret)
                /* the frame has used,release the frame data */
                ak_vi_release_frame(j, &frame);
        }
    }

    /* save data */
    save_yuv_frame();

    /* release osd */
    ak_osd_ex_destroy();

    ak_print_normal_ex(MODULE_ID_APP, "osd set over\n");
}/* end of app_osd_set */


/**
 * Preconditions:
 * your main video progress must stop
 */
#ifdef AK_RTOS
static int osd_ex_sample(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    sdk_run_config config;
    memset(&config, 0, sizeof(config));
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_APP, "****************************\n");
    ak_print_normal(MODULE_ID_APP, "** osd version: %s **\n", \
        ak_osd_get_version());
    ak_print_normal(MODULE_ID_APP, "****************************\n");

    /* parse cmd line */
    if (0 == parse_option(argc, argv))
    {
        return 0;
    }

    ak_print_normal(MODULE_ID_APP, "osd test start.\n");

    /*
        * params check
        */
    if(dev_cnt < 1 || dev_cnt > 2)
    {
        ak_print_error_ex(MODULE_ID_APP,\
            "dev_cnt error! use default value 1.\n");
        dev_cnt = 1;
    }

    if ((bit_width != 1) && (bit_width != 2) && (bit_width != 4))
    {
        ak_print_error_ex(MODULE_ID_APP,"bit_width error!\n");
        return 0;
    }

    if (1 == bit_width)
    {
        if ((g_font_color < 0) || (g_font_color > 1))
        {
            ak_print_error_ex(MODULE_ID_APP,"g_font_color error! "\
                "when bit_width is 1, g_font_color must be in [0,1]\n");
            return 0;
        }

        if ((g_bg_color < 0) || (g_bg_color > 1))
        {
            ak_print_error_ex(MODULE_ID_APP,"g_bg_color error! "\
                "when bit_width is 1, g_bg_color must be in [0,1]\n");
            return 0;
        }
    }
    else if (2 == bit_width)
    {
        if ((g_font_color < 0) || (g_font_color > 3))
        {
            ak_print_error_ex(MODULE_ID_APP,"g_font_color error! "\
                "when bit_width is 2, g_font_color must be in [0,3]\n");
            return 0;
        }

        if ((g_bg_color < 0) || (g_bg_color > 3))
        {
            ak_print_error_ex(MODULE_ID_APP,"g_bg_color error!"\
                "when bit_width is 2, g_bg_color must be in [0,3]\n");
            return 0;
        }

        if ((g_edge_color < 0) || (g_edge_color > 3))
        {
            ak_print_error_ex(MODULE_ID_APP,"g_edge_color error!"\
                "when bit_width is 2, g_edge_color must be in [0,3]\n");
            return 0;
        }
    }
    else
    {
        if ((g_font_color < 0) || (g_font_color > 15))
        {
            ak_print_error_ex(MODULE_ID_APP,"g_font_color error! "\
                "when bit_width is 4, g_font_color must be in [0,15]\n");
            return 0;
        }

        if ((g_bg_color < 0) || (g_bg_color > 15))
        {
            ak_print_error_ex(MODULE_ID_APP,"g_bg_color error!"\
                "when bit_width is 4, g_bg_color must be in [0,15]\n");
            return 0;
        }

        if ((g_edge_color < 0) || (g_edge_color > 15))
        {
            ak_print_error_ex(MODULE_ID_APP,"g_edge_color error! "\
                "when bit_width is 4, g_edge_color must be in [0,15]\n");
            return 0;
        }
    }

    int dual_cis_mode = 0;
    if(dev_cnt > 1)
        dual_cis_mode = 1;
    /*
        * step 1: start vi
        */
    if (app_start_vi(VIDEO_DEV0,dual_cis_mode))
        goto exit;


    if (dev_cnt > 1)
    {
        app_start_vi(VIDEO_DEV1,dual_cis_mode);
    }

    /*
        * step 2: osd main function
        */
    app_osd_set(font_file);

exit:
    /*
     * step 3: release resource
     */
    app_stop_vi(VIDEO_DEV0);
    if (dev_cnt > 1)
    {
        app_stop_vi(VIDEO_DEV1);
    }

    ak_print_normal(MODULE_ID_APP, "exit osd sample\n");
    ak_sdk_exit();

    return 0;
}

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(osd_ex_sample, ak_osd_ex_sample, vi sample)
#endif

/* end of file */
