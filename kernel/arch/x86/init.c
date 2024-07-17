/*  
*   File: init.c
*
*   Author: Garnek
*   
*   Description: x86 CPU initialisation
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <arch/arch-internals.h>
#include <arch/x86/gdt/gdt.h>
#include <arch/x86/interrupts/interrupt-internals.h>
#include <arch/x86/apic/apic.h>
#include <arch/x86/fpu/fpu.h>
#include <garn/arch.h>

bool interruptsInitialised = false;
bool ioapicsInitialised = false;

void arch_init_full(int cpu){
    arch_init_early(cpu);
    arch_init_late(cpu);
}

void arch_init_early(int cpu){
    gdt_init(cpu);

    if(!interruptsInitialised){
        interrupts_init();
        interruptsInitialised = true;
    }

	fpu_init();
}

void arch_init_late(int cpu){
    apic_init();

    //Initialise I/O APICS
    if(!ioapicsInitialised){
        ioapic_init();
        ioapicsInitialised = true;
    }
}
