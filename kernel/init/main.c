#include"types.h"
#include"boot/multiboot.h"
#include"gates/gdt.h"
#include"console.h"
#include"string.h"
#include"gates/idt.h"
#include"hardware/timer.h"
#include"mem/page.h"
#include"mem/memorylayout.h"
#include"mem/vmm.h"
#include"mem/malloc.h"
#include"kobjects/kobject.h"
#include"process/task.h"
#include"clock.h"
#include"hardware/ata.h"
#define VIDEO 0xB8000
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
    while(1)printf("taskA!;");
}
void test_t2(void *args)
{
    while(1)printf("taskB!;");
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
    
    *((uint8_t*)VIDEO)='d';
    Klogger.putstr("hello log!\n");
    //kstart=&kstart;
    //kend=&kend;
    init_memorylayout((uint32_t)&kstart,(uint32_t)&kend,get_max_pm_addr(mbi)/1024);
    //We set the value by its address because we prepare the addresses in linker.ld;
    printf("Kernel from 0x%x to 0x%x mem_size:0x%d bitmap:0x%x\n",kernel_mem_map.kstart,kernel_mem_map.kend,kernel_mem_map.total_mem_in_kb/1024,kernel_mem_map.phy_bitmap_addr);
    //asm volatile ("int $0x1");
    //asm volatile ("sti");
    //asm volatile ("int $0x2");
    init_page();
    init_vmm();
    init_timer();
    init_task();
    //clock_init();
    //create_thread(1,&test_t1,0,kmalloc_page(1),1);
    //create_thread(2,&test_t2,0,kmalloc_page(1),1);
    

    asm volatile("sti");//TODO: This code is necessary, but why?
    /*uint32_t *t=kmalloc_page(1);
    *t=114514;
    printf("%d %x\n",*t,t);
    kfree_page(t,1);*/
    //*t=233;//page fault!
    init_kslab();
    init_kobject();
    //init_ide();
    ide_initialize(0x1F0, 0x3F6, 0x170, 0x376, 0x000);
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
    //printf("MEM INFO:[%d/%d]\n",pmm_get_used(),kernel_mem_map.total_mem_in_kb);
    while(1);
}