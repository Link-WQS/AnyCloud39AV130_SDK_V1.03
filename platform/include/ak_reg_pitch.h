#ifndef _AK_REG_PITCH_H_
#define _AK_REG_PITCH_H_

/**
 * ak_pitch_get_version -  get pitch version
 * return: version string
 * notes:
 */
const char *ak_pitch_get_version(void);

/**
 * ak_pitch_register -  register pitch
 * return: 0 success, other failed
 * notes:
 */
int ak_pitch_register(void);


#endif

