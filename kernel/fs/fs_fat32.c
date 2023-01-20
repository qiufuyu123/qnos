#include"kobjects/obj_vfs.h"
#include"fs/fs_fat32.h"
#include"console.h"
#include"string.h"
#include"mem/malloc.h"
#include"mem/memorylayout.h"
slab_unit_t *fat_data_slab=0;
slab_unit_t *fat_cache_slab=0;
slab_unit_t *fat_file_cache_slab=0;

#define LOWORD(l) ((uint16_t)(l)) 
#define HIWORD(l) ((uint16_t)((uint32_t)(l) >> 16))
#define PALIGN_DOWN(x,align) (x & ~(align-1))
#define fat_cluster2sect(info,x) ((x-2)*((fat_data_t*)info)->bpb.sectors_per_group+((fat_data_t*)info)->fs_info.data_sect_start)
#define fat_chkdirvalid(dir) (*((char*)dir)!=0xe5)
#define fat_max_cache 32*1024*1024
/**
 * To simplificate the process of reading 
 * We assume that the MAXIMUM file cache buffer is 32MB
 * TODO: Dynamic cache loading
*/
int fat_detect_disk(fat_data_t *fdata,vfs_super_block_t*sb)
{
    sb->dev->ops->read(sb->dev,0,1,&fdata->bpb,0);
    if(fdata->bpb.sign!=0x28&&fdata->bpb.sign!=0x29)
    {
        printf("FAT32: Bad BPB sign:0x%x\n",fdata->bpb.sign);
        return -1;
    }
    uint32_t addr=fdata->bpb.fs_info_sect;
    sb->dev->ops->read(sb->dev,addr,1,&fdata->fs_info,0);
    if(fdata->fs_info.client_sign!=0x41615252)
    {
        printf("FAT32: Bad FS_INFO sign 0x%x!\n",fdata->fs_info.client_sign);
        return -1;
    }
    return 1;
}
void fat_load_fat(vfs_super_block_t*sb,uint32_t cluster)
{
    fat_data_t*fdata=sb->self_data;
    uint32_t addr=fdata->bpb.num_of_reserved_sects+cluster/(512/4);
    
    // Please view comments below function `get_cluster_value`
    sb->dev->ops->read(sb->dev,addr,fdata->bpb.sectors_per_group,fdata->cache_list->data,0);
    fdata->cache_list->start_cluster=cluster;
}
void fat_flush_fat(vfs_super_block_t*sb,uint32_t idx)
{
    fat_data_t*fdata=sb->self_data;
    uint32_t addr=fdata->bpb.num_of_reserved_sects+fdata->cache_list->start_cluster/(512/4);
    
    // Please view comments below function `get_cluster_value`
    sb->dev->ops->write(sb->dev,addr,fdata->bpb.sectors_per_group,&fdata->cache_list->data[0],0);
}
uint32_t fat_get_free_fat(vfs_super_block_t*sb)
{
    fat_data_t*fdata=sb->self_data;
    uint32_t tmp=fdata->cache_list->start_cluster;
    uint32_t unit=fdata->bpb.sectors_per_group*512/4;
    while (1)
    {
        for (int j = 0; j < unit; j++)
        {
            if(fdata->cache_list->data[j]==0)
            {
                uint32_t now=fdata->cache_list->start_cluster+j;
                if(now>=3)
                    return now;
            }
        }    
        tmp+=unit;
        if(tmp>fdata->fs_info.cnt_in_last_useful_fat)return 0;
        fat_load_fat(sb,tmp);
    }
}
void fat_set_cluster_value(vfs_super_block_t *sb,uint32_t cluster,uint32_t val)
{
    fat_data_t*fdata=sb->self_data;
    // 512/4=0x80
    // align : 0xffffff80
    int i=0;
    uint32_t prep_cluster=PALIGN_DOWN(cluster,512/4);  //To find the lastest one
    if(fdata->cache_list->start_cluster==prep_cluster)
    {
        fdata->cache_list->data[cluster-prep_cluster]=val;
        goto final;
    }
    //Not found?
    //Well, load new fat table to cache

    //TODO: Well, we can optimize this logic here
    //      Perhaps we can set up a refer-counter to 
    //      find out the least frequent cache elem 
    //      and clear it only
    fat_load_fat(sb,prep_cluster);
    fdata->cache_list->data[cluster-prep_cluster]=val;
final:
    fat_flush_fat(sb,i);
}
uint32_t fat_get_cluster_value(vfs_super_block_t *sb,uint32_t cluster)
{
    fat_data_t*fdata=sb->self_data;
    // 512/4=0x80
    // align : 0xffffff80
    uint32_t prep_cluster=PALIGN_DOWN(cluster,512/4);  //To find the lastest one
    if(fdata->cache_list->start_cluster==prep_cluster)
    {
        return fdata->cache_list->data[cluster-prep_cluster];
    }
    //Not found?
    //Well, load new fat table to cache

    //TODO: Well, we can optimize this logic here
    //      Perhaps we can set up a refer-counter to 
    //      find out the least frequent cache elem 
    //      and clear it only
    fat_load_fat(sb,prep_cluster);
    return fdata->cache_list->data[cluster-prep_cluster];

}
uint32_t fat_get_elem_size(vfs_super_block_t*sb,uint32_t root_cluster,uint32_t *lst_cluster)
{
    uint32_t cnt=1;
    while (true)
    {
        *lst_cluster=root_cluster;
        root_cluster=fat_get_cluster_value(sb,root_cluster);
        //printf("lst_cluster %d\n",*lst_cluster);
        if(root_cluster>=0x0ffffff7)
        {
            return cnt;
        }
        cnt++;
    }
    return cnt;
    
}
int fat_cache_load_cluster(vfs_super_block_t*sb, fat_file_cache_t *fcache)
{
    int i=0;
    uint32_t nxt=fcache->fst_fat;
    fat_data_t*data=(fat_data_t*)sb->self_data; 
    //printf("FAT CACHE:%d %d",fcache->size_byte,fcache->fst_fat);
    while (true)
    {
        if(fcache->size_byte-i*data->bpb.sectors_per_group*512<data->bpb.sectors_per_group*512)
        {
            return VFS_FORMAT_ERR;
        }
        sb->dev->ops->read(sb->dev,fat_cluster2sect(sb->self_data,nxt),data->bpb.sectors_per_group,fcache->buffer+i*data->bpb.sectors_per_group*512,0);
        i++;
        //printf("LOAD CLUSTER %d;",nxt);
        nxt= fat_get_cluster_value(sb,nxt);
        //printf("LOAD CLUSTER 2[%d];",nxt);
        if(nxt>=0x0FFFFFF7)
        {
            break;
        }
    }
    return i;
}
int fat_create_file_cache(vfs_super_block_t*sb, fat_file_cache_t*fcache)
{
    //printf("[crt]] %x %d %d",fcache,fcache->buffer,fcache->size_byte);
    if(fcache->size_byte>fat_max_cache)return -1;
    fcache->pg_nd=0;
    if(fcache->size_byte<=2048)
    {
        //printf("[");
        fcache->buffer=kmalloc(fcache->size_byte);
        //printf("a]");
    }
    else
    {
        fcache->pg_nd=ngx_align(fcache->size_byte,4096)/4096;
        fcache->buffer=kmalloc_page(fcache->pg_nd);
    }
    if(!fcache->buffer)return -1;
    //printf("[crt]]");
    return fat_cache_load_cluster(sb,fcache);
}
void fat_free_fcache(fat_file_cache_t*fcache)
{
    //printf("[FATFREE:%d %d]\n",fcache->pg_nd,fcache->size_byte);
    if(fcache->pg_nd)
    {
        kfree_page(fcache->buffer,fcache->pg_nd);
    }else
    {
        kfree(fcache->buffer);
    }
}
void fat_flush_fcache(vfs_super_block_t*sb, fat_file_cache_t*fcache)
{
    int i=0;
    uint32_t nxt=fcache->fst_fat;
    fat_data_t*data=(fat_data_t*)sb->self_data; 
    //printf("FLUSH FCACHE,old:\n");
    //printhex(fcache->buffer,20);
    //printf("FAT CACHE:%d %d",fcache->size_byte,fcache->fst_fat);
    while (true)
    {
        if(fcache->size_byte-i*data->bpb.sectors_per_group*512<data->bpb.sectors_per_group*512)
        {
            return VFS_FORMAT_ERR;
        }
        sb->dev->ops->write(sb->dev,fat_cluster2sect(sb->self_data,nxt),data->bpb.sectors_per_group,fcache->buffer+i*data->bpb.sectors_per_group*512,0);
        i++;
        //printf("FLUSH 1:%d;",nxt);
        nxt= fat_get_cluster_value(sb,nxt);
        //printf("FLUSH 2:%d;",nxt);
        if(nxt>=0x0FFFFFF7)
        {
            break;
        }
    }
    return i;
}
int fat_inode_close(inode_handle file)
{
    vfs_inode_t*inode=inode2ptr(file);
    vfs_super_block_t*sb=inode->sb;
    if(inode->magic_num!=INODE_MAGIC_NUM)return VFS_FORMAT_ERR;
    // 1. Free fcache buffer
    fat_file_cache_t*fcache=inode->inode_ptr;
    fat_free_fcache(fcache);
    kfree(fcache);
    printf("FAT32 Inode close!\n");
    return 1;
}
uint32_t fat_add_cluster_chain(vfs_super_block_t*sb,fat_file_cache_t*fcache,uint32_t cnt)
{
    uint32_t root_cluster=fcache->lst_fat;
    //printf("ADD CHAIN:[%d %d]",cnt,root_cluster);
    fat_data_t*fdata=sb->self_data;
    uint32_t old_cnt=cnt;
    while (cnt--)
    {
        uint32_t v=fat_get_free_fat(sb);
        if(!v)
            return 0;
        sb->dev->ops->zeros(sb->dev,fat_cluster2sect(sb->self_data,v),fdata->bpb.sectors_per_group,0);
        fat_set_cluster_value(sb,root_cluster,v);
        root_cluster=v;
        //printf("ADD CHAIN [%d]",v);
    }
    //printf("root now:[%d]",root_cluster);
    fat_set_cluster_value(sb,root_cluster,0x0ffffff8);
    fcache->lst_fat=root_cluster;

    // DO NOTE Modify the size_byte here
    // because in 'enlarge' function, we use the original size
    // to free the original buffer
    // fcache->size_byte+=old_cnt*fdata->bpb.sectors_per_group*512;
    // printf("New size:%d",fcache->size_byte);
    return fcache->size_byte+ old_cnt*fdata->bpb.sectors_per_group*512;
}
int fat_fcache_enlarge(vfs_super_block_t*sb, fat_file_cache_t*fcache,uint32_t size)
{
    fat_data_t*fdata=sb->self_data;
    uint32_t fat_sz=fdata->bpb.sectors_per_group*512;
    if(size<=fcache->size_byte)return 1;
    uint32_t rem=ngx_align(size-fcache->size_byte,fat_sz);
    uint32_t new_sz=0;
    fat_free_fcache(fcache);
    new_sz=fat_add_cluster_chain(sb,fcache,rem/fat_sz);
    if(!new_sz)
        return -1;
    // 2. enlarge buffer
    fcache->buffer=0;
    fcache->size_byte=new_sz;
    if(fat_create_file_cache(sb,fcache)<0)
        return -1;
    printf("ENLARGE %d\n",fcache->size_byte);
    return 1;
}
int fat_inode_write(inode_handle file,uint32_t size,void* buffer, uint32_t flag)
{
    vfs_inode_t*inode=inode2ptr(file);
    //printf("INODE:0x%x",inode);
    vfs_super_block_t*sb=inode->sb;
    if(inode->magic_num!=INODE_MAGIC_NUM)return VFS_FORMAT_ERR;
    fat_file_cache_t*fcache=inode->inode_ptr;
    // hanle the creating case:
    if(!fcache)
    {
        // inode->inode_ptr=alloc_in_slab_unit(fat_file_cache_slab);
        // fcache=inode->inode_ptr;
        // if(!fcache)
        //     return VFS_ALLOC_ERR;
        // fcache->fst_fat=fat_get_free_fat(sb);
        // if(!fcache->fst_fat)
        //     return VFS_ALLOC_ERR;
        // fat_set_cluster_value(sb,fcache->fst_fat,0x0ffffff8);
        // fcache->lst_fat=fcache->fst_fat;
        // fcache->buffer=0;
        return VFS_NULL_OBJECT_ERR;
    }
    if(!fcache->buffer)
    {
        //printf("INODE WRITE:load_fcache 0x%x 0x%x %d %d",sb,fcache,fcache->size_byte,fcache->fst_fat);
        if(fat_create_file_cache(sb,fcache)<0)
            return VFS_ALLOC_ERR; 
        //printf("FCACHE OK");
    }
    // enlarge the buffer
    if(fcache->size_byte<inode->seek_offset+size)
    {
        if(fat_fcache_enlarge(sb,inode->inode_ptr,size+inode->seek_offset)<0)return VFS_ALLOC_ERR;
    }
    
    // reset the variable since we edited the pointer
    fcache=inode->inode_ptr;
    memcpy(fcache->buffer+inode->seek_offset,buffer,size);
    inode->seek_offset+=size;
    if(inode->seek_offset>inode->size_in_byte)
    {
        // flush parent dir information
        inode->size_in_byte=inode->seek_offset;
        vfs_inode_t*parent=inode2ptr(inode->parent_inode);
        if(parent->magic_num!=INODE_MAGIC_NUM)
        {
            return VFS_FORMAT_ERR;
        }
        fat_file_cache_t*parent_cache=parent->inode_ptr;
        fat_dir_desc_t*desc=(fat_dir_desc_t*)((uint32_t)parent_cache->buffer+fcache->dir_offset);
        desc->size_in_byte=inode->size_in_byte;
        //printf("INODE WRITE: OFFSET:%d\n",fcache->dir_offset);
        fat_flush_fcache(inode->sb,parent_cache);
    }
    fat_flush_fcache(sb,fcache);
    return size;
    
}
int fat_inode_read(inode_handle file,uint32_t size,void* buffer, uint32_t flag)
{
    vfs_inode_t*inode=inode2ptr(file);
    vfs_super_block_t*sb=inode->sb;
    if(inode->magic_num!=INODE_MAGIC_NUM)return VFS_FORMAT_ERR;
    fat_file_cache_t*fcache=inode->inode_ptr;
    if(!fcache->buffer)
    {
        if(fat_create_file_cache(sb,fcache)<0)return VFS_ALLOC_ERR; 
    }
    //if(fat_cache_load_cluster(sb,fcache)<0)return VFS_HARDLOAD_ERR;
    if(inode->size_in_byte-inode->seek_offset<size)size=inode->size_in_byte-inode->seek_offset;
    if(size==0)return 0;
    memcpy(buffer,fcache->buffer+inode->seek_offset,size);
    inode->seek_offset+=size;
    return size;
    
}
int fat_inode_seek(inode_handle file,uint32_t source,int offset)
{
    vfs_inode_t*inode=inode2ptr(file);
    fat_file_cache_t*fcache=inode->inode_ptr;
    if(inode->magic_num!=INODE_MAGIC_NUM)return -1;
    uint32_t ori=inode->seek_offset;
    if(source==SEEK_SET)
    {
        inode->seek_offset=offset;
    }else if(source==SEEK_CUR)
    {
        inode->seek_offset+=offset;
    }else
    {
        inode->seek_offset=inode->size_in_byte+offset;
    }
    if(inode->seek_offset+1>fcache->size_byte)
    {
        if(fat_fcache_enlarge(inode->sb,inode->inode_ptr,inode->seek_offset+1)<0)
        {
            inode->seek_offset=ori;
            return -1;
        }
        inode->size_in_byte=inode->seek_offset;
    }
    return 1;
}
inode_ops_t fat_inode_ops={
    .fs_read=fat_inode_read,
    .fs_lseek=fat_inode_seek,
    .fs_close=fat_inode_close,
    .fs_write=fat_inode_write
};
fat_dir_desc_t* fat_find_free_desc(vfs_inode_t*dir,uint32_t *offset)
{
    fat_file_cache_t*fcache=dir->inode_ptr;
    fat_dir_desc_t*desc=fcache->buffer;
    while (1)
    {
        if(desc->fname[0]=='\0')
            break;
        else if(desc->fname[0]=='\xe5')
            break;
        desc++;
    }
    *offset=(uint32_t)desc-(uint32_t)fcache->buffer;
    return desc;
    
}
vfs_dir_elem_t* fat_add_elem(struct vfs_dentry* dir,char *fname)
{
    vfs_inode_t* inode=inode2ptr(dir->dir_file);
    vfs_super_block_t* sb=inode->sb;
    // 1. Write raw data to the cache buf and flush it
    //    to the hard-disk

    // 1.1 Try to enlarge fcache
    //
    fat_file_cache_t*fcache=inode->inode_ptr;
    uint32_t offset=0;
    fat_dir_desc_t* desc=fat_find_free_desc(inode,&offset);
    if(!desc)
        return 0;
    if(offset+sizeof(fat_dir_desc_t)>fcache->size_byte)
    {
        printf("ENLARGING...[%d]",fcache->lst_fat);
        if(fat_fcache_enlarge(sb,fcache,offset+sizeof(fat_dir_desc_t))<0)
            return 0;
        
        //Re-search since the address of buffer has been changed
        //
        desc=fat_find_free_desc(inode,&offset);
        if(!desc)
            return 0;
    }
    memset(desc,0,sizeof(fat_dir_desc_t));
    uint32_t fname_len=0;
    uint32_t extname_len=0;
    char namelen=strlen(fname);
    vfs_dir_elem_t *d_elem=vfs_alloc_delem();
    vfs_inode_t*ninode=vfs_alloc_inode();
    fat_file_cache_t*nfcache=alloc_in_slab_unit(fat_file_cache_slab);
    uint32_t entrynum=fat_get_free_fat(sb);
    if(entrynum==0||!d_elem||!ninode||!nfcache)
        goto clean;
    d_elem->name=strdup(fname);
    if(!d_elem->name)
        goto clean;
    //printf("entry is %d\n",entrynum);
    fat_set_cluster_value(sb,entrynum,0x0ffffff8);
    for (int i = 0; i < namelen; i++)
    {
        if(fname[i]=='.')
        {
            fname_len=i;
            extname_len=namelen-fname_len-1;
            break;
        }
    }
    if(extname_len>3)
        extname_len=3;
    if(fname_len>8)
        fname_len=8;
    
    // Clean disk buf
    //
    //printf("Sects:%d %d %x",entrynum,fat_cluster2sect(sb->self_data,entrynum),sb->dev->ops->zeros);
    sb->dev->ops->zeros(sb->dev,fat_cluster2sect(sb->self_data,entrynum),1,0);
    //printf("Free Sects");
    // 2. Add elem to the dentry list
    //
    memset(desc->fname,' ',8);
    memset(desc->fextname,'\0',3);
    memcpy(desc->fname,fname,fname_len);
    memcpy(desc->fextname,fname+fname_len+1,extname_len);
    desc->high_16_entry_num=HIWORD(entrynum);
    desc->low_16_entry_num=LOWORD(entrynum);
    fat_flush_fcache(sb,inode->inode_ptr);

    // 3. Add inode and d_elem 
    //
    d_elem->d_dir=dir;
    d_elem->file=&ninode->inode_ptr;
    ninode->file_type=VFS_INODE_TYPE_FILE;
    ninode->i_ops=&fat_inode_ops;
    ninode->inode_ptr=nfcache;
    ninode->size_in_byte=ninode->seek_offset=0;
    nfcache->fst_fat=entrynum;
    nfcache->lst_fat=entrynum;
    nfcache->dir_offset=offset;
    nfcache->size_byte=512;
    ninode->magic_num=INODE_MAGIC_NUM;
    ninode->parent_inode=dir->dir_file;
    ninode->sb=inode->sb;
    list_append(&sb->inode_list,&ninode->inode_elem);
    list_append(&dir->file_elems,&d_elem->list_tag);
    //printf("ADD offset:%d\n",nfcache->dir_offset);
    return d_elem;
clean:
    kfree(d_elem);
    kfree(nfcache);
    kfree(ninode);
    return 0;
}

