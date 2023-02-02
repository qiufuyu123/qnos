#ifndef _H_BOOTCONFIG
#define _H_BOOTCONFIG
#include"types.h"

typedef struct 
{
    bool locked;    // if boot is locked, kernel will not load
    char versions[3];
    char init_bin[32];

    bool console_double_buffered;   //whether to switch on the kernel-tty's double buffer
    uint32_t sleep_after_boot;  //ms
    
}kbootconfig_t;


#endif