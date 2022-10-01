#ifndef _H_SYSMODULE
#define _H_SYSMODULE
#include"types.h"
#include"process/task.h"
#include"kelf.h"
#include"kobjects/kobject.h"
#include"KMDK/KMDK.h"
#define SYSM_NULL 0
#define SYSM_EXECUTABLE 1
#define SYSM_RESOURCES 2

typedef struct sysmodule
{
    struct elf_module *elf;
    uint8_t sys_type;
    kobject_t *bind_obj;
    //char *name;
    KMDKInfo_t *km_info;
    TCB_t *bind_thread;

    int (*init_func)(struct sysmodule *m);
    int (*release_func)();
    int(*main_thread)();
    int (*resource_handle)();
    int (*user_main_thread)(int argc,char **argv);
}sysmodule_t;


sysmodule_t* load_sys_module(char *path);

void unload_sys_module(sysmodule_t*m);

int execute_sys_module(sysmodule_t*m);

void sys_module_dump(sysmodule_t*m);
#endif