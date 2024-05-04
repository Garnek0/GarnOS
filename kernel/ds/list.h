/*  
*   File: list.h
*
*   Author: Garnek
*   
*   Description: Linked list data structure implementation
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef DS_LIST_H
#define DS_LIST_H

#include <types.h>
#include <cpu/multiproc/spinlock.h>

typedef struct _list_node {
    struct _list_node* next;
    void* value;
} list_node_t;

#define foreach(i, list) for (list_node_t* i = (list)->head; i != NULL; i = i->next)

typedef struct {
    list_node_t* head;
    list_node_t* tail;
    size_t nodeCount;

    spinlock_t lock; 
} list_t;

list_t* list_create();
void list_destroy(list_t* list);
void list_insert(list_t* list, void* value);
int list_index_of(list_t* list, void* value);
void* list_index(list_t* list, size_t index);
int list_remove(list_t* list, void* value);
int list_remove_index(list_t* list, size_t index);

#endif //DS_LIST_H