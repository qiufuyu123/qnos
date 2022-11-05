#ifndef _H_U_SYSCALL
#define _H_U_SYSCALL
#include"inttypes.h"
#define __USER__LIB
#include"../../../include/process/syscall.h"
static inline int __base_syscall(uint32_t num,uint32_t v1,uint32_t v2,uint32_t v3,uint32_t v4)
{
    int a;
    //,"d"((int)v3),"D"((int)v4)
    asm volatile("int $0x80" : "=a" (a) : "a" (num), "b" ((int)v1), "c" ((int)v2), "d" ((int)v3),"D"((int)v4));
    return a;
}
#endif