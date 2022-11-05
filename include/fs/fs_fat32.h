#ifndef _H_FAT32
#define _H_FAT32
#include"types.h"
#pragma pack (1)
typedef struct fat32_bpb
{
    uint8_t boot_code[3];
}PACKED ;
typedef struct fat32_bpb fat32_bpb_t;



#endif