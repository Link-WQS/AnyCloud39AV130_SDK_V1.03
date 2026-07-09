/**
 * @file ak_fast_aov_sample.c
 * @author yang_jiande (yang_jiande@anyka.oa)
 * @brief 快启AOV功能展示
 * @version 0.1
 * @date 2024-09-30
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/statvfs.h>
#include "ak_common.h"
#include "ak_log.h"
#include "ak_common_video.h"
#include "ak_venc.h"
#include "ak_thread.h"
#include "ak_mem.h"
#include "ak_vi.h"
#include "ak_video.h"
#include "ak_vpss.h"
#include "ak_svp2.h"
#include "ak_npu.h"
#include "isp_quick_ioctl.h"

#define RUN_FAST		0
#define RUN_FAST_AOV	1
#define THREAD_PRIO             (-1)    //线程优先级配置
//统计上电次数
#define NUM_FILE                "/tmp/sd/num.txt"
/*快启设备节点*/
#define QS_DEV_NODE		"/dev/quick_start"

#define LEN_HINT                512
#define LEN_OPTION_SHORT        512

#define SAMPLE_MAX_DEV_NUM		4
#define SAMPLE_MAX_VENC_NUM		4
#define SAMPLE_MAX_VIDEO_NUM	4
#define SAMPLE_MAX_NPU_NUM	    4 
#define SAMPLE_MAX_STITCH_GROUP 4
#define MAX_FILE_LEN            8*1024*1024 
//#define DEBUG

/*拼接属性结构体*/
struct stitch_attr_s{
    int stitch_mode;
    int stitch_chn_id;
    int stitch_num;
    int stitch_bind_chn[MAX_STITCH_BIND_CHN_NUM];
};

// venc参数结构体
struct video_attr_s
{
    int chn_id;
    int venc_id;
    ak_pthread_t pid;
    int th_run_flag;	
    int frame_rate;
    int frame_depth;
    int mode;
};

// 添加 SVP 相关的全局变量
static int dma_create_flag[4] = {0,0,0,0};  // 支持最多4路
static int svp_run_flag[4] = {0,0,0,0};
static ak_pthread_t svp_run_thread_id[4] = {0,0,0,0};
static int svp_chn_id[4] = {0,1,2,3};

static char *pc_prog_name = NULL;                      //demo名称
int  tf_write_enable            = 1; //写T卡标志
int  stream_save_flag   = 1; 		//快启道码流保存
int  fast_mode            = RUN_FAST;  //RUN_FAST： 只跑快启； RUN_FAST_AOV： 跑快启+aov
int fast_stream_cnt		= 200;
int aov_frame_cnt   = 30;
int aov_sleep_time  = 1000;
/*CIS计数*/
int CIS_cnt = 0;
/*编码器计数*/
int VENC_cnt = 0;
/*流式编码计数*/
int VIDEO_cnt = 0;
/*激活的流式编码计数*/
int VIDEO_enable_cnt=0;
/*拼接配置计数*/
int STITCH_cnt = 0;
/*NPU 计数*/
int NPU_cnt = 0;
/* quick start configuration for VI dev*/
qs_vi_ioctl_info_t vi_info[SAMPLE_MAX_DEV_NUM]={{0},{0},{0},{0}};
/* quick start configuration for VENC*/
qs_venc_ioctl_info_t venc_info_s[SAMPLE_MAX_VENC_NUM]={{0},{0},{0},{0}};
/* quick start configuration for VIDEO*/
qs_video_ioctl_info_t video_info[SAMPLE_MAX_VIDEO_NUM]={{0},{0},{0},{0}};
/* quick start configuration for VIDEO*/
qs_npu_ioctl_info_t npu_info[SAMPLE_MAX_NPU_NUM]={{0},{0},{0},{0}};

struct stitch_attr_s g_stitch_attr[SAMPLE_MAX_STITCH_GROUP]={{0},{0}, {0}, {0}};
struct video_attr_s g_video_attr[SAMPLE_MAX_VIDEO_NUM]={{0},{0},{0},{0}};

static char *save_path    = "/mnt";
int g_qs_fd = -1;



// VI通道工作信息结构体
struct vi_chn_svp_work_data {
    int dev_id;
    int vi_chn_id;
    int get_frame_chn_id;  // 取帧用的通道ID
    int npu_chn_ids[SAMPLE_MAX_NPU_NUM];  // 绑定的NPU通道ID数组
    int npu_count;         // 绑定的NPU通道数量
    int enabled;           // 是否启用
    ak_pthread_t thread_id; // 线程ID
    int run_flag;          // 运行标志
};

// 全局变量
static struct vi_chn_svp_work_data vi_svp_work_info[SAMPLE_MAX_DEV_NUM][3] = {{{0}}}; // 最多3个通道
static int vi_svp_work_count = 0;

static ak_pthread_t venc_stream_th[2][3] = {{0}};

static int file_finish_cnt = 0;
static int qs_change_flag = 0;
static int qs_fastae_change_flag = 0;
static int qs_night_mode_change_flag = 0;
static int qs_mirror_change_flag = 0;
static int qs_flip_change_flag = 0;

/* this is the message to print */
static char ac_option_hint[  ][ LEN_HINT ] = {             //操作提示数组
    "打印帮助信息" ,
    "[NUM] run fast or fast_aov, 0: fast, 1: fast_aov,  default 0" ,
    "[NUM] frame count that save in fast launch phase",
    "[NUM] aov sleep time,  default 1000 ms" ,
    "[NUM] aov frame num,  default 30" ,
    "[NUM] stream save flag, 0: not save, 1: save, default 1",
    "[NUM] quickstart config changed flag, 0: do not change, 1: change, default 0",
    "[NUM] quickstart fastae config changed flag, 0: do not change, 1: change, default 0",
    "[NUM] quickstart night mode config changed flag, 0: do not change, 1: change, default 0",
    "[NUM] quickstart mirror config changed flag, 0: do not change, 1: change, default 0",
    "[NUM] quickstart flip config changed flag, 0: do not change, 1: change, default 0",
    "",
};

/* opt for print the message */
static struct option option_long[ ] = {
    { "help"              , no_argument       , NULL , 'h' } ,      //"打印帮助信息" ,
    { "run_mode"     	, required_argument , NULL , 'm' } ,  //运行模式： fast or fast_aov
    { "frame_cnt"     , required_argument , NULL , 'c' } ,  //快启阶段保存的编码帧
    { "aov_sleep_time"     , required_argument , NULL , 'e' } ,     //aov mode 休眠时间 单位：ms
    { "aov_frame_cnt"     , required_argument , NULL , 'f' } ,      //aov 阶段保存的帧数
    { "stream_save"     , required_argument , NULL , 's' } ,      //录像保存flag，默认是1
    { "qs_change"     , required_argument , NULL , 'q' } ,      //quick start配置文件修改，默认是0
    { "qs_fastae_change"     , required_argument , NULL , 'a' } ,      //quick start fastae配置修改，默认是0
    { "qs_night_mode_change"     , required_argument , NULL , 'n' } ,      //quick start夜间模式配置修改，默认是0
    { "qs_mirror_change"     , required_argument , NULL , 'r' } ,      //quick start画面镜像配置修改，默认是0
    { "qs_flip_change"     , required_argument , NULL , 'p' } ,      //quick start画面翻转配置修改，默认是0
    { 0                   , 0                 , 0    , 0   } ,
};

/*
* get_option_short: fill the stort option string.
* return: option short string addr.
*/
static char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short, int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    /* get the option */
    for( i = 0 ; i < i_num_option ; i ++ )
    {
        if( ( c_option = p_option[ i ].val ) == 0 )
        {
            continue;
        }

        switch( p_option[ i ].has_arg )
        {
        case no_argument:
            /* if no argument, set the offset for default */
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c" , c_option );
            break;
        case required_argument:
            /* required argument offset calculate */
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c:" , c_option );
            break;
        case optional_argument:
            /* calculate the option offset */
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c::" , c_option );
            break;
        }
    }
    return pc_option_short;
}

static void usage(const char * name)
{
    printf("eg.: %s -m 0  -c 300 \n", name);
}

/* if opt is not supported, print the help message */
static int help_hint(void)
{
    int i;

    printf("%s\n" , pc_prog_name);
    /* parse the all supported option */
    for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++)
    {
        if( option_long[ i ].val != 0 )
            printf("\t--%-16s -%c %s\n" ,
                                                option_long[ i ].name , option_long[ i ].val , ac_option_hint[ i ]);
    }
    usage(pc_prog_name);
    printf("\n\n");
    return AK_SUCCESS;
}

/* parse the option from cmd line */
static int parse_option( int argc, char **argv )
{
    int i_option;

    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ; /* get the option num*/
    char c_flag = AK_TRUE;
    pc_prog_name = argv[ 0 ];   /* get the option */

    optind = 0;
    get_option_short( option_long, i_array_num , ac_option_short , LEN_OPTION_SHORT );  /* parse the cmd line input */
    while((i_option = getopt_long(argc , argv , ac_option_short , option_long , NULL)) > 0)
    {
        switch(i_option)
        {
        //help
        case 'h' :                                    
            help_hint();
            c_flag = AK_FALSE;
            goto parse_option_end;
        //run fast aov mdoe
        case 'm':
            fast_mode = atoi(optarg);       
            break;
        /*stream save count */
        case 'c':
            fast_stream_cnt = atoi(optarg);     
            break;
        /*sleep time interval*/
        case 'e':
            aov_sleep_time = atoi(optarg);         
            break;
        /* frame save cnt in aov mode */
        case 'f':
            aov_frame_cnt = atoi(optarg);          
            break;
        /*save flag */
        case 's':
            stream_save_flag = atoi(optarg);          
            break;
        case 'q':
            qs_change_flag = atoi(optarg);          
            break;
        case 'a':
            qs_fastae_change_flag = atoi(optarg);          
            break;
        case 'n':
            qs_night_mode_change_flag = atoi(optarg);          
            break;
        case 'r':
            qs_mirror_change_flag = atoi(optarg);          
            break;
        case 'p':
            qs_flip_change_flag = atoi(optarg);          
            break;
        default :
            help_hint();
            c_flag = AK_FALSE;
            goto parse_option_end;
        }
    }
parse_option_end:
    return c_flag;
}

