#ifndef _H_CONIO
#define _H_CONIO
#define __USER__LIB
#include"../../../include/hardware/keyboard/keyboard.h"
#include"../../../include/kobjects/kobj_ktty.h"
#include"../../../include/hardware/vga.h"
void clrscr();
char getch();
void gotoxy(int x,int y);
void textcolor(char c);
void textbackground(char c);
void conlock(char c);
void cprintf(char *str,...);
#endif