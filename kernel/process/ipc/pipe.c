#include"process/ipc/pipe.h"
#include"process/task.h"
#include"mem/malloc.h"
#include"console.h"
#include"hardware/timer.h"
#define PIPE_SIZE 4000
typedef struct 
{
    int left;
    int right;
    int ref;
    char *buf;
    vfs_file_t *matched_file;
}kpipe_content_t;
slab_unit_t *kpipe_slab=0;
void pipe_remind(vfs_file_t*file)
{
    // printf("waking up (pipe) 0x%x 0x%x",file,file->content);
    // kpipe_content_t*ct=file->content;
    // if(ct->matched_file)
    // {
    //     printf("going to wakeup thread:[%s] ... ",((TCB_t*)ct->matched_file->owner_ptr)->name);
    //     //ksleep(2000);
    //     thread_wakeup(ct->matched_file->owner_ptr);
    //     printf("Wakeok\n");
    // }
}
int pipe_write(struct vfs_file*file,uint32_t size,uint8_t *buffer)
{
    kpipe_content_t *kp=file->content;
    if(!kp)
        return VFS_NULL_OBJECT_ERR;
    if(kp->right==kp->left)
        return VFS_FULL_ERR;//Full
    if(PIPE_SIZE-kp->right<size)
    {
        int rem=size-(PIPE_SIZE-kp->right);
        if(kp->right<kp->left)
            return -1;
        //if(kp->left==0&&rem)return -1;
        if(kp->left<rem)
            return -1;
        memcpy(kp->buf+kp->right,buffer,PIPE_SIZE-kp->right);
        buffer+=PIPE_SIZE-kp->right;
        size-=PIPE_SIZE-kp->right;
        kp->right=0;
        rem=size;
        memcpy(kp->buf+kp->right,buffer,rem);
        kp->right+=rem;
        pipe_remind(file);
    }else
    {
        memcpy(kp->buf+kp->right,buffer,size);
        kp->right+=size;
        if(kp->right==PIPE_SIZE)kp->right=0;
        pipe_remind(file);
    }
    return 1;

}
int pipe_read(struct vfs_file*file,uint32_t size,uint8_t *buffer)
{
    kpipe_content_t *kp=file->content;
    if(!kp)
        return -1;
    if(kp->right>kp->left)
    {
        if(size>kp->right-kp->left-1)
            size=kp->right-kp->left-1;
        memcpy(buffer,kp->buf+kp->left+1,size);
        kp->left+=size;
        pipe_remind(file);
        return size;
    }else
    {
        int rem=PIPE_SIZE-kp->left-1;
        if(rem+kp->right<size)
            size=rem+kp->right;
        uint32_t old=size;
        if(size<=rem)
        {
            memcpy(buffer,kp->buf+kp->left+1,size);
            kp->left+=size;
            pipe_remind(file);
            return old;
        }else
        {
            memcpy(buffer,kp->buf+kp->left+1,rem);
            size-=rem;
            buffer+=rem;
            memcpy(buffer,kp->buf,size);
            kp->left=size-1;
            pipe_remind(file);
            return old;
        }
    }
}
int pipe_close(vfs_file_t*file)
{
    if(file->ref_cnt)
    {
        file->ref_cnt--;
        return 1;
    }
    kpipe_content_t *pp=file->content;
    if(pp->matched_file)
    {
        kpipe_content_t *pp2=pp->matched_file->content;
        pp2->matched_file=0;
    }
    kfree(file);
    
    if(pp->ref)
    {
        pp->ref--;
    }else
    {
        kfree_page(pp->buf,1);
        kfree(pp);
    }
    return 1;
}
vfs_file_ops_t pipe_ops={
    .write=pipe_write,
    .read=pipe_read,
    .close=pipe_close
};
vfs_file_t *create_kpipe()
{
    if(!kpipe_slab)kpipe_slab=alloc_slab_unit(sizeof(kpipe_content_t),"kpipe_slab");
    if(!kpipe_slab)return NULL;    
    vfs_file_t*re=vfs_alloc_file();
    if(!re)return NULL;
    kpipe_content_t *kp=alloc_in_slab_unit(kpipe_slab);
    if(!kp)
    {
        kfree(re);
        return NULL;
    }
    kp->ref=0;
    re->owner_ptr=get_running_progress();
    re->lseek=0;
    re->content=kp;
    kp->buf=kmalloc_page(1);
    re->ops=&pipe_ops;
    if(!kp->buf)
    {
        kfree(re);
        kfree(kp);
        return NULL;
    }
    kp->left=0;
    kp->right=1;
    return re;
}


int user_pipe(int *fd)
{
    vfs_file_t*pp=create_kpipe();
    if(!pp)return -1;
    pp->open_flag=O_RDONLY;
    vfs_file_t*pp2=vfs_alloc_file();
    if(!pp2)
    {
        pp->ops->close(pp);
        return -1;
    }
    
    memcpy(pp2,pp,sizeof(vfs_file_t));
    pp2->open_flag=O_WRONLY;
    ((kpipe_content_t*)pp2->content)->ref++;
    fd[0]=thread_add_fd(pp);
    fd[1]=thread_add_fd(pp2);
    kpipe_content_t*ct1= pp->content;
    kpipe_content_t*ct2= pp2->content;
    ct1->matched_file=pp2;
    ct2->matched_file=pp;
    return 1;
}
int pipe_bind(int fd,TCB_t* task)
{
    vfs_file_t* f=thread_get_fd(fd);
    if(!f)
        return -1;
    f->owner_ptr=task;
    return 1;
}