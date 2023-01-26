#include"kobjects/kobjs.h"
#include"kobjects/kobject.h"
#include"console.h"
#include"kobjects/kobj_ktty.h"
#include"string.h"
kobject_t*ktty_instance;
int ktty_setattr(uint32_t attr,uint8_t *val)
{
    if(attr==TTY_SETXY)
    {
        Klogger->setcurse(val[0],val[1]);
        //printf("SET CURSE:%d %d",val[0],val[1]);
    }
    
}
int ktty_getattr(uint32_t attr)
{

}
kobject_operations_t ktty_obj_operate={
    .attrset=ktty_setattr,
    .attrget=ktty_getattr
};
void init_ktty_obj()
{
    ktty_instance=create_kobject(0,0,&ktty_obj_operate,"ktty");
    //printf("allloc %x %d\n",kobj_instance,kobj_instance->type);
    //kobj_instance->ops->write(0,"aaa",0,0);
}