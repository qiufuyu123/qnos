#include"process/task.h"
#include"console.h"
TCB_t *cur_task;
TCB_t kernel_task;
#define TIME_OUT 2
void init_task()
{
    cur_task=&kernel_task;
    cur_task->tid=0;
    cur_task->kern_stack_top=0;
    cur_task->next=cur_task;
    cur_task->page_addr=0;
    cur_task->page_counte=0;
    cur_task->task_status=TASK_RUNNING;
    cur_task->time_counter=0;
    cur_task->time_left=TIME_OUT;
    
}
uint32_t create_TCB(uint32_t tid,uint32_t page_addr,uint32_t page_counte){
	TCB_t * tcb_buffer_addr = (TCB_t*)page_addr;
	tcb_buffer_addr->tid = tid;         
	tcb_buffer_addr->time_counter=0;
	tcb_buffer_addr->time_left=TIME_OUT;
	tcb_buffer_addr->task_status = TASK_RUNNING;
	tcb_buffer_addr->page_counte=page_counte; 
	tcb_buffer_addr->page_addr=page_addr;
	tcb_buffer_addr->kern_stack_top=page_addr+page_counte*4096;
	return page_addr;
}
void remove_thread(){
	asm volatile("cli");
	if(cur_task->tid==0)
		printf("ERRO:main thread can`t use function exit\n");
	else{
		TCB_t *temp = cur_task;
		for(;temp->next!=cur_task;temp=temp->next)
			;
		temp->next = cur_task->next;
	}
}
void exit(){
	remove_thread();
	TCB_t *now = cur_task;
	TCB_t *next_tcb = cur_task->next;
	next_tcb->time_left = TIME_OUT;
	cur_task = cur_task->next;
	switch_to(&(now->context),&(next_tcb->context));
	//注意 暂时没有回收此线程页
}
void create_thread(uint32_t tid,thread_function *func,void *args,uint32_t addr,uint32_t page_counte){	
	asm volatile("cli");  //由于创建过程会使用到共享的数据 不使用锁的话会造成临界区错误 所以我们在此处关闭中断
	TCB_t * new_tcb = create_TCB(tid,addr,page_counte);
	TCB_t * temp_next = cur_task->next;
	cur_task->next = new_tcb;
	new_tcb->next = temp_next;
	*(--new_tcb->kern_stack_top)=args;     //压入初始化的参数与线程执行函数
	*(--new_tcb->kern_stack_top)=exit;
	*(--new_tcb->kern_stack_top)=func;
	new_tcb->context.eflags = 0x200;
	new_tcb->context.esp =new_tcb->kern_stack_top;
	asm volatile("sti");	
}
void schedule(){      //调度函数  检测时间片为0时调用此函数
	if(cur_task->next==cur_task){
		cur_task->time_left = TIME_OUT;    //如果只有一个线程 就再次给此线程添加时间片
		return ;
	}
	TCB_t *now = cur_task;
	TCB_t *next_tcb = cur_task->next;
	next_tcb->time_left = TIME_OUT;
	cur_task = next_tcb;
	get_esp();      //有一个隐藏bug 需要call刷新寄存器
	switch_to(&(now->context),&(next_tcb->context));      
}