/**
 * @file circlequeue.h
 * @author qiufuyu (3038742090@qq.com)
 * @brief As the file name
 * @version 0.1
 * @date 2022-08-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _H_CIRCLE_QUEUE
#define _H_CIRCLE_QUEUE
#include"types.h"


/**
 * @brief REALIZE
 * 
 * Remember, This is a very tiny circle queue
 * 
 * First, we create a linear data buffer like this:
 *  [0,0,0,0,...0]
 * And we have 2 pointer called 'foot' and 'head'
 *  [0,0,0...0]
 *  f/h
 * First init, the data like this ^^^^^^^^^^, foot and head at the same position
 * While writing data, the head pointer move
 * [0,0,0,...0]
 *  f        h 
 * Like this ^
 * 
 * When h move to the end, it comes back
 * [0,0,0,...0]
 * f/h
 * When h > f, it means queue is full
 * When we want to pick up elements
 * the f pointer moves
 * When h < f again, it means the queue has free capacity now.
 * 
 *          modify by qiufuyu
 */

typedef struct circlequeue
{
    char *data;
    uint32_t data_len;
    uint32_t data_unit_len;
    uint32_t used_cnt;
    int foot;   //use -1 represents not inited at all
    int head;
}circlequeue_t;

int circlequeue_init(circlequeue_t* queue,uint32_t unit_cnt,uint32_t unit_len);

bool circlequeue_empty(circlequeue_t*queue);

bool circlequeue_full(circlequeue_t*queue);

uint32_t circlequeue_get(circlequeue_t* queue);

int circlequeue_push(circlequeue_t*queue,char *data);

void circlequeue_release(circlequeue_t* queue);


#endif