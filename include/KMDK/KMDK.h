/**
 * @file KMDK.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief Some macro and function for the initialize of a kernel modul
 * @version 0.1
 * @date 2022-08-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _H_KMDK
#define _H_KMDK
#include"kobjects/kobject.h"
#include"types.h"
#define KM_MAGIC 0xd0d8e1f4
#define KM_VER_MAIN 0
#define KM_VER_SUB 1
#define KM_VER_DEBUG 2
typedef struct KMDKInfo
{
    uint8_t versions[3];
    uint8_t* name;
    uint8_t* description;
    kobject_operations_t ops;
}KMDKInfo_t;
#define KM_MAKE_VERSION(a,b,c){a,b,c}
#define KM_SETUP_INFO_STR "__KM_SETUP_INFO_"
#define KM_CREATE(a) uint32_t __KM_MAGIC_=KM_MAGIC; KMDKInfo_t a =
#define KM_RELEASE(a) KMDKInfo_t* __KM_SETUP_INFO_=&a;
#endif