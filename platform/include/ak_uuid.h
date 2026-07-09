#ifndef _AK_UUID_H_
#define _AK_UUID_H_

#include "ak_common.h"

enum ak_uuid_error_type
{
    //参数检查错误
    ERROR_UUID_INVALID_ARG  = (MODULE_ID_UUID << 24) + 0,
    //GLOBE ID为空，不可读取
    ERROR_UUID_GLOBAL_ID_ZERO,
    //CUSTOM ID 已经有数据，不可以再写
    ERROR_UUID_UNWRITABLE,
    //其他错误，数据比对错误等
    ERROR_UUID_OTHERS ,
    //uuid设备没有打开
    ERROR_UUID_DEV_NOT_OPENED,
    //打开uuid失败,不存在该设备
    ERROR_UUID_DEV_NOT_EXISTED,
    //设备已打开
    ERROR_UUID_DEV_ALREADY_OPENED,
    //未知错误
    ERROR_UUID_UNKNOW
};

/**
 * ak_uuid_open - open uuid
 * return: return 0 on success  or -1 on failed
 * notes:
 */
int ak_uuid_open(void);

/**
 * ak_uuid_close - close uuid
 * return: return 0 on success  or -1 on failed
 * notes:
 */
int ak_uuid_close(void);

/**
 * ak_uuid_get_global_id   - get global_id
 * @pathname[OUT]: buf
 * return: return 0 on success   or -1 on failed
 * notes: fore read from hardware
 */
int ak_uuid_get_global_id(unsigned char** buf);

/*
* ak_uuid_get_global_id_length   - get global_id length
* @pathname[OUT]: length
* return: return 0 on success   or -1 on failed
* notes:
*/
int ak_uuid_get_global_id_length(int* length);

/*
* ak_uuid_get_version   - get uuid version
* return: return version
* notes:
*/
const char* ak_uuid_get_version(void);

#endif

/* end of file */
