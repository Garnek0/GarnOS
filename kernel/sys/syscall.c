/*  
*   File: syscall.c
*
*   Author: Garnek
*   
*   Description: System Calls
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "syscall.h"

#include <sys/uname-internals.h>
#include <sys/dal/dal-internals.h>
#include <garn/dal/dal.h>
#include <garn/fal/file.h>
#include <garn/fal/filesys.h>
#include <mem/mm-internals.h>
#include <process/process.h>
#include <process/sched/sched.h>
#include <sys/fal/fal-internals.h>

#include <garn/irq.h>
#include <cpu/interrupts/interrupt-internals.h>
#include <cpu/interrupts/idt.h>
#include <garn/kstdio.h>

#include <garn/syscall.h>

void* syscallTable[SYSCALL_MAX];
void* gsysSyscallTable[SYSCALL_MAX];

void syscall_register(size_t num, void* addr){
    syscallTable[num] = addr;
}

void gsys_syscall_register(size_t num, void* addr){
    gsysSyscallTable[num] = addr;
}

void syscall_init(){

    // POSIX & Linux Syscalls

    syscall_register(0, sys_read);
    syscall_register(1, sys_write);
    syscall_register(2, sys_open);
    syscall_register(3, sys_close);
    syscall_register(7, sys_waitpid);
    syscall_register(9, sys_mmap);
    syscall_register(11, sys_munmap);
    syscall_register(57, sys_fork);
    syscall_register(59, sys_execve);
    syscall_register(60, sys_exit);
    syscall_register(63, sys_uname);
    syscall_register(78, sys_getdents);
    syscall_register(79, sys_getcwd);
    syscall_register(80, sys_chdir);
    syscall_register(162, sys_sync);

    // > 0x80000000

    gsys_syscall_register(0x80000000 - 0x80000000, sys_set_fs_base);

    idt_set_entry(0x80, isr128, INT_USER_GATE);
}

void syscall_handler(stack_frame_t* regs){
    thread_t* callerThread = sched_get_current_thread();

    callerThread->regs = *regs;

    if(regs->rax < 0x80000000){
        int (*syscall)(stack_frame_t* regs,
        uint64_t arg1, 
        uint64_t arg2, 
        uint64_t arg3, 
        uint64_t arg4, 
        uint64_t arg5, 
        uint64_t arg6) = (int(*)(stack_frame_t* regs, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6))syscallTable[regs->rax];

        asm volatile("sti"); //enable interrupts

        if(syscall) regs->rax = syscall(regs, regs->rdi, regs->rsi, regs->rdx, regs->r10, regs->r8, regs->r9);
    } else {
        int (*syscall)(stack_frame_t* regs,
        uint64_t arg1, 
        uint64_t arg2, 
        uint64_t arg3, 
        uint64_t arg4, 
        uint64_t arg5, 
        uint64_t arg6) = (int(*)(stack_frame_t* regs, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6))gsysSyscallTable[regs->rax - 0x80000000];

        asm volatile("sti"); //enable interrupts

        if(syscall) regs->rax = syscall(regs, regs->rdi, regs->rsi, regs->rdx, regs->r10, regs->r8, regs->r9);
    }
}