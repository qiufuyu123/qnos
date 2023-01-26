#include"conio.h"
#include"usyscall.h"
#include"stdio.h"
#include"qnos.h"
static khandle_t cur_console;
uint32_t chk_obj()
{
    if(!cur_console)
    {
        cur_console=qnkobject_get("ktty");
    }
    return cur_console;
}
#define CHKOBJ(x) if(!chk_obj()){x;}
void clrscr()
{
    //printf("doing clsscr");
    __base_syscall(SYSCALL_CLRSCR,0,0,0,0);
}
void gotoxy(int x,int y)
{
    CHKOBJ(return)
    uint8_t curse[2];
    curse[0]=x;
    curse[1]=y;
    qnkobject_setattr(cur_console,TTY_SETXY,&curse[0]);
}