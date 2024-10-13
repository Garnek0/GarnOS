#include "interrupt-internals.h"
#include <garn/arch/common.h>
#include <arch/x86_64/interrupts/apic/apic.h>
#include <garn/mm.h>
#include <arch/x86_64/interrupts/idt.h>
#include <garn/kstdio.h>

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

int arch_get_irq_number(stack_frame_t* regs){
    return (int)regs->errCode;
}

