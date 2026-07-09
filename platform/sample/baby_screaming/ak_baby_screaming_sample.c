/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_baby_screaming_sample.c
* Description: This is a simple example to show how the VAD module BABY_SCREAM working.
* Notes:Before running please insmod ak_pcm.ko at first.
* History: V1.0.0
*/
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

#include "ak_ai.h"
#include "ak_common.h"
#include "ak_log.h"
#include "ak_mem.h"
#include "ak_thread.h"

#include "ak_npu.h"
#include "ak_reg_AIcry.h"


#ifdef AK_RTOS
#include <kernel.h>
#endif

#define LEN_HINT         512
#define LEN_OPTION_SHORT 512
#define DEV_STR_LEN      10
#define AK32Q15(x)    ((signed long)((x)*(1<<15)))

static void setup_default_audio_argument(void *audio_args, char args_type);

static const char model_def_parth[] = "/tmp/nfs/model_aicrydet_V1.00.02.bin";
static const char save_def_parth[] = "/mnt/";
static FILE *fp = NULL;
static int sample_rate = 8000;
static int volume = 5;
static int save_time = 20000;        // set save time(ms)
static char *save_path= (char *)save_def_parth;    // set save path
static char *model_name= (char *)model_def_parth;    // set model path
static int channel_num = AUDIO_CHANNEL_MONO;
static int dev_sel = DEV_ADC;
static int cry_mode = AK_VAD_TYPE_PARAM(AK_VAD_DET_AFTER_NR, AK_VAD_DET_BABY_SCREAM_AI , 0, 0);
// 哭声检测方式，33=0x21:选择用npu
static int frame_interval = 32;
static int ai_handle_id = -1;
static int vad_thresh = AK32Q15(0.05);//327
static int vad_level = 0;
/* static float vad_thresh_fs_sum = 0;// vad thresh in FS,
 *vad thresh in FS is vad_thresh/AK32Q15(1), 10 times vad thresh in FS add together
 */

/* ***********************************
    ***********************************
    *
    use this sample
    must follow this:
    1. make sure the driver is insmode;
    2. mount the T card;
    3. the file path is exit;
    *
    ***********************************
    ***********************************
*/

static const char ac_option_hint[][LEN_HINT] = {
    "	打印帮助信息" ,
    "	采集时间" ,
    "	采样率[8000  16000 ]" ,
    "	通道数[1 ]" ,
    "	音量[-90~20db]" ,
    "	文件保存路径" ,
    "	设备选择,0-adc,1-pdm, 2-I2S0" ,
    "	哭声检测的方式选择,默认(33:使用NPU方式哭声检测(只支持16K采样率),17:使用纯软件方式哭声检测)" ,
    "	侦测声音阈值,默认(327)" ,
    "	侦测声音等级,默认(2)" ,
    "	npu方式时使用的模型参数文件名带路径" ,
    ""
};

static struct option option_long[] = {
    { "help"        , no_argument       , NULL , 'h' } , //"       打印帮助信息" ,
    { "second"      , required_argument , NULL , 't' } , //"[SEC]  采集时间" ,
    { "sample-rate" , required_argument , NULL , 's' } , //"[NUM]  采样率[8000 16000 32000 44100 48000]" ,
    { "channel-number" , required_argument , NULL , 'c' } , //"[NUM]  通道数[1 2]" ,
    { "volume"      , required_argument , NULL , 'v' } , //"[NUM]  数字音量[-90~20db]" ,
    { "path"        , required_argument , NULL , 'p' } , //"[PATH] 文件保存路径" ,
    { "dev-sel"     , required_argument , NULL , 'd' } , //"[DEV_SEL] 输入设备选择" ,
    { "cry_mode"    , required_argument , NULL , 'm' } , //"哭声检测选择 默认(npu)" ,
    { "vad_thresh"  , required_argument , NULL , 'n' } , //"侦测声音阈值，npu方式时输入只能配置为0" ,
    { "vad_level"  , required_argument , NULL , 'l' } , //"侦测声音等级" ,
    { "model_name"  , required_argument , NULL , 'e' } , //"npu方式时使用的模型参数文件名带路径" ,
    {0, 0, 0, 0}
};


/*
 * help_hint: use the -h --help option.Print option of help information
 * return: 0
 */
