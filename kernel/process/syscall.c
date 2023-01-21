#include"process/syscall.h"
#include"process/task.h"
#include"gates/irq.h"
#include"mem/malloc.h"
#include"gates/isr.h"
#include"console.h"
#include"hardware/keyboard/keyboard.h"
#include"mem/memorylayout.h"
#include"mem/page.h"
#include"process/ipc/pipe.h"
int *syscall_handles=0;

int syscall_nop(int v1,int v2,int v3,int v4)
{
    printf("test syscall: %d %d %d %d",v1,v2,v3,v4);
}
int syscall_printf(char *str)
{
    printf("%s",str);
}
int syscall_dup(int v1,int v2,int v3,int v4)
{
    return user_dup(v1,v2);
}
char syscall_getch()
{
    //printf("getch!");
    //return keyboard_get_key();
    char c=0;
    while (1)
    {
        sys_read(0,&c,1);
        if(c)break;
    }
    //printf("[getc]%c",c);
    
    return c;
}
int syscall_interrupt(registers_t*reg)
{
    
    //printf("in syscall! 0x%x %d %d %d %d %d",get_esp(),reg->ebx,reg->ecx,reg->edx,reg->edx,reg->edi);
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
int syscall_exit(int v1)
{
    //printf("in syscall exit!");
    user_exit();
}
int syscall_foperate(int v1,int v2,int v3,int v4)
{
    syscall_extra_args*arg=v4;
    if(v1==FOP_OPEN)
    {
        //printf("[SYSCALL OPEN:%s]",v2);
        return sys_open(v2,v3);
    }
    else if(v1==FOP_READ)return sys_read(v2,v3,v4);
    else if(v1==FOP_CLOSE)return sys_close(v2);
    else if(v1==FOP_MMAP)return sys_mmap(v2,v3,arg->v1,arg->v2,arg->v3);
    else if(v1==FOP_WRITE)return sys_write(v2,v3,v4);
    else if(v1==FOP_SEEK)return sys_lseek(v2,v3,v4);
}

int syscall_gets(char *buf,int size,int v3,int v4)
{
    while (1)
    {
        //printf("in syscall get");
        int r=sys_read(0,buf,size);
        //printf("[%d/%d]",r,size);
        if(r!=size)
        {
            
            thread_block();
            //printf("wake up!");
        }
        else
        {
            //printf("[return from syscall gets%s]",buf);
            return size;
        }
    }
    
}
void syscall_cls()
{
    printf("in cls");
    Klogger->cls();
}
void sys_fork(int v1,int v2,int v3,int v4)
{
    return user_fork();
}
void sys_ls(int v1,int v2,int v3,int v4)
{
    printf("Listing Dir:%s\n",v1);
    vfs_print_dir(v1);
}
void sys_ps(int v1,int v2,int v3,int v4)
{
    task_ps();
}
int sys_exec(int v1,int v2,int v3,int v4)
{
    return user_exec(v1);
}
int sys_sleep(int v1,int v2,int v3,int v4)
{
    user_sleep(v1);
}
int sys_meminfo(int *v1,int *v2,int v3,int v4)
{
    *v1=pmm_get_used();
    *v2=kernel_mem_map.total_mem_in_kb;
}
int sys_wait(int v1,int v2,int v3,int v4)
{
    user_wait();
}
int sys_pipe(int v1,int v2,int v3,int v4)
{
    return user_pipe(v1);   
}
void init_syscall()
{
    //if(syscall_handles)return;
    syscall_handles=kmalloc(SYSCALL_NR*4);
    syscall_handles[SYSCALL_NOP]=syscall_nop;
    syscall_handles[SYSCALL_PRINTF]=syscall_printf;
    syscall_handles[SYSCALL_EXIT]=syscall_exit;
    syscall_handles[SYSCALL_GETCH]=syscall_getch;
    syscall_handles[SYSCALL_CLRSCR]=syscall_cls;
    //syscall_handles[SYSCALL_READ]=syscall_read;
    syscall_handles[SYSCALL_GETS]=syscall_gets;
    syscall_handles[SYSCALL_FOP]=syscall_foperate;
    syscall_handles[SYSCALL_FORK]=sys_fork;
    syscall_handles[SYSCALL_TEST_LIST_DIR]=sys_ls;
    syscall_handles[SYSCALL_EXEC]=sys_exec;
    syscall_handles[SYSCALL_PS]=sys_ps;
    syscall_handles[SYSCALL_SLEEP]=sys_sleep;
    syscall_handles[SYSCALL_MEMINFO]=sys_meminfo;
    syscall_handles[SYSCALL_WAIT]=sys_wait;
    syscall_handles[SYSCALL_PIPE]=sys_pipe;
    syscall_handles[SYSCALL_DUP]=syscall_dup;
    register_interrupt_handler(0x80,syscall_interrupt);
}