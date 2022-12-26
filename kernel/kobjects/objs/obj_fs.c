
#include "kobjects/obj_vfs.h"
#include "fs/fs_iso9660.h"
#include "mem/malloc.h"
#include "console.h"
#include "mem/memorylayout.h"
#include "string.h"
#include "process/task.h"
#include "process/sync.h"
#include "string.h"
//#define __check_open_flag(flag,need_flag) (flag)&(need_flag)
lock_t fs_lock;
#define IN_LOCK lock_acquire(&fs_lock);
#define OUT_LOCK lock_release(&fs_lock);
slab_unit_t *inode_slab;
slab_unit_t *dir_elem_slab;
slab_unit_t *dentry_slab;
slab_unit_t *file_slab;
 list_t sb_list;
 vfs_super_block_t *root_sb;
 int pub__travel_dir_find(list_elem_t*elem,char * arg)
{
    
    vfs_dir_elem_t *n_dir_elem= elem2entry(vfs_dir_elem_t,list_tag,elem);
    vfs_inode_t*inode=inode2ptr(n_dir_elem->file);
    if(inode->magic_num!=INODE_MAGIC_NUM)
    {
        printf("[ISO] Bad magic num!\n");
        return 0;
        //return 1;
    }
    //printf("Find a %s, name: %s max_byte:%d;\n",((char *[]){"file","dir"})[inode->file_type],n_dir_elem->name,inode->size_in_byte);   
    if(!strcmp(n_dir_elem->name,arg))return 1;
    return 0;
}
vfs_dir_elem_t *pub_dentry_find(vfs_dentry_t *dir,char *name)
{
    list_elem_t *elem= list_traversal(&dir->file_elems,pub__travel_dir_find,name);
    if(!elem)return 0;
    return elem2entry(vfs_dir_elem_t,list_tag,elem);
}
//extern slab_unit_t *file_slab;
 vfs_sb_ops_t *fs_ops_list[FS_MAX_NUM];
vfs_inode_t *vfs_alloc_inode()
{
    return alloc_in_slab_unit(inode_slab);
}
vfs_dentry_t *vfs_alloc_dentry()
{
    return alloc_in_slab_unit(dentry_slab);
}
vfs_dir_elem_t *vfs_alloc_delem()
{
    return alloc_in_slab_unit(dir_elem_slab);
}
vfs_file_t*vfs_alloc_file()
{
    return alloc_in_slab_unit(file_slab);
}
//#define vfs_alloc_file 
int mount_root_sb()
{
    root_sb = kmalloc(sizeof(vfs_super_block_t));
    if (!root_sb)
        return -1;
    root_sb->disk_id = 0;
    root_sb->ops = fs_ops_list[0];
    list_init(&root_sb->inode_list);
    list_init(&root_sb->dentry_list);
    return root_sb->ops->fs_mount(root_sb,device_find("ata0"));
}

int init_vfs()
{
    if (!fs_ops_list[0] || !root_sb)
        return -1;
    // We must init the vfs after root_sb is inited!
}

int __get_inode_type(inode_handle file)
{
    vfs_inode_t *inode = inode2ptr(file);
    if (inode->magic_num != INODE_MAGIC_NUM)
        return -1;
    return inode->file_type;
}
vfs_super_block_t *__get_inode_sb(inode_handle file)
{
    vfs_inode_t *inode = inode2ptr(file);
    if (inode->magic_num != INODE_MAGIC_NUM)
        return 0;
    return inode->sb;
}

#define __CHAR_ROOT 1
#define __CHAR_NONE 0
char *__travel_path(char *path, int *cnt, int len)
{
    // printf("cnt:%d;",*cnt);
    path = (char *)((uint32_t)path + *cnt);
    char *old_path = path;
    
    if (*cnt >= len)
    {
        return __CHAR_NONE;
    }
    while (1)
    {
        if (*cnt >= len)
        {
            return old_path;
        }
        if (*path == '/')
        {

            *path = '\0';
            (*cnt)++;
            // printf("cnt:%d;",*cnt);
            if (*cnt == 1)
            {
                
                return __CHAR_ROOT;
            }
            return old_path;
        }
        (*cnt)++;
        path++;
    }
}

