/**
 * @file ata.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief A simple driver for ata devices 
 * @version 0.1
 * @date 2022-08-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _H_ATA
#define _H_ATA
#include"types.h"


typedef struct 
{
    
}ata_device_t;


void ide_initialize(unsigned int BAR0, unsigned int BAR1, unsigned int BAR2, unsigned int BAR3,
unsigned int BAR4);
int ide_read_sectors(unsigned char drive, unsigned char numsects, unsigned int lba,
                      uint8_t *buffer);
#endif