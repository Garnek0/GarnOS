/*  
*   File: process.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef PROCESS_H
#define PROCESS_H

#include <types.h>

typedef struct _process {
    int pid;

    void* pml4;
} process_t;

void process_create_kernel();

#endif //PROCESS_H