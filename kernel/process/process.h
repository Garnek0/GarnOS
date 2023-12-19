/*  
*   File: process.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef PROCESS_H
#define PROCESS_H

#include <types.h>
#include <mem/mm/vmm/vmm.h>

typedef struct _process {
    int pid;

    struct _page_table* pml4;

    struct _process* parent;
} process_t;

void process_init();

#endif //PROCESS_H