#ifndef BCACHE_H
#define BCACHE_H

#include <garn/types.h>
#include <garn/spinlock.h>

#define BCACHE_BUF_COUNT 30

#define BCACHE_BLOCK_SIZE 512
 
typedef struct _bcache_buf {
    bool valid; //is this buffer actually loaded with the block?
    bool dirty; //has this buffer been modified?
    uint32_t refCount;
    uint8_t data[BCACHE_BLOCK_SIZE]; //the actual buffer
    size_t block;

    struct _drive* drive;

    struct _bcache_buf* next;
    struct _bcache_buf* prev;

    spinlock_t lock;
} bcache_buf_t;

bcache_buf_t* bcache_get(struct _drive* drive, size_t block);
int bcache_read(bcache_buf_t* buf);
int bcache_write(bcache_buf_t* buf);
void bcache_release(bcache_buf_t* buf);

#endif //BCACHE_H
