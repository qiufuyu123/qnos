#include"process/sync.h"
#include"io.h"
#include"console.h"
void lock_init(lock_t*lock)
{
    list_init(&lock->wait_list);
    lock->repeat_ref_count=lock->holder_task=0;
}
void lock_acquire(lock_t*lock)
{
    if(!get_running_progress())return;
    cli();
    if(lock->holder_task==get_running_progress())
    {
        //printf("repeat acq;");
        lock->repeat_ref_count++;
        sti();
        return;
    }
    if(!lock->holder_task)
    {
        //printf("hold an empty lock;");
        lock->holder_task=get_running_progress();
        sti();
        return;
    }
    else{
        list_append(&lock->wait_list,&get_running_progress()->lock_tag);
        //printf("append to wait;");
        while(lock->holder_task!=get_running_progress())
        // ^^^^^^^^^^  This loop is used to prevent some SB who wants to wake up a blocked thread 
        //and unfortunately, this thread is blocked by the lock
        //
        {
        //printf("call sleep;");
        //task_sleep_cur
        //cur_task->task_status=TASK_BLOCKED;
        thread_block();
        //Now, current task is SLEEPING 
        //After being woken up,
        //Code will return here vvvvvvv
        //schedule();
        
        }
        //OK, lock is free now
        //At the same time, the blocked thread is woken up 

        //lock->holder_task=cur_task;
        // ^^^^^^ REMEMBER,
        //We dont need these codes 
        //Because we set the holder after sth is released.
        //printf("block end!;");
        sti();
        return;
    }
}
void lock_release(lock_t *lock)
{
    if(!get_running_progress())return;
    //printf("in releasing...;");
    if(lock->holder_task==get_running_progress())
    {
        //printf("release...;");
        if(lock->repeat_ref_count!=0)
        {
            lock->repeat_ref_count--;
            return;
        }
        cli();
        if(list_empty(&lock->wait_list))
        {
            //printf("empty waiting;");
            lock->holder_task=0;
            sti();
            return;
        }
        //printf("not empty waiting...");
        TCB_t*t=elem2entry(TCB_t,lock_tag,list_pop(&lock->wait_list));
        //printf("get waittask:%d;",t->tid);
        lock->holder_task=t;
        thread_wakeup(t);
        //switch_to(&(cur_task->context),&(t->context));
        sti();
        return;
    }
    //Not allow
    printf("Bad release.;");
}