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
#include <kstdio.h>

//these lists hold the function pointers to all irq handlers
list_t* irqHandlerLists[224];

void irq_add_handler(uint8_t irq, void* handler){
    if(!irqHandlerLists[irq]) irqHandlerLists[irq] = list_create();
    list_insert(irqHandlerLists[irq], handler);
}

void irq_remove_handler(uint8_t irq, void* handler){
    if(!irqHandlerLists[irq]) return;
    list_remove(irqHandlerLists[irq], handler);
}

void irq_handler(stack_frame_t* regs){
    //for irqs, errCode stores the irq number
    if(regs->errCode == 224) return; //This is an APIC Spurious Interrupt. Return without signaling EOI.
    if(!irqHandlerLists[regs->errCode]) goto done;
    foreach(i, irqHandlerLists[regs->errCode]){
        void (*irq)(stack_frame_t* regs) = (void(*)(stack_frame_t* regs))i->value;
        irq(regs);
    }
    // (apic_eoi() contains a PIC EOI as well,
    // so things wont break if the system is
    // using the PIC as a fallback ;))
done:
    apic_eoi();
    return;
}
