#include "exceptions.h"
#include <cpu/interrupts/idt.h>
#include <cpu/interrupts/interrupts.h>

#include <drivers/serial/serial.h>

#include <kstdio.h>

#include <sys/panic.h>

void exception_handler(stack_frame_t* regs){

    //TODO: Remake this

    if(regs->intn == 0){
        panic("Exception: Divide Error!");
    } else if (regs->intn == 13){
        panic("Exception: General Protection Fault!");
    } else if (regs->intn == 14){
        panic("Exception: Page Fault!");
        
    } else {
        panic("Exception: Undefined Exception!");
    }
}

void exceptions_init(){
    idt_set_entry(0, isr0, INT_GATE);
    idt_set_entry(1, isr1, INT_GATE);
    idt_set_entry(2, isr2, INT_GATE);
    idt_set_entry(3, isr3, INT_GATE);
    idt_set_entry(4, isr4, INT_GATE);
    idt_set_entry(5, isr5, INT_GATE);
    idt_set_entry(6, isr6, INT_GATE);
    idt_set_entry(7, isr7, INT_GATE);
    idt_set_entry(8, isr8, INT_GATE);
    idt_set_entry(9, isr9, INT_GATE);
    idt_set_entry(10, isr10, INT_GATE);
    idt_set_entry(11, isr11, INT_GATE);
    idt_set_entry(12, isr12, INT_GATE);
    idt_set_entry(13, isr13, INT_GATE);
    idt_set_entry(14, isr14, INT_GATE);
    idt_set_entry(15, isr15, INT_GATE);
    idt_set_entry(16, isr16, INT_GATE);
    idt_set_entry(17, isr17, INT_GATE);
    idt_set_entry(18, isr18, INT_GATE);
    idt_set_entry(19, isr19, INT_GATE);
    idt_set_entry(20, isr20, INT_GATE);
    idt_set_entry(21, isr21, INT_GATE);
    idt_set_entry(22, isr22, INT_GATE);
    idt_set_entry(23, isr23, INT_GATE);
    idt_set_entry(24, isr24, INT_GATE);
    idt_set_entry(25, isr25, INT_GATE);
    idt_set_entry(26, isr26, INT_GATE);
    idt_set_entry(27, isr27, INT_GATE);
    idt_set_entry(28, isr28, INT_GATE);
    idt_set_entry(29, isr29, INT_GATE);
    idt_set_entry(30, isr30, INT_GATE);
    idt_set_entry(31, isr31, INT_GATE);

    klog("Exception Handler Initialised Successfully.\n", KLOG_OK);
}