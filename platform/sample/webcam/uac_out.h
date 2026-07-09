#ifndef _UAC_OUT_H_
#define _UAC_OUT_H_

#define UAC_PLAYBACK_CONTROL_DEV_NAME  "/dev/uacp0"
#define UAC_PLAYBACK_DATA_DEV_NAME  "/dev/uacp1"

#include "ak_pcm.h"

#define USE_PCM_DRIVER           0

struct uac_playback_runtime {
    int control_fd, data_fd;
    #if USE_PCM_DRIVER
    struct akpcm_pars para;
    #endif
    unsigned char *data_buffer;
    int dev_buf_size;
    int periods, sample_rate;
    unsigned int started;
    unsigned int delay;
    int ao_handle;
}; 
struct uac_playback_ctrl_data{
    int cmd;
    int value;
};
extern struct uac_playback_runtime * uac_playback_init(int buf_size);
extern int playback_control_process(struct uac_playback_runtime *rt,struct uac_playback_ctrl_data *ctrl_data);
#endif