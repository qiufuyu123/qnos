/**
 * @file console.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief A common interface for kernel mode console line
 * @version 0.1
 * @date 2022-07-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _H_CONSOLE
#define _H_CONSOLE
#include"types.h"
typedef struct kconsole
{
    void(*init)();
    void(*update)();
    void(*pause)();
    void(*putstr)(char *str);
    void(*putchr)(char c);
    void(*setcolor)(int bg,int fg);
    void(*setcurse)(int x,int y);
    void(*drawcurse)(int x,int y);
    int id;
    void(*cls)();
    uint32_t frame_buffer;
    uint32_t page_cnt;
}kconsole_t;
enum VgaTextColor
{
    Vga_Black,
    Vga_Blue,
    Vga_Green,
    Vga_Cyan,
    Vga_Red,
    Vga_Magenta,
    Vga_Brown,
    Vga_Gray,
    Vga_DarkGray,
    Vga_BrightBlue,
    Vga_BrightGreen,
    Vga_BrightCyan,
    Vga_BrightRed,
    Vga_BrightMagenta,
    Vga_Yellow,
    Vga_White  
};
extern kconsole_t *Klogger;
void init_printlock();
void printf(const char *s, ...);
int sprintf(char * str, const char *fmt, ...);
void printhex(uint32_t *vaddr,uint32_t num);
#define ASSERT(expr,info) if(!(expr)){printf("PANIC:ASSERT ERROR!\nINFO %s\n",info);while(1);}
#define ASSERT(expr) if(!(expr)){printf("PANIC:ASSERT ERROR!\n");while(1);}
#define PANIC(info)do{printf(info);while(1);}while(0)
#endif