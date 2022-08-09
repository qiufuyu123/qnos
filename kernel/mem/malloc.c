#include"mem/malloc.h"
#include"mem/vmm.h"
#include"mem/page.h"
#include"console.h"
#include"mem/memorylayout.h"
#include"string.h"
enum
{
    KSLAB_8=1,
    KSLAB_16,
    KSLAB_32,
    KSLAB_64,
    KSLAB_128,
    KSLAB_256,
    KSLAB_512,
    KSLAB_1024,
    KSLAB_2048
};
slab_unit_t *kslab_list[SLAB_MAX_NUM];
uint32_t kmalloc_page(uint32_t pnum)
{
    uint32_t addr=vmm_buddy_alloc(pnum);
    if(!addr)return 0;
    //printf("vaddr:0x%x %d",addr,pnum);
    for(int i=0;i<pnum;i++)
    {
        page_map_set(addr+i*4096);
    }
    //memset(addr,0,pnum*4096);
    //printf("mapped ok!");
    return addr;
}
void kfree_page(uint32_t vaddr,uint32_t pnum)
{
    vmm_buddy_free(vaddr,pnum);
    for(int i=0;i<pnum;i++)
    {
        page_map_unset(vaddr+i*4096);
    }
}
/**
 * @brief SLAB
 * 
 * So the first thing is slab allocator
 * which is used for allocing small mem blocks
 * Just like the buddy
 * 
 */
slab_unit_t *alloc_slab_unit(uint32_t unit_size,char *name)
{
    if(ngx_align(unit_size,4096)/4096>7||unit_size<8)return 0;
    double stand_wast=(double)1/8;
    for (int i = 1; i < SLAB_MAX_NUM; i++)
    {
        if(kslab_list[i]==0)
        {

            //calc the best fit page cnt
            uint32_t exact_space=4096-sizeof(slab_unit_t)-1;
            uint32_t need_page=1;
            if(unit_size>exact_space)
            {
                need_page=ngx_align(unit_size,4096)/4096+1;
            }
            uint32_t real_space=exact_space+(need_page-1)*4096;
            uint32_t waste_space=(exact_space+(need_page-1)*4096)%(unit_size+1);
            //double waste_rate=(waste_space)/(exact_space+(need_page-1)*4096);
            
            /**
             * @brief The waste rate standart
             * 
             * The waste rate(wastespace/realspace) should not bigger than 1/8 of the realspace
             * 
             */
            
            
            while (need_page<8&&waste_space>=real_space/8)
            {
                //printf("waste:%d>%d;",waste_space,real_space/8);
                need_page++;
                waste_space=(exact_space+(need_page-1)*4096)%(unit_size+1);
                real_space=exact_space+(need_page-1)*4096;
                //waste_rate=(waste_space)/(exact_space+(need_page-1)*4096);
            }
            
            kslab_list[i]=kmalloc_page(need_page);
            //printf("need %d 0x%x",need_page,kslab_list[i]);
            if(!kslab_list[i])return 0;
            kslab_list[i]->flag_sz=i;
            memset(kslab_list[i]->slab_name,0,20);
            memcpy(kslab_list[i]->slab_name,name,20);
            kslab_list[i]->pg_cnt=need_page;
            kslab_list[i]->total_max_cnt=(exact_space+(need_page-1)*4096)/(unit_size+1);
            kslab_list[i]->unit_size=unit_size;
            kslab_list[i]->used_cnt=0;
            kslab_list[i]->free_data=(uint32_t)kslab_list[i]+sizeof(slab_unit_t)+1;
            kmem_elem_t *cur=kslab_list[i]->free_data;
            //*(uint8_t*)((uint32_t)cur+1)=i;
            //slab_unit_t*test=kslab_list[i];
            
            //printf("pause! pg_cnt%d\n",kslab_list[i]->pg_cnt);
            for (int j = 1; j <kslab_list[i]->total_max_cnt; j++)
            {
                *(uint8_t*)(cur)=i;
                cur->next=(uint32_t)cur+unit_size+1;
                cur=cur->next;
                //printf("%x",cur);
            }
            //printf("xxxx");
            kslab_list[i]->next=0;
            kslab_list[i]->free_data_addr=(uint32_t)cur+unit_size+1;
            //build the whole list!
            return kslab_list[i];
            
        }
        
    }
    
}
slab_unit_t* clone_slab_unit(slab_unit_t * base)
{
    slab_unit_t *re=kmalloc_page(base->pg_cnt);
    if(!re)return 0;
    memcpy(re,base,sizeof(slab_unit_t));
    re->used_cnt=0;
    re->free_data=(uint32_t)re+sizeof(slab_unit_t)+1;
    re->next=0;
    kmem_elem_t*cur=re->free_data;
    for (int j = 1; j <re->total_max_cnt-1 ; j++)
    {
        *(uint8_t*)(cur)=re->flag_sz;
        cur->next=(uint32_t)cur+re->unit_size+1;
        cur=cur->next;
        //printf("%x",cur);
    }
            //printf("xxxx");
    re->free_data_addr=(uint32_t)cur+re->unit_size+1;
    return re;
}
uint32_t alloc_in_slab_unit(slab_unit_t* unit)
{
    slab_unit_t*cur=unit;
    while(cur->used_cnt>=cur->total_max_cnt)
    {
        //this slab is full
        if(cur->next)cur=cur->next;
        else
        {
            //we have to alloc a new one...
            //printf("a new slab:(");
            cur->next=clone_slab_unit(cur);
            if(!cur->next)return 0;
            return alloc_in_slab_unit(cur->next);
        }
    }
    kmem_elem_t*head=cur->free_data;
    cur->free_data=cur->free_data->next;
    cur->used_cnt++;
    printf("alloc ok0x%x;",head);
    *(uint8_t*)((uint32_t)head-1)=cur->flag_sz;
    //printf("%d;",cur->used_cnt);
    return (uint32_t)head;
}

