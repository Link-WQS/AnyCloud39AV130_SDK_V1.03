#ifndef _UVC_DEVICE_H_
#define _UVC_DEVICE_H_

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>

/* ------------------------------------------------------------------------
 * Constants
 */
#define UVC_EVENT_FIRST			(V4L2_EVENT_PRIVATE_START + 0)
#define UVC_EVENT_CONNECT		(V4L2_EVENT_PRIVATE_START + 0)
#define UVC_EVENT_DISCONNECT		(V4L2_EVENT_PRIVATE_START + 1)
#define UVC_EVENT_STREAMON		(V4L2_EVENT_PRIVATE_START + 2)
#define UVC_EVENT_STREAMOFF		(V4L2_EVENT_PRIVATE_START + 3)
#define UVC_EVENT_SETUP			(V4L2_EVENT_PRIVATE_START + 4)
#define UVC_EVENT_DATA			(V4L2_EVENT_PRIVATE_START + 5)
#define UVC_EVENT_LAST			(V4L2_EVENT_PRIVATE_START + 5)

#define UVCIOC_SEND_RESPONSE		_IOW('U', 1, struct uvc_request_data)

#define UVC_INTF_CONTROL		0
#define UVC_INTF_STREAMING		1

/* ------------------------------------------------------------------------
 * Generic stuff
 */
/* IO methods supported */
enum io_method {
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
};

/* ------------------------------------------------------------------------
 * Structures
 */
struct uvc_request_data
{
    __s32 length;
    __u8 data[60];
};

struct uvc_event
{
    union {
        enum usb_device_speed speed;
        struct usb_ctrlrequest req;
        struct uvc_request_data data;
    };
};

/* ------------------------------------------------------------------------
 * Callback
 */
typedef int (*video_fill_stream_cb)(void *dev,void*buf,int index);
typedef int (*video_empty_stream_cb)(void *dev);

/* Represents a UVC based video output device */
struct uvc_device {
    /* uvc device specific */
    int uvc_fd;
    int is_streaming;
    int run_standalone;
    char* uvc_devname;

    /* uvc control request specific */
    struct uvc_streaming_control probe;
    struct uvc_streaming_control commit;
    int control;
    struct uvc_request_data request_error_code;
    unsigned int brightness_val;

    /* uvc buffer specific */
    enum io_method io;
    struct buffer* mem;
    struct buffer* dummy_buf;
    unsigned int nbufs;
    unsigned int fcc;
    unsigned int width;
    unsigned int height;

    unsigned int bulk;
    uint8_t color;
    //unsigned int imgsize;
    //void* imgdata;

    /* USB speed specific */
    int mult;
    int burst;
    int maxpkt;
    enum usb_device_speed speed;

    /* uvc specific flags */
    int first_buffer_queued;
    int uvc_shutdown_requested;

    /* uvc buffer queue and dequeue counters */
    unsigned long long int qbuf_count;
    unsigned int dqbuf_count;

    void *user_dev;
    /* to notify update stream data */
    //video_fill_stream_cb video_fill_stream;
    //video_empty_stream_cb video_empty_stream;
};

/* ------------------------------------------------------------------------
 * Functions
 */
// open uvc
extern int uvc_open(struct uvc_device** uvc, char* devname);
// init uvc events
extern void uvc_events_init(struct uvc_device* dev);
// process uvc events
extern __u32 uvc_events_process(struct uvc_device* dev);
// process uvc video
extern int uvc_video_process(struct uvc_device* dev,unsigned char *data,unsigned int len );

#endif /* _UVC_DEVICE_H_ */

