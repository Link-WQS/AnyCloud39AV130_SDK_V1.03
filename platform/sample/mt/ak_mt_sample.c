/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_mt_sample.c
* Description: This is a simple example to show how the mt module working.
* Notes: Before running this example, please insmode ak_isp.ko sensor_xxx.ko 
*        ak_venc_adapter.ko ak_venc_bridge.ko ak_motor.ko at first.
* History: V1.0.0
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <fcntl.h>

#include "ak_common.h"
#include "ak_log.h"
#include "ak_mem.h"
#include "ak_thread.h"
#include "ak_vpss.h"
#include "ak_vi.h"
#include "ak_venc.h"
#include "ak_mrd.h"
#include "ak_mt.h"
#include "ak_mt_inner.h"
#include "ak_motor.h"

#define LEN_HINT                512
#define LEN_OPTION_SHORT        512

#define MOTOR0	"/dev/motor0"
#define MOTOR1	"/dev/motor1"
#define MAIN_WIDTH              1920
#define MAIN_HEIGHT             1080

#define CLIP255(x) ((x)>0?((x)<255?(x):255):0)


static char *pc_prog_name = NULL;
static int run_time = 20;
static char *isp_path = "";
static void *mt_handle = NULL;
static int motor_fd[2] = {-1, -1};
static struct motor_parm motorparm = {0};
static char *save_path    = "/mnt/mt";
static ak_pthread_t venc_stream_th;
static int venc_handle = -1;
static int venc_run = 0;
static int day_night_mode = 1;

/*box of motion rect*/
struct _box
{
  int left;
  int top;
  int width;
  int height;
};

static struct _box box_rect = {0};
static ak_mutex_t box_lock = PTHREAD_MUTEX_INITIALIZER;


static char ac_option_hint[  ][ LEN_HINT ] = {                                         //操作提示数组
    "help info" ,
    "[NUM] run time in seconds, default 20s" ,
    "[PATH] isp cfg file path" ,
    "[PATH] save video stream path" ,
    "" ,
};

static struct option option_long[ ] = {
    { "help"            , no_argument       , NULL , 'h' } ,    //"help info"
    { "time"            , required_argument , NULL , 't' } ,    //"[NUM] run time in seconds, default 20s"
    { "cfg"             , required_argument , NULL , 'f' } ,    //"[PATH] isp cfg path"
    { "save-path"       , required_argument , NULL , 'p' } ,    //"[PATH] save video stream path"
    {0, 0, 0, 0}
 };

static void usage(const char * name)
{
    ak_print_normal(MODULE_ID_APP, "Usage: %s -t time -f isp_cfg_path -p save_path\n", name);
    ak_print_notice(MODULE_ID_APP, "e.g: %s -t 30 -f /etc/config/isp*.conf -p /mnt/mt\n", name);
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
        case 'p' :  //save path
            save_path = optarg;
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
    int width = MAIN_WIDTH;
    int height = MAIN_HEIGHT;
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
    memset(&chn_attr, 0, sizeof(VI_CHN_ATTR));
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
    memset(&chn_attr_sub, 0, sizeof(VI_CHN_ATTR));
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

    return AK_SUCCESS;
}
/* end of func */

/**
 * brief: stop vi
 * @void
 * return: void;
 * notes:
 */
static void stop_vi(void)
{
    ak_vi_disable_chn(VIDEO_CHN0);
    ak_vi_disable_dev(VIDEO_DEV0);
    ak_vi_close(VIDEO_DEV0);
}

/**
 * brief: motor open and self check
 * @void
 * return: 0 success, -1 failed;
 * notes:
 */
