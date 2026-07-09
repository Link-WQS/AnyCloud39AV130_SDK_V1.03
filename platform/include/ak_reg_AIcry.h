#ifndef _AK_REG_AICRY_H_
#define _AK_REG_AICRY_H_

/**
 * ak_AIcry_get_version -  get AIcry version
 * return: version string
 * notes:
 */
const char *ak_AIcry_get_version(void);


/**
 * ak_AIcry_register -  register AIcry
 * return: 0 success, other failed
 * notes:
 */
int ak_AIcry_register(void);


#endif

