#ifndef _H_VGA
#define _H_VGA

enum CONSOLE_COLOR
{
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHT_GREY,
    DARK_GREY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    LIGHT_BROWN,
    WHITE
};
#ifndef __USER__LIB
#include"types.h"
#include"boot/multiboot.h"

void init_vga(multiboot_info_t *mbi);
extern uint32_t vga_width,vga_height;
void vga_enable_doubled();
void vga_setbgcolor_16(char c);
void vga_setftcolor_16(char c);
#endif
#endif