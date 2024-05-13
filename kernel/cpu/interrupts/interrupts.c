/*  
*   File: interrupts.c
*
*   Author: Garnek
*   
*   Description: Interrupt Handler
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "interrupts.h"
#include <cpu/interrupts/idt.h>
#include <sys/syscall.h>
#include <kstdio.h>

#include <cpu/interrupts/exceptions.h>
#include <cpu/interrupts/irq.h>
#include <sys/syscall.h>

//initialise interrupts
void interrupts_init(){
    idt_init();
    exceptions_init();
    irq_init();
    syscall_init();

    asm volatile ("sti");
}