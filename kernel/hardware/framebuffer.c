#include"hardware/framebuffer.h"
#include"hardware/vga.h"
#include"console.h"
#include"string.h"
int fbuf_write(uint32_t addr,uint32_t num,char *buffer,int flag)
{
     
}
int fbuf_read(uint32_t addr,uint32_t num,char *buffer,int flag)
{
    if(flag==FMB_CFG)
    {
        if(num==sizeof(framebuffer_cfg_t))
        {
            framebuffer_cfg_t*b=buffer;
            b->channel=32;
            b->width=vga_width;
            b->height=vga_height;
            return 1;
        }
    }else if(flag==FMB_FRESH)
    {
        
    }
    return 0;
}
kdevice_t* device_create_framebuffer(int id)
{

}