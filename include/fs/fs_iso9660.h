#ifndef _FS_ISO9660
#define _FS_ISO9660
#include"types.h"
#include"kobjects/obj_vfs.h"
#define VOL_BOOT_SYM 0
#define VOL_MAIN_SYM 1
#define VOL_END_SYM 255
#define ISO_ROOTDIR_SIZE 34
#pragma pack (1)
struct iso_volumn_symbol
{
    uint8_t type_code;
    char strA[5];
    int8_t version_code;
    uint8_t reserved[2041];
} PACKED;
typedef struct iso_volumn_symbol iso_volumn_symbol_t;
struct iso_date_info
{
    uint8_t year_num_since1900;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t GMT_offset;
}PACKED;
struct iso_ascii_date_info
{
    uint8_t year[4];
    uint8_t month[2];
    uint8_t day[2];
    uint8_t hour[2];
    uint8_t min[2];
    uint8_t sec[2];
    uint8_t secsec[2];// 1/100 sec
    uint8_t gmt_offset;
}PACKED;
struct iso_dirent
{
    uint8_t dir_record_len ;
    uint8_t dir_extra_record_len;
    uint32_t data_lba[2] ;
    uint32_t data_len[2] ;
    struct iso_date_info date ;
    uint8_t file_flag;
    uint8_t always_zero_size[2];
    uint16_t volumn_num[2];
    uint8_t fname_len;
    uint8_t fname_start;
}PACKED;
//int a= sizeof(struct iso_dirent);
typedef struct iso_dirent iso_dirent_t;

struct iso_main_volumn_info
{
    int8_t type_code;
    char strA[5];
    int8_t version_code;
    uint8_t reserve;
    char sys_desc_strA[32];
    char sys_desc_strD[32];
    uint8_t reserve2[8];
    uint32_t volumn_size[2];
    uint8_t reserve3[32];
    uint8_t i_dont_want_to_use_this[12];// Guess what
    uint32_t path_table_len[2];
    uint32_t path_table_lba_L;
    uint32_t path_table_lba_L_optional;
    uint32_t path_table_lba_M;
    uint32_t path_table_lba_M_optional;
    uint8_t root_dir[34];
    uint8_t volumn_group_desc[128];
    uint8_t publish_info[128];
    uint8_t data_ready_desc[128];
    uint8_t program_info[128];
    uint8_t copy_right_info[37];
    uint8_t vir_file_flag[37];
    uint8_t book_content_info[37];
    //447
    struct iso_ascii_date_info create_data;
    struct iso_ascii_date_info edit_data;
    struct iso_ascii_date_info end_data;
    struct iso_ascii_date_info val_data;
    uint8_t always_1;
    uint8_t free_always_zero;
    uint8_t used_program_free[512];
    uint8_t free_reserved[653];
}PACKED;
typedef struct iso_main_volumn_info iso_main_volumn_info_t;
//int b=sizeof(iso_main_volumn_info_t);
typedef struct iso_volumn_group
{
    iso_volumn_symbol_t *boot_vol;
    iso_main_volumn_info_t *main_vol;
    iso_volumn_symbol_t *end_vol;
    uint32_t main_lba;
}iso_volumn_group_t;
typedef struct iso_inode_info
{
    uint32_t phy_lba;
    //uint32_t size;
    uint32_t base_offset;
}iso_inode_info_t;

vfs_sb_ops_t *iso_getops();

#endif