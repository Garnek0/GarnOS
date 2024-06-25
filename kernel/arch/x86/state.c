/*  
*   File: state.c
*
*   Author: Garnek
*   
*   Description: x86 CPU state and control
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <arch/arch-internals.h>
#include <garn/arch.h>
#include <garn/kstdio.h>

void arch_dump_cpu_state(stack_frame_t* regs){
    uint64_t cr0, cr2, cr3, cr4;

    asm volatile(
        "push %%rax\n"
        "mov %%cr0, %%rax\n"
        "mov %%rax, %0\n"
        "mov %%cr2, %%rax\n"
        "mov %%rax, %1\n"
        "mov %%cr3, %%rax\n"
        "mov %%rax, %2\n"
        "mov %%cr4, %%rax\n"
        "mov %%rax, %3\n"
        "pop %%rax"
    : "=r" (cr0), "=r" (cr2), "=r" (cr3), "=r" (cr4) :: "%rax");

    kprintf("RAX=0x%x RBX=0x%x RCX=0x%x\n"
            "RDX=0x%x RSI=0x%x RDI=0x%x\n"
            "RBP=0x%x RSP=0x%x RIP=0x%x (no KOFF 0x%x)\n"
            "RFLAGS=0x%x\n"
            "CS=0x%x DS=0x%x SS=0x%x\n"
            "CR0=0x%x CR2=0x%x CR3=0x%x CR4=0x%x\n"
            "V=0x%x ERRCODE=0x%x\n", regs->rax, regs->rbx, regs->rcx, regs->rdx, regs->rsi, regs->rdi, regs->rbp, regs->rsp, regs->rip,
            (regs->rip > bl_get_kernel_virt_base() ? regs->rip-bl_get_kernel_virt_base() : regs->rip), regs->rflags, regs->cs, regs->ds, regs->ss, cr0, cr2, cr3, cr4, regs->intn, regs->errCode);
}

void arch_stop(){
    asm volatile("cli");
    while(true){
        asm volatile("hlt");
    }
}

void arch_no_op(){
    asm volatile("nop");
}