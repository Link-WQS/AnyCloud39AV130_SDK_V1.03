#ifndef _ADEC_MP3_H_
#define _ADEC_MP3_H_

#include "ak_types.h"

// get version string
const AK_CHAR *ADEC_Mp3_GetVersionInfo(AK_VOID);

// followings are to be set into ADEC_DECODE_S

// enType shall set to AK_AUDIO_TYPE_MP3

AK_VOID *ADEC_OpenMp3(AK_VOID *open_arg, AK_VOID *pstAttr);

#endif //_ADEC_MP3_H_

