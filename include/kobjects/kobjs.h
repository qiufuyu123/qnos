#ifndef _H_KOBJS
#define _H_KOBJS
#include"kobjects/obj_vfs.h"
//void init_kernel_obj();
void init_kernel_obj();
void init_ata_obj();
void init_stdio_obj();
void init_all_objs();
uint32_t stdin_write(struct vfs_file*file,uint32_t size,uint8_t *buffer);
vfs_file_t* create_stdin_file();
#endif