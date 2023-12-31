/*  
*   File: syscall.c
*
*   Author: Garnek
*   
*   Description: System Calls
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "syscall.h"
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
    syscall_register(0, sys_test);

    idt_set_entry(0x80, isr128, INT_USER_GATE);
}

void syscall_handler(stack_frame_t* regs){
    int (*syscall)(uint64_t arg1, 
    uint64_t arg2, 
    uint64_t arg3, 
    uint64_t arg4, 
    uint64_t arg5, 
    uint64_t arg6) = (int(*)(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6))syscallTable[regs->rax];

    regs->rax = syscall(regs->rdi, regs->rsi, regs->rdx, regs->r10, regs->r8, regs->r9);
}