static int help_hint( char *pc_prog_name )
{
    int i;
    printf( "version: \r\n  %s \n\n", ak_ai_get_version());
    printf( "usage: \r\n  %s  [options] <value>\n\n", pc_prog_name);
    printf( "options: \r\n");
    for(i = 0; i < sizeof(option_long) / sizeof(struct option) - 1; i++) {
        printf("  -%c,--%-16s\t%s \n" , option_long[i].val , option_long[i].name , ac_option_hint[i]);
    }
    printf( "\nexample: \r\n"
    "  %s -t 60 -s 16000 -c 1 -v 5 -d 0 -m 33 -p /mnt/ -e /tmp/nfs/model_aicrydet_V1.00.02.bin \n", pc_prog_name);
    printf("\n\n");
    return 0;
}

static char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short,
                        int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    for (i = 0; i < i_num_option; i++) {
        c_option = p_option[i].val;
        switch (p_option[i].has_arg) {
        case no_argument:
            i_offset += snprintf(pc_option_short + i_offset, i_len_option - i_offset, "%c", c_option);
            break;
        case required_argument:
            i_offset += snprintf( pc_option_short + i_offset, i_len_option - i_offset, "%c:", c_option);
            break;
        case optional_argument:
            i_offset += snprintf( pc_option_short + i_offset, i_len_option - i_offset, "%c::", c_option);
            break;
        }
    }
    return pc_option_short;
}

static int parse_option(int argc, char **argv)
{
    int i_option;
    char ac_option_short[LEN_OPTION_SHORT];
    int i_array_num = sizeof(option_long) / sizeof(struct option) ;
    char c_flag = 1;

    if (argc < 5 && argc != 1) {
        help_hint(argv[0]);
        c_flag = 0;
        goto parse_option_end;
    }

#ifdef AK_RTOS
    optind = 0;
#endif
    get_option_short(option_long, i_array_num , ac_option_short , LEN_OPTION_SHORT);
    while ((i_option = getopt_long(argc , argv , ac_option_short , option_long , NULL)) > 0) {
        switch (i_option) {
            case 'h' :                             //help
                help_hint(argv[0]);
                c_flag = 0;
                goto parse_option_end;
            case 't' :                           //second
                save_time = atoi(optarg) * 1000 ;
                break;
            case 's' :                          //sample-rate
                sample_rate = atoi(optarg);
                break;
            case 'c' :                          //channel number
                channel_num = atoi(optarg);
                break;
            case 'v' :                          //volume
                volume = atoi(optarg);
                break;
            case 'd':                           // choose device
                dev_sel = atoi(optarg);
                break;
            case 'p' :                          //path
                save_path = optarg;
                break;
            case 'm' :
                cry_mode = atoi(optarg);
                break;
            case 'n' :                          // voice detect thresh
                vad_thresh = atoi(optarg);
                break;
            case 'l' :                          // voice detect level
                vad_level = atoi(optarg);
                break;
            case 'e':                            // model_name and path
                model_name = optarg;
                break;

        }
    }
parse_option_end:
    return c_flag;
}

/*
 * open_pcm_file: open pcm file.
 * sample_rate[IN]: audio input sample rate.
 * channel_num[IN]: audio input channle number.
 * path[IN]: pointer to the path which will be checking.
 * fp[OUT]: pointer of opened pcm file.
 * save_time[IN]: audio input save time.
 * dev_sel[IN]: audio input device id, see enum ak_audio_ai_dev_id.
 * return: 0 is success, -1 is error.
 */
