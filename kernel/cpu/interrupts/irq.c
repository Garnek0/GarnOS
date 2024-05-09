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
uint32_t irqFlags[224];

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
