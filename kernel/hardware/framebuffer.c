#include"hardware/framebuffer.h"
#include"hardware/vga.h"
#include"console.h"
#include"string.h"
#include"process/task.h"
#include"mem/vmm.h"
extern uint64_t FONT[256];
int fbuf_write(kdevice_t*self,uint32_t addr,uint32_t num,char *buffer,int flag)
{
     
}
int fbuf_read(kdevice_t*self, uint32_t addr,uint32_t num,char *buffer,int flag)
{
    printf("frambuffer reading! %d %d\n",num,sizeof(framebuffer_cfg_t));
    if(num==sizeof(framebuffer_cfg_t))
    {
        framebuffer_cfg_t*b=buffer;
        b->channel=32;
        b->width=vga_width;
        b->height=vga_height;
        return 1;
    }else if(num==256*sizeof(uint64_t))
    {
        printf("copying the fonts!\n");
        memcpy(buffer,FONT,256*sizeof(uint64_t));
        return 1;
    }
    return 0;
}
int fb_mmap(struct kdevice*self, void*starts,uint32_t length,uint32_t offset,int flag)
{
    //printf("in fb_mmap 0ffset:0x%x",offset);
    vmm_doublemap_pages(get_running_progress()->pdt_vaddr,Klogger->frame_buffer+offset,length,starts);
}
kdevice_ops_t fb_ops=
{
    .read=fbuf_read,
    .write=fbuf_write,
    .mmap=fb_mmap
};

kdevice_t* device_create_framebuffer(int id)
{
    return device_create("fb0",KDEV_BLOCK,KDEV_FB,0,0,512,&fb_ops,0,0);
}
