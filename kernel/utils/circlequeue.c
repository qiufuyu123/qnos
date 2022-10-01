#include"utils/circlequeue.h"
#include"mem/malloc.h"
#include"string.h"
int circlequeue_init(circlequeue_t* queue,uint32_t unit_cnt,uint32_t unit_len)
{
    queue->data_len=unit_cnt*unit_len;
    queue->data_unit_len=unit_len;
    queue->data=kmalloc(queue->data_len);
    if(!queue->data)return -1;
    queue->foot=0;
    queue->head=0;
    queue->used_cnt=0;
    return 1;
}

uint32_t circlequeue_get(circlequeue_t* queue)
{
    if(!queue)return 0;
    if(queue->used_cnt==0)return 0;
    uint32_t ptr=(uint32_t)queue->data+queue->foot*queue->data_unit_len;
    queue->used_cnt--;
    if((queue->foot+1)*queue->data_unit_len>=queue->data_len)queue->foot=0;
    else queue->foot++;
    return ptr;
}
bool circlequeue_empty(circlequeue_t*queue)
{
    return queue->used_cnt==0;
}
bool circlequeue_full(circlequeue_t*queue)
{
    return (queue->used_cnt>=queue->data_len/queue->data_unit_len);
}
int circlequeue_push(circlequeue_t*queue,char *data)
{
    if(!queue|!data)return 0;
    uint32_t max_cnt=queue->data_len/queue->data_unit_len;
    if(queue->used_cnt>=max_cnt)return -1;
    memcpy((uint32_t)queue->data+queue->head*queue->data_unit_len,data,queue->data_unit_len);
    queue->used_cnt++;
    if((queue->head+1)*queue->data_unit_len>=queue->data_len)queue->head=0;
    else queue->head++;
    return 1;
}

void circlequeue_release(circlequeue_t* queue)
{

}