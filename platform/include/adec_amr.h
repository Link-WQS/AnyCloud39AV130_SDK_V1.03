#ifndef _ADEC_AMR_H_
#define _ADEC_AMR_H_

#include "ak_types.h"

// get version string
const AK_CHAR *ADEC_Amr_GetVersionInfo(AK_VOID);

// followings are to be set into ADEC_DECODE_S

// enType shall set to AK_AUDIO_TYPE_AMR

AK_VOID *ADEC_OpenAmr(AK_VOID *open_arg, AK_VOID *pstAttr);

#endif //_ADEC_AMR_H_

