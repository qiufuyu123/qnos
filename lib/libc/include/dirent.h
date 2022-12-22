#ifndef _H_DIRENT
#define _H_DIRENT
#include"sys/types.h"
typedef uint32_t DIR;
struct dirent
{
    uint32_t d_ino;
    char name[NAME_MAX+1];
};
DIR *opendir(const char *path);
int readdir(DIR *dir);
#endif