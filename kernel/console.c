#include"console.h"
#include"mem/memorylayout.h"
#include"io.h"
#include"string.h"
#include"kobjects/obj_serial.h"
char *video_buffer=(char*)TEXT_VIDEO_BUFFER;
int cx=0,cy=0;
char bg=0,fg=Vga_White;
#define VGA_WIDTH 80
#define VGA_HEITGH 25
kconsole_t *Klogger;
void update_cursor(int x, int y)
{
	uint16_t pos = y * VGA_WIDTH + x;
 
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}
void flush_cursor()
{
    update_cursor(cx,cy);
}
void init()
{
    //update_cursor(cx,cy);
    clear();
}
void pause()
{

}
void putchar(char c)
{
    if(cx>=80)
    {
        cy++;
        cx=0;
    }
    if(cy>=25)
    {
        cy=0;
        clear();
    }
    if(c=='\r')
    {
        cx=0;
    }else if(c=='\b')
    {
        if(cx==0)
        {
            if(cy!=0)cy--;
        }else cx--;
    }
    else if(c=='\n')
    {
        cy++;
        cx=0;
        if(cy>=25)clear();
    }else
    {
    video_buffer[(cy*VGA_WIDTH+cx)*2]=c;
    video_buffer[(cy*VGA_WIDTH+cx)*2+1]=((bg<<4)|fg);
    write_serial(c);
    cx++;
    }
    flush_cursor();
}
void putstr(char *str)
{
    while (*str)
    {
        putchar(*str);
        str++;
    }
    
}

void setcolor(char bgc,char fgc)
{
    bg=bgc;
    fg=fgc;
}
void setcurse(int x,int y)
{
    cx=x;
    cy=y;
    flush_cursor();
}
void clear()
{
    memset(video_buffer,0,VGA_HEITGH*VGA_WIDTH*2);
    cx=cy=0;
    flush_cursor();
}
kconsole_t default_logger={
    .id=1,
    .init=init,
    .pause=pause,
    .putchr=putchar,
    .putstr=putstr,
    .setcolor=setcolor,
    .setcurse=setcurse,
    .cls=clear,
    .frame_buffer=0xb8000
};
