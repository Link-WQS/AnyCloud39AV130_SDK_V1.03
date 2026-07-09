#ifndef __AK_PP_APP_H__
#define __AK_PP_APP_H__

#define AK_PP_MAGIC 'P'

#define AK_PP_SET_PALETTE           _IOW(AK_PP_MAGIC, 1, int)
#define AK_PP_GET_PALETTE           _IOR(AK_PP_MAGIC, 2, int)
#define AK_PP_SET_OSD_CTRL          _IOW(AK_PP_MAGIC, 3, int)
#define AK_PP_SET_OSD_BUF           _IOW(AK_PP_MAGIC, 4, int)
#define AK_PP_GET_OSD_BUF           _IOR(AK_PP_MAGIC, 5, int)
#define AK_PP_GET_OSD_CTRL          _IOR(AK_PP_MAGIC, 6, int)
#define AK_PP_SET_DRAW              _IOW(AK_PP_MAGIC, 7, int)
#define AK_PP_GET_DRAW              _IOR(AK_PP_MAGIC, 8, int)
#define AK_PP_SET_FLIP              _IOW(AK_PP_MAGIC, 9, int)
#define AK_PP_SET_STITCH            _IOW(AK_PP_MAGIC, 10, int)
#define AK_PP_GET_DMABUF_FD         _IOR(AK_PP_MAGIC, 11, int)
#define AK_PP_QUICK_START_FLAG      _IOW(AK_PP_MAGIC, 12, int)
#define AK_PP_GET_BUFFER_ADDR       _IOW(AK_PP_MAGIC, 13, int)
#define AK_PP_GET_QUICK_START_STAT  _IOW(AK_PP_MAGIC, 14, int)
#define AK_PP_SET_CAPUTRE_MODE      _IOW(AK_PP_MAGIC, 15, int)



/*
 *
 * PP_APP_SET_LINKS : 用于绑定多个节点组成一个pipeline, 
 *                  例如vicap-》isp-》pp是一个pipeline
 * PP_APP_GET_ID : 好的这个vicap的ID号
 */
enum ak_pp_app_cmd {
    PP_APP_SET_LINKS     = 0x100,
    PP_APP_SET_MAX_DMA_SIZE,
    PP_APP_SET_CHN_DONE_MODE,
    PP_APP_SET_CHN_SLICE_NUM,
    PP_APP_SET_CHN_BLOCK_NUM,
    PP_APP_SET_CHN_CTRL_FPS,

    PP_APP_GET_PAD_ID    = 0x200,
};

typedef enum pp_chn {
    MAIN_CHN = 0,
    SUB_CHN,
    THIRD_CHN,
    CHN_NUM
}PP_CHANNEL;

typedef enum pp_osd_channel {
    PP_OSD_CHN0 = 0,
    PP_OSD_CHN1,
    PP_OSD_CHN2,
    PP_OSD_CHN3,
    PP_OSD_CHN_NUM
} PP_OSD_CHANNEL;

enum PP_CHN_OSD_LAYER_MODE {
    PP_CHN_OSD_LAYER_MODE_CLOSE = 0,//关闭osd图层
    PP_CHN_OSD_LAYER_MODE_GREY,//黑白
    PP_CHN_OSD_LAYER_MODE_COLORINDEX,//索引色
};

enum PP_CHN_DRAW_LAYER_MODE {
    PP_CHN_DRAW_LAYER_MODE_CLOSE = 0,//not use
    PP_CHN_DRAW_LAYER_MODE_MOSAIC,//马赛克
    PP_CHN_DRAW_LAYER_MODE_RECTANGULARx1,//纯色矩形
    PP_CHN_DRAW_LAYER_MODE_RECTANGULARx5,//纯色矩形框
    PP_CHN_DRAW_LAYER_MODE_TRIANGLEx1,//直角三角/正菱形
    PP_CHN_DRAW_LAYER_MODE_TRIANGLEx5,//直角三角斜边/正菱形框
};

enum PP_CHN_MOSAIC_SIZE {
    PP_CHN_MOSAIC_SIZE_0 = 0,//no effect
    PP_CHN_MOSAIC_SIZE_1,//2^x pixels -> 2 pixels
    PP_CHN_MOSAIC_SIZE_2,
    PP_CHN_MOSAIC_SIZE_3,
    PP_CHN_MOSAIC_SIZE_4,
    PP_CHN_MOSAIC_SIZE_5,
    PP_CHN_MOSAIC_SIZE_6,
    PP_CHN_MOSAIC_SIZE_7,//2^x pixels -> 128 pixels
};

/* PP_APP_SET_CHN_DONE_MODE */
enum pp_chn_done_mode {
    PP_CHN_FRAME_DONE_MODE = 0,
    PP_CHN_SLICE_DONE_MODE,
    PP_CHN_DONE_MODE_NUM,
};

