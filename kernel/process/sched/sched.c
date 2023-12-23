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
#include <cpu/user.h>
#include <kstdio.h>

list_t* threadList;
list_node_t* currentThreadNode;
thread_t* currentThread;

void sched_init(){
    threadList = list_create("schedThreadList");

    process_init();
}

void sched_add_thread(thread_t* thread){
    thread->status = THREAD_STATUS_READY; // just in case
    list_insert(threadList, (void*)thread);

    if(!currentThread) currentThread = thread;
    if(!currentThreadNode) currentThreadNode = threadList->head;
}

static void _sched_get_next_thread(){
    if(threadList->head->next == NULL) return; // There is only one thread

    currentThread->status = THREAD_STATUS_READY;

checkready:
    if(currentThreadNode == NULL) currentThreadNode = threadList->head;

    currentThread = (thread_t*)currentThreadNode->value;
    currentThreadNode = currentThreadNode->next;

    if(currentThread->status != THREAD_STATUS_READY) goto checkready;
}

static void _sched_store_context(stack_frame_t* regs){
    currentThread->regs = *regs;
}

static void _sched_switch_context(stack_frame_t* regs){
    *regs = currentThread->regs;

    vaspace_switch(currentThread->process->pml4);
}

void sched_preempt(stack_frame_t* regs){
    if(!threadList || threadList->head == NULL) return;

    asm volatile("cli");

    _sched_store_context(regs);

    _sched_get_next_thread();

    tss_set_rsp(0, currentThread->kernelStack);

    _sched_switch_context(regs);
}