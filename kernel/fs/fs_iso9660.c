#include"fs/fs_iso9660.h"
#include"kobjects/obj_vfs.h"
#include"kobjects/kobject.h"
#include"mem/malloc.h"
#include"string.h"
#include"mem/memorylayout.h"
#include"console.h"
slab_unit_t *iso_inode_info_slab;
#define SEARCH_SECTOR_NUM 20
#define alloc_iso_inode_info alloc_in_slab_unit(iso_inode_info_slab)
inode_handle iso_get_inode(uint32_t lba);
iso_volumn_group_t* detect_for_volumns(vfs_super_block_t *sb)
{
    // I. Polling (idk if this word is right here...) For the volums 
    //ata_dev->open(sb->disk_id,0);
    
    iso_volumn_group_t *ret=kmalloc(sizeof(iso_volumn_group_t));
    if(!ret)return NULL;
    for (int i = 0; i <3 ; i++)
    {
        iso_volumn_symbol_t *buf=kmalloc(2048);
        sb->dev->read(sb->dev,0x10+i,1,buf,0);
        //char *test=buf;
        //printf("\n===>\n");
        //for (int i = 0; i < 100; i++)
        //{
        //    if(test[i]>32)printf("%c",test[i]);
        //}
        /**
         * @brief NOTE
         * if(!strcmp(buf->strA,"CD001"))
         * Q:Why not use this?  ^^^^^^^^
         * A:Guess why ? :) 
         */
        //
        if(buf->strA[0]=='C'&&buf->strA[1]=='D'&&buf->strA[2]=='0'&&buf->strA[3]=='0'&&buf->strA[4]=='1')
        {
            printf("Find a volumn symbol:");
            if(buf->type_code==VOL_BOOT_SYM)
            {
                ret->boot_vol=buf;
                printf("BOOT SYM\n");
            }else if(buf->type_code==VOL_MAIN_SYM)
            {
                ret->main_vol=buf;
                ret->main_lba=0x10+i;
                printf("MAIN SYM\n");
            }
            else if(buf->type_code==VOL_END_SYM)
            {
                ret->end_vol=buf;
                printf("END SYM\n");
            }else
            {
                kfree(buf);
                printf("Unknown\n");
            }
        }else
        {

        kfree(buf);
        }
    }
    /**
     * @brief NOTE
     * To be honest
     * We dont really care about boot volumn and end volumn
     * Anyway, I read them
     * Perhaps....
     * Someday I may use it(May be in a strange situation? idk...)
     *          -QIUFUYU (:D)
     *          
     * 
     */
    return ret;
}
void free_volumn_group(iso_volumn_group_t *g)
{  
    if(g)
    {
        if(g->boot_vol)kfree(g->boot_vol);
        if(g->main_vol)kfree(g->main_vol);
        if(g->end_vol)kfree(g->end_vol);
        kfree(g);
    }
}
//
void replace_str_with_zero(char *strAD,uint8_t len)
{
    for (int i = len-1; i >=0; i--)
    {
        if(strAD[i]==0x20)strAD[i]=0x0;
        else break;
    }   
}
char *convert_str(char *strAD,uint8_t len)
{
    char *new_str=kmalloc(len+1);
    memset(new_str,0,len+1);
    memcpy(new_str,strAD,len);
    replace_str_with_zero(new_str,len);
    return new_str;
}

