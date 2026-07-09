#ifndef __AK_OSD_APP_H__
#define __AK_OSD_APP_H__



#define AK_PP_MAGIC 'P'

#define AK_PP_SET_PALETTE           _IOW(AK_PP_MAGIC, 1, int)
#define AK_PP_GET_PALETTE           _IOR(AK_PP_MAGIC, 2, int)
#define AK_PP_SET_OSD_CTRL          _IOW(AK_PP_MAGIC, 3, int)
#define AK_PP_SET_OSD_BUF           _IOW(AK_PP_MAGIC, 4, int)
#define AK_PP_GET_OSD_BUF           _IOR(AK_PP_MAGIC, 5, int)
#define AK_PP_GET_OSD_CTRL          _IOR(AK_PP_MAGIC, 6, int)


typedef enum pp_chn {
    MAIN_CHN = 0,
    SUB_CHN,
    THIRD_CHN,
    CHN_NUM
}PP_CHANNEL;

enum PP_CHN_OSD_LAYER_MODE {
    PP_CHN_OSD_LAYER_MODE_CLOSE = 0,//关闭osd图层
    PP_CHN_OSD_LAYER_MODE_GREY,//黑白
    PP_CHN_OSD_LAYER_MODE_COLORINDEX,//索引色
};

struct pp_chn_osd_ctrl {
    unsigned int chn_osd_buf_size;//缓存buf的size
    unsigned short chn_osd_start_xpos;
    unsigned short chn_osd_start_ypos;
    unsigned short chn_osd_width;
    unsigned short chn_osd_height;
    enum PP_CHN_OSD_LAYER_MODE chn_osd_layer_mode;
    unsigned short chn_osd_layer_size;//2的x次方的颜色索引宽度,取值0~2
};

struct pp_chn_osd_buf {
    void *chn_osd_buf_paddr;
    void *chn_osd_buf_vaddr;
};

typedef struct pp_chn_osd_info {
    PP_CHANNEL chn_id;
    unsigned int chn_osd_index;//每个通道支持多个osd，这个参数表示哪个osd
    unsigned int chn_osd_enable;
    struct pp_chn_osd_ctrl chn_osd_ctrl;
    struct pp_chn_osd_buf chn_osd_buf;
}PP_OSD_ATTR;

#endif
