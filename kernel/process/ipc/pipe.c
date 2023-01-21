#include"process/ipc/pipe.h"
#include"process/task.h"
#include"mem/malloc.h"
#define PIPE_SIZE 4000
typedef struct 
{
    int left;
    int right;
    int ref;
    char *buf;
}kpipe_content_t;
slab_unit_t *kpipe_slab=0;
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
    }else
    {
        memcpy(kp->buf+kp->right,buffer,size);
        kp->right+=size;
        if(kp->right==PIPE_SIZE)kp->right=0;
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
            return old;
        }else
        {
            memcpy(buffer,kp->buf+kp->left+1,rem);
            size-=rem;
            buffer+=rem;
            memcpy(buffer,kp->buf,size);
            kp->left=size-1;
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
    
    return 1;
}