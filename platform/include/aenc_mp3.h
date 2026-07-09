#ifndef _AENC_MP3_H_
#define _AENC_MP3_H_

#include "ak_types.h"

// get version string
const AK_CHAR *AENC_Mp3_GetVersionInfo(AK_VOID);

// followings are to be set into AENC_ENCODE_S

// enType shall set to AK_AUDIO_TYPE_MP3

AK_VOID *AENC_OpenMp3(AK_VOID *open_arg, AK_VOID *pstAttr);

#endif //_AENC_MP3_H_