int fat_load_dentry(struct vfs_dentry*dir, inode_handle file)
{
    //printf("Loading ...");
    vfs_inode_t*inode= inode2ptr(file);
    fat_file_cache_t*fcache=inode->inode_ptr;
    fat_data_t*fdata=inode->sb->self_data;
    //printf("FAT LOADING: %d sz:%d",fcache->fst_fat,fcache->size_byte);
    if(fcache->size_byte==0)return 1;
    if(!fcache)return VFS_FORMAT_ERR;
    if(!fcache->buffer)
    {
        if(fat_create_file_cache(inode->sb,fcache)<0)return VFS_ALLOC_ERR;
    }
    fat_dir_desc_t *dirs=fcache->buffer;
    while (true)
    {
        //printf("%x ;",*(char*)dirs);
        if(dirs->fname[0]=='\xe5')
        {
            dirs++;
            continue;
        }
        if(*(char*)dirs==0x00)break;
        if(dirs->attr==0x0f)
        {
            dirs++;
            continue;
        }
        char *name=kmalloc(12);
        vfs_dir_elem_t*n_dir_elem=vfs_alloc_delem();
        vfs_inode_t*new_inode=vfs_alloc_inode();
        fat_file_cache_t*new_fcache=alloc_in_slab_unit(fat_file_cache_slab);
        if(!n_dir_elem||!new_inode||!name||!new_fcache)
        {
            kfree(n_dir_elem);
            kfree(new_inode);
            kfree(name);
            kfree(new_fcache);
            return VFS_ALLOC_ERR;
        }
        int i=0;
        for (; i < 8; i++)
        {
            if(dirs->fname[i]==' ')break;
            name[i]=dirs->fname[i];
        }
        name[i]='\0';
        uint32_t fst_cluster=((uint32_t)dirs->high_16_entry_num<<16)|(uint32_t)dirs->low_16_entry_num;
        if(!(dirs->attr&0x10))
        {
            name[i]='.';
            memcpy(name+i+1,dirs->fextname,3);
            name[i+1+3]='\0';
        }
        new_fcache->size_byte=fat_get_elem_size(inode->sb,fst_cluster,&new_fcache->lst_fat)*fdata->bpb.sectors_per_group*512;

        n_dir_elem->name=name;
        new_fcache->fst_fat=fst_cluster;
        new_fcache->dir_offset=(uint32_t)dirs-(uint32_t)fcache->buffer;
        new_inode->inode_ptr=new_fcache;
        new_inode->sb=inode->sb;
        new_inode->magic_num=inode->magic_num;
        new_inode->file_type=dirs->attr&0x10?VFS_INODE_TYPE_DIR:VFS_INODE_TYPE_FILE;
        new_inode->size_in_byte=dirs->size_in_byte;
        new_inode->i_ops=&fat_inode_ops;
        new_inode->parent_inode=dir->dir_file;
        new_inode->seek_offset=0;
        n_dir_elem->file=&new_inode->inode_ptr;
        list_append(&inode->sb->inode_list,&new_inode->inode_elem);
        list_append(&dir->file_elems,&n_dir_elem->list_tag);
        //printf("[FAT]:add %s %x %d!\n",n_dir_elem->name,(uint32_t)dirs->fname[0],dirs->fname[0]=='\xe5');
        dirs++;
    }
    inode->size_in_byte=(uint32_t)dirs-(uint32_t)fcache->buffer;
    return 1;
}
vfs_dentry_ops_t fat_dentry_ops={
    .load_inode_dirs=fat_load_dentry,
    .add_delem=fat_add_elem
};
int fat_mount(vfs_super_block_t*sb,kdevice_t*dev)
{
    if(!fat_data_slab)
    {
        fat_data_slab=alloc_slab_unit(sizeof(fat_data_t),"fat32_data_slab");
        if(!fat_data_slab)return VFS_ALLOC_ERR;
    }
    if(!fat_cache_slab)
    {
        fat_cache_slab=alloc_slab_unit(sizeof(fat_fattable_cache_t),"fat32_fat_cache_slab");
        if(!fat_cache_slab)return VFS_ALLOC_ERR;
    }
    if(!fat_file_cache_slab)
    {
        fat_file_cache_slab=alloc_slab_unit(sizeof(fat_file_cache_t),"fat32_file_cache_slab");
        if(!fat_file_cache_slab)return VFS_ALLOC_ERR;
    }
    sb->dev=dev;
    //printf("Mouting FAT32 fs\n");
    memcpy(sb->fs_name,"FAT32",6);
    sb->self_data_size=sizeof(fat_data_t);
    sb->self_data=alloc_in_slab_unit(fat_data_slab);
    if(!sb->self_data)return VFS_ALLOC_ERR;
    fat_data_t*fdata=sb->self_data;
    if(fat_detect_disk(sb->self_data,sb)<0)
    {
        goto clean;
    }
    fat32_bpb_t bpb=((fat_data_t*)sb->self_data)->bpb;
    //fat_fs_info_t info=((fat_data_t*)sb->self_data)->fs_info;
    ((fat_data_t*)sb->self_data)->fs_info.data_sect_start=bpb.num_of_reserved_sects+(bpb.num_of_fat*bpb.sects_per_fat);
    
    // bpb.num_of_reserved_sects;
    // bpb.num_of_fat
    printf("MARK:%s \n[ROOT SECT:%d %d %d %d]OK\n", bpb.sys_mark_str,bpb.num_of_reserved_sects,((fat_data_t*)sb->self_data)->fs_info.data_sect_start,bpb.fat_of_root, fat_cluster2sect(sb->self_data,bpb.fat_of_root));
    uint32_t root_dir_sect=fat_cluster2sect(sb->self_data,bpb.fat_of_root);
    vfs_inode_t*root_inode=vfs_alloc_inode();
    vfs_dentry_t*root_dentry=vfs_alloc_dentry();
    fat_file_cache_t*fcache=alloc_in_slab_unit(fat_file_cache_slab);
    if(!root_dentry||!root_inode||!fcache)goto clean2;
    root_inode->i_ops=&fat_inode_ops;
    root_dentry->ops=&fat_dentry_ops;
    
    root_inode->inode_ptr=0;

    root_inode->magic_num=INODE_MAGIC_NUM;
    root_inode->sb=sb;
    root_inode->refer_count=root_inode->seek_offset=root_inode->sync_mark=0;
    fdata->cache_list=alloc_in_slab_unit(fat_cache_slab);
    if(!fdata->cache_list)
    {
        goto clean2;
    }
    fat_load_fat(sb,0);
    //fdata->max_cluster_num=sect_num*512/8
    uint32_t len=fat_get_elem_size(sb,bpb.fat_of_root,&fcache->lst_fat);
    printf("[FAT 32]Root Dir %d  %dOccupied:%d Bytes!\n",bpb.fat_of_root,fat_cluster2sect(fdata,bpb.fat_of_root), len*bpb.sectors_per_group*512);
    root_inode->size_in_byte=len*bpb.sectors_per_group*512;
    root_inode->file_type=VFS_INODE_TYPE_DIR;
    fcache->size_byte=root_inode->size_in_byte;
    fcache->buffer=0;
    fcache->fst_fat=bpb.fat_of_root;
    //fcache->lst_fat=fcache->fst_fat;
    root_inode->inode_ptr=fcache;
    list_append(&sb->inode_list,&root_inode->inode_elem);
    list_append(&sb->dentry_list,&root_dentry->dentry_elem);
    list_init(&root_dentry->file_elems);
    root_dentry->dir_file=&root_inode->inode_ptr;
    root_dentry->parent_dir=root_dentry;
    //
    root_dentry->ops->load_inode_dirs(root_dentry,&root_inode->inode_ptr);
    //
    sb->root_dir=root_dentry;
    sb->root_inode=&root_inode->inode_ptr;
    
    //bpb.num_of_root_path
    return 1;
clean2:
    if(fcache)kfree(fcache);
    if(root_inode)kfree(root_inode);
    if(root_dentry)kfree(root_dentry);
clean:
    kfree(sb->self_data);
    sb->self_data_size=0;
    printf("Mount FAT32 fs FAILED!\n");
    return VFS_FORMAT_ERR;
}
vfs_sb_ops_t fat_ops={
    .fs_mount=fat_mount
};
vfs_sb_ops_t *fat_getops()
{
    return &fat_ops;
}