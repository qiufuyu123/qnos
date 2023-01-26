#include"qnos.h"
#include"usyscall.h"
khandle_t qnkobject_get(char *name)
{
    return __base_syscall(SYSCALL_MOP,MOP_GET,name,0,0);
}

int qnkobject_setattr(khandle_t obj,uint32_t attr,uint32_t val)
{
    return __base_syscall(SYSCALL_MOP,MOP_SETATTR,obj,attr,val);
}

int qnkobject_getattr(khandle_t obj,uint32_t attr)
{
    return __base_syscall(SYSCALL_MOP,MOP_GETATTR,obj,attr,0);
}