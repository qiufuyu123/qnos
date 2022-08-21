#ifndef _LIB_KERNEL_LIST_H
#define _LIB_KERNEL_LIST_H

#include "types.h"

/**
 * 获取结构体内成员在结构体的偏移.
 */ 
#define __offset(struct_type, member) (int) (&((struct_type*)0)->member)

#define elem2entry(struct_type, struct_member_name, elem_ptr) \
    (struct_type*) ((int) elem_ptr - __offset(struct_type, struct_member_name))

/**
 * 链表节点.
 */ 
typedef struct list_elem {
    struct list_elem* prev;
    struct list_elem* next;
}list_elem_t ;

/**
 * 链表结构.
 */ 
typedef struct list {
    struct list_elem head;
    struct list_elem tail;
}list_t;

/**
 * 用于链表遍历的回调函数.
 */ 
typedef int (function) (struct list_elem*, int arg);

void list_init(struct list* list);
void list_insert_before(struct list_elem* before, struct list_elem* elem);
void list_push(struct list* list, struct list_elem* elem);
void list_append(struct list* list, struct list_elem* elem);
void list_remove(struct list_elem* elem);
struct list_elem* list_pop(struct list* list);
int list_find(struct list* list, struct list_elem* elem);
int list_empty(struct list* list);
uint32_t list_length(struct list* list);
struct list_elem* list_traversal(struct list* list, function func, int arg);

# endif