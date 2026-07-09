#ifndef _AENC_AAC_H_
#define _AENC_AAC_H_

#include "ak_types.h"

// get version string
const AK_CHAR *AENC_Aac_GetVersionInfo(AK_VOID);

// followings are to be set into AENC_ENCODE_S

// enType shall set to AK_AUDIO_TYPE_AAC

AK_VOID *AENC_OpenAac(AK_VOID *open_arg, AK_VOID *pstAttr);

#endif //_AENC_AAC_H_

