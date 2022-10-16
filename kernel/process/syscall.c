#include"process/syscall.h"
#include"gates/irq.h"
#include"mem/malloc.h"
#include"gates/isr.h"
#include"console.h"
int *syscall_handles=0;

int syscall_nop(int v1,int v2,int v3,int v4)
{
    printf("test syscall: %d %d %d %d",v1,v2,v3,v4);
}
int syscall_interrupt(registers_t*reg)
{
    
    //printf("in syscall! 0x%x %d %d %d %d %d",reg,reg->ebx,reg->ecx,reg->edx,reg->edx,reg->edi);
    uint32_t syscall_id= reg->eax;
    //reg->eax=114;
    if(syscall_id>=SYSCALL_NR)
    {
        reg->eax=-1;
        return -1;
    }
    int (*s_func)(int, int,int,int)=syscall_handles[syscall_id];
    reg->eax=s_func(reg->ebx,reg->ecx,reg->edx,reg->edi); 
}
void init_syscall()
{
    //if(syscall_handles)return;
    syscall_handles=kmalloc(SYSCALL_NR*4);
    syscall_handles[SYSCALL_NOP]=syscall_nop;
    syscall_handles[SYSCALL_PRINTF]=0;
    register_interrupt_handler(0x80,syscall_interrupt);
}