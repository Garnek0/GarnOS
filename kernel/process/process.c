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
#include <sys/term/tty.h>
#include <process/sched/sched.h>
#include <sys/fal/fal.h>
#include <sys/input.h>
#include <cpu/gdt/gdt.h>
#include <cpu/user.h>
#include <exec/elf.h>
#include <kstdio.h>
#include <kerrno.h>
#include <kernel.h>

process_t* processList;
process_t* processListLast;

static int _process_gen_pid(){
    static int pid = 1;
    return pid++;
}

void process_init(){
    process_create_init();
}

void process_terminate(process_t* process){
    process_t* prev = NULL;
    for(process_t* proc = processList; proc != NULL; proc=proc->next){
        if(proc == process){
            kmfree(proc->name); //name and other strings should be strduped
            kmfree(proc->cwd);
            for(int i = 0; i < proc->fdMax; i++){
                if(proc->fdTable[i].file) proc->fdTable[i].file->refCount--;
            }
            if(prev) prev->next = proc->next;
            vaspace_destroy(proc);

            for(process_t* childProc = processList; childProc != NULL; childProc=childProc->next){
                if(childProc->parent == proc) childProc->parent = proc->parent;
            }

            sched_remove_thread(proc->mainThread);

            kmfree(proc);
            return;
        }
        prev = proc;
    }
}

void process_create_init(){
    process_t* initProcess = kmalloc(sizeof(process_t));
    memset(initProcess, 0, sizeof(process_t));

    //Set PID and parent
    initProcess->pid = _process_gen_pid();
    initProcess->parent = NULL;

    //Create VA Space and allocate the main thread
    vaspace_new(initProcess);

    thread_t* thread = kmalloc(sizeof(thread_t));
    thread->process = initProcess;
    thread->status = THREAD_STATUS_READY;

    initProcess->mainThread = thread;

    //No alignment is needed since addresses
    //returned by kmalloc() are already 16-byte aligned
    thread->kernelStack = kmalloc(VMM_INIT_KERNEL_STACK_SIZE+1) + VMM_INIT_KERNEL_STACK_SIZE;

    vaspace_create_thread_user_stack(thread);

    //Set name
    initProcess->name = strdup("init");

    //Create fd table
    initProcess->fdMax = PROCESS_INIT_FD-1;
    initProcess->fdTable = file_alloc_fd_table(PROCESS_INIT_FD);
    
    //Set rootDir and cwd
    initProcess->cwd = strdup("0:/");

    //Set tty FDs
    initProcess->fdTable[0].file = kbd;
    initProcess->fdTable[1].file = tty;
    initProcess->fdTable[2].file = tty;
    initProcess->fdTable[0].flags = tty->flags;
    initProcess->fdTable[1].flags = tty->flags;
    initProcess->fdTable[2].flags = tty->flags;
    tty->refCount+=2;
    kbd->refCount++;

    if(elf_exec_load(initProcess, "0:/bin/init.elf") != 0){
        panic("Could not load init!");
    }
    sched_add_thread(initProcess->mainThread);

    processList = processListLast = initProcess;
    initProcess->next = NULL;

    vaspace_switch(initProcess->pml4);
    tss_set_rsp(0, initProcess->mainThread->kernelStack);
    user_jump(initProcess->mainThread->regs.rip, initProcess->mainThread->regs.rsp);
}

int sys_fork(){
    process_t* currentProcess = sched_get_current_process();
    process_t* newProcess = kmalloc(sizeof(process_t));
    memset(newProcess, 0, sizeof(process_t));

    newProcess->pid = _process_gen_pid();
    newProcess->parent = currentProcess;
    newProcess->name = strdup("procfork");

    newProcess->fdMax = currentProcess->fdMax;
    newProcess->fdTable = file_alloc_fd_table(newProcess->fdMax+1);

    newProcess->cwd = strdup(currentProcess->cwd);

    for(int i = 0; i < currentProcess->fdMax; i++){
        newProcess->fdTable[i] = currentProcess->fdTable[i];
        if(newProcess->fdTable[i].file) newProcess->fdTable[i].file->refCount++;
    }

    newProcess->mainThread = kmalloc(sizeof(thread_t));
    memset(newProcess->mainThread, 0, sizeof(thread_t));
    memcpy((void*)&newProcess->mainThread->regs, (void*)&currentProcess->mainThread->regs, sizeof(stack_frame_t));

    newProcess->mainThread->kernelStack = kmalloc(VMM_INIT_KERNEL_STACK_SIZE+1) + VMM_INIT_KERNEL_STACK_SIZE;
    newProcess->mainThread->status = THREAD_STATUS_READY;
    newProcess->mainThread->process = newProcess;

    newProcess->pml4 = vaspace_clone(currentProcess->pml4);

    sched_add_thread(newProcess->mainThread);

    newProcess->mainThread->regs.rax = 0;
    return newProcess->pid;
}

__attribute__((noreturn))
void sys_exit(int status){
    process_t* currentProcess = sched_get_current_process();

    process_terminate(currentProcess);

    asm volatile("sti"); //enable interrupts

    while(1) asm volatile("nop"); //wait for reschedule
}