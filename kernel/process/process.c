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

void process_create_init(){
    process_t* initProcess = kmalloc(sizeof(process_t));
    memset(initProcess, 0, sizeof(process_t));
    initProcess->pid = _process_gen_pid();
    vaspace_new(initProcess);
    
    thread_t* initThread = kmalloc(sizeof(thread_t));
    memset(initThread, 0, sizeof(thread_t));
    initThread->process = initProcess;
    initThread->status = THREAD_STATUS_READY;
    //no alignment is needed since addresses
    //returned by kmalloc() are already 16-byte aligned
    initThread->kernelStack = kmalloc(VMM_INIT_KERNEL_STACK_SIZE+1) + VMM_INIT_KERNEL_STACK_SIZE;
    vaspace_create_thread_user_stack(initThread);
    initProcess->mainThread = initThread;

    if(elf_exec_load(initProcess, "0:/bin/init.elf") != 0){
        panic("Could not load init!");
    }

    sched_add_thread(initThread);

    vaspace_switch(initProcess->pml4);
    tss_set_rsp(0, initThread->kernelStack);
    user_jump(initThread->regs.rip, initThread->regs.rsp);
}