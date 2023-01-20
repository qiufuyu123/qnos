#ifndef _H_FAT32
#define _H_FAT32
#include"types.h"
#pragma pack(1)
typedef struct fat32_bpb
{
    char __boot_cmd[3];
    char oem_flag[8];
    short bytes_per_sector;
    char sectors_per_group;
    short num_of_reserved_sects;
    char num_of_fat;
    short num_of_root_path;
    short num_of_sects_in_logic;
    char media_desc;
    char __useless[2];
    short num_of_sects_per_track;
    short __useless2;
    uint32_t num_of_hidden_sects;
    uint32_t largenum_of_sects;

    uint32_t sects_per_fat;
    short __flag;
    char fat_ver[2];
    uint32_t fat_of_root;
    short fs_info_sect;
    short backup_boot_sect_num;
    char __useless3[12];
    char idx_of_driver;
    char __useless4;
    char sign;
    uint32_t series_num;
    char volumn_mark_str[11];
    char sys_mark_str[8];//ALWAYS FAT32
    char boot_code[420];
    short boot_sign;//0xaa55
}PACKED ;
typedef struct fat32_bpb fat32_bpb_t;
typedef struct fat_fs_info
{
    uint32_t client_sign;//must be 0x41615252
    char __useless[480];
    uint32_t client_sign2;//must be 0x61417272
    uint32_t cnt_in_last_useful_fat;
    uint32_t num_of_start_search_fat;
    char __useless1[12];
    uint32_t data_sect_start;
    //       ^^^^^^^^^^^^^^
    //DO NOT USE THIS VARIABLE WHEN FIRST READING!
    //Variable data_sect_start won't be initiated until mount_fs
    //
}PACKED ;
typedef struct fat_fs_info fat_fs_info_t;
typedef struct fat_dir_desc
{
    char fname[8];//1st char can be 0x00:none file behind or oxe5:erased entry(you shouldn't read it)
    char fextname[3];
    char attr;
    //READ_ONLY=0x01 HIDDEN=0x02 SYSTEM=0x04 VOLUME_ID=0x08 DIRECTORY=0x10 ARCHIVE=0x20 LFN=READ_ONLY|HIDDEN|SYSTEM|VOLUME_ID 
    char __useless;
    char create_time;
    char file_create_h:5;
    char file_create_m:6;
    char file_create_s:5;
    char file_create_date_y:7;
    char file_create_date_m:4;
    char file_create_date_d:5;
    char last_access_date[2];
    short high_16_entry_num;
    char last_modify_time[2];
    char last_modify_date[2];
    short low_16_entry_num;
    uint32_t size_in_byte;
}PACKED ;
typedef struct fat_dir_desc fat_dir_desc_t;
typedef struct 
{
    uint32_t *data;
    uint32_t start_cluster;
}fat_fattable_cache_t;
typedef struct 
{
    uint32_t size_byte;
    char *buffer;
    uint32_t fst_fat;
    uint32_t lst_fat;
    uint32_t dir_offset;
    char pg_nd;
}fat_file_cache_t;

typedef struct fat_data
{
    fat32_bpb_t bpb;
    fat_fs_info_t fs_info;
    fat_fattable_cache_t *cache_list;
    uint32_t max_cluster_num;
}fat_data_t;

//#define FAT_CACHE_MAX 3
vfs_sb_ops_t *fat_getops();
#pragma pack()
#endif