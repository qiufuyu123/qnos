#include"kobjects/obj_vfs.h"
#include"mem/malloc.h"
#include"utils/fastmapper.h"
#include"console.h"
slab_unit_t* kdev_slab;
fastmapper_t dev_list;
int dev_read(inode_handle file,uint32_t size,void* buffer, uint32_t flag)
{
    if(!file)return -1;
    kdevice_t* dev=(kdevice_t*)(*((uint32_t*)file));
    vfs_inode_t*n=inode2ptr(file);
    return dev->ops->read(dev,n->seek_offset,size,buffer,flag);
}
int dev_write(inode_handle file,uint32_t size,void* buffer, uint32_t flag)
{
    if(!file)return -1;
    kdevice_t* dev=(kdevice_t*)(*((uint32_t*)file));
    vfs_inode_t*n=inode2ptr(file);
    return dev->ops->write(dev,n->seek_offset,size,buffer,flag);
}
int dev_mmap(inode_handle file, void*starts,uint32_t length,int offset,int flag)
{
    if(!file)return -1;
    kdevice_t* dev=(kdevice_t*)(*((uint32_t*)file));
    vfs_inode_t*n=inode2ptr(file);
    return dev->ops->mmap(dev,starts,length,offset,flag);
}
inode_ops_t dev_ops={
    .fs_read=dev_read,
    .fs_write=dev_write,
    .fs_mmap=dev_mmap
};
int device_init()
{
    if(kdev_slab)return -1;
    kdev_slab=alloc_slab_unit(sizeof(kdevice_t),"kdev_slab");
    if(!kdev_slab)return -1;
    fastmapper_init(&dev_list,KDEV_MAX);
    
}
kdevice_t *device_create(char *name,char root_type,char type,int dev_id,kobject_t*hardware,uint32_t usize,
    kdevice_ops_t*ops,
    char *data,uint32_t extra_data_sz)
{
    kdevice_t* re=alloc_in_slab_unit(kdev_slab);
    if(!re)return NULL;
    re->data=data;
    re->extra_data_sz=extra_data_sz;
    re->dev_id=dev_id;
    re->hardware=hardware;
    memcpy(re->name,name,20);
    re->root_type=root_type;
    re->type=type;
    re->unit_size=usize;
    re->ops=ops;
    return re;
}
void device_enum2()
{
    int i=0;
    //printf("enuming...");
    while (1)
    {
        kdevice_t*dev=fastmapper_get(&dev_list,i);
        // printf("get %s",dev->name);
        if(!dev)return;
        vfs_dir_elem_t*elem= vfs_mkvdir("/dev/", dev->name,dev);
        // vfs_inode_t*inode= inode2ptr(elem->file);
        // inode->file_type=VFS_INODE_TYPE_FILE;
        i++;
    }
    //printf("222333");
}
int device_add(kdevice_t*dev)
{
    if(!dev)return -1;
    //printf("[DEVICE :ADD NEW DEVICE:%s]\n",dev->name);
    return fastmapper_add_auto(&dev_list,dev);
}
bool dev_cmp(kdevice_t *value,char*expect)
{
    //printf("[cmping %s %s]\n",value->name,expect);
    return (!strcmp(value->name,expect));
}
kdevice_t* device_find(char *name)
{
    return fastmapper_find(&dev_list,name,dev_cmp);
}
