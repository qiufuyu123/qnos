
#include"hardware/vga.h"
#include"console.h"
#include"boot/multiboot.h"
#include"string.h"
#include"mem/memorylayout.h"
#include"mem/malloc.h"
#include"io.h"
int color_16[16]={
    0x000000,
    0x0000AA,
    0X00AA00,
    0X00AAAA,
    0XAA0000,
    0XAA00AA,
    0xAA5500,
    0xAAAAAA,
    0x555555,
    0x5555FF,
    0x55FF55,
    0X55ffff,
    0xff5555,
    0xff55ff,
    0xffff55,
    0xffffff
};
/**
 * @brief NOTE
 * Anyway....
 * In fact, I dont want to set up vga at kernel mode
 * I once wanted to set up ascii mode (0xb8000) to printf in kernel 
 * and then, set up vga mode when we switch to user or use kernel modules
 * But...
 * I found it really difficult to switch to vesa/vga mode in x86 protected mode
 * So i just let it be inited in grub mode
 *  
 * TODO: I hope one day I will find out the way ....
 * @author: qiufuyu
 */
#define CHAR_W 8
#define CHAR_H 8
#include"font256.inc"
#define USE_VGA
uint32_t cur_x,cur_y;
//uint32_t max_text_x,max_text_y;
uint32_t vga_width,vga_height;
uint32_t* vga_buffer;
uint32_t* vga_double_buff;
char is_doubled=0;
char buffer_state=0;
int vbg=0x000000,vfg=0xffffff;

