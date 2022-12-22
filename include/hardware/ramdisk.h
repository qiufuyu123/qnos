#ifndef _H_RAMDISK
#define _H_RAMDISK
#include"hardware/devices.h"
#define RAMDISK_SIZE 2*1024*1024 //2mb
kdevice_t* device_create_ramdisk(int id);

#endif