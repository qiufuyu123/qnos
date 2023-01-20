#ifndef _H_FCNTL
#define _H_FCNTL
#define _USER_LIB
#include"../../../include/kobjects/obj_vfs.h"
#include"inttypes.h"
int close(int fd);
int open(char *path,int openflag);
int read(int fd,char *buffer,uint32_t size);
int mmap(void*starts,uint32_t length,int fd,int offset,int flag);
int write(int fd,char *buffer,uint32_t size);
int lseek(int fd,int offset,int base);
#endif