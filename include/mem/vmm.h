/**
 * @file vmm.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief Buddy and slab allocater
 * @version 0.1
 * @date 2022-08-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _H_VMM
#define _H_VMM
#include"types.h"
#include"mem/page.h"
//262144 page in 1g kernel space ==> 2^18
enum 
{
    PG_1=0,
    PG_2,
    PG_4,
    PG_8,
    PG_16,
    PG_32,
    PG_64,
    PG_128,
    PG_256,
    PG_512,
    PG_1024,
    PG_ERROR
};
#define VMM_MAX_PAGE_NUM 262144
typedef struct vmm_page
{
    uint32_t next_page  :18;    //only store the index(page num)
    uint32_t status :1;         //status 0: all free 1: used
}vmm_page_t;
extern vmm_page_t kvmm_page_list[262144];
typedef struct vmm_area
{
    uint32_t p_first_num;
    //uint8_t p_size;
}vmm_area_t;

typedef struct vmm_buddy_list
{
    vmm_area_t area_list[PG_ERROR];
}vmm_buddy_list_t;

typedef struct alloced_page
{
    uint32_t vaddr;
    uint8_t pg_sz;
}alloced_page_t;


uint32_t vmm_buddy_alloc(uint32_t size);
void vmm_buddy_free(uint32_t addr, uint32_t size);
void init_vmm();
void vmm_doublemap_pages(page_directory_t*newpdt, uint32_t vaddr_in_kernel,uint32_t page_cnt,uint32_t new_vaddr);
void vmm_remap_pages(page_directory_t*newpdt, uint32_t vaddr_in_kernel,uint32_t page_cnt,uint32_t new_vaddr);
void vmm_remapcls_pages(page_directory_t*newpdt, uint32_t vaddr_in_kernel,uint32_t page_cnt,uint32_t new_vaddr);
#endif