bool is_sdcard_full(const char *mount_point, unsigned long long threshold_bytes) {
    struct statvfs stat;

    // 检查statvfs调用是否成功
    if (statvfs(mount_point, &stat) != 0) {
        perror("statvfs");
        exit(EXIT_FAILURE); 
    }

    // 计算剩余空间（以字节为单位）
    unsigned long long available_bytes = stat.f_bavail * stat.f_frsize;

    // 判断是否已满
    if (available_bytes <= threshold_bytes) {
        return true;
    } else {
        return false;
    }
}

/* start_vi  -- 配置vi属性
 * dev_id [IN]:  dev id
 */

static int start_vi(int dev_id)
{
    int ret = -1;

    /*初始化参数*/
    int chn_main_id = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].chn_id;
    int chn_sub_id = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[1].chn_id;
    int chn_thr_id = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[2].chn_id;
    VI_DEV_ATTR dev_attr={0};
    VI_CHN_ATTR_EX chn_attr = {0};
    VI_CHN_ATTR_EX chn_attr_sub = {0};

    /*打开dev设备节点*/
    ret = ak_vi_open(dev_id);
    if (AK_SUCCESS != ret) {
        printf("vi device %d open failed\n", dev_id);
        return ret;
    }

    if(g_qs_fd > 0){
        ak_vi_set_cfg_file_path(dev_id, vi_info[dev_id].qs_vi_info.qs_vi_dev_info.isp_path);
    }

    /* configure vi_dev attr*/
    memset(&dev_attr, 0, sizeof(VI_DEV_ATTR));
    dev_attr.dev_id             = dev_id;
    if(CIS_cnt > 1)
        dev_attr.interf_mode = VI_INTF_DUAL_MIPI_2;

    /*配置dev属性*/
    dev_attr.crop.left          = vi_info[dev_id].qs_vi_info.qs_vi_dev_info.crop.left;
    dev_attr.crop.top           = vi_info[dev_id].qs_vi_info.qs_vi_dev_info.crop.top;
    dev_attr.crop.width         = vi_info[dev_id].qs_vi_info.qs_vi_dev_info.crop.width;
    dev_attr.crop.height        = vi_info[dev_id].qs_vi_info.qs_vi_dev_info.crop.height;
    dev_attr.max_width          = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].max_res.width;
    dev_attr.max_height         = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].max_res.height;
    dev_attr.sub_max_width      = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[1].max_res.width;
    dev_attr.sub_max_height     = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[1].max_res.height;
    dev_attr.frame_rate         = vi_info[dev_id].qs_vi_info.qs_vi_dev_info.frame_rate;

    /*读取真实cis分辨率*/
    RECTANGLE_S res;
    ret = ak_vi_get_sensor_resolution(dev_id, &res);
    if (ret) {
        printf("Can't get dev[%d]resolution\n", dev_id);
        return ret;
    } else {
        printf("get dev res w:[%d]h:[%d]\n",res.width, res.height);
        dev_attr.crop.width = res.width;
        dev_attr.crop.height = res.height;
    }

    /*
     * 设置3dnr大小
     * 如果是压缩模式，才生效
    */
    int buf_size_3dnr = vi_info[dev_id].qs_vi_info.qs_vi_dev_info.isp_3dnr_info.isp_3dnr_total_len;
    ak_vi_set_dev_3DNR_memsize(dev_id, buf_size_3dnr); //KB

    /*设置dev属性*/
    ret = ak_vi_set_dev_attr(dev_id, &dev_attr);
    if (ret) {
        printf("vi device %d set device attribute failed!\n", dev_id);
        return ret;
    }


    /* 设置主通道属性 */
    memset(&chn_attr, 0, sizeof(VI_CHN_ATTR_EX));
    chn_attr.chn_id         = chn_main_id;
    chn_attr.res.width      = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].res.width;
    chn_attr.res.height     = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].res.height;
    chn_attr.frame_depth    = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].frame_depth;
    chn_attr.data_type      = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].data_type;
    chn_attr.mode           = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].mode;

#ifdef DEBUG
            printf("dev_id:[%d]\n", dev_id);
            printf("chn_id:[%d]\n", vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].chn_id);
            printf("chn_enable:[%d]", vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].enable);
            printf("chn_res:[%d*%d]\n", vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].res.width, 
                                        vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].res.height);
            printf("chn_max_res:[%d*%d]\n", vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].max_res.width, 
                                            vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].max_res.height);
            printf("chn_frame_depth:[%d]\n", vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].frame_depth);
            printf("chn_mode:[%d]\n", vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].mode);
#endif
    ret = ak_vi_set_chn_attr_ex(chn_main_id, &chn_attr);
    if (ret) {
        printf("vi device %d set channel [%d] attribute failed!\n", dev_id, chn_main_id);
        return ret;
    }

    /* 设置次通道属性 */
    memset(&chn_attr_sub, 0, sizeof(VI_CHN_ATTR_EX));
    chn_attr_sub.chn_id         = chn_sub_id;
    chn_attr_sub.res.width      = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[1].res.width;
    chn_attr_sub.res.height     = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[1].res.height;
    chn_attr_sub.frame_depth    = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[1].frame_depth;
    chn_attr_sub.data_type      = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[1].data_type;
    chn_attr_sub.mode           = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[1].mode;

    ret = ak_vi_set_chn_attr_ex(chn_sub_id, &chn_attr_sub);
    if (ret) {
        printf("vi device %d set channel [%d] attribute failed!\n", dev_id, chn_sub_id);
        return ret;
    }

    /*设置第三通道属性*/
    VI_CHN_ATTR_EX chn_attr_thr = {0};
    memset(&chn_attr_thr, 0, sizeof(VI_CHN_ATTR_EX));
    chn_attr_thr.chn_id         = chn_thr_id;
    chn_attr_thr.res.width      = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[2].res.width;
    chn_attr_thr.res.height     = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[2].res.height;
    chn_attr_thr.frame_depth    = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[2].frame_depth;
    chn_attr_thr.data_type      = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[2].data_type;
    chn_attr_thr.mode           = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[2].mode;

    ret = ak_vi_set_chn_attr_ex(chn_thr_id, &chn_attr_thr);
    if (ret) {
        printf("vi device %d set channel [%d] attribute failed!\n", dev_id, chn_thr_id);
        return ret;
    }

    
    return ret;
}
/* end of func */

int get_encode_type(int venc_id)
{
    int i = 0;
    for(; i<VENC_cnt; i++)
    {
        if(venc_id == venc_info_s[i].qs_venc_info.venc_id)
            return venc_info_s[i].qs_venc_info.enc_out_type;
    }

    return -1;
}

/* disable_vi  -- 关闭vi 通道
 * dev_id [IN]:  dev id
 */
static int disable_vi(int dev_id)
{
    /*关闭chn 通道*/
    int chn_main_id = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].chn_id;
    int chn_sub_id = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[1].chn_id;
    int chn_thr_id = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[2].chn_id;

    ak_vi_disable_chn(chn_main_id);
    ak_vi_disable_chn(chn_sub_id);
    ak_vi_disable_chn(chn_thr_id);
    return 0;
}

/* stop_vi  -- 关闭vi设备
 * dev_id [IN]:  dev id
 */
static int stop_vi(int dev_id)
{
    /* 关闭dev设备 */
    int ret = 0;
    ak_vi_disable_dev(dev_id);
    ret = ak_vi_close(dev_id);

    return ret;
}

/* stop_stitch_chn  -- 销毁拼接通道
 * chn_id [IN]:  stitch_chn_id
 */
static int stop_stitch_chn(int chn_id)
{
    /*销毁拼接通道*/
    int ret = -1;
    ak_vi_destroy_stitch_chn(chn_id);
    return ret;
}

/* start_stitch_chn  -- 销毁拼接通道
 *  stit_attr[IN]:  拼接通道属性
 */
static int start_stitch_chn(struct stitch_attr_s *stit_attr)
{
    int ret = -1;
    /*配置通道属性*/
    VI_STITCH_ATTR stitch_attr = {0};
    stitch_attr.mode = stit_attr->stitch_mode;
    stitch_attr.stitch_chn_num = stit_attr->stitch_num;
    printf("[%s %d]stitch_chn_id[%d],stitch_num:[%d]\n",
            __func__,__LINE__,stit_attr->stitch_chn_id,stitch_attr.stitch_chn_num);   

    /*创建拼接通道*/
    memcpy(stitch_attr.stitch_bind_chn,stit_attr->stitch_bind_chn, sizeof(stitch_attr.stitch_bind_chn));
    ret = ak_vi_create_stitch_chn(stit_attr->stitch_chn_id, NULL, &stitch_attr);

    if(ret != AK_SUCCESS)
    {
        printf("create stitch_chn [%d] failed!\n", stit_attr->stitch_chn_id);
    }

    return ret;
}

/* enable_vi  -- 启动vi设备与通道
 * dev_id [IN]:  dev id
 */
