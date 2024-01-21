/*  
*   File: syscall.c
*
*   Author: Garnek
*   
*   Description: System Calls
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "syscall.h"

#include <sys/dal/dal.h>
#include <sys/fal/fal.h>
#include <process/process.h>
#include <process/sched/sched.h>

#include <cpu/interrupts/interrupts.h>
#include <cpu/interrupts/idt.h>
#include <kstdio.h>

void* syscallTable[SYSCALL_MAX];

inline void syscall_register(size_t num, void* addr){
    syscallTable[num] = addr;
}

void syscall_init(){
    syscall_register(0, sys_read);
    syscall_register(1, sys_write);
    syscall_register(2, sys_open);
    syscall_register(3, sys_close);
    syscall_register(7, sys_waitpid);
    syscall_register(57, sys_fork);
    syscall_register(59, sys_execve);
    syscall_register(60, sys_exit);
    syscall_register(79, sys_getcwd);
    syscall_register(80, sys_chdir);
    syscall_register(162, sys_sync);

    idt_set_entry(0x80, isr128, INT_USER_GATE);
}

void syscall_handler(stack_frame_t* regs){
    thread_t* callerThread = sched_get_current_thread();

    callerThread->regs = *regs;

    int (*syscall)(stack_frame_t* regs,
    uint64_t arg1, 
    uint64_t arg2, 
    uint64_t arg3, 
    uint64_t arg4, 
    uint64_t arg5, 
    uint64_t arg6) = (int(*)(stack_frame_t* regs, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6))syscallTable[regs->rax];

    asm volatile("sti"); //enable interrupts

    regs->rax = syscall(regs, regs->rdi, regs->rsi, regs->rdx, regs->r10, regs->r8, regs->r9);
}