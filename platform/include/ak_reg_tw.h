#ifndef _AK_REG_TW_H_
#define _AK_REG_TW_H_

// tonal waves: detection

/**
 * ak_tw_get_version -  get tw version
 * return: version string
 * notes:
 */
const char *ak_tw_get_version(void);


/**
 * ak_tw_register -  register tw
 * return: 0 success, other failed
 * notes:
 */
int ak_tw_register(void);


#endif
