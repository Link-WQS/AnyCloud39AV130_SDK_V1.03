#ifndef _ADEC_AAC_H_
#define _ADEC_AAC_H_

#include "ak_types.h"

// get version string
const AK_CHAR *ADEC_Aac_GetVersionInfo(AK_VOID);

// followings are to be set into ADEC_DECODE_S

// enType shall set to AK_AUDIO_TYPE_AAC

AK_VOID *ADEC_OpenAac(AK_VOID *open_arg, AK_VOID *pstAttr);

#endif //_ADEC_AAC_H_

