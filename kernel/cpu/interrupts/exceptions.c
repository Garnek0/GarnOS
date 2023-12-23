/*  
*   File: exceptions.c
*
*   Author: Garnek
*   
*   Description: Exception Handler
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "exceptions.h"
#include <cpu/interrupts/idt.h>
#include <cpu/interrupts/interrupts.h>

#include <hw/serial/serial.h>

#include <sys/panic.h>
#include <kstdio.h>

void exception_handler(stack_frame_t* regs){
    switch (regs->intn){
    case 0:
        panic_with_stack_frame("Exception: Divide Error!", regs);
        break;
    case 1:
        panic_with_stack_frame("Exception: Debug!\n", regs);
        break;
    case 2:
        panic_with_stack_frame("NMI Triggered!", regs);
        break;
    case 3:
        panic_with_stack_frame("Exception: Breakpoint!\n", regs);
        break;
    case 4:
        panic_with_stack_frame("Exception: Overflow!", regs);
        break;
    case 5:
        panic_with_stack_frame("Exception: Bound Range Exceeded!", regs);
        break;
    case 6:
        panic_with_stack_frame("Exception: Invalid Opcode!", regs);
        break;
    case 7:
        panic_with_stack_frame("Exception: Device Not Available!", regs);
        break;
    case 8:
        panic_with_stack_frame("Exception: Double Fault!", regs);
        break;
    case 9:
        panic_with_stack_frame("Exception: Coprocessor Segment Overrun!", regs);
        break;
    case 10:
        panic_with_stack_frame("Exception: Invalid TSS!", regs);
        break;
    case 11:
        panic_with_stack_frame("Exception: Segment Not Present!", regs);
        break;
    case 12:
        panic_with_stack_frame("Exception: Stack Segment Fault!", regs);
        break;
    case 13:
        panic_with_stack_frame("Exception: General Protection Fault!", regs);
        break;
    case 14:
        panic_with_stack_frame("Exception: Page Fault!", regs);
        break;
    case 16:
        panic_with_stack_frame("Exception: x87 Floating-Point Fault!", regs);
        break;
    case 17:
        panic_with_stack_frame("Exception: Alignment Check!", regs);
        break;
    case 18:
        panic_with_stack_frame("Exception: Machine Check!", regs);
        break;
    case 19:
        panic_with_stack_frame("Exception: SIMD Floating-Point Fault!", regs);
        break;
    case 20:
        panic_with_stack_frame("Exception: Virtualisation Fault!", regs);
        break;
    case 21:
        panic_with_stack_frame("Exception: Control Protection Fault!", regs);
        break;
    case 28:
        panic_with_stack_frame("Exception: Hypervisor Injection!", regs);
        break;
    case 29:
        panic_with_stack_frame("Exception: VMM Communication!", regs);
        break;
    case 30:
        panic_with_stack_frame("Exception: Security Fault!", regs);
        break;
    default:
        panic_with_stack_frame("Exception: (Reserved)", regs);
        break;
    }
}

//initialise exception handler
void exceptions_init(){
    //set up the isrs
    idt_set_entry(0, isr0, INT_GATE);
    idt_set_entry(1, isr1, INT_GATE);
    idt_set_entry(2, isr2, INT_GATE);
    idt_set_entry(3, isr3, INT_GATE);
    idt_set_entry(4, isr4, INT_GATE);
    idt_set_entry(5, isr5, INT_GATE);
    idt_set_entry(6, isr6, INT_GATE);
    idt_set_entry(7, isr7, INT_GATE);
    idt_set_entry(8, isr8, INT_GATE);
    idt_set_entry(9, isr9, INT_GATE);
    idt_set_entry(10, isr10, INT_GATE);
    idt_set_entry(11, isr11, INT_GATE);
    idt_set_entry(12, isr12, INT_GATE);
    idt_set_entry(13, isr13, INT_GATE);
    idt_set_entry(14, isr14, INT_GATE);
    idt_set_entry(15, isr15, INT_GATE);
    idt_set_entry(16, isr16, INT_GATE);
    idt_set_entry(17, isr17, INT_GATE);
    idt_set_entry(18, isr18, INT_GATE);
    idt_set_entry(19, isr19, INT_GATE);
    idt_set_entry(20, isr20, INT_GATE);
    idt_set_entry(21, isr21, INT_GATE);
    idt_set_entry(22, isr22, INT_GATE);
    idt_set_entry(23, isr23, INT_GATE);
    idt_set_entry(24, isr24, INT_GATE);
    idt_set_entry(25, isr25, INT_GATE);
    idt_set_entry(26, isr26, INT_GATE);
    idt_set_entry(27, isr27, INT_GATE);
    idt_set_entry(28, isr28, INT_GATE);
    idt_set_entry(29, isr29, INT_GATE);
    idt_set_entry(30, isr30, INT_GATE);
    idt_set_entry(31, isr31, INT_GATE);
}