static int enable_vi(int dev_id)
{
    int ret = 0;
    int chn_main_id = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[0].chn_id;
    int chn_sub_id = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[1].chn_id;
    int chn_thr_id = vi_info[dev_id].qs_vi_info.qs_vi_chn_info[2].chn_id;

    /*
    * step 8: enable vi device
    */
    ret = ak_vi_enable_dev(dev_id);
    if (ret) {
        printf("vi device %d enable device failed! ret:%d\n", dev_id,ret);
        return ret;
    }
    
    /*
     * step 9: enable vi main channel
     */
    ret = ak_vi_enable_chn(chn_main_id);
    if(ret)
    {
        printf("vi channel[%d] enable failed! ret:%d\n", chn_main_id,ret);
        return ret;
    }
    
    /*
     * step 10: enable vi sub channel
     */
    ret = ak_vi_enable_chn(chn_sub_id);
    if(ret)
    {
        printf("vi channel[%d] enable failed!\n",chn_sub_id);
        return ret;
    }
    
    /*
     * step 11: enable vi third channel
     */
    ret = ak_vi_enable_chn(chn_thr_id);
    if(ret)
    {
        printf("vi channel[%d] enable failed!\n",chn_thr_id);
        return ret;
    } 
    
    
    return ret;
}


/**
 * brief:  normal stream save
 * return: 0 success, otherwise error code;
 * notes:
 */
static int normal_stream_save(struct video_attr_s *venc_th, struct video_stream *stream, 
                              int frame_index, int encode_type)
{
    int ret = 0;
    int finish_save_flag = 0;
    /*设置线程独立的静态局部变量*/
    static __thread FILE *fp=NULL;
    static __thread char path[128] = {0};
    static __thread unsigned write_len=0;

    //快启主通道码流保存
    if (stream_save_flag){
        //打开文件
        if (fp == NULL){
            /*根据编码类型设定保存文件路径*/
            if(encode_type == MJPEG_ENC_TYPE)
                sprintf(path, "/tmp/test_chn_%d_%d_snap.jpg", venc_th->chn_id, venc_th->venc_id);
            else
                sprintf(path, "/tmp/test_chn_%d_%d_no%d.str", venc_th->chn_id, venc_th->venc_id, frame_index);
            /*创建文件*/
            fp = fopen(path, "w+");
            if(fp != NULL)
                ak_print_normal_ex(MODULE_ID_APP, "save the stream to %s\n", path);
        }

        //保存码流到tmp目录
        if(fp && frame_index <= fast_stream_cnt){
            ret = fwrite(stream->data, 1, stream->len, fp);
            write_len += ret;
            /*如果写入数据长度不一致，报错*/
            if(ret != stream->len){
                ak_print_error_ex(MODULE_ID_APP,"CHN [%d], fwrite file stream [%d] failed len %d write error count %d total_len[%u].\n",
                                                                               venc_th->chn_id, frame_index, stream->len, ret, write_len);
            }
            else {
                /*如果是JPEG 编码，保存文件准确就立即退出*/
                if(encode_type == MJPEG_ENC_TYPE)
                {
                    ak_print_error_ex(MODULE_ID_VENC,"CHN [%d], jpeg record done path:%s.\n",
                                                                            venc_th->chn_id, path);
                    fclose(fp);
                    fp = NULL;
                    finish_save_flag = 1;
                    file_finish_cnt++;
                }

                //H264类型，如果码流触发保存上限，关闭文件。
                if( (frame_index >= fast_stream_cnt) || write_len > MAX_FILE_LEN){
                    ak_print_error_ex(MODULE_ID_VENC,"CHN [%d], frame_index[%d], record done , file size[%d], path:%s.\n",
                                                                            venc_th->chn_id, frame_index, write_len, path);
                    fclose(fp);
                    fp = NULL;
                    finish_save_flag = 1;
                    file_finish_cnt++;
                }
            }
        }
    }

    return finish_save_flag;
}

/**
 * brief: normal stream thread
 * return: 0 success, otherwise error code;
 * notes:
 */
static void *normal_stream_thread(void *arg)
{
    int ret = -1;
    int vi_chn_id = 0;
    int frame_index = 0;
    int fast_stream_finish_save = 0;
    char thr_name[128] = {0};
    char cmd[128] = {0};
    struct video_attr_s *venc_th = (struct video_attr_s *)arg; //venc handle

    /* 设置线程名称 */
    sprintf(thr_name,"ns_vth_%d_%d",venc_th->chn_id,venc_th->venc_id);
    ak_thread_set_name(thr_name);
    printf("Run thread:%s\n",thr_name);

    /* 打印提示当前运行的线程模式 */
    ak_print_notice_ex(MODULE_ID_APP, "normal stream vi[%d],venc[%d]mode = %d \n", 
                                    venc_th->chn_id, venc_th->venc_id, venc_th->mode);

    /* vi chn id 处理 */
    vi_chn_id = venc_th->chn_id;


    /*------------- 普流 --------------*/
    APP_CHN_S Schn;
    Schn.mid = MODULE_ID_VI;
    Schn.oid = venc_th->chn_id;

    APP_CHN_S Dchn;
    Dchn.mid = MODULE_ID_VENC;
    Dchn.oid = venc_th->venc_id;

    APP_BIND_PARAM param;
    param.frame_rate = venc_th->frame_rate;
    param.frame_depth = venc_th->frame_depth;
    param.mode = venc_th->mode;
    
    /* 绑定vi venc通道 */
    ret = ak_app_video_bind_chn(&Schn, &Dchn, &param);  
    if(ret != AK_SUCCESS)
    {
        printf("ak_app_video_bind_chn failed [%d]\n",ret);
        return NULL;
    }

    /* 强制I帧 */
    ak_venc_request_idr(venc_th->venc_id);  

    /* 使能码流输出 */
    ret = ak_app_video_set_dst_chn_active(&Dchn, AK_TRUE);
    if(ret != AK_SUCCESS)
    {
        printf("ak_app_video_set_dst_chn_active failed [%d]\n",ret);
        return NULL;
    }

    /*获取编码类型*/
    int encode_type = get_encode_type(venc_th->venc_id);

    /*申请码流缓存*/
    struct video_stream *stream = ak_mem_alloc(MODULE_ID_APP, sizeof(struct video_stream));
    if(NULL == stream)
    {
        printf("Can't alloc memory for stream buffer!\n");
        return NULL;
    }

    venc_th->th_run_flag = AK_TRUE;
    VIDEO_enable_cnt++;
    //普流采集编码
    while(venc_th->th_run_flag)
    {
        memset(stream, 0, sizeof(struct video_stream));
        /* 获取编码帧 */
        ret = ak_app_video_venc_get_stream(&Dchn, stream);
        if(ret == AK_SUCCESS)
        {

            frame_index++;
            if(encode_type == MJPEG_ENC_TYPE)
            {
                ret = normal_stream_save(venc_th, stream, frame_index, encode_type);
                if(ret == 1)
                {
                    venc_th->th_run_flag = AK_FALSE;
                }
            }
            else
            {

                //打印前10帧信息，查看应用获取到第一帧的时间
                if(frame_index < 10){
                    printf("[chn:%d fast_venc_handle:%d %lu] Get_Stream[%s] len:%4u ts:%d \n",
                        venc_th->chn_id,venc_th->venc_id,stream->seq_no,
                        stream->frame_type==FRAME_TYPE_I?"I":"P",stream->len,stream->ts);
                }

                //打印I帧信息，用于监控程序正在运行
                int encode_type = venc_info_s[venc_th->venc_id].qs_venc_info.enc_out_type;
                if(stream->frame_type==FRAME_TYPE_I && encode_type != MJPEG_ENC_TYPE)
                    printf("chn:%d handle:%d %lu] Get Stream[%s] len:%4u ts:%5llu\n",
                            venc_th->chn_id,venc_th->venc_id,stream->seq_no,
                            stream->frame_type==FRAME_TYPE_I?"I":"P",stream->len,stream->ts);

                //码流保存
                if(fast_stream_finish_save == 0){
                    fast_stream_finish_save = normal_stream_save(venc_th, stream, frame_index, encode_type);
                }
            }
            /*释放编码帧*/
            ak_venc_release_stream(venc_th->venc_id, stream);

        }
        else
        {
            ak_sleep_ms(10);
        }
    }

exit:
    /*------------- 普流结束，线程退出 --------------*/
    if(stream) {
        ak_mem_free(stream);
    }

    /*关闭流式编码*/
    ret = ak_app_video_set_dst_chn_active(&Dchn, AK_FALSE);
    if(ret != AK_SUCCESS)
    {
        printf("ak_app_video_unbind_chn failed [%d]\n",ret);
    }
    /*解绑编码器和采集通道*/
    ret = ak_app_video_unbind_chn(&Schn, &Dchn);
    if(ret != AK_SUCCESS)
    {
        printf("ak_app_video_unbind_chn failed [%d]\n",ret);
    }

    //system("rm -rf /tmp/*.str");

    printf("######Thread[%s] out\n",thr_name);

    return NULL;

}/* end of func */

