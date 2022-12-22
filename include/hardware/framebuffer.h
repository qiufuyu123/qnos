#ifndef _H_FRAMEBUFFER
#define _H_FRAMEBUFFER
#include"hardware/devices.h"
typedef struct 
{
    uint32_t width;
    uint32_t height;
    uint32_t channel;
}framebuffer_cfg_t;
#define FMB_CFG 1
#define FMB_FRESH 0
kdevice_t* device_create_framebuffer(int id);



#endif