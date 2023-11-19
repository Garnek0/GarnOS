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

void irq_set_handler(uint8_t irq, void* handler){
    switch(irq){
        case 0:
            irqHandler.irq0 = handler;
            break;
        case 1:
            irqHandler.irq1 = handler;
            break;
        case 2:
            irqHandler.irq2 = handler;
            break;
        case 3:
            irqHandler.irq3 = handler;
            break;
        case 4:
            irqHandler.irq4 = handler;
            break;
        case 5:
            irqHandler.irq5 = handler;
            break;
        case 6:
            irqHandler.irq6 = handler;
            break;
        case 7:
            irqHandler.irq7 = handler;
            break;
        case 8:
            irqHandler.irq8 = handler;
            break;
        case 9:
            irqHandler.irq9 = handler;
            break;
        case 10:
            irqHandler.irq10 = handler;
            break;
        case 11:
            irqHandler.irq10 = handler;
            break;
        case 12:
            irqHandler.irq10 = handler;
            break;
        case 13:
            irqHandler.irq10 = handler;
            break;
        case 14:
            irqHandler.irq10 = handler;
            break;
        case 15:
            irqHandler.irq10 = handler;
            break;
        default:
            break;
    }
}

void irq_handler(stack_frame_t* regs){ 
    switch(regs->errCode){ //for irqs, errCode stores the irq number
        case 0:
            if(irqHandler.irq0) irqHandler.irq0(regs);
            break;
        case 1:
            if(irqHandler.irq1) irqHandler.irq1(regs);
            break;
        case 2:
            if(irqHandler.irq2) irqHandler.irq2(regs);
            break;
        case 3:
            if(irqHandler.irq3) irqHandler.irq3(regs);
            break;
        case 4:
            if(irqHandler.irq4) irqHandler.irq4(regs);
            break;
        case 5:
            if(irqHandler.irq5) irqHandler.irq5(regs);
            break;
        case 6:
            if(irqHandler.irq6) irqHandler.irq6(regs);
            break;
        case 7:
            if(irqHandler.irq7) irqHandler.irq7(regs);
            break;
        case 8:
            if(irqHandler.irq8) irqHandler.irq8(regs);
            break;
        case 9:
            if(irqHandler.irq9) irqHandler.irq9(regs);
            break;
        case 10:
            if(irqHandler.irq10) irqHandler.irq10(regs);
            break;
        case 11:
            if(irqHandler.irq11) irqHandler.irq11(regs);
            break;
        case 12:
            if(irqHandler.irq12) irqHandler.irq12(regs);
            break;
        case 13:
            if(irqHandler.irq13) irqHandler.irq13(regs);
            break;
        case 14:
            if(irqHandler.irq14) irqHandler.irq14(regs);
            break;
        case 15:
            if(irqHandler.irq15) irqHandler.irq15(regs);
            break;
        default:
            break;
    }
    apic_eoi();
    outb(0x20, 0x20); //PIC EOI
    outb(0xA0, 0x20);
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