static int start_venc(int venc_id)
{
    int ret = -1;

    struct venc_rc_param rc_param = {0};
    rc_param.enable_MMA = 1;
    int handle_id = -1;
    struct venc_param ve_param = {0};
    ve_param.width  = venc_info_s[venc_id].qs_venc_info.width;
    ve_param.height = venc_info_s[venc_id].qs_venc_info.height;
    
    /* decode type is h264*/
    int encoder_type = venc_info_s[venc_id].qs_venc_info.enc_out_type;
    if(encoder_type == H264_ENC_TYPE)
    {
        /* profile type */
        ve_param.profile     = PROFILE_MAIN;

    }
    /*decode type is jpeg*/
    else if(encoder_type == MJPEG_ENC_TYPE)
    {
        /* jpeg enc profile */
        ve_param.profile = PROFILE_JPEG;
    }
    /*decode type is h265*/
    else
    {
        /* hevc profile */
        ve_param.profile = PROFILE_HEVC_MAIN;
    }

    /* set venc param according to qs venc info */
    ve_param.fps                    = venc_info_s[venc_id].qs_venc_info.fps;
    ve_param.goplen                 = venc_info_s[venc_id].qs_venc_info.goplen;
    ve_param.target_kbps            = venc_info_s[venc_id].qs_venc_info.target_kbps;
    ve_param.max_kbps               = venc_info_s[venc_id].qs_venc_info.max_kbps;
    ve_param.br_mode                = venc_info_s[venc_id].qs_venc_info.br_mode;
    ve_param.minqp                  = venc_info_s[venc_id].qs_venc_info.minqp;
    ve_param.maxqp                  = venc_info_s[venc_id].qs_venc_info.maxqp;
    ve_param.initqp                 = venc_info_s[venc_id].qs_venc_info.initqp;
    ve_param.jpeg_qlevel            = JPEG_QLEVEL_DEFAULT;
    ve_param.chroma_mode            = CHROMA_4_2_0;
    ve_param.max_picture_size       = venc_info_s[venc_id].qs_venc_info.max_picture_size;
    ve_param.enc_level              = 30;
    ve_param.enc_out_type = encoder_type;           //enc type


    printf("venc chn:%d ve_param.target_kbps:%u ve_param.enc_out_type:%u\n",
            venc_id,ve_param.target_kbps,ve_param.enc_out_type);


    ret = ak_venc_open_ex(&ve_param, &rc_param,  &handle_id);
    if (ret || (-1 == handle_id) )
    {
        printf("venc chn[%d] open venc failed ret:%d\n",venc_id, ret);
        return ret;
    }
    printf("venc chn%d open venc succeed!! ret:%d\n", venc_id, ret);

    /* set the venc handle id to g_video_attr*/
    g_video_attr[venc_id].venc_id = handle_id;

    return ret;
}

int start_video(void)
{
    int ret = AK_FAILED;
    int i=0;
    for(; i<VIDEO_cnt; i++)
    {
        int j=0;
        if(video_info[i].qs_video_info.enable != AK_TRUE)
            continue;

        for(;j<VENC_cnt; j++)
        {
            if(video_info[i].qs_video_info.venc_id == g_video_attr[j].venc_id)
            {
                g_video_attr[j].chn_id = video_info[i].qs_video_info.chn_id;
                g_video_attr[j].frame_rate = video_info[i].qs_video_info.frame_rate;
                g_video_attr[j].frame_depth = video_info[i].qs_video_info.frame_depth;
                g_video_attr[j].mode = video_info[i].qs_video_info.venc_encode_mode;
#ifdef DEBUG
                printf("video_vi_chn[%d][%d]\n", j, g_video_attr[j].chn_id);
                printf("video_venc_chn[%d][%d]\n", j, g_video_attr[j].venc_id);
#endif
                ret = ak_thread_create(&g_video_attr[j].pid, normal_stream_thread, (void*)&g_video_attr[j],   \
                                                    ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);
            }
        }
    }
}

int start_vi_venc_normal()
{
    int ret = -1;
    int i = 0;
    //VI初始化
    for (i = 0; i < CIS_cnt; i++)
    {
        ret = start_vi(i);
        if(ret)
        {
            ak_print_error_ex(MODULE_ID_APP,"vi init failed!\n");
            return ret;
        }
    }
    
    //拼接VI初始化
    for(i=0;i<STITCH_cnt; i++)
    {
        start_stitch_chn(&g_stitch_attr[i]);	
    }

    //启动VI
    for (i = 0; i < CIS_cnt; i++)
    {
        ret = enable_vi(i);
        if(ret)
        {
            ak_print_error_ex(MODULE_ID_APP,"vi enable failed!\n");
            return ret;
        }
        
    }

    //启动拼接VI
    for (i = 0; i < STITCH_cnt; i++)
    {
        ret = ak_vi_enable_chn(g_stitch_attr[i].stitch_chn_id);
        if(ret)
        {
            ak_print_error_ex(MODULE_ID_APP,"enable stitch chn [%d] failed!\n", g_stitch_attr[i].stitch_chn_id);
            return ret;
        }
    }

    // 开启全部需要的VENC通道
    for (i=0; i<VENC_cnt; i++)
    {
        ret=start_venc(i);
        if(ret)
        {
            ak_print_error_ex(MODULE_ID_APP,"Start venc [%d] failed!\n", i);
            return ret;
        }
    }

    //启动流式编码
    ret=start_video();	
    
    return ret;
}

void mount_tf_card(void)
{
    if(tf_write_enable){
        //挂卡
        sleep(1);
        printf("tf_card.sh;mkdir /tmp/sd\n");
        system("tf_card.sh;mkdir /tmp/sd");

        sleep(1);
        if(access("/dev/mmcblk0p1", F_OK) == -1){
            //没有p1节点，挂非p1
            printf("mount /dev/mmcblk0 /tmp/sd\n");
            system("mount /dev/mmcblk0 /tmp/sd");
        }else{
            //有p1，则挂p1
            printf("mount /dev/mmcblk0p1 /tmp/sd\n");
            system("mount /dev/mmcblk0p1 /tmp/sd");
        }
    }
}

void fast_stream_save(void)
{
    if(tf_write_enable){ 
        char cmd[128] = {0};
        /*判断计数文件是否存在,不存在则创建*/
        if (access(NUM_FILE, F_OK))
        {
            //no exist!
            printf("save_num_buffer:0\n");
            snprintf(cmd, sizeof(cmd), "echo 0 > %s",NUM_FILE);
            system(cmd);
            
            snprintf(cmd, sizeof(cmd), "mkdir /tmp/sd/jpeg");
            system(cmd);
            snprintf(cmd, sizeof(cmd), "mkdir /tmp/sd/video");
            system(cmd);

            while(1)
            {
                /* mv the tmp file to tfcard */
                if(file_finish_cnt == VIDEO_enable_cnt){
                    snprintf(cmd, sizeof(cmd), "cp -ar /tmp/test_*.str /tmp/sd/video/; sync");
                    system(cmd);
                    snprintf(cmd, sizeof(cmd), "cp -ar /tmp/test_*.jpg /tmp/sd/jpeg/; sync");
                    system(cmd);
                /*记录 demesg */
                    snprintf(cmd,sizeof(cmd), "dmesg >> /tmp/sd/dmesg.log");
                    system(cmd);
                    break;
                }
                else
                    ak_sleep_ms(10);
            }
        }
        else
        {
            //exist
            char num_buffer[8] = {0};
            snprintf(cmd, sizeof(cmd), "cat %s",NUM_FILE);

            FILE *fp = popen(cmd, "r");
            if (fp != NULL)
            {
                int ret = fread(num_buffer, 1, 8, fp);
                pclose(fp);
                fp = NULL;
            }
            /* 更新计数 */
            int num = atoi(num_buffer);
            num++;

            while(1)
            {
                /* mv the tmp file to tfcard */
                if(file_finish_cnt == VIDEO_enable_cnt){
                    /* save the str file*/
                    snprintf(cmd,sizeof(cmd), "mkdir /tmp/sd/video/%d", num);
                    system(cmd);
                    snprintf(cmd, sizeof(cmd), "cp -ar /tmp/test_*.str /tmp/sd/video/%d/; sync",num);
                    system(cmd);

                    /* save the jpeg file*/
                    snprintf(cmd,sizeof(cmd), "mkdir /tmp/sd/jpeg/%d", num);
                    system(cmd);
                    snprintf(cmd, sizeof(cmd), "cp -ar /tmp/test_*.jpg /tmp/sd/jpeg/%d/; sync",num);
                    system(cmd);

                    snprintf(cmd, sizeof(cmd), "echo [**** reboot_%d *****] >> /tmp/sd/dmesg.log", num);
                    system(cmd);

                    snprintf(cmd, sizeof(cmd), "dmesg >> /tmp/sd/dmesg.log");
                    system(cmd);
                    break;
                }
                else
                    ak_sleep_ms(10);
            }
            /* 更新计数到文件中 */
            printf("save_num_buffer:%d\n",num);
            sprintf(cmd,"echo %d > %s ; sync",num,NUM_FILE);
            system(cmd);
        }

    }
}

int find_stitch_id(int stitch_chn_id)
{
    /* 查找正确的拼接通道信息 */
    int i=0;
    for(;i<SAMPLE_MAX_STITCH_GROUP; i++)
    {
        if(g_stitch_attr[i].stitch_mode == 0 || g_stitch_attr[i].stitch_chn_id == stitch_chn_id)
        {
            if(g_stitch_attr[i].stitch_mode == 0)
                STITCH_cnt++;

            break;
        }
    }
    return i;			
}




/* argb8888 table */
static unsigned int rgb_color_table[3] =
{
    0xff0000ff, 0xff00ff00, 0xffff0000
};


// 禁用驱动的SVP检测 - 按VI通道为单位
static int disable_qs_svp(void)
{
    int ret = 0;
    struct quick_start_ctrl_svp ctrl_info = {0};
    
    //printf("Disabling driver SVP detection for all VI channels...\n");
    
    // 遍历所有启用的VI设备和通道
    for(int dev_id = 0; dev_id < SAMPLE_MAX_DEV_NUM; dev_id++) {
        if(vi_info[dev_id].qs_vi_info.qs_vi_dev_info.enable != 1) {
            continue;
        }
        
        // 遍历该设备的所有VI通道
        for(int chn_id = 0; chn_id < 3; chn_id++) {  // 最多3个通道
            struct ak_qs_vi_chn_info *vi_chn_info = &vi_info[dev_id].qs_vi_info.qs_vi_chn_info[chn_id];
            
            // 检查此VI通道是否有NPU启用
            if(vi_chn_info->npu_attr.npu_enable == 1) {
                // 构造控制信息 - 按VI通道为单位禁用
                ctrl_info.dev_id = dev_id;
                ctrl_info.chn_id = vi_chn_info->chn_id;  // VI通道ID
                ctrl_info.svp_enable = 0;                // 禁用
                
                // 发送ioctl命令禁用驱动的SVP检测
                ret = ioctl(g_qs_fd, IO_QUICK_START_SET_SVP_CTRL, &ctrl_info);
                if (ret < 0) {
                    printf("Disable driver SVP for dev %d vi_chn %d failed: %s\n", 
                           dev_id, vi_chn_info->chn_id, strerror(errno));
                } else {
                    printf("Disabled driver SVP for dev %d vi_chn %d\n", 
                           dev_id, vi_chn_info->chn_id);
                }
            }
        }
    }
    
    return 0;
}