vfs_dentry_t *__copy_dentry(vfs_dentry_t *parent)
{
    vfs_super_block_t *sb = __get_inode_sb(parent->dir_file);
    if (!sb)
    {
        printf("No sb found in inode!;");
        return 0;
    }
    vfs_dentry_t *dentry = vfs_alloc_dentry();
    if (!dentry)
    {
        printf("Alloc err;");
        return 0;
    }
    dentry->parent_dir = parent;
    dentry->name_len = parent->name_len;
    dentry->ops = parent->ops;
    list_init(&dentry->file_elems);
    list_append(&sb->dentry_list, &dentry->dentry_elem);
    return dentry;
}
vfs_dentry_t* __search_dir(char *path)
{
    int t_cnt = 0;
    int len = strlen(path);
    uint8_t root_check = __travel_path(path, &t_cnt, len);
    if (root_check != __CHAR_ROOT)
    {
        printf("[SEARCHDIR]Cant locate '/' in the path;%s",path);
        return 0;
    }
    char *sub_path;
    vfs_dentry_t *cur_entry = root_sb->root_dir;
    vfs_dir_elem_t *d_elem;
    while ((sub_path = __travel_path(path, &t_cnt, len)) != __CHAR_NONE)
    {
        if (!strcmp(sub_path, "."))
        {
        }
        else if (!strcmp(sub_path, ".."))
        {
            cur_entry = cur_entry->parent_dir;
        }
        else
        {
            d_elem = pub_dentry_find(cur_entry, sub_path);
            if (!d_elem)
            {
                printf("Cant find sub_dir(file):%s;", sub_path);
                return 0;
            }
            int type = __get_inode_type(d_elem->file);
            if (type == -1)
            {
                printf("Bad inode format!;");
                return 0;
            }
            else if(type==VFS_INODE_TYPE_DIR)
            {
                // printf("%s is a dir:%x;",sub_path,d_elem->d_dir);
                if (!d_elem->d_dir)
                {
                    d_elem->d_dir = __copy_dentry(cur_entry);
                    if (!d_elem->d_dir)
                    {
                        printf("Cant copy dentry!;");
                        return 0;
                    }
                    d_elem->d_dir->dir_file = d_elem->file;
                    if(d_elem->d_dir->ops->load_inode_dirs)
                    {
                    if (d_elem->d_dir->ops->load_inode_dirs(d_elem->d_dir, d_elem->file) < 0)
                    {
                        printf("Cant load all d_elem!%s;", sub_path);
                        return 0;
                    }
                    }
                }
                cur_entry = d_elem->d_dir;
            }else
            {
                printf("NOT A DIRECTORY!");
                return 0;
            }
        }
    }
    return cur_entry;
}
vfs_dir_elem_t *__search_file_elem(char *path)
{
    int t_cnt = 0;
    int len = strlen(path);
    uint8_t root_check = __travel_path(path, &t_cnt, len);
    if (root_check != __CHAR_ROOT)
    {
        printf("Cant locate '/' in the path;%s",path);
        return 0;
    }
    char *sub_path;
    vfs_dentry_t *cur_entry = root_sb->root_dir;
    vfs_dir_elem_t *d_elem;
    while ((sub_path = __travel_path(path, &t_cnt, len)) != __CHAR_NONE)
    {
        // printf("sub:%s;",sub_path);
        if (!strcmp(sub_path, "."))
        {
        }
        else if (!strcmp(sub_path, ".."))
        {
            cur_entry = cur_entry->parent_dir;
        }
        else
        {
            d_elem = pub_dentry_find(cur_entry, sub_path);
            if (!d_elem)
            {
                printf("Cant find sub_dir(file):%s %s;",path, sub_path);
                return 0;
            }
            int type = __get_inode_type(d_elem->file);
            if (type == -1)
            {
                printf("Bad inode format!;");
                return 0;
            }
            else if (type == VFS_INODE_TYPE_FILE)
            {
                // No more path after this
                if (__travel_path(path, &t_cnt, len) != __CHAR_NONE)
                {
                    printf("Bad path format!;");
                    return 0;
                }
                //*f_type=type;
                return d_elem;
            }
            else
            {
                
                if (!d_elem->d_dir)
                {
                    d_elem->d_dir = __copy_dentry(cur_entry);
                    if (!d_elem->d_dir)
                    {
                        printf("Cant copy dentry!;");
                        return 0;
                    }
                    d_elem->d_dir->dir_file = d_elem->file;
                    if(d_elem->d_dir->ops->load_inode_dirs)
                    {
                    if (d_elem->d_dir->ops->load_inode_dirs(d_elem->d_dir, d_elem->file) < 0)
                    {
                        printf("Cant load all d_elem!%s;", sub_path);
                        return 0;
                    }
                    }
                }
                cur_entry = d_elem->d_dir;
            }
        }
    }
    printf("Not a file!;");
    return 0;
}
/**
 * TODO:A LOT FOR MMAP
*/
int vfs_mmap(vfs_file_t *file, void*starts,uint32_t length,int offset,int flag)
{
    printf("in vfs mmap!");
    vfs_inode_t *inode = inode2ptr(file->content->file);
    return inode->i_ops->fs_mmap(&inode->inode_ptr,starts,length,offset,flag);
}
int vfs_fread(vfs_file_t *file, uint32_t size, uint32_t buffer)
{
    if (file->open_flag & O_RDONLY || file->open_flag & O_RDWR)
    {
        vfs_inode_t *inode = inode2ptr(file->content->file);
        if (inode->magic_num != INODE_MAGIC_NUM)
            return VFS_FORMAT_ERR;
        inode->seek_offset=file->lseek;
        int r= inode->i_ops->fs_read(&inode->inode_ptr, size, buffer, 0);
        file->lseek=inode->seek_offset;
        return r;
    }
    return VFS_PREMISSION_ERR;
}
int vfs_fwrite(vfs_file_t *file, uint32_t size, uint32_t buffer)
{
    if (file->open_flag & O_WRONLY || file->open_flag & O_RDWR)
    {
        vfs_inode_t *inode = inode2ptr(file->content->file);
        if (inode->magic_num != INODE_MAGIC_NUM)
            return VFS_FORMAT_ERR;
        inode->seek_offset=file->lseek;
        //printf("ipswrite:0x%x",inode->i_ops->fs_write);
        int r= inode->i_ops->fs_write(&inode->inode_ptr, size, buffer, 0);
        file->lseek=inode->seek_offset;
        return r;
    }
    return VFS_PREMISSION_ERR;
}
int vfs_tell(vfs_file_t *file)
{
    // vfs_inode_t *inode = inode2ptr(file->content->file);
    // if (inode->magic_num != INODE_MAGIC_NUM)
    //     return VFS_FORMAT_ERR;
    // return inode->seek_offset;
    return file->lseek;
}
int __travel_dir_print(list_elem_t*elem,char * arg)
{
    
    vfs_dir_elem_t *n_dir_elem= elem2entry(vfs_dir_elem_t,list_tag,elem);
    vfs_inode_t*inode=inode2ptr(n_dir_elem->file);
    //printf("Ttt");
    if(inode->magic_num!=INODE_MAGIC_NUM)
    {
        printf("[VFS] Bad magic num!\n");
        return 0;
        //return 1;
    }
    printf("Find a ");
    if(inode->file_type==VFS_INODE_TYPE_DIR)Klogger->setcolor(0x000000,0x6495ED);
    printf("%s",((char *[]){"file","dir"})[inode->file_type]);
    Klogger->setcolor(0x000000,0xffffff);
    printf(", name: ");
    Klogger->setcolor(0x000000,0xffff00);
    printf("%s",n_dir_elem->name);
    Klogger->setcolor(0x000000,0xffffff);
    printf(" max_byte:%d;\n",inode->size_in_byte);   
    //if(!strcmp(n_dir_elem->name,arg))return 1;
    return 0;
}
void vfs_print_dir(char *path)
{
char *old_path=strdup(path);
    vfs_dentry_t*dir= __search_dir(old_path);
    kfree(old_path);
    if(!dir)return NULL;
    //printf("pppppp");
    list_traversal(&dir->file_elems,__travel_dir_print,0);
}
vfs_dir_elem_t* vfs_mkvdir(char *root_path,char *name,void* elem)
{
    char *old_path=strdup(root_path);
    vfs_dentry_t*dir= __search_dir(old_path);
    kfree(old_path);
    if(!dir)return NULL;
    char *name_buf=kmalloc(dir->name_len+1);
    if(!name_buf)
    {
        return NULL;
    }
    memset(name_buf,0,dir->name_len+1);
    strcpy(name_buf,name);
    vfs_dir_elem_t *n_dir_elem=vfs_alloc_delem();
    vfs_inode_t*pinode=inode2ptr(dir->dir_file);
    if(pinode->magic_num!=INODE_MAGIC_NUM)
    {
        printf("PATH BROKEN!\n");
        kfree(name_buf);
        return NULL;
    }
    if(!n_dir_elem)
    {
        kfree(name_buf);
        return NULL;
    }
    
    n_dir_elem->name=name_buf;
    n_dir_elem->ref_counter=0;
    vfs_inode_t*inode=vfs_alloc_inode();
    inode->file_type=elem?VFS_INODE_TYPE_FILE: VFS_INODE_TYPE_DIR;
    inode->v_type=VFS_INODE_VTYPE_DEV;
    inode->magic_num=INODE_MAGIC_NUM;
    inode->sb=root_sb;
    inode->inode_ptr=elem;
    inode->i_ops=&dev_ops;
    n_dir_elem->file=&inode->inode_ptr;
    //inode->i_ops->fs_mmap=dev_ops.fs_mmap;
    if(!elem)
    {
        n_dir_elem->d_dir=__copy_dentry(root_sb->root_dir);
        
        if(n_dir_elem->d_dir)
        {
            n_dir_elem->d_dir->ops=0;
            n_dir_elem->d_dir->dir_file=n_dir_elem->file;
        }

    }
    
    //n_dir_elem->d_dir
    list_append(&pinode->sb->inode_list,&inode->inode_elem);
    list_append(&dir->file_elems,&n_dir_elem->list_tag);
    return n_dir_elem;

}
int vfs_lseek(vfs_file_t *file, uint32_t offset, uint32_t source)
{
    vfs_inode_t *inode = inode2ptr(file->content->file);
    if (inode->magic_num != INODE_MAGIC_NUM)
        return VFS_FORMAT_ERR;
    inode->seek_offset=file->lseek;
    inode->i_ops->fs_lseek(&inode->inode_ptr, source, offset);
    file->lseek=inode->seek_offset;
    return 0;
}
int vfs_close(vfs_file_t *file)
{
    vfs_inode_t *inode = inode2ptr(file->content->file);
    if (inode->magic_num != INODE_MAGIC_NUM)
        return VFS_FORMAT_ERR;
    
}
vfs_file_ops_t file_ops = {
    .read = vfs_fread,
    .tell = vfs_tell,
    .lseek = vfs_lseek,
    .write = vfs_fwrite,
    .mmap=vfs_mmap
    };
