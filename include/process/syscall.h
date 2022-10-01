#ifndef _H_SYSCALL
#define _H_SYSCALL
#include"types.h"


enum QNSyscall
{
    SYSCALL_NOP,
    SYSCALL_PRINTF,
    SYSCALL_NR
};

void init_syscall(); 
static inline int __base_syscall(uint32_t num,uint32_t v1,uint32_t v2,uint32_t v3,uint32_t v4)
{
    int a;
    //,"d"((int)v3),"D"((int)v4)
    asm volatile("int $0x80" : "=a" (a) : "b" (num), "c" ((int)v1), "d" ((int)v2), "D" ((int)v3));
    return a;
}
//#define __BASE_SYSCALL asm volatile("int $0x80" : "=a" (a) : "0" (0), "b" ((int)1), "c" ((int)2));
//int __base_syscall(uint32_t num,uint32_t v1,uint32_t v2,uint32_t v3,uint32_t v4);
#endif