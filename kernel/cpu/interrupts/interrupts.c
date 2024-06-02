/*  
*   File: interrupts.c
*
*   Author: Garnek
*   
*   Description: Interrupt Handler
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "interrupt-internals.h"
#include <cpu/interrupts/idt.h>
#include <sys/syscall_internals.h>
#include <garn/kstdio.h>

#include <garn/irq.h>

//initialise interrupts
void interrupts_init(){
    idt_init();
    exceptions_init();
    irq_init();
    syscall_init();

    asm volatile ("sti");
}