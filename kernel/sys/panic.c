/*  
*   File: panic.c
*
*   Author: Garnek
*   
*   Description: Kernel Panic Routine
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "panic.h"
#include <kstdio.h>

void panic(char* str, ...){

    va_list args;
    va_start(args, str);

    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp, rip, rflags;
    uint64_t cs, ds, es, fs, gs, ss;
    uint64_t cr0, cr2, cr3, cr4;
    asm volatile("movq %%rax,%0" : "=r"(rax));
    asm volatile("movq %%rbx,%0" : "=r"(rbx));
    asm volatile("movq %%rcx,%0" : "=r"(rcx));
    asm volatile("movq %%rdx,%0" : "=r"(rdx));
    asm volatile("movq %%rsi,%0" : "=r"(rsi));
    asm volatile("movq %%rdi,%0" : "=r"(rdi));
    asm volatile("movq %%rbp,%0" : "=r"(rbp));
    asm volatile("movq %%rsp,%0" : "=r"(rsp));
    asm volatile("lea (%%rip),%0" : "=r"(rip));
    asm volatile("push %%rax; pushfq; pop %%rax; mov %%rax,%0; pop %%rax" : "=r"(rflags));
    asm volatile("movq %%cs,%0" : "=r"(cs));
    asm volatile("movq %%ds,%0" : "=r"(ds));
    asm volatile("movq %%es,%0" : "=r"(es));
    asm volatile("movq %%fs,%0" : "=r"(fs));
    asm volatile("movq %%gs,%0" : "=r"(gs));
    asm volatile("movq %%ss,%0" : "=r"(ss));

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
    : "=r" (cr0), "=r" (cr2), "=r" (cr3), "=r" (cr4));

    klog("Kernel Panic!\n\n", KLOG_FATAL);
    kvprintf(str, args);
    kprintf("\n\n");
    kprintf("RAX=0x%x RBX=0x%x RCX=0x%x\n"
            "RDX=0x%x RSI=0x%x RDI=0x%x\n"
            "RBP=0x%x RSP=0x%x RIP=0x%x\n"
            "RFLAGS=0x%x\n"
            "CS=0x%x DS=0x%x ES=0x%x FS=0x%x GS=0x%x SS=0x%x\n"
            "CR0=0x%x CR2=0x%x CR3=0x%x CR4=0x%x\n", rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp, rip, rflags, cs, ds, es, fs, gs, ss, cr0, cr2, cr3, cr4);

    va_end(args);
    
    asm("cli");
    while(true){
        asm("hlt");
    }
}