int iso_inode_read(inode_handle file,uint32_t size,uint8_t *buffer,uint32_t flag)
{
    if(!file)return -1;//null?
    //printf("I:reading;");
    iso_inode_info_t *phy_info=(iso_inode_info_t*)(*(uint32_t*)file);
    vfs_inode_t *inode=inode2ptr(file);
    if(inode->magic_num!=INODE_MAGIC_NUM)return VFS_INODE_ERR;
    if(size>inode->size_in_byte)size=inode->size_in_byte;//TOO LARGE ???
    // I. well, now we start reading ...
    uint32_t old_sz=size;
    bool repeat=0;
    if(size>=1023*4096)
    {
        size=1023*4096;
        old_sz-=size;
        repeat=1;
    }
    uint32_t offset_in_sect=inode->seek_offset/2048;
    uint32_t offset_in_byte=inode->seek_offset%2048;
    uint32_t pg_cnt=ngx_align(size+offset_in_byte+phy_info->base_offset,4096)/4096;
    char *buf_page=kmalloc_page(pg_cnt);
    //printf("II:Prepare page buf; %d %d %d %d s:%d",offset_in_sect,offset_in_byte,phy_info->base_offset,phy_info->phy_lba,size);
    inode->sb->dev->read(inode->sb->dev,phy_info->phy_lba+offset_in_sect,ngx_align(size+offset_in_byte+phy_info->base_offset,2048)/2048,buf_page,0);
    //while(1);
    //printf("III:read ok!");
    memcpy(buffer,(uint32_t)buf_page+offset_in_byte+phy_info->base_offset,size);
    //if(pg_cnt==2)printf("%s",buf_page);
    //printf("IV:free");
    kfree_page(buf_page,pg_cnt);
    // II.mark sync flag 
    //inode->sync_mark=flag;//actually we dont need in READ mode!
    inode->seek_offset+=size;
    if(repeat){return iso_inode_read(file,old_sz,(uint32_t)buffer+size,flag);}
    return size;
}
int iso_inode_lseek(inode_handle file,uint32_t source,int offset)
{
    iso_inode_info_t *phy_info=file;
    vfs_inode_t *inode=inode2ptr(file);
    if(source==SEEK_SET)inode->seek_offset=offset;
    else if(source==SEEK_CUR)inode->seek_offset+=offset;
    else inode->seek_offset=inode->size_in_byte+offset;
}
int __travel_dir_find(list_elem_t*elem,char * arg)
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
vfs_dir_elem_t *iso_dentry_find(vfs_dentry_t *dir,char *name)
{
    list_elem_t *elem= list_traversal(&dir->file_elems,__travel_dir_find,name);
    if(!elem)return 0;
    return elem2entry(vfs_dir_elem_t,list_tag,elem);
}
int iso_decode_fname(char *name)
{
    for (int i = strlen(name)-1; i >=0; i--)
    {
        if(name[i]==';')
        {
            name[i]='\0';
            return 1;
        }
        name[i]='\0';
    }
    return -1;
    
}
int iso_dentry_inode_load(vfs_dentry_t*dir, inode_handle file)
{
    vfs_inode_t *inode=inode2ptr(file);
    if(inode->file_type==VFS_INODE_TYPE_FILE)return VFS_FORMAT_ERR;
    dir->name_len=12;//8+3
    dir->dir_file=file;
    uint32_t dir_len=inode->size_in_byte;
    //printf("x %d",dir_len);
    //while(1);
    char *buff=kmalloc(dir_len);
    
    //printf("z");
    if(!buff)return VFS_ALLOC_ERR;
    //printf("y");
    

    inode->i_ops->fs_read(file,dir_len,buff,0);
    uint32_t write_cnt=0;
    int dir_cnt=0;
    while (1)
    {
        if(write_cnt>=dir_len)break;
        iso_dirent_t *cur_dir=(iso_dirent_t*)((uint32_t)buff+write_cnt);
        if(cur_dir->dir_record_len==0)break;
        //if()
        //if(dir->name_len)
        char *name_buf=kmalloc(dir->name_len+1);
        if(!name_buf)
        {
            kfree(buff);
            return VFS_ALLOC_ERR;
        }
        memset(name_buf,0,dir->name_len+1);
        //printf("A");
        //So, why I convert the pointer so standardly?
        //i dont know ~ ~ ~
        //(By the way , writing those pointer-convert makes me feel safer
        //in some way.)
        vfs_dir_elem_t *n_dir_elem=vfs_alloc_delem();
        if(!n_dir_elem)
        {
            kfree(buff);
            kfree(name_buf);
            return VFS_ALLOC_ERR;
        }
        //printf("B");
        
        write_cnt+=cur_dir->dir_record_len;
        memcpy(name_buf,&cur_dir->fname_start,cur_dir->fname_len);
        n_dir_elem->name=name_buf;
        vfs_inode_t *d_inode=inode2ptr(iso_get_inode(cur_dir->data_lba[0]));
        //printf("C");
        if(d_inode->magic_num!=INODE_MAGIC_NUM)
        {
            kfree(buff);
            kfree(name_buf);
            return VFS_FORMAT_ERR;
        }
        //printf("D");
        d_inode->sb=inode->sb;
        d_inode->file_type=cur_dir->file_flag&0x02?VFS_INODE_TYPE_DIR:VFS_INODE_TYPE_FILE;
        n_dir_elem->file=&d_inode->inode_ptr;
        d_inode->size_in_byte=cur_dir->data_len[0];
        if(dir_cnt==0)
        {
            strcpy(name_buf,".");
        }else if(dir_cnt==1)
        {
            strcpy(name_buf,"..");
        }
        if(cur_dir->fname_len && d_inode->file_type==VFS_INODE_TYPE_FILE)
        {
            if(iso_decode_fname(name_buf)==-1)
            {
                printf("[ISO]: Not ISO filename!\n");
                return VFS_FORMAT_ERR;
            }
        }
        //printf("Find a %s, name: %s lba:%d max_byte:%d;",((char *[]){"file","dir"})[d_inode->file_type],n_dir_elem->name,cur_dir->data_lba[0],d_inode->size_in_byte);
        list_append(&inode->sb->inode_list,&d_inode->inode_elem);
        dir_cnt++;
        list_append(&dir->file_elems,&n_dir_elem->list_tag);
        
        //printf("E");
    }
    iso_debug_list_dir(dir);
    return 0;
    
    
}
int __travel_dir(list_elem_t*elem,uint32_t arg)
{
    
    vfs_dir_elem_t *n_dir_elem= elem2entry(vfs_dir_elem_t,list_tag,elem);
    vfs_inode_t*inode=inode2ptr(n_dir_elem->file);
    if(inode->magic_num!=INODE_MAGIC_NUM)
    {
        printf("[ISO] Bad magic num!\n");
        return 1;
    }
    printf("Find a %s, name: %s max_byte:%d;\n",((char *[]){"file","dir"})[inode->file_type],n_dir_elem->name,inode->size_in_byte);   
    return 0;
}
void iso_debug_list_dir(vfs_dentry_t *dir)
{
    list_traversal(&dir->file_elems,&__travel_dir,0);
}
inode_ops_t iso_inode_ops={
    .fs_read=iso_inode_read,
    .fs_write=0,
    .fs_lseek=iso_inode_lseek
};
vfs_dentry_ops_t iso_dentry_ops={
    .load_inode_dirs=iso_dentry_inode_load,
    .find_dentry=iso_dentry_find
};

