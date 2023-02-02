//Start Your Codes here:
#include"conio.h"
#include"stdio.h"
#include"stdlib.h"
#include"tui.h"
#include"string.h"
#define FROM_00 clrscr();gotoxy(0,0);
#define printf cprintf
void show_window(int x,int y,char border_color)
{
    gotoxy(x,y);
    textbackground(border_color);
    printf(" %s \n",tui_80blank);
    for (int i = 0; i < 23; i++)
    {
        printf(" ");
        textbackground(BLACK);
        printf("%s",tui_80blank);
        textbackground(border_color);
        printf(" \n");
    }
    printf("%c%s%c",' ',tui_80blank,' ');
    //80 * 25
    textbackground(BLACK);
}
int show_mid_title(char *str,int left,int w,int h)
{
    int len=strlen(str);
    int start=left+(w-len)/2;
    gotoxy(start,h);
    printf("%s",str);
    return start;
}
void show_welcome_btn(int start,int idx)
{
    gotoxy(start,9);
    if(idx==0)
    {
        textcolor(LIGHT_BROWN);
        printf(" >[ABOUT]< ");
        textcolor(WHITE);
        printf("[OPEN][EXIT]");
    }else if(idx==1)
    {
        printf("[ABOUT]");
        textcolor(LIGHT_BROWN);
        printf(" >[OPEN]< ");
        textcolor(WHITE);
        printf("[EXIT]");
    }else
    {
        printf("[ABOUT][OPEN]");
        textcolor(LIGHT_BROWN);
        printf(" >[EXIT]< ");
        textcolor(WHITE);
    }
    
}
void show_str_win(char *str,int y)
{
    gotoxy(1,y);
    printf("%s",str);
}
void do_select(int idx)
{
    if(idx==0)
    {
        //About
        show_window(0,25,BLUE);
        show_str_win("Hello!",26);
        show_str_win("About EDIT:",27);
        show_str_win("Author: qiufuyu",28);
        show_str_win("Email: qiufuyuTony@outlook.com",29);
    }
}

void show_welcome_pg()
{
    clrscr();
    show_window(0,0,LIGHT_GREEN);
    
    show_mid_title("Welcome to use EDIT !",0,80,5);
    int same=show_mid_title("USE [LEFT] [RIGHT] Arrow and [ENTER]to select:",0,80,7);
    char selected=0;
    show_welcome_btn(same,selected);
    while (1)
    {
        char c=getch();
        if(!c)
        {
            c=getch();
            if(c==KEY_LEFT)
            {
                if(!selected)
                    selected=2;
                else
                    selected--;
            }else if(c==KEY_RIGHT)
            {
                if(selected==2)
                    selected=0;
                else
                    selected++;
            }
            show_welcome_btn(same,selected);
        }else
        {
            if(c=='\n')
            {
                if(!selected)
                    do_select(selected);
                else 
                    break;
            }
        }
    }
    
}
int main()
{
    //clrscr();
    show_welcome_pg();
    exit(1);
}