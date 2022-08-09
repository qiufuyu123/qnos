/**
 * @file task.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief Sturctures for task
 * @version 0.1
 * @date 2022-08-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _H_TASK
#define _H_TASK
#include"types.h"

typedef 			//定义线程或进程状态
enum task_status_t{     
	TASK_RUNNING,
	TASK_READY,
	TASK_BLOCKED,
	TASK_WAITING,
	TASK_HANGING,
	TASK_DIED
} task_status_t;

typedef
struct context_t{       //存放在内核栈中的任务上下文
    uint32_t ebp;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;
	uint32_t esp;     //esp是保存在kern_stack_top中的       
} __attribute__((packed)) context_t;       //由于要在汇编中使用 要编译成连续的分布

typedef
struct TCB_t{
	uint32_t * kern_stack_top;    //对应的内核栈顶地址
	task_status_t task_status;  
	uint32_t time_counter;      //记录运行总的时钟中断数
	uint32_t time_left;         //剩余时间片
	struct TCB_t * next;        //下一个TCB(用于线程调度)
	//uint32_t idt_addr;        在用户进程中使用的页表
	uint32_t tid;               //线程id
	uint32_t page_counte;       //分配的页空间大小
	uint32_t page_addr;			//page_counte与page_addr用于释放内存
	context_t context;
} TCB_t;
extern TCB_t *cur_task;
typedef void * thread_function(void * args);       //定义线程的实际执行函数类型
void init_task();
void schedule();
void create_thread(uint32_t tid,thread_function *func,void *args,uint32_t addr,uint32_t page_counte);
#endif