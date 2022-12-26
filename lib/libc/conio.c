#include"conio.h"
#include"usyscall.h"
#include"stdio.h"
void clrscr()
{
    //printf("doing clsscr");
    __base_syscall(SYSCALL_CLRSCR,0,0,0,0);
}