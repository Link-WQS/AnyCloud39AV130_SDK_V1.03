#ifndef _AK_SPS_H_
#define _AK_SPS_H_


/**
 * ak_ps_start: start photosensitive switch
 * @dev: dev id
 * @ps_mode: photosensitive mode
 * @day_night_mode: day night switch mode
 * @cfg_path[IN]:  ps ir config path
 * return: 0 success, -1 failed
 */
int ak_sps_start(int dev, char *cfg_path, int dev_cnt);

/**
 * ak_ps_stop - stop ircut auto switch
 * return: 0 success, -1 failed
 * notes:
 */
void ak_sps_stop(void);


#endif

/* end of file */
