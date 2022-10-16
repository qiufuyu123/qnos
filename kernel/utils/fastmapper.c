#include"utils/fastmapper.h"
#include"mem/malloc.h"
slab_unit_t *array_slab;
#define alloc_array alloc_in_slab_unit(array_slab);
#define alloc_array_elem alloc_in_slab_unit(array_elem_slab);
void fastmapper_init(fastmapper_t*map,uint8_t unit)
{
    if(!array_slab)
    {
        array_slab=alloc_slab_unit(sizeof(karray_t),"fastmapper_karray");
    }
    if(!array_slab)return;
    map->unit_len=unit;
    list_init(&map->karray_list);
    karray_t *fst=alloc_array
    if(!fst)return;
    list_append(&map->karray_list,&fst->next_tag);
    fst->id_start=0;
    fst->id_end=unit;
    fst->elems=kmalloc(unit*sizeof(uint32_t));
    memset(fst->elems,0,unit*sizeof(uint32_t));
}
int __fast_travel(list_elem_t *e,int id)
{
    karray_t*cur_array=elem2entry(karray_t,next_tag, e);
    if(id>=cur_array->id_start&&id<cur_array->id_end)
    {
        return 1;
    }
    return 0;
}
int __fast_travel__key(list_elem_t *e,int key)
{
    karray_t*cur_array=elem2entry(karray_t,next_tag, e);
    for (int i = cur_array->id_start; i < cur_array->id_end; i++)
    {
        
    }
    
    return 0;
}
void *fastmapper_get(fastmapper_t*map,uint32_t id)
{
    if(!map)return 0;
    if(list_empty(&map->karray_list))return 0;
    
    list_elem_t*r= list_traversal(&map->karray_list,__fast_travel,id);
    if(!r)return 0;
    karray_t*cur_array=elem2entry(karray_t,next_tag, r);
    return cur_array->elems[id%map->unit_len];
    
}

void *fastmapper_key_get(fastmapper_t*map,uint32_t key)
{
    // if(!map)return 0;
    // if(list_empty(&map->karray_list))return 0;
    // int ret=key;
    // list_elem_t*r= list_traversal(&map->karray_list,__fast_travel__key,&ret);
    // if(!r)return 0;
    // karray_t*cur_array=elem2entry(karray_t,next_tag, r);
    // return cur_array->elems[id%map->unit_len];
}
typedef struct trav_v
{
    uint32_t addr;
    int *id;
}trav_v_t;

int __fast_travel__a(list_elem_t *e,trav_v_t *elem)
{
    karray_t*cur_array=elem2entry(karray_t,next_tag, e);
    for (int i = 0; i <cur_array->id_end-cur_array->id_start; i++)
    {
        if(!cur_array->elems[i])
        {
            cur_array->elems[i]=elem->addr;
            *(elem->id)=i+cur_array->id_start;
            return 1;
        }
    }
    
    return 0;
}
int fastmapper_add_auto(fastmapper_t*map,uint32_t elem)
{
    trav_v_t tr;
    if(!map)return -1;
    if(list_empty(&map->karray_list))return -1;
    tr.addr=elem;
    int id=0;
    tr.id=&id;
    list_elem_t*r= list_traversal(&map->karray_list,__fast_travel__a,&tr);
    if(!r)return -1;
    return id;
}
void fastmapper_remove(fastmapper_t*map,uint32_t id)
{
    if(!map)return 0;
    if(list_empty(&map->karray_list))return 0;
    list_elem_t*r= list_traversal(&map->karray_list,__fast_travel,id);
    if(r)
    {
        karray_t*cur_array=elem2entry(karray_t,next_tag, r);
        cur_array->elems[id%map->unit_len]=0;
        return 0;
    }
}
int fastmapper_add(fastmapper_t*map,uint32_t elem,uint32_t id)
{
    if(!map)return 0;
    if(list_empty(&map->karray_list))return 0;
    list_elem_t*r= list_traversal(&map->karray_list,__fast_travel,id);
    if(r)
    {
        karray_t*cur_array=elem2entry(karray_t,next_tag, r);
        cur_array->elems[id%map->unit_len]=elem;
        return 1;
    }else
    {
        uint32_t start_id=(id/map->unit_len)*map->unit_len;
        karray_t *n_array=alloc_array
        if(!n_array)return 0;
        n_array->id_start=start_id;
        n_array->id_end=n_array->id_start+map->unit_len;
        n_array->elems=kmalloc(map->unit_len*sizeof(uint32_t));
        if(!n_array->elems)
        {
            kfree(n_array);
            return 0;
        }
        n_array->elems[id%map->unit_len]=elem;
        list_append(&map->karray_list,&n_array->next_tag);
        return 1;
    }
}