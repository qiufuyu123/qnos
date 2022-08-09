/**
 * @file malloc.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief Kernel slab alloc includes kmalloc and kfree
 * @version 0.1
 * @date 2022-08-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _H_MALLOC
#define _H_MALLOC
#include"types.h"
#include"mem/bitmap.h"
#define SLAB_MAX_NUM 255
typedef struct kmem_elem
{
    struct kmem_elem *next;
}kmem_elem_t;
/**
 * @brief SLAB&HEAP
 * Obviously, after allocing slab, some mem space is extra(after 'free_data_addr')
 * We build a heap after this address so that we can use the space effectively
 */

//TODO: kheap for extra area

typedef struct kheap
{
    
}kheap_t;



typedef struct slab_unit
{
    //uint8_t *data;
    char slab_name[20];
    uint8_t flag_sz;
    uint32_t pg_cnt;
    uint32_t total_max_cnt;
    uint32_t used_cnt;
    uint16_t unit_size;
    kmem_elem_t* free_data;
    uint32_t free_data_addr;    //for extra data(heap)
    struct slab_unit*next;
}slab_unit_t;



bool init_kslab();
uint32_t kmalloc_page(uint32_t pnum);
void kfree_page(uint32_t vaddr,uint32_t pnum);
uint32_t kmalloc(uint32_t size);
void kfree(uint32_t addr);
slab_unit_t *alloc_slab_unit(uint32_t unit_size,char *name);
uint32_t alloc_in_slab_unit(slab_unit_t* unit);
void free_in_slab_unit(uint32_t vaddr);
#endif