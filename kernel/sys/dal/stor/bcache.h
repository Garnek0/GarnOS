/*  
*   File: bcache.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef BCACHE_H
#define BCACHE_H

#define BCACHE_BUF_COUNT 30
#define BCACHE_BLOCK_SIZE 512

#include <types.h>
#include <cpu/smp/spinlock.h>
#include <cpu/interrupts/interrupts.h>
#include <sys/dal/dal.h>

typedef struct _bcache_buf {
    bool valid; //is this buffer actually loaded with the block?
    bool dirty; //has this buffer been modified?
    uint32_t refCount;
    uint8_t data[BCACHE_BLOCK_SIZE]; //the actual buffer
    size_t block;

    struct _drive* drive;

    struct _bcache_buf* next;
    struct _bcache_buf* prev;
} bcache_buf_t;

typedef struct {
    bcache_buf_t* head;
    bcache_buf_t* tail;
    bcache_buf_t buf[BCACHE_BUF_COUNT];

    spinlock_t spinlock;
} bcache_t;

void bcache_init();
bcache_buf_t* bcache_get(struct _drive* drive, size_t block);
bcache_buf_t* bcache_read(struct _drive* drive, size_t block);
void bcache_write(bcache_buf_t* buf);
void bcache_release(bcache_buf_t* buf);

int sys_sync(stack_frame_t* regs);

#endif //BCAHCE_H