#ifndef _H_STDIO
#define _H_STDIO
#include"inttypes.h"
int sprintf(char * str, const char *fmt, ...);
int printf(char *str,...);
int scanf(char *str,...);
int read(int fd,char *buffer,int size);
char *gets(char*str);
char *gets_s(char*str,int len);
char getch();
#endif