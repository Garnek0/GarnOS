/*  
*   File: idt.c
*
*   Author: Garnek
*   
*   Description: Interrupt Descriptor Table
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "idt.h"

idtr_t idtr;
idt_entry_t idt[256];

//set an IDT entry
void idt_set_entry(uint8_t entry, void* isr, uint8_t PDPLGateType){
    idt[entry].offset0 = (uint64_t)isr & 0xFFFF;
    idt[entry].offset1 = ((uint64_t)isr >> 16) & 0xFFFF;
    idt[entry].offset2 = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    idt[entry].ist = 0;
    idt[entry].PDPLGateType = PDPLGateType;
    idt[entry].reserved = 0;
    idt[entry].seg = 0x08; //kernel codeseg
}

//initialise IDT
void idt_init(){
    idtr.offset = (uint64_t)idt;
    idtr.size = sizeof(idt_entry_t)*256 - 1; //256 max entries in an IDT

    //load the IDT and set the interrupt flag
    asm volatile ("lidt %0" : : "m"(idtr));
    asm volatile ("sti");
}