/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_svp_sample.c
* Description: This is a simple example to show how the SVP module working.
* Notes:
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
#include "ak_vpss.h"
#include "ak_mem.h"
#include "ak_log.h"
#include "ak_svp2.h"
#include "ak_npu.h"
#include "ak_svp_md.h"
#include "ak_object_filter.h"

#ifdef AK_RTOS
#include <kernel.h>
#endif

#define LEN_HINT         512
#define LEN_OPTION_SHORT 512

#define DEF_FRAME_DEPTH		2

static int frame_num = 200;
static int channel_num = 0;
static char *isp_path = "";
static char *svp_path = "";
static VI_CHN_ATTR_EX chn_attr = {0};
static VI_CHN_ATTR_EX chn_attr_sub = {0};
static VI_CHN_ATTR_EX chn_attr_td = {0};
static int svp_mode = 0;
static int svp_data_type = 1;
static int svp_chn_id = 0;
static int svp_rate = 3;
static int filter_enable = 0;
/*
* percentage of overlapped area for svp threshold, valid range [1-100],
* the less, the more sensitive, but the less accurate, default 7,means 7%
*/
static int svp_thrld = 7;
/*
* percentage of overlapped area for md threshold, valid range [1-100],
* the less, the more sensitive, but the less accurate, default 40, means 40%
*/
static int svp_md_thrld = 40;

typedef struct BOX
{
    int x1;
    int y1;
    int x2;
    int y2;
}BOX;

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
    "			打印帮助信息" ,
    "[NUM]		采样帧数[0 1000]" ,
    "[NUM]		检测帧率调节[1 30]，注意指的是每多少帧检测一帧!" ,
    "[PATH]		ISP config file 保存路径, 默认为空,需要填写" ,
    "[PATH]		SVP config file 保存路径, 默认为空,需要填写" ,
    "[NUM]      人形人脸检测模型，0：320*180",
    "[NUM]      人形人脸检测数据类型，0：YUV，1：RGB",
    "[NUM]      人形滤波功能(filter+md)，0：disable(only svp)，1：svp+md，2: svp+filter+md"
};

static struct option option_long[ ] = {
    { "help"            , no_argument       , NULL , 'h' } , //"       打印帮助信息" ,
    { "NUM"             , required_argument , NULL , 'n' } , //"		[NUM]  采集帧数" ,
    { "svp_rate"        , required_argument , NULL , 'g' } , //"		[NUM]  SVP检测帧率" ,
    { "isp_cfg"         , required_argument , NULL , 'f' } , //"		[CONFIG] ISP config file 路径" ,
    { "svp_cfg"         , required_argument , NULL , 'c' } , //"		[CONFIG] SVP config file 路径" ,
    { "svp_mode"        , required_argument , NULL , 'm' } , //"		[NUM]  SVP检测模型" ,
    { "svp_data_type"   , required_argument , NULL , 'd' } , //"		[NUM]  SVP数据类型" ,
    { "filter_enable"   , required_argument , NULL , 'e' } , //"		[NUM]  MD+Filter辅助检测功能" ,

    {NULL    , 0, 0, 0}
};

