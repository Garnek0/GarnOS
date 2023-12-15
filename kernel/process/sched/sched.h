/*  
*   File: sched.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef SCHED_H
#define SCHED_H

#include <types.h>
#include <cpu/interrupts/interrupts.h>
#include <process/thread/thread.h>

void sched_preempt(stack_frame_t* regs);
void sched_add_thread(thread_t* thread);
void sched_init();

#endif //SCHED_H