/*  
*   File: kheap.c
*
*   Author: Garnek
*   
*   Description: Kernel heap implementation
*/
// SPDX-License-Identifier: BSD-2-Clause

//TODO: This kernel heap implementation has some massive flaws. I think, at this point, the only
//Reasonable choice is to rewrite the entire thing from scratch.

#include "kheap.h"
#include <mem/mm/pmm.h>
#include <mem/memutil/memutil.h>
#include <kstdio.h>
#include <sys/rblogs.h>
#include <sys/panic.h>
#include <sys/bootloader.h>

kheap_block_header_t* start;
kheap_block_header_t* end;

size_t kheapSize;

static void kheap_extend(size_t size){
    size = ALIGN_UP((size+sizeof(kheap_block_header_t)), PAGE_SIZE);
    kheap_block_header_t* newh = (kheap_block_header_t*)pmm_allocate(size/PAGE_SIZE);
    memset(newh, 0, size);
    newh->size = size - sizeof(kheap_block_header_t);
    newh->flags = 0;
    newh->prev = end;
    newh->next = NULL;

    kheapSize += size;
    
    end->next = newh;
    end = newh;

    kmfree((kheap_block_header_t*)((uint64_t)newh+sizeof(kheap_block_header_t)));
}

static void kheap_create_block(kheap_block_header_t* h, size_t size){
    kheap_block_header_t* newh = (kheap_block_header_t*)((uint64_t)h + size + sizeof(kheap_block_header_t));

    newh->size = h->size - size;
    h->size = size;
    newh->next = h->next;
    h->next = newh;
    newh->prev = h;
    newh->flags = h->flags;
    h->flags = KHEAP_FLAGS_FREE;
    if(newh->next){
        newh->next->prev = newh;
    }

    //calculate new end
    for(kheap_block_header_t* nend = start; nend; nend = nend->next){
        end = nend;
    }
}

void kheap_init(){
    start = end = (kheap_block_header_t*)pmm_allocate(KHEAP_INIT_PAGES); //Initial kernel heap size is 16KiB
    memset(start, 0, sizeof(kheap_block_header_t));
    start->size = KHEAP_INIT_PAGES * PAGE_SIZE - sizeof(kheap_block_header_t);
    start->prev = NULL;
    start->flags = KHEAP_FLAGS_FREE;

    kheapSize = PAGE_SIZE*KHEAP_INIT_PAGES;

    klog("Kernel Heap Initialised Successfully (kheap size: %uKiB)\n", KLOG_OK, (PAGE_SIZE*KHEAP_INIT_PAGES)/1024);
    rb_log("kheap", KLOG_OK);
}

void* kmalloc(size_t size){
    size = ALIGN_UP(size, 0x10);
    kheap_block_header_t* h;
    for(h = start; h; h = h->next){
        if(!(h->flags & KHEAP_FLAGS_FREE)) continue;
        if(h->size == size){
            h->flags &= ~(KHEAP_FLAGS_FREE);
            return (void*)((uint64_t)h + sizeof(kheap_block_header_t)) + bl_get_hhdm_offset();
        } else if (h->size > size + sizeof(kheap_block_header_t)){
            kheap_create_block(h, size);
            h->flags &= ~(KHEAP_FLAGS_FREE);
            return (void*)((uint64_t)h + sizeof(kheap_block_header_t)) + bl_get_hhdm_offset();
        }

    }
    kheap_extend(size);
    return kmalloc(size);
}

void kmfree(void* ptr){
    kheap_block_header_t* h;
    h = (kheap_block_header_t*)((uint64_t)ptr - sizeof(kheap_block_header_t));

    if(h->flags & KHEAP_FLAGS_FREE)
        klog("Invalid kheap free operation", KLOG_WARNING);

    h->flags |= KHEAP_FLAGS_FREE;

    if(h->next && (h->next->flags & KHEAP_FLAGS_FREE)){
        kheap_block_header_t* next = h->next;
        h->size += next->size;
        if(next->next){
            next->next->prev = h;
        }
        h->next = next->next;
    }

    if(h->prev && (h->prev->flags & KHEAP_FLAGS_FREE)){
        kheap_block_header_t* prev = h->prev;
        prev->size += h->size;
        if(h->next){
            h->next->prev = prev;
        }
        prev->next = h->next;
        
    }
}