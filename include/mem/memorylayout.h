/**
 * @file memorylayout.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief All information about memory structure is defined here
 * @version 0.1
 * @date 2022-07-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */



#ifndef _H_MEM_LAYOUT
#define _H_MEM_LAYOUT
#include"types.h"
#define TEXT_VIDEO_BUFFER 0xB8000
#define KPAGE_MEM_SIZE 1024*1024*4  //4MB actually
//#define KERNEL_BASE_ADDR_END 
#define ngx_align(d, a)     (((d) + (a - 1)) & ~(a - 1))

/**
 * @brief SPECIAL MEMORY LAYOUT
 * So, if you look through the memory layout
 * , you will find that the memory structure is quite different from 
 * the normal high-half kernel. It is totally opposite 
 * 
 * The first 1g 0x0~0x40000000 is for kernel
 * while the other 3g is after 0x40000000 which is for user space xD
 * 
 * May be you ask why
 * However, i dont want to move the whole kernel to 0xc0000000
 * Because i dont want to write anything about paging with assembly
 * you know, i hate x86 ASM!!
 * 
 *     qwq         
 *          edited by qiufuyu
 * 
 * P.S. I am not sure whether it will cause any problems... you know, i am unsure...
 */

typedef struct kmemorylayout
{
    uint32_t text_video_buffer;
    uint32_t total_mem_in_kb;
    uint32_t kstart;
    uint32_t kend;
    uint32_t phy_bitmap_addr; //256kb
    uint32_t kpde_phy_addr;
    //uint32_t vir_bitmap_addr;
    uint32_t phy_bitmap_need_bytes;

    uint32_t kpage_dir_phy_addr;
    uint32_t kfree_paddr_start;

    uint32_t user_base_addr_start;
}kmemorylayout_t;
extern kmemorylayout_t kernel_mem_map;

uint32_t get_aligned_addr(uint32_t addr);
void init_memorylayout(uint32_t kstart,uint32_t kend,uint32_t total_mem_in_kb);

void pmm_init();
uint32_t pmm_get_page();
void pmm_free_page(uint32_t addr);
uint32_t pmm_get_used();
#endif