/*  
*   File: interrupts.c
*
*   Author: Garnek
*   
*   Description: Interrupt Handler
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "interrupts.h"
#include <cpu/interrupts/idt.h>
#include <sys/syscall.h>
#include <kstdio.h>

#include <cpu/interrupts/exceptions.h>
#include <sys/syscall.h>

//initialise interrupts
void interrupts_init(){
    idt_init();
    exceptions_init();
    syscall_init();

    idt_set_entry(32, irq0, INT_GATE);
    idt_set_entry(33, irq1, INT_GATE);
    idt_set_entry(34, irq2, INT_GATE);
    idt_set_entry(35, irq3, INT_GATE);
    idt_set_entry(36, irq4, INT_GATE);
    idt_set_entry(37, irq5, INT_GATE);
    idt_set_entry(38, irq6, INT_GATE);
    idt_set_entry(39, irq7, INT_GATE);
    idt_set_entry(40, irq8, INT_GATE);
    idt_set_entry(41, irq9, INT_GATE);
    idt_set_entry(42, irq10, INT_GATE);
    idt_set_entry(43, irq11, INT_GATE);
    idt_set_entry(44, irq12, INT_GATE);
    idt_set_entry(45, irq13, INT_GATE);
    idt_set_entry(46, irq14, INT_GATE);
    idt_set_entry(47, irq15, INT_GATE);
}