#include <garn/mm.h>
#include <garn/kstdio.h>
#include <garn/ds/list.h>
#include <garn/irq.h>
#include <garn/arch.h>

typedef struct {
    void* handler;
} irq_t;

list_t* irqHandlerLists[IRQ_MAX];
uint32_t irqFlags[IRQ_MAX];

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
    //get irq number
    int irqNumber = arch_get_irq_number(regs);

#ifdef __x86_64__
    if(irqNumber == 224) return; //This is an APIC Spurious Interrupt. Return without signaling EOI.
#endif

    if(!irqHandlerLists[irqNumber]) goto done;
    foreach(i, irqHandlerLists[irqNumber]){
        irq_t* irqStruct = (irq_t*)i->value;
        void (*irq)(stack_frame_t* regs) = (void(*)(stack_frame_t* regs))irqStruct->handler;
        irq(regs);
    }

done:
    arch_end_interrupt();
    return;
}
