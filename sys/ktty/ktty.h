/**
 * @file ktty.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief Kernel Module : TTY
 * @version 0.1
 * @date 2022-08-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#ifndef _H_KTTY
#define _H_KTTY
#include"types.h"

typedef struct ktty_context
{
    char *buffer;
    uint32_t buffer_sz;
    uint32_t rw_start;   //OFFSET
    uint32_t rw_end;     //OFFSET
}ktty_context_t;

//uint32_t a=sizeof(ktty_context_t);

#endif