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
#include <sys/panic.h>
#include <sys/bootloader.h>

kheap_block_header_t* start;
kheap_block_header_t* end;

size_t kheapSize;

kheap_info_t kheap_info;

static void kheap_extend(size_t size){
    size = ALIGN_UP((size+sizeof(kheap_block_header_t)), PAGE_SIZE);
    kheap_block_header_t* newh = (kheap_block_header_t*)((uint64_t)pmm_allocate(size/PAGE_SIZE) + bl_get_hhdm_offset());
    memset(newh, 0, size);
    newh->size = size - sizeof(kheap_block_header_t);
    newh->flags = KHEAP_FLAGS_FREE;
    newh->prev = end;
    newh->next = NULL;

    kheap_info.kheapSize += size;
    
    end->next = newh;
    end = newh;

    if(newh->prev && (newh->prev->flags & KHEAP_FLAGS_FREE)){
        kheap_block_header_t* prev = newh->prev;
        prev->size += newh->size;
        if(newh->next){
            newh->next->prev = prev;
        }
        prev->next = newh->next;
        
    }
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
    start = end = (kheap_block_header_t*)((uint64_t)pmm_allocate(KHEAP_INIT_PAGES) + bl_get_hhdm_offset()); //Initial kernel heap size is 16KiB
    kprintf("%p", end);
    memset(start, 0, sizeof(kheap_block_header_t));
    start->size = KHEAP_INIT_PAGES * PAGE_SIZE - sizeof(kheap_block_header_t);
    start->prev = NULL;
    start->flags = KHEAP_FLAGS_FREE;

    kheap_info.kheapSize = PAGE_SIZE*KHEAP_INIT_PAGES;

    klog("kheap: Kernel Heap Initialised Successfully (kheap size: %uKiB)\n", KLOG_OK, (PAGE_SIZE*KHEAP_INIT_PAGES)/1024);
}

void* kmalloc(size_t size){
    size = ALIGN_UP(size, 0x10);
    kheap_block_header_t* h;

    lock(kheap_info.lock, {
        for(h = start; h; h = h->next){
            if(!(h->flags & KHEAP_FLAGS_FREE)) continue;
            if(h->size == size){
                h->flags &= ~(KHEAP_FLAGS_FREE);
                releaseLock(&kheap_info.lock);
                return (void*)((uint64_t)h + sizeof(kheap_block_header_t));
            } else if (h->size > size + sizeof(kheap_block_header_t)){
                kheap_create_block(h, size);
                h->flags &= ~(KHEAP_FLAGS_FREE);
                releaseLock(&kheap_info.lock);
                return (void*)((uint64_t)h + sizeof(kheap_block_header_t));
            }
        }
        kheap_extend(size);
    });

    return kmalloc(size);
}

void kmfree(void* ptr){
    kheap_block_header_t* h;
    h = (kheap_block_header_t*)((uint64_t)ptr - sizeof(kheap_block_header_t));

    if(h->flags & KHEAP_FLAGS_FREE)
        klog("kheap: Invalid kheap free operation", KLOG_WARNING);

    lock(kheap_info.lock, {

        h->flags |= KHEAP_FLAGS_FREE;

        if(h->next && (h->next->flags & KHEAP_FLAGS_FREE) && ((void*)((uint64_t)h+(uint64_t)h->size+sizeof(kheap_block_header_t)) == h->next)){
            kheap_block_header_t* next = h->next;
            h->size += next->size;
            if(next->next){
                next->next->prev = h;
            }
            h->next = next->next;
        }

        if(h->prev && (h->prev->flags & KHEAP_FLAGS_FREE) && ((void*)((uint64_t)h->prev+(uint64_t)h->prev->size+sizeof(kheap_block_header_t)) == h)){
            kheap_block_header_t* prev = h->prev;
            prev->size += h->size;
            if(h->next){
                h->next->prev = prev;
            }
            prev->next = h->next;
            
        }
    });
}