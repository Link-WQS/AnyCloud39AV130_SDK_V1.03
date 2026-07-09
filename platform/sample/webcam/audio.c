#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include "ak_common.h"
#include "ak_log.h"
#include "ak_thread.h"
#include "ak_mem.h"
#include "ak_ai.h"
#include "ak_ao.h"
#include "uac_in.h"
#include "uac_out.h"
#include "list.h"
#include <sys/time.h>
#include <time.h>

#include <linux/usb/audio.h>

#include "ak_pcm.h"
#include <sys/ioctl.h>

#define AO_BUFF_SIZE    512
#define  EN_AEC    0
#define PRINT(fmt, ...) fprintf(stderr,"[UAC_AUDIO][func:%s][line:%d] " fmt, __func__,__LINE__,##__VA_ARGS__)

#define THREAD_PRIO -1
//static int ai_handle = -1;

#define AO_SAMPLE_BITS_DEFAULT   16
#define AO_CHANNEL_NUM_DEFAULT   1
#define AO_SAMPLE_RATE_DEFAULT   AK_AUDIO_SAMPLE_RATE_8000


static  ak_pthread_t ai_data_th;
static  ak_pthread_t ai_ctrl_th;
static  ak_pthread_t ao_data_th;
static  ak_pthread_t ao_data_tx_th;
static  ak_pthread_t ao_ctrl_th;
struct list_head *ao_head;
static ak_mutex_t ao_lock;
static unsigned int list_num = 0;


typedef struct stream_node{
    unsigned char *data;
    unsigned int len;
    struct list_head list;
}stream_list;

