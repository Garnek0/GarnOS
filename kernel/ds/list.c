/*  
*   File: list.c
*
*   Author: Garnek
*   
*   Description: Linked list data structure implementation
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "list.h"
#include <mem/mm/kheap.h>

list_t* list_create(char* name){
    list_t* list = kmalloc(sizeof(list_t));
    list->head = NULL;
    list->tail = NULL;
    list->nodeCount = 0;
    list->name = name;
    
    return list;
}

void list_destroy(list_t* list){
    foreach(node, list){
        kmfree((void*)node);
    }
    kmfree(list);
}

void list_insert(list_t* list, void* value){
    list_node_t* node = kmalloc(sizeof(list_node_t));
    node->value = value;

    if(list->nodeCount == 0){
        list->head = list->tail = node;
        node->next = NULL;
        list->nodeCount++;
    } else {
        list->tail->next = node;
        list->tail = node;
        list->nodeCount++;
    }
}

int list_index_of(list_t* list, void* value){
    int i = 0;
    foreach(node, list){
        if(node->value == value){
            return i;
        }
        i++;
    }
    return -1;
}

void* list_index(list_t* list, size_t index){
    int i = 0;
    foreach(node, list){
        if(i == index) return node->value;
        i++;
    }
    return NULL;
}