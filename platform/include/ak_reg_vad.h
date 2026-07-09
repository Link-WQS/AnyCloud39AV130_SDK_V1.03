#ifndef _AK_REG_VAD_H_
#define _AK_REG_VAD_H_

/**
 * ak_vad_get_version -  get vad version
 * return: version string
 * notes:
 */
const char *ak_vad_get_version(void);


/**
 * ak_vad_register -  register vad
 * return: 0 success, other failed
 * notes:
 */
int ak_vad_register(void);


#endif

