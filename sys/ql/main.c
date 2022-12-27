//#include"process/syscall.h"
#include"unistd.h"
#include"stdio.h"
#include"usyscall.h"
#include"string.h"
#include"stdlib.h"
#include"fcntl.h"
#include"conio.h"
#include"ql.h"
char membuf[1024*4];
int main()
{
    ql_ctx ctx;
	ql_init(&ctx, membuf, 1024 * 4);
    char codes[400];
    char buf[80];
    memset(buf,80,0);
    memset(codes,0,400);
   printf("QLANG VER 0.1 \n");
    while (1)
    {
        printf(">");
        while (1)
		{
            gets_s(buf,79);
            if(!strcmp(buf,"cls"))
            {
                clrscr();
                continue;
            }
            strcat(codes,buf);
			if (buf[strlen(buf) - 1] == ';')break;
            
            else if(!strcmp(buf,"exit"))
            {
                printf("Bye~!\n");
                exit(0);
            }
		}
		codes[strlen(codes) - 1] = '\0';
		//std::cout << ll;
        char*copy=codes;
		
        ql_gc_static(&ctx);
		if(ql_eval(&ctx,&copy,0)>=0)printf("result is:%d\n", ctx.last_res.addr);
        memset(buf,80,0);
        memset(codes,0,400);
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