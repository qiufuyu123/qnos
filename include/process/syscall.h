#ifndef _H_SYSCALL
#define _H_SYSCALL
#ifndef __USER__LIB
#include"types.h"
#endif
typedef struct 
{
    int v1;
    int v2;
    int v3;
    int v4;
}syscall_extra_args;

enum QNFileOperate
{
    FOP_OPEN,
    FOP_READ,
    FOP_CLOSE,
    FOP_MMAP,
    FOP_WRITE,
    FOP_SEEK,
};
enum QNModuleOperate
{
    MOP_GET,
    MOP_SETATTR,
    MOP_GETATTR
};
enum QNSyscall
{
    SYSCALL_NOP=0,
    SYSCALL_PRINTF,
    SYSCALL_EXIT,
    SYSCALL_GETCH,
    SYSCALL_GETS,
    SYSCALL_FOP,
    SYSCALL_CLRSCR,
    SYSCALL_FORK,
    SYSCALL_TEST_LIST_DIR,
    SYSCALL_EXEC,
    SYSCALL_PS,
    SYSCALL_SLEEP,
    SYSCALL_MEMINFO,
    SYSCALL_WAIT,
    SYSCALL_PIPE,
    SYSCALL_DUP,
    SYSCALL_MOP,
    SYSCALL_BRK,
    SYSCALL_SBRK,
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