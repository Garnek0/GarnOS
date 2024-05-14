/*  
*   File: mm-internals.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MM_INTERNALS_H
#define MM_INTERNALS_H

#include <garn/types.h>
#include <garn/spinlock.h>
#include <garn/irq.h>
#include <garn/mm.h>

struct _thread;

typedef struct _kheap_block_header {
    size_t size;
    uint8_t flags; //bit 0 - block is free 
    struct _kheap_block_header* next;
    struct _kheap_block_header* prev;
} kheap_block_header_t;

//kheap

void kheap_init();

//pmm

void pmm_init();

//vmm

void vmm_init();

//vaspace

page_table_t* vaspace_new();
void vaspace_destroy(page_table_t* pml4);
void vaspace_clear(page_table_t* pml4);
page_table_t* vaspace_clone(page_table_t* toClone);
void vaspace_switch(page_table_t* pml4);
void* vaspace_create_area(page_table_t* pml4, uint64_t virtAddr, size_t size, uint32_t flags);
void vaspace_create_thread_user_stack(struct _thread* thread);

void* sys_mmap(stack_frame_t* regs, void* addr, size_t length, int prot, int flags, int fd, uint64_t offset);
int sys_munmap(stack_frame_t* regs, void* addr, size_t length);

#endif //MM_INTERNALS_H