/*  
*   File: interrupts.c
*
*   Author: Garnek
*   
*   Description: Interrupt Handler
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "interrupts.h"
#include <hw/ports.h>
#include <cpu/interrupts/idt.h>
#include <kstdio.h>

#include <cpu/apic/apic.h>
#include <cpu/interrupts/exceptions.h>

//irq struct, this holds function pointers to all irq handlers
irq_handler_t irqHandler;

void irq_handler(stack_frame_t* regs){   
    switch(regs->errCode){ //for irqs, errCode stores the irq number
        case 0:
            if(irqHandler.pit_handler) irqHandler.pit_handler(regs);
            break;
        case 1:
            if(irqHandler.keyboard_handler) irqHandler.keyboard_handler(regs);
            break;
        case 8:
            if(irqHandler.rtc_handler) irqHandler.rtc_handler(regs);
            break;
        default:
            break;
    }
    apic_eoi();
}

//initialise interrupts
void interrupts_init(){
    idt_init();
    exceptions_init();

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