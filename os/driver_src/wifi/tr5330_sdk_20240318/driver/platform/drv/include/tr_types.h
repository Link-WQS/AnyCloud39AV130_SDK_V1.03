/*
 * Copyright (c) CompanyNameMagicTag 2019-2023. All rights reserved.
 * Description: types header.
 * This file should be changed only infrequently and with great care.
 */

#ifndef __TR_TYPES_H__
#define __TR_TYPES_H__

#include "tr_errno.h"

#if (defined(KERNEL_WITH_OS_ADAPTATION_LAYER) || defined(CONFIG_DSP_CORE) || defined(CONFIG_CGRA_CORE))
#include "stdint.h"
#endif

#define T_DESC(_desc_, _y_) (_y_)

/* 基本数据类型定义 */
typedef unsigned char           tr_uchar;
typedef unsigned char           tr_u8;
typedef unsigned short          tr_u16;
typedef unsigned int            tr_u32;
typedef unsigned long long      tr_u64;
typedef unsigned long           tr_ulong;
typedef char                    tr_char;
typedef signed char             tr_s8;
typedef short                   tr_s16;
typedef int                     tr_s32;
typedef long long               tr_s64;
typedef long                    tr_slong;
typedef float                   tr_float;
typedef double                  tr_double;
typedef unsigned long           tr_size_t;
typedef unsigned long           tr_length_t;
typedef tr_u32                  tr_handle;
typedef tr_u8                   tr_bool;
#if ((!defined(KERNEL_WITH_OS_ADAPTATION_LAYER)) && (!defined(CONFIG_DSP_CORE)) && (!defined(CONFIG_CGRA_CORE)))
typedef unsigned int            uintptr_t;
typedef unsigned long long      uint64_t;
#endif
typedef void                    tr_void;
typedef void*                   tr_pvoid;

typedef tr_u8                   tr_byte;
typedef tr_byte*                tr_pbyte;
#if (!defined(CONFIG_CGRA_CORE))
typedef tr_u32                  size_t;
#endif

typedef unsigned int            tr_uintptr_t;
typedef unsigned int            tr_phys_addr_t;
typedef unsigned int            tr_virt_addr_t;
typedef volatile tr_u32 tr_u32_reg;

#undef ERROR
#define ERROR (-1)

/* defines */
#ifndef NULL
#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void*)0)
#endif
#endif

#define TR_CONST    const
#define TR_REG      register

#define TR_U64_MAX  0xFFFFFFFFFFFFFFFFUL
#define TR_U32_MAX  0xFFFFFFFF
#define TR_S32_MAX  0x7FFFFFFF
#define TR_U16_MAX  0xFFFF
#define TR_S16_MAX  0x7FFF
#define TR_U8_MAX   0xFF
#define TR_S8_MAX   0x7F
#define TR_U4_MAX   0x0f

#define TR_S32_MIN  (-0x80000000)
#define TR_S16_MIN  (-0x8000)
#define TR_S8_MIN   (-0x80)

#define TR_U32_BITS         32
#define TR_S32_BITS         32
#define TR_U24_BITS         24
#define TR_U16_BITS         16
#define TR_U8_BITS          8
#define TR_U4_BITS          4

#define TR_U8_BIT_INDEX_MAX 7

#define TR_PUBLIC    extern

#if defined(__BUILD_IN_ROM__) || defined(CONFIG_FEATURE_UT)
#define TR_PRV
#define TR_PRVL TR_PRV TR_INLINE
#else
#define TR_PRV static
#define TR_PRVL
#endif

#define TR_INLINE inline
#define TR_API
#define TR_EXTERN extern
#ifdef __cplusplus
# define TR_CPP_START    extern "C" {
# define TR_CPP_END      }
#else
# define TR_CPP_START
# define TR_CPP_END
#endif

#ifdef NDEBUG
#define TR_ASSERT(ignore)   ((void)0)
#else
#define TR_ASSERT(x)        ((void)0)
#endif

#define TR_START_HEADER    TR_CPP_START
#define TR_END_HEADER      TR_CPP_END

#undef TR_OUT
#undef TR_IN
#undef TR_INOUT
#define TR_OUT
#define TR_IN
#define TR_INOUT

