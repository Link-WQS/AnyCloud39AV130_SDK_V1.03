/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_vpss_qr_sample.c
* Description: This is a simple example to show how the vpss qr module working.
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
static char *save_path = "/mnt/";
static char *isp_path = "";
static int dev_cnt = 1;

static struct vpss_rgb_gamma_attr rgb_gamma_bak = {0};
static struct vpss_rgb_gamma_attr rgb_gamma = {0};
static struct vpss_contrast_attr contrast_bak = {0};
static struct vpss_contrast_attr contrast = {0};
static struct vpss_wdr_attr wdr_bak = {0};
static struct vpss_wdr_attr wdr = {0};
static struct vpss_yuv_effect_attr effect_bak = {0};
static struct vpss_yuv_effect_attr effect = {0};
#ifndef __CHIP_AK39EV200_SERIES
static struct vpss_lce_attr lce_bak = {0};
static struct vpss_lce_attr lce = {0};
#endif
static struct vpss_isp_ae_attr ae_bak = {0};
static struct vpss_isp_ae_attr ae = {0};


static char ac_option_hint[  ][ LEN_HINT ] = {
    /*鎿嶄綔鎻愮ず鏁扮粍**/
    /*"help info"*/
    "help info" ,
    /*"isp cfg file path"*/
    "[PATH] isp cfg file path, (dev_cnt:1  isp_xxx.conf, dev_cnt:2  isp_xxx_dual.conf)" ,
    /*"save path"*/
    "[PATH] path to save yuv file, default /mnt/" ,
    /*"device count"*/
    "[NUM] device count [1 - 2], default 1" ,
    "" ,
};

static struct option option_long[ ] = {
    /*"help info"*/
    { "help"      , no_argument       , NULL , 'h' } ,
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
    ak_print_normal(MODULE_ID_APP, "Usage: %s -f isp_cfg_path -p savepath\n", name);
    ak_print_notice(MODULE_ID_APP, "e.g: %s -f /etc/config/isp*.conf -p /mnt/\n", name);
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

    /*
     * disable chn
     */
    ak_vi_disable_chn(chn_main_id);
    ak_vi_disable_chn(chn_sub_id);

    /*
     * disable dev
     */
    ak_vi_disable_dev(dev_id);

    /*
     * vi close
     */
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

