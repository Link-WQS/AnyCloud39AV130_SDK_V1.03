/*
 * @file  ak_efuse.h
 * @brief efuse operation interface
 * Copyright (C) 2021 Anyka (Guangzhou) Software Technology Co., LTD
 * @author yangruibin
 * @date  2021-02-25
 * @version 1.0
 */

#ifndef __AK_EFUSE_H__
#define __AK_EFUSE_H__

/* use only in AK3918AV100 */
#ifndef __ENUM_SECURE_LEVEL__
#define __ENUM_SECURE_LEVEL__
typedef enum _secure_level
{
    LEVEL0_NO_SECURE = 0,           // 级别0:  无安全启动
    LEVEL1_RSA2048,                 // 级别1:  只做RSA2048验签
    LEVEL2_AES256,                  // 级别2:  只做AES256解密
    LEVEL3_RSA2048_AES256,          // 级别3:  RSA2048验签+ AES256解密
#if defined(CONFIG_MACH_AK3918AV100) || defined(CONFIG_MACH_AK3918AV100PLUS)
    LEVEL4_RSA2048_AES256_BONDING,  // 级别4:  RSA2048验签+ AES256解密+ 1对1绑定
#endif
#if defined(CONFIG_MACH_AK3918AV130)
    LEVEL4_SPECIAL_SECURE,            // 级别4:  特殊安全模式
#endif
} secure_level;
#endif

/********************** IOCTL *********************************************/
#define AK_EFUSE_IOC_MAGIC          'F'

#define DO_GLOBE_ID_GET_LEN         _IOR(AK_EFUSE_IOC_MAGIC, 0x101, int)
#define DO_GLOBE_ID_GET             _IOR(AK_EFUSE_IOC_MAGIC, 0x102, int)
#define DO_CUSTOMER_DATA_GET_LEN    _IOR(AK_EFUSE_IOC_MAGIC, 0x103, int)
#define DO_CUSTOMER_DATA_GET        _IOR(AK_EFUSE_IOC_MAGIC, 0x104, int)
#define DO_CUSTOMER_DATA_SET        _IOW(AK_EFUSE_IOC_MAGIC, 0x105, int)

/* use in not recommended! */
#define DO_CUSTOMER_ID_GET_LEN      DO_CUSTOMER_DATA_GET_LEN
#define DO_CUSTOMER_ID_GET          DO_CUSTOMER_DATA_GET
#define DO_CUSTOMER_ID_SET          DO_CUSTOMER_DATA_SET

/* use only in AK3918AV100 */   
#define DO_BURN_AES256_KEY          \
            _IOW(AK_EFUSE_IOC_MAGIC, 0x106, unsigned char)
#define DO_BURN_PUBLIC_SHA256       \
            _IOW(AK_EFUSE_IOC_MAGIC, 0x107, unsigned char)
#define DO_BURN_SECURE_LEVEL        \
            _IOW(AK_EFUSE_IOC_MAGIC, 0x108, secure_level)
#define DO_BURN_ONEBYONE_BOND       \
            _IO(AK_EFUSE_IOC_MAGIC, 0x109)
#define DO_WRITE_FINISH             \
            _IO(AK_EFUSE_IOC_MAGIC, 0x10A)
#define DO_READ_SECURE_LEVEL        \
            _IOR(AK_EFUSE_IOC_MAGIC, 0x10B, secure_level)
#define DO_READ_LOCK                \
                _IOR(AK_EFUSE_IOC_MAGIC, 0x10C, char)

#define DO_CUSTOMER_DATA_LOCK       _IOW(AK_EFUSE_IOC_MAGIC, 0x10D, char)
#define DO_READ_CUSTOMER_DATA_LOCK  _IOW(AK_EFUSE_IOC_MAGIC, 0x10E, char)

#define ERR_PARAM         1         // ioctl参数检查错误
#define ERR_NOLOCK        2         // GLOBE ID读取，没有锁定错误码
#define ERR_LOCKED        3         // Customer ID写入，已经锁定错误码
#define ERR_OTHER         4         // 其他错误，数据比对错误等

/* use only in AK3918AV100 */
#define ERR_BURN          5         // 烧写出错

#endif //__AK_EFUSE_H__
