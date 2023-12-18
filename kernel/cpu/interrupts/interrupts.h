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

typedef struct {
   uint64_t ds;
   uint64_t rdi, rsi, rbp, prev_rsp, rbx, rdx, rcx, rax;
   uint64_t intn, errCode;
   uint64_t rip, cs, rflags, rsp, ss;
} stack_frame_t;

typedef struct {
   void (*irq0)(stack_frame_t* frame);
   void (*irq1)(stack_frame_t* frame);
   void (*irq2)(stack_frame_t* frame);
   void (*irq3)(stack_frame_t* frame);
   void (*irq4)(stack_frame_t* frame);
   void (*irq5)(stack_frame_t* frame);
   void (*irq6)(stack_frame_t* frame);
   void (*irq7)(stack_frame_t* frame);
   void (*irq8)(stack_frame_t* frame);
   void (*irq9)(stack_frame_t* frame);
   void (*irq10)(stack_frame_t* frame);
   void (*irq11)(stack_frame_t* frame);
   void (*irq12)(stack_frame_t* frame);
   void (*irq13)(stack_frame_t* frame);
   void (*irq14)(stack_frame_t* frame);
   void (*irq15)(stack_frame_t* frame);
} irq_handler_t;
extern irq_handler_t irqHandler;

void interrupts_init();
void irq_set_handler(uint8_t irq, void* handler);

#endif //INTERRUPTS_H