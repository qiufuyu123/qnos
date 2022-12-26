#ifndef _H_UNISTD
#define _H_UNISTD

int fork();
void ls_dir(char *dir);
void exec(char *path);
void ps();
void sleep(int ms);
void meminfo(int *usedkb,int *allkb);
void wait();
#endif