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
int main(int argc,char *argv[])
{
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
        //while(1);
        if(!strcmp(buf,"ver"))
        {
            printlogo();
            printf("\n[QNKERNEL Ver 0.3.10A 01/20/23]\n");
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
        }else if(!strcmp(buf,"test3"))
        {
            int fd=open("/dev/fb0",O_RDWR); //打开 framebuffer设备文件
            printf("OPEN FB0:%d\n",fd);     
            mmap(0x90000000,10,fd,0x138000,0);
            //将文件内容映射到当前用户内存中
            //(framebuffer即 显存设备，内核会处理)

            memset(0x90000000,0xff,4096*10);
            //填充映射的空间
            //即 填充一部分显存
        }
        else if(!strcmp(buf,"test4"))
        {
            gotoxy(0,0);
        }
        else if(!strcmp(buf,"test6"))
        {
            textcolor(GREEN);
            printf("ARGC:%d\n",argc);
            for (int i = 0; i < argc; i++)
            {
                printf("[ARG %d]: %s \n",i+1,argv[i]);
            }
            textcolor(WHITE);
        }
        else if(!strcmp(buf,"test7"))
        {
            // uint32_t old= sbrk(4096);
            // printf("old prg brk:0x%x\n",old);
            // brk(old);
            // printf("free 4096 bytes!");
            char *test=malloc(4096);
            memcpy(test,"123123",7);
            printf("FROM MALLOC:%x %s\n",test,test);
            free(test);
        }
        else if(!strcmp(buf,"test5"))
        {
            //printf("argv[0]:%s\n",argv[0]);
            while (1)
            {
                char c=getch();
                if(!c)
                {
                    c=getch();
                    //printf("0x%x",c);
                    if(c==KEY_UP)
                    {
                        printf("PGUP");
                    }else if(c==KEY_DOWN)
                    {
                        printf("PGDOWN");
                    }else if(c==KEY_LEFT)
                        printf("LEFT");
                    else if(c==KEY_RIGHT)
                        printf("RIGHT");
                }
            }
            
        }
        else if(!strcmp(buf,"exit"))
        {
            printf("Bye~\n");
            exit(1);
        }
        else if(!strcmp(buf,"ps"))
        {
            ps();
        }
        else if(!strncmp(buf,"ls",2))
        {
            ls_dir(buf+3);
        }
        else if(!strncmp(buf,"new",3))
        {
            char *fname=&buf[4];
            int fd=open(fname,O_CREAT|O_RDWR);
            write(fd,"123123",6);
            close(fd);

        }
        else if(!strncmp(buf,"pipe",4))
        {
            int test_pipe[2];
            if(pipe(&test_pipe)<0)
            {
                printf("OpenPipeFail!\n");
            }else
            {
                printf("PIPE WRITE:%d\n",write(1,"TEST WRITE TO kernel!\n",23));
                int child=fork();
                
                if(child==0)
                {
                    char pbuf[20]={0};
                    while (1)
                    {
                        if(read(test_pipe[0],pbuf,5))
                        {
                            printf("[CHILD]: Read 5 bytes from pipe: %s\n",pbuf);
                            memset(pbuf,0,20);
                        }

                    }
                    
                }
                write(test_pipe[1],"123456",6);
                write(test_pipe[1],"7890",4);
                // close(test_pipe[0]);
                // close(test_pipe[1]);
            }

            
        }
        else if(!strncmp(buf,"exe",3))
        {
            char is_wait=1;
            if(buf[strlen(buf)-1]=='&')
            {
                buf[strlen(buf)-1]='\0';
                is_wait=0;
            }
            if(fork()==0)
            {
                printf("FORK A THREAD FOR EXECUTE!\n");
                exec(buf+4);
                //while(1);//This line will never be executed!
            }
            if(is_wait)wait();
        }
        else if(!strncmp(buf,"read",4))
        {
            char *fname=&buf[5];
            int fd=open(fname,O_RDONLY);
            printf("[SHELL]:Read: %s , fd:%d\n",fname,fd);
            char buff[100];
            int e=read(fd,buff,100);
            buff[99]='\0';
            printf("%s\n[READ OK:%d]\n",buff,e);
            e=close(fd);
            printf("[READ CLOSE:%d %d]\n",e,fd);
        }else if(!strncmp(buf,"write",5))
        {
            char *fname=&buf[6];
            int fd=open(fname,O_RDWR);
            printf("[SHELL]:Write: %s , fd:%d\n",fname,fd);
            lseek(fd,0,SEEK_END);
            char buff[100];
            
            memcpy(buff,"THIS IS TEST SYS_WRITE!!\n",25);
            int e=write(fd,buff,25);
            //printf("%s\n[WRITE OK:%d]\n",buf,e);
            e=close(fd);
            printf("[READ CLOSE:%d %d]\n",e,fd);
        }
        else 
        {
            memset(buf,40,0);
        }
        //printf("\n[%s] <-- unknown command!\n",buf);
        //while(1);
       
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