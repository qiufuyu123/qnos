#ifndef _H_SYSCALL
#define _H_SYSCALL
#ifndef __USER__LIB
#include"types.h"
#endif


enum QNSyscall
{
    SYSCALL_NOP=0,
    SYSCALL_PRINTF,
    SYSCALL_EXIT,
    SYSCALL_GETCH,
    SYSCALL_GETS,
    SYSCALL_READ,
    SYSCALL_CLRSCR,
    SYSCALL_NR
};
#ifndef __USER__LIB
void init_syscall(); 
static inline int __base_syscall(uint32_t num,uint32_t v1,uint32_t v2,uint32_t v3,uint32_t v4)
{
    int a;
    //,"d"((int)v3),"D"((int)v4)
    asm volatile("int $0x80" : "=a" (a) : "a" (num), "b" ((int)v1), "c" ((int)v2), "d" ((int)v3),"D"((int)v4));
    return a;
}
#endif
//#define __BASE_SYSCALL asm volatile("int $0x80" : "=a" (a) : "0" (0), "b" ((int)1), "c" ((int)2));
//int __base_syscall(uint32_t num,uint32_t v1,uint32_t v2,uint32_t v3,uint32_t v4);
#endif