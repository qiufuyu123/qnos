/**
 * @file qhash.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief Hash function
 * @version 0.1
 * @date 2022-08-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _H_QHASH
#define _H_QHASH
#include"string.h"
#include"types.h"
#define QHASH uint32_t
QHASH get_qhash(const char * key);
#endif