enum pp_chn_stitch_mode {
    PP_CHN_DISABLE_STITCH_MODE = 0,//no stitch
    PP_CHN_VERTICAL_STITCH_MODE,//vertical stitch
    PP_CHN_HORIZONTAL_STITCH_MODE,//horizontal stitch
};

/* PP_APP_SET_CHN_DONE_MODE */
enum pp_capture_mode {
    PP_CAPTURE_NOMAL_MODE = 0,
    PP_CAPTURE_AOV_MODE,
};
enum pp_quick_start_state {
    PP_QUICK_START_DISABLE = 0,
    PP_QUICK_START_STATE_KERNEL,
    PP_QUICK_START_STATE_APP,
};

typedef struct pp_flip_info {
    //为1表示使能对应flip
    AK_U32 hflip;//horizon_flip
    AK_U32 vflip;//vertical_flip
}PP_FLIP_ATTR;

struct priv_chn_max_dma_size {
    int type;
    int max_dma_size;
};

typedef struct pp_dmabuf_info {
    AK_U32 id;
    AK_S32 dmabuf_fd;
}PP_DMABUF_ATTR;

typedef struct pp_palette_info {
    AK_U32 osd_color_table[12];
    AK_U32 draw_color_table[3];
    AK_U32 mask_color_table;
}PP_PALETTE_ATTR;

struct pp_chn_osd_ctrl {
    AK_U32 chn_osd_buf_size;//缓存buf的size
    AK_U16 chn_osd_start_xpos;
    AK_U16 chn_osd_start_ypos;
    AK_U16 chn_osd_width;
    AK_U16 chn_osd_height;
    enum PP_CHN_OSD_LAYER_MODE chn_osd_layer_mode;
    AK_U16 chn_osd_layer_size;//2的x次方的颜色索引宽度,取值0~2
};

struct pp_chn_osd_buf {
    void *chn_osd_buf_paddr;
    void *chn_osd_buf_vaddr;
};

struct pp_chn_yuv_buf {
    void *chn_yuv_buf_paddr;
    void *chn_yuv_buf_vaddr;
};

typedef struct pp_chn_osd_info {
    PP_CHANNEL chn_id;
    AK_U32 chn_osd_index;//每个通道支持多个osd，这个参数表示哪个osd
    AK_U32 chn_osd_enable;
    struct pp_chn_osd_ctrl chn_osd_ctrl;
    struct pp_chn_osd_buf chn_osd_buf;
}PP_OSD_ATTR;

typedef struct pp_chn_draw_info {
    PP_CHANNEL chn_id;
    AK_U32 chn_draw_index;//每个通道支持多个draw，这个参数表示哪个draw
    AK_U32 chn_draw_enable;
    AK_S32 chn_draw_frame_num;

    AK_U16 chn_draw_x0;
    AK_U16 chn_draw_y0;
    AK_U16 chn_draw_x1;
    AK_U16 chn_draw_y1;
    enum PP_CHN_DRAW_LAYER_MODE chn_draw_layer_mode;
    /*
     *矩形框/直角三角斜边/正菱形框：表示边框线宽，填入实际值减1。如填入0代表1像素宽度，填入15（最大值）代表16像素宽度。
     */
    AK_U16 chn_draw_layer_aux0;
    /*
     *马赛克：马赛克的alpha值，0表示完全透明（无意义），31表示完全不透明。
     *纯色矩形/矩形框/直角三角：调色板索引编号，最大取值取决于调色板配置。
     */
    AK_U16 chn_draw_layer_aux1;

        /*global*/
    /*
     *马赛克网格的水平尺寸的幂次
     */
    enum PP_CHN_MOSAIC_SIZE chn_draw_mosaic_size_hor;
    enum PP_CHN_MOSAIC_SIZE chn_draw_mosaic_size_ver;//保留,暂不实现
}PP_DRAW_ATTR;

/* PP_APP_SET_CHN_DONE_MODE */
struct pp_chn_done_mode_info {
    enum pp_chn_done_mode mode;
};

struct pp_capture_mode_info {
    enum pp_capture_mode mode;
};


struct pp_chn_slice_num {
    AK_U32 slice_num;
};

struct pp_chn_block_num {
    AK_U32 block_num;
};

struct pp_chn_ctrl_fps {
    AK_S32 fps;
};

typedef struct pp_chn_stitch_attr {
    enum pp_chn_stitch_mode stitch_mode;
    AK_S32 stitch_num;
    AK_S32 stitch_first_id;
    AK_S32 stitch_index;
}PP_STITCH_ATTR;

struct priv_quick_start {
    AK_S32 quick_start_flag;
};

#endif
