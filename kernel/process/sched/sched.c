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
#include <ds/list.h>
#include <cpu/gdt/gdt.h>
#include <cpu/msr.h>
#include <cpu/user.h>
#include <kstdio.h>

list_t* threadList;
list_node_t* currentThreadNode;
thread_t* currentThread;
process_t* currentProcess;

void sched_init(){
    threadList = list_create("schedThreadList");

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

static void _sched_get_next_thread(){
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

static void _sched_store_context(stack_frame_t* regs){
    currentThread->regs = *regs;
}

static void _sched_switch_context(stack_frame_t* regs){
    *regs = currentThread->regs;

    vaspace_switch(currentThread->process->pml4);

    //wrmsr(0xC0000100, currentThread->fsbase);
}

int sys_set_fs_base(stack_frame_t* regs, void* pointer){
    currentThread->fsbase = pointer;
    wrmsr(0xC0000100, (uint64_t)pointer);
    return 0;
}

void sched_preempt(stack_frame_t* regs){
    if(!threadList || threadList->head == NULL) return;

    asm volatile("cli");

    _sched_store_context(regs);

    _sched_get_next_thread();

    tss_set_rsp(0, currentThread->kernelStack);

    _sched_switch_context(regs);
}