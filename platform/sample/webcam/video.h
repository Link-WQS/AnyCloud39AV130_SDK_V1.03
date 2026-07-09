#ifndef _VIDEO_H_
#define _VIDEO_H_

/* resolation define */
struct resolution_t {
    unsigned int width;
    unsigned int height;
    unsigned char str[20];
};

typedef enum {
    DE_VIDEO_240P = 0,
    DE_VIDEO_320P,
    DE_VIDEO_VGA,
    DE_VIDEO_480P,
    DE_VIDEO_720P,
    DE_VIDEO_1080P,
    DE_VIDEO_1440P,
}ch_resolutions;

struct dev_ch_t {
     int main_ch;
     int sub_ch;
 };

struct dual_param{
    int  chn_index;
    int  frame_num;
    int  main_res;
    int  sub_res;
    int  target_kbps;
    char vi_fps;
    char br_mode;
    char save_path[128];
    char cfg[128];
    char type[128];
};

#endif
