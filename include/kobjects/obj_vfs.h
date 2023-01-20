/**
 * @file obj_fs.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief Filesystem Manager
 * @version 0.1
 * @date 2022-08-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _H_FS
#define _H_FS
#ifndef _USER_LIB
#include"types.h"
#include"mem/malloc.h"
#include"hardware/devices.h"
#include"list.h"
struct vfs_super_block;
struct vfs_file;
#define inode_handle uint32_t
struct vfs_dentry;
#define VFS_INODE_TYPE_FILE 0
#define VFS_INODE_VTYPE_DEV 1
#define VFS_INODE_TYPE_DIR 1
#endif
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define O_DRVONLY 0
#define O_RDONLY 1
#define O_WRONLY 1<<1
#define O_RDWR O_RDONLY|O_WRONLY
#define O_APPEND 1<<3
#define O_TRUNC 1<<4
#define O_CREAT 1<<5
#define O_EXCL 1<<6
#define O_SYNC 1<<7
typedef struct 
{
    /* data */
}udirent_;

#ifndef _USER_LIB
enum
{
    INODE_LSEEK_OFFSET_MODE,
    INODE_LSEEK_ABS_MODE
};
enum{
    VFS_ALLOC_ERR=-20,
    VFS_FORMAT_ERR,
    VFS_HARDLOAD_ERR,
    VFS_INODE_ERR,
    VFS_DENTRY_ERR,
    VFS_SB_ERR,
    VFS_NULL_OBJECT_ERR,
    VFS_LBA_ERR,
    VFS_SIZE_ERR,
    VFS_PREMISSION_ERR,
    VFS_BAD_ARG_ERR
};


typedef struct vfs_dir_elem
{
    char *name;
    //uint8_t name_len;
    //uint8_t type_tag;
    inode_handle file;

    /**
     * @brief NOTICE
     * Remember, we dont need a double-direction pointer
     * It means that only fd(file descriptions) know the elem but
     * elem doesnt no any fd which is related to him
     * It just knows how many fds are related to it
     * Simply notice that...
     */
    struct vfs_dentry *d_dir;
    // union
    // {
    //     struct vfs_dentry *d_dir;
    //     struct vfs_file *d_file;
    // };
    
    list_elem_t list_tag;
}vfs_dir_elem_t;
typedef struct 
{
    vfs_dir_elem_t*root_enum_dir;
    int enum_times;
    vfs_dir_elem_t*current_enum_dir;
}vfs_DIR_t;
typedef struct vfs_dentry_ops
{
    vfs_dir_elem_t* (*find_dentry)(struct vfs_dentry *dir,char *name);
    int (*load_inode_dirs)(struct vfs_dentry*dir, inode_handle file);
    void (*delete_dentry)(struct vfs_dentry*dir,struct vfs_dentry *d_elem);
    vfs_dir_elem_t* (*add_delem)(struct vfs_dentry* dir,char *fname);
    void (*release_dentry)(struct vfs_dentry*dir);
}vfs_dentry_ops_t;
typedef struct inode_ops
{
    //inode_handle (*fs_open)(inode_handle parent_dir,uint32_t idx,uint32_t flag);
    int (*fs_close)(inode_handle file);
    int (*fs_read)(inode_handle file,uint32_t size,void* buffer, uint32_t flag);
    int (*fs_write)(inode_handle file,uint32_t size,void* buffer, uint32_t flag);
    int (*fs_lseek)(inode_handle file,uint32_t source,uint32_t offset);
    int(*fs_mmap)(inode_handle file, void*starts,uint32_t length,int offset,int flag);
    //vfs_dirents_t* (*fs_opendir)(inode_handle parent_inode,int *dir_cnt);
    //void (*fs_closedir)(vfs_dirents_t*dirs,int dir_cnt);
}inode_ops_t;

