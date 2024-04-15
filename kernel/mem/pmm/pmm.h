/*  
*   File: pmm.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef PMM_H
#define PMM_H

#include <types.h>
#include <cpu/multiproc/spinlock.h>

typedef struct {
    size_t freePages;
    size_t usedPages;
    size_t usablePages;
    spinlock_t lock;
} pmm_info_t;
extern pmm_info_t pmm_info;

void pmm_init();
void* pmm_allocate(int npages);
void pmm_free(void* base, int npages);

#endif //PMM_H