inode_handle iso_get_inode(uint32_t lba)
{
    vfs_inode_t *re=vfs_alloc_inode();
    if(!re)return 0;
    re->file_type=0;
    re->i_ops=&iso_inode_ops;
    re->inode_ptr=alloc_iso_inode_info;
    if(!re->inode_ptr)
    {
        kfree(re);
        return 0;
    }
    iso_inode_info_t *phy=re->inode_ptr;
    phy->base_offset=0;
    phy->phy_lba=lba;
    re->magic_num=INODE_MAGIC_NUM;
    re->refer_count=re->seek_offset=re->sync_mark=0;
    return &re->inode_ptr;
}
void iso_init_phy(iso_inode_info_t *info,uint32_t lba,uint32_t offset)
{
    info->phy_lba=lba;
    info->base_offset=offset;   
}
int iso_mount(vfs_super_block_t*sb,kdevice_t*dev)
{
    if(!dev)
    {
        printf("CANNOT FIND SUITABLE HARDWARE DISK FOR ISO MOUNTING...\nABORT!\n");
        while(1);
    }
    sb->dev=dev;
    if(!iso_inode_info_slab)
    {
        iso_inode_info_slab=alloc_slab_unit(sizeof(iso_inode_info_t),"iso_inode_info");
        if(!iso_inode_info_slab)return VFS_ALLOC_ERR;
    }
    printf("Mounting iso fs\n");
    //ata_dev=kobject_get_ops(KO_ATA_DEV);
    iso_volumn_group_t *volumn_info=detect_for_volumns(sb);
    if(!volumn_info)
    {
        return VFS_ALLOC_ERR;
    }
    if(!volumn_info->main_vol)
    {
        free_volumn_group(volumn_info);
        printf("No Main Volumn!\n");
        return VFS_FORMAT_ERR;
    }
    iso_dirent_t *root_dirent=&volumn_info->main_vol->root_dir[0];
    char *info=convert_str(volumn_info->main_vol->data_ready_desc,128);
    
    printf("\n%s\n",info);
    //OK ,
    //Now, lets alloc the root inode
    vfs_inode_t*root_inode= vfs_alloc_inode();
    vfs_dentry_t *root_dentry=vfs_alloc_dentry();
    root_dentry->ops=&iso_dentry_ops;
    if(!root_inode||!root_dentry){printf("[ISO]:Fail to alloc root_inode!\n");return VFS_ALLOC_ERR;}
    root_inode->i_ops=&iso_inode_ops;
    iso_dirent_t *iso_root_info=&volumn_info->main_vol->root_dir[0];
    printf("root_lba:%d\n",iso_root_info->data_lba[0]);
    root_inode->inode_ptr=alloc_iso_inode_info;
    printf("a");
    iso_init_phy(root_inode->inode_ptr,iso_root_info->data_lba[0],0);
    printf("b");
    if(!root_inode->inode_ptr)return VFS_ALLOC_ERR;
    root_inode->magic_num=INODE_MAGIC_NUM;
    root_inode->sb=sb;
    root_inode->refer_count=root_inode->seek_offset=root_inode->sync_mark=0;
    root_inode->size_in_byte=iso_root_info->data_len[0];
    root_inode->file_type=VFS_INODE_TYPE_DIR;
    printf("c");
    list_append(&sb->inode_list,&root_inode->inode_elem);
    printf("d");
    sb->root_dir=root_dentry;
    root_dentry->dir_file=&root_inode->inode_ptr;
    list_append(&sb->dentry_list,&root_dentry->dentry_elem);
    list_init(&root_dentry->file_elems);
    root_dentry->parent_dir=root_dentry;
    //root_dentry->dir_data_size_in_byte=iso_root_info->data_len[0];
    printf("%d",root_dentry->ops->load_inode_dirs(root_dentry,&root_inode->inode_ptr));
    printf("[ISO] mount root dir OK!\n");
    
    
    vfs_dir_elem_t *elem=root_dentry->ops->find_dentry(root_dentry,"boot");
    if(!elem)printf("[ISO] Cannot find 'boot'");
    printf("FIND! [%s]\n",elem->name);
}

vfs_sb_ops_t fs_ops={
    .fs_mount=iso_mount,
    .fs_umount=0
};
vfs_sb_ops_t *iso_getops()
{
    //
    return &fs_ops;
}