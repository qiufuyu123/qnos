#include"kobjects/kobjs.h"
#include"kobjects/kobject.h"
#include"console.h"
#include"hardware/ata.h"
//#include"kobjects/objs/ata.h"
static kobject_t*kobj_instance;
uint32_t current_disk=0;
//DiskDevice *current_dev=0;
uint64_t get_64(uint32_t l,uint32_t r)
{
    uint64_t val=l;
    val=val<<32;
    val|=r;
    return val;
}
uint32_t kata_write(uint32_t id,uint32_t buffer,uint32_t sz,uint32_t flag)
{
    // if(!current_dev)return 0;
    // current_dev->write_raw(current_dev,id,sz,buffer);
    // return 1;
    
}
uint32_t kata_read(uint32_t id,uint32_t buffer,uint32_t sz,uint32_t flag)
{
    // if(!current_dev)return 0;
    // current_dev->read_raw(current_dev,id,sz,buffer);
    // return 1; 
    printf("raad%dsec;",sz);
    return ide_read_sectors(current_disk,sz,id,buffer);
}
uint32_t kata_open(uint32_t idx,uint32_t flag)
{
    // if(idx>=DISK_MAX_CNT)return 0;
    // current_dev=disk_dev_list[idx];
    // return 1;
    current_disk=idx;
}
kobject_operations_t kernel_atapi_op={
    .attrset=0,
    .exfunc=0,
    .open=kata_open,
    .read=kata_read,
    .write=kata_write,
};
void init_ata_obj()
{
    kobj_instance=create_kobject(0,0,&kernel_atapi_op,"obj_ata");
    //printf("allloc %x %d\n",kobj_instance,kobj_instance->type);
    //kobj_instance->ops->write(0,"aaa",0,0);
}