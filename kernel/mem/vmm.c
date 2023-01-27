#include"mem/vmm.h"
#include"console.h"
#include"string.h"
#include"mem/memorylayout.h"
/**
 * @brief NOTICE
 * I am not sure whether there is any bug here
 * However, i dont want to debug anymore
 * (Luckily, i have done a lot of tests, generally, they all work properly)
 * 
 *      edit by qiufuyu
 */
vmm_buddy_list_t kbuddy_list;
vmm_page_t kvmm_page_list[262144];
int vmm_alloc_pnum(uint32_t size)
{
    uint32_t now_s=0;
    uint32_t rec=0;
    for (int i = 0; i < VMM_MAX_PAGE_NUM; i++)
    {
        if(kvmm_page_list[i].status==0)
        {
            if(!rec)rec=i;
            now_s++;
        }
        else
        {
            now_s=0;
            rec=0;
        } 
        if(now_s==size)
        {
            for (int j = rec; j <rec+size; j++)
            {
                kvmm_page_list[j].status=1;
            }
            
            return rec;
        }
    }
    return -1;
}

//Uhhhhhhhhh
//So , obviously, the problem is that we dont know whether it is success to 
//append the page
//However
//may be its not necessary
//idk............runnnnnnn........
//BY THE WAY 
//Why i am using MACRO?
//confused......
void vmm_page_append_to_buddy(uint8_t buddy_idx,uint32_t new_page)
{
    uint32_t next_idx=kbuddy_list.area_list[buddy_idx].p_first_num;
    kbuddy_list.area_list[buddy_idx].p_first_num=new_page;
    kvmm_page_list[new_page].next_page=next_idx;
} 

