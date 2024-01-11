/*  
*   File: bcache.c
*
*   Author: Garnek
*   
*   Description: Buffer Cache
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "bcache.h"
#include <mem/kheap/kheap.h>
#include <sys/panic.h>
#include <kstdio.h>

bcache_t bcache;

void bcache_init(){
    bcache.head = &bcache.buf[0];
    bcache.tail = &bcache.buf[BCACHE_BUF_COUNT-1];

    for(int i = 0; i < BCACHE_BUF_COUNT-1; i++){
        bcache.buf[i].next = &bcache.buf[i+1];
    }
    bcache.tail->next = NULL;
    for(int i = BCACHE_BUF_COUNT-1; i > 0; i--){
        bcache.buf[i].prev = &bcache.buf[i-1];
    }
    bcache.head->prev = NULL;
}

bcache_buf_t* bcache_get(drive_t* drive, size_t block){
    lock(bcache.spinlock, {
        for(bcache_buf_t* i = bcache.head; i != NULL; i = i->next){
            if(i->drive == drive && i->block == block){
                i->refCount++;
                releaseLock(&bcache.spinlock);
                return i;
            }
        }

        for(bcache_buf_t* i = bcache.head; i != NULL; i = i->next){
            if(i->refCount == 0){
                i->refCount++;
                i->block = block;
                i->drive = drive;
                i->valid = false;
                i->dirty = false;
                releaseLock(&bcache.spinlock);
                return i;
            }
        }
    });
    panic("No available bcache buffers!");
    return NULL;
}

bcache_buf_t* bcache_read(drive_t* drive, size_t block){
    bcache_buf_t* buf = bcache_get(drive, block);
    if(!buf->valid){
        drive->read(drive, block, 1, buf->data);
        buf->valid = true;
    }
    return buf;
}

void bcache_write(bcache_buf_t* buf){
    if(buf->dirty){
        buf->drive->write(buf->drive, buf->block, 1, buf->data);
        buf->dirty = false;
    }
}

void bcache_release(bcache_buf_t* buf){
    lock(bcache.spinlock, {
        buf->refCount--;

        if(buf->refCount == 0){
            bcache.tail->next = buf;
            if(buf->next) buf->next->prev = buf->prev;
            if(buf->prev) buf->prev->next = buf->next;
            buf->prev = bcache.tail;
            buf->next = NULL;
            bcache.tail = buf;
        }
    });
}

int sys_sync(){
    for(bcache_buf_t* i = bcache.head; i != NULL; i = i->next){
        if(i->dirty) bcache_write(i);
    }
    return 0;
}