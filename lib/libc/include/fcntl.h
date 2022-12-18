#ifndef _H_FCNTL
#define _H_FCNTL
#define _USER_LIB
#include"../../../include/kobjects/obj_vfs.h"
#include"inttypes.h"
int open(char *path,int openflag);
int read(int fd,char *buffer,uint32_t size);
#endif