#define TR_FALSE 0
#define TR_TRUE  1


#ifdef __cplusplus
#define TR_NULL       0
#else
#define TR_NULL    ((void *)0)
#endif

#ifndef CACHE_ALIGNED_SIZE
#define CACHE_ALIGNED_SIZE        32
#endif

#define TR_ALWAYS_INLINE __attribute__((always_inline)) inline
#define TR_ALWAYS_STAIC_INLINE __attribute__((always_inline)) static inline

#define tr_array_size(_array)  (sizeof(_array) / sizeof((_array)[0]))

#define tr_unused(var) \
    do { \
        (void)(var); \
    } while (0)

#define tr_align_4(x)        ((unsigned int)((x) + 0x3) & (~0x3)) /* 构造4字节地址对齐 */
#define tr_is_align_u32(x)   (!((x) & 3))                         /* 判断是否为4字节对齐 */
#define tr_is_unalign_u32(x) ((x) & 3)                            /* 判断是否为4字节对齐 */

#if defined(HAVE_PCLINT_CHECK)
#define tr_fieldoffset(s, m) (0) /* 结构成员偏移 */
#else
#define tr_fieldoffset(s, m) ((tr_u32) & (((s*)0)->m)) /* 结构成员偏移 */
#endif

#define TR_CHAR_CR             '\r' /* 0x0D */
#define TR_CHAR_LF             '\n' /* 0x0A */
#define tr_tolower(x)          ((x) | 0x20)  /* Works only for digits and letters, but small and fast */

#define tr_makeu16(a, b)       ((tr_u16)(((tr_u8)(a)) | ((tr_u16)((tr_u8)(b))) << 8))
#define tr_makeu32(a, b)       ((tr_u32)(((tr_u16)(a)) | ((tr_u32)((tr_u16)(b))) << 16))
#define tr_makeu64(a, b)       ((tr_u64)(((tr_u32)(a)) | ((tr_u64)((tr_u32)(b))) <<32))
#define tr_joinu32(a, b, c, d) ((a) | ((tr_u32)(b) << 8) | ((tr_u32)(c) << 16) | ((tr_u32)(d) << 24))

#define tr_hiu32(l)            ((tr_u32)(((tr_u64)(l) >> 32) & 0xFFFFFFFF))
#define tr_lou32(l)            ((tr_u32)(l))

#define tr_hiu16(l)            ((tr_u16)(((tr_u32)(l) >> 16) & 0xFFFF))
#define tr_lou16(l)            ((tr_u16)(l))
#define tr_hiu8(l)             ((tr_u8)(((tr_u16)(l) >> 8) & 0xFF))
#define tr_lou8(l)             ((tr_u8)(l))

#define tr_max(a, b)           (((a) > (b)) ? (a) : (b))
#define tr_min(a, b)           (((a) < (b)) ? (a) : (b))
#define tr_sub(a, b)           (((a) > (b)) ? ((a) - (b)) : 0)
#define tr_abs(a)              (((a) > 0) ? (a) : (- (a)))
#define tr_abs_sub(a, b)       (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#define tr_byte_align(value, align)            (((value) + (align) - 1) & (~((align) -1)))
#define tr_is_byte_align(value, align)         (((tr_u32)(value) & ((align) - 1))== 0)

#define tr_set_bit_i(val, n)                          ((val) |= (1 << (n)))
#define tr_clr_bit_i(val, n)                          ((val) &= ~(1 << (n)))
#define tr_is_bit_set_i(val, n)                       ((val) & (1 << (n)))
#define tr_is_bit_clr_i(val, n)                       (~((val) & (1 << (n))))
#define tr_switch_bit_i(val, n)                       ((val) ^= (1 << (n)))
#define tr_get_bit_i(val, n)                          (((val) >> (n)) & 1)

#define tr_u8_bit_val(b7, b6, b5, b4, b3, b2, b1, b0) \
    (((b7) << 7) | ((b6) << 6) | ((b5) << 5) | ((b4) << 4) | ((b3) << 3) | ((b2) << 2) | ((b1) << 1) | ((b0) << 0))

