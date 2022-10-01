#include"process/task.h"
#include"mem/page.h"
#include"gates/isr.h"
#include"console.h"
#include"gates/tss.h"
void exit_int(uint32_t esp);
int start_user_task(void *function,void *args,int argc)
{
    if(!function)return -1;
    TCB_t *cur_tcb=get_running_progress();
    uint32_t kern_stack_max_addr = cur_tcb->page_addr + cur_tcb->page_counte*4096;
    registers_t int_stack;
    int_stack.eip=function;
    //int_stack.cs=
}
void active_task(TCB_t *task)
{
    if(task->is_kern_thread)
    {
        page_setup_kernel_pdt();
    }else
    {
        page_directory_t *pd=(task->pdt_vaddr);
        if(!pd)
        {
             PANIC("NO PDT IN USER TASK!");
        }
        page_setup_pdt(pd);
        //printf("switch user:%s;",task->name);
        tss_update(task);
    }
}