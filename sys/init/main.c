//#include"process/syscall.h"
#include"unistd.h"
#include"stdio.h"
#include"usyscall.h"
#include"string.h"
#include"stdlib.h"
#include"fcntl.h"
#include"conio.h"

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
    printf("Load in init\n");
    sleep(2000);
    int e=fork();
    if(e==0)
    {
        exec("/boot/sys/usertest.elf");
        while (1)
        {
            /* code */
        }
    }
    wait();
}