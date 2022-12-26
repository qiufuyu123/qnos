#include"unistd.h"
#include"usyscall.h"
#include"stdio.h"
int fork()
{
    int r= __base_syscall(SYSCALL_FORK,0,0,0,0);
    //printf("[!!fork ret:%d]\n",r);
    return r;
}
void ls_dir(char *dir)
{
    __base_syscall(SYSCALL_TEST_LIST_DIR,dir,0,0,0);
}
void exec(char *path)
{
    __base_syscall(SYSCALL_EXEC,path,0,0,0);
}
void ps()
{
    __base_syscall(SYSCALL_PS,0,0,0,0);
}
void sleep(int ms)
{
    __base_syscall(SYSCALL_SLEEP,ms,0,0,0);
}
void meminfo(int *usedkb,int *allkb)
{
    __base_syscall(SYSCALL_MEMINFO,usedkb,allkb,0,0);
}
void wait()
{
    __base_syscall(SYSCALL_WAIT,0,0,0,0);
}