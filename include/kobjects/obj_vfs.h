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
#include"types.h"

typedef struct vfs_dirents
{
    char *name;
    uint8_t name_len;
}vfs_dirents_t;
#define inode_handle uint32_t
typedef struct inode_ops
{
    inode_handle (*fs_open)(inode_handle parent_inode,vfs_dirents_t *parent_dirs,uint32_t idx,uint32_t flag);
    int (*fs_close)(inode_handle file);
    int (*fs_read)(inode_handle file,uint32_t size,void* buffer, uint32_t flag);
    int (*fs_open)(inode_handle file,uint32_t size,void* buffer, uint32_t flag);
    int (*fs_sleek)(inode_handle file,uint32_t source,uint32_t offset);
    vfs_dirents_t* (*fs_opendir)(inode_handle parent_inode,int *dir_cnt);
    void (*fs_closedir)(vfs_dirents_t*dirs,int dir_cnt);
}inode_ops_t;



typedef struct vfs_super_block
{
    char fs_name[20];
    uint8_t version[2];


}vfs_super_block_t;



#endif