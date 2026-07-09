#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/usb/ch9.h>
#include <linux/usb/audio.h>
#include "ak_common.h"
#include "ak_log.h"
#include "ak_mem.h"
#include "ak_thread.h"
#include "uac_out.h"

#define THREAD_PRIO -1
#define PRINT(fmt, ...) fprintf(stderr,"[UAC_OUT][func:%s][line:%d] " fmt, __func__,__LINE__,##__VA_ARGS__)

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

static struct uac_playback_runtime uac_playback_rt = {0};

static void hex_dump(const void *src, size_t length, size_t line_size)
{
    int i = 0;
    const unsigned char *address = src;

    while (length-- > 0) {
        PRINT("%02X ", *address++);
        if (!(++i % line_size) || (length == 0 && i % line_size)) {
            PRINT("\n");
        }
    }
}

struct uac_playback_runtime *uac_playback_init(int buf_size)
{
    struct uac_playback_runtime *rt = &uac_playback_rt;

    /*open uac dev file*/
    rt->control_fd = open(UAC_PLAYBACK_CONTROL_DEV_NAME, O_RDONLY);
    if (rt->control_fd < 0) {
        PRINT("open %s failed(%d): %s!\n",
                UAC_PLAYBACK_CONTROL_DEV_NAME, errno, strerror(errno));
       return NULL;
    }

    rt->data_fd = open(UAC_PLAYBACK_DATA_DEV_NAME, O_RDONLY);//  | O_NONBLOCK
    if (rt->data_fd < 0) {
        PRINT("open %s failed(%d): %s!\n",
                UAC_PLAYBACK_DATA_DEV_NAME, errno, strerror(errno));
        return NULL;
    }

    rt->dev_buf_size = buf_size;
    rt->data_buffer = malloc(rt->dev_buf_size);
    if (rt->data_buffer == NULL) {
        PRINT("cannot alloc data buffer!\n");
        return NULL;
    }
    rt->ao_handle = -1;
    
    return rt;

}

int playback_control_process(struct uac_playback_runtime *rt, struct uac_playback_ctrl_data *ctrl_data)
{
    struct usb_ctrlrequest ctrl;
    ssize_t ret = -1;
    uint8_t con_sel;
    uint16_t len, w_value;
    uint32_t value = 0;
    int dump_ctrl = 0, dump_data = 0;

    ret = read(rt->control_fd, &ctrl, sizeof(ctrl));
    if (ret < 0) {
        PRINT("read usb_ctrlrequest failed(%d): %s!\n", errno, strerror(errno));
        return ret;
    } else if (ret < sizeof(ctrl)) {
        PRINT("read usb_ctrlrequest failed!\n");
        dump_ctrl = 1;
        goto error_dump;
    }

    switch (ctrl.bRequestType) {
        case 0x21:
        case 0x22:
        case 0x01:
        case 0x00:
            break;
        default:
            printf("bRequestType(0x%x) is invalid!\n", ctrl.bRequestType);
            dump_ctrl = 1;
    }

    PRINT("bRequestType(0x%x)\n", ctrl.bRequestType);
    PRINT("bRequest(0x%x)\n", ctrl.bRequest);
    PRINT("wValue(0x%x)\n", le16toh(ctrl.wValue));
    PRINT("wLength(%d)\n", le16toh(ctrl.wLength));

    if (ctrl.bRequestType == 0x21 || ctrl.bRequestType == 0x22) {
        w_value = le16toh(ctrl.wValue);
        con_sel = (w_value >> 8) & 0xFF;
        len = le16toh(ctrl.wLength);
        if (len > sizeof(value)) {
            printf("wLength(0x%x) is invalid!\n", len);
            dump_ctrl = 1;
            dump_data = 1;
            goto error_dump;
        }

        ret = read(rt->control_fd, &value, len);
        if (ret < 0) {
            PRINT("read control data failed(%d): %s!\n", errno, strerror(errno));
            return ret;
        } else if (ret < len) {
            PRINT("read control data failed!\n");
            dump_ctrl = 1;
            dump_data = 1;
            goto error_dump;
        }
        
        value = le16toh(value);
        PRINT("value(%d)\n", value);

        if (ctrl.bRequest == UAC_SET_CUR) {
            if (ctrl.bRequestType == 0x21) {
                switch (con_sel) {
                    case UAC_FU_VOLUME:
                    case UAC_FU_MUTE:
                        ctrl_data->cmd = ((con_sel<< 8)| ctrl.bRequestType);
                        ctrl_data->value = value;
                        ret = 0;
                        break;
                    default:
                        PRINT("cs %d not support!\n", con_sel);
                        break;
                }
            }
            if (ctrl.bRequestType == 0x22) {
                switch (con_sel) {
                    case UAC_EP_CS_ATTR_SAMPLE_RATE:
                            ctrl_data->cmd = ((con_sel<< 8) | ctrl.bRequestType);
                            ctrl_data->value = value;
                            ret = 0;
                        break;
                    default:
                        PRINT("bRequestType 0x%x cs %d not support!\n", ctrl.bRequestType, con_sel);
                        break;
                }
            }
        }
    } else {
        if (ctrl.bRequest == USB_REQ_SET_INTERFACE) {
            /*
             * streaming on/off event
             */
            w_value = le16toh(ctrl.wValue);
            if (w_value == 0) {
                printf("playback stream off\n");
            } else {
                printf("playback stream on\n");
            }
        } else if (ctrl.bRequest == AK_USB_REQ_DISCONNECT) {
            /*
             * disconnect event
             */
            printf("disconnect\n");
        } else {
            dump_ctrl = 1;
        }
    }
    return ret;

error_dump:
    if (dump_ctrl) {
        PRINT("usb_ctrlrequest ");
        hex_dump(&ctrl, sizeof(ctrl), 32);
    }
    if (dump_data) {
        PRINT("data 0x%x\n", value);
    }
    return ret;
}