vfs_file_t *__vfs_check_reopen(vfs_dir_elem_t *elem)
{
    TCB_t *cur = get_running_progress();
    for (int i = 0; i < 20; i++)
    {
        vfs_file_t*f=cur->fd_list[i];
        if(f->content==elem)return f;
    }
    return 0;

    // fastmapper
}

vfs_file_t *vfs_fopen(char *path, uint8_t flag)
{
    // int t;
    IN_LOCK
    //printf("vfs:%s",path);
    //printf("s0s0s0s0");
    vfs_dir_elem_t *d_elem = __search_file_elem(path);
    if (!d_elem)
    {
        OUT_LOCK
        return 0;
    }
    //printf("s1s1s1s1");
    // uint32_t reopen=__vfs_check_reopen(d_elem);
    // if (reopen)
    // {
    //     OUT_LOCK
    //     return reopen;
    // }
    vfs_file_t *f = vfs_alloc_file();
    if (!f)
    {
        OUT_LOCK
        return 0;
    }
    f->content = d_elem;
    f->owner_ptr = get_running_progress();
    f->open_flag = flag;
    f->ops = &file_ops;
    f->lseek = 0;
    // f->ref_count=1;
    d_elem->ref_counter++;
    OUT_LOCK
    return f;
}
int sys_read(int fd, char *buffer, uint32_t size)
{
    //printf("IN SYSREAD");
    //printf("[fd is %d]",fd);
    if (fd < 0)
        return VFS_BAD_ARG_ERR;
    IN_LOCK
    vfs_file_t *f = thread_get_fd(fd);
    if (!f)
    {
        OUT_LOCK
        return VFS_NULL_OBJECT_ERR;
    }
    
    int r = f->ops->read(f, size, buffer);
    OUT_LOCK
    return r;
}
int sys_mmap(void*starts,uint32_t length,int fd,int offset,int flag)
{
        if (fd < 0)
        return VFS_BAD_ARG_ERR;
    IN_LOCK
    vfs_file_t *f = thread_get_fd(fd);
    if (!f)
    {
        OUT_LOCK
        return VFS_NULL_OBJECT_ERR;
    }
    int r = f->ops->mmap(f,starts,length,offset,flag);
    OUT_LOCK
    return r;
}
int sys_write(int fd, char *buffer, uint32_t size)
{
    if (fd < 0)
        return VFS_BAD_ARG_ERR;
    IN_LOCK
    vfs_file_t *f = thread_get_fd(fd);
    if (!f)
    {
        OUT_LOCK
        return VFS_NULL_OBJECT_ERR;
    }
    int r = f->ops->write(f, size, buffer);
    OUT_LOCK
    return r;
}
int sys_tell(int fd)
{
    if (fd < 0)
        return VFS_BAD_ARG_ERR;
    IN_LOCK
    vfs_file_t *f = thread_get_fd(fd);
    if (!f)
    {
        OUT_LOCK
        return VFS_NULL_OBJECT_ERR;
    }
    
    int r = f->ops->tell(f);
    OUT_LOCK
    return r;
}
int sys_lseek(int fd, uint32_t offset, uint8_t base)
{
    if (fd < 0)
        return VFS_BAD_ARG_ERR;
    IN_LOCK
    vfs_file_t *f = thread_get_fd(fd);
    if (!f)
    {
        OUT_LOCK
        return VFS_NULL_OBJECT_ERR;
    }
    int r = f->ops->lseek(f, offset, base);
    OUT_LOCK
    return r;
}
int sys_close(int fd)
{

}
int sys_open(char *path, uint8_t flag)
{
    if (!path)
        return VFS_BAD_ARG_ERR;
    if(strlen(path)>=120)
        return VFS_BAD_ARG_ERR;
    char *buf_path=strdup(path);
    path=buf_path;
    IN_LOCK
    vfs_file_t *f = vfs_fopen(path, flag);
    kfree(buf_path);
    if (!f)
    {
        OUT_LOCK
        
        return VFS_NULL_OBJECT_ERR;
    }
    //printf("fd open ok!;");
    int fd = thread_add_fd(f);
    OUT_LOCK
    if (fd < 0)
        return VFS_ALLOC_ERR;
    return fd;
}
int init_fslist()
{
    lock_init(&fs_lock);
    inode_slab = alloc_slab_unit(sizeof(vfs_inode_t), "vfs_inode");
    dentry_slab = alloc_slab_unit(sizeof(vfs_dentry_t), "vfs_dentry");
    dir_elem_slab = alloc_slab_unit(sizeof(vfs_dir_elem_t), "vfs_dir_elem");
    file_slab = alloc_slab_unit(sizeof(vfs_file_t), "vfs_file");
    if (!inode_slab || !dentry_slab || !dir_elem_slab)
    {
        printf("[VFS]: Fail to alloc inode_slab or dentry_slab!\n");
        return -1;
    }
    list_init(&sb_list);
    fs_ops_list[0] = iso_getops();
    mount_root_sb();
    list_append(&sb_list, &root_sb->elem);
    vfs_mkvdir("/","dev",0);
    vfs_print_dir("/");
    printf("test iso:\n");
    int fd = sys_open("/boot/grub/./../grub/grub.cfg", O_RDONLY);
    if (fd)
    {
        printf("[VFS] open cfg ok!;");
        char *text = kmalloc(30);
        int s = sys_read(fd,text,30);
        printf("[%s][%d]", text, s);
    }

    return 1;
}
int kread_all(char *path, uint32_t *vaddr, int *pgnum)
{
    int fd = sys_open(path, O_RDONLY);
    if (fd < 0)
        return fd;
    sys_lseek(fd, 0, SEEK_END);
    uint32_t sz = sys_tell(fd);
    sys_lseek(fd, 0, SEEK_SET);
    uint32_t pg_num = ngx_align(sz, 4096) / 4096;
    char *buffer = kmalloc_page(pg_num);
    sys_read(fd, buffer, sz);
    *pgnum = pg_num;
    // printf("load %d bytes:%s;",sz,buffer);
    *vaddr = buffer;
    return fd;
}