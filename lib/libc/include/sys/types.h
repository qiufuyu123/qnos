#ifndef _H_SYS_TYPES
#define _H_SYS_TYPES
#include"inttypes.h"
typedef struct 
{
    uint32_t width;
    uint32_t height;
    uint32_t channel;
}framebuffer_cfg_t;

typedef uint32_t khandle_t;
#define NAME_MAX 255
#endif