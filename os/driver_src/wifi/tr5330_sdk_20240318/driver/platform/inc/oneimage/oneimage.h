/*
 * Copyright (c) CompanyNameMagicTag 2020-2023. All rights reserved.
 * Description: platform oneimage file
 * Create: 2020-10-13
 */

#ifndef __ONEIMAGE_H__
#define __ONEIMAGE_H__

#include "platform/platform_linux_osal_oneimage.h"
#include "platform/platform_security_oneimage.h"
#include "platform/platform_linux_oal_oneimage.h"
#include "platform/platform_hcc_oneimage.h"

#ifdef WSCFG_ONEIMAGE
#define platform_oneimage_rename(NAME)  NAME##_tr5330
#endif

#endif /* #ifndef __ONEIMAGE_H__ */