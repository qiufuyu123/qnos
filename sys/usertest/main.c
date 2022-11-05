//#include"process/syscall.h"
#include"stdio.h"
#include"usyscall.h"
#include"stdlib.h"
int main()
{
    __asm__ ("movl %%esp,%%eax\n\t" \
    "pushl $0x23\n\t" \
    "pushl %%eax\n\t" \
    "pushfl\n\t" \
    "pushl $0x1b\n\t" \
    "pushl $1f\n\t" \
    "iret\n\t" \
    "1:\tmovl $0x23,%%eax\n\t" \
    "movw %%ax,%%ds\n\t" \
    "movw %%ax,%%es\n\t" \
    "movw %%ax,%%fs\n\t" \
    "movw %%ax,%%gs" \
    :::"ax");
    int a;
    //asm volatile("int $0x80" : "=a" (a) : "a" (2), "b" ((int)4));
    char buf[20];
    printf("hello printf![%d]",233);
    printf("hello printf![%d]",332);
    exit(1);
    while(1);
    
    //printf(buf);
    // while (1)
    // {
    //     /* code */
    // }
    
    return 989;
}