static int motor_open(void)
{
    int ret = -1;

    /*open all motors*/
    motor_fd[0] = open(MOTOR0, 0);


    /*simple check*/
    if (motor_fd[0] < 0)
    {
        ak_print_normal(MODULE_ID_APP, "open motor 0 fail.\n");
        return -1;
    }

    motor_fd[1] = open(MOTOR1, 0);

    if (motor_fd[1] < 0)
    {
        ak_print_normal(MODULE_ID_APP, "open motor 1 fail.\n");
        close(motor_fd[0]);
        return -1;
    }

    /*set motor params*/

    motorparm.pos = 0;
    motorparm.speed_step = 1000;
    motorparm.steps_one_circle = 4096;
    motorparm.total_steps = 4000;
    motorparm.boundary_steps = 20;

    ret = ioctl(motor_fd[0], MOTOR_PARM, &motorparm);
    if (ret)
        ak_print_normal(MODULE_ID_APP, "motor 0 ioctl fail, ret:%d\n", ret);

    motorparm.pos = 0;
    motorparm.speed_step = 1000;
    motorparm.steps_one_circle = 4096;
    motorparm.total_steps = 1000;
    motorparm.boundary_steps = 20;

    ret = ioctl(motor_fd[1], MOTOR_PARM, &motorparm);
    if (ret)
        ak_print_normal(MODULE_ID_APP, "motor 1 ioctl fail, ret:%d\n", ret);

    /*self check*/
    struct motor_reset_data reset;
    ioctl(motor_fd[0], MOTOR_RESET, &reset);
    ioctl(motor_fd[1], MOTOR_RESET, &reset);

    return 0;
}

/**
 * brief: motor close
 * @void
 * return: 0 success, -1 failed;
 * notes:
 */
static int motor_close(void)
{
    if (motor_fd[0])
        close(motor_fd[0]);

    if (motor_fd[1])
        close(motor_fd[1]);

    return 0;
}

/**
 * brief: angle to step
 * @angle : angle value
 * return: step value;
 * notes:
 */
static int motor_angle2step(int angle)
{
    return angle * motorparm.steps_one_circle / 360;
}

/**
 * brief: step to angle
 * @step : step value
 * return: angle value;
 * notes:
 */
static int motor_step2angle(int step)
{
    return step * 360 / motorparm.steps_one_circle;
}

/**
 * draw_box - draw a rectangle box on the frame
 * @frame[IN]: frames
 * @r[IN]: R component of RGB color space
 * @g[IN]: G component of RGB color space
 * @b[IN]: B component of RGB color space
 * @vid_height: video height
 * @vid_width: video width
 * @box_x: the horizontal position of left top point of the drawing box
 * @box_y: the vertical position of left top point of the drawing box
 * @box_height: the height of drawing box
 * @box_width: the width of drawing box
 * @line_size: the size of drawing lines
 * return: 0 success, -1 failed
 * notes: if line_size is too small, for example, line_size = 2, the real-time drawing box seems to miss certain lines
 */
