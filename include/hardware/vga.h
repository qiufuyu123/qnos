#ifndef _H_VGA
#define _H_VGA
#include"types.h"
#include"boot/multiboot.h"
void init_vga(multiboot_info_t *mbi);
extern uint32_t vga_width,vga_height;

#endif