static void usage(const char * name)
{

    printf(" %s -n [frame_num] -f [isp_config_path] -c [svp_config_path] -m [svp_mode] -d [svp_data_type]\n", name);
    printf("\t[frame_num]: 	 		 default is 200 \n");
    printf("\t[svp_rate]: 	 		 default is 3 \n");
    printf("\t[isp_config_path]: 	 default is null \n");
    printf("\t[svp_config_path]: 	 default is null \n");
    printf("\t[svp_mode]: 	 		 default is 0 \n");
    printf("\t[svp_data_type]: 	 	 default is 1 \n");
    printf("\t[filter_enable]: 	 	 	 default is 0 \n");
    printf("eg: %s -n 1000 -g 3 -f isp_gc4653_mipi_2lane.conf -c /usr/sbin/dt320_param.bin -m 0 -d 1 -e 0\n", name);
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
        case 'h' :                                                          //help
            help_hint( argv[ 0 ] );
            c_flag = 0;
            goto parse_option_end;
        case 'n' :                                                          //frame count
            frame_num = atoi( optarg );
            break;
        case 'g' :                                                          //frame count
            svp_rate = atoi( optarg );
            break;
        case 'f' :                                                          //path
            isp_path = optarg;
            break;
        case 'c' :                                                          //path
            svp_path = optarg;
            break;
        case 'm' :
            svp_mode = atoi(optarg);
            break;
        case 'd' :
            svp_data_type = atoi(optarg);
            break;
        case 'e' :
            filter_enable = atoi(optarg);
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
 * * @BRIEF  计算两个矩形框的交集分针占源BOX的百分比
 * * @PARAM  a: one box
 * * @PARAM  b: one box
 * * @PARAM  r_IOUa: IoU ratio of Box a
 * * @PARAM  r_IOUb: IoU ratio of Box b
 * * @NOTE:
 * */
void  compute_iou2(BOX a, BOX b, double *r_IOUa, double *r_IOUb)
{
    int maxX = 0;
    int maxY = 0;
    int minX = 0;
    int minY = 0;
    double IOU_I;
    double A, B;
    double IOUA,IOUB;
    //ak_print_normal_ex(MODULE_ID_APP, "A[%d,%d,%d,%d],B[%d,%d,%d,%d]\n", a.x1, a.y1, a.x2, a.y2, b.x1, b.y1, b.x2, b.y2);
    maxX = (a.x1 > b.x1) ? a.x1 : b.x1;
    maxY = (a.y1 > b.y1) ? a.y1 : b.y1;
    minX = (a.x2 < b.x2) ? a.x2 : b.x2;
    minY = (a.y2 < b.y2) ? a.y2 : b.y2;

    maxX = ((minX - maxX)>0) ? (minX - maxX) : 0;
    maxY = ((minY - maxY)>0) ? (minY - maxY) : 0;

    IOU_I = maxX * maxY;
    A = (a.x2 - a.x1)*(a.y2 - a.y1);
    B = (b.x2 - b.x1)*(b.y2 - b.y1);

    //ak_print_normal_ex(MODULE_ID_APP, "IOU_I[%lf], A[%lf],B[%lf]\n",IOU_I, A,B);
    IOUA = IOU_I / A;
    IOUB = IOU_I / B;

    *r_IOUa = IOUA;
    *r_IOUb = IOUB;
}

static int start_vi(int dev_id)
{
    /*
     * step 0: global value initialize
     */
    int ret = -1;                                //return value
    int width = 1920;
    int height = 1080;
    int subwidth = 640;
    int subheight = 360;
    int thdwidth = 320;
    int thdheight = 180;

    int crop_width = width;
    int crop_height = height;

    int chn_main_id = VIDEO_CHN0;
    int chn_sub_id = VIDEO_CHN1;
    int chn_trd_id = VIDEO_CHN16;
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
        ak_print_error_ex(MODULE_ID_APP, "vi device %d load isp cfg [%s] failed!\n", dev_id, isp_path);
        return ret;
    }

    /*
     * step 3: get sensor support max resolution
     */
    RECTANGLE_S res;                //max sensor resolution
    VI_DEV_ATTR    dev_attr;
    memset(&dev_attr, 0, sizeof(VI_DEV_ATTR));
    dev_attr.dev_id = dev_id;
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
        ak_print_error_ex(MODULE_ID_APP, "Can't get dev[%d]resolution\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    } else {
        ak_print_normal_ex(MODULE_ID_APP, "get dev res w:[%d]h:[%d]\n",res.width, res.height);
        dev_attr.crop.width = res.width;
        dev_attr.crop.height = res.height;
        dev_attr.max_width = res.width;
        dev_attr.max_height = res.height;
        width = res.width;
        height = res.height;
    }

    /*
     * step 4: set vi device working parameters
     * default parameters: 25fps, day mode
     */
    ret = ak_vi_set_dev_attr(dev_id, &dev_attr);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device %d set device attribute failed!\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    }

    /*
     * step 5: set main channel attribute
     */

    memset(&chn_attr, 0, sizeof(VI_CHN_ATTR_EX));
    chn_attr.chn_id = chn_main_id;
    chn_attr.res.width = width;
    chn_attr.res.height = height;
    chn_attr.frame_depth = DEF_FRAME_DEPTH;
    /*disable frame control*/
    chn_attr.frame_rate = 0;
    chn_attr.data_type = VI_DATA_TYPE_YUV420SP;
    ret = ak_vi_set_chn_attr_ex(chn_main_id, &chn_attr);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device %d set channel [%d] \
        attribute failed!\n", dev_id, chn_main_id);
        ak_vi_close(dev_id);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP, "vi device %d main sub channel attribute\n", dev_id);

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
    chn_attr_sub.data_type = VI_DATA_TYPE_YUV420SP;
    ret = ak_vi_set_chn_attr_ex(chn_sub_id, &chn_attr_sub);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device %d set channel [%d] attribute failed!\n", dev_id, chn_sub_id);
        ak_vi_close(dev_id);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP, "vi device %d set sub channel attribute\n", dev_id);

    /*
     * step 7: set third channel attribute
     */
    if(svp_mode == 1)
    {
        thdwidth = 480;
        thdheight = 270;
    }else if(svp_mode == 2 || svp_mode == 3)
    {
        thdwidth = 640;
        thdheight = 360;
    }
    memset(&chn_attr_td, 0, sizeof(VI_CHN_ATTR_EX));
    chn_attr_td.chn_id = chn_trd_id;
    /* resolution of third channel must be half of the resolution of sub channel */
    chn_attr_td.res.width = thdwidth;
    chn_attr_td.res.height = thdheight;
    chn_attr_td.frame_depth = DEF_FRAME_DEPTH;
    /*disable frame control*/
    chn_attr_td.frame_rate = 0;
    /*set thd chn data mode*/
    if(svp_data_type == 0)
        chn_attr_td.data_type = VI_DATA_TYPE_YUV420SP;
    else
        chn_attr_td.data_type = VI_DATA_TYPE_RGB_LINEINTL;

    ret = ak_vi_set_chn_attr_ex(chn_trd_id, &chn_attr_td);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device %d set channel [%d] attribute failed!\n", dev_id, chn_trd_id);
        ak_vi_close(dev_id);
        return ret;
    }
    ak_print_normal_ex(MODULE_ID_APP, "vi device %d set td channel attribute\n", dev_id);
    /*
     * step 8: enable vi device
     */
    ret = ak_vi_enable_dev(dev_id);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, "vi device %d enable device  failed!\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    }
