/*  
*   File: interrupts.h
*
*   Author: Garnek
*   
*   Description: Extern declarations for every ASM stub and
*                 definition of the stack frame structure
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <types.h>

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr128();
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void irq223();
extern void irq224();

typedef struct {
   uint64_t ds;
   uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
   uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
   uint64_t intn, errCode;
   uint64_t rip, cs, rflags, rsp, ss;
} stack_frame_t;

void interrupts_init();

#endif //INTERRUPTS_H