/** @file bcache.h
 * @brief Buffer cache manager
 * 
 * @author Garnek
 * @date 2024
 */

/*  
*   File: bcache.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef BCACHE_H
#define BCACHE_H

#include <garn/types.h>
#include <garn/spinlock.h>

/**
 * @brief Default number of buffer cache buffers
 * 
 */
#define BCACHE_BUF_COUNT 30

/**
 * @brief Default block size of a buffer cache buffer
 * 
 */
#define BCACHE_BLOCK_SIZE 512

/** @struct _bcache_buf
 * @brief Block cache buffer structure
 * 
 * @var _bcache_buf::valid
 * Set to **true** if this buffer is valid. Otherwise, set to **false**. This field is better left untouched, as the bcache component sets it accordingly.
 * 
 * @var _bcache_buf::dirty
 * Set to **true** if this buffer has been modified (i.e. written to). Otherwise, set to **false**. Modules are responsible for setting this field to **true** after every write to the bcache buffer.
 * 
 * @var _bcache_buf::refCount
 * Number of references to this buffer.
 * 
 * @var _bcache_buf::data
 * Holds the actual block.
 * 
 * @var _bcache_buf::block
 * Block number
 * 
 * @var _bcache_buf::drive
 * The drive this buffer corresponds to.
 * 
 * @var _bcache_buf::next
 * Next buffer in the buffer cache buffer list. Should not be modified.
 * 
 * @var _bcache_buf::prev
 * Previous buffer in the buffer cache buffer list. Should not be modified.
 * 
 * @var _bcache_buf::lock
 * Buffer lock
 */
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

/**
 * @brief Allocate/Get a bcache buffer.
 * 
 * The bcache component will try to fetch an existing buffer for the supplied drive and block. 
 * If there is no existing buffer, then a new one will be created and the block will be read from the drive into the buffer.
 * 
 * @param drive The drive to register the buffer for
 * @param block Block number.
 * @retval NULL An error occured.
 * @retval !NULL Buffer pointer
 * 
 * @note Buffer cache should only be used for blocks that are frequently accessed (e.g. filesystem metadata), and not for random drive accesses (e.g. file data), 
 * otherwise performance issues may appear.
 * 
 * @note When a bcache buffer is no longer needed, bcache_release() must be called to allow the system to reuse that buffer.
 */
bcache_buf_t* bcache_get(struct _drive* drive, size_t block);

/**
 * @brief Read data from the drive into the buffer.
 * 
 * The block from the drive (both the block and the drive are specified in the bcache buffer structure) gets written into the buffer (**buf** argument).
 * Any modifications not saved to the drive prior to calling this routine will be overwritten.
 * 
 * @param buf The bcache buffer
 * @retval 0 Success
 * @retval -1 An error occured. Check kerrno.
 */
int bcache_read(bcache_buf_t* buf);

/**
 * @brief Write data to the drive from the buffer.
 * 
 * The data from the buffer (**buf** argument) gets written to the drive, essentially "syncing" the drive's block with the buffer.
 * 
 * @param buf The bcache buffer
 * @retval 0 Success
 * @retval -1 An error occured. Check kerrno.
 */
int bcache_write(bcache_buf_t* buf);

/**
 * @brief Release a bcache buffer.
 * 
 * If buf->refCount > 1, then buf->refCount is simply decremented by one.
 * Else if buf->refCount == 1, then the buffer is completely freed and made available for (possible) future allocations.
 * 
 * @param buf The bcache buffer
 * 
 * @note bcache_release() should always be called after the kernel or a module is done using a bcache buffer.
 */
void bcache_release(bcache_buf_t* buf);

#endif //BCACHE_H
