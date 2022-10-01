#include"sysmodule.h"
#include"kelf.h"
#include"kobjects/obj_vfs.h"
#include"io.h"
#include"mem/malloc.h"
#include"console.h"
slab_unit_t *sys_module_slab;
#define check_sysmodule_slab if(!sys_module_slab)sys_module_slab=alloc_slab_unit(sizeof(sysmodule_t),"sys_module");
kobject_operations_t common_op={  
};

KMDKInfo_t* check_sys_format(struct elf_module*e)
{
    KMDKInfo_t**k2= elf_module_sym(e,KM_SETUP_INFO_STR);
    if(k2)return *k2;
    return 0;
}
sysmodule_t* load_sys_module(char *path)
{
    //1st loadfile
    char *buf;
    uint32_t pgn;
    int fd=kread_all(path,&buf,&pgn);
    if(fd<0)return 0;
    struct elf_module*elf= elf_module_init(buf);
    kfree_page(buf,pgn);
    if(!elf)
    {
        return 0;
    }

    KMDKInfo_t*info=check_sys_format(elf);
    if(!info)
    {
        elf_module_release(elf);
        return 0;
    }
    check_sysmodule_slab
    sysmodule_t*m=alloc_in_slab_unit(sys_module_slab);
    if(!m)return 0;
    m->init_func=elf_module_sym(elf,"init");
    m->release_func=elf_module_sym(elf,"release");
    m->main_thread=elf_module_sym(elf,"main");
    m->resource_handle=elf_module_sym(elf,"resource_handle");
    m->user_main_thread=elf_module_sym(elf,"user_main_thread");
    m->elf=elf;
    m->sys_type=SYSM_EXECUTABLE;
    if(m->resource_handle)m->sys_type|=SYSM_RESOURCES;
    m->bind_obj=m->bind_thread=0;
    m->km_info=info;
    return m;
}

void unload_sys_module(sysmodule_t*m)
{
    
    //kfree_page(m->vaddress,m->pgnum);
    cli();
    if(m->bind_obj)release_kobject(m->bind_obj);
    if(m->bind_thread)thread_die(m->bind_thread);
    elf_module_release(m->elf);
    sti();
    kfree(m);
}
int _thread_exe(sysmodule_t *m)
{
    m->bind_thread=get_running_progress();
    //int (*init)()=elf_get_symbol(m->vaddress,"init");
    if(m->main_thread)return m->main_thread();
    return -1;
}
int execute_sys_module(sysmodule_t*m)
{
    if(m->bind_thread)return m->bind_thread;
    if(!m->bind_obj)
    {
        //kobject_operations_t* (*getop)()=elf_get_symbol(m->vaddress,"OGetOp");
        //uint32_t own_op=0;
        //if(getop)own_op=getop();
        //if(!m->name)return -1;
        m->bind_obj=create_kobject(0,0,&m->km_info->ops,m->km_info->name);
        m->init_func(m);
    }else return -1;
    if(m->sys_type&SYSM_EXECUTABLE)
    {
        char *namebuf=kmalloc(5+strlen(m->km_info->name)+1);
        strcpy(namebuf,"KMOD_");
        strcat(namebuf,m->km_info->name);
        create_kern_thread(namebuf,_thread_exe,m);
    }
    return 1;
}
void sys_module_dump(sysmodule_t*m)
{
    //printf("Kernel Module Name:%s Desc:%s Version:%d.%d.%d;",m->km_info->name,m->km_info->description,m->km_info->versions[0],m->km_info->versions[1],m->km_info->versions[2]);
    //printf("enum Function: Init:0x%x Release:0x%x Main:0x%x",m->init_func,m->release_func,m->main_thread);
}