#ifndef _AK_OSD_EX_H_
#define _AK_OSD_EX_H_

#include "ak_osd.h"

struct bitmap_attr
{
    int bitmap_height;		// 位图高度，单位像素
    int bitmap_width;		// 位图宽度，单位像素
    int bitperpixel;		// 位图像素所占的bit大小(位深度)
};

/* 位图信息记录结构体 */
struct osd_bitmap_info
{
    unsigned char 			*buffer;		// 位图数据
    int						buffer_length;	// 位图数据大小
    struct bitmap_attr		osd_bmp_attr;	// 位图属性
};

struct font_color_ctrl
{
    unsigned char font_color_id;	            //font color id	
};
struct font_ctrl_attr
{
    int channel; 
    int osd_rect; 
    int xoffset;
    int yoffset;
    const unsigned short *disp_str;      
    int str_len; 
    struct font_color_ctrl *font_color;    
};

/**
 * ak_osd_ex_get_version - get osd ex version
 * return: version string
 */
const char* ak_osd_ex_get_version(void);

/**
 * ak_osd_ex_init2 - osd init with param.
 * @param[IN]: init param
 * return: 0 - success; otherwise error code;
 */
int ak_osd_ex_init2(struct osd_attr *param);

/**
 * ak_osd_ex_set_color_table - set osd color table.
 * @color_t[IN]:  color table to be set
 * @color_format[IN]: color foramt to set, 0 - YUV format, 1 - ARGB8888 format
 * NOTE: this functiong should be called before ak_osd_ex_init;
 * return: 0 - success; otherwise error code;
 */
int ak_osd_ex_set_color_table(unsigned int *color_t, int color_format);

/**
 * ak_osd_ex_init - init osd param( include screen resolution,
 *			default color table and color id );malloc osd buf.
 * return: 0 - success; otherwise error code;
 */
int ak_osd_ex_init(void);

/**
 * ak_osd_ex_set_font_file - set char font data file.
 * @font_size[IN]:  size of font  in font data file
 * @file[IN]: font data file  ,file can be read
 * return: 0 - success; otherwise error code;
 */
int ak_osd_ex_set_font_file(int font_size, const char *file);

/**
 * ak_osd_ex_get_max_rect - get max rect of display.
 * @channel[IN]:  [0,3],  0 - main channel, 1 - sub channel;
 * 2 -dev 1 main channel, 3 - dev 1 sub channel
 * @width[OUT]:   screen width
 * @height[OUT]:  screen height
 * return: 0 - success; otherwise error code;
 */
int ak_osd_ex_get_max_rect(int channel, int *width, int *height);

/**
 * ak_osd_ex_set_rect - set osd rect.
 * @channel[IN]:  [0,3],  0 - main channel, 1 - sub channel;
 * 2 -dev 1 main channel, 3 - dev 1 sub channel
 * @osd_rect[IN]: [0,2], osd display area number.
 * @xstart[IN]:  start pixel x-ray
 * @ystart[IN]:  start pixel y-ray
 * @width[IN]:   osd width
 * @height[IN]:  osd height
 * return: 0 - success; otherwise error code;
 * note: the rect to be set should be in range of max_rect
 * NOTE: you need 2 more pixel for width and height for each char to draw the edge.
 * like if you char is 16*16, the width*height should
 * be (num*18)*18.
 */
int ak_osd_ex_set_rect(int channel, int osd_rect, int xstart, int ystart,
        int width, int height);

/**
 * ak_osd_ex_draw_matrix - draw appointed matrix on screen.
 * @channel[IN]:  [0,3],  0 - main channel, 1 - sub channel;
 * 2 -dev 1 main channel, 3 - dev 1 sub channel
 * @osd_rect[IN]: [0,2], osd display area number.
 * @xoffset[IN]:  x offset of rect
 * @yoffset[IN]:  y offset of rect
 * @font_w[IN]:  width of font
 * @font_h[IN]:  height of font
 * @dot_buf[IN]: display dot buffer
 * @buf_len[IN]: display buffer len
 * return: 0 - success; otherwise error code;
 * notes: 4bit in dot_buf stands for 1 pixel,
 *           the value of the 4bit stands for ID of color_tables.
 *           ( font_w *  font_h ) / 2 == buf_len
 *unsigned int def_color_tables[] =
 *{
 *	0x000000, 0xff7f7f, 0x007f7f, 0x266ac0, 0x71408a, 0x4b554a, 0x599540, 0x0ec075,
 *	0x34aab5, 0x786085, 0x2c8aa0, 0x68d535, 0x34aa5a, 0x43e9ab, 0x4b55a5, 0x008080
 *};
 */
int ak_osd_ex_draw_matrix(int channel, int osd_rect, int xoffset, \
    int yoffset, int font_w, int font_h, const unsigned char *dot_buf, \
    unsigned int buf_len);

/**
 * ak_osd_ex_get_str_bmp - convert str to bitmap.
 * @channel[IN]:  [0,3],  0 - main channel, 1 - sub channel;
 * 2 -dev 1 main channel, 3 - dev 1 sub channel
 * @disp_str[IN]: display string code, gb2312
 * @str_len[IN]: display string len
 * @out_bitmap_info[OUT]: bitmap info
 * return: 0 - success; otherwise error code;
 */
