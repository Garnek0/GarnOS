/*  
*   File: process.c
*
*   Author: Garnek
*   
*   Description: Process Implementation
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "process.h"
#include <mem/mm/kheap.h>
#include <mem/mm/vmm.h>
#include <mem/memutil/memutil.h>
#include <process/thread/thread.h>
#include <process/sched/sched.h>

//TODO: add process list;

static int _process_gen_pid(){
    static int pid = 0;
    return pid++;
}

void process_create_kernel(){
    process_t* kernelProcess = kmalloc(sizeof(process_t));
    memset(kernelProcess, 0, sizeof(kernelProcess));
    kernelProcess->pid = _process_gen_pid();
    kernelProcess->pml4 = vmm_get_current_address_space();

    thread_t* kernelThread = kmalloc(sizeof(thread_t));
    memset(kernelThread, 0, sizeof(kernelThread));
    kernelThread->process = kernelProcess;
    kernelThread->status = THREAD_STATUS_READY;

    sched_add_thread(kernelThread);
}