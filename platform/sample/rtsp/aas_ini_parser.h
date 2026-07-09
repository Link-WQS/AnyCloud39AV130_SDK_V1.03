/*
* Copyright (C) 2023 Anyka(GuangZhou) Microelectronics Co., Ltd.
* @DATE 2024-10-8
* @VERSION 1.0
*/

#ifndef _AAS_INI_PARSER_H_
#define _AAS_INI_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif


#if !defined(WIN32)
    #define __PACKED__        __attribute__ ((__packed__))
#else
    #define __PACKED__
#endif

//key:a-z/A-Z/0-9/_

typedef void* AAS_INI_HANDLE;

/**
 * aas_ini_load: aas_ini_load
 * @file[IN]: file
 * return: 0 - success; otherwise error code;
 */
AAS_INI_HANDLE aas_ini_load(const char *file);

/**
 * aas_ini_free: aas_ini_free
 * @handle[IN]: handle
 * return: 0 - success; otherwise error code;
 */
int aas_ini_free(AAS_INI_HANDLE handle);


/**
 * aas_ini_save_file: aas_ini_save_file
 * @handle[IN]: handle
 * @file[IN]: file
 * return: 0 - success; otherwise error code;
 */
int aas_ini_save_file(AAS_INI_HANDLE handle, const char *file);

/**
 * aas_ini_get_int: aas_ini_get_int
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * @def[IN]: def
 * return: value
 */
int aas_ini_get_int(AAS_INI_HANDLE handle,const char *section,const char *key,int def);

/**
 * aas_ini_set_int: aas_ini_set_int
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * @value[IN]: value
 * return: 0 - success; otherwise error code;
 */
int aas_ini_set_int(AAS_INI_HANDLE handle,const char *section,const char *key,int value);

/**
 * aas_ini_get_string: aas_ini_get_string
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * @def[IN]: def
 * return: str
 */
const char *aas_ini_get_string(AAS_INI_HANDLE handle,const char *section,const char *key,char* def);

/**
 * aas_ini_set_string: aas_ini_set_string
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * @string[IN]: string
 * return: 0 - success; otherwise error code;
 */
int aas_ini_set_string(AAS_INI_HANDLE handle,const char *section,const char *key,char* string);

/**
 * aas_ini_get_double: aas_ini_get_double
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * @def[IN]: def
 * return: value
 */
double aas_ini_get_double(AAS_INI_HANDLE handle,const char *section,const char *key,double def);

/**
 * aas_ini_set_double: aas_ini_set_double
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * @value[IN]: value
 * return: 0 - success; otherwise error code;
 */
int aas_ini_set_double(AAS_INI_HANDLE handle,const char *section,const char *key,double value);


#ifdef __cplusplus
}
#endif


#endif




