/*  
*   File: process.c
*
*   Author: Garnek
*   
*   Description: Process Implementation
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "process.h"
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <process/thread/thread.h>
#include <process/sched/sched.h>
#include <sys/fal/fal.h>
#include <cpu/gdt/gdt.h>
#include <cpu/user.h>
#include <exec/elf.h>
#include <kstdio.h>
#include <kernel.h>

//TODO: add process list;

static int _process_gen_pid(){
    static int pid = 0;
    return pid++;
}

void process_init(){
    process_create_init();
}

static process_t* _process_create(process_t* parent){
    process_t* proc = kmalloc(sizeof(process_t));
    memset(proc, 0, sizeof(process_t));

    //Set PID and parent
    proc->pid = _process_gen_pid();
    proc->parent = parent;

    //Create VA Space and allocate the main thread
    vaspace_new(proc);

    thread_t* thread = kmalloc(sizeof(thread_t));
    thread->process = proc;
    thread->status = THREAD_STATUS_READY;

    //no alignment is needed since addresses
    //returned by kmalloc() are already 16-byte aligned
    thread->kernelStack = kmalloc(VMM_INIT_KERNEL_STACK_SIZE+1) + VMM_INIT_KERNEL_STACK_SIZE;

    vaspace_create_thread_user_stack(thread);

    proc->mainThread = thread;

    //create fd table
    proc->fdMax = PROCESS_MAX_FD-1;
    proc->fdTable = file_alloc_fd_table(PROCESS_MAX_FD);

    return proc;
}

void process_create_init(){
    process_t* initProcess = _process_create(NULL);

    if(elf_exec_load(initProcess, "0:/bin/init.elf") != 0){
        panic("Could not load init!");
    }

    sched_add_thread(initProcess->mainThread);

    vaspace_switch(initProcess->pml4);
    tss_set_rsp(0, initProcess->mainThread->kernelStack);
    user_jump(initProcess->mainThread->regs.rip, initProcess->mainThread->regs.rsp);
}