/*
    设置各种音频处理参数结构体为默认值，默认值是从ak_audio_config.h抄的。
    参数仅作参考，实际参数要根据不同产品独立设定。
    1：NEAR(ai), ak_audio_nr_attr
    2：NEAR(ai), ak_audio_agc_attr
    3：NEAR(ai), ak_audio_aec_attr
    4：NEAR(ai), ak_audio_aslc_attr
    5：FAR(ao),  ak_audio_nr_attr
    6：FAR(ao),  ak_audio_aslc_attr
    7：NEAR(ai), ak_ai_set_eq_attr
    8：FAR(ao),  ak_ai_set_eq_attr
*/
static void setup_default_audio_argument(void *audio_args, char args_type)
{
    struct ak_audio_nr_attr     default_ai_nr_attr      = {-40, 0, EN_AEC};
    struct ak_audio_agc_attr    default_ai_agc_attr     = {24576, 4, 0, 80, 0, EN_AEC};
    struct ak_audio_aec_attr    default_ai_aec_attr     = {0, 1024, 1024, 0, 512, EN_AEC, 0};
    struct ak_audio_aslc_attr   default_ai_aslc_attr    = {32768, 0, 0};
    struct ak_audio_eq_attr     default_ai_eq_attr      = {
    0,
    10,
    {50, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {717, 717, 717, 717, 717, 717, 717, 717, 717, 717},
    {TYPE_HPF, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, \
    TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1},
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    struct ak_audio_nr_attr     default_ao_nr_attr      = {0, 0, EN_AEC};
    struct ak_audio_aslc_attr   default_ao_aslc_attr    = {32768, 0, EN_AEC};
    struct ak_audio_eq_attr     default_ao_eq_attr      = {
    0,
    10,
    {50, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {717, 717, 717, 717, 717, 717, 717, 717, 717, 717},
    {TYPE_HPF, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, \
    TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1},
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    switch (args_type) {
    case 1:
        *(struct ak_audio_nr_attr*)audio_args = default_ai_nr_attr;
        break;
    case 2:
        *(struct ak_audio_agc_attr*)audio_args = default_ai_agc_attr;
        break;
    case 3:
        *(struct ak_audio_aec_attr*)audio_args = default_ai_aec_attr;
        break;
    case 4:
        *(struct ak_audio_aslc_attr*)audio_args = default_ai_aslc_attr;
        break;
    case 5:
        *(struct ak_audio_nr_attr*)audio_args = default_ao_nr_attr;
        break;
    case 6:
        *(struct ak_audio_aslc_attr*)audio_args = default_ao_aslc_attr;
        break;
    case 7:
        *(struct ak_audio_eq_attr*)audio_args = default_ai_eq_attr;
        break;
    case 8:
        *(struct ak_audio_eq_attr*)audio_args = default_ao_eq_attr;
        break;

    default:
        break;
    }

    return;
}

/*
 * @BRIEF        stream_buf_in
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */

static int stream_buf_in(struct list_head *frame_list, unsigned char *data, unsigned int len)
{
    stream_list *stream_node;
    
    stream_node = ak_mem_alloc(MODULE_ID_APP, sizeof(stream_list));
    if(NULL == stream_node) {
        ak_print_error_ex(MODULE_ID_APP, "Alloc stream_node failed!\n");
        return AK_FAILED;
    }

    memset(stream_node, 0, sizeof(stream_list));

    stream_node->data = ak_mem_alloc(MODULE_ID_APP, len);
    if(NULL == stream_node->data) {
        ak_print_error_ex(MODULE_ID_APP, "Alloc stream buffer failed!\n");
        ak_mem_free(stream_node);
        return AK_FAILED;
    }

    memcpy(stream_node->data, data, len);
    stream_node->len = len;

    ak_thread_mutex_lock(&ao_lock);
    list_add_tail(&stream_node->list, frame_list);
    list_num++;
    ak_thread_mutex_unlock(&ao_lock);

    return AK_SUCCESS;
}

/**
 * init_ai - open ai, set nr, set agc, set gain, set volume
 * notes:
 */
static int ai_open(int *handle ,int sample_rate)
{
    struct ak_audio_in_param param;
    int channel_num = 1;
    int  frame_interval;
    int ret = -1;

    memset(&param, 0, sizeof(struct ak_audio_in_param));
    ak_print_notice_ex(MODULE_ID_APP, "sample_rate=%d\n", sample_rate);
    param.pcm_data_attr.sample_rate = sample_rate;              // set sample rate
    param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;    // sample bits only support 16 bit
    param.pcm_data_attr.channel_num = channel_num;              // channel number
    param.dev_id = DEV_ADC;                                     //DEV_ADC;
    
    ret = ak_ai_open(&param, handle);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_open failed ret:%d %#x\n",ret,ret);
        return AK_FAILED;
    }

    /* set source, source include mic and linein */
    if (ak_ai_set_source(*handle, AI_SOURCE_MIC)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_source failed\n");
        return AK_FAILED;
    }

    /* enable_nr, nr only support 8000 or 16000 sample rate */
    struct ak_audio_nr_attr nr_attr;
    setup_default_audio_argument(&nr_attr, 1);
    if (sample_rate != AK_AUDIO_SAMPLE_RATE_8000 && sample_rate != AK_AUDIO_SAMPLE_RATE_16000) {
        ak_print_warning_ex(MODULE_ID_APP, "ak_ai_set_nr_attr only support sample rate 8000 or 16000\n");
    } else {
        if (ak_ai_set_nr_attr(*handle, &nr_attr)) {
            ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_nr_attr failed\n");
            return AK_FAILED;
        }
    }

    /*enable_agc, agc only support 8000 or 16000 sample rate */
    struct ak_audio_agc_attr agc_attr;
    setup_default_audio_argument(&agc_attr, 2);
    if (sample_rate != AK_AUDIO_SAMPLE_RATE_8000 && sample_rate != AK_AUDIO_SAMPLE_RATE_16000) {
        ak_print_warning_ex(MODULE_ID_APP, "ak_ai_set_agc_attr only support sample rate 8000 or 16000\n");
    } else {
        if (ak_ai_set_agc_attr(*handle, &agc_attr)) {
            ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_agc_attr failed\n");
            return AK_FAILED;
        }
    }

    /*enable_aec, aec only support 8000 or 16000 sample rate, aec will real open when ai and ao all open */
    struct ak_audio_aec_attr aec_attr;
    setup_default_audio_argument(&aec_attr, 3);
    if (sample_rate != AK_AUDIO_SAMPLE_RATE_8000 && sample_rate != AK_AUDIO_SAMPLE_RATE_16000) {
        ak_print_warning_ex(MODULE_ID_APP, "ak_ai_set_aec_attr only support sample rate 8000 or 16000\n");
    } else {
        if (ak_ai_set_aec_attr(*handle, &aec_attr)) {
            ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_aec_attr failed\n");
            return AK_FAILED;
        }
    }

    if (ak_ai_set_gain(*handle, 5)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_gain failed\n");
        return AK_FAILED;
    }

    struct ak_audio_aslc_attr ai_aslc_attr;
    setup_default_audio_argument(&ai_aslc_attr, 4);
    if (ak_ai_set_aslc_attr(*handle, &ai_aslc_attr)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_aslc_attr failed\n");
        return AK_FAILED;
    }

    struct ak_audio_eq_attr ai_eq_attr;
    setup_default_audio_argument(&ai_eq_attr, 7);
    if (ak_ai_set_eq_attr(*handle, &ai_eq_attr)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_eq_attr failed\n");
        return AK_FAILED;
    }

    int frame_len = 0;
    if (ak_ai_get_frame_length(*handle, &frame_len)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_frame_length failed\n");
        return AK_FAILED;
    }
    // frame_len = frame_len*2;
    // ak_ai_set_frame_length(*handle, frame_len);

    frame_interval = ak_audio_len_to_interval(&param.pcm_data_attr, frame_len);
    PRINT("%s:frame_interval=%d\n",__func__,frame_interval);
    if (ak_ai_start_capture(*handle)) {
        ak_print_error(MODULE_ID_APP, "*** ak_ai_start_capture failed. ***\n");
        return AK_FAILED;
    }

    PRINT("ai open ok\n");
    return frame_interval;
}

/*
 * @BRIEF        ai_close
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */

int ai_close(int handle)
{
    if(handle == -1)
        return -1;

    PRINT("%s:%d\n",__func__,__LINE__);
    ak_ai_stop_capture(handle);
    ak_ai_close(handle);
    handle = -1;

    return AK_SUCCESS;
}

/*
 * @BRIEF        ai_set_volume
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */
static int ai_set_volume(int handle,int value)
{
    int vol;

    if (handle == -1) {
        PRINT("%s: ai capture has not opened\n", __func__);
        return -1;
    }
    vol = (value/5  - 31);
    ak_ai_set_volume(handle,vol);
}

/*
 * @BRIEF        ai_set_mute
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */
static int ai_set_mute(int handle,int value)
{
    PRINT("%s:value=%d\n",__func__,value);
    return 0;
}

/*
 * @BRIEF        uac_capture_control_thread
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */
static void *uac_capture_control_thread(void *arg)
{
    int ret;
    struct uac_capture_runtime *capture =  (struct uac_capture_runtime *)arg;
    fd_set rfds;
    //struct timeval tv;
    struct uac_capture_ctrl_data ctrl;

    while(1) 
    {
        FD_ZERO(&rfds);
        FD_SET(capture->control_fd, &rfds);
        //tv.tv_sec = 0;
        //tv.tv_usec = 0;
        ret = select(capture->control_fd + 1, &rfds, NULL, NULL, NULL);
        if (ret <= 0) {
            PRINT("select failed(%d): %s\n", errno, strerror(errno));
            continue;
        }
        ret = capture_control_process(capture, &ctrl);
        if(!ret)
        {
            switch (ctrl.cmd)
            {
            case (0x21|0x1<<8):  //UAC_FU_MUTE
                ai_set_mute(capture->ai_handle, ctrl.value);
                break;
            case (0x21|0x2<<8): //UAC_FU_VOLUME
                ai_set_volume(capture->ai_handle, ctrl.value);
               break;
            case (0x22|0x1<<8): //SAMPLE_RATE
                if(capture->ai_handle != -1)
                {
                    capture->started = 0;
                    ak_sleep_ms(10);
                    ai_close(capture->ai_handle);
                }
               capture->frame_interval = ai_open(&(capture->ai_handle), ctrl.value);
               ak_sleep_ms(10);
               capture->started = 1;
               break;
            default:
                break;
            }
        }
    }
    return NULL;
}

/*
 * @BRIEF        uac_capture_data_thread
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */
static void *uac_capture_data_thread(void *arg)
{
    struct uac_capture_runtime *capture =  (struct uac_capture_runtime *)arg;
    fd_set  wfds;
    struct frame frame = {0};
    int ret = -1;
    //FILE *save_fp = NULL;
    //save_fp = fopen("/tmp/uac_input.pcm", "w+");

    while(1) 
    {
        if (capture->started && capture->ai_handle != -1) {
            ret = ak_ai_get_frame(capture->ai_handle, &frame, 0);
            if (ret) {
                ak_sleep_ms(5);
                continue;   
            }
            //fwrite((char*)frame.data, 1, frame.len, save_fp);
            FD_ZERO(&wfds);
            FD_SET(capture->data_fd, &wfds);
            ret = select(capture->data_fd + 1,NULL, &wfds, NULL, NULL);
            if (ret <= 0) {
                PRINT("select failed(%d): %s\n", errno, strerror(errno));
                ak_ai_release_frame(capture->ai_handle, &frame);
                continue;
            }
            // PRINT("sdata(%d): %s\n", frame.len);
            capture_data_process(capture,frame.data,frame.len);   
            ak_ai_release_frame(capture->ai_handle, &frame);
        } else {
            ak_sleep_ms(5);
        }
    }
    return NULL;
}

/*
 * @BRIEF        uac_capture_process
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */
int uac_capture_process(struct uac_capture_runtime *uac_in_rt)
{
    int ret;

    ret = ak_thread_create(&ai_ctrl_th, uac_capture_control_thread, uac_in_rt, \
                            ANYKA_THREAD_MIN_STACK_SIZE, 90);
    ret = ak_thread_create(&ai_data_th, uac_capture_data_thread, uac_in_rt, \
                            ANYKA_THREAD_MIN_STACK_SIZE, 88);
    return ret;
}

/**
 * init_ao - open ao
 * notes:
 */
static int ao_open(int *handle, int sample_rate)
{
    if(*handle != -1)
    {
        PRINT("ao already open!\n");
        return -1;
    }

    struct ak_audio_out_param ao_param;

    ao_param.pcm_data_attr.sample_bits = AO_SAMPLE_BITS_DEFAULT;
    ao_param.pcm_data_attr.channel_num = AO_CHANNEL_NUM_DEFAULT;
    ao_param.pcm_data_attr.sample_rate = sample_rate;
    ao_param.dev_id = DEV_DAC;

    /* open ao */
    if (ak_ao_open(&ao_param, handle)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ao_open failed\n");
        return AK_FAILED;
    }

    if (ak_ao_set_gain(*handle, 6)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ao_set_gain failed\n");
        return AK_FAILED;
    }

    if ((AUDIO_CHANNEL_MONO == ao_param.pcm_data_attr.channel_num )) {
        struct ak_audio_nr_attr nr_attr;
        setup_default_audio_argument(&nr_attr, 5);

        /* enable_nr, nr only support 8000 or 16000 sample rate */
        if (sample_rate != AK_AUDIO_SAMPLE_RATE_8000 && sample_rate != AK_AUDIO_SAMPLE_RATE_16000) {
            ak_print_warning_ex(MODULE_ID_APP, "ak_ai_enable_agc only support sample rate 8000 or 16000\n");
        } else {
            if (ak_ao_set_nr_attr(*handle, &nr_attr)) {
                ak_print_error_ex(MODULE_ID_APP, "ak_ao_set_nr_attr failed\n");
                return AK_FAILED;
            }
        }
    }

    struct ak_audio_aslc_attr ao_aslc_attr;
    setup_default_audio_argument(&ao_aslc_attr, 6);
    if (ak_ao_set_aslc_attr(*handle, &ao_aslc_attr)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ao_set_aslc_attr failed\n");
        return AK_FAILED;
    }

    struct ak_audio_eq_attr ao_eq_attr;
    setup_default_audio_argument(&ao_eq_attr, 8);
    if (ak_ao_set_eq_attr(*handle, &ao_eq_attr)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ao_set_eq_attr failed\n");
        return AK_FAILED;
    }

    PRINT("ao open ok\n");
    return AK_SUCCESS;
}

/*
 * @BRIEF        ao_send_frame
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */
static int ao_send_frame(int handle, unsigned char *data, int len)
{
    int send_len = 0;
    int ret = 0;
    /* send frame and play */
#if USE_PCM_DRIVER
    send_len = write(handle, data, len);
    if (send_len != len)
        PRINT("not all data is writed to playback pcm: %d,%d\n", len, send_len);
#else
    if(handle != -1 && data != NULL && len >0)
    {
        ret = ak_ao_send_frame(handle, data, len, &send_len);
        if(ret)
        {
            PRINT("ak_ao_send_frame failed !! ret:%#x %d handle:%d len:%d\n",ret,ret,handle,len);
        }
    }
        
#endif

    return send_len;
}
/*
 * @BRIEF        ao_close
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */
static int ao_close(int *handle)
{
    if (*handle == -1)
    {
        PRINT("ao already close!\n");
        return -1;
    }
    
    ak_ao_close(*handle);
    *handle = -1;
    return 0;
}
/*
 * @BRIEF        ao_set_volume
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */
static int ao_set_volume(int handle, int value)
{
    int vol;

    if (handle == -1)
        return -1;

#if USE_PCM_DRIVER
    /*
     * turn value to gain
     */
    vol = (value - VOLUME_MIN) * (HEADPHONE_GAIN_MAX - HEADPHONE_GAIN_MIN)
    / (VOLUME_MAX - VOLUME_MIN) + HEADPHONE_GAIN_MIN;
    if (ioctl(handle, IOC_SET_GAIN, (void*)(&vol)) < 0) {
        PRINT("IOC_SET_GAIN failed(%d): %s!\n", errno, strerror(errno));
    }
#else
    vol = (value/5 - 31);
    ak_ao_set_volume(handle, vol);
#endif
    return 0;
}
/*
 * @BRIEF        ao_set_mute
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */
static int ao_set_mute(int handle, int value)
{
    int vol;

    if (value) {
        /*
         * disable speaker
         */
        #if USE_PCM_DRIVER
        vol = 0;
        if (ioctl(handle, IOC_SET_SPEAKER, (void*)&vol) < 0) {
            PRINT("IOC_SET_SPEAKER failed(%d): %s!\n", errno, strerror(errno));
        }
        #else
        ak_ao_cancel(handle);
        #endif
#if 0
        vol = MIXER_OUT_SEL_MUTE;
        if (ioctl(rt->pcm_fd, IOC_SET_SOURCES, (void*)(&vol)) < 0) {
            printf("IOC_SET_SOURCES failed(%d): %s!\n", errno, strerror(errno));
        }
#endif
    } else {
        /*
         * enable speaker
         */
        #if USE_PCM_DRIVER
        vol = 1;
        if (ioctl(handle, IOC_SET_SPEAKER, (void*)&vol) < 0) {
            PRINT("IOC_SET_SPEAKER failed(%d): %s!\n", errno, strerror(errno));
        }
        #else
        ak_ao_restart(handle);
        #endif
#if 0
        vol = MIXER_OUT_SEL_DAC;
        if (ioctl(rt->pcm_fd, IOC_SET_SOURCES, (void*)(&vol)) < 0) {
            printf("IOC_SET_SOURCES failed(%d): %s!\n", errno, strerror(errno));
        }
#endif
    }
    return 0;
}
/*
 * @BRIEF        ao_set_sample_rate
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */
static void ao_set_sample_rate(struct uac_playback_runtime *playback, int sample_rate)
{
#if USE_PCM_DRIVER
    int value;
    /*
     * pause
     */
    if (ioctl(playback->ao_handle, IOC_RSTBUF, NULL) < 0) {
        PRINT("IOC_RSTBUF failed(%d): %s!\n", errno, strerror(errno));
    }
    /*
     * set pcm sample rate
     */
    playback->sample_rate = sample_rate;
    playback->para.rate   = sample_rate;
    if (ioctl(playback->ao_handle, IOC_PREPARE, (void *)(&(playback->para))) < 0) {
        PRINT("IOC_PREPARE failed(%d): %s!\n", errno, strerror(errno));
    }
    /*
     * enable speaker
     */
    value = 1;
    if (ioctl(playback->ao_handle, IOC_SET_SPEAKER, (void*)&value) < 0) {
        PRINT("IOC_SET_SPEAKER failed(%d): %s!\n", errno, strerror(errno));
    }
    /*
     * resume
     */
    if (ioctl(playback->ao_handle, IOC_RESUME, NULL) < 0) {
        PRINT("IOC_RESUME failed(%d): %s\n", errno, strerror(errno));
    }

#else
    ao_close(&playback->ao_handle);
    ao_open(&playback->ao_handle, sample_rate);
#endif
}
/*
 * @BRIEF        uac_playback_control_thread
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */
static void *uac_playback_control_thread(void *arg)
{
    int ret;
    fd_set fds;
    struct uac_playback_runtime *playback = (struct uac_playback_runtime *)arg;
    struct uac_playback_ctrl_data ctrl;
    struct timeval tv = {0};

    while (1) 
    {
        FD_ZERO(&fds);
        FD_SET(playback->control_fd, &fds);

        tv.tv_sec = 2;
        tv.tv_usec = 0;

        ret = select(playback->control_fd + 1, &fds, NULL, NULL, &tv);
        if (ret == 0) 
        {
            // PRINT("select timeout\n");
            continue;
        }
        if (ret == -1)
        {
            PRINT("select error\n");
            continue;
        }

        if (FD_ISSET(playback->control_fd, &fds))
        {
            memset(&ctrl, 0, sizeof(ctrl));
            ret = playback_control_process(playback, &ctrl);
            if(!ret)
            {
                switch (ctrl.cmd)
                {
                case (0x21 | UAC_FU_MUTE<<8):
                    ao_set_mute(playback->ao_handle, ctrl.value);
                    break;
                case (0x21 | UAC_FU_VOLUME<<8):
                    ao_set_volume(playback->ao_handle, ctrl.value);
                    break;
                case (0x22 | UAC_EP_CS_ATTR_SAMPLE_RATE<<8):
                    if (playback->sample_rate != ctrl.value) {
                        ao_set_sample_rate(playback, ctrl.value);
                        playback->started = 1;
                        // playback->delay = 1;
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
    return NULL;
}
/*
 * @BRIEF        uac_playback_data_thread
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */

static void *uac_playback_data_thread(void *arg)
{
    int ret, read_bytes;
    fd_set rfds;
    struct uac_playback_runtime *playback = (struct uac_playback_runtime *)arg;
    struct timeval tv = {0};

    while (1)
    {
        FD_ZERO(&rfds);
        FD_SET(playback->data_fd, &rfds);

        tv.tv_sec = 2;
        tv.tv_usec = 0;//10 * 1000;

        ret = select(playback->data_fd + 1, &rfds, NULL, NULL, &tv);
        if (ret == 0) {
            // PRINT("%s select timeout\n", UAC_PLAYBACK_DATA_DEV_NAME);
            continue;
        }

        if (ret == -1) {
            PRINT("%s select error\n", UAC_PLAYBACK_DATA_DEV_NAME);
            continue;
        }

        if (FD_ISSET(playback->data_fd, &rfds)) {
            read_bytes = read(playback->data_fd, playback->data_buffer, playback->dev_buf_size);
            // unsigned int test_var = 0;
            // PRINT("read_bytes:%d (%d)\n", read_bytes, ++test_var);

            if (playback->started) 
            {
                ao_send_frame(playback->ao_handle, playback->data_buffer, read_bytes);
            }
            
            // stream_buf_in(ao_head, playback->data_buffer, read_bytes);
        }
    }
    return NULL;
}

#if 0
/*
 * @BRIEF        uac_playback_data_tx_thread
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */
static void *uac_playback_data_tx_thread(void *arg)
{
    stream_list *pos = NULL;
    struct uac_playback_runtime *playback = (struct uac_playback_runtime *)arg;

    while (1)
    {
        if (!list_empty(ao_head)) {
            /* get one frame from the frame list*/
            if(playback->delay) {
                if(list_num < 16) {
                    ak_sleep_ms(5);
                    continue;
                } else {
                    playback->delay = 0;
                }
            }

            pos = list_first_entry(ao_head, stream_list, list);
            if (playback->started) {
                ao_send_frame(playback->ao_handle, pos->data, pos->len);
            }

            // unsigned int test_var = 0;
            // PRINT("send ao frame(%d)\n", ++test_var);

            /*delete the pos from the frame list*/
            ak_thread_mutex_lock(&ao_lock);
            list_del(&pos->list);
            list_num--;
            ak_thread_mutex_unlock(&ao_lock);

            ak_mem_free(pos->data);
            ak_mem_free(pos);
        } else {
            ak_sleep_ms(5);
        }
    }
    return NULL;
}
#endif

/*
 * pcm device files
 */
#define DAC_DEV_FILE_NAME  "/dev/pcmC0D0p"
#define I2S0_DEV_FILE_NAME "/dev/pcmC1D0p"
static int pcm_open(int *handle, struct akpcm_pars *pars)
{
    int pcm_fd;

    pcm_fd = open(DAC_DEV_FILE_NAME, O_WRONLY);
    if (pcm_fd < 0) {
        PRINT("open device file failed(%d): %s!\n", errno, strerror(errno));
        return -1;
    }

    if (fcntl(pcm_fd, F_SETFD, FD_CLOEXEC)) {
        PRINT("device file setfd failed(%d): %s!\n", errno, strerror(errno));
        close(pcm_fd);
        return -1;
    }

    /*
     * set source
     */
    int value = MIXER_OUT_SEL_DAC;
    if (ioctl(pcm_fd, IOC_SET_SOURCES, (void*)(&value)) < 0) {
        PRINT("IOC_SET_SOURCES failed(%d): %s!\n", errno, strerror(errno));
        return -3;
        goto err_close_pcm_fd;
    }

    /*
     * set gain
     */
    value = 0;
    if (ioctl(pcm_fd, IOC_SET_GAIN, (void*)(&value)) < 0) {
        PRINT("IOC_SET_GAIN failed(%d): %s!\n", errno, strerror(errno));
        return -3;
        goto err_close_pcm_fd;
    }

    /*
     * set parameters
     */
    if (ioctl(pcm_fd, IOC_PREPARE, (void*)pars) < 0) {
        PRINT("IOC_PREPARE failed(%d): %s!\n", errno, strerror(errno));
        return -5;
        goto err_close_pcm_fd;
    }

    /*
     * enable speaker
     */
    value = 1;
    if (ioctl(pcm_fd, IOC_SET_SPEAKER, (void*)&value) < 0) {
        PRINT("IOC_SET_SPEAKER failed(%d): %s!\n", errno, strerror(errno));
        return -6;
        goto err_close_pcm_fd;
    }

    /*
     * pause
     */
    if (ioctl(pcm_fd, IOC_RSTBUF, NULL) < 0) {
        PRINT("IOC_RSTBUF failed(%d): %s!\n", errno, strerror(errno));
        return -7;
        goto err_close_pcm_fd;
    }

    *handle = pcm_fd;
    return 0;

err_close_pcm_fd:
    if (pcm_fd)
        close(pcm_fd);
    return -1;
}
/*
 * @BRIEF        uac_playback_process
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */

int uac_playback_process(struct uac_playback_runtime *uac_out_rt)
{
    int ret;

#if USE_PCM_DRIVER
    uac_out_rt->para.id = 0;
    uac_out_rt->para.rate = AO_SAMPLE_RATE_DEFAULT;
    uac_out_rt->para.channels = 1;
    uac_out_rt->para.sample_bits  = 16;
    uac_out_rt->para.period_bytes = uac_out_rt->dev_buf_size;
    uac_out_rt->para.periods = 8;
    ret = pcm_open(&uac_out_rt->ao_handle, &uac_out_rt->para);
    if (ret) {
        ak_print_error_ex(MODULE_ID_APP, "pcm open error:%d\n",ret);
        return -1;
    }
#else
    ao_open(&uac_out_rt->ao_handle, AO_SAMPLE_RATE_DEFAULT);
#endif

    ret = ak_thread_create(&ao_ctrl_th, uac_playback_control_thread, uac_out_rt, \
                            ANYKA_THREAD_MIN_STACK_SIZE, 90);
    ret = ak_thread_create(&ao_data_th, uac_playback_data_thread, uac_out_rt, \
                            ANYKA_THREAD_MIN_STACK_SIZE, 87);
    // ret = ak_thread_create(&ao_data_tx_th, uac_playback_data_tx_thread, uac_out_rt, \
    //                        ANYKA_THREAD_MIN_STACK_SIZE, 88);
    return ret;
}
/*
 * @BRIEF        audio_entry
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */

void audio_entry(void)
{
    struct uac_capture_runtime *uac_in_rt;
    struct uac_playback_runtime *uac_out_rt;

    uac_in_rt = uac_capture_init();
    if(uac_in_rt == NULL) {
        ak_print_error_ex(MODULE_ID_APP, "uac capture init error!\n");
        return;
    }

    uac_capture_process(uac_in_rt);

    ao_head = ak_mem_alloc(MODULE_ID_APP, sizeof(struct list_head));
    if(ao_head == NULL) {
        ak_print_error_ex(MODULE_ID_APP,"Alloc ao_head failed!\n");
        return;
    }

    INIT_LIST_HEAD(ao_head);
    ak_thread_mutex_init(&ao_lock, NULL);

    uac_out_rt = uac_playback_init(AO_BUFF_SIZE);
    if(uac_out_rt == NULL) {
        ak_print_error_ex(MODULE_ID_APP, "uac playback init error!\n");
        return;
    }

    uac_playback_process(uac_out_rt);
}
/*
 * @BRIEF        audio_exit
 * @DATE date
 * @PARAM[in]
 * @RETURN
 * @RETVAL
 */

int audio_exit(void)
{
    ak_thread_join(ai_ctrl_th);
    ak_thread_join(ai_data_th);
    ak_thread_join(ao_ctrl_th);
    ak_thread_join(ao_data_th);
    // ak_thread_join(ao_data_tx_th);
}
