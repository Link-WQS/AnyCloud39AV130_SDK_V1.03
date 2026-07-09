#ifndef _AK_AO_H_
#define _AK_AO_H_

#include "ak_common_audio.h"
#include "ak_common.h"

#define DAC_DEV_MIN_GAIN        (0)       /* analog gain */
#define DAC_DEV_MAX_GAIN        (6)       /* analog gain */

enum ak_ao_error_type
{
    ERROR_AO_OPEN_LIB_ERROR = (MODULE_ID_AO << 24) + 0,
    ERROR_AO_INIT_LIB_ERROR,
    ERROR_AO_CLOSE_LIB_ERROR,
    ERROR_AO_DEV_ID_NOT_SUPPORT,
    ERROR_AO_CHN_ID_NOT_SUPPORT,
    ERROR_AO_DEV_INFO_NULL,
    ERROR_AO_CHN_INFO_NULL,
    ERROR_AO_DEV_ALREADY_OPEN,
    ERROR_AO_CHN_ALREADY_OPEN,
    ERROR_AO_DEV_HANDLE_NULL,
    ERROR_AO_DEV_INVALID_FD,
    ERROR_AO_DEV_PLAYING,
    ERROR_AO_SET_PARAM_IS_THE_SAME,
    ERROR_AO_FRAME_SIZE_TOO_LONG,
    ERROR_AO_SET_MUTE_ERROR,
    ERROR_AO_SET_DAC_GAIN_ERROR,
    ERROR_AO_NOTICE_END_ERROR,
    ERROR_AO_DRIVER_FAILED,
    ERROR_AO_EQ_ERROR,
    ERROR_AO_CLOSE_FILTER_LIB_ERROR,
    ERROR_AO_GAIN_ERROR,
    ERROR_AO_MUTE_IS_ON,
    ERROR_AO_VQE_ERROR,
    ERROR_AO_THREAD_CREATE_FAILED,
    ERROR_AO_PUB_ATTR_ERROR,
    ERROR_AO_DEV_CHN_NOT_MATCH,
    ERROR_AO_DEV_CLOSE_ERROR,
    ERROR_AO_CHN_RUNNING,
    ERROR_AO_CHN_HAS_BEEN_ENABLED,
    ERROR_AO_CONF_FILE_ERROR,
    ERROR_AO_OPEN_LOOPBACK,
    ERROR_AO_INIT_SEM,
    ERROR_AO_FILL_VQE_ERROR,
    ERROR_AO_CHN_NOT_ENABLE,
    ERROR_AO_STRING_TOO_BIG,
    ERROR_AO_DEV_NOT_OPEN,
    ERROR_AO_NOT_SET_PUBATTR
};

/* audio output param */
struct ak_audio_out_param
{
    struct ak_audio_data_attr pcm_data_attr;
    int dev_id; // see enum ak_audio_ao_dev_id
};

/**
 * ak_ao_get_version - get audio out version
 * return: version string
 * notes:
 */
const char* ak_ao_get_version(void);

/**
 * ak_ao_open - open audio out device
 * @param[IN]: open DA param, dev_id  see enum ak_audio_ao_dev_id
 * @ao_handle_id[OUT]: audio out opened handle id
 * return: 0 success, otherwise failed
 * notes: sample_bits set 16 bit
 */
int ak_ao_open(const struct ak_audio_out_param *param, int *ao_handle_id);

/**
 * ak_ao_get_handle_id - get ao handle id
 * @dev_id[IN]: audio out device id
 * @ao_handle_id[OUT]: audio out opened handle id
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_get_handle_id(int dev_id, int *ao_handle_id);

/**
 * ak_ao_send_frame - send frame to device and play
 * @ao_handle_id[IN]: audio out opened handle id
 * @send_pcm_data[IN]: audio pcm data
 * @send_pcm_len[IN]: audio pcm data len, unit: byte
 * @play_pcm_len[OUT]: send to play data len, unit: byte
 * return: 0 success, otherwise failed
 * notes: this function is block mode function
 */
int ak_ao_send_frame(int ao_handle_id, unsigned char *send_pcm_data, \
                     int send_pcm_len, int *play_pcm_len);