// 获取驱动SVP检测结果
static int get_driver_svp_result(void)
{
    int ret = 0;
    struct quick_start_svp_info svp_info = {0};

    //遍历所有npu通道
    for (int i = 0; i < NPU_cnt && i < SAMPLE_MAX_NPU_NUM; i++) {
        if (npu_info[i].npu_info.chn_id >= 0) {
            memset(&svp_info, 0, sizeof(struct quick_start_svp_info));
            svp_info.chn_id = npu_info[i].npu_info.chn_id;
    
            ret = ioctl(g_qs_fd, IO_QUICK_START_GET_SVP_INFO, &svp_info);
            if (ret < 0) {
                printf("Get SVP result for NPU chn %d failed: %s\n", svp_info.chn_id, strerror(errno));
                return -1;
            }
            
            printf("=== SVP Detection Results ===\n");
            printf("NPU Channel ID: %d\n", svp_info.chn_id);
            printf("Total Detection Results: %d\n", svp_info.total_frame);  // 总共返回了多少帧检测结果
            printf("SVP Exit Flag: %d\n\n", svp_info.bSvpExit);
            
            if (svp_info.total_frame > 0) {
                for (int i = 0; i < svp_info.total_frame; i++) {
                    // 使用 frame_seq 显示这是第几帧检测
                    unsigned long frame_seq = svp_info.results[i].frame_seq;
                    // 使用 timestamp_ms 显示时间戳
                    unsigned long timestamp = svp_info.results[i].timestamp_ms;
                    unsigned long sec = timestamp / 1000;
                    unsigned long msec = timestamp % 1000;
                    // 使用 total_num 显示这一帧检测到的目标数量
                    int total_num = svp_info.results[i].total_num;
                    
                    printf("--- Detection Result %d (Frame Sequence: %lu) ---\n", i, frame_seq);
                    printf("Timestamp: %lu.%03lu seconds (ms: %lu)\n", sec, msec, timestamp);
                    printf("Targets in this frame: %d\n", total_num);
                    
                    if (total_num > 0) {
                        // 只打印这一帧的第一个目标坐标
                        quick_start_svp_target_boxes_t *first_box = &svp_info.results[i].target_boxes[0];
                        printf("First Target:\n");
                        printf("  Label: %u\n", first_box->label);
                        printf("  Score: %d\n", first_box->score);
                        printf("  Location: (%lu, %lu, %lu, %lu)\n",
                            first_box->left, first_box->top, first_box->right, first_box->bottom);
                    } else {
                        printf("  No targets detected in this frame\n");
                    }
                    printf("\n");
                }
            } else {
                printf("No detection results\n");
            }
            printf("================================\n");

        }
    }
    
    return 0;
}


// 查找启用 NPU 的 VI 通道信息
static int find_npu_enabled_vi_chn(int npu_chn_id, int *dev_id, int *vi_chn_id, int *width, int *height)
{
    // 遍历所有 VI 设备
    for(int i = 0; i < SAMPLE_MAX_DEV_NUM; i++)
    {
        if(vi_info[i].qs_vi_info.qs_vi_dev_info.enable != 1)
            continue;
            
        // 遍历该设备的所有 VI 通道
        for(int j = 0; j < 3; j++)  // 最多3个通道
        {
            struct ak_qs_vi_chn_info *chn_info = &vi_info[i].qs_vi_info.qs_vi_chn_info[j];
            
            // 检查该通道是否启用 NPU 且绑定到指定的 NPU 通道
            if(chn_info->npu_attr.npu_enable == 1)
            {
                // 检查是否绑定到我们的 NPU 通道
                for(int k = 0; k < DSS_NPU_MAX_NUM; k++)
                {
                    if(chn_info->npu_attr.npu_chn_id[k] == npu_chn_id)
                    {
                        *dev_id = i;
                        *vi_chn_id = chn_info->chn_id;
                        *width = chn_info->res.width;
                        *height = chn_info->res.height;

                        return 0;  // 找到了
                    }
                }
            }
        }
    }
    
    printf("No VI channel found for NPU chn %d\n", npu_chn_id);
    return -1;  // 未找到
}

// 查找启用 NPU 的 VI 通道信息（修改返回值）
static int find_npu_enabled_vi_chn_count(void)
{
    int count = 0;
    // 遍历所有 VI 设备
    for(int i = 0; i < SAMPLE_MAX_DEV_NUM; i++)
    {
        if(vi_info[i].qs_vi_info.qs_vi_dev_info.enable != 1)
            continue;
            
        // 遍历该设备的所有 VI 通道
        for(int j = 0; j < 3; j++)  // 最多3个通道
        {
            struct ak_qs_vi_chn_info *chn_info = &vi_info[i].qs_vi_info.qs_vi_chn_info[j];
            // 检查该通道是否启用 NPU
            if(chn_info->npu_attr.npu_enable == 1)
            {
                count++;
            }
        }
    }
    return count;
}
// SVP 处理线程 - 按VI通道为单位
static void* vi_svp_thread(void *arg)
{
    struct vi_chn_svp_work_data *work_data = (struct vi_chn_svp_work_data *)arg;
    int dev_id = work_data->dev_id;
    int vi_chn_id = work_data->vi_chn_id;
    int get_frame_chn_id = work_data->get_frame_chn_id;
    struct video_input_frame frame;
    int ret = -1;
    int vi_width = 0, vi_height = 0;
    
    //printf("VI SVP thread started for dev %d chn %d\n", dev_id, vi_chn_id);

    while (work_data->run_flag)
    {
        // 获取帧
        ret = ak_vi_get_frame(vi_chn_id, &frame);
        if (!ret)
        {
            // 将同一帧送入所有绑定的NPU通道处理
            for (int i = 0; i < work_data->npu_count; i++) {
                int npu_chn_id = work_data->npu_chn_ids[i];
                find_npu_enabled_vi_chn(npu_chn_id, &dev_id, &vi_chn_id, &vi_width, &vi_height);

                AK_SVP2_IMG_INFO_T input = {0};
                AK_SVP2_OUTPUT_T output = {0};
                
                // 设置输入图像参数
                input.img_type = AK_SVP2_IMG_RGB_LI;
                input.width = vi_width;
                input.height = vi_height;
                input.pos_info.left = 0;
                input.pos_info.top = 0;
                input.pos_info.width = input.width;
                input.pos_info.height = input.height;
                input.vir_addr = frame.vi_frame.data;
                input.phy_addr = frame.phyaddr;

                ret = ak_svp2_process(npu_chn_id, &input, &output);
                if(ret == AK_SUCCESS)
                {
                    if(output.result.hd_result.total_num > 0)
                    {
                        printf("SVP dev %d chn %d detect NPU %d: %d targets\n",
                               dev_id, vi_chn_id, npu_chn_id, output.result.hd_result.total_num);
                    }
                    // 释放output
                    ak_svp2_release(&output);
                }
                else{
                    printf("npu %d process fail \n",npu_chn_id);
                }
            }
            
            ak_vi_release_frame(vi_chn_id, &frame);
        }
        else
        {
            ak_sleep_ms(30);
        }
    }

    printf("VI SVP thread stopped for dev %d chn %d\n", dev_id, vi_chn_id);
    return NULL;
}

// 映射VI通道ID到取帧通道ID
static int map_vi_chn_to_get_frame_chn(int vi_chn_id)
{
    if(vi_chn_id == 0 || vi_chn_id == 2 || vi_chn_id == 4 || vi_chn_id == 6){
        return 0;
    }
    else if(vi_chn_id == 1 || vi_chn_id == 3 || vi_chn_id == 5 || vi_chn_id == 7){
        return 1;
    }
    else if(vi_chn_id >= 16 && vi_chn_id <= 19){
        return 2;
    }
    else{
        return vi_chn_id;
    }
}

// 初始化所有NPU通道
static int init_all_npu_channels(void)
{
    int ret = 0;
    AK_SVP2_CHN_ATTR_T *attr = NULL;
    
    printf("Initializing all NPU channels...\n");
    
    // 初始化NPU硬件
    ret = ak_npu_init();
    if(ret != AK_SUCCESS)
    {
        printf("Init NPU failed!\n");
        return ret;
    }
    
    // 遍历所有配置的NPU通道
    for (int i = 0; i < NPU_cnt && i < SAMPLE_MAX_NPU_NUM; i++) {
        if (npu_info[i].npu_info.chn_id >= 0) {
            printf("Creating NPU channel %d with model %s\n", 
                   npu_info[i].npu_info.chn_id, npu_info[i].npu_info.model_path);
            
            // 创建SVP通道属性
            int model_num = 1;
            int svp_len = sizeof(AK_SVP2_CHN_ATTR_T) + model_num*sizeof(AK_SVP2_MODEL_ATTR_T);
            attr = (AK_SVP2_CHN_ATTR_T *)ak_mem_alloc(MODULE_ID_APP, svp_len);
            if (!attr) {
                printf("Alloc SVP attr failed for chn_id: %d\n", npu_info[i].npu_info.chn_id);
                continue;
            }
            memset(attr, 0, svp_len);
            attr->model_num = model_num;
            attr->model_attr->model_type = npu_info[i].npu_info.model_type;
            
            // 设置SVP参数
            attr->model_attr->model_param.param.hd_param.classify_threshold = 700;
            attr->model_attr->model_param.param.hd_param.IoU_threshold = 3;
            attr->model_attr->model_param.param.hd_param.target_type = AK_SVP2_HUMAN_SHAPE_AND_FACE;
            
            AK_SVP2_CONF_INFO conf_info = {0};
            conf_info.conf_mode = 0;
            conf_info.conf_type.conf = npu_info[i].npu_info.model_path;
            
            // 创建SVP通道
            ret = ak_svp2_create_chn(npu_info[i].npu_info.chn_id, attr, &conf_info);
            if(ret != AK_SUCCESS)
            {
                printf("Create SVP chn %d failed, ret: %d\n", npu_info[i].npu_info.chn_id, ret);
            }
            else
            {
                printf("Create SVP chn %d succeed! model: %s\n", 
                       npu_info[i].npu_info.chn_id, npu_info[i].npu_info.model_path);
            }
            
            if(attr != NULL) {
                ak_mem_free(attr);
            }
        }
    }
    
    return 0;
}

