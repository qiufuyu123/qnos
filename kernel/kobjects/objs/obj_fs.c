
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
    return root_sb->ops->fs_mount(root_sb);
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
    // uint32_t len=strlen(path);
    if (*cnt >= len)
        return __CHAR_NONE;
    while (1)
    {
        if (*cnt >= len)
            return old_path;
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
            d_elem = cur_entry->ops->find_dentry(cur_entry, sub_path);
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
                    if (d_elem->d_dir->ops->load_inode_dirs(d_elem->d_dir, d_elem->file) < 0)
                    {
                        printf("Cant load all d_elem!%s;", sub_path);
                        return 0;
                    }
                }
                cur_entry = d_elem->d_dir;
            }
        }
    }
    printf("Not a file!;");
    return 0;
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
    printf("vfs:%s",path);
    vfs_dir_elem_t *d_elem = __search_file_elem(path);
    if (!d_elem)
    {
        OUT_LOCK
        return 0;
    }
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
    printf("fd open ok!;");
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