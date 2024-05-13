/*  
*   File: irq.c
*
*   Author: Garnek
*   
*   Description: Interrupt Requests
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "irq.h"
#include <hw/ports.h>
#include <cpu/apic/apic.h>
#include <mem/kheap/kheap.h>
#include <cpu/interrupts/idt.h>
#include <kstdio.h>

list_t* irqHandlerLists[224];
uint32_t irqFlags[224];

void irq_init(){
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
    idt_set_entry(48, irq16, INT_GATE);
    idt_set_entry(49, irq17, INT_GATE);
    idt_set_entry(50, irq18, INT_GATE);
    idt_set_entry(51, irq19, INT_GATE);
    idt_set_entry(52, irq20, INT_GATE);
    idt_set_entry(53, irq21, INT_GATE);
    idt_set_entry(54, irq22, INT_GATE);
    idt_set_entry(55, irq23, INT_GATE);

    idt_set_entry(254, irq223, INT_GATE); //APIC Error
    idt_set_entry(255, irq224, INT_GATE); //APIC Spurious Interrupt Vector
}

int irq_add_handler(uint8_t irq, void* handler, uint32_t flags){
    irq_t* irqStruct = kmalloc(sizeof(irq_t));
    irqStruct->handler = handler;
    if(!irqHandlerLists[irq]) {
        irqHandlerLists[irq] = list_create();
    }
    if(!(irqFlags[irq] & IRQ_SHARED) && irqHandlerLists[irq]->nodeCount != 0){
        klog("Attempt to register a second IRQ handler for non-shared IRQ %u ignored!\n", KLOG_WARNING, "IRQ", irq);
        kmfree(irqStruct);
        return -1;
    }
    if((irqFlags[irq] & IRQ_SHARED) && !(flags & IRQ_SHARED)){
        klog("Attempt to make shared IRQ %u non-shared while active handlers are registered ignored!\n", KLOG_WARNING, "IRQ", irq);
        kmfree(irqStruct);
        return -1;
    }
    list_insert(irqHandlerLists[irq], (void*)irqStruct);
    irqFlags[irq] = flags;
}

void irq_remove_handler(uint8_t irq, void* handler){
    if(!irqHandlerLists[irq]) return;
    foreach(i, irqHandlerLists[irq]){
        irq_t* irqStruct = (irq_t*)i->value;
        if(irqStruct->handler == handler){
            list_remove(irqHandlerLists[irq], handler);
            if(irqHandlerLists[irq]->nodeCount == 0) irqFlags[irq] = 0;
            break;
        }
    }
}

void irq_handler(stack_frame_t* regs){
    //for irqs, errCode stores the irq number
    if(regs->errCode == 224) return; //This is an APIC Spurious Interrupt. Return without signaling EOI.
    if(!irqHandlerLists[regs->errCode]) goto done;
    foreach(i, irqHandlerLists[regs->errCode]){
        irq_t* irqStruct = (irq_t*)i->value;
        void (*irq)(stack_frame_t* regs) = (void(*)(stack_frame_t* regs))irqStruct->handler;
        irq(regs);
    }
    // (apic_eoi() contains a PIC EOI as well,
    // so things wont break if the system is
    // using the PIC as a fallback ;))
done:
    apic_eoi();
    return;
}
