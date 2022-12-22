

#include"process/task.h"
#include"mem/page.h"
#include"gates/isr.h"
#include"console.h"
#include"gates/tss.h"
extern uint32_t main_esp;
void exit_int(uint32_t esp);
int start_user_task(void *function,void *args,int argc)
{
    
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
        //tss_update(main_esp+6*4096);
        /**
         * ^^^^^^^^^^
         * @brief DONT TOUCH IT!
         * 
         */
        tss_update(task->kern_user2kern_stack_top+4095);
        //printf("userswitch!");
        
    }
}