#define tr_u16_bit_val(b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0) \
    (tr_u16)(((b12) << 12) | ((b11) << 11) | ((b10) << 10) | ((b9) << 9) | ((b8) << 8) | ((b7) << 7) | \
    ((b6) << 6) | ((b5) << 5) | ((b4) << 4) | ((b3) << 3) | ((b2) << 2) | ((b1) << 1) | ((b0) << 0))

#define tr_set_u32_ptr_val(ptr, offset, val)  (*((tr_u32*)(((tr_u8*)(ptr)) + (offset))) = (val))
#define tr_get_u32_ptr_val(ptr, offset)      *((tr_u32*)(((tr_u8*)(ptr)) + (offset)))

#ifndef bit
#define bit(x)                         (1UL << (x))
#endif
#ifndef BIT0
#define BIT31                          ((tr_u32)(1UL << 31))
#define BIT30                          ((tr_u32)(1 << 30))
#define BIT29                          ((tr_u32)(1 << 29))
#define BIT28                          ((tr_u32)(1 << 28))
#define BIT27                          ((tr_u32)(1 << 27))
#define BIT26                          ((tr_u32)(1 << 26))
#define BIT25                          ((tr_u32)(1 << 25))
#define BIT24                          ((tr_u32)(1 << 24))
#define BIT23                          ((tr_u32)(1 << 23))
#define BIT22                          ((tr_u32)(1 << 22))
#define BIT21                          ((tr_u32)(1 << 21))
#define BIT20                          ((tr_u32)(1 << 20))
#define BIT19                          ((tr_u32)(1 << 19))
#define BIT18                          ((tr_u32)(1 << 18))
#define BIT17                          ((tr_u32)(1 << 17))
#define BIT16                          ((tr_u32)(1 << 16))
#define BIT15                          ((tr_u32)(1 << 15))
#define BIT14                          ((tr_u32)(1 << 14))
#define BIT13                          ((tr_u32)(1 << 13))
#define BIT12                          ((tr_u32)(1 << 12))
#define BIT11                          ((tr_u32)(1 << 11))
#define BIT10                          ((tr_u32)(1 << 10))
#define BIT9                           ((tr_u32)(1 << 9))
#define BIT8                           ((tr_u32)(1 << 8))
#define BIT7                           ((tr_u32)(1 << 7))
#define BIT6                           ((tr_u32)(1 << 6))
#define BIT5                           ((tr_u32)(1 << 5))
#define BIT4                           ((tr_u32)(1 << 4))
#define BIT3                           ((tr_u32)(1 << 3))
#define BIT2                           ((tr_u32)(1 << 2))
#define BIT1                           ((tr_u32)(1 << 1))
#define BIT0                           ((tr_u32)(1 << 0))
#endif

#define HALFWORD_BIT_WIDTH              16

#define BYTE_WIDTH                  1
#define HALF_WIDTH                  2
#define WORD_WIDTH                  4

#define tr_reg_write(addr, val)              (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#define tr_reg_read(addr, val)               ((val) = *(volatile unsigned int *)(uintptr_t)(addr))
#define tr_reg_write32(addr, val)            (*(volatile unsigned int *)(uintptr_t)(addr) = (val))
#define tr_reg_read32(addr, val)             ((val) = *(volatile unsigned int *)(uintptr_t)(addr))
#define tr_reg_read_val32(addr)              (*(volatile unsigned int*)(uintptr_t)(addr))
#define tr_reg_setbitmsk(addr, msk)          ((tr_reg_read_val32(addr)) |= (msk))
#define tr_reg_clrbitmsk(addr, msk)          ((tr_reg_read_val32(addr)) &= ~(msk))
#define tr_reg_clrbit(addr, pos)             ((tr_reg_read_val32(addr)) &= ~((unsigned int)(1) << (pos)))
#define tr_reg_setbit(addr, pos)             ((tr_reg_read_val32(addr)) |= ((unsigned int)(1) << (pos)))
#define tr_reg_clrbits(addr, pos, bits)      (tr_reg_read_val32(addr) &= ~((((unsigned int)1 << (bits)) - 1) << (pos)))
#define tr_reg_setbits(addr, pos, bits, val) (tr_reg_read_val32(addr) =           \
    (tr_reg_read_val32(addr) & (~((((unsigned int)1 << (bits)) - 1) << (pos)))) | \
    ((unsigned int)((val) & (((unsigned int)1 << (bits)) - 1)) << (pos)))