static int open_pcm_file(int sample_rate, int channel_num, const char *path, FILE **fp, int save_time, int dev_sel)
{
    struct stat s = {0};
    if (stat(path, &s) != 0) {
        printf("stat error. path:%s, errno:%d\n", path, errno);
        return -1;
    }

    if (S_ISDIR(s.st_mode) == 0) {
        printf("path is not dir. path:%s\n", path);
        return -1;
    }

    struct ak_date date;
    ak_get_localdate(&date);
    char time_str[20] = {0};
    ak_date_to_string(&date, time_str);
    char file_path[255] = {0};
    char dev[DEV_STR_LEN] = {0};

    if (DEV_ADC == dev_sel) {
        strncpy(dev, "ADC", DEV_STR_LEN);
    } else if (DEV_PDM == dev_sel) {
        strncpy(dev, "PDM", DEV_STR_LEN);
    } else if (DEV_I2S0_AI== dev_sel) {
        strncpy(dev, "I2S0", DEV_STR_LEN);
    } else if (DEV_I2S1_AI == dev_sel) {
        strncpy(dev, "I2S1", DEV_STR_LEN);
    }

    sprintf(file_path, "%s/%s_%d_%d_%d_%s.pcm", path, time_str, sample_rate, channel_num, save_time, dev);

    /* open file */
    *fp = fopen(file_path, "wb");
    if (NULL == *fp) {
        ak_print_normal(MODULE_ID_APP, "open pcm file: %s error\n", file_path);
        return -1;
    } else {
        ak_print_normal(MODULE_ID_APP, "open pcm file: %s OK\n", file_path);
        return 0;
    }
}

/*
 * close_pcm_file: close pcm file.
 * fp[IN]: pointer of opened pcm file.
 * return: void.
 */
static void close_pcm_file(FILE *fp)
{
    /* close file handle */
    if (NULL != fp) {
        fclose(fp);
        fp = NULL;
    }
}

/*
 * ai_capture_loop: loop to get and release pcm data, between get and release,
 *                  here we just save the frame to file, on your platform,
 *                  you can rewrite the save_function with your code.
 */