// 设置VI通道的NPU处理
static int setup_vi_channels_svp(void)
{
    int work_count = 0;
    int ret = 0;
    
    //printf("Setting up VI channels for SVP processing\n");
    
    // 初始化工作信息数组
    memset(vi_svp_work_info, 0, sizeof(vi_svp_work_info));
    
    // 遍历所有VI设备
    for(int dev_id = 0; dev_id < SAMPLE_MAX_DEV_NUM; dev_id++){
        if(vi_info[dev_id].qs_vi_info.qs_vi_dev_info.enable != 1) {
            continue;
        }
        
        // 遍历所有VI通道
        for(int chn_id = 0; chn_id < 3; chn_id++){
            struct ak_qs_vi_chn_info *vi_chn_info = &vi_info[dev_id].qs_vi_info.qs_vi_chn_info[chn_id];
            
            // 检查此VI通道是否有NPU启用
            if (vi_chn_info->npu_attr.npu_enable == 1) {
                struct vi_chn_svp_work_data *work_data = &vi_svp_work_info[dev_id][chn_id];
                int npu_count = 0;
                
                // 收集所有绑定的NPU通道
                for(int i = 0; i < DSS_NPU_MAX_NUM; i++){
                    int target_npu_chn = vi_chn_info->npu_attr.npu_chn_id[i];
                    if(target_npu_chn >= 0 && target_npu_chn < SAMPLE_MAX_NPU_NUM){
                        work_data->npu_chn_ids[npu_count] = target_npu_chn;
                        npu_count++;
                    }
                }
                
                if(npu_count > 0){
                    // 设置工作数据
                    work_data->dev_id = dev_id;
                    work_data->vi_chn_id = vi_chn_info->chn_id;
                    work_data->npu_count = npu_count;
                    work_data->enabled = 1;
                    work_data->run_flag = 1;
                    // 设置取帧通道ID映射
                    work_data->get_frame_chn_id = map_vi_chn_to_get_frame_chn(vi_chn_info->chn_id);
                    
                    // 创建并启动工作线程
                    char thread_name[32];
                    sprintf(thread_name, "svp_dev%d_chn%d", dev_id, chn_id);
                    
                    ret = ak_thread_create(&work_data->thread_id,
                                         vi_svp_thread,
                                         (void*)work_data,
                                         ANYKA_THREAD_NORMAL_STACK_SIZE,
                                         -1);
                    if (ret == AK_SUCCESS) {
                        ak_thread_set_name(thread_name);
                        printf("Created VI SVP work for dev %d chn %d, bound to %d NPU channels\n",
                               dev_id, vi_chn_info->chn_id, npu_count);
                        work_count++;
                    } else {
                        printf("Failed to create SVP thread for dev %d chn %d\n", dev_id, chn_id);
                        work_data->enabled = 0;
                        work_data->run_flag = 0;
                    }
                }
            }
        }
    }
    
    vi_svp_work_count = work_count;
    //printf("SVP setup completed: %d VI channels configured\n", work_count);
    return work_count > 0 ? 0 : -1;
}

// 停止所有VI SVP处理
static int stop_all_vi_svp(void)
{
    printf("Stopping all VI SVP processing\n");
    
    // 停止所有工作线程
    for(int dev_id = 0; dev_id < SAMPLE_MAX_DEV_NUM; dev_id++){
        for(int chn_id = 0; chn_id < 3; chn_id++){
            struct vi_chn_svp_work_data *work_data = &vi_svp_work_info[dev_id][chn_id];
            if(work_data->enabled && work_data->run_flag) {
                work_data->run_flag = 0;
                if(work_data->thread_id != 0) {
                    ak_thread_join(work_data->thread_id);
                    work_data->thread_id = 0;
                }
                work_data->enabled = 0;
                printf("Stopped SVP for dev %d chn %d\n", dev_id, chn_id);
            }
        }
    }
    
    // 销毁所有NPU通道
    for (int i = 0; i < NPU_cnt && i < SAMPLE_MAX_NPU_NUM; i++) {
        if (npu_info[i].npu_info.chn_id >= 0) {
            ak_svp2_destroy_chn(npu_info[i].npu_info.chn_id);
            printf("Destroyed SVP chn %d\n", npu_info[i].npu_info.chn_id);
        }
    }
    
    // 反初始化NPU
    ak_npu_deinit();
    
    printf("All VI SVP processing stopped\n");
    return 0;
}



int load_qs_config(void)
{
    int i;
    int ret = AK_FAILED;
    if(g_qs_fd < 0)
    {
        ak_print_error_ex(MODULE_ID_APP, "/dev/quick_start node is not opened yet!\n");
        return AK_FAILED;
    }

    for(i=0; i<SAMPLE_MAX_DEV_NUM; i++)
    {
        /* get the vi_info, include vi_dev and vi_chn */
        vi_info[i].qs_vi_info.qs_vi_dev_info.dev_id = i;
        ret = ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_GET_VI_INFO, &vi_info[i]);
        if(ret != AK_SUCCESS)
        {		
            if(ret == ERR_ENTRY_NOT_EXIST)
                break;
            else
            {
                ak_print_error_ex(MODULE_ID_APP, "Get qs dev[%d] info failed, ret[%d]!\n", i, ret);
                return AK_FAILED;
            }
        }

        /* if dev is not enable , break the cycle  */
        if(vi_info[i].qs_vi_info.qs_vi_dev_info.enable != 1)
        {
            break;
        }
        CIS_cnt++;

#ifdef DEBUG
        printf("dev:id[%d]\n",vi_info[i].qs_vi_info.qs_vi_dev_info.dev_id);
        printf("dev_enable:[%d]\n", vi_info[i].qs_vi_info.qs_vi_dev_info.enable);
        printf("isp_id[%d]\n", vi_info[i].qs_vi_info.qs_vi_dev_info.isp_id);
        printf("isp_path[%s]\n", vi_info[i].qs_vi_info.qs_vi_dev_info.isp_path);
#endif
        /* generate stitch attr frme the chn attr*/
        int j = 0;
        for(;j<3;j++)
        {
#ifdef DEBUG
            printf("chn_id:[%d]\n", vi_info[i].qs_vi_info.qs_vi_chn_info[j].chn_id);
            printf("chn_enable:[%d]", vi_info[i].qs_vi_info.qs_vi_chn_info[j].enable);
            printf("chn_res:[%d*%d]\n", vi_info[i].qs_vi_info.qs_vi_chn_info[j].res.width, 
                                        vi_info[i].qs_vi_info.qs_vi_chn_info[j].res.height);
            printf("chn_max_res:[%d*%d]\n", vi_info[i].qs_vi_info.qs_vi_chn_info[j].max_res.width, 
                                        vi_info[i].qs_vi_info.qs_vi_chn_info[j].max_res.height);
            printf("chn_frame_depth:[%d]\n", vi_info[i].qs_vi_info.qs_vi_chn_info[j].frame_depth);
            printf("chn_mode:[%d]\n", vi_info[i].qs_vi_info.qs_vi_chn_info[j].mode);
#endif
            if(vi_info[i].qs_vi_info.qs_vi_chn_info[j].stitch_attr.stitch_mode > 0)
            {
                int chn_id = vi_info[i].qs_vi_info.qs_vi_chn_info[j].chn_id;
                int id = find_stitch_id(vi_info[i].qs_vi_info.qs_vi_chn_info[j].stitch_attr.stitch_chn_id);
                g_stitch_attr[id].stitch_chn_id = vi_info[i].qs_vi_info.qs_vi_chn_info[j].stitch_attr.stitch_chn_id;
                g_stitch_attr[id].stitch_mode = vi_info[i].qs_vi_info.qs_vi_chn_info[j].stitch_attr.stitch_mode;
                g_stitch_attr[id].stitch_num = vi_info[i].qs_vi_info.qs_vi_chn_info[j].stitch_attr.stitch_num;
                g_stitch_attr[id].stitch_bind_chn[vi_info[i].qs_vi_info.qs_vi_chn_info[j].stitch_attr.stitch_index] = chn_id;
            }
        }
    }/*endof(for(i=0; i<SAMPLE_MAX_DEV_NUM; i++))*/

#ifdef DEBUG
    printf("STITCH_cnt:[%d]\n", STITCH_cnt);
    for(i=0; i<STITCH_cnt; i++)
    {
        printf("stitch_chn_id[%d]\n", g_stitch_attr[i].stitch_chn_id);
        printf("stitch_mode[%d]\n", g_stitch_attr[i].stitch_mode);
        printf("stitch_num[%d]\n", g_stitch_attr[i].stitch_num);
        printf("stitch_bind_chn[%d:%d:%d:%d]\n", g_stitch_attr[i].stitch_bind_chn[0],
                                                 g_stitch_attr[i].stitch_bind_chn[1],
                                                 g_stitch_attr[i].stitch_bind_chn[2],
                                                 g_stitch_attr[i].stitch_bind_chn[3]);
    }