    /*
         * dev 0
         */
    ret = ak_vi_get_frame(VIDEO_CHN0, &frame);
    if (!ret) {
        ak_get_localdate(&date);
        ak_date_to_string(&date, time_str);
        sprintf(file_path, "%sdev0_qr_%s.yuv", save_path, time_str);

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
        /*
             * dev 1
             */
        ret = ak_vi_get_frame(VIDEO_CHN2, &frame);
        if (!ret) {
            ak_get_localdate(&date);
            ak_date_to_string(&date, time_str);
            sprintf(file_path, "%sdev1_qr_%s.yuv", save_path, time_str);

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
#if 0
static int cleanup_isp_attr(void)       // not used
{

    memset(&rgb_gamma, 0x00, sizeof(struct vpss_rgb_gamma_attr));
    memset(&rgb_gamma_bak, 0x00, sizeof(struct vpss_rgb_gamma_attr));

    memset(&contrast, 0x00, sizeof(struct vpss_contrast_attr));
    memset(&contrast_bak, 0x00, sizeof(struct vpss_contrast_attr));

    memset(&wdr, 0x00, sizeof(struct vpss_wdr_attr));
    memset(&wdr_bak, 0x00, sizeof(struct vpss_wdr_attr));

    memset(&effect, 0x00, sizeof(struct vpss_yuv_effect_attr));
    memset(&effect_bak, 0x00, sizeof(struct vpss_yuv_effect_attr));

    memset(&ae, 0x00, sizeof(struct vpss_isp_ae_attr));
    memset(&ae_bak, 0x00, sizeof(struct vpss_isp_ae_attr));
#ifndef __CHIP_AK39EV200_SERIES
    memset(&lce, 0x00, sizeof(struct vpss_lce_attr));
    memset(&lce_bak, 0x00, sizeof(struct vpss_lce_attr));
#endif

    return 0;
}
#endif
static int __adjust_rgb_gamma(int dev)
{
    int ret = ak_vpss_get_rgb_gamma_attr(dev, &rgb_gamma);

    if (AK_SUCCESS != ret)
    {
        return ret;
    }

    memcpy(&rgb_gamma_bak, &rgb_gamma, sizeof(struct vpss_rgb_gamma_attr));
    for(int i=0; i<(sizeof(rgb_gamma.linkage_rgb_gamma[0].r_gamma)/sizeof(unsigned short)); i++)
        rgb_gamma.linkage_rgb_gamma[0].r_gamma[i] = i*8;

    rgb_gamma.linkage_rgb_gamma[0].r_gamma[(sizeof(rgb_gamma.linkage_rgb_gamma[0].r_gamma)/ \
        sizeof(unsigned short))-1] = 1023;
    memcpy(rgb_gamma.linkage_rgb_gamma[0].g_gamma, rgb_gamma.linkage_rgb_gamma[0].r_gamma, \
        sizeof(rgb_gamma.linkage_rgb_gamma[0].r_gamma));
    memcpy(rgb_gamma.linkage_rgb_gamma[0].b_gamma, rgb_gamma.linkage_rgb_gamma[0].r_gamma, \
        sizeof(rgb_gamma.linkage_rgb_gamma[0].r_gamma));
    rgb_gamma.linkage_rgb_gamma[0].rgb_gamma_enable = 0;

    memcpy(rgb_gamma.linkage_rgb_gamma[1].r_gamma, rgb_gamma.linkage_rgb_gamma[0].r_gamma, \
        sizeof(rgb_gamma.linkage_rgb_gamma[0].r_gamma));
    memcpy(rgb_gamma.linkage_rgb_gamma[1].g_gamma, rgb_gamma.linkage_rgb_gamma[0].r_gamma, \
        sizeof(rgb_gamma.linkage_rgb_gamma[0].r_gamma));
    memcpy(rgb_gamma.linkage_rgb_gamma[1].b_gamma, rgb_gamma.linkage_rgb_gamma[0].r_gamma, \
        sizeof(rgb_gamma.linkage_rgb_gamma[0].r_gamma));
    rgb_gamma.linkage_rgb_gamma[1].rgb_gamma_enable = 0;

    ret = ak_vpss_set_rgb_gamma_attr(dev, &rgb_gamma);

    return ret;
}

static int __recover_rgb_gamma(int dev)
{
    return ak_vpss_set_rgb_gamma_attr(dev, &rgb_gamma_bak);
}

static int __adjust_contrast(int dev)
{
    int ret = ak_vpss_get_contrast_attr(dev, &contrast);

    if (AK_SUCCESS != ret)
    {
        return ret;
    }

    memcpy(&contrast_bak, &contrast, sizeof(struct vpss_contrast_attr));

    /*
     * set to manual mode
     */
    //contrast.contrast_mode = 0;
    contrast.cc_mode = 0;
    contrast.manual_contrast.y_contrast = 64;
    contrast.manual_contrast.y_shift = 0;
    ret = ak_vpss_set_contrast_attr(dev, &contrast);

    return ret;
}

static int __recover_contrast(int dev)
{
    return ak_vpss_set_contrast_attr(dev, &contrast_bak);
}

static int __adjust_wdr(int dev)
{
    int ret = ak_vpss_get_wdr_attr(dev, &wdr);

    if (AK_SUCCESS != ret)
    {
        return ret;
    }

    memcpy(&wdr_bak, &wdr, sizeof(struct vpss_wdr_attr));
    for(int i=0; i<(sizeof(wdr.manual_wdr.area_tb1)/sizeof(unsigned short)); i++)
        wdr.manual_wdr.area_tb1[i] = i*16;
    wdr.manual_wdr.area_tb1[(sizeof(wdr.manual_wdr.area_tb1)/sizeof(unsigned short))-1] = 1023;
    memcpy(wdr.manual_wdr.area_tb2, wdr.manual_wdr.area_tb1, sizeof(wdr.manual_wdr.area_tb1));
    memcpy(wdr.manual_wdr.area_tb3, wdr.manual_wdr.area_tb1, sizeof(wdr.manual_wdr.area_tb1));
    memcpy(wdr.manual_wdr.area_tb4, wdr.manual_wdr.area_tb1, sizeof(wdr.manual_wdr.area_tb1));
    memcpy(wdr.manual_wdr.area_tb5, wdr.manual_wdr.area_tb1, sizeof(wdr.manual_wdr.area_tb1));
    memcpy(wdr.manual_wdr.area_tb6, wdr.manual_wdr.area_tb1, sizeof(wdr.manual_wdr.area_tb1));

    /*
     * set to manual mode
     */
    wdr.wdr_mode = 0;
    ret = ak_vpss_set_wdr_attr(dev, &wdr);

    return ret;
}

static int __recover_wdr(int dev)
{
    return ak_vpss_set_wdr_attr(dev, &wdr_bak);
}

static int __adjust_effect(int dev)
{
    int ret = ak_vpss_get_yuv_effect_attr(dev, &effect);

    if (AK_SUCCESS != ret)
    {
        return ret;
    }

    memcpy(&effect_bak, &effect, sizeof(struct vpss_yuv_effect_attr));
    effect.y_a = 64;
    effect.y_b = 0;
    effect.uv_a = 0;
    effect.uv_b = 0;
    ret = ak_vpss_set_yuv_effect_attr(dev, &effect);

    return ret;
}

static int __recover_effect(int dev)
{
    return ak_vpss_set_yuv_effect_attr(dev, &effect_bak);
}

static int __adjust_ae(int dev)
{
    int ret = ak_vpss_get_ae_attr(dev, &ae);

    if (AK_SUCCESS != ret)
    {
        return ret;
    }

    memcpy(&ae_bak, &ae, sizeof(struct vpss_isp_ae_attr));

#ifndef __CHIP_AK39EV200_SERIES
    for(int i=0; i<(sizeof(ae.ae_strgy_para[0].hist_weight)/sizeof(unsigned int)); i++)
        ae.ae_strgy_para[0].hist_weight[i] = 0x02;
    ae.ae_strgy_para[0].hist_weight[11] = 3;
    ae.ae_strgy_para[0].hist_weight[12] = 4;
    ae.ae_strgy_para[0].hist_weight[13] = 6;
    ae.ae_strgy_para[0].hist_weight[14] = 10;
    ae.ae_strgy_para[0].hist_weight[15] = 13;

    memcpy(ae.ae_strgy_para[1].hist_weight, ae.ae_strgy_para[0].hist_weight, \
        sizeof(ae.ae_strgy_para[0].hist_weight));
    memcpy(ae.ae_strgy_para[2].hist_weight, ae.ae_strgy_para[0].hist_weight, \
        sizeof(ae.ae_strgy_para[0].hist_weight));
#endif

    ret = ak_vpss_set_ae_attr(dev, &ae);

    return ret;
}

static int __recover_ae(int dev)
{
    return ak_vpss_set_ae_attr(dev, &ae_bak);
}

#ifndef __CHIP_AK39EV200_SERIES
static int __adjust_lce(int dev)
{
    int ret = ak_vpss_get_lce_attr(dev, &lce);

    if (AK_SUCCESS != ret)
    {
        return ret;
    }

    memcpy(&lce_bak, &lce, sizeof(struct vpss_lce_attr));

    /*
     * set to manual mode and disable the module
     */
    lce.lce_mode = VPSS_OP_TYPE_MANUAL;
    lce.manual_lce.lce_enable = 0;

    ret = ak_vpss_set_lce_attr(dev, &lce);

    return ret;
}

static int __recover_lce(int dev)
{
    return ak_vpss_set_lce_attr(dev, &lce_bak);
}
#endif

static int set_qr_state(int dev)
{
    __adjust_rgb_gamma(dev);
    __adjust_contrast(dev);
    __adjust_wdr(dev);
    __adjust_effect(dev);
    __adjust_ae(dev);
#ifndef __CHIP_AK39EV200_SERIES
    __adjust_lce(dev);
#endif

    return 0;
}

static int set_normal_state(int dev)
{
    __recover_rgb_gamma(dev);
    __recover_contrast(dev);
    __recover_wdr(dev);
    __recover_effect(dev);
    __recover_ae(dev);
#ifndef __CHIP_AK39EV200_SERIES
    __recover_lce(dev);
#endif

    return 0;
}


/**
 * brief: qr main function
 * @void
 * return: void;
 * notes:
 */
static void qr_test(void)
{
    struct video_input_frame frame = {{0}};
    int i = 0;
    int drop_num = 60;
    int ret = -1;

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

    /*
     * set isp params
     */
    set_qr_state(VIDEO_DEV0);

    if (dev_cnt > 1)
    {
        set_qr_state(VIDEO_DEV1);
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

    ak_print_normal_ex(MODULE_ID_APP, "qr set over\n");

    /*
     * recover isp params
     */
    set_normal_state(VIDEO_DEV0);

    if (dev_cnt > 1)
    {
        set_normal_state(VIDEO_DEV1);
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

    ak_print_normal_ex(MODULE_ID_APP, "recover over\n");
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
static int vpss_qr_sample(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    /*
         * sdk init first
         */
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    ak_print_normal(MODULE_ID_APP, "***********************************\n");
    ak_print_normal(MODULE_ID_APP, "** vpss version: %s **\n", \
        ak_vpss_get_version());
    ak_print_normal(MODULE_ID_APP, "***********************************\n");

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


    ak_print_normal(MODULE_ID_APP, "qr test start.\n");


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
        * step 2: qr main function
        */
    qr_test();

exit:
    /*
     * step 3: release resource
     */
    stop_vi(VIDEO_DEV0);
    if (dev_cnt > 1)
    {
        stop_vi(VIDEO_DEV1);
    }

    /*
         * sdk exit
         */
    ak_sdk_exit();
    ak_print_normal(MODULE_ID_APP, "exit vpss qr sample\n");

    return 0;
}

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(vpss_qr_sample, ak_vpss_qr_sample, vpss qr sample)
#endif
/* end of file*/