static int draw_box(struct video_input_frame *frame,
    unsigned char r, unsigned char g, unsigned char b,
    int vid_height, int vid_width, int box_x, int box_y, int box_height, int box_width, int line_size)
{
    int i, j; // code_updating
    unsigned char yuv_y, yuv_u, yuv_v;

    return 0;

    yuv_y = CLIP255((306*r + 601*g + 117*b) >> 10);
    yuv_u = CLIP255(((-173*r - 339*g +512*b) >> 10) + 128);
    yuv_v = CLIP255(((512*r -429*g -83*b) >> 10) + 128);

    // normalize the input parameters
    if (box_x < 0)
        box_x *= -1;
    if (box_y < 0)
        box_y *= -1;
    if (box_width < 0)
        box_width *= -1;
    if (box_height < 0)
        box_height *= -1;
    if (line_size < 0)
        line_size *= -1;

    if (box_x % 2)
        box_x -= 1;
    if (box_width % 2)
        box_width -= 1;
    if (box_y % 2)
        box_y -= 1;
    if (box_height % 2)
        box_height -= 1;

    if (line_size % 2)
    {
        line_size += 1;
    }

    // if line size is too big, correct it automatically
    if (line_size * 2 > box_height)
    {
        line_size = box_height / 2;
    }

    if (line_size * 2 > box_width)
    {
        line_size = box_width / 2;
    }

    if ((vid_height != 0) && (vid_width != 0) && (box_height != 0) && (box_width != 0)
        && (line_size != 0))
    {
        // change the Y component of YUV color space
        for (i = 0; i < line_size; i++) {
            memset(frame->vi_frame.data + (box_y + i) * vid_width + box_x,
                yuv_y, box_width);
        }

        for (i = 0; i < line_size; i++) {
            memset(frame->vi_frame.data + (box_y + box_height - 1 - i) * vid_width + box_x,
                yuv_y, box_width);
        }

        for (i = 0; i < line_size; i++)  // code_updating
        {
            for (j = 0; j < box_height; j++)
            {
                frame->vi_frame.data[(box_y + j) * vid_width + box_x + i] = yuv_y;
                frame->vi_frame.data[(box_y + j) * vid_width + box_x + box_width - 1 - i] = yuv_y;
            }
        }

        // change the U and V component of YUV color space
        for (i = 0; i < line_size; i = i + 2)
        {
            for (j = 0; j < box_width; j++)
            {
                if (j % 2)
                {
                    frame->vi_frame.data[vid_height * vid_width +
                        (box_y + i) / 2 * vid_width + box_x + j] = yuv_v;
                    frame->vi_frame.data[vid_height * vid_width +
                        (box_y + box_height - 1 - i) / 2 * vid_width + box_x + j] = yuv_v;
                }
                else
                {
                    frame->vi_frame.data[vid_height * vid_width +
                        (box_y + i) / 2 * vid_width + box_x + j] = yuv_u;
                    frame->vi_frame.data[vid_height * vid_width +
                        (box_y + box_height - 1 - i) / 2 * vid_width + box_x + j] = yuv_u;
                }
            }
        }

        for (i = 0; i < line_size; i++)
        {
            for (j = 0; j < box_height; j = j + 2)
            {
                if (i % 2)
                {
                    frame->vi_frame.data[vid_height * vid_width +
                        (box_y + j) / 2 * vid_width + box_x + i] = yuv_v;
                    frame->vi_frame.data[vid_height * vid_width +
                        (box_y + j) / 2 * vid_width + box_x + box_width - 1 - i] = yuv_u;
                }
                else
                {
                    frame->vi_frame.data[vid_height * vid_width +
                        (box_y + j) / 2 * vid_width + box_x + i] = yuv_u;
                    frame->vi_frame.data[vid_height * vid_width +
                        (box_y + j) / 2 * vid_width + box_x + box_width - 1 - i] = yuv_v;
                }
            }
        }
        return 0;
    }
    else
    {
        return -1;
    }
}
/* end of func */

/**
 * set_box_rect - set box rect info
 * @x[IN]: left of rect
 * @y[IN]: top of rect
 * @width[IN]: width of rect
 * @height[IN]: height of rect
 * return: 0 success, -1 failed
 * notes:
 */
int set_box_rect(int x, int y, int width, int height)
{
    ak_thread_mutex_lock(&box_lock);
    box_rect.left = x;
    box_rect.top = y;
    box_rect.width = width;
    box_rect.height = height;
    ak_thread_mutex_unlock(&box_lock);

    return 0;
}



