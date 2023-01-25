/**
 * @file kobject.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief Kernel object manager
 * @version 0.1
 * @date 2022-08-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _H_KOBJECT
#define _H_KOBJECT
#include"types.h"
#define KOBJ_MAX 128
enum{
    OPEN_NEW,
    OPEN_BY_OLD,
    OPEN_UPDATA
};
enum{
    KO_KERNEL_DEV,
    KO_ATA_DEV
};
/**
 * @brief SLAB FOR KOBJ
 * We use a slab to alloc and free all kobject
 */
#include"mem/malloc.h"

//kernel handle for kobject resources
typedef uint32_t KHANDLE;
typedef struct kobject_operations
{
    uint32_t(*write)(uint32_t idx, void*buffer,uint32_t size,uint32_t flags);
    uint32_t(*read)(uint32_t idx, void*buffer,uint32_t size,uint32_t flags);
    uint32_t(*open)(uint32_t val,uint32_t flags);
    uint32_t(*attrset)(uint32_t attr,uint32_t val);
    uint32_t(*attrget)(uint32_t attr);
    uint32_t(*exfunc)(uint32_t v1,uint32_t v2,uint32_t v3,uint32_t v4);
}kobject_operations_t;
#define NONE_WRITE uint32_t non_write(uint32_t idx,uint32_t buffer,uint32_t sz,uint32_t flgs){return 0;}
#define NONE_READ uint32_t non_read(uint32_t idx,uint32_t buffer,uint32_t sz,uint32_t flgs){return 0;}
#define NONE_OPEN uint32_t non_open(uint32_t val,uint32_t flgs){return 0;}
#define NONE_ATTRSET uint32_t non_attrset(uint32_t idx,uint32_t flgs){return 0;}
#define NONE_EXFUNC uint32_t non_exfunc(uint32_t v1,uint32_t v2,uint32_t v3,uint32_t v4){return 0;}
#define NONE_BEGIN uint32_t non_begin(uint32_t vals){}
#define NONE_END void non_end(){}
typedef struct kobject
{
    //uint8_t* data;
    KHANDLE type;
    uint32_t(*begin)(uint32_t vals);
    uint32_t(*end)();
    kobject_operations_t *ops;
    char name[32];
    uint32_t name_hash;
}kobject_t;

void init_kobject();
kobject_operations_t *kobject_get_ops(uint32_t type);
kobject_t* create_kobject(uint32_t(*begin)(uint32_t val),uint32_t(*end)(),kobject_operations_t *op,char *name);
kobject_t* kobject_find(char *name);
void release_kobject(kobject_t*ko);




#endif