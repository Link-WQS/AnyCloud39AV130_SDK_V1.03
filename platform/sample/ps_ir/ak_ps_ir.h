#ifndef _AK_PS_H_
#define _AK_PS_H_

enum ak_photosensitive_mode {
    HARDWARE_PHOTOSENSITIVE,	// hardware photosensitive
    AUTO_PHOTOSENSITIVE		// auto ir switch
};

enum day_night_switch_mode {
    SET_DAY_MODE,
    SET_NIGHT_MODE,
    SET_AUTO_MODE,
    SET_CLOSE_MODE
};

const char* ak_ps_get_version(void);


/**
 * ak_ps_start: start photosensitive switch
 * @dev: dev id
 * @ps_mode: photosensitive mode
 * @day_night_mode: day night switch mode
 * @cfg_path[IN]:  ps ir config path
 * return: 0 success, -1 failed
 */
int ak_ps_start(int dev, enum ak_photosensitive_mode ps_mode,
                            enum day_night_switch_mode day_night_mode,
                            char *cfg_path);

/**
 * ak_ps_stop - stop ircut auto switch
 * return: 0 success, -1 failed
 * notes:
 */
void ak_ps_stop(void);

/**
 * ak_ps_set_ain_th: set ain threshold
 * @th_high: threshold high level
 * @th_low: threshold low level
 * return: 0 success, -1 failed
 */
 int ak_ps_set_ain_th(int th_high, int th_low);


#endif

/* end of file */