int get_free_page_in_area(vmm_area_t *area )
{
    if(area->p_first_num==0)return -1;
    uint32_t cur_page=area->p_first_num;
    area->p_first_num=kvmm_page_list[cur_page].next_page;
    kvmm_page_list[cur_page].next_page=0;
    return cur_page;
    
}
int get_friend_size(uint8_t old_sz)
{
    if(old_sz==PG_1024)return-1;
    return old_sz+1;
}
void do_page_merge(uint32_t prv_page,uint32_t cur_page, uint32_t m_page,uint32_t cur_pg_sz)
{
    int now_sz=get_friend_size(cur_pg_sz);
    if(now_sz==-1)return;
    if(m_page>cur_page)
    {
        //kvmm_page_list[cur_page].next_page=m_page;
        kvmm_page_list[prv_page].next_page=kvmm_page_list[m_page].next_page;
        vmm_page_append_to_buddy(now_sz,cur_page);
    }else
    {
        kvmm_page_list[prv_page].next_page=kvmm_page_list[cur_page].next_page;
        vmm_page_append_to_buddy(now_sz,m_page);  
    }
}
void vmm_page_merge(uint32_t pg_num, uint8_t pg_sz)
{
    if(pg_sz==PG_1024)return;
    int next_page=0;
    int cur_page=kbuddy_list.area_list[pg_sz].p_first_num;
    int last_page=0;
    int target_last_page=0;
    while (kvmm_page_list[cur_page].next_page)
    {
        if(cur_page>pg_num&& cur_page-pg_num==1<<pg_sz)
        {
            //MERGE IT!
            //printf("MERGE! %d and %d\n",cur_page,pg_num);
            kvmm_page_list[last_page].next_page=kvmm_page_list[cur_page].next_page;
            kbuddy_list.area_list[pg_sz].p_first_num=kvmm_page_list[pg_num].next_page;
            vmm_page_append_to_buddy(get_friend_size(pg_sz),pg_num);

            return;
        }else if(pg_num>cur_page&&pg_num-cur_page==1<<pg_sz)
        {
            //printf("MERGE! %d and %d\n",cur_page,pg_num);
            kvmm_page_list[last_page].next_page=kvmm_page_list[cur_page].next_page;
            kbuddy_list.area_list[pg_sz].p_first_num=kvmm_page_list[pg_num].next_page;
            vmm_page_append_to_buddy(get_friend_size(pg_sz),cur_page);
            return;
        }
        last_page=cur_page;
        cur_page=kvmm_page_list[cur_page].next_page;
    }
}
void vmm_page_free(uint32_t pg_num,uint8_t pg_sz)
{
    //memset(pg_num*0x1000,0,1<<pg_sz);
    vmm_page_append_to_buddy(pg_sz,pg_num);
    vmm_page_merge(pg_num,pg_sz);
    
}
int vmm_page_alloc(uint8_t pg_sz)
{
    //first,search in the suitable mem area list
    int ret_pg;
    ret_pg= get_free_page_in_area(&kbuddy_list.area_list[pg_sz]);
    if(ret_pg!=-1)return ret_pg;
    //Unluckily,the area is fullll;
    //printf("full area.%d",pg_sz);
    int friend_sz=get_friend_size(pg_sz);
    if(friend_sz==-1)
    {
        printf("PAGE IS EXGHAUST!(TOO LARGE)\n:(\n");
        while(1);
    }
    //most of time , it is OK .
    //I mean....
    //Then alloc in friend block
    //:)
    //TO USE 递归
    int friend_pg=vmm_page_alloc(friend_sz);
    if(friend_pg==-1)
    {
        printf("PAGE IS EXGHAUST!(FRIEND IS EXGHAUST!)\n:(\n");
        while(1);
    }
    //cancel it!
    kvmm_page_list[friend_pg].status=0;
    //split it
    int p1=friend_pg;
    int p2=friend_pg+(1<<pg_sz);
    kvmm_page_list[p2].status=0;
    kvmm_page_list[p2].next_page=0;
    //OK,FINALLY,WE GET A PAGE FROM FRIEND
    //now, lets append it
    //NOTICE:we dont need to alloc friend page again!!!
    //just append it!
    
    vmm_page_append_to_buddy(pg_sz,p1);
    vmm_page_append_to_buddy(pg_sz,p2);
    //LAST,give this area another chance
    //给你机会别不中用啊
    return vmm_page_alloc(pg_sz);
    //We dont need to check anything
    //if it works properly...
}
//#define __PROCESS_SIZE(size,pg_size) if(size==)
uint32_t vmm_buddy_alloc(uint32_t size)
{
    //let size align 4k!
    //size=ngx_align(size,4096)/4096;
    //split it!
    //printf("psz:%d", size);
    if(size==1)
    {
        return vmm_page_alloc(PG_1)*4096;
    }else if(size>1&&size<=2)return vmm_page_alloc(PG_2)*4096;
    else if(size>2&&size<=4)return vmm_page_alloc(PG_4)*4096;
    else if(size>4&&size<=8)return vmm_page_alloc(PG_8)*4096;
    else if(size>8&&size<=16)return vmm_page_alloc(PG_16)*4096;
    else if(size>16&&size<=32)return vmm_page_alloc(PG_32)*4096;
    else if(size>32&&size<=64)return vmm_page_alloc(PG_64)*4096;
    else if(size>64&&size<=128)return vmm_page_alloc(PG_128)*4096;
    else if(size>128&&size<=256)return vmm_page_alloc(PG_256)*4096;
    else if(size>256&&size<=512)return vmm_page_alloc(PG_512)*4096;
    else if(size>512&&size<=1024)return vmm_page_alloc(PG_1024)*4096;
    else if(size>1024)return 0;
}

void vmm_buddy_free(uint32_t addr,uint32_t size)
{
    //size=ngx_align(size,4096)/4096;
    if(size==1)vmm_page_free(addr/4096,PG_1);
    else if(size>1&&size<=2)vmm_page_free(addr/4096,PG_2);
    else if(size>2&&size<=4)vmm_page_free(addr/4096,PG_4);
    else if(size>4&&size<=8)vmm_page_free(addr/4096,PG_8);
    else if(size>8&&size<=16)vmm_page_free(addr/4096,PG_16);
    else if(size>16&&size<=32)vmm_page_free(addr/4096,PG_32);
    else if(size>32&&size<=64)vmm_page_free(addr/4096,PG_64);
    else if(size>64&&size<=128)vmm_page_free(addr/4096,PG_128);
    else if(size>128&&size<=256)vmm_page_free(addr/4096,PG_256);
    else if(size>256&&size<=512)vmm_page_free(addr/4096,PG_512);
    else if(size>512&&size<=1024)vmm_page_free(addr/4096,PG_1024);
    
}

