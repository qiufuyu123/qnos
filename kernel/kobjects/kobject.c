#include"kobjects/kobject.h"
#include"mem/malloc.h"
#include"string.h"
#include"kobjects/kobjs.h"
#include"console.h"
#include"utils/qhash.h"
kobject_t *kobj_list[KOBJ_MAX];
slab_unit_t *kobj_slab;
NONE_OPEN
NONE_WRITE
NONE_READ
NONE_ATTRSET
NONE_EXFUNC
NONE_BEGIN
NONE_END
#define  KOBJECT_ALLOC alloc_in_slab_unit(kobj_slab)
int search_in_list()
{
    for (int i = 0; i < KOBJ_MAX; i++)
    {
        if(kobj_list[i]==0)return i;
    }
    return -1;
    
}
int get_type_name(char *name)
{
    uint32_t hash=get_qhash(name);
    for (int i = 0; i < KOBJ_MAX; i++)
    {
        if(kobj_list[i])
        {
            if(hash==kobj_list[i]->name_hash)return i;
        }
    }
    return -1;    
}
kobject_t* create_kobject(uint32_t(*begin)(uint32_t val),uint32_t(*end)(),kobject_operations_t *op,char *name)
{
    if(get_type_name(name)>-1)return 0;
    kobject_t*re=KOBJECT_ALLOC;
    //printf("kobj:0x%x",re);
    if(!re)return 0;
    re->begin=begin?begin:&non_begin;
    re->end=end?end:&non_end;
    strcpy(re->name,name);
    re->ops=op;
    if(!re->ops->attrset)re->ops->attrset=non_attrset;
    if(!re->ops->exfunc)re->ops->exfunc=non_exfunc;
    if(!re->ops->open)re->ops->open=non_open;
    if(!re->ops->read)re->ops->read=non_read;
    if(!re->ops->write)re->ops->write=non_write;
    int idx=search_in_list();
    //printf("idx%d",idx);
    if(idx==-1)
    {
        free_in_slab_unit(re);
        return 0;
    }
    re->type=idx;
    re->name_hash=get_qhash(name);
    kobj_list[idx]=re;
    re->begin(re);
    //printf("allocok!\n");
    return re;
}
void init_all_objs()
{
    init_kernel_obj();
    init_ata_obj();
    //kobj_list[KO_KERNEL_DEV]->ops->write(0,"hihihi",0,0);
}
void init_kobject()
{
    kobj_slab=alloc_slab_unit(sizeof(kobject_t),"slab_kobject");
    if(!kobj_slab)return;
    
    init_all_objs();
}
kobject_operations_t *kobject_get_ops(uint32_t type)
{
    if(type>=KOBJ_MAX)return 0;
    return kobj_list[type]->ops;
}
void release_kobject(kobject_t*ko)
{
    kobj_list[ko->type]=0;
    ko->end();
    kfree(ko);
}