static void* ai_capture_loop(void *arg)
{
    ak_thread_set_name("[vad sample]attachment");
    unsigned long long start_ts = 0;// use to save capture start time
    unsigned long long end_ts = 0;    // the last frame time
    unsigned long long trigger_ts = 0;
    // unsigned long long temp_ts = 0;
    struct frame frame = {0};
    int ret = AK_FAILED;
    // int probe_count = 0;

    struct ak_audio_vad_status_ex vad_status = {0};
    vad_status.vadType = cry_mode;
    int sleep_time = frame_interval == 0 ? 16: (frame_interval / 2);
    struct ak_timeval time_start = {0};
    struct ak_timeval time_end = {0};
    long time = 0;
    ak_print_normal_ex(MODULE_ID_APP, "sleep_time=%d\n", sleep_time);


    ak_print_normal(MODULE_ID_APP, "*** detect start ***\n");

    ak_get_ostime(&time_start);
    while (1) {

        ak_ai_get_vad_status_ex(ai_handle_id, &vad_status);
        if (vad_status.ts != trigger_ts) {
            if(vad_status.trigger){
                ak_print_normal_ex(MODULE_ID_APP, "cry: trigger = %d, vadThresh =%ld @ %lldms\n",
                            vad_status.trigger, vad_status.vadThresh, vad_status.ts / 1000);
            } else {
                ak_print_normal_ex(MODULE_ID_APP, "cry: trigger = %d, vadThresh =%ld @ %lldms\n",
                            vad_status.trigger, vad_status.vadThresh, vad_status.ts / 1000);
            }
            trigger_ts = vad_status.ts;
        }

        //1秒钟取一次哭声检测结果
        ak_sleep_ms(1000);
        ak_get_ostime(&time_end);

        //获取总的运行时间。
        time = ak_diff_ms_time(&time_end, &time_start);

        if ((time) >= save_time) {
            ak_print_normal(MODULE_ID_APP, "*** timp is up, total save time = %dms ***\n", time);
            break;
        }

    }

    ak_print_normal_ex(MODULE_ID_APP, "*** capture finish ***\n\n");
    ak_thread_exit();
    return NULL;
}

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
    struct ak_audio_nr_attr     default_ai_nr_attr      = {-40, 0, 1};
    struct ak_audio_agc_attr    default_ai_agc_attr     = {24576, 4, 0, 80, 0, 1};
    struct ak_audio_aec_attr    default_ai_aec_attr     = {0, 1024, 1024, 0, 512, 1, 0};
    struct ak_audio_aslc_attr   default_ai_aslc_attr    = {32768, volume, 0};
    struct ak_audio_eq_attr     default_ai_eq_attr      = {
    0,
    10,
    {50, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {717, 717, 717, 717, 717, 717, 717, 717, 717, 717},
    {TYPE_HPF, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1},
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    struct ak_audio_nr_attr     default_ao_nr_attr      = {0, 0, 0};
    struct ak_audio_aslc_attr   default_ao_aslc_attr    = {32768, 0, 0};
    struct ak_audio_eq_attr     default_ao_eq_attr      = {
    0,
    10,
    {50, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {717, 717, 717, 717, 717, 717, 717, 717, 717, 717},
    {TYPE_HPF, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1},
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

/**
 * init_ai - open ai, set nr, set agc, set gain, set volume
 * notes:
 */
static int init_ai(void)
{
    struct ak_audio_in_param param;
    memset(&param, 0, sizeof(struct ak_audio_in_param));
    ak_print_notice_ex(MODULE_ID_APP, "capture sample_rate=%d\n", sample_rate);
    param.pcm_data_attr.sample_rate = sample_rate;              // set sample rate
    param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;    // sample bits only support 16 bit
    param.pcm_data_attr.channel_num = channel_num;              // channel number
    param.dev_id = dev_sel;                                     //DEV_ADC;

    int ret = 0;
    ret = ret;

    ret = ak_npu_init();
    if(ret != AK_SUCCESS)    {
        ak_print_error_ex(MODULE_ID_APP, "ak_npu_init failed\n");
        return AK_FAILED;
    }else{
        ak_print_notice_ex(MODULE_ID_APP, "Notice: ak_npu_init OK!\n");
    }

    if (ak_ai_open(&param, &ai_handle_id)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_open failed\n");
        return AK_FAILED;
    }

    /* set source, source include mic and linein */
    if (DEV_ADC == dev_sel) {
        if (ak_ai_set_source(ai_handle_id, AI_SOURCE_MIC)) {
            ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_source failed\n");
            return AK_FAILED;
        }

        /* enable_nr, nr only support 8000 or 16000 sample rate */
        struct ak_audio_nr_attr nr_attr;
        setup_default_audio_argument(&nr_attr, 1);
        if (sample_rate != AK_AUDIO_SAMPLE_RATE_8000 && sample_rate != AK_AUDIO_SAMPLE_RATE_16000) {
            ak_print_warning_ex(MODULE_ID_APP, "ak_ai_set_nr_attr only support sample rate 8000 or 16000\n");
        } else {

            if (ak_ai_set_nr_attr(ai_handle_id, &nr_attr)) {
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
            if (ak_ai_set_agc_attr(ai_handle_id, &agc_attr)) {
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
            if (ak_ai_set_aec_attr(ai_handle_id, &aec_attr)) {
                ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_aec_attr failed\n");
                return AK_FAILED;
            }
        }
 
    }

    if (DEV_PDM == dev_sel) {
        ak_print_warning_ex(MODULE_ID_APP, "ak_ai_set_gain no support PDM\n");
    } else {
        if (ak_ai_set_gain(ai_handle_id, 5)) {
            ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_gain failed\n");
            return AK_FAILED;
        }
    }

    struct ak_audio_aslc_attr ai_aslc_attr;
    setup_default_audio_argument(&ai_aslc_attr, 4);
    if (ak_ai_set_aslc_attr(ai_handle_id, &ai_aslc_attr)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_aslc_attr failed\n");
        return AK_FAILED;
    }

    struct ak_audio_eq_attr ai_eq_attr;
    setup_default_audio_argument(&ai_eq_attr, 7);
    if (ak_ai_set_eq_attr(ai_handle_id, &ai_eq_attr)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_eq_attr failed\n");
        return AK_FAILED;
    }

    int frame_len = 0;
    if (ak_ai_get_frame_length(ai_handle_id, &frame_len)) {
        ak_print_error_ex(MODULE_ID_APP, "ak_ai_set_frame_length failed\n");
        return AK_FAILED;
    }
    frame_interval = ak_audio_len_to_interval(&param.pcm_data_attr, frame_len);


#if DEBUG
    ak_ai_set_lib_print_level(ai_handle_id, 
                              AUDIO_PRINT_LEVEL_ERROR | AUDIO_PRINT_LEVEL_WARNING |
                              AUDIO_PRINT_LEVEL_INFO | AUDIO_PRINT_LEVEL_PARAM |
                              AUDIO_PRINT_LEVEL_OPEN | AUDIO_PRINT_LEVEL_CLOSE);
#endif
    return AK_SUCCESS;
} // end of init_ai

/**
 * destroy_ai - close ai
 * notes:
 */
static void destroy_ai(void)
{
    if (-1 != ai_handle_id) {
        /* ai close */
        ak_ai_close(ai_handle_id);
        ai_handle_id = -1;
    }
    ak_npu_deinit();
}

/**
 * set_vad_param - set vad parameter
 * notes:
 */
static int set_vad_param(void)
{
    struct ak_audio_vad_attr vad_attr = {0};
    struct ak_audio_vad_npu_attr vad_npu_attr = {0};

    vad_npu_attr.vad_attr.enable = 1;              // enable voice detect attribute
    vad_npu_attr.vad_attr.vad_level = vad_level;   // default use 0
    vad_npu_attr.vad_attr.vad_thresh = 0;          // default use 0
    vad_npu_attr.vad_attr.vad_type = cry_mode;
    vad_npu_attr.model_name = model_name;

    // call  ak_AIcry_register  before ak_ai_set_vad_npu_attr
    //ak_ai_set_lib_print_level(ai_handle_id, (AUDIO_PRINT_LEVEL_DEFAULT | AUDIO_PRINT_LEVEL_PARAM));
    ak_AIcry_register();
    return ak_ai_set_vad_npu_attr(ai_handle_id, &vad_npu_attr);
}

/**
 * close_vad_ai_cry - close vad ai cry
 * notes:
 */
static int close_vad_ai_cry(void)
{
    struct ak_audio_vad_attr vad_attr = {0};
    struct ak_audio_vad_npu_attr vad_npu_attr = {0};

    vad_npu_attr.vad_attr.enable = 0;              // disable voice detect attribute
    vad_npu_attr.vad_attr.vad_level = vad_level;   // default use 0
    vad_npu_attr.vad_attr.vad_thresh = 0;          // default use 0
    vad_npu_attr.vad_attr.vad_type = cry_mode;
    vad_npu_attr.model_name = model_name;

    return ak_ai_set_vad_npu_attr(ai_handle_id, &vad_npu_attr);
}

#ifdef AK_RTOS
static void param_init(void)
{
    fp = NULL;
    sample_rate = 8000;
    volume = 5;
    save_time = 20000;		 // set save time(ms)
    save_path= (char *)save_def_parth;    // set save path
    channel_num = AUDIO_CHANNEL_MONO;
    dev_sel = DEV_ADC;
}
#endif

/**
 * Preconditions:
 * 1. T card is already mounted
 * 2. mic or linein must ready
 */
#ifdef AK_RTOS
static int baby_screaming_sample(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    /* start the application */
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );
    int ret = AK_FALSE;
#ifdef AK_RTOS
    param_init();
#endif

    if (parse_option(argc, argv) == 0){
        goto exit;
    }
    ak_print_normal(MODULE_ID_APP, "*****************************************\n");
    ak_print_normal(MODULE_ID_APP, "** ai version: %s **\n", ak_ai_get_version());
    ak_print_normal(MODULE_ID_APP, "*****************************************\n");

    if (init_ai()) {
        ak_print_error_ex(MODULE_ID_APP, "init ai failed\n");
        goto ai_end;
    }

    if (set_vad_param()) {
        ak_print_error_ex(MODULE_ID_APP, "init vqe failed\n");
        goto ai_end;
    }

    if (0 > open_pcm_file(sample_rate, channel_num, save_path, &fp, save_time,dev_sel)) {
        goto ai_end;
    }


    ak_pthread_t capture_loop_tid;
    ret = ak_thread_create(&(capture_loop_tid), ai_capture_loop, NULL, ANYKA_THREAD_MIN_STACK_SIZE, 19);
    if (ret) {
        goto ai_end;
    }

    /* wait thread end */
    ak_thread_join(capture_loop_tid);

    close_vad_ai_cry();

ai_end:
    destroy_ai();
    close_pcm_file(fp);

exit:
    ak_print_normal(MODULE_ID_APP, "******** exit baby_screaming sample ********\n");
    ak_sdk_exit();
    return ret;
}

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(baby_screaming_sample, ak_baby_screaming_sample, voice active detection sample)
#endif
/* end of file */
