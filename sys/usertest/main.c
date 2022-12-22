//#include"process/syscall.h"
#include"unistd.h"
#include"stdio.h"
#include"usyscall.h"
#include"string.h"
#include"stdlib.h"
#include"fcntl.h"
#include"conio.h"
void printlogo()
{
printf("   ______    __    __   ______     ______   \n");
printf(" /      \\ /  \\  /  | /      \\  /      \\ \n");
printf("/$$$$$$  |$$  \\ $$ |/$$$$$$  |/$$$$$$  |\n");
printf("$$ |  $$ |$$$  \\$$ |$$ |  $$ |$$ \\__$$/ \n");
printf("$$ |  $$ |$$$$  $$ |$$ |  $$ |$$      \\ \n");
printf("$$ |_ $$ |$$ $$ $$ |$$ |  $$ | $$$$$$  |\n");
printf("$$ / \\$$ |$$ |$$$$ |$$ \\__$$ |/  \\__$$ |\n");
printf("$$ $$ $$< $$ | $$$ |$$    $$/ $$    $$/ \n");
printf(" $$$$$$  |$$/   $$/  $$$$$$/   $$$$$$/  \n");
printf("     $$$/                               \n");
}
void testtt()
{
    char buff[100];
    memset(buff,1,100);
    __base_syscall(SYSCALL_PRINTF,"testtttt\n",0,0,0);
}
int main()
{
    __asm__ ("movl %%esp,%%eax\n\t" \
    "pushl $0x23\n\t" \
    "pushl %%eax\n\t" \
    "pushfl\n\t" \
    "pushl $0x1b\n\t" \
    "pushl $1f\n\t" \
    "iret\n\t" \
    "1:\tmovl $0x23,%%eax\n\t" \
    "movw %%ax,%%ds\n\t" \
    "movw %%ax,%%es\n\t" \
    "movw %%ax,%%fs\n\t" \
    "movw %%ax,%%gs" \
    :::"ax");
    int a;
    //asm volatile("int $0x80" : "=a" (a) : "a" (2), "b" ((int)4));
    char buf[40];
    char buff[20];
    printf("hello printf![%d];",233);
    printf("hello printf![%c];\n",'a');
    memset(buf,20,0);
    printf("doing cls...0x%x\n",clrscr);
    clrscr();
    printlogo();
    while (1)
    {
        printf(">");
        gets_s(buf,39);
        if(!strcmp(buf,"ver"))
        {
            printlogo();
            printf("\n[QNKERNEL Ver 0.3.3F Alpha 12/20/22]\n");
        }
        else if(!strcmp(buf,"cls"))
        {
            clrscr();
        }else if(!strcmp(buf,"test"))
        {
            int fd=open("/boot/setup.ini",O_RDONLY);
            
            strcpy(buff,"123");
            printf("fd is%d content:%s\n",fd,buff);
            int e=read(fd,buff,19);
            printf("fd is%d content:%s error:%d\n",fd,buff,e);
        }else if(!strcmp(buf,"test2"))
        {
            int r=fork();
            //printf("fork ret:%d\n",r);
            //__base_syscall(SYSCALL_PRINTF,"rrrrrr!\n",0,0,0);
            //testtt();
            if(r==0)
            {
                printf("[THIS IS CHILD!]\n");
                while(1);
            }else
            {
                printf("[THIS IS PARENT!]\n[CHILD IS %d]buf is 0x%x\n",r,buf);
            }
        }else if(!strcmp(buf,"ps"))
        {
            ps();
        }
        else if(!strncmp(buf,"ls",2))
        {
            ls_dir(buf+3);
        }else if(!strncmp(buf,"exe",3))
        {
            exec(buf+4);
        }
        else printf("\n[%s] <-- unknown command!\n",buf);
        //while(1);
        memset(buf,40,0);
        /* code */
    }
    
    exit(1);
    while(1);
    
    //printf(buf);
    // while (1)
    // {
    //     /* code */
    // }
    
    return 989;
}