#endif
    /* 读取编码器信息 */
    for(i=0; i<SAMPLE_MAX_VENC_NUM; i++)
    {
        venc_info_s[i].qs_venc_info.venc_id = i;
        ret = ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_GET_VENC_ATTR, &venc_info_s[i]);
        if(ret != AK_SUCCESS)
        {
            if(ret == ERR_ENTRY_NOT_EXIST)
                break;
            else
            {
                ak_print_error_ex(MODULE_ID_APP, "Get qs venc[%d] info failed, ret[%d]!\n", i, ret );
                return AK_FAILED;
            }
        }

        /* check whether the venc info is valid */
        if(venc_info_s[i].qs_venc_info.venc_id < 0)
            break;

        VENC_cnt++;
#ifdef DEBUG
        printf("venc_id:[%d]\n", venc_info_s[i].qs_venc_info.venc_id);
        printf("venc_width:[%d]\n", venc_info_s[i].qs_venc_info.width);
        printf("venc_height:[%d]\n", venc_info_s[i].qs_venc_info.height);
        printf("venc_encode_type:[%d]\n", venc_info_s[i].qs_venc_info.enc_out_type);
#endif
    }

    /* 读取VIDEO 信息*/
    for(i=0; i<SAMPLE_MAX_VIDEO_NUM; i++)
    {
        video_info[i].index = i;
        ret = ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_GET_VIDEO_INFO, &video_info[i]);
        if(ret != AK_SUCCESS)
        {
            if(ret == ERR_ENTRY_NOT_EXIST)
                break;
            else
            {
                ak_print_error_ex(MODULE_ID_APP, "Get qs video[%d] info failed, ret[%d]!\n", i , ret);
                return AK_FAILED;
            }
        }
        /* check whether the video info is valid */
        if(video_info[i].qs_video_info.chn_id < 0)
            break;

        VIDEO_cnt++;
#ifdef DEBUG
        printf("video_id:[%d]\n", video_info[i].index);
        printf("video_chn_id:[%d]\n", video_info[i].qs_video_info.chn_id);
        printf("video_venc_id:[%d]\n", video_info[i].qs_video_info.venc_id);
        printf("video_frame_rate:[%d]\n", video_info[i].qs_video_info.frame_rate);
        printf("video_frame_depth:[%d]\n", video_info[i].qs_video_info.frame_depth);
        printf("video_encode_mode:[%d]\n", video_info[i].qs_video_info.venc_encode_mode);
#endif
    }

    /* 读取NPU信息 */
    for(i=0; i<SAMPLE_MAX_NPU_NUM; i++)
    {
        npu_info[i].npu_info.chn_id = i;
        ret = ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_GET_NPU_ATTR, &npu_info[i]);
        if(ret != AK_SUCCESS)
        {
            if(ret == ERR_ENTRY_NOT_EXIST)
                break;
            else
            {
                ak_print_error_ex(MODULE_ID_APP, "Get qs video[%d] info failed, ret[%d]!\n", i , ret);
                return AK_FAILED;
            }
        }
        
        NPU_cnt++;
#ifdef DEBUG
        printf("chn_id:[%d]\n", npu_info[i].npu_info.chn_id);
        printf("model_type:[%d]\n", npu_info[i].npu_info.model_type);
        printf("model_path:[%s]\n", npu_info[i].npu_info.model_path);
#endif
    }
    
    if(ret == ERR_ENTRY_NOT_EXIST)
        ret == AK_SUCCESS;

    return ret;
}/* end of func */

// 修改DSS配置函数
int modify_qs_config(void)
{
    int i, j, k;
    bool change = false;
    int width, height;
    int ret = AK_SUCCESS;

    if(qs_change_flag == AK_FALSE)
        return ret;

    // 轮询所有video_info
    for (i = 0; i < VIDEO_cnt; i++) {
        // 检查chn_id是否有效
        if (video_info[i].qs_video_info.chn_id < 0)
            continue;
        // 判断是单目功能还是拼接通道
        if (video_info[i].qs_video_info.chn_id < VIDEO_CHN24) {
            // 单目功能处理
            // 轮询vi_info[].qs_vi_info.qs_vi_chn_info查找匹配的chn_id
            for (j = 0; j < CIS_cnt; j++) {
                for (k = 0; k < 3; k++) {
                    if (vi_info[j].qs_vi_info.qs_vi_chn_info[k].chn_id == video_info[i].qs_video_info.chn_id) {
                        // 找到匹配的qs_vi_chn_info结构体，修改分辨率
                        width = vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.width;
                        height = vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.height;
                        // 只处理主通道数据
                        if(width < 1280)
                            continue;

                        // 根据当前width值进行切换
                        if (width == 2304) {
                            vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.width = 1920;
                        } else if (width == 1920) {
                            vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.width = 1280;
                        } else if (width == 1280) {
                            vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.width = 2304;
                        }
                        
                        // 设置max_res.width为2304
                        vi_info[j].qs_vi_info.qs_vi_chn_info[k].max_res.width = 2304;
                        
                        // 根据当前height值进行切换
                        if (height == 1296) {
                            vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.height = 1080;
                        } else if (height == 1080) {
                            vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.height = 720;
                        } else if (height == 720) {
                            vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.height = 1296;
                        }
                        
                        // 设置max_res.height为1296
                        vi_info[j].qs_vi_info.qs_vi_chn_info[k].max_res.height = 1296;
                        
                        change = true;
                        
                        //修改vi快启相应的配置
                        ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_SET_VI_INFO, &(vi_info[j].qs_vi_info));
                        
                        // 查找匹配的venc_info_s并更新其width和height
                        for (int venc_idx = 0; venc_idx < VENC_cnt; venc_idx++) {
                            if (venc_info_s[venc_idx].qs_venc_info.venc_id == video_info[i].qs_video_info.venc_id) {
                                venc_info_s[venc_idx].qs_venc_info.width = vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.width;
                                venc_info_s[venc_idx].qs_venc_info.height = vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.height;

                                //修改venc快启相应的配置
                                ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_SET_VENC_ATTR, &(venc_info_s[venc_idx].qs_venc_info));

                                break;
                            }
                        }
                        
                        break;
                    }
                }
            }
        } else {
            // 拼接通道处理
            // 轮询vi_info[].qs_vi_info.qs_vi_chn_info.stitch_attr.stitch_chn_id查找匹配的chn_id
            for (j = 0; j < CIS_cnt; j++) {
                for (k = 0; k < 3; k++) {
                    if (vi_info[j].qs_vi_info.qs_vi_chn_info[k].stitch_attr.stitch_chn_id == video_info[i].qs_video_info.chn_id) {
                        // 找到匹配的qs_vi_chn_info结构体，修改分辨率
                        width = vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.width;
                        height = vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.height;
                        // 只处理主通道数据
                        if(width < 1280)
                            continue;

                        // 根据当前width值进行切换
                        if (width == 2304) {
                            vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.width = 1920;
                        } else if (width == 1920) {
                            vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.width = 1280;
                        } else if (width == 1280) {
                            vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.width = 2304;
                        }
                        
                        // 设置max_res.width为2304
                        vi_info[j].qs_vi_info.qs_vi_chn_info[k].max_res.width = 2304;
                        
                        // 根据当前height值进行切换
                        if (height == 1296) {
                            vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.height = 1080;
                        } else if (height == 1080) {
                            vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.height = 720;
                        } else if (height == 720) {
                            vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.height = 1296;
                        }
                        
                        // 设置max_res.height为1296
                        vi_info[j].qs_vi_info.qs_vi_chn_info[k].max_res.height = 1296;

                        change = true;

                        //修改vi快启相应的配置
                        ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_SET_VI_INFO, &(vi_info[j].qs_vi_info));

                        // 查找匹配的venc_info_s并更新其width和height
                        for (int venc_idx = 0; venc_idx < VENC_cnt; venc_idx++) {
                            if (venc_info_s[venc_idx].qs_venc_info.venc_id == video_info[i].qs_video_info.venc_id) {
                                venc_info_s[venc_idx].qs_venc_info.width = vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.width;
                                venc_info_s[venc_idx].qs_venc_info.height = vi_info[j].qs_vi_info.qs_vi_chn_info[k].res.height * 2;  // 高度设为2倍

                                //修改venc快启相应的配置
                                ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_SET_VENC_ATTR, &(venc_info_s[venc_idx].qs_venc_info));

                                break;
                            }
                        }
                        
                        break;
                    }
                }
            }
        }
    }

    //如果配置有修改，提交到flash
    if (change){
        ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_INFO_COMMIT, NULL);
    }

    return AK_SUCCESS;
} /*end of modify_qs_config*/

// 修改DSS fastae配置函数
int modify_qs_fastae_config(void)
{
    int i;
    bool change = false;
    int fastae_mode;
    int ret = AK_SUCCESS;

    if(qs_fastae_change_flag == AK_FALSE)
        return ret;

    // 轮询所有vi_info
    for (i = 0; i < CIS_cnt; i++) {
        // 检查vi是否启用
        if(vi_info[i].qs_vi_info.qs_vi_dev_info.enable != 1) {
            continue;
        }

        fastae_mode = vi_info[i].qs_vi_info.qs_vi_dev_info.fastae_info.fastae_mode;

        //根据当前的fastae_mode值来切换
        if (fastae_mode == 0) {
            vi_info[i].qs_vi_info.qs_vi_dev_info.fastae_info.fastae_mode = 2;
        } else if (fastae_mode == 2) {
            vi_info[i].qs_vi_info.qs_vi_dev_info.fastae_info.fastae_mode = 0;
        } else if (fastae_mode != -1){
            vi_info[i].qs_vi_info.qs_vi_dev_info.fastae_info.fastae_mode = 0;
        }

        change = true;
        
        //修改vi快启相应的配置
        ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_SET_VI_INFO, &(vi_info[i].qs_vi_info));
    }

    //如果配置有修改，提交到flash
    if(change){
        ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_INFO_COMMIT, NULL);
    }

    return AK_SUCCESS;
}

