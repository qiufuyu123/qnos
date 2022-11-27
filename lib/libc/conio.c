#include"conio.h"
#include"usyscall.h"
void clrscr()
{
    __base_syscall(SYSCALL_CLRSCR,0,0,0,0);
}