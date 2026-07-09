/*
 * @file  ak_pcm_uapi.h
 * @brief sound common definations
 * Copyright (C) 2021 Anyka (Guangzhou) Software Technology Co., LTD
 * @version 1.0
 */

#ifndef __AK_PCM_UAPI_H__
#define __AK_PCM_UAPI_H__

#include <linux/cdev.h>
#include <linux/clk.h>

#include "../include/ak_pcm.h"

#define USE_FORMATS           (AKPCM_FMTBIT_U8 | AKPCM_SMPL_BIT_U16)
#define USE_RATE              (AKPCM_RATE_CONTINUOUS | AKPCM_RATE_ALL)
#define USE_RATE_MIN          8000
#define USE_RATE_MAX          48000
#define PLAY_PERIOD_BYTES_MIN 512
#define PLAY_PERIOD_BYTES_MAX 65536
#define PLAY_PERIODS_MIN      4
#define PLAY_PERIODS_MAX      1024
#define DELAYS_FOR_CLOSE_DAC  (HZ * 30)
#define CAPT_PERIOD_BYTES_MIN 512
#define CAPT_PERIOD_BYTES_MAX 32768
#define CAPT_PERIODS_MIN      4
#define CAPT_PERIODS_MAX      512
#define MAX_TIMESTAMP_CNT     80

/* GAIN channels*/
#define MIXER_VOL_HP  0
#define MIXER_VOL_LI  1
#define MIXER_VOL_MIC 2
#define MIXER_VOL_END 2

/* GAIN  default value */
#define DEFAULT_HPVOL     4
#define DEFAULT_LINEINVOL 3
#define DEFAULT_MICVOL    3

/* Mixer */
#define MIXER_OUT 0
#define MIXER_IN  1
#define MIXER_NUM 2

/* devices for playback */
#define PLAYDEV_HP  (AKPCM_PLAYDEV_HP)
#define PLAYDEV_MSK (AKPCM_PLAYDEV_HP)

/* devices for capture */
#define CPTRDEV_MIC (AKPCM_CPTRDEV_MIC)
#define CPTRDEV_LI  (AKPCM_CPTRDEV_LI)
#define CPTRDEV_MSK (AKPCM_CPTRDEV_MIC | AKPCM_CPTRDEV_LI)

typedef enum {
    I2SST_MODE = 0, // i2s slave send mode
    I2SMT_MODE, // i2s master send mode
    I2SSR_MODE, // i2s slave receive mode
    I2SMR_MODE, // i2s master receive mode
    I2SM_Duplex_MODE, // i2s master send and receive mode
    I2SS_Duplex_MODE, // i2s slave send and receive mode
    I2S_MODE_MAX,
} i2s_mode;

typedef enum {
    I2S0,
    I2S1,
    I2S_MAX,
} i2s_id;

enum snd_hwparams_type {
    SND_SET_GAIN = 0,
    SND_SET_MUX,
    SND_SPEAKER_CTRL,
    SND_SET_POWER,
};

enum akpcm_device {
    AK_PCM_DEV_PLAYBACK,
    AK_PCM_DEV_CAPTURE,
    AK_PCM_DEV_LOOPBACK,
    AK_PCM_DEV_MAX,
};

typedef enum snd_card_type {
    SND_CARD_DAC_PLAYBACK = 0,
    SND_CARD_ADC_CAPTURE,
    SND_CARD_DAC_LOOPBACK,
    SND_CARD_I2S0_SEND,
    SND_CARD_I2S0_RECV,
    SND_CARD_I2S0_LOOPBACK,
    SND_CARD_I2S1_RECV,
    SND_CARD_PDM_RECV,
    SND_CARD_TYPE_MAX,
} snd_dev;

typedef struct snd_format {
    unsigned long sample_rate;
    int channel;
    int bps; // bit per sample
} SND_FORMAT;

typedef struct snd_card_ops {
    int (*open)(snd_dev dev);
    int (*close)(snd_dev dev);
    unsigned long (*set_format)(snd_dev dev, SND_FORMAT* format);
    int (*hwparam)(
        snd_dev dev, enum snd_hwparams_type type, int addr, unsigned int param);
} SND_CARD_OPS;

typedef struct snd_controller {
    unsigned char l2_buf_id;
    SND_CARD_OPS* ops;
} SND_CONTROLLER;

struct akpcm_pm_ops {
    int (*suspend)(void* data);
    int (*resume)(void* data);
};

struct i2s_param {
    unsigned int sample_rate; // sample rate
    unsigned int mclk; // i2s mclk

    struct clk* source_clk;
    struct clk* sdadc_gclk;
    struct clk* sddac_gclk;
    struct clk* i2s0_mclk;
    struct clk* i2s0_b_lr_clk;

    void __iomem* rx_base;
    void __iomem* tx_base;

    unsigned int i2s_mode; // i2s workmode, I2S_MODE list
    unsigned char i2sr_pol; // i2s polarity, 0: L_low, R_high, 1: L_high, R_low
    unsigned char i2st_pol; // i2s polarity, 0: L_low, R_high, 1: L_high, R_low
    unsigned char i2s_clksource; // i2s pllclk_source, 0:code_pll, 1:audio_pll
};

struct akpcm_runtime {
    struct list_head list;
    struct device* dev;
    struct akpcm_pars cfg;
    unsigned int hw_ptr; /* hardware ptr*/
    unsigned int app_ptr; /* application ptr*/
    spinlock_t ptr_lock; /* lock to protect hw_ptr & app_ptr */
    struct mutex io_lock; /* mutex to pretect ioctl */

    /* -- SW parameters -- */
    unsigned int buffer_bytes;
    unsigned int boundary;

    /* -- DMA -- */
    unsigned char* dma_area; /* DMA area(vaddr) */
    dma_addr_t dma_addr; /* physical bus address */

    unsigned long long ts;

    /* cache buff */
    unsigned char* cache_buff; /* DMA area(vaddr) */

    /* runtime timestamp */
    unsigned long long timestamp[MAX_TIMESTAMP_CNT];

    /* the completion for playback or capture */
    struct completion rt_completion;

    /* wait_queue_head_t member */
    wait_queue_head_t wq;

    unsigned long actual_rate; /* actual rate in Hz */
    unsigned int hp_gain_cur; /* current value for headphone gain */
    unsigned int linein_gain_cur; /* current value for linein gain */
    unsigned int mic_gain_cur; /* current value for mic gain */

    /* bit[0]-opened; bit[1]-prepared ,
     * bit[2]-stream; bit[3]-DMA; bit[4] -pausing */
    unsigned long runflag;

    long long remain_app_bytes;
    struct buf_status ak_status;

    /* the mixer_source for MIXER_OUT and MIXER_IN */
    int mixer_source;

    int notify_threshold;
    int snd_dev;
    int snd_type;
    int snd_idx;

    struct akpcm_pm_ops pm_ops;
    bool is_suspended;

    unsigned int last_transfer_count;
    unsigned int last_isr_count;
    unsigned int log_print;
    unsigned int io_trace_in;
    unsigned int io_trace_out;
    struct cdev cdev;
    struct work_struct stop_work;
};

typedef struct akpcm_runtime* (*akpcm_runtime_register_func)(
    struct device_node* np, int snd_dev, int snd_type);

typedef int (*snd_card_register_func)(
    snd_dev dev, struct snd_controller* controller);

#endif //__AK_PCM_UAPI_H__
/*end of file*/
