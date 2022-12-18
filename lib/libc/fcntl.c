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