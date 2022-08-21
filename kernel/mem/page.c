#include"mem/page.h"
#include"mem/bitmap.h" 
#include"console.h"
#include"mem/memorylayout.h"
#include"string.h"
page_directory_t kpdir;
static inline void invlpg(void* m)
{
    /* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
    asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}
void switch_page_directory(page_directory_t *dir)
{
   //current_directory = dir;
   asm volatile("mov %0, %%cr3":: "r"(dir->ptable_dir));
   uint32_t cr0;
   asm volatile("mov %%cr0, %0": "=r"(cr0));
   cr0 |= 0x80000000; // Enable paging!
   asm volatile("mov %0, %%cr0":: "r"(cr0));
}
int alloc_page(page_t *p,bool is_kernel,bool writeable)
{
    if(!p)return-1;
    if(p->frame!=0)
    {
        printf("alloced?%x",*(uint32_t*)p);
        return 0;//already alloced
    }else
    {
        uint32_t paddr=pmm_get_page();
        //printf("paddr:0x%x!",paddr);
        if(!paddr)return -1;
        p->present=1;
        p->rw=(writeable)?1:0;
        p->user=(is_kernel)?0:1;
        p->frame=paddr/0x1000;
        return paddr/0x1000;
    }
}
int alloc_page_paddr(page_t *p,bool is_kernel,bool writeable,uint32_t paddr)
{
    
    if(!p)return-1;
    if(p->frame!=0)
    {
        printf("alloced?%x",*(uint32_t*)p);
        return 0;//already alloced
    }else
    {
        
        //printf("paddr:0x%x!",paddr);
        if(!paddr)return -1;
        pmm_set_page(paddr);
        p->present=1;
        p->rw=(writeable)?1:0;
        p->user=(is_kernel)?0:1;
        p->frame=paddr/0x1000;
        return paddr/0x1000;
    }
}
bool free_page(page_t *p)
{
    if(p->frame==0)return true;
    else
    {
        printf("freeing 0x%x",p->frame);
        pmm_free_page(p->frame*0x1000);
        p->present=0;
        p->frame=0;
        //  |
        //  v   We dont need to reflush the whole dir
        //switch_page_directory(&kpdir);
        printf("[%x]",*p);
        
        return true;
    }
}
page_t *get_page_from_pdir(page_directory_t *pd,uint32_t vaddr)
{  
    vaddr/=4096;
    int idx=vaddr/1024;
    if(pd->ptable[idx])
    {
        pd->ptable_dir[idx]=(uint32_t)pd->ptable[idx]|0x7;
        return &pd->ptable[idx]->pages[vaddr%1024];
    }
    else
    {
        //TODO make a pte
        printf(":(");
        return 0;
    }
}
void page_set_WR(page_directory_t *pdir,uint32_t vaddr,uint8_t value)
{
    page_t*p= get_page_from_pdir(pdir,vaddr);
    if(p)
    {
        p->rw=value;
        invlpg(vaddr);
    }
}
void page_u_map_unset(page_directory_t*pdir, uint32_t vaddr)
{
    free_page(get_page_from_pdir(pdir,vaddr));
    invlpg(vaddr);
}
void page_u_map_set(page_directory_t*pdir,uint32_t vaddr)
{
    alloc_page(get_page_from_pdir(pdir,vaddr),1,1);
    //printf("set:0x%x;",vaddr);
    invlpg(vaddr);
}
void page_u_map_set_pa(page_directory_t*pdir,uint32_t vaddr,uint32_t pa)
{
    alloc_page_paddr(get_page_from_pdir(pdir,vaddr),1,1,pa);
    //printf("set:0x%x;",vaddr);
    invlpg(vaddr);
}
void page_map_unset(uint32_t vaddr)
{
    free_page(get_page_from_pdir(&kpdir,vaddr));
    invlpg(vaddr);
}
void page_map_set(uint32_t vaddr)
{
    alloc_page(get_page_from_pdir(&kpdir,vaddr),1,1);
    //printf("set:0x%x;",vaddr);
    invlpg(vaddr);
}
void page_map_set_pa(uint32_t vaddr,uint32_t pa)
{
    alloc_page_paddr(get_page_from_pdir(&kpdir,vaddr),1,1,pa);
    //printf("set:0x%x;",vaddr);
    invlpg(vaddr);
}
void init_page()
{
    uint32_t j=0;
    
    for (int i = 0; i < 1024; i++)
    {
        //int s=sizeof(page_t);
        kpdir.ptable[i]=kernel_mem_map.kpage_dir_phy_addr+i*sizeof(page_table_t);
        //memset(kpdir.ptable[i],0,sizeof(page_table_t));
        //kpdir.ptable_dir[i]=(uint32_t)(kpdir.ptable[i])|0x07;
    }
    memset(kernel_mem_map.kpage_dir_phy_addr,0,kernel_mem_map.kfree_paddr_start-kernel_mem_map.kpage_dir_phy_addr+1);
    //map kernel part to itself!
    printf("kfree_paddr:0x%x bitmap:0x%x pdir:0x%x kpde:0x%x\n",kernel_mem_map.kfree_paddr_start,kernel_mem_map.phy_bitmap_addr,kernel_mem_map.kpage_dir_phy_addr,kernel_mem_map.kpde_phy_addr);
    //while(1);
    kpdir.ptable_dir=kernel_mem_map.kpde_phy_addr;//IMPORTANT!


    /**
     * @brief IMPORTANT MARK OF PAGING
     * 
     * What a *** paging!
     * very stupid mistake
     * The base address of pde (kpdir.ptable_dir) must be 4k aligned?????!!!!!!
     * I thought only the pte and paging address should be 4k aligned
     * but......
     * Too strange! I spend so many time here...
     * I cant believe
     * 
     * By the way , bochs debug is really important....
     * 
     *      edited by qiufuyu IMPORTANT...
     * 
     */


    for (uint32_t i = 0x00001000; i <kernel_mem_map.video_frambuffer_addr; i+=0x1000)
    {
        int idx= alloc_page(get_page_from_pdir(&kpdir,i),1,1);
    }
    //last map the video buffer
    
    
    //page_map_set(Klogger->frame_buffer);
    printf("map vbuffer:%x\n",Klogger->frame_buffer);
    switch_page_directory(&kpdir);
    for (int i = 0; i < Klogger->page_cnt; i++)
    {
        page_map_set_pa(kernel_mem_map.video_frambuffer_addr+i*4096,Klogger->frame_buffer+i*4096);
    }
    Klogger->frame_buffer=kernel_mem_map.video_frambuffer_addr;
    Klogger->update();
    printf("PAGE OK! 0x100000~0x%x WUHOOOOOOOO!",kernel_mem_map.kfree_paddr_start);
}
