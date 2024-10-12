#include <garn/ds/list.h>
#include <garn/mm.h>
#include <garn/kstdio.h>

//Singly linked list data structure

list_t* list_create(){
    list_t* list = kmalloc(sizeof(list_t));
    list->head = NULL;
    list->tail = NULL;
    list->nodeCount = 0;
    list->lock = 0;
    
    return list;
}

void list_destroy(list_t* list){
    lock(list->lock, {
        foreach(node, list){
            kmfree((void*)node);
        }
        kmfree(list);
    });
}

void list_insert(list_t* list, void* value){
    list_node_t* node = kmalloc(sizeof(list_node_t));
    node->value = value;

    lock(list->lock, {
        if(list->nodeCount == 0){
            list->head = list->tail = node;
        } else {
            list->tail->next = node;
            list->tail = node;
        }
        node->next = NULL;
        list->nodeCount++;
    });
}

int list_index_of(list_t* list, void* value){
    int i = 0;
    lock(list->lock, {
        foreach(node, list){
            if(node->value == value){
                releaseLock(&list->lock);
                return i;
            }
            i++;
        }
    });
    return -1;
}

void* list_get(list_t* list, size_t index){
    size_t i = 0;

    lock(list->lock, {
        foreach(node, list){
            if(i == index){
                releaseLock(&list->lock);
                return node->value;
            }
            i++;
        }
    });
    return NULL;
}

int list_remove(list_t* list, void* value){
    list_node_t* prev = NULL;
    lock(list->lock, {
        foreach(node, list){
            if(node->value == value){
                if(list->head == node){
                    list->head = node->next;
                }
                if(list->tail == node){
                    list->tail = prev;
                }
                if(prev != NULL){
                    prev->next = node->next;
                }
                list->nodeCount--;
                kmfree(node);
                releaseLock(&list->lock);
                return 0;
            }
            prev = node;
        }
    });
    return -1;
}

int list_remove_index(list_t* list, size_t index){
    list_node_t* prev = NULL;
    size_t i = 0;
    lock(list->lock, {
        foreach(node, list){
            if(i == index){
                if(list->head == node){
                    list->head = node->next;
                }
                if(list->tail == node){
                    list->tail = prev;
                }
                if(prev != NULL){
                    prev->next = node->next;
                }
                list->nodeCount--;
                kmfree(node);
                releaseLock(&list->lock);
                return 0;
            }
            prev = node;
            i++;
        }
    });
    return -1;
}
