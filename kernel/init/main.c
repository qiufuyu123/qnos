#include"types.h"
#include"kelf.h"
#include"boot/multiboot.h"
#include"gates/gdt.h"
#include"console.h"
#include"KMDK/KMDK.h"
#include"string.h"
#include"gates/idt.h"
#include"hardware/timer.h"
#include"hardware/keyboard/keyboard.h"
#include"mem/page.h"
#include"mem/memorylayout.h"
#include"mem/vmm.h"
#include"mem/malloc.h"
#include"kobjects/kobject.h"
#include"process/task.h"
#include"clock.h"
#include"hardware/ata.h"
#include"hardware/vga.h"
#include"kobjects/obj_vfs.h"
#include"process/sync.h"
#include"utils/fastmapper.h"
#include"process/symbol.h"
#include"io.h"
#include"utils/qconfig.h"
#include"utils/circlequeue.h"
#include"sysmodule.h"
#include"process/syscall.h"
#include"kobjects/obj_serial.h"
#define VIDEO 0xB8000
#define __DEBUG_FILE_SYSTEM 0
lock_t test_lock;
sysmodule_t*t_m;
extern uint32_t kstart,kend;
static uint32_t get_max_pm_addr(multiboot_info_t *mboot_ptr){          //qemu默认为128M
	uint32_t max_addr=0;
	for(pm_entry_t * pm_entry_cur = mboot_ptr->mmap_addr;pm_entry_cur<mboot_ptr->mmap_addr+mboot_ptr->mmap_length;pm_entry_cur++){
		//printf("0x%h-0x%h-0x%h-%d\n",pm_entry_cur->base_addr_low,pm_entry_cur->length_low,pm_entry_cur->base_addr_low+pm_entry_cur->length_low,pm_entry_cur->type);
		if(pm_entry_cur->type==1&&max_addr<pm_entry_cur->base_addr_low+pm_entry_cur->length_low)
			max_addr=pm_entry_cur->base_addr_low+pm_entry_cur->length_low;		
	}
	return max_addr;
}
void test_t1(void *args)
{
    /*for (int i = 0; i < 10000000; i++)
    {
        printf("1");
    }*/
    #ifdef __DEBUG_FILE_SYSTEM
    int fd=sys_open("/boot/sys/test.txt",O_RDONLY);
    if(fd<0)
    {
        printf("t1 open fail!\n");
        while(1);
    }
    char buf[3];
    sys_read(fd,buf,3);
    printf("t1:read 3 bytes from 0:%s",buf);
    #endif
    // //while(1)printf("taskA!;");
    // printf("acquiring lock...(T1);");
    // lock_acquire(&test_lock);
    // //get_tick();
    // printf("get the lock!(T1);");
    // //ksleep(10000*2);
    // printf("ready to release!;");
    // lock_release(&test_lock);
    // printf("release the lock!(T1);");
    // printf("T1 ready...;");
    // lock_acquire(&test_lock);
    // printf("T1 get lock!;");
    
    // lock_release(&test_lock);
    // printf("T1 release lock!;");
    // printf("[%s];",get_running_progress()->name);
    while (1)
    {
        uint8_t c= keyboard_get_key();
        if(c)printf("%c",c);
        /* code */
    }
    
}
void test_t2(void *args)
{
    #ifdef __DEBUG_FILE_SYSTEM
    int fd=sys_open("/boot/sys/test.txt",O_RDONLY);
    if(fd<0)
    {
        printf("t2 open fail!\n");
        while(1);
    }
    char buf[3];
    sys_lseek(fd,3,SEEK_SET);
    sys_read(fd,buf,3);
    printf("t2:read 3 bytes from 3:%s",buf);
    #endif
    //     printf("acquiring lock...(T2);");
    // lock_acquire(&test_lock);
    // //get_tick();
    // printf("get the lock!(T2);");
    // ksleep(1000*2);
    // //printf("ready to release;");
    // lock_release(&test_lock);
    // printf("release the lock!(T2);");
    /*for (int i = 0; i < 10000000; i++)
    {
        printf("2");
    }*/
    /*printf("T2 ready...;");
    lock_acquire(&test_lock);
    printf("T2 get lock!;");
    ksleep(2*1000);
    lock_release(&test_lock);
    printf("T2 release lock!;");
    printf("[%s];",get_running_progress()->name);*/
    
    int ret=kernel_fork();
    printf("ret from fork!;%d next:0x%x;",get_running_progress()->tid,test_t2);

    if(ret==0)
    {
        printf("This is a forked thread!\n");
        while (1)
        {
            /* code */
        }
    }else
    {
        printf("We fork a child thread:%d!\n",ret);
        while (1)
        {
            /* code */
        }
        
    }

}
int test_user_task(void *argv)
{
    //printf("USER MODE!");
    int a=0;
   
    //asm volatile("int $0x80" : "=a" (a) : "0" (a), "b" ((int)t_m->user_main_thread), "c" ((int)a),"d"((int)a),"D"((int)a));
    __base_syscall(0,1,2,3,4);
    //t_m->user_main_thread(0,0);
    
    //test_func_call();
    while (1)
    {
        /* code */
    }
    
    return 114;
}
int kernelmain(uint32_t magic,uint32_t addr)
{
    multiboot_info_t *mbi;
    if(magic!=MULTIBOOT_BOOTLOADER_MAGIC)
    {
        while(1);
    }
    mbi=(multiboot_info_t*)addr;
    init_gdt();
    init_idt();
    init_serial();
    *((uint8_t*)VIDEO)='d';
    //Klogger.putstr("hello log!\n");
    init_vga(mbi);
    printf("[framebuffer:0x%x w:%d h:%d]\n",mbi->framebuffer_addr,mbi->framebuffer_width,mbi->framebuffer_height);
    //kstart=&kstart;
    //kend=&kend;
    init_memorylayout((uint32_t)&kstart,(uint32_t)&kend,get_max_pm_addr(mbi)/1024);
    //We set the value by its address because we prepare the addresses in linker.ld;
    printf("Kernel from 0x%x to 0x%x mem_size:0x%d bitmap:0x%x\n",kernel_mem_map.kstart,kernel_mem_map.kend,kernel_mem_map.total_mem_in_kb/1024,kernel_mem_map.phy_bitmap_addr);
    //while(1);
    //asm volatile ("int $0x1");
    //asm volatile ("sti");
    //asm volatile ("int $0x2");
    init_page();
    init_vmm();
    init_timer();
    
    /*uint32_t *t=kmalloc_page(1);
    *t=114514;
    printf("%d %x\n",*t,t);
    kfree_page(t,1);*/
    //*t=233;//page fault!
    init_kslab();
    init_kobject();
    threads_init();
    keyboard_init();
    //clock_init();
    lock_init(&test_lock);
    //asm volatile("cli");//TODO: This code is necessary, but why?
    
    //create_thread(1,&test_t1,0,kmalloc_page(1),1,1,0);
    //create_thread(2,&test_t2,0,kmalloc_page(1),1,1,0);
    //init_ide();
    printf("stage 1!");
    asm volatile("sti");//TODO: This code is necessary, but why?
    //while(1);
    #ifdef __DEBUG_FILE_SYSTEM
    ide_initialize(0x1F0, 0x3F6, 0x170, 0x376, 0x000);
    init_fslist();
    #endif
    //create_kern_thread("1",&test_t1,0);
    //create_kern_thread("2",&test_t2,0);
    //kobject_get_ops(KO_ATA_DEV)->open(0,0);
    //char *buf=kmalloc(2048);
    //printf("buf is %x\n",buf);
    //memset(buf,0,2048);
    //kobject_get_ops(KO_ATA_DEV)->read(0,buf,1,0);
    // printf("==>\n");
    // for (int i = 0; i < 512; i++)
    // {
    //     if(buf[i])printf("%c",buf[i]);
    // }
    // printf("==>\n");
    // kobject_get_ops(0)->write(0,"hihih",0,0);
    //ata_init();
    /*
    char *str_test=kmalloc(20);
    strcpy(str_test,"HELLO?!");
    printf("%s\n",str_test);
    kfree(str_test);
    */
    //
    fastmapper_t test_map;
    fastmapper_init(&test_map,10);
    fastmapper_add(&test_map,114514,5);
    fastmapper_add(&test_map,1919810,12);
    printf("stage 2!");
    printf("[get %d %d!];\n",fastmapper_get(&test_map,5),fastmapper_get(&test_map,12));
    //TCB_t*ttt=get_tcb(2);
    //printf("task : pid:%d name:%d kaddr:0x%x\n",ttt->tid,ttt->name,ttt->page_addr);
    printf("MEM INFO:[%d/%d]\n",pmm_get_used(),kernel_mem_map.total_mem_in_kb);
    //vfs_file_t*f= vfs_fopen("/boot/setup.ini",O_RDWR);
    // int fd=sys_open("/boot/setup.ini",O_RDONLY);
    // if(fd<0)printf("open setuo.ini fail!\n");
    // else
    // {
    //     sys_lseek(fd,0,SEEK_END);
    //     uint32_t sz=sys_tell(fd);
    //     printf("SETUP.INI size:%d bytes",sz);
    //     char *buf=kmalloc(sz);
    //     sys_lseek(fd,0,SEEK_SET);
    //     int ret=sys_read(fd,buf,sz);
    //     printf("%d;%s",ret,buf);
    //     struct read_ini *cfg=NULL;
    //     struct ini*ini=read_ini(&cfg,"setup.ini",buf,sz);
    //     ini_pp(ini);
    //     qconfig_value_t*v= qconfig_get_value(ini,"c_cfg/showtext");
    //     if(!v)printf("NULL V");
    //     else printf("value:%d %s type:%d;",v->number,v->pure_str,v->type);
    // }
    // symbol_init();
    circlequeue_t test_queue;
    circlequeue_init(&test_queue,5,1);
    const char *tstr="ABCDEFG";
    for (int i = 0; i < 6; i++)
    {
        printf("push:%d;",circlequeue_push(&test_queue,&tstr[i]));
    }
    for (int i = 0; i < 6; i++)
    {
        char *s= circlequeue_get(&test_queue);
        if(s)printf("get:%c;",*s);
        else printf("fail get!");
    }
    //printf("printf:0x%x",symbol_find("printf"));
    //fd=sys_open("/boot/test.sys",O_RDONLY);
    uint32_t sys_addr,pgn;
    //fd= kread_all("/boot/test.sys",&sys_addr,&pgn);
   // printf("sys module fd:%d 0x%x;",fd,sys_addr);

    //printf("%s",sys_addr);
    //int m=1;

    // if(fd>=0)
    // {
    //     t_m= load_sys_module("/boot/sys/test.sys");
    //     printf("fuser:0x%x",t_m->user_main_thread);
    //     if(t_m)
    //     {
    //     sys_module_dump(t_m);
    //     execute_sys_module(t_m);
    //     }
    //     //struct elf_module*e= elf_module_init(sys_addr);
    //     //int (*init_func)() =elf_module_sym(e,"init");
    //     //uint32_t r=init_func();
    //     //printf("%s",r);
    //     //KMDKInfo_t**k2=elf_module_sym(e,"__KM_SETUP_INFO_");
    //     //KMDKInfo_t*k=*k2;
    //     //KMDKInfo_t kk=elf_module_sym(e,"km_info");
    //     //if(k)
    //     //{
    //     //    printf("Addr:0x%xName:%s Des:%s Ver:%d.%d.%d",k,k->name,k->description,k->versions[0],k->versions[1],k->versions[2]);
    //     //}
    //     //printf("relocating...");
    //     //elf_relocate(sys_addr);
    //     //printf("getting sym...;");
    //     //int (*sys_init)()=elf_get_symbol(sys_addr,"init");
    //     //if(sys_init)printf("sys return:%d",sys_init());
    //     //sysmodule_t *m=load_sys_module("/boot/test.sys");
    //     //if(m)
    //     //{
    //     //    printf("load sysmodule,type:%d;",m->sys_type);
    //     //    if(m->name)printf("%s;",m->name);
    //         //execute_sys_module(m);
    //     //}
    // }
    //sti();

    init_syscall();
    printf("user adddress:0x%x;",test_user_task);
    create_user_init_thread();
    //jump_usermode();
    //switch_to_user_mode();
    while(1)
    {

        //if(c)printf("%x",c);
    }
}