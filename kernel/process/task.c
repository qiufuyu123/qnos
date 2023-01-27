
#include"process/task.h"
#include"console.h"
#include"io.h"
#include"hardware/timer.h"
#include"mem/malloc.h"
#include"mem/vmm.h"
#include"mem/memorylayout.h"
#include"mem/page.h"
#include"string.h"
#include"utils/fastmapper.h"
#include"kobjects/kobjs.h"
#include"gates/tss.h"
#include"process/syscall.h"
#include"process/ipc/pipe.h"
#include"kelf.h"
#include"io.h"
#define TIME_CONT  10 //默认时间片计数
#define MAX_PID 65535
#define FORK_COPY_MARK 2
#define __DEBUG_FILE_SYSTEM 0
uint32_t main_esp;
extern int log_fd[2];
extern int log_key_fd[2];
TCB_t main_TCB;    //内核主线程TCB
TCB_t* cur_tcb;
bitmap_t kpid_map;
fastmapper_t pid_mapper;
list_t dead_thread_list;
int clean_up_dead(TCB_t* tmp);
TCB_t* get_running_progress(){
	return cur_tcb;
}
TCB_t*get_tcb(uint32_t pid)
{
	return fastmapper_get(&pid_mapper,pid);
}
bool check_kern_stack_overflow(TCB_t* tcb_ptr){
	if(tcb_ptr->tcb_magic_number==TCB_MAGIC_NUMBER){
		return 1;
	}
	else{
		return 0;
	}	
}

static void kern_overflow_handler(TCB_t * tcb_ptr){
	uint32_t tid = tcb_ptr->tid;
	printf("Thread Kern Stack Overflow!STOP!");
}

//线程调度的第一步
//主要功能： 1 页表切换   2  tss栈修改    
// static void active_task(TCB_t * next){
	
// }

static void  _init_main_thread(TCB_t * main_tcb){
	TCB_t *tcb_buffer_addr = main_tcb;
	tcb_buffer_addr->tid = thread_get_pid();        //主线程的编号为0 
	fastmapper_add(&pid_mapper,main_tcb,0); 
	//fastmapper_init(&tcb_buffer_addr->fd_list,20);
	tcb_buffer_addr->fd_list=kmalloc(4*FD_MAX);
	tcb_buffer_addr->time_counter=0;
	tcb_buffer_addr->time_left=TIME_CONT;
	tcb_buffer_addr->task_status = TASK_RUNNING;
	tcb_buffer_addr->page_counte=0;   //主线程不会被回收内存 所以可以任意赋值
	tcb_buffer_addr->page_addr=0;
	tcb_buffer_addr->next = tcb_buffer_addr;
	tcb_buffer_addr->kern_stack_top=0;
	tcb_buffer_addr->is_kern_thread = 1;
	tcb_buffer_addr->tcb_magic_number = TCB_MAGIC_NUMBER;
}
void thread_release_pid(uint32_t pid)
{
	if(pid>=MAX_PID)return;
	bitmap_set(&kpid_map,pid,0);
}
int thread_get_pid()
{
	int idx= bitmap_scan(&kpid_map,1);
	if(idx<0)return -1;
	bitmap_set(&kpid_map,idx,1);
	return idx;
}
void threads_init(){
	int use_page=ngx_align(MAX_PID/8,4096)/4096;
	kpid_map.bits=kmalloc_page(use_page);
	kpid_map.btmp_bytes_len=use_page*4096;
	bitmap_init(&kpid_map);
	fastmapper_init(&pid_mapper,20);
	TCB_t *tcb_buffer_addr = &main_TCB;
	_init_main_thread(&main_TCB);
	cur_tcb = &main_TCB;
	main_esp=kmalloc_page(6);
	//tss_update(main_esp+6*4096);
	printf("MAINESP%x\n",main_esp);
	list_init(&dead_thread_list);
	// for (int i = 0; i < 4; i++)
	// {
	// 	page_t *p=get_page_from_pdir(&kpdir,main_esp+i*4096);
	// 	p->user=1;
	// }
	// page_setup_kernel_pdt();
	
}
void user_sleep(uint32_t ms)
{
	get_running_progress()->ticks_cnt=get_tick();
	get_running_progress()->sleep_time=ms;
	thread_block();
}
//用于创建线程的PCB
TCB_t* create_TCB(uint32_t tid,uint32_t page_addr,uint32_t page_counte){
	TCB_t * tcb_buffer_addr = (TCB_t*)page_addr;
	tcb_buffer_addr->tid = tid; 
	//fastmapper_init(&tcb_buffer_addr->fd_list,20);
	tcb_buffer_addr->fd_list=kmalloc(4*FD_MAX);
	if(!tcb_buffer_addr->fd_list)return 0;
	fastmapper_add(&pid_mapper,tcb_buffer_addr,tid);        
	tcb_buffer_addr->time_counter=0;
	tcb_buffer_addr->time_left=TIME_CONT;
	tcb_buffer_addr->task_status = TASK_READY;
	tcb_buffer_addr->page_counte=page_counte; 
	tcb_buffer_addr->page_addr=page_addr;
	tcb_buffer_addr->kern_stack_top=page_addr+page_counte*4096;    
	tcb_buffer_addr->tcb_magic_number = TCB_MAGIC_NUMBER;
	list_init(&tcb_buffer_addr->child_thread_list);
	tcb_buffer_addr->parent_thread=0;
	return (TCB_t*)page_addr;
}

