#ifndef _AENC_AMR_H_
#define _AENC_AMR_H_

#include "ak_types.h"

// get version string
const AK_CHAR *AENC_Amr_GetVersionInfo(AK_VOID);

// followings are to be set into AENC_ENCODE_S

// enType shall set to AK_AUDIO_TYPE_AMR

AK_VOID *AENC_OpenAmr(AK_VOID *open_arg, AK_VOID *pstAttr);

#endif //_AENC_AMR_H_