/* get the encode data from file path */
void *video_encode_func(void *arg)
{
    char save_file[128] = {0};
    struct ak_date date;
    char time_str[32] = {0};

    ak_thread_set_name("venc_th");

    /* a dir opt */
    FILE *fp_venc = NULL;
    FILE *fp_md = NULL;
    struct video_input_frame frame;
    unsigned char *p = NULL;
    unsigned short *st = NULL;
    int j = 0;

    ak_print_normal(MODULE_ID_APP, "capture start\n");

    if (access(save_path, W_OK) != 0)
    {
        if (mkdir(save_path, 0755))
        {
            ak_print_error_ex(MODULE_ID_APP, "mkdir: %s failed\n", save_path);
            return NULL;
        }
    }

    /* init the save file name and open it */
    memset(save_file, 0x00, sizeof(save_file));
    memset(time_str, 0, 32);

    /* get time string */
    ak_get_localdate(&date);
    ak_date_to_string(&date, time_str);

    sprintf(save_file, "%s/mt_%s_h264_chn%d.str", save_path, time_str, 0);


    /* save venc file open */
    fp_venc = fopen(save_file, "w+");

    sprintf(save_file, "%s/mt_info_%s.txt", save_path, time_str);


    /* save venc file open */
    fp_md = fopen(save_file, "w+");

    /* frame num cal */
    while(venc_run)
    {
        memset(&frame, 0x00, sizeof(frame));

        /* to get frame */
        int ret = ak_vi_get_frame(VIDEO_CHN0, &frame);
        if (!ret)
        {
            ak_thread_mutex_lock(&box_lock);
            draw_box(&frame, 255, 255, 255, MAIN_HEIGHT, MAIN_WIDTH,
                box_rect.left, box_rect.top,
                box_rect.height, box_rect.width, 4);
            box_rect.left = 0;
            box_rect.top = 0;
            box_rect.height = 0;
            box_rect.width = 0;
            ak_thread_mutex_unlock(&box_lock);

            p = (unsigned char *)frame.mdinfo;
            st = (void *)(p+128);

            fprintf(fp_md,"md info start!\n");
            for (j=0; j<32*24; j++)
            {
                fprintf(fp_md, "%d\n", st[j]);
            }
            fprintf(fp_md,"md info end!\n");


            /* send it to encode */
            struct video_stream *stream = ak_mem_alloc(MODULE_ID_APP, sizeof(struct video_stream));
            ret = ak_venc_encode_frame(venc_handle, frame.vi_frame.data, frame.vi_frame.len, frame.mdinfo, stream);
            if (ret)
            {
                /* send to encode failed */
                ak_print_error_ex(MODULE_ID_APP, "send to encode failed\n");
            }
            else
            {
                fwrite(stream->data, stream->len, 1, fp_venc);
                ak_venc_release_stream(venc_handle, stream);
            }

            ak_mem_free(stream);
            ak_vi_release_frame(VIDEO_CHN0, &frame);
        }
        else
        {
            /*
             *	If getting too fast, it will have no data,
             *	just take breath.
             */
            ak_print_normal_ex(MODULE_ID_APP, "get frame failed!\n");
            ak_sleep_ms(10);
        }

    }

    /* finished */
    fclose(fp_venc);
    fclose(fp_md);

    return NULL;
}

/**
 * brief: start venc
 * @void
 * return: 0 success, -1 failed;
 * notes:
 */
static int start_venc(void)
{
    /* open venc */
    struct venc_param ve_param = {0};
    int ret = -1;

    ve_param.width  = MAIN_WIDTH;            //resolution width
    ve_param.height = MAIN_HEIGHT;           //resolution height
    ve_param.fps    = 20;               //fps set
    ve_param.goplen = 50;               //gop set
    ve_param.target_kbps = 800;         //k bps
    ve_param.max_kbps    = 1024;        //max kbps
    ve_param.br_mode     = BR_MODE_CBR; //br mode
    ve_param.minqp       = 25;          //qp set
    ve_param.maxqp       = 50;          //qp max value
    ve_param.initqp       = (ve_param.minqp + ve_param.maxqp)/2;    //qp value
    ve_param.jpeg_qlevel = JPEG_QLEVEL_DEFAULT;     //jpeg qlevel
    ve_param.chroma_mode = CHROMA_4_2_0;            //chroma mode
    ve_param.max_picture_size = 0;                  //0 means default
    ve_param.enc_level        = 30;                 //enc level
    ve_param.smart_mode       = 0;                  //smart mode set
    ve_param.smart_goplen     = 100;                //smart mode value
    ve_param.smart_quality    = 50;                 //quality
    ve_param.smart_static_value = 0;                //value
    ve_param.enc_out_type = H264_ENC_TYPE;          //enc type
    ve_param.profile     = PROFILE_MAIN;


    ret = ak_venc_open(&ve_param, &venc_handle);

    if (ret || (-1 == venc_handle) )
    {
        ak_print_error_ex(MODULE_ID_APP, "open venc failed\n");
        return -1;
    }

    venc_run = 1;
    /* create the venc thread */
    ak_thread_create(&venc_stream_th, video_encode_func, NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);

    return 0;
}

