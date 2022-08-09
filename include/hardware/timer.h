/**
 * @file timer.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief Timer for process
 * @version 0.1
 * @date 2022-07-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _H_TIMER
#define _H_TIMER
#include"types.h"
void init_timer();
void reset_tick();
uint32_t get_tick();
void ksleep(uint32_t ms);

#endif