#ifndef _H_STDIO
#define _H_STDIO
#include"inttypes.h"
#include"fcntl.h"
int sprintf(char * str, const char *fmt, ...);
int printf(char *str,...);
int scanf(char *str,...);
char *gets(char*str);
char *gets_s(char*str,int len);
char getchar();
#endif