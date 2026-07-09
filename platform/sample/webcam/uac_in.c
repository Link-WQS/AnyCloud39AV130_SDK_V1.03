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
#include "ak_ai.h"
#include "ak_thread.h"
#include "uac_in.h"

#define THREAD_PRIO -1
#define PRINT(fmt, ...) fprintf(stderr,"[UAC_IN][func:%s][line:%d] " fmt, __func__,__LINE__,##__VA_ARGS__)


static struct uac_capture_runtime uac_capture_rt = {0};

extern int ai_close(int handle);


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


struct uac_capture_runtime *uac_capture_init(void)
{
    struct uac_capture_runtime *capture = &uac_capture_rt;

    capture->control_fd = open(UAC_CAPTURE_CONTROL_FILE_NAME, O_RDONLY);
    if (capture->control_fd < 0) {
        PRINT("open %s failed(%d): %s!\n",
                UAC_CAPTURE_CONTROL_FILE_NAME, errno, strerror(errno));
        return NULL;
    }

    capture->data_fd = open(UAC_CAPTURE_DATA_FILE_NAME, O_WRONLY );//O_NONBLOCK
    if (capture->data_fd < 0) {
        PRINT("open %s failed(%d): %s!\n",
                UAC_CAPTURE_DATA_FILE_NAME, errno, strerror(errno));
        close(capture->control_fd);
        return NULL;
    }
    capture->frame_interval = 32;
    capture->started = 0;
    capture->ai_handle = -1;
    
    return capture;
}

int capture_control_process(struct uac_capture_runtime *capture, struct uac_capture_ctrl_data *ctrl_data)
{
    struct usb_ctrlrequest ctrl;
    ssize_t ret = -1;
    uint8_t con_sel;
    uint16_t len, w_value;
    uint32_t value = 0;
    int dump_ctrl = 0, dump_data = 0;
    int val;

    ret = read(capture->control_fd, &ctrl, sizeof(ctrl));
    if (ret < 0) {
        PRINT("read usb_ctrlrequest failed(%d): %s!\n", errno, strerror(errno));
        return ret;
    } else if (ret < sizeof(ctrl)) {
        PRINT("read usb_ctrlrequest failed!\n");
        dump_ctrl = 1;
        goto error_dump;
    }

    PRINT("bRequestType(0x%x) bRequest(0x%x) wValue(0x%x) wLength(%d)\n", 
        ctrl.bRequestType,ctrl.bRequest,le16toh(ctrl.wValue),le16toh(ctrl.wLength));
    
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

        ret = read(capture->control_fd, &value, len);
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
                                ctrl_data->cmd = ((con_sel<< 8)| ctrl.bRequestType);
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
             * stream on/off event
             */
            w_value = le16toh(ctrl.wValue);
            if (w_value == 0) {
                printf("capture stream off %d\n",capture->ai_handle);
                capture->started = 0;
                // ai_close(capture->ai_handle);
            } else {
                printf("capture stream on w_value:%d\n",w_value);
            }
        } else if (ctrl.bRequest == 0xf0) {
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

void capture_data_process(struct uac_capture_runtime *capture, unsigned char *data, unsigned int len)
{
    int ret;
    ret = write(capture->data_fd, data, len);
}
