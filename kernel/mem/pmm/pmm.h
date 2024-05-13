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

void pmm_init();
void* pmm_allocate(int npages);
void* pmm_allocate32(int npages);
void pmm_free(void* base, int npages);

size_t pmm_get_usable_pages_count();
size_t pmm_get_used_pages_count();
size_t pmm_get_free_pages_count();

#endif //PMM_H