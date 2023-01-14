//#include"kobjects/obj_vfs.h"
#include"kobjects/kobjs.h"
#include"utils/circlequeue.h"
#include"process/task.h"
#include"console.h"
#define MAX_BUF_LEN 200
char stdin_buf[MAX_BUF_LEN+1];
int stdin_pointer=0;
TCB_t *stdin_wait_queue=NULL;
void init_stdio_obj()
{
    memset(stdin_buf,0,MAX_BUF_LEN+1);
}
uint32_t stdin_write(struct vfs_file*file,uint32_t size,uint8_t *buffer)
{
    if(*buffer=='\b')
    {
        if(stdin_pointer>0)stdin_pointer--;
    }
    else if(stdin_pointer<MAX_BUF_LEN+1)
    {
        stdin_buf[stdin_pointer]=(*buffer);
        stdin_pointer++;
    }
    //printf("wake up%s;",stdin_wait_queue->name);
    if(stdin_wait_queue)
    {
        thread_wakeup(stdin_wait_queue);
    } 
    
}
uint32_t stdin_read(struct vfs_file*file,uint32_t size,uint8_t *buffer)
{
    //printf("[last %c]",stdin_buf[stdin_pointer-1]);
    if(stdin_buf[stdin_pointer-1]=='\n')
    {
        memcpy(buffer,stdin_buf,stdin_pointer-1);
        buffer[stdin_pointer-1]='\0';
        //printf("copy finish!");
        memset(stdin_buf,MAX_BUF_LEN+1,0);
        
        stdin_pointer=0;
        stdin_wait_queue=0;
        //printf("ready to return");
        return size;
    }
    if(stdin_pointer>=size)
    {
        //printf("ready to copy to 0x%x from 0x%x",buffer,stdin_buf);
        memcpy(buffer,stdin_buf,size);
        //printf("copy finish!");
        memset(stdin_buf,MAX_BUF_LEN+1,0);
        
        stdin_pointer=0;
        stdin_wait_queue=0;
        //printf("ready to return");
        return size;
    }
    stdin_wait_queue=get_running_progress();
    return stdin_pointer;
        // 
        // thread_block();
}
vfs_file_ops_t stdin_ops={
    .write=stdin_write,
    .read=stdin_read
};
vfs_file_t* create_stdin_file()
{
    vfs_file_t*f =vfs_alloc_file();
    if(!f)return NULL;
    f->ops=&stdin_ops;
    f->open_flag=O_RDWR;
    return f;
}