#define tr_reg_getbits(addr, pos, bits)      ((tr_reg_read_val32(addr) >> (pos)) & (((unsigned int)1 << (bits)) - 1))

#define tr_reg_write16(addr, val)      (*(volatile unsigned short *)(uintptr_t)(addr) = (val))
#define tr_reg_read16(addr, val)       ((val) = *(volatile unsigned short *)(uintptr_t)(addr))
#define tr_reg_read_val16(addr)        (*(volatile unsigned short*)(uintptr_t)(addr))
#define tr_reg_clrbit16(addr, pos)       ((tr_reg_read_val16(addr)) &= ~((unsigned short)(1) << (pos)))
#define tr_reg_setbit16(addr, pos)       ((tr_reg_read_val16(addr)) |= ((unsigned short)(1) << (pos)))
#define tr_reg_clrbits16(addr, pos, bits) (tr_reg_read_val16(addr) &= ~((((unsigned short)1 << (bits)) - 1) << (pos)))
#define tr_reg_setbits16(addr, pos, bits, val) (tr_reg_read_val16(addr) =           \
    (tr_reg_read_val16(addr) & (~((((unsigned short)1 << (bits)) - 1) << (pos)))) | \
    ((unsigned short)((val) & (((unsigned short)1 << (bits)) - 1)) << (pos)))
#define tr_reg_getbits16(addr, pos, bits) ((tr_reg_read_val16(addr) >> (pos)) & (((unsigned short)1 << (bits)) - 1))

#define tr_reg_write8(addr, val)      (*(volatile unsigned char *)(uintptr_t)(addr) = (val))
#define tr_reg_read8(addr, val)       ((val) = *(volatile unsigned char *)(uintptr_t)(addr))
#define tr_reg_read_val8(addr)        (*(volatile unsigned char*)(uintptr_t)(addr))
#define tr_reg_clrbit8(addr, pos)       ((tr_reg_read_val8(addr)) &= ~((unsigned char)(1) << (pos)))
#define tr_reg_setbit8(addr, pos)       ((tr_reg_read_val8(addr)) |= ((unsigned char)(1) << (pos)))
#define tr_reg_clrbits8(addr, pos, bits) (tr_reg_read_val8(addr) &= ~((((unsigned char)1 << (bits)) - 1) << (pos)))
#define tr_reg_setbits8(addr, pos, bits, val) (tr_reg_read_val8(addr) =           \
    (tr_reg_read_val8(addr) & (~((((unsigned char)1 << (bits)) - 1) << (pos)))) | \
    ((unsigned char)((val) & (((unsigned char)1 << (bits)) - 1)) << (pos)))
#define tr_reg_getbits8(addr, pos, bits) ((tr_reg_read_val8(addr) >> (pos)) & (((unsigned char)1 << (bits)) - 1))

#ifndef align_next
#define align_next(val, a)          ((((val) + ((a)-1)) & (~((a)-1))))
#define align_length(val, a)        align_next(val, a)
#endif

#define BITS_PER_BYTE               8
#define HEXADECIMAL                 16
#define DECIMAL                     10

#define SZ_1KB 1024
#define SZ_1MB (SZ_1KB * SZ_1KB)
#define SZ_4KB 4096
#define TR_SYS_WAIT_FOREVER           0xFFFFFFFF

typedef tr_void  (*tr_void_callback)(tr_void);
typedef unsigned long long      tr_mem_size_t;
typedef long long               tr_mem_handle_t;
typedef struct {
    tr_mem_handle_t mem_handle;
    tr_mem_size_t addr_offset;
} tr_mem_handle;

typedef struct {
    tr_s32 x;
    tr_s32 y;
    tr_u32 width;
    tr_u32 height;
} tr_rect;

#ifdef HAVE_PCLINT_CHECK
#define tr_likely(x)    (x)
#define tr_unlikely(x)  (x)
#else
#define tr_likely(x) __builtin_expect(!!(x), 1)
#define tr_unlikely(x) __builtin_expect(!!(x), 0)
#endif

#endif /* __TR_TYPES_H__ */

