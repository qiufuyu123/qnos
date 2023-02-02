#ifndef _H_UNISTD
#define _H_UNISTD

int fork();
void ls_dir(char *dir);
int exec(char *path);
void ps();
void sleep(int ms);
void meminfo(int *usedkb,int *allkb);
void wait();
int pipe(int *fd);
int dup(int fd);
int dup2(int fd,int newfd);
int brk(void *addr);
void* sbrk(int increasement);
#endif