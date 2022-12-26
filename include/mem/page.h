/**
 * @file page.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief functions about operations of pages and mem map
 * @version 0.1
 * @date 2022-07-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _H_PAGE
#define _H_PAGE
#include"types.h"
//Why we all use u32 here?
//notice that we put '1' after each variable
//since we make the size of each element as the size of 1 bit by hand

typedef struct page
{
   uint32_t present    : 1;   // Page present in memory
   uint32_t rw         : 1;   // Read-only if clear, readwrite if set
   uint32_t user       : 1;   // Supervisor level only if clear
   uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
   uint32_t dirty      : 1;   // Has the page been written to since last refresh?
   uint32_t unused     : 7;   // Amalgamation of unused and reserved bits
   uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_table
{
   page_t pages[1024];
} page_table_t;
typedef struct page_directory
{
    page_table_t* ptable[1024];
    uint32_t  *ptable_dir;  //The 1st page table which we should put to the cr04
    uint32_t cr0_paddr;
}page_directory_t;
extern page_directory_t kpdir;
void init_page();
page_directory_t *page_clone_cleaned_page();
page_directory_t *page_clone_user_page(page_directory_t*updt,uint32_t from_vaddr);
uint32_t page_kv2p(uint32_t va);
//int alloc_page(page_t *p,bool is_kernel,bool writeable);
void page_map_unset(uint32_t vaddr);
void page_map_set(uint32_t vaddr);
void page_unlink_user(page_directory_t*pdt, uint32_t vaddr);
void page_unlink_pa(uint32_t vaddr);
page_t *get_page_from_pdir(page_directory_t *pd,uint32_t vaddr);
void page_set_WR(page_directory_t *pdir,uint32_t vaddr,uint8_t value);
void page_u_map_set(page_directory_t*pdir,uint32_t vaddr);
void page_u_map_unset(page_directory_t*pdir,uint32_t vaddr);
void page_u_map_set_pa(page_directory_t*pdir,uint32_t vaddr,uint32_t pa);
void page_setup_pdt(page_directory_t*p);
void page_setup_kernel_pdt();
bool page_chk_user(page_directory_t*updt,uint32_t vaddr);
void page_free_pdt(page_directory_t *p);
#endif