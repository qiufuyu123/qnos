#include"kobjects/obj_vfs.h"
#include"fs/fs_fat32.h"
#include"console.h"
#include"string.h"
#include"mem/malloc.h"
#include"mem/memorylayout.h"
slab_unit_t *fat_data_slab=0;
slab_unit_t *fat_cache_slab=0;
slab_unit_t *fat_file_cache_slab=0;
#define fat_cluster2sect(info,x) ((x-2)*((fat_data_t*)info)->bpb.sectors_per_group+((fat_data_t*)info)->fs_info.data_sect_start)
#define fat_chkdirvalid(dir) (*((char*)dir)!=0&&*((char*)dir)!=0xe5)
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
    sb->dev->ops->read(sb->dev,addr,1,&fdata->cache_list[0],0);

}
uint32_t fat_get_cluster_value(vfs_super_block_t *sb,uint32_t cluster)
{
    fat_data_t*fdata=sb->self_data;
    // 512/4=0x80
    // align : 0xffffff80
    uint32_t prep_cluster=cluster&(0xffffff80);  //To find the lastest one
    for (int i = 0; i < FAT_CACHE_MAX; i++)
    {
        if(fdata->cache_list[i].start_cluster==prep_cluster)
        {
            return fdata->cache_list[i].data[cluster-prep_cluster];
        }
    }
    //Not found?
    //Well, load new fat table to cache

    //TODO: Well, we can optimize this logic here
    //      Perhaps we can set up a refer-counter to 
    //      find out the least frequent cache elem 
    //      and clear it only
    fat_load_fat(sb,prep_cluster);
    return fdata->cache_list[0].data[cluster-prep_cluster];

}
uint32_t fat_get_elem_size(vfs_super_block_t*sb,uint32_t root_cluster)
{
    uint32_t cnt=1;
    while (true)
    {
        root_cluster=fat_get_cluster_value(sb,root_cluster);
        if(root_cluster>=0x0ffffff7)return cnt;
        cnt++;
    }
    return cnt;
    
}
int fat_cache_load_cluster(vfs_super_block_t*sb, fat_file_cache_t *fcache)
{
    int i=0;
    uint32_t nxt=fcache->fst_fat;
    fat_data_t*data=(fat_data_t*)sb->self_data; 
    while (true)
    {
        if(fcache->size_byte-i*data->bpb.sectors_per_group*512<data->bpb.sectors_per_group*512)
        {
            return VFS_FORMAT_ERR;
        }
        sb->dev->ops->read(sb->dev,fat_cluster2sect(sb->self_data,nxt),data->bpb.sectors_per_group,fcache->buffer+i*data->bpb.sectors_per_group*512,0);
        i++;
        nxt= fat_get_cluster_value(sb,nxt);
        if(nxt>=0x0FFFFFF7)
        {
            break;
        }
    }
    return i;
}
int fat_create_file_cache(vfs_super_block_t*sb, fat_file_cache_t*fcache)
{
    if(fcache->size_byte>fat_max_cache)return -1;
    fcache->pg_nd=0;
    if(fcache->size_byte<=2048)fcache->buffer=kmalloc(fcache->size_byte);
    else
    {
        fcache->pg_nd=ngx_align(fcache->size_byte,4096)/4096;
        fcache->buffer=kmalloc_page(fcache->pg_nd);
    }
    if(!fcache->buffer)return -1;
    return fat_cache_load_cluster(sb,fcache);
}


inode_ops_t fat_inode_ops={

};

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
        if(!fat_chkdirvalid(dirs))return 1;
        if(dirs->low_16_entry_num==0)
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
            new_fcache->size_byte=dirs->size_in_byte;
        }else
        {
            new_fcache->size_byte=fat_get_elem_size(inode->sb,fst_cluster)*fdata->bpb.sectors_per_group*512;
        }
        n_dir_elem->name=name;
        new_fcache->fst_fat=fst_cluster;
        new_inode->inode_ptr=new_fcache;
        new_inode->sb=inode->sb;
        new_inode->magic_num=inode->magic_num;
        new_inode->file_type=dirs->attr&0x10?VFS_INODE_TYPE_DIR:VFS_INODE_TYPE_FILE;
        new_inode->size_in_byte=dirs->size_in_byte;
        n_dir_elem->file=&new_inode->inode_ptr;
        list_append(&inode->sb->inode_list,&new_inode->inode_elem);
        list_append(&dir->file_elems,&n_dir_elem->list_tag);
        //printf("[FAT]:add %s !\n",n_dir_elem->name);
        dirs++;
    }
    return 1;
}
vfs_dentry_ops_t fat_dentry_ops={
    .load_inode_dirs=fat_load_dentry
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
    printf("Mouting FAT32 fs\n");
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
    uint32_t len=fat_get_elem_size(sb,bpb.fat_of_root);
    printf("[FAT 32]Root Dir Occupied:%d Bytes!\n",len*bpb.sectors_per_group*512);
    root_inode->size_in_byte=len*bpb.sectors_per_group*512;
    root_inode->file_type=VFS_INODE_TYPE_DIR;
    fcache->size_byte=root_inode->size_in_byte;
    fcache->buffer=0;
    fcache->fst_fat=bpb.fat_of_root;
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