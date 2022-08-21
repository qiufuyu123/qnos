#include"kobjects/kobjs.h"
#include"kobjects/kobject.h"
#include"console.h"
kobject_t*kobj_instance;
uint32_t kobj_write(uint32_t id,uint32_t buffer,uint32_t sz,uint32_t flag)
{
    Klogger->putstr(buffer);
    //printf("%s",buffer);
}
uint32_t kobj_read(uint32_t id,uint32_t buffer,uint32_t sz,uint32_t flag)
{
    
}
kobject_operations_t kernel_obj_op={
    .attrset=0,
    .exfunc=0,
    .open=0,
    .read=kobj_read,
    .write=kobj_write,
};
void init_kernel_obj()
{
    kobj_instance=create_kobject(0,0,&kernel_obj_op,"obj_kernel");
    //printf("allloc %x %d\n",kobj_instance,kobj_instance->type);
    //kobj_instance->ops->write(0,"aaa",0,0);
}