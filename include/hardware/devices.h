#ifndef _H_DEVICES
#define _H_DEVICES
#include"types.h"
#include"kobjects/kobject.h"
enum
{
    KDEV_BLOCK,
    KDEV_CHAR,
    KDEV_LOOP
};
enum
{
    KDEV_ATA,
    KDEV_ATAPI,
    KDEV_RAMDISK,
    KDEV_TTY,
    KDEV_KBD,
    KDEV_MOUSE,
    KDEV_FB
};

struct kdevice_ops;
typedef struct kdevice
{
    char name[20];
    char root_type;
    char type;
    int dev_id;
    kobject_t* hardware;
    uint32_t unit_size;
    struct kdevice_ops *ops;
    uint32_t extra_data_sz;
    char *data;
}kdevice_t;
typedef struct kdevice_ops
{
    int(*mmap)(struct kdevice*self, void*starts,uint32_t length,int offset,int flag);
    int(*read)(struct kdevice *self, uint32_t addr,uint32_t num,char *buffer,int flag);
    int(*write)(struct kdevice *self, uint32_t addr,uint32_t num,char *buffer,int flag);
}kdevice_ops_t;
#define KDEV_MAX 15

int device_init();
void device_enum2();

kdevice_t *device_create(char *name,char root_type,char type,int dev_id,kobject_t*hardware,uint32_t usize,
    kdevice_ops_t*ops,
    char *data,uint32_t extra_data_sz);
int device_add(kdevice_t*dev);
kdevice_t* device_find(char *name);
//int device_dentry_inode_load(vfs_dentry_t*dir, inode_handle file);
#endif