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

int sys_test(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6){
    kprintf("\nRecieved Syscall!\n"
            "arg1 (rdi) = 0x%x\n"
            "arg2 (rsi) = 0x%x\n"
            "arg3 (rdx) = 0x%x\n"
            "arg4 (r10) = 0x%x\n"
            "arg5 (r8) = 0x%x\n"
            "arg6 (r9) = 0x%x", arg1, arg2, arg3, arg4, arg5, arg6);
    return 0;
}

void syscall_init(){
    syscall_register(0, sys_read);
    syscall_register(1, sys_write);
    syscall_register(2, sys_open);
    syscall_register(3, sys_close);
    syscall_register(4, sys_test);
    syscall_register(57, sys_fork);
    syscall_register(60, sys_exit);
    syscall_register(162, sys_sync);

    idt_set_entry(0x80, isr128, INT_USER_GATE);
}

void syscall_handler(stack_frame_t* regs){
    sched_store_context_to_thread(sched_get_current_thread(), regs);

    int (*syscall)(uint64_t arg1, 
    uint64_t arg2, 
    uint64_t arg3, 
    uint64_t arg4, 
    uint64_t arg5, 
    uint64_t arg6) = (int(*)(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6))syscallTable[regs->rax];

    regs->rax = syscall(regs->rdi, regs->rsi, regs->rdx, regs->r10, regs->r8, regs->r9);
}