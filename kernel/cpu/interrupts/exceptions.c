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

#include <drivers/serial/serial.h>

#include <sys/panic.h>

void exception_handler(stack_frame_t* regs){
    switch (regs->intn){
    case 0:
        panic("Exception: Divide Error!");
        break;
    case 1:
        klog("Exception: Debug!\n", KLOG_INFO);
        break;
    case 2:
        panic("NMI Triggered!");
        break;
    case 3:
        klog("Exception: Breakpoint!\n", KLOG_INFO);
        break;
    case 4:
        panic("Exception: Overflow!");
        break;
    case 5:
        panic("Exception: Bound Range Exceeded!");
        break;
    case 6:
        panic("Exception: Invalid Opcode!");
        break;
    case 7:
        panic("Exception: Device Not Available!");
        break;
    case 8:
        panic("Exception: Double Fault!");
        break;
    case 9:
        panic("Exception: Coprocessor Segment Overrun!");
        break;
    case 10:
        panic("Exception: Invalid TSS!");
        break;
    case 11:
        panic("Exception: Segment Not Present!");
        break;
    case 12:
        panic("Exception: Stack Segment Fault!");
        break;
    case 13:
        panic("Exception: General Protection Fault!");
        break;
    case 14:
        panic("Exception: Page Fault!");
        break;
    case 16:
        panic("Exception: x87 Floating-Point Fault!");
        break;
    case 17:
        panic("Exception: Alignment Check!");
        break;
    case 18:
        panic("Exception: Machine Check!");
        break;
    case 19:
        panic("Exception: SIMD Floating-Point Fault!");
        break;
    case 20:
        panic("Exception: Virtualisation Fault!");
        break;
    case 21:
        panic("Exception: Control Protection Fault!");
        break;
    case 28:
        panic("Exception: Hypervisor Injection!");
        break;
    case 29:
        panic("Exception: VMM Communication!");
        break;
    case 30:
        panic("Exception: Security Fault!");
        break;
    default:
        panic("Exception: (Reserved)");
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