void vga_flush_buffer()
{
    if(buffer_state)
    {
        memcpy(vga_buffer,vga_double_buff,vga_width*vga_height*4);
        buffer_state=0;
    }

}
void vga_put_pixel(uint32_t x,uint32_t y,uint32_t color)
{
    if (vga_width <= x ||  vga_height <= y)
        return;
    vga_buffer[ vga_width * y + x] = color;
}
void vga_putchar(char ch, uint32_t x, uint32_t y, uint32_t color) {
    #ifdef USE_VGA
    uint32_t px = 0;  // position in the charactor - an 8x8 font = 64 bits
    uint64_t bCh = FONT[ch];
    
    // check if it will be drawn off screen
    //if (x+8 < 0 || x > vga_width || y+8 < 0 || y > vga_height)
    //    return;
    
    // test if the charactor will be clipped (will it be fully in the screen or partially)
    if (x >= 0 && x+8 <= vga_width && y >= 0 && y+8 <= vga_height) {
        // fully in the screen - pre calculate some of the values
        // so there is less going on in the loop
        int i = vga_width * (y - 1) + x + 8;
        int incAmount = vga_width - 8;
        for (int yy = 7; yy >= 0; yy-- ) {
            i += incAmount;
            for (int xx = 7; xx >= 0; xx-- ) {
                // test if a pixel is drawn here
                if ((bCh >> px++) & 1) {
                    vga_buffer[i] = vfg;
                }else
                {
                    vga_buffer[i]=vbg;
                }
                i++;
            }
        } 
    } else {
        // partially in the screen
        int xpos = 0;
        int i = vga_width* (y - 1);    
        for (int yy = 0; yy < 8; yy++ ) {
            i += vga_width;
            xpos = x;
            for (int xx = 7; xx >= 0; xx-- ) {
                // test if a pixel is drawn here
                if ((bCh >> px++) & 1) {
                    // test if the pixel will be off screen
                    if (xpos > 0 && xpos < vga_width && yy+y > 0 && yy+y < vga_height)
                        vga_buffer[ i + xpos] = vfg;
                }else vga_buffer[i]=vbg;
                xpos++;
            }
        } 
    }
    #endif
}
void vga_scroll_up()
{
    //Prevent scrolling while inputing(printing)...  
    memcpy(vga_buffer,vga_buffer+vga_width*CHAR_H,(vga_height-CHAR_H)*vga_width*4);
    memset(vga_buffer+vga_width*(vga_height-CHAR_H),0,vga_width*CHAR_H*4);
}
void vga_gen_puchar(char ch)
{
        if(ch=='\r')
        {
            cur_x=0;
            return;
        }else if(ch=='\b')
        {
            if(cur_x>0)
                cur_x-=CHAR_W;
            else if(cur_y>0)
            {
                cur_x=(vga_width/CHAR_W)*CHAR_W;
                cur_y-CHAR_H;
            }
            
            vga_putchar(' ',cur_x,cur_y,0xffffffff);
            return;
        }else if(ch=='\n')
        {
            _IO_ATOMIC_IN
            cur_y+=CHAR_H;
            if(cur_y>=vga_height)
            {
                cur_y-=CHAR_H;
                vga_scroll_up();
            }
            cur_x=0;
            _IO_ATOMIC_OUT
            return;
        }else vga_putchar(ch,cur_x,cur_y,0xffffffff);
        cur_x+=CHAR_W;
        if(cur_x>=vga_width)
        {
            cur_x=0;
            cur_y+=CHAR_H;
        }
        if(cur_y>=vga_height)
        {
            int flg=load_eflags();
            cli();
            cur_x=0;
            cur_y-=CHAR_H;
            vga_scroll_up();
            store_eflags(flg);    
        }
}
void vga_printstr(char *str)
{
    while (*str)
    {
        vga_gen_puchar(*str);
        str++;
    }
    //vga_flush_buffer();
}
void vga_cls()
{
    #ifdef USE_VGA
    memset(vga_buffer,0,vga_width*vga_height*4);
    vga_flush_buffer();
    // for (uint32_t i = 0; i < vga_width*vga_height; i++)
    // {
    //     vga_buffer[i]=0;
    // }
    #endif
    cur_x=cur_y=0;
}
void vga_updata()
{
    vga_buffer=Klogger->frame_buffer;
}
void vga_setcolor(int bbg,int ffg)
{
    vbg=bbg;
    vfg=ffg;
}
void vga_setcurse(uint8_t x,uint8_t y)
{
    cur_x=x*CHAR_W;
    cur_y=y*CHAR_H;
    if(cur_x>=vga_width || cur_y>=vga_height)
    {
        cur_x=cur_y=0;
    }
}
kconsole_t vga_console={
    .cls=vga_cls,
    .id=1,
    .init=init_vga,
    .pause=0,
    .putchr=vga_gen_puchar,
    .flush=vga_flush_buffer,
    .putstr=vga_printstr,
    .setcolor=vga_setcolor,
    .setcurse=vga_setcurse,
    .update=vga_updata
};
void vga_enable_doubled()
{
    vga_double_buff=kmalloc_page(vga_console.page_cnt);
    if(!vga_double_buff)
    {
        PANIC("Cannot Alloc MemBuffer For VGA!\n(Double-Buffered Required!)\n");
    }
    is_doubled=1;
}
void init_vga(multiboot_info_t *mbi)
{
    vga_width=mbi->framebuffer_width;
    vga_height=mbi->framebuffer_height;
    vga_buffer=mbi->framebuffer_addr;
    memset(vga_buffer,0,vga_width*vga_height*4);
    // for (uint32_t i = 0; i < vga_width*vga_height; i++)
    // {
    //     vga_buffer[i]=0;
    // }
    //max_text_x=vga_width/8;
    //max_text_y=vga_height/8;
    cur_x=cur_y=0;
    vga_console.frame_buffer=vga_buffer;
    vga_console.page_cnt=ngx_align(vga_width*vga_height*4,4096)/4096;
    // vga_buffer=kmalloc_page(vga_console.page_cnt);
    // if(!vga_buffer)
    // {
    //     PANIC("Cannot Alloc MemBuffer For VGA!\n(Double-Buffered Required!)\n");
    // }

    Klogger=&vga_console;
    //init_printlock();
    printf("[VGA WIDTH:%d HEIGHT:%d PGNUM:%d]\n",vga_width,vga_height,vga_console.page_cnt);
    //vga_gen_puchar('h');
    //vga_gen_puchar('e');
    //vga_printstr("hello vga!\n");
    //while(1);
}
void vga_setbgcolor_16(char c)
{
    if(c<16)
        vbg=color_16[c];
}
void vga_setftcolor_16(char c)
{
    if(c<16)
        vfg=color_16[c];
}