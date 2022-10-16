#include"console.h"
#include"KMDK/KMDK.h"
#include"process/task.h"
#include"sysmodule.h"
#include"utils/fastmapper.h"
#include"string.h"
//char *a_str="This is a string stored in .data section";
//const char *b_str="Test Module"
#include"ktty.h"
slab_unit_t*ktty_slab;
int tty_read(vfs_file_t*f,uint32_t size,char *buffer)
{
    ktty_context_t *tty=f->content;
    if(!tty)return VFS_BAD_ARG_ERR;
    if(size>tty->buffer_sz-tty->rw_end)
    {
        memset(tty->buffer,0,tty->buffer_sz);
        tty->rw_end=tty->rw_start=0;
    }
    
    

}
int tty_write(vfs_file_t*f,uint32_t size,char *buffer)
{

}
vfs_file_ops_t ttyops={
    .read=tty_read,
    .write=tty_write
};
vfs_file_t*_get_file(uint32_t owner)
{
    // vfs_file_t*f=alloc_in_slab_unit(file_slab);
    // if(!f)return NULL;
    // f->ref_count=f->open_flag=0;
    // f->owner_ptr=owner;
    // f->open_flag=O_DRVONLY;
    // f->content=alloc_in_slab_unit(ktty_slab);
    // f->ops=&ttyops;
    // if(!f->content)
    // {
    //     kfree(f);
    //     return 0;
    // }
    //return f;

}
int kopen(uint32_t val,uint32_t flags)
{
    return _get_file(val);
}
int kread(int idx,char *buffer,uint32_t size,uint32_t flag)
{

}
int kwrite(int idx,char *buffer,uint32_t size,uint32_t flag)
{
    
}
int kfunc()
{
    return -1;
}
int kattrset(uint32_t attr,uint32_t val)
{

}
KM_CREATE(km_info){
    .description="A Kernel TTY manager",
    .name="KTTY",
    .versions=KM_MAKE_VERSION(1,0,0),
    .ops.read=kread,
    .ops.write=kwrite,
    .ops.exfunc=kfunc,
    .ops.attrset=kattrset,
    .ops.open=kopen
};
KM_RELEASE(km_info)
//uint32_t __QNOS_SYSM_NAME=1;
//static int pv2;
int release()
{

}
int main()
{
    printf("Welcome to the main thread! %d:%s",get_running_progress()->tid,get_running_progress()->name);
    while(1);
}
int init( sysmodule_t*m)
{
    //pv2 =2;
    //__QNOS_SYSM_NAME+=4;
    ktty_slab=alloc_slab_unit(sizeof(ktty_context_t),"ktty_slab");
    if(!ktty_slab)return -1;
    printf("sysmodule:%s has loaded!; bind kobj:%s",m->km_info->name,m->bind_obj->name);
    return 1;
}
//KMDKInfo_t* a=&km_info;