#if 1
    /*
     * step 9: enable vi main channel
     */
    ret = ak_vi_enable_chn(chn_main_id);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP, "vi channel[%d] enable failed!\n", chn_main_id);
        ak_vi_close(dev_id);
        return ret;
    }

    /*
     * step 10: enable vi sub channel
     */
    ret = ak_vi_enable_chn(chn_sub_id);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP, "vi channel[%d] enable failed!\n",chn_sub_id);
        ak_vi_close(dev_id);
        return ret;
    }
#endif
    /*
     * step 11: enable vi third channel
     */
    ret = ak_vi_enable_chn(chn_trd_id);
    if(ret)
    {
        ak_print_error_ex(MODULE_ID_APP, "vi channel[%d] enable failed!\n",chn_trd_id);
        ak_vi_close(dev_id);
        return ret;
    }
    return 0;
}
/* end of start_vi */

static int stop_vi(int dev_id)
{
    int ret = -1;
    int chn_main_id = VIDEO_CHN0;
    int chn_sub_id = VIDEO_CHN1;
    int chn_trd_id = VIDEO_CHN16;

    ak_vi_disable_chn(chn_main_id);

    ak_vi_disable_chn(chn_sub_id);
    ak_vi_disable_chn(chn_trd_id);
    ak_vi_disable_dev(dev_id);
    ret = ak_vi_close(dev_id);

    return ret;
}

static int start_svp(void)
{
    int ret = AK_FAILED;
    int model_num = 1;
    /* init the svp attr */
    int svp_len = sizeof(AK_SVP2_CHN_ATTR_T) + model_num*sizeof(AK_SVP2_MODEL_ATTR_T);
    AK_SVP2_CHN_ATTR_T *attr = (AK_SVP2_CHN_ATTR_T *)ak_mem_alloc(MODULE_ID_APP, svp_len);
    memset(attr, 0, svp_len);
    attr->model_num = model_num;

    if(svp_mode == 0)
        attr->model_attr->model_type = AK_SVP2_MODEL_HD_180P;
    else if(svp_mode == 2)
        attr->model_attr->model_type = AK_SVP2_MODEL_HD_360P;

    /* set svp classsify threshold, default is 700, range[400-900],
     * the less, the more sensitive, but the less accurate
     */
    attr->model_attr->model_param.param.hd_param.classify_threshold = 700;
    /* set the IoU threshold, range [3-10], default is 3*/
    attr->model_attr->model_param.param.hd_param.IoU_threshold = 3;

    attr->model_attr->model_param.param.hd_param.target_type = AK_SVP2_HUMAN_SHAPE_AND_FACE;

    AK_SVP2_CONF_INFO conf_info = {0};
    conf_info.conf_mode = 0;
    conf_info.conf_type.conf = svp_path;

    ret = ak_npu_init();
    if(ret != AK_SUCCESS)
    {
        ak_print_error_ex(MODULE_ID_APP, "Init SVP failed!\n");
        return ret;
    }

    ret = ak_svp2_create_chn(svp_chn_id, attr, &conf_info);
    if(ret != AK_SUCCESS)
    {
        ak_print_error_ex(MODULE_ID_APP, "Create SVP chn failed\n");
        ak_npu_deinit();
        return ret;
    }


    if(attr != NULL) {
        ak_mem_free(attr);
        attr = NULL;
    }
    return ret;
}

