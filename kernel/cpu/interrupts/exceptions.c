/*  
*   File: exceptions.c
*
*   Author: Garnek
*   
*   Description: Exception Handler
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "exceptions.h"
#include <cpu/interrupts/idt.h>
#include <cpu/interrupts/interrupts.h>

#include <hw/serial/serial.h>

#include <process/process.h>
#include <process/sched/sched.h>

#include <sys/panic.h>
#include <kstdio.h>

char* exceptionMessages[] = {
    "Divide Error",
    "Debug",
    "NMI Triggered",
    "Breakpoint Reached",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "(Reserved)",
    "(Reserved)",
    "(Reserved)",
    "(Reserved)",
    "(Reserved)",
    "(Reserved)",
    "x87 Floating-Point Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Fault",
    "Virtualisation Fault",
    "Control Protection Fault",
    "(Reserved)",
    "Hypervisor Injection",
    "VMM Communication",
    "Security Fault",
    "(Reserved)"
};

void exception_handler(stack_frame_t* regs){
    if(regs->ds & 0x3){
        //If the last 2 bits of ds are 0b11, then the exception occured in userspace,
        //terminate the currently running process.
        process_t* currentProcess = sched_get_current_process();
        kprintf("PID %d (%s): Process terminated due to exception. (%s)\n", currentProcess->pid, currentProcess->name, exceptionMessages[regs->intn]);
        process_terminate(currentProcess);

        asm volatile("sti");
        while(1) asm volatile("nop"); //Wait for reschedule
    } else {
        panic_exception(exceptionMessages[regs->intn], regs);
    }
}

//initialise exception handler
void exceptions_init(){
    //set up the isrs
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
}