//创建最终线程的核心函数     创建用户进程以及创建内核线程的函数都是对这个函数的封装
//会操作TCB链表 需要加锁
void create_thread(char *name,uint32_t tid,thread_function *func,void *args,uint32_t addr,uint32_t page_counte,bool is_kern_thread,uint32_t pdt_vaddr,TCB_t *parent){	
	//asm volatile("cli");  //由于创建过程会使用到共享的数据 不使用锁的话会造成临界区错误 所以我们在此处关闭中断
	TCB_t * new_tcb = create_TCB(tid,addr,page_counte);
	//sizeof(TCB_t);
	TCB_t * temp_next = cur_tcb->next;
	cur_tcb->next = new_tcb;
	new_tcb->next = temp_next;
	new_tcb->is_kern_thread = is_kern_thread;
	if(!is_kern_thread){
		//用户进程需要填充页表等
		//new_tcb->user_vmm_pool = user_vmm_pool;
		//new_tcb->pdt_vaddr = pdt_vaddr;
	}
	new_tcb->context.ebp=new_tcb->kern_stack_top;
	*(--new_tcb->kern_stack_top)=args;     //压入初始化的参数与线程执行函数
	*(--new_tcb->kern_stack_top)=user_exit;
	*(--new_tcb->kern_stack_top)=func;
	//此处存在修改！    0x200 ------->0x202    IF为1（打开硬中断）   IOPL为0（只允许内核访问IO）   1号位为1（eflags格式默认）
	new_tcb->context.eflags = 0x202; 
	//new_tcb->context.ebp=new_tcb->page_addr+new_tcb->page_counte*4096;
	new_tcb->context.esp =new_tcb->kern_stack_top;
	//new_tcb->context.ebp=new_tcb->context.esp;
	new_tcb->parent_thread=parent;
	new_tcb->name=strdup(name);
	//asm volatile("sti");	
}
void switch_to_user_mode() {
			    // Set up a stack structure for switching to user mode.	
	//cli();
// 	printf("before tss");
 	//tss_update(get_running_progress()->kern_user2kern_stack_top+4096);
	printf("after tss");
   asm volatile("  \
      cli; \
      mov $0x23, %ax; \
      mov %ax, %ds; \
      mov %ax, %es; \
      mov %ax, %fs; \
      mov %ax, %gs; \
                    \
       \
      mov %esp, %eax; \
      pushl $0x23; \
      pushl %esp; \
      pushf; \
	  pop %eax; \
	  mov $0x202,%eax;\
	  push %eax;\
      pushl $0x1B; \
      push $1f; \
      iret; \
    1: \
      "); 
                                                                                                              

}
/*
[GLOBAL intr_exit]
intr_exit:
   ;pop ebx        ; reload the original data segment descriptor
   ;mov ds, bx
   ;mov es, bx
   ;mov fs, bx
   ;mov gs, bx
   ;pop eax
   sti
   iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP*/
