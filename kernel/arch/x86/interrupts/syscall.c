/*  
*   File: syscall.c
*
*   Author: Garnek
*   
*   Description: x96-specific syscall stuff
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <arch/x86/interrupts/idt.h>
#include <arch/x86/interrupts/interrupt-internals.h>

#include <process/process.h>
#include <sys/syscall_internals.h>

#include <garn/kstdio.h>

void arch_syscall_init(){
    idt_set_entry(0x80, isr128, INT_USER_GATE);
}

int arch_syscall_number(stack_frame_t* regs){
    return regs->rax;
}

void arch_syscall_return(stack_frame_t* regs, long value){
    regs->rax = value;
}

long arch_syscall_arg0(stack_frame_t* regs){
    return regs->rdi;
}

long arch_syscall_arg1(stack_frame_t* regs){
    return regs->rsi;
}

long arch_syscall_arg2(stack_frame_t* regs){
    return regs->rdx;
}

long arch_syscall_arg3(stack_frame_t* regs){
    return regs->r10;
}

long arch_syscall_arg4(stack_frame_t* regs){
    return regs->r8;
}

long arch_syscall_arg5(stack_frame_t* regs){
    return regs->r9;
}