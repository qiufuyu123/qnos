#include"stdlib.h"
#include"usyscall.h"
#include"stdlib.h"
void exit(int status)
{
    __base_syscall(SYSCALL_EXIT,0,0,0,0);
}