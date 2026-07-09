#ifndef __AK_PP_H__
#define __AK_PP_H__

// OSD索引宽度
typedef enum {
    PP_OSD_1BIT = 0,
    PP_OSD_2BIT = 1,
    PP_OSD_4BIT = 2,
} AK_ISP_PP_OSD_SIZE;

typedef struct {
    unsigned int   strX;
    unsigned int   strY;
    unsigned int   endX;
    unsigned int   endY;
    unsigned int   width;
    unsigned int   height;
} AK_ISP_PP_WINDOW;

// OSD图层模式
typedef enum {
    PP_OSD_OFF = 0,         //关闭
    PP_OSD_MONO = 1,        //黑白
    PP_OSD_COLOR = 2,       //索引
} AK_ISP_PP_OSD_MODE;

typedef enum osd_channel {
    OSD_CHN0 = 0,
    OSD_CHN1,
    OSD_CHN2,
    OSD_CHN3,
    OSD_CHN_NUM
} OSD_CHANNEL;

typedef struct ak_isp_osd_mask_color_table_attr {
    unsigned int color_table[24];//高8位为alpha  
} AK_ISP_OSD_MASK_COLOR_TABLE_ATTR;

typedef struct ak_isp_osd_mem_attr {
    OSD_CHANNEL chn;
    unsigned char *dma_paddr;
    unsigned char *dma_vaddr;
    unsigned int size;
}AK_ISP_OSD_MEM_ATTR;

typedef struct ak_isp_osd_context_attr {

#if 0
    OSD_CHANNEL chn;
    unsigned int *osd_context_addr;
    unsigned int osd_width;
    unsigned int osd_height;
    unsigned short start_xpos;
    unsigned short start_ypos;
    unsigned short alpha;
    unsigned char enable;
#endif
    OSD_CHANNEL chn;
    unsigned int *osd_context_addr;
    AK_ISP_PP_OSD_MODE  mode;
    AK_ISP_PP_WINDOW    win;
    AK_ISP_PP_OSD_SIZE  size;
    unsigned int        addr;
    unsigned int        stride;
}AK_ISP_OSD_CONTEXT_ATTR;

// Draw模块图层模式
typedef enum {
	PP_DRAW_OFF = 0,		//关闭
	PP_DRAW_MOSAIC = 1, 	//马赛克
	PP_DRAW_RECTANGLE = 2,	//矩形
	PP_DRAW_BOX = 3 		//矩形框
} AK_ISP_PP_DRAW_MODE;

typedef struct ak_isp_mask_area {
	unsigned short	aux1_cfg;
	unsigned short	aux0_cfg;
	AK_ISP_PP_DRAW_MODE mode;
	AK_ISP_PP_WINDOW win;
}AK_ISP_MASK_AREA;

typedef struct ak_isp_mask_masic_attr{
	unsigned short	mosai_size_hor;
	unsigned short	mosai_size_vec;
}AK_ISP_MASK_MASIC_ATTR;

typedef struct ak_isp_main_chan_mask_area_attr {
	AK_ISP_MASK_AREA  mask_area[14];
	AK_ISP_MASK_MASIC_ATTR mask_masic_attr;
}AK_ISP_MAIN_CHAN_MASK_AREA_ATTR;

typedef struct ak_isp_sub_chan_mask_area_attr {
	AK_ISP_MASK_AREA  mask_area[14];
	AK_ISP_MASK_MASIC_ATTR mask_masic_attr;
}AK_ISP_SUB_CHAN_MASK_AREA_ATTR;

struct pp_profile_src_attr {
    int flip;
    int mirror;
};

void ak_pp_profile_set_src_attr(void *isp_struct, struct pp_profile_src_attr *pf_src);


/*********************************************osd**********************************************************/
/*********************************************osd**********************************************************/
int ak_isp_vpp_set_osd_mask_color_table_attr(void *isp_struct,AK_ISP_OSD_MASK_COLOR_TABLE_ATTR *p_isp_color_table);
int ak_isp_vpp_set_main_channel_osd_mem_attr(void *isp_struct,AK_ISP_OSD_MEM_ATTR *p_mem);
int ak_isp_vpp_set_sub_channel_osd_mem_attr(void *isp_struct,AK_ISP_OSD_MEM_ATTR *p_mem);
int ak_isp_vpp_set_main_channel_osd_context_attr(void *isp_struct,AK_ISP_OSD_CONTEXT_ATTR *p_context);
int ak_isp_vpp_set_sub_channel_osd_context_attr(void *isp_struct,AK_ISP_OSD_CONTEXT_ATTR *p_context);
int ak_isp_osd_irq_update(void *isp_struct);






/*********************************************draw**********************************************************/
/*********************************************draw**********************************************************/
int ak_isp_vpp_set_main_chan_mask_area_attr( void *isp_struct,AK_ISP_MAIN_CHAN_MASK_AREA_ATTR *p_mask);
int ak_isp_vpp_set_sub_chan_mask_area_attr( void *isp_struct,AK_ISP_SUB_CHAN_MASK_AREA_ATTR *p_mask);


/*********************************************scale**********************************************************/
/*********************************************scale**********************************************************/
int ak_isp_vo_set_main_channel_scale(void *isp_struct,int width, int height);
int ak_isp_vo_set_sub_channel_scale(void *isp_struct,int width, int height);
int ak_isp_vo_set_ch3_channel_scale(void *isp_struct,int width, int height);

#endif
