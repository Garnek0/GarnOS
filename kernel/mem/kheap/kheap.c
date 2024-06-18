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

#include <mem/mm-internals.h>
#include <garn/kstdio.h>
#include <garn/panic.h>
#include <sys/bootloader.h>

kheap_block_header_t* start;
kheap_block_header_t* end;

size_t kheapSize;

spinlock_t kheapLock;

static void kheap_extend(size_t size){
    size += sizeof(kheap_block_header_t);
    if(size%PAGE_SIZE != 0) size = ALIGN_UP(size, PAGE_SIZE);
    kheap_block_header_t* newh = (kheap_block_header_t*)((uint64_t)pmm_allocate(size/PAGE_SIZE) + bl_get_hhdm_offset());
    memset(newh, 0, size);
    newh->size = size - sizeof(kheap_block_header_t);
    newh->flags = KHEAP_FLAGS_FREE;
    newh->prev = end;
    newh->next = NULL;

    kheapSize += size;
    
    end->next = newh;
    end = newh;

    if(newh->prev && (newh->prev->flags & KHEAP_FLAGS_FREE) && ((void*)((uint64_t)newh->prev+(uint64_t)newh->prev->size+sizeof(kheap_block_header_t)) == newh)){
        kheap_block_header_t* prev = newh->prev;
        prev->size += newh->size + sizeof(kheap_block_header_t);
        prev->next = NULL;
        end = prev;
    }

    klog("Extended kheap size. New size: %uKiB.\n", KLOG_INFO, "kheap", kheapSize/1024);
}

static void kheap_create_block(kheap_block_header_t* h, size_t size){
    kheap_block_header_t* newh = (kheap_block_header_t*)((uint64_t)h + sizeof(kheap_block_header_t) + size);
    memset(newh, 0, sizeof(kheap_block_header_t));

    newh->size = h->size - (size + sizeof(kheap_block_header_t));
    h->size = size;
    newh->next = h->next;
    h->next = newh;
    newh->prev = h;
    newh->flags = KHEAP_FLAGS_FREE;
    if(newh->next){
        newh->next->prev = newh;
    }

    if(h == end) end = newh;
}

void kheap_init(){
    start = end = (kheap_block_header_t*)((uint64_t)pmm_allocate(KHEAP_INIT_PAGES) + bl_get_hhdm_offset()); //Initial kernel heap size is 64KiB
    memset(start, 0, sizeof(kheap_block_header_t));
    start->size = (KHEAP_INIT_PAGES * PAGE_SIZE) - sizeof(kheap_block_header_t);
    start->prev = NULL;
    start->next = NULL;
    start->flags = KHEAP_FLAGS_FREE;

    kheapSize = PAGE_SIZE*KHEAP_INIT_PAGES;

    klog("Kernel Heap Initialised Successfully (kheap size: %uKiB)\n", KLOG_OK, "kheap", (PAGE_SIZE*KHEAP_INIT_PAGES)/1024);
}

void* kmalloc(size_t size){
    if(size%0x10 != 0) size = ALIGN_UP(size, 0x10);
    kheap_block_header_t* h;

    lock(kheapLock, {
        for(h = start; h; h = h->next){
            if(!(h->flags & KHEAP_FLAGS_FREE) || h->size < size) continue;
            if(h->size == size){
                h->flags &= ~(KHEAP_FLAGS_FREE);
                releaseLock(&kheapLock);
                return (void*)((uint64_t)h + sizeof(kheap_block_header_t));
            } else if (h->size > (size + sizeof(kheap_block_header_t))){
                kheap_create_block(h, size);
                h->flags &= ~(KHEAP_FLAGS_FREE);
                releaseLock(&kheapLock);
                return (void*)((uint64_t)h + sizeof(kheap_block_header_t));
            }
        }
        kheap_extend(size);
    });

    return kmalloc(size);
}

void kmfree(void* ptr){
    if(ptr == NULL){
        klog("kheap free operation with NULL pointer!\n", KLOG_WARNING, "kheap");
        return;
    }
    kheap_block_header_t* h;
    h = (kheap_block_header_t*)((uint64_t)ptr - sizeof(kheap_block_header_t));

    if(h->flags & KHEAP_FLAGS_FREE)
        klog("Invalid kheap free operation!\n", KLOG_WARNING, "kheap");

    lock(kheapLock, {
        h->flags |= KHEAP_FLAGS_FREE;

        if(h->next && (h->next->flags & KHEAP_FLAGS_FREE) && ((void*)((uint64_t)h+(uint64_t)h->size+sizeof(kheap_block_header_t)) == h->next)){
            kheap_block_header_t* next = h->next;
            if(next == end) end = h;
            h->size += next->size + sizeof(kheap_block_header_t);
            if(next->next){
                next->next->prev = h;
            }
            h->next = next->next;
        }

        if(h->prev && (h->prev->flags & KHEAP_FLAGS_FREE) && ((void*)((uint64_t)h->prev+(uint64_t)h->prev->size+sizeof(kheap_block_header_t)) == h)){
            kheap_block_header_t* prev = h->prev;
            if(h == end) end = prev;
            prev->size += h->size + sizeof(kheap_block_header_t);
            if(h->next){
                h->next->prev = prev;
            }
            prev->next = h->next;
            
        }
    });
}

inline size_t kheap_get_size(){
    return kheapSize;
}