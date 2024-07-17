/*  
*   File: sched.c
*
*   Author: Garnek
*   
*   Description: Thread Scheduler
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "sched.h"
#include <process/process.h>
#include <garn/ds/list.h>
#include <garn/kstdio.h>
#include <mem/mm-internals.h>
#include <arch/arch-internals.h>

list_t* threadList;
list_node_t* currentThreadNode;
thread_t* currentThread;
process_t* currentProcess;

void sched_init(){
    threadList = list_create();

    process_init();
}

void sched_add_thread(thread_t* thread){
    thread->status = THREAD_STATUS_READY; // just in case
    list_insert(threadList, (void*)thread);

    if(!currentThread) currentThread = thread;
    if(!currentProcess) currentProcess = thread->process;
    if(!currentThreadNode) currentThreadNode = threadList->head;
}

//TODO: move to thread.c
void sched_remove_thread(thread_t* thread){
    kmfree(thread->kernelStackDeallocAddress);

    thread->status = THREAD_STATUS_DESTROYED;
}

inline process_t* sched_get_current_process(){
    return currentProcess;
}

inline thread_t* sched_get_current_thread(){
    return currentThread;
}

static void sched_get_next_thread(){
    if(threadList->head->next == NULL) return; // There is only one thread

checkready:

    if(currentThread->status == THREAD_STATUS_DESTROYED){
        list_remove(threadList, currentThread);
    } else {
        currentThread->status = THREAD_STATUS_READY;
    }

    if(currentThreadNode == NULL) currentThreadNode = threadList->head;

    currentThread = (thread_t*)currentThreadNode->value;
    currentThreadNode = currentThreadNode->next;

    currentProcess = currentThread->process;

    if(currentThread->status != THREAD_STATUS_READY) goto checkready;
}

int sys_set_tsp(stack_frame_t* regs, void* pointer){
    arch_set_tsp(pointer, regs);
    return 0;
}

void sched_preempt(stack_frame_t* regs){
    if(!threadList || threadList->head == NULL) return;

    arch_disable_interrupts();

    arch_store_context(regs);

    sched_get_next_thread();

    arch_set_kernel_stack(0, (uint64_t)currentThread->kernelStack);

    arch_restore_context(regs);

    vaspace_switch(currentThread->process->pt);
}
