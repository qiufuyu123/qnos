#include"hardware/ramdisk.h"
#include"mem/memorylayout.h"
#include"mem/malloc.h"
#include"console.h"
#include"string.h"
int ram_read(kdevice_t*self,uint32_t addr,uint32_t nums,char *buffer,int flag)
{
    for (int i = 0; i < nums; i++)
    {
        memcpy(buffer+i*512,self->data+i*512,512);
    }
    return nums;
}
int ram_write(kdevice_t*self,uint32_t addr,uint32_t nums,char *buffer,int flag)
{
    printf("\nRRRRRram write!\n");
    for (int i = 0; i < nums; i++)
    {
        memcpy(self->data+i*512,buffer+i*512,512);
    }
    return nums;
}
kdevice_ops_t ram_ops={
    .read=ram_read,
    .write=ram_write
};
kdevice_t* device_create_ramdisk(int id)
{
    uint32_t num_need=ngx_align(RAMDISK_SIZE,4096)/4096;
    char *buffer=kmalloc_page(num_need);
    if(!buffer)return 0;
    char buf[20]={0};
    sprintf(buf,"ramdisk%d",id);
    return device_create(buf,KDEV_BLOCK,KDEV_RAMDISK,id,0,512,&ram_ops,buffer,num_need);
}