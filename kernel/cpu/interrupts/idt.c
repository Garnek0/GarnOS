#include "idt.h"

#include <kstdio.h>
#include <sys/rblogs.h>

idtr_t idtr;
idt_entry_t idt[256];

void idt_set_entry(uint8_t entry, void* isr, uint8_t PDPLGateType){
    idt[entry].offset0 = (uint64_t)isr & 0xFFFF;
    idt[entry].offset1 = ((uint64_t)isr >> 16) & 0xFFFF;
    idt[entry].offset2 = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    idt[entry].ist = 0;
    idt[entry].PDPLGateType = PDPLGateType;
    idt[entry].reserved = 0;
    idt[entry].seg = 0x08;
}

void idt_init(){
    idtr.offset = idt;
    idtr.size = sizeof(idt_entry_t)*256 - 1;

    asm volatile ("lidt %0" : : "m"(idtr));
    asm volatile ("sti");

    klog("IDT Loaded Successfully.\n", KLOG_OK);
    rb_log("IDT", KLOG_OK);
}