/**
 * brief: stop venc
 * @void
 * return: void;
 * notes:
 */
static void stop_venc(void)
{
    venc_run = 0;
    ak_thread_join(venc_stream_th);

    if(venc_handle != -1)
    {
        /* close the venc*/
        ak_venc_close(venc_handle);
    }
}

/**
 * brief: mt main fuction
 * @void
 * return: 0 success, -1 failed;
 * notes:
 */
static int mt_test_main(void)
{
    int ret = AK_FAILED;
    struct ak_timeval start_tv;
    struct ak_timeval end_tv;
    void *mrd_handle = NULL;
    struct vpss_md_info md = {{{0}}};
    MRD_RECTANGLE boxes[MAX_CAP];
    int mv_cnt = 0;
    int mt_ret = 0;
    int step = 0;
    MT_POINT rt_angle_save = {0};
    struct motor_message msg = {0};

    ak_get_ostime(&start_tv);
    ak_print_normal(MODULE_ID_APP, "*** start to init mrd module.***\n");

    /*
    * step 1:mrd init
    */
    mrd_handle = ak_mrd_init();
    if (NULL == mrd_handle)
    {
        ak_print_error(MODULE_ID_APP, "mrd init fail\n");
        return ret;
    }

    /*
    * step 2:set mrd params
    */
    if(day_night_mode)   // set day mrd params
    {
        ak_mrd_set_mode(mrd_handle, 1); //single frame mode

        ak_mrd_set_floating_wadding_params(mrd_handle, 3, 2, 4, 36, 2, 3);

        if (AK_SUCCESS != ak_mrd_set_filters(mrd_handle, 12000, 6000))
        {
            ak_print_error(MODULE_ID_APP, "ak_mrd_set_filters fail\n");
            goto EXIT;
        }

        if (AK_SUCCESS != ak_mrd_set_motion_region_size(mrd_handle, 10, 300))
        {
            ak_print_error(MODULE_ID_APP, "ak_mrd_set_motion_region_size fail\n");
            goto EXIT;
        }

    }
    else    // set night mrd params
    {
        ak_mrd_set_mode(mrd_handle, 0);	

        ak_mrd_set_floating_wadding_params(mrd_handle, 4, 2, 4, 9, 2, 3);

        if (AK_SUCCESS != ak_mrd_set_filters(mrd_handle, 15000, 10000))
        {
            ak_print_error(MODULE_ID_APP, "ak_mrd_set_filters fail\n");
            goto EXIT;
        }

        if (AK_SUCCESS != ak_mrd_set_motion_region_size(mrd_handle, 10, 300))
        {
            ak_print_error(MODULE_ID_APP, "ak_mrd_set_motion_region_size fail\n");
            goto EXIT;
        }

    }
 

    /*
    * step 3:mt init
    */
    mt_handle = ak_mt_init();

    if (NULL == mt_handle)
    {
        ak_print_error(MODULE_ID_APP, "ak_mt_init fail\n");
        goto EXIT;
    }

    /*
    * step 4:mt params set
    */
    ak_mt_set_motion_region_params(mt_handle, 24, 10, 300, 8, 3);
    ak_mt_set_field_angles(mt_handle, 90, 60, 32, 24);
    ak_mt_set_rotation_range(mt_handle, 2, 65, 2, 65);
    ak_mt_set_max_decision_frames(mt_handle, 5);
    ak_mt_set_invalid_distance(mt_handle, 1);
    ak_mt_open_camera_rotation_switch_v(mt_handle);
    ak_mt_set_fps(mt_handle, 15);
    ak_mt_set_max_invalid_frames(mt_handle, 150);


    ioctl(motor_fd[0], MOTOR_GET_STATUS, &msg);
    rt_angle_save.px = motor_step2angle(msg.pos);
    ioctl(motor_fd[1], MOTOR_GET_STATUS, &msg);
    rt_angle_save.py = motor_step2angle(msg.pos);
    ak_mt_reset_tracking_param(mt_handle);

    /*
    * step 5:run
    * you can create a new thread to do it
    */
    do {
        if (AK_SUCCESS != ak_vpss_md_get_stat(VIDEO_DEV0, &md))
        {
            ak_sleep_ms(40);
            continue;
        }

        memset(boxes, 0, MAX_CAP * sizeof(MRD_RECTANGLE));
        mv_cnt = ak_mrd(mrd_handle, md.stat, 24, boxes);

        if (mv_cnt > 0)
        {
            MT_RECTANGLE rect = {{0}};

            set_box_rect(boxes[0].left * MAIN_WIDTH / 32,
                    boxes[0].top * MAIN_HEIGHT / 24,
                    (boxes[0].right - boxes[0].left) * MAIN_WIDTH / 32,
                    (boxes[0].bottom - boxes[0].top) * MAIN_HEIGHT / 24);

            rect.pt.px = boxes[0].left;
            rect.pt.py = boxes[0].top;
            rect.width = boxes[0].right - boxes[0].left;
            rect.height = boxes[0].bottom - boxes[0].top;

            MT_POINT rt_angle = {0};


            mt_ret = ak_mt_tracking(mt_handle, 24, &rect, &rt_angle);

            if (mt_ret > 0)
            {
                //ptz need turn
                ak_print_normal(MODULE_ID_APP, "***need turn %d, %d ***\n",
                    rt_angle.px, rt_angle.py);

                //rotate h
                step = motor_angle2step(0 - rt_angle.px);   //(rt_angle.px) or (0 - rt_angle.px) according to the device
                ioctl(motor_fd[0], MOTOR_MOVE_LIMIT, &step);
                ioctl(motor_fd[0], MOTOR_GET_STATUS, &msg);
                rt_angle_save.px = motor_step2angle(msg.pos);


                //rotate v
                step = motor_angle2step(0 - rt_angle.py);   //(rt_angle.py) or (0 - rt_angle.py) according to the device
                ioctl(motor_fd[1], MOTOR_MOVE_LIMIT, &step);
                ioctl(motor_fd[1], MOTOR_GET_STATUS, &msg);
                rt_angle_save.py = motor_step2angle(msg.pos);

                ak_sleep_ms(200);

                ak_mt_reset_tracking_param(mt_handle);
            }
        }

        ak_get_ostime(&end_tv);
        /* run time is over and exit */
        if (ak_diff_ms_time(&end_tv, &start_tv) > (run_time * 1000))
            break;
        else
            ak_sleep_ms(40);

    } while(1);

    ret = AK_SUCCESS;

EXIT:
    /*
    * step 6:mrd destroy
    */
    ak_mrd_destroy(mrd_handle);
    if (mt_handle)
        ak_mt_destroy(mt_handle);
    ak_print_normal(MODULE_ID_APP, "*** mt exit ***\n");

    return ret;
}
/* end of func */

/**
 * Preconditions:
 * your main video progress must stop
 */
int main(int argc, char **argv)
{
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    if (0 == parse_option(argc, argv))
    {
        return 0;
    }

    ak_print_normal(MODULE_ID_APP, "mt test start.\n");

    /*
        * step 1: start vi and venc
        */
    if (start_vi())
        goto exit;

    if (start_venc())
        goto exit;

    /*
        * step 2: open motor
        */
    if (motor_open())
        goto exit;

    /*
        * step 3: mt test main function
        */
    mt_test_main();

exit:
    /*
     * step 4: release resource
     */

    stop_venc();
    stop_vi();
    motor_close();

    ak_sdk_exit();
    ak_print_normal(MODULE_ID_APP, "exit mt sample\n");

    return 0;
}