static void stop_svp(void)
{
    ak_svp2_destroy_chn(svp_chn_id);
    ak_npu_deinit();
    ak_print_normal(MODULE_ID_APP, "svp exit!\n");
}

static int start_md(int dev_id)
{
    int ret = 0;
    struct svp_md_param param = {0};
    /*init the md param, please note that, this param is only used for SVP correction*/
    param.dev_id = dev_id;
    /* set the md detect frame rate, refer to the frame rate of sensor*/
    ak_vpss_get_sensor_fps(VIDEO_DEV0, &param.md_fps);
    /* set the valid area range of md detect, range is [1-768]*/
    param.move_size_min = 2;
    param.move_size_max = 300;
    /* set the sensitivity param of md detect, range [1-65535], the less the more sensitive, but the less accurate */
    param.flt_big = 1500;
    param.flt_small = 1500;

    /* init svp md function */
    ret = ak_svp_md_init(&param);
    if(ret != AK_SUCCESS)
    {
        ak_print_error_ex(MODULE_ID_APP, "SVP init md failed!\n");
        return ret;
    }

    /* enable the svp md function */
    ret = ak_svp_md_enable(dev_id, 1);
    if(ret != AK_SUCCESS)
    {
        ak_print_error_ex(MODULE_ID_APP, "MD enable failed!\n");
        ak_svp_md_uninit(dev_id);
        return ret;
    }

    return ret;
}

static void stop_md(int dev_id)
{
    ak_svp_md_uninit(dev_id);
}

/*
 * svp_loop: loop to get and release frame from third chn and do svp, between get and release,
 *                  here we just save the frame to file, on your platform,
 *                  you can rewrite the save_function with your code.
 * number[IN]: save numbers
 * attr[IN]:   vi channel attribute.
 */
