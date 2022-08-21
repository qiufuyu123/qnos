/**
 * @file sync.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief Some methods to realize thread sync
 * @version 0.1
 * @date 2022-08-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _H_SYNC
#define _H_SYNC
#include"types.h"
#include"list.h"
#include"process/task.h"
typedef struct lock
{
    list_t wait_list;
    TCB_t *holder_task;
    uint32_t repeat_ref_count;
}lock_t;


void lock_init(lock_t*lock);
void lock_acquire(lock_t*lock);
void lock_release(lock_t *lock);
#endif