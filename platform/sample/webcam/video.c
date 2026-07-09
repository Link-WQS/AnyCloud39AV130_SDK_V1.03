/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_video_sample.c
* Description: This is a simple example to show how the video module working.
* Notes: Before running please insmod sensor_xxx.ko ak_isp.ko ak_venc_adapter.ko ak_venc_bridge.ko at first.
* History: V1.0.0
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include "ak_common.h"
#include "ak_log.h"
#include "ak_common_video.h"
#include "ak_venc.h"
#include "ak_thread.h"
#include "ak_mem.h"
#include "ak_vi.h"
#include "ak_video.h"
#include "ak_vpss.h"
#include "list.h"
#include "uvc.h"
#include "video.h"

#define THREAD_PRIO -1
#define __STATIC__
#define PRINT(fmt, ...) printf("[video] " fmt, ##__VA_ARGS__)

/* video resolution num */
#define DE_VIDEO_SIZE_MAX       7
// #define DUAL_CIS_MODE           0

#define ENC_TYPE_MAX            3
#define MAX_ENC_NUM             4

#define THIRD_CHN_SUPPORT       0

#define CHN_MODE                FRAME_MODE
// #define CHN_MODE                SLICE_MODE
#define DEF_FRAME_DEPTH         2
#define DEF_SLICE_DEPTH         4

// #define VIDEO_RESOLUTION        DE_VIDEO_720P

/* this is length for parse */
#define LEN_HINT                512
#define LEN_OPTION_SHORT        512

/* decoder resolution */
static const struct resolution_t resolutions[DE_VIDEO_SIZE_MAX] = {
    {320,   240,   "DE_VIDEO_240P"},
    {480,   320,   "DE_VIDEO_320P"},
    {640,   480,   "DE_VIDEO_VGA"},
    {800,   480,   "DE_VIDEO_480P"},
    {1280,  720,   "DE_VIDEO_720P"},
    {1920,  1080,  "DE_VIDEO_1080P"},
    {2560,  1440,  "DE_VIDEO_1440P"},
};

static const struct dev_ch_t dev_ch[VIDEO_DEV_MUX] = {
    {VIDEO_CHN0,VIDEO_CHN1},
    {VIDEO_CHN2,VIDEO_CHN3},
};

static const enum profile_mode encoder_profile [ENC_TYPE_MAX]  = {
    PROFILE_MAIN,       //H264
    PROFILE_JPEG,       //jpeg
    PROFILE_HEVC_MAIN,  //H265
};

struct video_param_t{
    unsigned short fps;
    unsigned int max_width;
    unsigned int max_height;
    VI_CHN  ch_id;
    char *isp_cfg;
    unsigned short goplen;
    unsigned short target_kbps;
    unsigned short max_kbps;
    enum bitrate_ctrl_mode br_mode;
    int venc_handle;

    struct list_head *p_head;
    char* uvc_devname;
    struct uvc_device* udev;
    ak_pthread_t uvc_ctrl_th;
    ak_pthread_t uvc_stream_th;
    ak_mutex_t stream_lock;
    int is_streaming;
};

typedef struct stream_node{
    struct video_stream stream;
    struct list_head list;
}stream_list;

static struct video_param_t video = {
    .fps = 25,
    .max_width = 640,
    .max_height = 480,
    .isp_cfg = "/etc/config/isp_sc2337p_mipi_2lane_av130_dual.conf",
    .ch_id = VIDEO_CHN0,
    .goplen = 25*2,
    .target_kbps = 1024,
    .max_kbps = 1024,
    .br_mode  = BR_MODE_VBR,
    .uvc_devname = "/dev/video8",
    .is_streaming = 0,
};
#define  UVC_STREAMING_MAXPKT   (512)

static char *pc_prog_name = NULL;                      //demo名称

static int main_res       = DE_VIDEO_720P;
static int sub_res        = DE_VIDEO_480P;

static int CIS_no = -1;
static int CIS_num = 1;
static enum encode_output_type encoder_type = MJPEG_ENC_TYPE;
static int bulk_or_iso_mode = 0;
static int stitch_mode = DISABLED_STITCH;
static int stitch_chn_id = 0;

