/**
 * @file list.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief Simple list
 * @version 0.1
 * @date 2022-08-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _H_LIST
#define _H_LIST
#include"types.h"

typedef struct list_elem
{
    struct list*next;
    struct list*prv;
}list_elem_t;

void list_elem_append(list_elem_t*parent,list_elem_t*add,uint32_t offset);
#define list_append(parent,child,name) list_elem_append((parent),(child),)

#endif