/**
 * ak_ao_set_gain - set ao dac gain
 * @ao_handle_id[IN]: audio out opened handle id
 * @gain[IN]: new dac gain, [0, 6]: 0-mute, 6-dac max volume
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_set_gain(int ao_handle_id, int gain);

/**
 * ak_ao_get_gain - get ao dac gain
 * @ao_handle_id[IN]: audio out opened handle id
 * @gain[OUT]: dac gain
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_get_gain(int ao_handle_id, int *gain);

/**
 * ak_ao_set_volume - set ao volume
 * @ao_handle_id[IN]: audio out opened handle id
 * @db[IN]: ao volume , -90~20db
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_set_volume(int ao_handle_id, int db);

/**
 * ak_ao_get_volume - get ao volume
 * @ao_handle_id[IN]: audio out opened handle id
 * @db[OUT]: ao volume
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_get_volume(int ao_handle_id, int *db);

/**
 * ak_ao_enable_eq - enable eq
 * @ao_handle_id[IN]: audio out opened handle id
 * @enable[IN]: 0 disable,1 enable
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_enable_eq(int ao_handle_id, int enable);

/**
 * ak_ao_set_eq_attr - set eq attribute
 * @ao_handle_id[IN]: audio out opened handle id
 * @eq_attr[IN]: eq attribute
 * return: 0 success,  otherwise failed
 * notes:
 */
int ak_ao_set_eq_attr(int ao_handle_id, struct ak_audio_eq_attr *eq_attr);

/**
 * ak_ao_get_eq_attr - get eq attribute
 * @ao_handle_id[IN]: audio out opened handle
 * @eq_attr[OUT]: eq attribute
 * return: 0 success  otherwise failed
 * notes:
 */
int ak_ao_get_eq_attr(int ao_handle_id, struct ak_audio_eq_attr *eq_attr);

/**
 * ak_ao_get_dev_buf_size - get audio output device size
 * @ao_handle_id[IN]: audio out opened handle id
 * @dev_buf_size[OUT]: drvice DMA buffer size, unit: byte
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_get_dev_buf_size(int ao_handle_id, int *dev_buf_size);

/**
 * ak_ao_print_runtime_status - print runtime debug information
 * @ao_handle_id[IN]: audio out opened handle id
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_print_runtime_status(int ao_handle_id);

/**
 * ak_ao_close - close audio output
 * @ao_handle_id[IN]: audio out opened handle id
 * return:0 success, otherwise failed
 * notes: for debug use
 */
int ak_ao_close(int ao_handle_id);

/**
 * ak_ao_save_file - save three file, one file is pcm input to ao, \
   two file is pcm output from ao, three is pcm after eq \
 * @ao_handle_id[IN]: audio in opened handle id
 * @save_file_info[IN]: save file infomation
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_save_file(int ao_handle_id, \
    struct ak_audio_save_file_info *save_file_info);

/**
 * ak_ao_set_aslc_attr - set aslc attribute
 * @ao_handle_id[IN]: audio out opened handle id
 * @aslc_attr[IN]: aslc attribute
 * return: 0 success, otherwise failed
 * notes: for debug use
 */
int ak_ao_set_aslc_attr(int ao_handle_id, struct ak_audio_aslc_attr *aslc_attr);

/**
 * ak_ao_get_aslc_attr - get aslc attribute
 * @ao_handle_id[IN]: audio out opened handle id
 * @aslc_attr[OUT]: aslc attribute
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_get_aslc_attr(int ao_handle_id, struct ak_audio_aslc_attr *aslc_attr);

/**
 * ak_ao_set_nr_attr - set nr attribute
 * @ao_handle_id[IN]: audio out opened handle id
 * @nr_attr[IN]: nr attribute
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_set_nr_attr(int ao_handle_id, struct ak_audio_nr_attr *nr_attr);

/**
 * ak_ao_get_nr_attr - get nr attribute
 * @ao_handle_id[IN]: audio out opened handle id
 * @nr_attr[OUT]: nr attribute
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_get_nr_attr(int ao_handle_id, struct ak_audio_nr_attr *nr_attr);

/**
 * ak_ao_get_filter_info - print audio filter version
 * return: 0 success, otherwise failed
 * notes:
 */
char *ak_ao_get_filter_info(void);

