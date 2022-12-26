//#include"process/syscall.h"
#include"unistd.h"
#include"stdio.h"
#include"usyscall.h"
#include"string.h"
#include"stdlib.h"
#include"fcntl.h"
#include"sys/types.h"
#include"conio.h"
uint32_t* vbuff;
uint32_t vb_pgcnt=0;
uint32_t width,height;
int vfg=0xffffffff,vbg=0x00000000;
uint64_t FONTS[256];
#define ngx_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
void vga_put_pixel(uint32_t x,uint32_t y,uint32_t color)
{
    if (width <= x ||  height <= y)
        return;
        
    vbuff[ width * y + x] = color;
}
void vga_putchar(char ch, uint32_t x, uint32_t y) {
    uint32_t px = 0;  // position in the charactor - an 8x8 font = 64 bits
    uint64_t bCh = FONTS[ch];
    
    // check if it will be drawn off screen
    //if (x+8 < 0 || x > vga_width || y+8 < 0 || y > vga_height)
    //    return;
    
    // test if the charactor will be clipped (will it be fully in the screen or partially)
    if (x >= 0 && x+8 < width && y >= 0 && y+8 < height) {
        // fully in the screen - pre calculate some of the values
        // so there is less going on in the loop
        int i = width * (y - 1) + x + 8;
        int incAmount = width - 8;
        for (int yy = 7; yy >= 0; yy-- ) {
            i += incAmount;
            for (int xx = 7; xx >= 0; xx-- ) {
                // test if a pixel is drawn here
                if ((bCh >> px++) & 1) {
                    //printf("pixel!");
                    vbuff[i] = vfg;
                }else
                {
                    vbuff[i]=vbg;
                }
                i++;
            }
        } 
    } else {
        // partially in the screen
        int xpos = 0;
        int i = width* (y - 1);    
        for (int yy = 0; yy < 8; yy++ ) {
            i += width;
            xpos = x;
            for (int xx = 7; xx >= 0; xx-- ) {
                // test if a pixel is drawn here
                if ((bCh >> px++) & 1) {
                    //printf("pixel!");
                    // test if the pixel will be off screen
                    if (xpos > 0 && xpos < width && yy+y > 0 && yy+y < height)
                        vbuff[ i + xpos] = vfg;
                }else vbuff[i]=vbg;
                xpos++;
            }
        } 
    }
}
void put_line(char *str,int x,int y)
{
    while (*str)
    {
        vga_putchar(*str,x,y);
        str++;
        x+=8;       
    }
    
}
int main()
{
    int fd2=open("/dev/kbd0",O_RDONLY);
    int fd=open("/dev/fb0",O_RDWR); //打开 framebuffer设备文件
    framebuffer_cfg_t cfg;
    int r= read(fd,&cfg,sizeof(framebuffer_cfg_t));
    read(fd,FONTS,sizeof(uint64_t)*256);
    printf("OPEN FB0:%d W:%d H:%d %d\n",fd,cfg.width,cfg.height,r); 
    
    uint32_t based=cfg.width*cfg.height*4-3*8*cfg.width*4; //最底下的3行字符扣掉
    uint32_t v2= based&0xfffff000;
    uint32_t offset=based-v2;
    vb_pgcnt=ngx_align(3*8*cfg.width*4,4096)/4096;
    width=cfg.width;
    height=cfg.height;
    mmap(0x90000000,vb_pgcnt,fd,v2,0);
    vfg=0x00000000;
    vbg=0xffffffff;
    vbuff=0x90000000+offset;
    //将文件内容映射到当前用户内存中
    //(framebuffer即 显存设备，内核会处理)
    height=3*8;
    
    char old_c=0;
    int c=0;
    char sbuf[100];
    while (1)
    {
        sleep(1000);
        c++;
        memset(sbuf,0,100);
        int v1,v2;
        meminfo(&v1,&v2);
        sprintf(sbuf,"BAR RUNNING IN %ds MEM:[%d/%d(kb) %%%d used]",c,v1,v2,(int)v1*100/v2);
        memset(vbuff,0xff,width*3*4*8);
        vfg=0x000000;
        put_line("HELLO BAR!",3,3);
        vfg=0xFF69B4;
        put_line(sbuf,11*8+3+8,3);
    }
    
    // vga_putchar('E',0+8,5);
    // vga_putchar('L',0+16,5);
    // vga_putchar('L',0+24,5);
    // vga_putchar('O',0+32,5);
    
    printf("THIS IS BAR!\n");
    while(1);

}