static int start_vi(int dev_id,int dual_cis_mode,ch_resolutions main,ch_resolutions sub)
{
    /*
     * step 0: global value initialize
     */
    int ret = -1;                                //return value
    unsigned int width = resolutions[main].width;
    unsigned int height = resolutions[main].height;
    unsigned int subwidth = resolutions[sub].width;;
    unsigned int subheight = resolutions[sub].height;
    int chn_main_id = dev_ch[dev_id].main_ch;
    int chn_sub_id = dev_ch[dev_id].sub_ch;

    /* open vi flow */
    /*
     * step 1: open video input device
     */
    ret = ak_vi_open(dev_id);
    if (AK_SUCCESS != ret) {
        PRINT("vi device %d open failed\n", dev_id);
        return ret;
    }
    /*
     * step 2: load isp config
     */
    ret = ak_vi_load_sensor_cfg(dev_id, video.isp_cfg);
    if (AK_SUCCESS != ret) {
        PRINT("vi device %d load isp cfg [%s] failed!\n", dev_id, video.isp_cfg);
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
        PRINT("Can't get dev[%d]resolution\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    } else {
        PRINT("get dev res w:[%d]h:[%d]\n",res.width, res.height);
        dev_attr.crop.width = res.width;
        dev_attr.crop.height = res.height;
    }

    /*
     * step 4: set vi device working parameters
     * default parameters: 25fps, day mode
     */
    ret = ak_vi_set_dev_attr(dev_id, &dev_attr);
    if (ret) {
        PRINT("vi device %d set device attribute failed!\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    }

    /*
     * step 5: set main channel attribute
     */
    VI_CHN_ATTR_EX chn_attr;
    memset(&chn_attr, 0, sizeof(VI_CHN_ATTR_EX));
    chn_attr.chn_id = chn_main_id;
    chn_attr.res.width = width;
    chn_attr.res.height = height;
    chn_attr.mode = CHN_MODE;
#if (CHN_MODE == FRAME_MODE)
    chn_attr.frame_depth = DEF_FRAME_DEPTH;
#else
    chn_attr.frame_depth = DEF_SLICE_DEPTH;
#endif
    /*disable frame control*/
    //chn_attr.frame_rate = 0;

    ret = ak_vi_set_chn_attr_ex(chn_main_id, &chn_attr);
    if (ret) {
        PRINT( "vi device %d set channel [%d] attribute failed!\n", dev_id, chn_main_id);
        ak_vi_close(dev_id);
        return ret;
    }
    PRINT("vi device %d main sub channel attribute\n", dev_id);
    /*
     * step 6: set sub channel attribute
     */

    memset(&chn_attr, 0, sizeof(VI_CHN_ATTR_EX));
    chn_attr.chn_id = chn_sub_id;
    chn_attr.res.width = subwidth;
    chn_attr.res.height = subheight;
    chn_attr.frame_depth = DEF_FRAME_DEPTH;
    /*disable frame control*/
    //chn_attr.frame_rate = 0;

    ret = ak_vi_set_chn_attr_ex(chn_sub_id, &chn_attr);
    if (ret) {
        PRINT("vi device %d set channel [%d] attribute failed!\n", dev_id, chn_sub_id);
        ak_vi_close(dev_id);
        return ret;
    }
    PRINT(
        "vi device %d set sub channel attribute\n", dev_id);
#if 0
    /*
     * step 8: enable vi device
     */
    ret = ak_vi_enable_dev(dev_id);
    if (ret) {
        PRINT("vi device %d enable device  failed!\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    }
    /*
     * step 9: enable vi main channel
     */
    ret = ak_vi_enable_chn(chn_main_id);
    if(ret)
    {
        PRINT("vi channel[%d] enable failed!\n", chn_main_id);
        ak_vi_close(dev_id);
        return ret;
    }

    /*
     * step 10: enable vi sub channel
     */
    ret = ak_vi_enable_chn(chn_sub_id);
    if(ret)
    {
        PRINT("vi channel[%d] enable failed!\n",chn_sub_id);
        ak_vi_close(dev_id);
        return ret;
    }
#endif
    return AK_SUCCESS;
}
/* end of func */

static int enable_vi(int dev_id)
{
    int ret = -1;                                //return value
    int chn_main_id = dev_ch[dev_id].main_ch;
    int chn_sub_id = dev_ch[dev_id].sub_ch;

    /*
     * step 8: enable vi device
     */
    ret = ak_vi_enable_dev(dev_id);
    if (ret) {
        PRINT("vi device %d enable device  failed!\n", dev_id);
        ak_vi_close(dev_id);
        return ret;
    }
    /*
     * step 9: enable vi main channel
     */
    ret = ak_vi_enable_chn(chn_main_id);
    if(ret)
    {
        PRINT("vi channel[%d] enable failed!\n", chn_main_id);
        ak_vi_close(dev_id);
        return ret;
    }

    /*
     * step 10: enable vi sub channel
     */
    ret = ak_vi_enable_chn(chn_sub_id);
    if(ret)
    {
        PRINT("vi channel[%d] enable failed!\n",chn_sub_id);
        ak_vi_close(dev_id);
        return ret;
    }

    return ret;
}

static int start_stitch_chn(int chn_id, int stitch_mode)
{
    int ret=AK_FAILED;
    VI_STITCH_ATTR stitch_attr = {0};
    stitch_attr.mode = stitch_mode;
    stitch_attr.stitch_chn_num = CIS_num;

    if(chn_id %2 == 0)
    {
        stitch_attr.stitch_bind_chn[0] = VIDEO_CHN0;
        stitch_attr.stitch_bind_chn[1] = VIDEO_CHN2;
        // if(CIS_num == 3)
        //     stitch_attr.stitch_bind_chn[2] = VIDEO_CHN4;

        //stitch main channel
        ret = ak_vi_create_stitch_chn(VIDEO_CHN24, NULL, &stitch_attr);
    }
    else
    {
        stitch_attr.stitch_bind_chn[0] = VIDEO_CHN1;
        stitch_attr.stitch_bind_chn[1] = VIDEO_CHN3;
        // if(CIS_num == 3)
        //     stitch_attr.stitch_bind_chn[2] = VIDEO_CHN5;

        //stitch sub channel
        ret = ak_vi_create_stitch_chn(VIDEO_CHN25, NULL, &stitch_attr);
    }

    if(ret != AK_SUCCESS)
    {
        ak_print_error_ex(MODULE_ID_APP,
            "create stitch_chn [%d] failed!\n", chn_id);
    }
    return ret;
}

static int wc_video_init(void)
{
    int ret;

    ret = start_vi(VIDEO_DEV0, CIS_num == 1 ? 0 : 1, main_res, sub_res);

    if (CIS_num > 1)
        ret |= start_vi(VIDEO_DEV1, CIS_num == 1 ? 0 : 1, main_res, sub_res);

     /* create stitch chn */
    if ((CIS_num > 1) && (stitch_mode != DISABLED_STITCH)) {
        ret |= start_stitch_chn(stitch_chn_id, stitch_mode);
        // if(ret != AK_SUCCESS)
        //     goto erro;
    }

    ret |= enable_vi(VIDEO_DEV0);
    if (CIS_num > 1)
        ret |= enable_vi(VIDEO_DEV1);

    if ((CIS_num > 1) && (stitch_mode != DISABLED_STITCH)) {
        if(stitch_chn_id %2 == 0)
            ak_vi_enable_chn(VIDEO_CHN24);
        else
            ak_vi_enable_chn(VIDEO_CHN25);
    }

    if(ret) {
        PRINT("vi init failed!\n");
    }
    return ret;
}


static int stop_vi(int dev_id)
{
    int ret = -1;
    int chn_main_id = dev_ch[dev_id].main_ch;
    int chn_sub_id = dev_ch[dev_id].sub_ch;

    ak_vi_disable_chn(chn_main_id);
    ak_vi_disable_chn(chn_sub_id);

    ak_vi_disable_dev(dev_id);
    ret = ak_vi_close(dev_id);

    return ret;
}

static int video_deinit(void){

    int ret;

    ret = stop_vi(VIDEO_DEV0);
    if (CIS_num > 1)
        ret |= stop_vi(VIDEO_DEV1);

    if(ret)
    {
        PRINT( "vi deinit failed!\n");
    }
    return ret;
}

#define STREAM_BUF_SIZE(w,h)     ( (w) * (h) * 3 / 4 )
static int start_venc(enum encode_output_type encoder_type,ch_resolutions res)
{
    int ret = -1;
    int width = resolutions[res].width;
    int height = resolutions[res].height;
    int handle_id;

    if (stitch_mode == VERTICAL_STITCH) {
        height += resolutions[res].height;
    }

    /* open venc */
    struct venc_param ve_param = {0};
    ve_param.width  = width;            //resolution width
    ve_param.height = height;           //resolution height

    ve_param.fps    = video.fps;               //fps set
    ve_param.goplen = video.goplen;               //gop set
    ve_param.target_kbps = video.target_kbps;         //k bps
    ve_param.max_kbps    = video.max_kbps;        //max kbps
    ve_param.br_mode     = video.br_mode; //br mode
    ve_param.minqp       = 25;          //qp set
    ve_param.maxqp       = 50;          //qp max value
    ve_param.initqp       = 70;    //qp value
    ve_param.jpeg_qlevel = JPEG_QLEVEL_DEFAULT;     //jpeg qlevel
    ve_param.chroma_mode = CHROMA_4_2_0;            //chroma mode
    ve_param.max_picture_size = STREAM_BUF_SIZE(width, height)/1024; //0 means default
    ve_param.enc_level        = 30;                 //enc level
    ve_param.smart_mode       = 0;                  //smart mode set
    ve_param.smart_goplen     = 100;                //smart mode value
    ve_param.smart_quality    = 50;                 //quality
    ve_param.smart_static_value = 0;                //value
    ve_param.enc_out_type = encoder_type;           //enc type
    ve_param.profile = encoder_profile[encoder_type];

    struct venc_rc_param rc_param = {0};
    rc_param.enable_MMA = 1;

    ret = ak_venc_open_ex(&ve_param, &rc_param,  &handle_id);

    if (ret || (-1 == handle_id) )
    {
        PRINT("open venc failed\n");
        return ret;
    }
/*
    if (ve_param.width > 1920 && ve_param.height > 1080 && ve_param.initqp > 60)
    {
        ak_venc_set_stream_buff(handle_id, 1*1024*1024);
    }
*/

    /* recode the file */
    //enc_pair.file = file;
    video.venc_handle = handle_id;
    video.p_head = ak_mem_alloc(MODULE_ID_VENC, sizeof(struct list_head));
    if(NULL == video.p_head){
        PRINT( "Alloc p_head failed!\n");
        return -1;
    }
    PRINT( "alloc video->p_head =%p!\n",video.p_head);
    INIT_LIST_HEAD(video.p_head);

    return AK_SUCCESS;
}
static void set_mipi_switch(int is_ir)
{
    struct vpss_isp_3d_nr_attr default_nr_3d_attr = {0};
    struct vpss_isp_3d_nr_attr off_nr_3d_attr = {0};
    int cur_fps = 0;

    ak_vpss_get_3d_nr_attr(0, &default_nr_3d_attr);
    memcpy(&off_nr_3d_attr, &default_nr_3d_attr, sizeof(struct vpss_isp_3d_nr_attr));
    off_nr_3d_attr._3d_nr_mode = VPSS_OP_TYPE_MANUAL;
    off_nr_3d_attr.manual_3d_nr.tnr_enable = 0;
    ak_vpss_set_3d_nr_attr(0, &off_nr_3d_attr);
    ak_sleep_ms(60);
    if (is_ir) {
        system("echo 1 > /sys/class/gpio/gpio3/value");
    } else {
        system("echo 0 > /sys/class/gpio/gpio3/value");
    }
    ak_vpss_set_3d_nr_attr(0, &default_nr_3d_attr);
}

static int fs_isp_camera_set_mipi_switch(int is_ir)
{
    set_mipi_switch(is_ir);

    return 0;
}

static int change_resolution(int ch, int venc_handle, int width, int height)
{
    VI_CHN_ATTR_EX chn_attr[2];
    struct venc_param param;

    memset(&chn_attr[0], 0, sizeof(VI_CHN_ATTR_EX));
    ak_vi_get_chn_attr_ex(ch, &chn_attr[0]);
    if (CIS_num > 1) {
        memset(&chn_attr[1], 0, sizeof(VI_CHN_ATTR_EX));
        ak_vi_get_chn_attr_ex(ch + 2, &chn_attr[1]);
        if (stitch_mode != DISABLED_STITCH) {
            if(ch % 2 == 0)
                ak_vi_disable_chn(VIDEO_CHN24);
            else
                ak_vi_disable_chn(VIDEO_CHN25);
        }
    }
    ak_vi_disable_chn(ch);
    if (CIS_num > 1) {
        ak_vi_disable_chn(ch + 2);
    }
   #if video_ROTATE
    chn_attr[0].res.width = height;
    chn_attr[0].res.height = width;
    if (CIS_num > 1) {
        chn_attr[1].res.width = height;
        chn_attr[1].res.height = width;
    }
   #else
    chn_attr[0].res.width = width;
    chn_attr[0].res.height = height;
    if (CIS_num > 1) {
        chn_attr[1].res.width = width;
        chn_attr[1].res.height = height;
    }
   #endif
    ak_vi_set_chn_attr_ex(ch, &chn_attr[0]);
    if (CIS_num > 1) {
        ak_vi_set_chn_attr_ex(ch + 2, &chn_attr[1]);
    }
    ak_vi_enable_chn(ch);
    if (CIS_num > 1) {
        ak_vi_enable_chn(ch + 2);
        if (stitch_mode != DISABLED_STITCH) {
            if(ch % 2 == 0)
                ak_vi_enable_chn(VIDEO_CHN24);
            else
                ak_vi_enable_chn(VIDEO_CHN25);
        }
    }
    memset(&param, 0, sizeof(struct venc_param));
    ak_venc_get_attr(venc_handle,&param);
    param.width = width;
    param.height = height;
    if (stitch_mode == VERTICAL_STITCH) {
        param.height += height;
    }
    ak_venc_set_attr(venc_handle,&param);
}

void video_switch_video_dev(int vi_mode)
{
    if (vi_mode)
        fs_isp_camera_set_mipi_switch(1);
    else
        fs_isp_camera_set_mipi_switch(0);

    ak_vi_switch_mode(VIDEO_DEV0,vi_mode);
}

static void video_rotate_nv12(uint8_t *dst, uint8_t *src, uint32_t src_width, uint32_t src_height, int rotate)
{
    uint32_t i, j, k;
    uint32_t y_size = src_width * src_height;
    uint32_t uv_width = src_width / 2;
    uint32_t uv_height = src_height / 2;
    uint8_t *pNvSrc = src + y_size;
    uint8_t *pNvDst = dst + y_size;

    switch (rotate) {
    case 90:
        for (j = 0; j < src_height; j++) {
            for (i = 0; i < src_width; i++) {
                k = (src_height * i) + ((src_height - 1) - j);
                *(dst + k) = *src++;
            }
        }

        for (j = 0; j < uv_height; j++) {
            for (i = 0; i < uv_width; i++) {
                k = ((uv_height * i) + ((uv_height - 1) - j)) * 2;
                *(pNvDst + k) = *pNvSrc++;
                *(pNvDst + k + 1) = *pNvSrc++;
            }
        }
        break;
    default:
        break;
    }
}

void init_mipi_switch(void)
{
    system("echo 3 > /sys/class/gpio/export");
    system("echo out > /sys/class/gpio/gpio3/direction");
    system("echo 1 > /sys/class/gpio/gpio3/value");
}

/* end of func */

/* get the encode data from vi module */
static void *video_uvc_control_th(void *arg)
{
    int ret;
    __u32 events;
    int video_dev = 0;
    ak_thread_set_name("uvc_ctrl_th");
    struct video_param_t *video = (struct video_param_t *)arg; //venc handle
    /* for uvc */
    fd_set cfds;
    while (1) {
        /* for uvc */
        FD_ZERO(&cfds);
        /* We want both setup and data events on UVC interface.. */
        FD_SET(video->udev->uvc_fd, &cfds);
        ret = select(video->udev->uvc_fd + 1, NULL, NULL, &cfds, NULL);
        if (-1 == ret) {
            PRINT("select error %d, %s\n", errno, strerror(errno));
            continue;
        }
        if (0 == ret) {
            PRINT("select timeout\n");
            continue;
        }
        if (FD_ISSET(video->udev->uvc_fd, &cfds)) {
            events = uvc_events_process(video->udev);
            if(events == UVC_EVENT_STREAMON && video->udev->is_streaming){
                //video_switch_video_dev(video_dev);
               // video_dev= !video_dev;
                PRINT("%s change resolution\n", __func__);
                change_resolution(video->ch_id,video->venc_handle,video->udev->width,video->udev->height);
                video->is_streaming = 1;
            }else if(events == UVC_EVENT_STREAMOFF && !video->udev->is_streaming){
                video->is_streaming = 0;
            }
        }
    }
    ak_thread_exit();
    return NULL;
}

static int video_encode_from_vi(struct video_param_t *video)
{
    int ret;
    struct video_input_frame frame;
    VI_CHN chn_id = video->ch_id;
    int len = 0;

    memset(&frame, 0x00, sizeof(frame));
    ret = ak_vi_get_frame(chn_id, &frame);
    if (!ret)
    {
        /* send it to encode */
        #if video_ROTATE
        void *data_tmp = ak_mem_alloc(MODULE_ID_APP,frame.vi_frame.len);
        if(data_tmp){
            memcpy(data_tmp,frame.vi_frame.data,frame.vi_frame.len);
            video_rotate_nv12(frame.vi_frame.data,data_tmp,wcam->udev->height,wcam->udev->width,90);
            ak_mem_free(data_tmp);
        }else{
            ak_sleep_ms(5);
            continue;
        }
        #endif
        struct video_stream *stream = ak_mem_alloc(MODULE_ID_APP, sizeof(struct video_stream));
        ret = ak_venc_encode_frame(video->venc_handle, frame.vi_frame.data, frame.vi_frame.len, frame.mdinfo, stream);
        if (AK_SUCCESS == ret)
        {
            //PRINT("stream->len=%d\n",stream->len);
            if(stream->len > 0)
            {
                //memcpy(buf,stream->data, stream->len);
                //len = stream->len;
                uvc_video_process(video->udev,stream->data, stream->len);
            }else{
                ak_print_notice_ex(MODULE_ID_VENC, "encode err, maybe drop\n");
            }
            ak_venc_release_stream(video->venc_handle, stream);
        }else {
            /* send to encode failed */
            ak_print_error_ex(MODULE_ID_APP, "send to encode failed\n");
        }
        ak_mem_free(stream);
        ak_vi_release_frame(chn_id, &frame);
    }
    return len;
}

static void *video_uvc_stream_th(void *arg)
{
    int ret;
    __u32 events;
    struct video_param_t *video = (struct video_param_t *)arg; //venc handle
    fd_set dfds;

    ak_thread_set_name("uvc_stream_th");
    while (1) {
        /* for uvc */
        FD_ZERO(&dfds);
        /* We want both setup and data events on UVC interface.. */
        FD_SET(video->udev->uvc_fd, &dfds);
        ret = select(video->udev->uvc_fd + 1, NULL, &dfds, NULL, NULL);
        if (-1 == ret) {
            PRINT("select error %d, %s\n", errno, strerror(errno));
            continue;
        }
        if (0 == ret) {
            PRINT("select timeout\n");
            continue;
        }
        if (FD_ISSET(video->udev->uvc_fd, &dfds)) {
            //PRINT("video->is_streaming=%d\n",video->is_streaming);
            if (video->is_streaming) {
                video_encode_from_vi(video);
                //ret = uvc_video_process(video->udev);
                //PRINT("uvc_video_process\n");
            }else{
                ak_sleep_ms(5);
            }
        }
    }
    ak_thread_exit();
    return NULL;
}
/* end of func */

/* this is the message to print */
static char ac_option_hint[  ][ LEN_HINT ] = {                                         //操作提示数组
    "打印帮助信息" ,
    "编码输出数据格式, [h264, h265, jpeg], 37e only support jpeg, 300L not support h264，default h264",
    "编码模式 0:cbr , 1:vbr, 2:CONST_QP, 4:AVBR (2,4 only av100 support), default 1:vbr",
    "目标码率, 单位:kbps, [大于 0], default 1024",
    "帧率, 大于 0，小于sensor实际帧率, default 30",
    "主通道分辨率, [0, 6], default 4",
    "次通道分辨率 [0, 3]. note:need smaller than main channel resolution , default 0",
    "isp conf file path",
    "[NUM] CIS dev: -d 1: single 2: dual" ,
    "[NUM] encode_mode, 0:user mode, 1:kernel mode(frame), 2:kernel mode(slice),default 0" ,
    "uvc bulk iso mode" ,
//    "[NUM] stitch mode, 0 not stitch mode, 1 vertical stitch mode, 2 horizontal stitch mode, default 0" ,
//    "[NUM] output_mode, [0: sensor0, 1: sensor1, 2: sensor2, 4: all sensor, default 4" ,
    "",
};

/* opt for print the message */
static struct option option_long[ ] = {
    { "help"              , no_argument       , NULL , 'h' } ,      //"打印帮助信息" ,
    { "data-output"       , required_argument , NULL , 'o' } ,      //"编码输出数据格式" ,
    { "br_mode"           , required_argument , NULL , 'b' } ,      //"编码br模式" ,
    { "target_kbps"       , required_argument , NULL , 't' } ,      //"目标码率" ,
    { "frame-rate"        , required_argument , NULL , 'r' } ,      //"帧率" ,
    { "main_res  "        , required_argument , NULL , 'm' } ,      //"主通道分辨率" ,
    { "sub_res  "         , required_argument , NULL , 's' } ,      //"次通道分辨率" ,
    { "isp conf path"     , required_argument , NULL , 'f' } ,      //"isp 配置文件或yuv数据源文件目录"
    { "CIS_no"            , required_argument , NULL , 'd' } ,      // CIS对应的编号选择
    { "encode_mode"       , required_argument , NULL , 'e' } ,      //"编码模式",
    { "bulk_iso"          , required_argument , NULL , 'i' } ,      //"编码模式",
//    { "stitch_mode"       , required_argument , NULL , 'j' } ,      //"拼接模式",
//    { "output_mode"       , required_argument , NULL , 'd' } ,      //"选择输出哪路码流"
    { "uvc"          , no_argument , NULL , 'v' } ,
    { "uac"          , no_argument , NULL , 'a' } ,
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
    printf("eg.: %s -v -a -m 2 -f /etc/conf/isp0.conf\n", name);
    printf("encode data type -o:          h264,  h265,  or  jpeg\n");
    printf("cis_num -d: [NUM]\n");
    printf("resolution -m -s:     value 0 ~ 6\n");
    printf("                      0 - 320*240\n");
    printf("                      1 - 480*320\n");
    printf("                      2 - 640*480\n");
    printf("                      3 - 800*480\n");
    printf("                      4 - 1280*720\n");
    printf("                      5 - 1920*1080\n");
    printf("                      6 - 2560*1440\n");
    printf("isp config file -f:       \n");
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

void video_usage(const char *app)
{
    usage(app);
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
        case 'h' :                                    //help
            help_hint();
            c_flag = AK_FALSE;
            goto parse_option_end;
        case 'o' :                                    //data file format
            if(!strcmp(optarg, "h264"))
            {
                /* h264 */
                encoder_type = PROFILE_MAIN;
            }
            else if(!strcmp(optarg, "h265"))
            {
                /* h265 */
                encoder_type = PROFILE_HEVC_MAIN;
            }
            else if(!strcmp(optarg, "jpeg"))
            {
                /* jpeg */
                encoder_type = PROFILE_JPEG;
            }
            break;
        case 'm' :                                    //main res
            main_res = atoi( optarg );
            break;
        case 's' :                                    //sub res
            sub_res = atoi( optarg );
            break;
        case 'f' :                                     //cfg file path
            video.isp_cfg =  optarg;
            break;
//        case 'a' :                                     //chn index
//            video.ch_id =  atoi( optarg );
//            break;
        case 'b' :                                     //br mode
            video.br_mode =  atoi( optarg );
            break;
        case 't' :                                     //target kbps
            video.target_kbps =  atoi( optarg );
            break;
        case 'r' :                                     //frame rate
            video.fps =  atoi( optarg );
            break;
        case 'd' :                                     //CIS no
//            CIS_no = atoi(optarg);
//            CIS_num++;
            CIS_num  = atoi(optarg);
            break;
        case 'i' :                                     //bulk or iso
            bulk_or_iso_mode = atoi(optarg);
            break;

        case 'v' :                                     //uvc
        case 'a' :                                     //uac
            break;
        // case 'f' :                                     //save file flag
        //     save_file_flag = atoi(optarg);
        //     break;
        // case 'e' :                                     //encode mode
        //     encode_mode = atoi(optarg);                
        //     break;
        // case 'j' :                                     //stitch mode
        //     stitch_mode = atoi(optarg);
        //     break;
        // case 'd' :                                     //output mode
        //     output_mode = atoi(optarg);
        //     break;
        default :
            help_hint();
            c_flag = AK_FALSE;
            goto parse_option_end;
        }
    }
parse_option_end:
    return c_flag;
}

int video_entry(int argc, char **argv)
{
    int ret;

    if( parse_option( argc, argv ) == AK_FALSE )
    {                                               //解释和配置选项
        return AK_FAILED;
    }

    if (CIS_num == 2) {
        video.uvc_devname = "/dev/video16";
        stitch_mode = VERTICAL_STITCH;
    }

    //init_mipi_switch();
    ret = wc_video_init();
    if(ret) {
        return ret;
    }

    ret = start_venc(encoder_type, main_res);
    if(ret) {
        return ret;
    }
     /* open uvc device before venc thread */
    ret = uvc_open(&video.udev, video.uvc_devname);
    if(video.udev == NULL || ret < 0)
        return ret;

    video.udev->uvc_devname = video.uvc_devname;
    video.udev->bulk = bulk_or_iso_mode ;//ISO
    video.udev->run_standalone = 1;
    video.udev->io   = IO_METHOD_MMAP;
    video.udev->nbufs = 3; /* Ping-Pong buffers */
    video.udev->mult  = 0;
    video.udev->burst = 0;
    video.udev->speed = USB_SPEED_HIGH;
    video.udev->height =  video.max_height;
    video.udev->width = video.max_width;
   // video.udev->video_empty_stream = video_empty_stream;
   // video.udev->video_fill_stream = video_fill_stream;
    video.udev->user_dev = &video;
    video.udev->maxpkt = UVC_STREAMING_MAXPKT;
    PRINT( "video =%p! video.udev->user_dev=%p\n",&video,video.udev->user_dev);
    /* Init UVC events. */
    uvc_events_init(video.udev);
    ret = ak_thread_create(&video.uvc_ctrl_th, video_uvc_control_th, &video, \
                            ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);
    if(ret != AK_SUCCESS){
        ak_thread_detach(video.uvc_ctrl_th);
    }
    ret = ak_thread_create(&video.uvc_stream_th, video_uvc_stream_th, &video, \
                            ANYKA_THREAD_MIN_STACK_SIZE, THREAD_PRIO);
    if(ret != AK_SUCCESS){
        ak_thread_detach(video.uvc_stream_th);
        ak_thread_detach(video.uvc_ctrl_th);
    }



    return ret;
}
/* end of func */


int video_exit(void)
{
    ak_thread_join(video.uvc_stream_th);
    ak_thread_join(video.uvc_ctrl_th);
    video_deinit();
    ak_sdk_exit();
}

/* end of file */

