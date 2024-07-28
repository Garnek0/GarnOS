/*  
*   File: bcache.c
*
*   Author: Garnek
*   
*   Description: Buffer Cache
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <sys/dal/dal-internals.h>
#include <garn/mm.h>
#include <garn/panic.h>
#include <garn/kstdio.h>
#include <garn/kerrno.h>

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
                i->valid = true;
                i->dirty = false;
                if(bcache_read(i) != 0){
					i->refCount = 0;
					releaseLock(&bcache.spinlock);
					return NULL;
				}
                releaseLock(&bcache.spinlock);
                return i;
            }
        }
    });
    panic("No available bcache buffers!", "bcache");
    return NULL;
}

int bcache_read(bcache_buf_t* buf){
    lock(buf->lock, {
        if(buf->drive->read){
            buf->drive->read(buf->drive, buf->block, 1, buf->data);
        } else {
            klog("Cant read from drive \"%s\"! Read unimplemented.\n", KLOG_FAILED, "bcache", buf->drive->name);
            kerrno = ENOSYS;
            releaseLock(&buf->lock);
            return -1;
        }
    });

    return 0;
}

int bcache_write(bcache_buf_t* buf){
    lock(buf->lock, {
        if(buf->dirty){
            buf->drive->write(buf->drive, buf->block, 1, buf->data);
            if(buf->drive->write){
                buf->drive->write(buf->drive, buf->block, 1, buf->data);
                buf->dirty = false;
            } else {
                klog("Cant write to drive \"%s\"! Write unimplemented.\n", KLOG_FAILED, "bcache", buf->drive->name);
                kerrno = ENOSYS;
                releaseLock(&buf->lock);
                return -1;
            }
        }
    });

    return 0;
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

int sys_sync(stack_frame_t* regs){
    for(bcache_buf_t* i = bcache.head; i != NULL; i = i->next){
        if(i->dirty) bcache_write(i);
    }
    return 0;
}
