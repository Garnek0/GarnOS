/*  
*   File: process.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef PROCESS_H
#define PROCESS_H

#define PROCESS_INIT_FD 128
#define PROCESS_MAX_FD 2048

#include <types.h>
#include <process/thread/thread.h>
#include <sys/fal/fal.h>
#include <mem/vmm/vmm.h>

typedef struct _process {
    char* name;
    int pid;

    struct _page_table* pml4;

    fd_t* fdTable;
    size_t fdMax;

    char* cwd;

    struct _thread* mainThread;
    struct _process* parent;

    struct _process* next;
} process_t;

void process_init();
void process_terminate(process_t* process);

int sys_fork();
void sys_exit(int status);

#endif //PROCESS_H