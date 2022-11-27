//#include"process/syscall.h"
#include"stdio.h"
#include"usyscall.h"
#include"string.h"
#include"stdlib.h"
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
    char buf[20];
    printf("hello printf![%d];",233);
    printf("hello printf![%c];\n",'a');
    memset(buf,20,0);
    clrscr();
    printlogo();
    while (1)
    {
        printf(">");
        gets_s(buf,19);
        if(!strcmp(buf,"ver"))
        {
            printlogo();
            printf("\n[QNKERNEL Ver 0.1.1U Alpha 11/17/22]\n");
        }
        else if(!strcmp(buf,"cls"))
        {
            clrscr();
        }
        else printf("\n[%s] <-- unknown command!\n",buf);
        //while(1);
        memset(buf,20,0);
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