void init_vmm()
{
    memset(kvmm_page_list,0,VMM_MAX_PAGE_NUM*sizeof(vmm_page_t));
    //first, we alloc every page that we used for kernel space
    for (int i = 0; i < kernel_mem_map.kfree_paddr_start/4096; i++)
    {
        kvmm_page_list[i].status=1;
    }
    //than , prepare for the buddy_list
    
    /*for (int i = 0; i < PG_ERROR; i++)
    {
        kbuddy_list.area_list[i].p_first_num=vmm_alloc_pnum((1<<i));
        //kvmm_page_list[kbuddy_list.area_list[i].p_first_num].status=1;//used
        printf("vmm:%d size:%d\n",kbuddy_list.area_list[i].p_first_num,1<<i);
    }*/
    //But i think may be i can alloc 2 page for 1k area 
    //OK, not necessary at all,is it?
    //AWA
    //vmm_page_append_to_area(kbuddy_list.area_list[0].p_first_num,vmm_alloc_pnum(1));
    //printf("another 1k for %d--->%d\nAHH HA! IT comes to endless....",kbuddy_list.area_list[0].p_first_num,kvmm_page_list[kbuddy_list.area_list[0].p_first_num].next_page);
   
    //last,alloc all the page for the max size 
    for (int i =PG_1024; i >=0; i--)
    {
        while (1)
        {
            int idx=vmm_alloc_pnum(1<<i);
            if(idx==-1)break;
            vmm_page_append_to_buddy(i,idx);
        //printf("%d,",idx);
        }
    }
    
    //while(1);
    //memory exhaustttttttttttttt....... LOL
    /*for (int i = 0; i < VMM_MAX_PAGE_NUM; i++)
    {
        kvmm_page_list[i].status=0;
    }*/
    //in fact, status is not useful any more after 1st init
    //But i dont delete it
    //Maybe it is useful in somewhere? IDK...
    
    printf("vmm ok!\n");
    uint32_t p1=vmm_buddy_alloc(1);
    uint32_t p2=vmm_buddy_alloc(1);
    uint32_t p3=vmm_buddy_alloc(1);
    printf("0x%x 0x%x x%x\n",p1,p2,p3);
    vmm_buddy_free(p1,1);
    vmm_buddy_free(p2,1);
    vmm_buddy_free(p3,1);
    //kbuddy_list.area_list[0]
}
void vmm_doublemap_pages(page_directory_t*newpdt, uint32_t vaddr_in_kernel,uint32_t page_cnt,uint32_t new_vaddr)
{
    
    //vmm_buddy_free(vaddr_in_kernel,page_cnt);//free vmm information
    for (int i = 0; i < page_cnt; i++)
    {
        //printf("s1\n");
        uint32_t paddr= page_kv2p(vaddr_in_kernel+i*4096);
        page_u_map_set_pa(newpdt,new_vaddr+i*4096,paddr);//rebuild mapping for user
        //printf("sn\n");
    }  
}
void vmm_remapcls_pages(page_directory_t*newpdt, uint32_t vaddr_in_kernel,uint32_t page_cnt,uint32_t new_vaddr)
{
    vmm_buddy_free(vaddr_in_kernel,page_cnt);//free vmm information
    for (int i = 0; i < page_cnt; i++)
    {
        uint32_t paddr= page_kv2p(vaddr_in_kernel+i*4096);
        page_unlink_pa(vaddr_in_kernel+i*4096);//Unlink the mapping
        if(page_chk_user(newpdt,new_vaddr+i*4096))
        {
            page_u_map_unset(newpdt,new_vaddr+i*4096); //free the used map
        }
        page_u_map_set_pa(newpdt,new_vaddr+i*4096,paddr);//rebuild mapping for user
    }
}
void vmm_remap_pages(page_directory_t*newpdt, uint32_t vaddr_in_kernel,uint32_t page_cnt,uint32_t new_vaddr)
{
    
    vmm_buddy_free(vaddr_in_kernel,page_cnt);//free vmm information
    for (int i = 0; i < page_cnt; i++)
    {
        uint32_t paddr= page_kv2p(vaddr_in_kernel+i*4096);
        page_unlink_pa(vaddr_in_kernel+i*4096);//Unlink the mapping
        page_u_map_set_pa(newpdt,new_vaddr+i*4096,paddr);//rebuild mapping for user
    }
}
int vmm_checkvalid(uint32_t vaddr)
{
    
}