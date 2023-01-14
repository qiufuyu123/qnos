#ifndef _H_PIPE
#define _H_PIPE
#include"kobjects/obj_vfs.h"

vfs_file_t *create_kpipe();

int user_pipe(int *fd);
#endif