static void svp_loop(int number, VI_CHN_ATTR_EX *chn_attr)
{
    int count = 0;
    struct video_input_frame frame;
    AK_SVP2_IMG_INFO_T input = {0};

    /* fill in input structure info */
    input.img_type = (chn_attr->data_type == VI_DATA_TYPE_YUV420SP ? AK_SVP2_IMG_YUV420SP : AK_SVP2_IMG_RGB_LI);
    input.width = chn_attr->res.width;
    input.height = chn_attr->res.height;
    input.pos_info.left = 0;
    input.pos_info.top = 0;
    input.pos_info.width = input.width;
    input.pos_info.height = input.height;
    unsigned long vi_cnt = 0;

    void *filter_handle = NULL;
    OBJECT_BOX *obj_box = NULL;
    if(filter_enable == 2)
    {
        filter_handle = ak_object_filter_init();
        ak_object_filter_set_frames(filter_handle, 3, 2, 4);
        ak_object_filter_set_distance_enhancement_params(filter_handle, 2, 8, 6);
        ak_object_filter_set_md_level(filter_handle, 2);
        ak_object_filter_set_continous_enhancement_params(filter_handle, 1, 5);
        ak_object_filter_set_false_record_params(filter_handle, 1, 7);
        ak_print_notice(MODULE_ID_APP, "libfilter version = %s\r\n", ak_object_filter_get_version());

        obj_box = ak_mem_alloc(MODULE_ID_APP, sizeof(OBJECT_BOX) * OBJECT_CAP);
        memset(obj_box, 0, sizeof(OBJECT_BOX) * OBJECT_CAP);
    }

    ak_print_normal(MODULE_ID_APP, "SVP Process start\n");
    /*
     * To get frame by loop
     */
    while (count <  number)
    {
        memset(&frame, 0x00, sizeof(frame));

        /* to get frame according to the channel number */
        int ret = ak_vi_get_frame(chn_attr->chn_id, &frame);

        if (!ret)
        {
            if(svp_rate != 0 && ++vi_cnt % svp_rate == 0)
            {
                input.vir_addr = frame.vi_frame.data;
                input.phy_addr = frame.phyaddr;

                int total_num = 0;
                AK_SVP2_OUTPUT_T output = {0};
                /* invoke the svp process to do detection */
                ret = ak_svp2_process(svp_chn_id, &input, (void *)&output);
                if(ret == AK_SUCCESS)
                {
                    total_num = output.result.hd_result.total_num;
                    if(total_num > 0)
                    {
                        int i = 0;

                        /* printf the svp process result base on the detect chn resolution, the third chn, chn16*/
                        for(;i<total_num; i++)
                        {
                            AK_SVP2_RECT_T *src = &output.result.hd_result.target_boxes[i];
                            if(filter_enable == 2)
                            {
                                obj_box[i].left		= src->left;
                                obj_box[i].top		= src->top;
                                obj_box[i].right	= src->right;
                                obj_box[i].bottom	= src->bottom;
                                obj_box[i].count	= 1;
                                obj_box[i].md		= 0;
                            }
                            /* user can invode ak_vi_draw_box api to mark the
                            human position, please notes: user need to convert
                            the position from svp chn to the chn that you mark*/
                            ak_print_normal(MODULE_ID_APP, "SVP [%d] :[%s] detect, \
                            score[%lu],Res is:left[%lu],top[%lu],right[%lu]:bottom[%lu]\n",
                            i, src->label ==  AK_SVP2_HUMAN_SHAPE ?"Body":"Face",
                            src->score, src->left, src->top, src->right, src->bottom);
                        }

                        /* if md is enable, used md result to help to do the correction,
                        and will increase the accuracy of the mobile humam detection*/
                        if(filter_enable != 0)
                        {
                            struct md_result_info ret_info = {0};
                            /* get the md result */
                            if (AK_SUCCESS == ak_svp_md_get_result(VIDEO_DEV0, &ret_info, 0))
                            {
                                /* get md result success and check the valid md boxes */
                                if(ret_info.result > 0)
                                {
                                    int j = 0;
                                    /* go through each svp info*/
                                    for(j = 0; j< total_num; j++)
                                    {
                                        AK_SVP2_RECT_T *src = &output.result.hd_result.target_boxes[j];
                                        BOX svp = {0};
                                        svp.x1 = src->left;
                                        svp.y1 = src->top;
                                        svp.x2 = src->right;
                                        svp.y2 = src->bottom;
                                        /* go throuth each md box to do the correction*/
                                        for(i = 0; i< ret_info.move_box_num; i++)
                                        {
                                            BOX md = {0};
                                            /* convert the MD result to MD box , according to the chn resolution that SVP used*/
                                            md.x1 = ret_info.boxes[i].left * chn_attr->res.width/32;
                                            md.y1 = ret_info.boxes[i].top * chn_attr->res.height/24;
                                            md.x2 = (ret_info.boxes[i].right == ret_info.boxes[i].left ?
                                                    ret_info.boxes[i].right + 1:ret_info.boxes[i].right) * 
                                                    chn_attr->res.width/32 ;
                                            md.y2 = (ret_info.boxes[i].bottom == ret_info.boxes[i].top ? 
                                                    ret_info.boxes[i].bottom + 1:ret_info.boxes[i].bottom) * 
                                                    chn_attr->res.height/24;
                                            double IoU_md=0,IoU_svp = 0;
                                            /*calc the MD box and SVP box IOU value*/
                                               compute_iou2(md,svp, &IoU_md, &IoU_svp);
                                            //ak_print_normal_ex(MODULE_ID_APP,"IoU_MD[%lf],Iou_SVP[%lf]\n", IoU_md, IoU_svp);

                                            /* it the percentage of the overlapped area for MD and SVP are over the threshold,mobile human detect */
                                            if(IoU_svp * 100 >= svp_thrld && IoU_md*100 >= svp_md_thrld)
                                            {
                                                ak_print_normal(MODULE_ID_APP, "SVP Alarm[%d] :Mobile [%s] detect, \
                                                        score[%lu],Res is:left[%lu],top[%lu],right[%lu]:bottom[%lu]\n",
                                                        j, src->label ==  AK_SVP2_HUMAN_SHAPE ?"Body":"Face",src->score,
                                                        src->left, src->top, src->right, src->bottom);
                                                if(filter_enable == 2)
                                                {
                                                    obj_box[j].md = 1;
                                                }
                                                break;
                                            }
                                        }
                                    }
                                }
                                if(filter_enable == 2)
                                {
                                    if(ak_object_filter_alarm(filter_handle, obj_box, total_num) != 0)
                                    {
                                        for(int j = 0; j < total_num; j++)
                                        {
                                            AK_SVP2_RECT_T *src = &output.result.hd_result.target_boxes[j];
                                            ak_print_normal(MODULE_ID_APP, "SVP Filter Alarm[%d] :Mobile [%s] detect, \
                                                    score[%lu],Res is:left[%lu],top[%lu],right[%lu]:bottom[%lu]\n",
                                                    j, src->label ==  AK_SVP2_HUMAN_SHAPE ?"Body":"Face",src->score,
                                                    src->left, src->top, src->right, src->bottom);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    /* release svp result*/
                    ret = ak_svp2_release(&output);

                    if(ret != AK_SUCCESS)
                    {
                        ak_print_error_ex(MODULE_ID_APP, "SVP rlease output failed!\n");
                    }
                }
                    /*
                     * in this context, this frame was useless,
                     * release frame data
                     */
                count++;
            }
            ak_vi_release_frame(chn_attr->chn_id, &frame);
        }
        else
        {
            /*
            *    If getting too fast, it will have no data,
            *    just take breath.
            */
            ak_print_normal_ex(MODULE_ID_APP, "get frame failed!\n");
            ak_sleep_ms(10);
        }

    }

    if(filter_enable == 2)
    {
        if(obj_box != NULL)
        {
            ak_mem_free(obj_box);
            obj_box = NULL;
        }
        ak_object_filter_destroy(filter_handle);
    }

    ak_print_normal(MODULE_ID_APP,"capture finish\n\n");
}
/* end of svp_loop */

/**
 * Preconditions:
 * 1??TF card is already mounted
 * 2??yuv_data is already created in /mnt
 * 3??ircut is already opened at day mode
 * 4??your main video progress must stop
 */
#ifdef AK_RTOS
static int svp_sample(int argc, char **argv)
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

    ak_print_normal(MODULE_ID_APP, "*****************************************\n");
    ak_print_normal(MODULE_ID_APP, "** svp demo version: %s **\n", ak_svp2_get_version());
    ak_print_normal(MODULE_ID_APP, "*****************************************\n");

    channel_num = VIDEO_CHN0;

    if( parse_option( argc, argv ) == 0 )
    {
        return 0;
    }
    /*check param validate*/
    if( svp_rate > 30 || svp_rate <= 0 || frame_num < 0 || frame_num > 1000
    || (svp_mode != 0 && svp_mode != 2)
    || (svp_data_type != 0 && svp_data_type != 1)
    || (filter_enable != 0 && filter_enable != 1 && filter_enable != 2))
    {
        ak_print_error_ex(MODULE_ID_APP,"INPUT param error!\n");
        help_hint( argv[ 0 ] );
        return 0;
    }

    if (strlen(isp_path) == 0 || strlen(svp_path) == 0)
    {
        ak_print_error_ex(MODULE_ID_APP,"INPUT param error!\n");
        help_hint( argv[ 0 ] );
        return 0;
    }

    int ret = -1;

    /*
     * step 1: start vi
     */
    ret = start_vi(VIDEO_DEV0);
    if (ret)
    {
        goto exit;
    }

    /*
     * step 2:start svp
     */
    ret = start_svp();
    if (ret)
    {
        goto exit_vi;
    }

    /*
     * step 3: if filter_enable, start md
     */
    if(filter_enable != 0)
    {
        ret = start_md(VIDEO_DEV0);
        if(ret)
        {
            goto exit_svp;
        }
    }

    /*
     * step 4: start to do the svp detection
     */
    svp_loop(frame_num, &chn_attr_td);


    /*
     *step 5: release md resource
     */
    if(filter_enable != 0)
    {
        stop_md(VIDEO_DEV0);
    }
exit_svp:
    /*
     *step 6: release svp resource
     */
    stop_svp();
exit_vi:
    /*
     * step 7: release vi resource
     */
    stop_vi(VIDEO_DEV0);

exit:

    ak_sdk_exit();
    ak_print_normal(MODULE_ID_APP, "exit vi demo\n");
    return ret;
}
/* end of main */

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(svp_sample, ak_svp2_sample, svp Sample Program);
#endif

/* end of file */