extern void intr_exit();
// {
	
// 	register uint32_t* r_esp asm("esp");
// 	uint32_t *v=0x3ffe2fff;
// 	printf("INTR EXIT:\n esp:0x%x %x %x %x %x %x \n]",r_esp,*(r_esp),*(r_esp+4),*(r_esp+8),*(r_esp+12),*(r_esp+16));
// 	printf("INTR EXIT:\n %x %x %x %x %x %x\n",*v,*(v-4),*(v-8),*(v-12),*(v-16),*(v-20));
// 	sti();
// 	asm volatile ("iret");
// }

void _fork_cpy_fd(TCB_t*newtcb)
{
	for (int i = 0; i < FD_MAX; i++)
	{
		if(newtcb->fd_list[i])
		{
			((vfs_file_t*)newtcb->fd_list[i])->ref_cnt++;
		}
	}
	
}

int user_fork()
{
	//printf("f1");
	if(get_running_progress()->is_kern_thread!=0)return -1;
	cli();
	TCB_t*old_tcb=get_running_progress();
	TCB_t*new_tcb=kmalloc_page(2);
	//printf("f2");
	if(!new_tcb)
	{
		sti();
		return -1;
	}
	memcpy(new_tcb,old_tcb,4096*2);
	new_tcb->fd_list=kmalloc(4*FD_MAX);
	if(!new_tcb->fd_list)
	{
		kfree_page(new_tcb,2);
		return -1;
	}
	memcpy(new_tcb->fd_list,old_tcb->fd_list,4*FD_MAX);
	new_tcb->tid=thread_get_pid();
	new_tcb->name=strsafecat(old_tcb->name,"_fork");
	if(!new_tcb->name)
	{
		kfree_page(new_tcb,1);
		sti();
		return -1;
	}
	list_append(&old_tcb->child_thread_list,&new_tcb->child_tag);
	new_tcb->parent_thread=old_tcb;
	new_tcb->next=old_tcb->next;
	old_tcb->next=new_tcb;
	new_tcb->kern_user2kern_stack_top=new_tcb;
	new_tcb->pdt_vaddr=page_clone_user_page(old_tcb->pdt_vaddr,0x80000000);
	new_tcb->kern_stack_top=(uint32_t)new_tcb+4096*2-1;
	new_tcb->context.ebp=new_tcb->kern_stack_top;
	*(new_tcb->kern_stack_top-6)=intr_exit;
	*(new_tcb->kern_stack_top-3)=0x202;
	//*(new_tcb->kern_stack_top-6)=0;//ret value
	new_tcb->context.esp=new_tcb->kern_stack_top-6;
	//printhex((uint32_t)new_tcb+4095-20*4,20);
	//*(uint32_t*)((uint32_t)new_tcb->context.ebp+4)=r;
	//printf("fork newesp:0x%x newebp:0x%x\n",new_tcb->context.esp,new_tcb->context.ebp);
	new_tcb->time_counter=0;
	new_tcb->time_left=TIME_CONT;
	new_tcb->task_status=TASK_READY;
	//printf("forked name is %s\n",new_tcb->name);
	//new_tcb->fork_mark=1;
	//int e=0;
	page_setup_pdt(old_tcb->pdt_vaddr);
	_fork_cpy_fd(new_tcb);
	sti();
	return new_tcb->tid;
}
/**
 * @brief NOTICE 
 *  
 * This function is a simple way to realize fork in kernel area
 * We simply use it to do some tests
 * When we enter userland, we will discard this function
 * 
 * enjoy it!
 */
