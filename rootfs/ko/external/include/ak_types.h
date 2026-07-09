/** @file
* @brief Define the global public types for anyka
*
* Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
* @author
* @date 2006-01-16
* @version 1.0
*/

#ifndef __AK_TYPES_H__
#define __AK_TYPES_H__

//#include <stdint.h> // for intN_t, uintN_t
//#include <stddef.h> // for size_t
#include <linux/types.h>

#if 1
#define __raw_writeq(value, addr)  ((*(volatile unsigned long long *)(addr))= value)
#define __raw_readq(addr)   (*(volatile unsigned long long *)(addr))

#else
#define __raw_writeq(value, addr)\
({\
    unsigned long high_bit = (unsigned long)((unsigned long long)value >> 32);\
    unsigned long low_bit = (unsigned long)((unsigned long long)value & 0xFFFFFFFF);\
    __raw_writel(low_bit, addr);\
    __raw_writel(high_bit, addr + 4);\
})

#define __raw_readq(addr) \
    ({ \
     (((unsigned long long)__raw_readl(addr)) | ((unsigned long long)__raw_readl(addr + 4)) << 32 ) ; \
     })

#endif



/** @defgroup GLOBALTYPES global types
*    @ingroup GLOBAL
*/
/*@{*/

/* preliminary type definition for global area */
typedef    uint8_t                  AK_U8;          /* unsigned 8 bit integer */
typedef    char                     AK_CHAR;        /* char */
typedef    uint16_t                 AK_U16;         /* unsigned 16 bit integer */
typedef    uint32_t                 AK_U32;         /* unsigned 32 bit integer */
typedef    double                   AK_DOUBLE;
#ifdef _MSC_VER
typedef    unsigned __int64         AK_U64;         /* unsigned 64 bit integer */
#else
typedef    uint64_t                 AK_U64;         /* unsigned 64 bit integer */
#endif
typedef    int8_t                   AK_S8;          /* signed 8 bit integer */
typedef    int16_t                  AK_S16;         /* signed 16 bit integer */
typedef    int32_t                  AK_S32;         /* signed 32 bit integer */
#ifdef _MSC_VER
typedef    __int64                  AK_S64;         /* signed 64 bit integer */
#else
typedef    int64_t                  AK_S64;         /* signed 64 bit integer */
#endif
typedef    void                     AK_VOID;        /* void */
typedef    volatile uint32_t        AK_UINT32;

// maximum AK_U8 value
#define    AK_U8_MAX             ((AK_U8)0xff)
// maximum AK_U16 value
#define    AK_U16_MAX            ((AK_U16)0xffff)
 // maximum AK_U32 value
#define    AK_U32_MAX            ((AK_U32)0xffffffff)
// maximum AK_U64 value
#define    AK_U64_MAX            ((AK_U64)0xffffffffffffffff)
// minimum AK_S8 value
#define    AK_S8_MIN             ((AK_S8)(-127-1))
// maximum AK_S8 value
#define    AK_S8_MAX             ((AK_S8)127)
 // minimum AK_S16 value
#define    AK_S16_MIN            ((AK_S16)(-32767L-1L))
// maximum AK_S16 value
#define    AK_S16_MAX            ((AK_S16)(32767L))
 // minimum AK_S32 value
#define    AK_S32_MIN            ((AK_S32)(-2147483647L-1L))
// maximum AK_S32 value
#define    AK_S32_MAX            ((AK_S32)(2147483647L))
 // minimum AK_S64 value
#define    AK_S64_MIN            ((AK_S64)(-9223372036854775807LL-1LL))
 // maximum AK_S64 value
#define    AK_S64_MAX            ((AK_S64)(9223372036854775807LL))

//----------------------
typedef  unsigned char          BYTE;
typedef  signed char            SBYTE;
typedef  unsigned short int     WORD;
typedef  signed short int       SWORD;
typedef  unsigned int       DWORD;
typedef  signed int     SDWORD;
//----------------------
typedef AK_S8                   AK_CHR;     /* char */
typedef void *                  AK_pVOID;   /* pointer of void data */
typedef const void *            AK_pCVOID;  /* const pointer of void data */
typedef AK_S8 *                 AK_pSTR;        /* pointer of string */
typedef const AK_S8 *           AK_pCSTR;   /* const pointer of string */
typedef AK_U16                  AK_WCHR;        /**< unicode char */
typedef AK_U16 *                AK_pWSTR;   /* pointer of unicode string */
typedef const AK_U16 *          AK_pCWSTR;  /* const pointer of unicode string */
//--------------------------------
typedef AK_U8 *                 AK_pDATA;   /* pointer of data */
typedef const AK_U8 *           AK_pCDATA;  /* const pointer of data */
typedef AK_U32                  AK_COLOR;       /* color value */
typedef AK_U32                  AK_HANDLE;          /* a handle */
//--------------------------------
#define AK_U8_MAX            ((AK_U8)0xff)                    // maximum AK_U8 value
#define AK_U16_MAX           ((AK_U16)0xffff)                    // maximum AK_U16 value
#define AK_U32_MAX           ((AK_U32)0xffffffff)                // maximum AK_U32 value
#define AK_S8_MIN            ((AK_S8)(-127-1))                // minimum AK_S8 value
#define AK_S8_MAX            ((AK_S8)127)                        // maximum AK_S8 value
#define AK_S16_MIN           ((AK_S16)(-32767L-1L))        // minimum AK_S16 value
#define AK_S16_MAX           ((AK_S16)(32767L))            // maximum AK_S16 value
#define AK_S32_MIN           ((AK_S32)(-2147483647L-1L))    // minimum AK_S32 value
#define AK_S32_MAX           ((AK_S32)(2147483647L))        // maximum AK_S32 value
//----------------

#define AK_VOID                  void

typedef    uint8_t              AK_BOOL;
#define AK_FALSE                (0)
#define AK_TRUE                 (1)
#define AK_NULL			((AK_pVOID)(0))

#define AK_EMPTY
typedef		void (*AK_pF_VOID)(void);

/*@}*/

#endif
