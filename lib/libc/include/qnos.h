#ifndef _H_QNOS
#define _H_QNOS

#include"sys/types.h"


khandle_t qnkobject_get(char *name);

int qnkobject_setattr(khandle_t obj,uint32_t attr,uint32_t val);

int qnkobject_getattr(khandle_t obj,uint32_t attr);


#endif