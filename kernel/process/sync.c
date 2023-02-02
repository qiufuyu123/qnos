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
    TCB_t* now=get_running_progress();
    if(!now)return;
    _IO_ATOMIC_IN
    if(lock->holder_task==now)
    {
        lock->repeat_ref_count++;
        _IO_ATOMIC_OUT
        return;
    }
    if(!lock->holder_task)
    {
        lock->holder_task=now;
        _IO_ATOMIC_OUT;
        return;
    }
    else{
        list_append(&lock->wait_list,&now->lock_tag);
        while(lock->holder_task!=now)
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
        //printf("lock over");
        
        }
        //OK, lock is free now
        //At the same time, the blocked thread is woken up 

        //lock->holder_task=cur_task;
        // ^^^^^^ REMEMBER,
        //We dont need these codes 
        //Because we set the holder after sth is released.
        //printf("block end!;");
        _IO_ATOMIC_OUT;
        return;
    }
}
void lock_release(lock_t *lock)
{
    TCB_t* now=get_running_progress();
    if(!now)return;
    //printf("in releasing...;");
    _IO_ATOMIC_IN
    if(lock->holder_task==now)
    {
        //
        if(lock->repeat_ref_count!=0)
        {
            
            lock->repeat_ref_count--;
            _IO_ATOMIC_OUT
            return;
        }
        
        if(list_empty(&lock->wait_list))
        {
            lock->holder_task=0;
            _IO_ATOMIC_OUT;
            return;
        }
        //printf("not empty waiting...");
        TCB_t*t=elem2entry(TCB_t,lock_tag,list_pop(&lock->wait_list));
        lock->holder_task=t;
        thread_wakeup(t);
        //switch_to(&(cur_task->context),&(t->context));
        _IO_ATOMIC_OUT;
        return;
    }
    //Not allow
    _IO_ATOMIC_OUT
    printf("Bad release.;");
}