// 修改DSS night mode配置函数
int modify_qs_night_mode_config(void)
{
    int i;
    bool change = false;
    int night_mode_light_select;
    int ret = AK_SUCCESS;

    if(qs_night_mode_change_flag == AK_FALSE)
        return ret;

    // 轮询所有vi_info
    for (i = 0; i < CIS_cnt; i++) {
        // 检查vi是否启用
        if(vi_info[i].qs_vi_info.qs_vi_dev_info.enable != 1) {
            continue;
        }

        night_mode_light_select = vi_info[i].qs_vi_info.qs_vi_dev_info.fastae_info.night_mode_light_select;

        //根据当前的night_mode_light_select值来切换
        if (night_mode_light_select == 0) { 
            vi_info[i].qs_vi_info.qs_vi_dev_info.fastae_info.night_mode_light_select = 2;
        //} else if (night_mode_light_select == 1) {
        //1不能用，昼夜切换，使用白光灯，切到夜视为全彩模式，quick start驱动默认使用第三个ISP conf子文件。目前ISP conf只有两份，会跑不起来
        //    vi_info[i].qs_vi_info.qs_vi_dev_info.fastae_info.night_mode_light_select = 2;
        } else if (night_mode_light_select == 2) {
            vi_info[i].qs_vi_info.qs_vi_dev_info.fastae_info.night_mode_light_select = 3;
        } else if (night_mode_light_select == 3) {
            vi_info[i].qs_vi_info.qs_vi_dev_info.fastae_info.night_mode_light_select = 4;
        } else if (night_mode_light_select == 4) {
            vi_info[i].qs_vi_info.qs_vi_dev_info.fastae_info.night_mode_light_select = 0;
        } else if (night_mode_light_select != -1){
            vi_info[i].qs_vi_info.qs_vi_dev_info.fastae_info.night_mode_light_select = 0;
        }

        change = true;
        
        //修改vi快启相应的配置
        ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_SET_VI_INFO, &(vi_info[i].qs_vi_info));
    }

    //如果配置有修改，提交到flash
    if(change){
        ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_INFO_COMMIT, NULL);
    }

    return AK_SUCCESS;
}

// 修改DSS mirror配置函数
int modify_qs_mirror_config(void)
{
    int i;
    bool change = false;
    AK_U32 mirror_en;
    int ret = AK_SUCCESS;

    if(qs_mirror_change_flag == AK_FALSE)
        return ret;

    // 轮询所有vi_info
    for (i = 0; i < CIS_cnt; i++) {
        // 检查vi是否启用
        if(vi_info[i].qs_vi_info.qs_vi_dev_info.enable != 1) {
            continue;
        }

        mirror_en = vi_info[i].qs_vi_info.qs_vi_dev_info.mirror_en;

        //根据当前的mirror_en值来切换
        if (mirror_en == 0) {
            vi_info[i].qs_vi_info.qs_vi_dev_info.mirror_en = 1;
        } else if (mirror_en == 1) {
            vi_info[i].qs_vi_info.qs_vi_dev_info.mirror_en = 0;
        } 

        change = true;

        //修改vi快启相应的配置
        ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_SET_VI_INFO, &(vi_info[i].qs_vi_info));
    }

    //如果配置有修改，提交到flash
    if(change){
        ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_INFO_COMMIT, NULL);
    }

    return AK_SUCCESS;
}

// 修改DSS flip配置函数
int modify_qs_flip_config(void)
{
    int i;
    bool change = false;
    AK_U32 flip_en;
    int ret = AK_SUCCESS;
    
    if(qs_flip_change_flag == AK_FALSE)
        return ret;
    
     // 轮询所有vi_info
    for (i = 0; i < CIS_cnt; i++) {
        // 检查vi是否启用
        if(vi_info[i].qs_vi_info.qs_vi_dev_info.enable != 1) {
            continue;
        }

        flip_en = vi_info[i].qs_vi_info.qs_vi_dev_info.flip_en;

        //根据当前的flip_on值来切换
        if (flip_en == 0) {
            vi_info[i].qs_vi_info.qs_vi_dev_info.flip_en = 1;
        } else if (flip_en == 1) {
            vi_info[i].qs_vi_info.qs_vi_dev_info.flip_en = 0;
        } 

        change = true;
        
        //修改vi快启相应的配置
        ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_SET_VI_INFO, &(vi_info[i].qs_vi_info));
    }

    //如果配置有修改，提交到flash
    if(change){
        ioctl(g_qs_fd, IO_ISP_QUICK_START_DSS_INFO_COMMIT, NULL);
    }

    return AK_SUCCESS;
}

void print_important_configs(char* prefix)
{
    int i;

    printf("%s:\n", prefix);

    for (i = 0; i < CIS_cnt; i++) {
        // 检查vi是否启用
        if(vi_info[i].qs_vi_info.qs_vi_dev_info.enable != 1) {
            continue;
        }

        printf("vi %d:\n", i);  
        printf("fastae_mode %d, ", vi_info[i].qs_vi_info.qs_vi_dev_info.fastae_info.fastae_mode);
        printf("night_mode_light_select %d, ", vi_info[i].qs_vi_info.qs_vi_dev_info.fastae_info.night_mode_light_select);
        printf("mirror_en %d, ", vi_info[i].qs_vi_info.qs_vi_dev_info.mirror_en);
        printf("flip_en %d\n", vi_info[i].qs_vi_info.qs_vi_dev_info.flip_en);
    }

    printf("\n");
}

int main(int argc, char **argv)
{
    /* init sdk running */
    sdk_run_config config;
    struct ak_timeval cur_tv;
    int i, ret;
    memset(&config, 0, sizeof(config));             //init the struct

    config.mem_trace_flag = SDK_RUN_NORMAL;
    config.isp_tool_server_flag = 0;                //isp tool sever
    ak_sdk_init( &config );

    if( parse_option( argc, argv ) == AK_FALSE )
    {                                               //解释和配置选项
        return AK_FAILED;
    }

    ak_print_set_level(MODULE_ID_ALL,LOG_LEVEL_ERROR);
    ak_print_set_syslog_level(MODULE_ID_ALL,LOG_LEVEL_ERROR);

    //保存上次的配置，仅作内部测试使用
    if(qs_change_flag == AK_TRUE)
    {
        // 执行先删除，再备份的命令
        int result = system("rm -f /etc/config/dss.bin.backup;cp /etc/config/dss.bin /etc/config/dss.bin.backup");  
        if (result == -1) {
            ak_print_error_ex(MODULE_ID_APP, "backup dss.bin failed!\n");
            return AK_FAILED;
        }
    }

    /*打开快启设备节点*/
    g_qs_fd = open(QS_DEV_NODE, O_RDWR, 0);
    if(g_qs_fd < 0)
    {
        ak_print_error_ex(MODULE_ID_APP, "Open /dev/quick_start node failed!\n");
        return AK_FAILED;
    }

    /* 从quick_start 设备节点读取快启配置 */
    ret = load_qs_config();
    if(ret != AK_SUCCESS)
    {
        ak_print_error_ex(MODULE_ID_APP, "load quick start config from qs dev node failed, ret[%d]\n", ret);
        system("dmesg");
        return AK_FAILED;
    }

    // 禁用快启的SVP检测
    disable_qs_svp();

    //正常模式，开启vi venc
    ret = start_vi_venc_normal();
    if(ret){
        ak_print_error_ex(MODULE_ID_APP,"start vi venc normal fail!\n");
        goto error;
    }

    ak_get_ostime(&cur_tv);
    printf("finish vi init ts: %d\n", cur_tv.sec*1000 + cur_tv.usec/1000);

    // 禁用快启的SVP检测
    //disable_qs_svp();

    //获取快启阶段SVP检测结果
    get_driver_svp_result();

    // 初始化所有NPU通道
    init_all_npu_channels();

    // 设置VI通道的SVP处理（按VI通道为单位）
    setup_vi_channels_svp();

    //挂卡
    mount_tf_card();

    //修改dss config
    modify_qs_config();

    print_important_configs("begin");

    //修改dss fastae config
    modify_qs_fastae_config();

    //修改dss night mode config
    modify_qs_night_mode_config();

    //修改dss mirror config
    modify_qs_mirror_config();

    //修改dss flip config
    modify_qs_flip_config();

    print_important_configs("end");

    //快启码流保存
    fast_stream_save();

    /* 循环测试 */
    while (1)
    {
        ak_sleep_ms(20);
    }


error:
//------反初始化-------
    /* stop the video thread */
    for(i=0;i<SAMPLE_MAX_VIDEO_NUM; i++)
    {
        if(g_video_attr[i].th_run_flag == AK_TRUE)
            g_video_attr[i].th_run_flag = AK_FALSE;
    }
    for(i=0;i<SAMPLE_MAX_VIDEO_NUM; i++)
    {
        if(g_video_attr[i].pid != 0)
            ak_thread_join(g_video_attr[i].pid);
    }

    /* close venc */	
    for(i=0;i<VENC_cnt; i++)
    {
        ak_venc_close(g_video_attr[i].venc_id);
    }

    /* disable stitch chn */
    for(i=0;i<STITCH_cnt;i++)
    {
        ak_vi_disable_chn(g_stitch_attr[i].stitch_chn_id);
    }

    /* disable vi */
    for(i = 0; i < CIS_cnt; i++)
    {
        disable_vi(i);
    }

    /* stop stitch chn */
    for(i=0;i<STITCH_cnt;i++)
    {
        stop_stitch_chn(g_stitch_attr[i].stitch_chn_id);
    }

    /* stop vi */
    for(i = 0; i < CIS_cnt; i++)
    {
        stop_vi(i);
    }

    /* close quick_start dev node */
    if(g_qs_fd > 0)close(g_qs_fd);

    return 0;
}
