#ifndef _AK_REG_RESAMPLE_H_
#define _AK_REG_RESAMPLE_H_

/**
 * ak_resample_get_version -  get resample version
 * return: version string
 * notes:
 */
const char *ak_resample_get_version(void);


/**
 * ak_resample_register -  register resample
 * return: 0 success, other failed
 * notes:
 */
int ak_resample_register(void);


#endif

