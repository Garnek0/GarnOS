/*  
*   File: kheap.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef KHEAP_H
#define KHEAP_H

#include <types.h>

#define KHEAP_FLAGS_FREE 1

#define KHEAP_INIT_PAGES 16

typedef struct _kheap_block_header {
    size_t size;
    uint8_t flags; //bit 0 - block is free 
    struct _kheap_block_header* next;
    struct _kheap_block_header* prev;
} kheap_block_header_t;

extern size_t kheapSize;

void kheap_init();
void* kmalloc(size_t size);
void kmfree(void* ptr);

#endif //KHEAP_H