#include"unistd.h"
#include"usyscall.h"
#include"stdio.h"
int fork()
{
    int r= __base_syscall(SYSCALL_FORK,0,0,0,0);
    //printf("[!!fork ret:%d]\n",r);
    return r;
}