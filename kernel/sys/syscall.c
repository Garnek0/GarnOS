/*  
*   File: syscall.c
*
*   Author: Garnek
*   
*   Description: System Calls
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "syscall.h"
#include <cpu/interrupts/interrupts.h>
#include <cpu/interrupts/idt.h>
#include <kstdio.h>

void syscall_init(){
    idt_set_entry(0x80, isr128, INT_USER_GATE);
}

void syscall_handler(stack_frame_t* regs){
    kprintf("\nRecieved Syscall! RAX=%x (If this is DEADBEEF then everything is ok)\n", regs->rax);
    //while(1);
}