/**
 * ak_ao_enable_nr - enable nr
 * @ao_handle_id[IN]: audio out opened handle id
 * @enable[IN]: 1 enable,0 disable
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_enable_nr(int ao_handle_id, int enable);

/**
 * ak_ao_cancel - audio output cancel playing, use this function can stop playing immediately
 * @ao_handle_id[IN]: audio out opened handle id
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_cancel(int ao_handle_id);

/**
 * ak_ao_get_buf_status - audio output get device buffer status
 * @ao_handle_id[IN]: audio out opened handle id
 * @buf_status[OUT]: get device buffer total length and \
 * device buffer remain length
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_get_buf_status(int ao_handle_id, \
    struct ak_dev_buf_status *buf_status);

/**
 * ak_ao_enable_hs - enable howling suppress
 * @ao_handle_id[IN]: audio out opened handle id
 * @enable[IN]: 1 enable,0 disable
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_enable_hs(int ao_handle_id, int enable);

/**
 * ak_ao_get_nr_attr - get howling suppress attribute
 * @ao_handle_id[IN]: audio out opened handle id
 * @hs_attr[OUT]: howling suppress attribute
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_get_hs_attr(int ao_handle_id, struct ak_audio_hs_attr *hs_attr);

/**
 * ak_ao_reset_params - use to reset sample rate or channel numbe
 * @ao_handle_id[IN]: audio out opened handle id
 * @param[IN]: ao params, include sample rate, sample bit, if param = NULL,\
 * use previous setting
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_reset_params(int ao_handle_id, struct ak_audio_out_param *param);

/**
 * ak_ao_restart - audio output restart running, use after ak_ao_cancel, \
 * use to restart running
 * @ao_handle_id[IN]: audio out opened handle id
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_restart(int ao_handle_id);

/**
 * ak_ao_get_params - get ao parameter, include sample rate, sample bit, \
 * channel number
 * @ao_handle_id[IN]: opened audio output handle id
 * @pcm_attr[OUT]: store ao params delivery by ak_ao_open;
 * return: 0 success, otherwise failed
 * notes: call after ak_ao_open
 */
int ak_ao_get_params(int ao_handle_id, struct ak_audio_data_attr *pcm_attr);

/**
 * ak_ao_set_driver_buf_periods - set playback driver buf periods
 * @periods[IN]: set playback driver buf periods
 * return: 0 success, otherwise failed
 * notes: call before ak_ao_open
 */
int ak_ao_set_driver_buf_periods(int periods);

/**
 * ak_ao_get_driver_buf_periods - get playback driver buf periods
 * @periods[OUT]:get playback driver buf periods
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_get_driver_buf_periods(int *periods);

/**
 * ak_ao_set_thread_priority - set mpp_ao thread priority
 * @priority[IN]: set mpp_ao thread priority
 * return: 0 success, otherwise failed
 * notes: call before ak_ao_open
 */
int ak_ao_set_thread_priority(int priority);

/**
 * ak_ao_get_thread_priority - get mpp_ao thread priority
 * @priority[OUT]:get mpp_ao thread priority
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_get_thread_priority(int *priority);

/**
 * ak_ao_get_hs_attr - get howling suppress attribute
 * @ao_handle_id[IN]: audio out opened handle id
 * @hs_attr[IN]: howling suppress attribute
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_set_hs_attr(int ao_handle_id, struct ak_audio_hs_attr *hs_attr);

 /**
  * ak_ao_flush - push the remaining data out
  * @ao_handle_id[IN]: opened audio output handle id
  * return: 0 success, otherwise failed
  * notes:
  */
int ak_ao_flush(int ao_handle_id);

/**
 * ak_ao_set_lib_print_level - set filter lib print level
 * @print_level[IN]: print level, enum ak_atclib_print_level, \
 * bit-or of AUDIO_PRINT_LEVEL_XXX. Default is AUDIO_PRINT_LEVEL_ERROR | \
 * AUDIO_PRINT_LEVEL_WARNING
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_set_lib_print_level(int print_level);

/**
 * ak_ao_load_ch_conf  : load ao channel config (vqe etc)
 * @ao_handle_id[IN]: opened audio output handle id
 * @pchConfFile[IN]       :  config file
 * @pchSection[IN]     :  which section
 * return 0 success otherwise failed
 */
int ak_ao_load_ch_conf(int ao_handle_id, char *pch_conf_file, char *pch_section);

/**
 * ak_ao_load_dev_conf  : load ao device config (vqe etc)
 * @dev_id[IN]: device id
 * @pchConfFile[IN]       :  config file
 * @pchSection[IN]     :  which section
 * return 0 success otherwise failed
 */
int ak_ao_load_dev_conf(int dev_id, char *pch_conf_file, char *pch_section);
#endif
/* end of file */

