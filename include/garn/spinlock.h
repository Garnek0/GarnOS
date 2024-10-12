#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <garn/types.h>
#include <garn/arch.h>

typedef uint64_t spinlock_t;

#define lock(l, cmds)  \
{                      \
    acquireLock(&l);   \
    cmds;              \
    releaseLock(&l);   \
}

// Implemented in each architecture's arch/*/ subdirectory
void releaseLock(spinlock_t *lock);
void acquireLock(spinlock_t *lock);

#define pause()             \
{                           \
    arch_pause();           \
}                           \

#endif //SPINLOCK_H
