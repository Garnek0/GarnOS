/*  
*   File: interrupts.c
*
*   Author: Garnek
*   
*   Description: x86 Interrupts
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "interrupt-internals.h"
#include <arch/x86/interrupts/idt.h>
#include <arch/arch-internals.h>
#include <arch/x86/interrupts/apic/apic.h>
#include <sys/syscall_internals.h>
#include <garn/kstdio.h>

#include <garn/irq.h>
#include <garn/arch.h>

//initialise interrupts
void interrupts_init(int cpu){
    idt_init();
    exceptions_init();
    irq_init();

    asm volatile ("sti");
}

void arch_disable_interrupts(){
    asm volatile("cli");
}

void arch_enable_interrupts(){
    asm volatile("sti");
}

void arch_end_interrupt(){
    // (apic_eoi() contains a PIC EOI as well,
    // so things wont break if the system is
    // using the PIC as a fallback ;))
    apic_eoi();
}