void free_in_slab_unit(uint32_t vaddr)
{
    uint8_t flag_sz=*(uint8_t*)(vaddr-1);
    slab_unit_t *cur=kslab_list[flag_sz];
    if(!flag_sz)return;
    //printf("free flsz=%d;",flag_sz);
    //if(!cur)return;
    uint32_t left_bund=0,right_bund=0;
    while (cur)
    {
        /**
         * @brief WARNING
         * DO NOT try to locate simply by cur.free_data
         * Because 'free_data' could be changed by allocing (It is just a `free_list`)
         * We must calc the bundary by hand like these codes below
         *          edit by qiufuyu
         *  v v v v v v v v v v v v v v v v v v  v v  v v v v  v  
         */
        left_bund=(uint32_t)cur+sizeof(slab_unit_t)+1;
        right_bund=left_bund+(cur->unit_size+1)*cur->total_max_cnt;
        //printf("check %x~%x;",left_bund,right_bund);
        if(vaddr>=left_bund&&vaddr<=right_bund)
        {
            //find it!
            kmem_elem_t *head= cur->free_data->next;
            cur->free_data=vaddr;
            cur->free_data->next=head;
            printf("freeok:0x%x;",vaddr);   
            cur->used_cnt--;
            return;
        }
        cur=cur->next;
    }
    
}
uint32_t kmalloc(uint32_t size)
{
    if(size>1024)return alloc_in_slab_unit(kslab_list[KSLAB_2048]);
    else if(size>512)return alloc_in_slab_unit(kslab_list[KSLAB_1024]);
    else if(size>256)return alloc_in_slab_unit(kslab_list[KSLAB_512]);
    else if(size>128)return alloc_in_slab_unit(kslab_list[KSLAB_256]);
    else if(size>64)return alloc_in_slab_unit(kslab_list[KSLAB_128]);
    else if(size>32)return alloc_in_slab_unit(kslab_list[KSLAB_64]);
    else if(size>16)return alloc_in_slab_unit(kslab_list[KSLAB_32]);
    else if(size>8)return alloc_in_slab_unit(kslab_list[KSLAB_16]);
    else{
        return alloc_in_slab_unit(kslab_list[KSLAB_8]);
    }
}
void kfree(uint32_t vaddr)
{
    free_in_slab_unit(vaddr);
}
bool init_kslab()
{
    //kslab=kmalloc_page(1);
    ///if(!kslab)return false;
    slab_unit_t *test=alloc_slab_unit(8,"slab_8");
    //printf("%s kslab:0x%x pg_cnt:%d max_cnt:%d  first:0x%x end:0x%x unit:%d\n",test->slab_name, test,test->pg_cnt,test->total_max_cnt,test->free_data,test->free_data_addr,test->unit_size);
    test=alloc_slab_unit(16,"slab_16");
    //printf("%s kslab:0x%x pg_cnt:%d max_cnt:%d  first:0x%x end:0x%x unit:%d\n",test->slab_name, test,test->pg_cnt,test->total_max_cnt,test->free_data,test->free_data_addr,test->unit_size);
    test=alloc_slab_unit(32,"slab_32");
    //printf("%s kslab:0x%x pg_cnt:%d max_cnt:%d  first:0x%x end:0x%x unit:%d\n",test->slab_name, test,test->pg_cnt,test->total_max_cnt,test->free_data,test->free_data_addr,test->unit_size);
    test=alloc_slab_unit(64,"slab_64");
    //printf("%s kslab:0x%x pg_cnt:%d max_cnt:%d  first:0x%x end:0x%x unit:%d\n",test->slab_name, test,test->pg_cnt,test->total_max_cnt,test->free_data,test->free_data_addr,test->unit_size);
    test=alloc_slab_unit(128,"slab_128");
    //printf("%s kslab:0x%x pg_cnt:%d max_cnt:%d  first:0x%x end:0x%x unit:%d\n",test->slab_name, test,test->pg_cnt,test->total_max_cnt,test->free_data,test->free_data_addr,test->unit_size);
    test=alloc_slab_unit(256,"slab_256");
    //printf("%s kslab:0x%x pg_cnt:%d max_cnt:%d  first:0x%x end:0x%x unit:%d\n",test->slab_name, test,test->pg_cnt,test->total_max_cnt,test->free_data,test->free_data_addr,test->unit_size);
    test=alloc_slab_unit(512,"slab_512");
    //printf("%s kslab:0x%x pg_cnt:%d max_cnt:%d  first:0x%x end:0x%x unit:%d\n",test->slab_name, test,test->pg_cnt,test->total_max_cnt,test->free_data,test->free_data_addr,test->unit_size);
    test=alloc_slab_unit(1024,"slab_1024");
    //printf("%s kslab:0x%x pg_cnt:%d max_cnt:%d  first:0x%x end:0x%x unit:%d\n",test->slab_name, test,test->pg_cnt,test->total_max_cnt,test->free_data,test->free_data_addr,test->unit_size);
    test=alloc_slab_unit(2048,"slab_2048");
    //printf("%s kslab:0x%x pg_cnt:%d max_cnt:%d  first:0x%x end:0x%x unit:%d\n",test->slab_name, test,test->pg_cnt,test->total_max_cnt,test->free_data,test->free_data_addr,test->unit_size);
    /*uint32_t addr1= alloc_in_slab_unit(kslab_list[1]);
    uint32_t addr2=alloc_in_slab_unit(kslab_list[1]);
    free_in_slab_unit(addr1);
    free_in_slab_unit(addr2);*/
}