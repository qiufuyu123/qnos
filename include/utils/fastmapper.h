/**
 * @file fastmapper.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief Map a list into a array
 * @version 0.1
 * @date 2022-08-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _H_FASTMAPPER
#define _H_FASTMAPPER
#include"types.h"
#include"list.h"
#include"utils/qhash.h"
typedef struct karray_mapper
{
    QHASH key;
    void *data;

}karray_mapper_t;

typedef struct karray
{
    uint32_t*elems;
    //uint32_t capacity;
    uint32_t id_start;
    uint32_t id_end;
    list_elem_t next_tag;
}karray_t;

typedef struct fastmapper
{
    list_t karray_list;
    uint8_t unit_len;
}fastmapper_t;

void fastmapper_init(fastmapper_t*map,uint8_t unit);

void *fastmapper_get(fastmapper_t*map,uint32_t id);

int fastmapper_add_auto(fastmapper_t*map,uint32_t elem);

void *fastmapper_key_get(fastmapper_t*map,uint32_t key);

int fastmapper_add(fastmapper_t*map,uint32_t elem,uint32_t id);

void fastmapper_remove(fastmapper_t*map,uint32_t id);

void *fastmapper_find(fastmapper_t*map,void *expect,bool (*cmp)(void*value,void*expect));

#endif