typedef struct vfs_inode
{
    uint16_t magic_num;
    uint32_t refer_count;   //guess what
    uint32_t sync_mark;   
    uint32_t size_in_byte;
    uint8_t v_type;
    uint8_t file_type;
    /**
     * @brief TODO
     * Time and rights for the file
     */
    uint32_t seek_offset;
    struct vfs_super_block *sb; 
    inode_ops_t *i_ops;
    uint32_t inode_ptr;
    list_elem_t inode_elem;
    struct vfs_inode *parent_inode;
}vfs_inode_t;
#define INODE_MAGIC_NUM 0x10DE





typedef struct vfs_dentry
{
    //vfs_name_match_t *name_matches;
    uint8_t name_len;
    //uint32_t dir_data_size_in_byte;
    list_t file_elems;
    list_elem_t dentry_elem;
    inode_handle dir_file;
    vfs_dentry_ops_t *ops;
    struct vfs_dentry *parent_dir;
}vfs_dentry_t;

typedef struct vfs_sb_ops
{
    char class_name[16];
    int (*fs_mount)(struct vfs_super_block *sb,kdevice_t*dev);
    void (*fs_umount)(struct vfs_super_block *sb);
}vfs_sb_ops_t;


typedef struct vfs_super_block
{
    char fs_name[20];
    uint8_t version[2];
    uint32_t fs_capacity_in_kb;
    uint32_t disk_id;
    vfs_sb_ops_t *ops;
    inode_handle root_inode;
    vfs_dentry_t *root_dir;
    list_t dentry_list;
    list_t inode_list;
    list_elem_t elem;
    kdevice_t *dev;
    char *self_data;
    uint32_t self_data_size;
}vfs_super_block_t;

typedef struct vfs_file_ops
{
    //int (*open)(struct vfs_file *file);
    //int (*close)(struct vfs_file*file);
    int (*read)(struct vfs_file*file,uint32_t size,uint8_t *buffer);
    int (*write)(struct vfs_file*file,uint32_t size,uint8_t *buffer);
    int (*lseek)(struct vfs_file*file,uint32_t offset,uint8_t base);
    int (*tell)(struct vfs_file*file);
    int (*close)(struct vfs_file *file);
    int (*mmap)(struct vfs_file *file, void*starts,uint32_t length,int offset,int flag);
}vfs_file_ops_t;
struct vfs_file
{
    //vfs_super_block_t*sb;
    vfs_dir_elem_t *content;
    vfs_file_ops_t *ops;
    uint32_t lseek;
    uint32_t open_flag;
    uint32_t owner_ptr;//record sth about who opened this file
    uint32_t ref_cnt;
};
extern inode_ops_t dev_ops;
typedef struct vfs_file vfs_file_t;
#define FS_MAX_NUM 4
#define inode2ptr(inode) elem2entry(vfs_inode_t,inode_ptr,(inode));
extern list_t sb_list;
extern vfs_super_block_t *root_sb;
extern slab_unit_t *file_slab;
extern vfs_sb_ops_t *fs_ops_list[FS_MAX_NUM];
extern slab_unit_t *sb_slab;
int init_fslist();
vfs_inode_t* vfs_alloc_inode();
vfs_dentry_t* vfs_alloc_dentry();
vfs_dir_elem_t *vfs_alloc_delem();
vfs_file_t*vfs_alloc_file();
int vfs_mount_subfs(int ops_idx,char *mount_path,char *dir_name,kdevice_t*target_dev);
int init_vfs();
void vfs_print_dir(char *path);
vfs_dir_elem_t* vfs_mkvdir(char *root_path,char *name,void*elem);
vfs_file_t *vfs_fopen(char *path,uint8_t flag);
int vfs_add_fsops(vfs_sb_ops_t *ops);
int sys_open(char *path,uint8_t flag);
int sys_read(int fd,char *buffer,uint32_t size);
int sys_mmap(void*starts,uint32_t length,int fd,int offset,int flag);
int sys_write(int fd, char *buffer, uint32_t size);
int sys_lseek(int fd,uint32_t offset,uint8_t base);
int sys_tell(int fd);
int sys_close(int fd);
int kread_all(char *path,uint32_t *vaddr,int *pgnum);

#endif
#endif