int kernel_fork()
{
	//1st check whether curtask is kernel
	//If it is a kernel thread, the page_addr should be '0' since we didnt set it at all
	//By the way, we cant fork kernel thread
	if(get_running_progress()->is_kern_thread!=1)return -1;
	cli();
	context_t tmp_con;
	//
	uint32_t *now_ebp;

	// register int r_ax asm("eax");
	// register int r_bx asm("ebx");
	// register int r_cx asm("ecx");
	// register int r_dx asm("edx");
	// register int r_di asm("edi");
	// register int r_si asm("esi");
	// tmp_con.ebx=r_bx;
	// tmp_con.ecx=r_cx;
	// tmp_con.edx=r_dx;
	// tmp_con.edi=r_di;
	// tmp_con.esi=r_si;
	//printf("cur thread ebp:0x%x",tmp_con.ebp);
	TCB_t *new_tcb=create_kern_thread(get_running_progress()->name,0,0);
	list_append(&get_running_progress()->child_thread_list,&new_tcb->child_tag);
	new_tcb->parent_thread=get_running_progress();
	switch_get(&tmp_con);
	new_tcb->context=tmp_con;
	//1st copy stack data
	
	uint32_t stack_top=get_running_progress()->page_addr+get_running_progress()->page_counte*4096;
	uint32_t stack_sz=stack_top-tmp_con.esp;
	uint32_t new_stack_top=new_tcb->page_addr+new_tcb->page_counte*4096;

	//2nd calc the esp and ebp
	//printf("STAGE 1");
	//switch_get(&tmp_con);
	//printf("STAGE 2");
	new_tcb->context=tmp_con;
	//printf("stack top:0x%x,esp:0x%x,ebp:0x%x;",stack_top,tmp_con.esp,tmp_con.ebp);
	uint32_t offset_esp=stack_top- tmp_con.esp;
	uint32_t offset_ebp=stack_top-tmp_con.ebp;
	//3rd reset the register
	new_tcb->context.esp=new_stack_top-offset_esp;
	new_tcb->context.ebp=new_stack_top-offset_ebp;
	
	//new_tcb->fork_mark=1;//Dont understand? roll back to the head of the file
	//new_tcb->context.eax=114;
	new_tcb->context.eflags=0x202;
	//new_tcb->time_counter=0;
	//new_tcb->time_left=TIME_CONT;
	//int t= schedule();
	//printf("latest exa:%d\n",t);
	//fastmapper_add
	//fastmapper_remove(&pid_mapper,new_tcb->tid);
	//printf("dst:0x%x,from:0x%x,sz:0x%x;",new_stack_top-stack_sz,stack_top-stack_sz,stack_sz);
	memcpy(new_stack_top-stack_sz,stack_top-stack_sz,stack_sz);
	if(get_running_progress()->tid==new_tcb->tid)
	{
		sti();
		return 0;
	}
	sti();
	return new_tcb->tid;
}
int user_test2()
{
	char as[100];
	memset(as,1,100);
	return &as;
}
int _user_task_func(void *args)
{
	cli();//IMPORTANT!!!!!
	//printf("before switch! 0x%x",(uint32_t)args);
	if(!args)
	{
		switch_to_user_mode();
		int r=__base_syscall(SYSCALL_FORK,0,0,0,0);
		if(r==0)
		{
			user_test2();
		}else{
			user_test2();
		}
		while(1);
	}
	void(*func)()=args;
	func();
	//switch_to_user_mode();
	while(1);
	//  __base_syscall(SYSCALL_PRINTF,"this is syscall 1",0,0,0);
	//  __base_syscall(SYSCALL_PRINTF,"this is syscall 2",0,0,0);
	// // __base_syscall(SYSCALL_EXIT,0,0,0,0);
	
	//printf("user thread idle!0x%x\n",page_kv2p_u(0x3ffda000));
	//while(1);
	
	while(1);
	//thread_function *func=args; 114;
}
void remove_tcb(TCB_t *tcb)
{
	kfree(tcb->fd_list);
	
}
static inline void test_invlpg(void* m)
{
    /* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
    asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}
char *look_for_filename(char *path)
{
	path+=strlen(path);
	while (1)
	{
		path--;
		if(*path=='/')
		{
			//printf("name is %s",path+1);
			return path+1;
		}

	}
	
}
void task_ps()
{
	TCB_t* now=get_running_progress();
	TCB_t* ori=now;
	int i=0;
	while (1)
	{
		Klogger->setcolor(0x000000,0xFF69B4);
		printf("%d ",i);
		Klogger->setcolor(0x000000,0xffffff);
		printf("PID:%d Name:%s     Level:%d      State:%d\n",now->tid,now->name?now->name:"",now->is_kern_thread,now->task_status);
		i++;
		if(now->next==ori)break;
		now=now->next;
	}
	
}
/**
 * argv: char **
 * char* --> uint32
 * char** --> uint32 *
*/
int count_argv(uint32_t *argv)
{
	printf("COUNT:%x;",argv);
	int cnt=0;
	while (*argv)
	{
		argv++;
		cnt++;
	}
	return cnt;
	
}
void do_arg_copy(uint32_t *ori,int len,uint32_t base)
{
	if(!len)
		return;
	uint32_t data_base=base+len*4;
	for (int i = 0; i < len; i++)
	{
		uint32_t addr=ori[i];
		int l=strlen(addr)+1;
		memcpy(data_base,addr,l);
		ori[i]=data_base;
		data_base+=l;
	}
	memcpy(base,ori,len*4);
}
int do_push_argv(uint32_t *argv,uint32_t base,uint32_t *stack)
{
	int len=count_argv(argv);
	printf("arg len:%d",len);
	do_arg_copy(argv,len,base);
	*(--stack)=base;
	*(--stack)=len;
	return len;
}
int user_exec(char*path,uint32_t *argv)
{
	//1st load file

	/**
	 * Since we will remap the memory area
	 * We have to dup a copy of old path 
	*/
	char *dup_path=strdup(path);
	TCB_t*now=get_running_progress();
	#ifdef __DEBUG_FILE_SYSTEM
	int fd= sys_open(dup_path,O_RDONLY);
	if(fd<0)
		return fd;
	char *file_buf=kmalloc_page(3);
	sys_read(fd,file_buf,4096*3);
	int l=sys_tell(fd);
	uint32_t func=0;
	cli();
	uint32_t r=elf_load_user(fd,now->pdt_vaddr);
	if(r==0)
	{
	QNBinary_t bhead;
	func=qbinary_load(file_buf,file_buf,4096*2-sizeof(QNBinary_t),&bhead);
	for (int i = 0; i < 5; i++)
	{
		if(!bhead.v_loads[i][0])continue;
		int add=bhead.v_loads[i][1];
		uint32_t base=bhead.v_loads[i][0];
		for (int j = 0; j < add; j++)
		{
			if(page_chk_user(now->pdt_vaddr,base+j*4096))page_u_map_unset(now->pdt_vaddr,base+j*4096);
			page_u_map_set(now->pdt_vaddr, base+j*4096);
		}
		
	}
	
	#endif
	//2nd remap it
	vmm_remapcls_pages(now->pdt_vaddr,file_buf,3,0x80000000);
	}else
	{
		func=r;
	}
	//3rd reset stacks!
	now->kern_stack_top=(uint32_t)now+4096*2-1;
	now->context.ebp=now->kern_stack_top;
	*(now->kern_stack_top-6)=intr_exit;
	*(now->kern_stack_top-3)=0x202;
	
	*(now->kern_stack_top-5)=func;
	uint32_t *next_stack=(uint32_t*)0xffffe000;
	page_u_map_set(now->pdt_vaddr,0xfffff000);
	do_push_argv(argv,0xfffff000,next_stack);
	next_stack-=2;
	*(--next_stack)=user_exit;
	*(now->kern_stack_top-2)=next_stack;
	//*(new_tcb->kern_stack_top-6)=0;//ret value
	now->context.esp=now->kern_stack_top-6;

	//DO NOT FREE
	//kfree(now->name);
	now->name=strdup(look_for_filename(dup_path));
	sti();
	return 1;
}

TCB_t *create_user_thread(char *path)
{
	#ifdef __DEBUG_FILE_SYSTEM
	int fd= sys_open(path,O_RDONLY);
	if(fd<0)return NULL;
	char *file_buf=kmalloc_page(3);
	sys_read(fd,file_buf,4096*2);
	int l=sys_tell(fd);
	QNBinary_t bhead;
	uint32_t func=qbinary_load(file_buf,file_buf,4096*2-sizeof(QNBinary_t),&bhead);

	#endif
	uint32_t TCB_page = kmalloc_page(2);
    if(TCB_page==0){
        printf("Can`t Create New User Task Because Of Error When Alloc TCB Page From Kernel VMM!STOP!");
		return 0;
    }
	int tid=thread_get_pid();
	if(tid==-1)
	{
		kfree_page(TCB_page,1);
		return 0;
	}
	cli();
	create_thread(strdup(look_for_filename(path)),tid,_user_task_func,0,TCB_page,1,1,0,0);


	//TCB_t *new_tcb=create_kern_thread("iserinit",_user_task_func,0);
	TCB_t*new_tcb=TCB_page;
	
	new_tcb->kern_user2kern_stack_top=TCB_page;
	//tss_update(new_tcb->kern_user2kern_stack_top+4095);
	if(!new_tcb->kern_user2kern_stack_top)
	{
		clean_up_dead(new_tcb);
		return NULL;
	}
	#ifdef __DEBUG_FILE_SYSTEM
	new_tcb->fd_list[0]=thread_get_fd(log_key_fd[0]);
	new_tcb->fd_list[1]=thread_get_fd(log_fd[1]);
	pipe_bind(log_key_fd[0],new_tcb);
	pipe_bind(log_fd[1],new_tcb);
	//else new_tcb->fd_list[0]=get_running_progress()->fd_list[0];
	#endif
	//new_tcb->is_kern_thread=1;
	new_tcb->is_kern_thread=0;
	new_tcb->pdt_vaddr=page_clone_cleaned_page();
	new_tcb->kern_stack_top=0xFFFFe000;
	page_u_map_set(new_tcb->pdt_vaddr,0xffffb000);
	page_u_map_set(new_tcb->pdt_vaddr,0xffffe000);
	page_u_map_set(new_tcb->pdt_vaddr,0xffffd000);
	page_u_map_set(new_tcb->pdt_vaddr,0xffffc000);
	//test_invlpg(0xffffe000);
	//page_u_map_set(new_tcb->pdt_vaddr,0xffffd000);
		for (int i = 0; i < 5; i++)
	{
		if(!bhead.v_loads[i][0])continue;
		if(page_chk_user(new_tcb->pdt_vaddr,bhead.v_loads[i][0]))page_u_map_unset(new_tcb->pdt_vaddr,bhead.v_loads[i][0]);
		page_u_map_set(new_tcb->pdt_vaddr, bhead.v_loads[i][0]);
	}
	//test_invlpg(0xffffd000);
	new_tcb->context.ebp=new_tcb->kern_stack_top;
	/**
	 * @brief NOTICE
	 * In order to setup stack for user program
	 * We switch to its pdt and set the stack
	 * then , we switch back to kernel 
	 */
	/**
	 * DO NOT USE get_page_from_pdir TO GET A user_page!!!!!!
	 * Instead,use get_page_from_u_pdir
	 * Because get_page_from_pdir will flush pte_mapping information!
	*/
	#ifdef __DEBUG_FILE_SYSTEM
	vmm_remap_pages(new_tcb->pdt_vaddr,file_buf,3,0x80000000);
	#endif
	page_setup_pdt(new_tcb->pdt_vaddr);
	#ifdef __DEBUG_FILE_SYSTEM
	//*(--new_tcb->kern_stack_top)=func;
	#else
	*(--new_tcb->kern_stack_top)=0;
	#endif
	*(--new_tcb->kern_stack_top)=0;// b
	*(--new_tcb->kern_stack_top)=1;// a
	*(--new_tcb->kern_stack_top)=user_exit;
	*(--new_tcb->kern_stack_top)=func;
	new_tcb->context.esp=new_tcb->kern_stack_top;
	page_setup_kernel_pdt();
sti();
	schedule();
	return new_tcb;
}


//内核线程必须要保证两点：
//                                            1.func必须保证存在，不会被内存回收
//                                            2.args必须保证存在， 不会被内存回收
//内核线程的func与args都是内核内存空间中的     func通过函数定义的方式保存在os内核的程序段中 ，args保存在调用者函数的定义中，一旦调用者函数退出，args就会被回收
//如何解决这个问题？  线程的创建者函数不能退出！！！使用特定指令阻塞对应的函数（join）
//使用detach，在detach中实现线程将参数复制
TCB_t* create_kern_thread(char* name,thread_function *func,void *args){
	//bitmap default_bitmap;
	//cli();
	uint32_t page_counte = 1;
	uint32_t TCB_page = kmalloc_page(1);
	uint32_t default_pdt_vaddr = 0x0;
	bool is_kern_thread = 1;
    if(TCB_page==0){
        printf("Can`t Create New User Task Because Of Error When Alloc TCB Page From Kernel VMM!STOP!");
		return 0;
    }
	int tid=thread_get_pid();
	if(tid==-1)
	{
		kfree_page(TCB_page,1);
		return 0;
	}
	cli();
	create_thread(name,tid,func,args,TCB_page,page_counte,is_kern_thread,default_pdt_vaddr,&main_TCB);
	sti();
	printf("[CREATE a kernel thread:stack0x%x %s]",((TCB_t*)TCB_page)->kern_stack_top,((TCB_t*)TCB_page)->name);
	//sti();
	return TCB_page;
}
int thread_add_fd(vfs_file_t* file)
{
	for (int i = 0; i < FD_MAX; i++)
	{
		if(get_running_progress()->fd_list[i]==0)
		{
			get_running_progress()->fd_list[i]=file;
			return i;
		}
	}
	return -1;
	//return fastmapper_add_auto(&get_running_progress()->fd_list,file);
}

vfs_file_t* thread_get_fd(int id)
{
	if(id<0||id>=20)return 0;
	return get_running_progress()->fd_list[id];
}
int clean_up_dead(TCB_t* tmp)
{
	//TCB_t*tmp= elem2entry(TCB_t,dead_tag,elem);
	//list_remove(&tmp->dead_tag);
	thread_release_pid(tmp->tid);
	kfree(tmp->fd_list);
	//if(!tmp->is_kern_thread) page_free_pdt(tmp->pdt_vaddr);
	printf("[clean up thread:%d]",tmp->tid);
	kfree_page(tmp,1);
	return 0;
}
uint32_t _schedule_now,_schedule_next;
int schedule(){
    //check if the thread module is available
    if(get_running_progress()==NULL){
        return ;
    }
    //调度函数  检测时间片为0时调用此函数
	//首先判定现在的线程内核栈是否溢出
	if(!check_kern_stack_overflow(get_running_progress())){
		//溢出处理！！！
		kern_overflow_handler(get_running_progress());
	}

	//find next thread
    TCB_t * probe = cur_tcb;
	//list_traversal(&dead_thread_list,__travel_dead_list,0);
    while(1){
        probe = probe->next;
        if(probe==cur_tcb){
            if(probe->task_status!=TASK_READY&&probe->task_status!=TASK_RUNNING){
                // all of the threads are blocked,which is not allowed
                //walk up cur thread
                printf("ALL OF THE THREADS ARE BLOCKED,SELF WAKE UP ONE!WARNNING!");
                cur_tcb->time_left = TIME_CONT;    //如果只有一个线程 就再次给此线程添加时间片
                cur_tcb->task_status = TASK_RUNNING;
                return ;
            }
            else{
                //only cur thread can run
                cur_tcb->time_left =TIME_CONT;
                return ;
            }
        }
        else{
            if(probe->task_status== TASK_READY){
                break;
            }
			else if(probe->task_status==TASK_DIED)
			{
				//remove_thread(probe);
				//clean_up_dead(probe);
				//TODO: CLEAN UP THE DIE THREAD
				continue;
			}else if(probe->task_status==TASK_BLOCKED)
			{
				if(probe->ticks_cnt)
				{
					if(get_tick()-probe->ticks_cnt>=probe->sleep_time)
					{
						probe->ticks_cnt=0;
						thread_wakeup(probe);
					}
				}
			}
            else{
                continue;
            }
        }
    }
	//进行调度
	TCB_t *now = cur_tcb;
	TCB_t *next_tcb = probe;

	//if cur task is blocked,it will not be set ready or cur thread will running next schedule
	if(now->task_status == TASK_RUNNING){
	    now->task_status = TASK_READY;
	}
	next_tcb->task_status = TASK_RUNNING;
	next_tcb->time_left = TIME_CONT;
	cur_tcb = next_tcb;
	//active_task(cur_tcb);
	//printf("switch:%x-%x %d-%d;",now,next_tcb,now->tid,next_tcb->tid);
	//printf("--->esp:%d eax:%d ebp:%d---><---esp:%d eax:%d ebp:%d\n",now->context.esp,now->context.eax,now->context.ebp,next_tcb->context.esp,next_tcb->context.eax,next_tcb->context.ebp);
	//main_esp=get_esp();
	_schedule_now=&(now->context);
	_schedule_next=&(next_tcb->context);
	//switch_get(_schedule_now);
	//active_task(next_tcb);
	if(next_tcb->tid==4)
	{
		// printf("kernel stack addr:0x%x ebp:0x%x esp:0x%x\n",next_tcb+4095,next_tcb->context.ebp,next_tcb->context.esp);
		// printhex((uint32_t)next_tcb+4095-20,5);
	}
	active_task(next_tcb);
	//switch_with_pdt(_schedule_now,_schedule_next);
	//active_task(next_tcb);
	//tss_update(get_esp());
	//printf("pdt already changed!");
	switch_to(_schedule_now,_schedule_next);  
	
}

int copy_fd(int fd)
{
	vfs_file_t*f= thread_get_fd(fd);
	if(!f)
		return -1;
    int n=thread_add_fd(f);
	if(n<0)
		return -1;
	f->ref_cnt++;
	return n;
}


int user_dup(int oldfd,int newfd)
{
	if(oldfd==newfd)
		return oldfd;
	if(newfd==-1)
	{
		newfd=copy_fd(oldfd);
		return newfd;
	}else
	{
		int tmp=copy_fd(oldfd);
		if(tmp<0)
			return -1;
		if(thread_get_fd(newfd))
		{
			if(sys_close(newfd)<0)
			{
				get_running_progress()->fd_list[tmp]=0;
				return -1;
			}
		}
		get_running_progress()->fd_list[newfd]=get_running_progress()->fd_list[tmp];
		get_running_progress()->fd_list[tmp]=0;
		return newfd;
	}
}

void thread_add_child(TCB_t*parent,TCB_t*child)
{
	child->parent_thread=parent;
	list_append(&parent->child_thread_list,&child->child_tag);
}
//block running threads , which must be called in kernel state(level 0)
//however,when the task is in user state(level 3),can use
//syscall to jump in kernel state, then invoke the function

void thread_block(){
    //cli and sti is used to sync to access the threads list and it`s node information
    TCB_t* now = get_running_progress();
    //bool condition=cli_condition();
	cli();
    now->task_status = TASK_BLOCKED;
    schedule();
    //reload the interrupt flag before block
    sti();
}
void thread_die(TCB_t*target_thread)
{
	cli();
	target_thread->task_status=TASK_DIED;
	sti();
}

void thread_wakeup(TCB_t * target_thread){
    int eflg=load_eflags();
	cli();
	if(target_thread->task_status==TASK_BLOCKED)
    	target_thread->task_status = TASK_READY;
    store_eflags(eflg);
}

void remove_thread(TCB_t* tmp){
	cli();
	if(tmp->tid==0)
		printf("ERRO:main thread can`t use function exit\n");
	else{
		TCB_t *temp = tmp;
		for(;temp->next!=tmp;temp=temp->next)
			;
		temp->next = tmp->next;
	}
}

void __freeing_mem_thread(TCB_t*now)
{
	//TCB_t *now=get_running_progress();
	//1.close fd_list
	//TODO
	kfree(now->fd_list);
	//2. freeing mapped memory
	if(!now->is_kern_thread)page_free_pdt(now->pdt_vaddr);
	
}
void user_wait()
{
	TCB_t* now = get_running_progress();
	cli();
    now->task_status = TASK_WAITING;
    schedule();
    sti();
}
void user_exit(){
	//cli and sti is used to sync to access the threads list and it`s node information
    TCB_t* now = get_running_progress();
    //bool condition=cli_condition();
	cli();
    now->task_status = TASK_DIED;
	if(now->parent_thread)
	{
		if(now->parent_thread->task_status==TASK_WAITING)thread_wakeup(now->parent_thread);
	}
    schedule();
    //reload the interrupt flag before block
    sti();
	//注意 暂时没有回收此线程页
}

void wait_child_threads()
{
	
}