#include"fcntl.h"
#include"usyscall.h"
#include"inttypes.h"
int open(char *path,int openflag)
{
    __base_syscall(SYSCALL_FOP,FOP_OPEN,path,openflag,0);
}
int read(int fd,char *buffer,uint32_t size)
{
    __base_syscall(SYSCALL_FOP,FOP_READ,fd,buffer,size);
}
int mmap(void*starts,uint32_t length,int fd,int offset,int flag)
{
    syscall_extra_args arg;
    arg.v1=fd;
    arg.v2=offset;
    arg.v3=flag;
    return __base_syscall(SYSCALL_FOP,FOP_MMAP,starts,length,&arg);
}