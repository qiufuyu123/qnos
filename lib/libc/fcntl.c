#include"fcntl.h"
#include"usyscall.h"
#include"inttypes.h"
int close(int fd)
{
    return __base_syscall(SYSCALL_FOP,FOP_CLOSE,fd,0,0);
}
int open(char *path,int openflag)
{
    return __base_syscall(SYSCALL_FOP,FOP_OPEN,path,openflag,0);
}
int read(int fd,char *buffer,uint32_t size)
{
    return __base_syscall(SYSCALL_FOP,FOP_READ,fd,buffer,size);
}
int write(int fd,char *buffer,uint32_t size)
{
    return __base_syscall(SYSCALL_FOP,FOP_WRITE,fd,buffer,size);
}
int lseek(int fd,int offset,int base)
{
    return __base_syscall(SYSCALL_FOP,FOP_SEEK,fd,offset,base);
}
int mmap(void*starts,uint32_t length,int fd,int offset,int flag)
{
    syscall_extra_args arg;
    arg.v1=fd;
    arg.v2=offset;
    arg.v3=flag;
    return __base_syscall(SYSCALL_FOP,FOP_MMAP,starts,length,&arg);
}