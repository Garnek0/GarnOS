/*  
*   File: spinlock.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <types.h>

typedef uint64_t spinlock_t;

#define lock(l, cmds)  \
{                      \
    acquireLock(&l);   \
    cmds;              \
    releaseLock(&l);   \
}

void releaseLock(spinlock_t *locker);
void acquireLock(spinlock_t *locker);

static inline void pause(){
    asm volatile("pause");
}

#endif //SPINLOCK_H
