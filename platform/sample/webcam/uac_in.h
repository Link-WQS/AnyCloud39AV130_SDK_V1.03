#ifndef _UAC_IN_H_
#define _UAC_IN_H_

#define UAC_CAPTURE_CONTROL_FILE_NAME "/dev/uacc0"
#define UAC_CAPTURE_DATA_FILE_NAME "/dev/uacc1"

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define ARRAY_SIZE(a) ((sizeof(a) / sizeof(a[0])))

#define UAC_FU_MUTE   0x01
#define UAC_FU_VOLUME 0x02
#define AK_USB_REQ_DISCONNECT (0xF0)

#define UAC__CUR      0x1
#define UAC__MIN      0x2
#define UAC__MAX      0x3
#define UAC__RES      0x4

#define VOLUME_MIN 1
#define VOLUME_MAX 255

/* ------------------------------------------------------------------------
 * Callback
 */
typedef int (*open_ai_cb)(int sample_rate,int channel_num);
typedef int (*close_ai_cb)(void);
typedef int (*get_frame_ai_cb)(int data_fd);
typedef int (*set_volume_ai_cb)(int value);
typedef int (*set_mute_ai_cb)(int value);

struct uac_capture_runtime {
    int control_fd, data_fd;
    int ai_handle; 
    int periods, sample_rate;
    int frame_interval;
    unsigned int started:1;
};
struct uac_capture_ctrl_data{
    int cmd;
    int value;
};


//extern int uac_capture_process(void);
extern struct uac_capture_runtime* uac_capture_init(void);
extern void capture_data_process(struct uac_capture_runtime *rt,unsigned char *data,unsigned int len);
extern int capture_control_process(struct uac_capture_runtime *rt,struct uac_capture_ctrl_data *ctrl_data);

#endif