int ak_osd_ex_get_str_bmp(int channel,const unsigned short *disp_str, \
    int str_len, struct osd_bitmap_info *out_bitmap_info);

/**
* ak_osd_ex_release_bmp_buf : release the bitmap buffer;
* @bmp_info[IN]: bitmap info
* return:0 for success , other value for failed;
* NOTE:    once you call the functions in this module,
*          you must call the following function to release the source.
*/
int ak_osd_ex_release_bmp_buf(struct osd_bitmap_info *bmp_info);

/**
* ak_osd_ex_release_bmp_buf : release the bitmap buffer;
* @bmp_info[IN]: bitmap info array
* @len[IN]: bitmap info array len
* return:0 for success , other value for failed;
* NOTE:    once you call the functions in this module,
*          you must call the following function to release the source.
*/
int ak_osd_ex_release_bmp_buf_ex(struct osd_bitmap_info *bmp_info, int len);

/**
 * ak_osd_ex_draw_str - draw string on screen.
 * @channel[IN]:  [0,3],  0 - main channel, 1 - sub channel; \
 * 2 -dev 1 main channel, 3 - dev 1 sub channel
 * @osd_rect[IN]: [0,2], osd display area number.
 * @xoffset[IN]:   x offset of rect
 * @yoffset[IN]:   y offset of rect
 * @disp_str[IN]: display string code, gb2312
 * @str_len[IN]:  display string len
 * return: 0 - success; otherwise error code;
 * note: display str on the rect that ak_osd_set_rect set
 * NOTE: you need 2 more pixel for width and height for each char to draw the edge.
 * like if you char is 16*16, the width*height should
 * be (num*18)*18.
 */
int ak_osd_ex_draw_str(int channel, int osd_rect, int xoffset, int yoffset,
        const unsigned short *disp_str, int str_len);

/** 
 * ak_osd_ex_draw_str_ex - draw string on screen and set font color
 * @font_ctrl_attr[IN]:  font_ctrl_attr
 * return: 0 - success; otherwise error code; 
 * note: display str on the rect that ak_osd_set_rect set
 * NOTE: you need 2 more pixel for width and height for each char to draw the edge.
 * like if you char is 16*16, the width*height should 
 * be (num*18)*18.
 */
int ak_osd_ex_draw_str_ex(struct font_ctrl_attr *font_ctrl_attr);

/**
 * ak_osd_ex_clean_str - clean osd info on screen.
 * @channel[IN]: [0,3],  0 - main channel, 1 - sub channel; \
 * 2 -dev 1 main channel, 3 - dev 1 sub channel
 * @osd_rect[IN]: [0,2], osd display area number.
 * @xoffset[IN]: x offset of rect
 * @yoffset[IN]: y offset of rect
 * @width[IN]: width of rect to clean
 * @height[IN]: height  of rect to clean
 * return: 0 - success; otherwise error code;
 */
int ak_osd_ex_clean_str(int channel, int osd_rect, int xoffset, int yoffset,\
    int width, int height);

/**
 * ak_osd_ex_set_color - set osd color param.
 * @front_color[IN]: front color of osd,  [0,15]
 * @bg_color[IN]: back color of osd,  [0,15]
 * return: 0 - success; otherwise error code;
 */
int ak_osd_ex_set_color(int front_color, int bg_color);

/**
 * ak_osd_ex_set_edge_color - set osd font edge color param.
 * @edge_color[IN]:   front edge color index of osd,  [0,15]
 * return: 0 - success; otherwise error code;
 */
int ak_osd_ex_set_edge_color(int edge_color);

/**
 * ak_osd_ex_set_alpha - set osd alpha
 * @alpha[IN]: alpha for osd overlay, [1,100]
 * return: 0 on success, error code failed
 */
int ak_osd_ex_set_alpha(int alpha);

/**
 * ak_osd_ex_set_font_size - set osd display font size.
 * @channel[IN]:  [0,3],  0 - main channel, 1 - sub channel;
 * 2 -dev 1 main channel, 3 - dev 1 sub channel
 * @size[IN]:    display font size
 * return: 0 - success; otherwise error code;
 * notice:
 *        if  ak_osd_set_font_file called, donot call this. this function work along
 *         with ak_osd_draw_matrix.
 */
int ak_osd_ex_set_font_size(int channel, int size);


/**
 * ak_osd_ex_set_font_interval - set osd display font interval.
 * @channel[IN]:  [0,3],  0 - main channel, 1 - sub channel;
 * 2 -dev 1 main channel, 3 - dev 1 sub channel
 * @osd_rect[IN]:    osd_rect
 * @interval[IN]:    display font interval ,can be divided by 2(bit_width 4),
 * can be divided by 4(bit_width 2), can be divided by 8(bit_width 1)
 * return: 0 - success; otherwise -1;
 */
int ak_osd_ex_set_font_interval(int channel, int osd_rect, int interval);

/**
 * ak_osd_ex_destroy - release resource of osd.
 * return:  0 - success; otherwise error code;
 */
int ak_osd_ex_destroy(void);

#endif

/* end of file */
