#include"mem/memorylayout.h"
#include"mem/bitmap.h"
bitmap_t pmm_bitmap;
uint32_t used_pmm;
void pmm_init()
{
    pmm_bitmap.bits=kernel_mem_map.phy_bitmap_addr;
    pmm_bitmap.btmp_bytes_len=kernel_mem_map.phy_bitmap_need_bytes;
    bitmap_init(&pmm_bitmap);
    /*for (int i = 0; i < 256; i++)
    {
        bitmap_set(&pmm_bitmap,i,1);
    }*/
    bitmap_set(&pmm_bitmap,0,1);
    used_pmm=1;
}
void pmm_set_page(uint32_t paddr)
{
    bitmap_set(&pmm_bitmap,paddr/4096,1);
    used_pmm++;   
}
uint32_t pmm_get_page()
{
    int idx=bitmap_scan(&pmm_bitmap,1);
    if(idx<0)return 0;
    bitmap_set(&pmm_bitmap,idx,1);
    used_pmm++;
    return idx*4096;
}
void pmm_free_page(uint32_t addr)
{
    bitmap_set(&pmm_bitmap,addr/4096,0);
    used_pmm--;
}
uint32_t pmm_get_used()
{
    return used_pmm*4;//convert to kb
}