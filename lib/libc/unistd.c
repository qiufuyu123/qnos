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