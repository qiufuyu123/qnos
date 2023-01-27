
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
#include"utils/fastmapper.h"
#include"list.h"
#include"kobjects/obj_vfs.h"
#define TCB_MAGIC_NUMBER  0xFC0D21AB

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
	//uint32_t eax;
    uint32_t eflags;
	uint32_t esp;     //esp是保存在kern_stack_top中的       
} __attribute__((packed)) context_t;       //由于要在汇编中使用 要编译成连续的分布

//TCB结构 对于用户进程与内核线程 TCB结构是相同的 不同之处在于：
//
extern uint32_t _schedule_now,_schedule_next;
typedef
struct TCB_t{
	uint32_t * kern_stack_top;    //对应的内核栈顶地址
	uint32_t kern_user2kern_stack_top;
	task_status_t task_status;  
	list_elem_t lock_tag;
	list_elem_t dead_tag;
	list_elem_t child_tag;
	uint32_t time_counter;      //记录运行总的时钟中断数
	uint32_t time_left;         //剩余时间片
	struct TCB_t * next;        //下一个TCB(用于线程调度)
	//uint32_t idt_addr;        在用户进程中使用的页表 内核线程可以不加入此字段
	uint32_t tid;               //线程id
	uint32_t page_counte;       //TCB分配的内核页空间大小  越大的空间 其内核栈大小越大
	uint32_t page_addr;			//page_counte与page_addr用于释放内存
	char *name;

	bool is_kern_thread;         //识别是否为内核线程     （如果为内核线程的话就不进行换页等处理步骤）
	//bitmap user_vmm_pool;    //定义虚拟内存池结构
	uint32_t pdt_vaddr;    //定义进程虚拟页表在内核态中的虚拟地址（使用内核页表）    由于分配在内核内存中 所以pdt是只能在内核中进行读写操作的

	context_t context;

	uint32_t tcb_magic_number;    //这个32位的magic number用来检查内核栈是否溢出
	//uint8_t fork_mark;
	struct TCB_t *parent_thread;
	list_t child_thread_list;
	//fastmapper_t fd_list;
	uint32_t *fd_list;
	uint32_t ticks_cnt;
	uint32_t sleep_time;
} TCB_t;
#define FD_MAX 20
typedef void * thread_function(void * args);       //定义线程的实际执行函数类型
TCB_t*get_tcb(uint32_t pid);
bool check_kern_stack_overflow(TCB_t* tcb_ptr);    

extern void switch_to(void * cur_context_ptr,void * next_context_ptr);    //使用汇编完成的切换上下文函数

extern uint32_t get_esp();

int kernel_fork();
int user_fork();
int schedule();

void create_thread(char *name,uint32_t tid,thread_function *func,void *args,uint32_t addr,uint32_t page_counte,bool is_kern_thread,uint32_t pdt_vaddr,TCB_t*parent);

TCB_t* create_kern_thread(char *name,thread_function *func,void *args);

void threads_init();    //线程模块初始化 需要把主线程加入运行表中
TCB_t *create_user_thread(char *path);
int user_exec(char*path,uint32_t *argv);
void task_ps();
TCB_t* create_TCB(uint32_t tid,uint32_t page_addr,uint32_t page_counte);
void user_sleep(uint32_t us);
TCB_t* get_running_progress();

void thread_block();

void thread_wakeup(TCB_t * target_thread);
void user_wait();
int thread_add_fd(vfs_file_t* file);

void thread_add_child(TCB_t*parent,TCB_t*child);

void thread_die(TCB_t*target_thread);
int user_dup(int oldfd,int newfd);
vfs_file_t* thread_get_fd(int id);
void active_task(TCB_t *task);
void user_exit();     //线程结束函数 关闭中断->移出执行链表->回收内存空间->开启中断
#endif