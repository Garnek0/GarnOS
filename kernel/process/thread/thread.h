/*  
*   File: thread.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef THREAD_H
#define THREAD_H

#define THREAD_STATUS_READY 0
#define THREAD_STATUS_RUNNING 1
#define THREAD_STATUS_BLOCKED 2
#define THREAD_STATUS_ZOMBIE 3

#include <types.h>
#include <cpu/interrupts/interrupts.h>
#include <process/process.h>

typedef struct _thread {
    int status;

    stack_frame_t regs;

    process_t* process;
} thread_t;

#endif //THREAD_H