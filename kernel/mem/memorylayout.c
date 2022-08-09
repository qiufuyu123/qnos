#include"mem/memorylayout.h"
#include"mem/bitmap.h"
#include"string.h"
#include"console.h"
kmemorylayout_t kernel_mem_map; 

void init_memorylayout(uint32_t kstart,uint32_t kend,uint32_t total_mem_in_kb)
{
    kernel_mem_map.kend=kend;
    kernel_mem_map.kstart=kstart;
    kernel_mem_map.text_video_buffer=0xB8000;
    kernel_mem_map.total_mem_in_kb=total_mem_in_kb;
    kernel_mem_map.kpde_phy_addr=ngx_align(kend,4096);
    kernel_mem_map.phy_bitmap_addr=kernel_mem_map.kpde_phy_addr+4096;// /4/8;// 
    kernel_mem_map.phy_bitmap_need_bytes=kernel_mem_map.total_mem_in_kb/4/8;
    //kernel_mem_map.vir_bitmap_addr=kernel_mem_map.phy_bitmap_addr+kernel_mem_map.phy_bitmap_need_bytes;
    kernel_mem_map.kpage_dir_phy_addr=ngx_align(kernel_mem_map.phy_bitmap_addr+kernel_mem_map.phy_bitmap_need_bytes,4096);//Why add something? No reason , just feel safer xD
    kernel_mem_map.kfree_paddr_start=kernel_mem_map.kpage_dir_phy_addr+KPAGE_MEM_SIZE;
    kernel_mem_map.kfree_paddr_start=ngx_align(kernel_mem_map.kfree_paddr_start,0x1000);
    kernel_mem_map.user_base_addr_start=0x40000000;
    //memset(kernel_mem_map.kpage_dir_phy_addr,0,KPAGE_MEM_SIZE);
    pmm_init();
}