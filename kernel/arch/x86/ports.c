/*  
*   File: ports.c
*
*   Author: Garnek
*   
*   Description: x86 I/O Ports
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <garn/types.h>
#include <garn/arch.h>

void arch_outb(unsigned long port, uint8_t data){
    asm volatile("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

uint8_t arch_inb(unsigned long port){
    uint8_t data;
    asm volatile("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}

void arch_outw(unsigned long port, uint16_t data){
    asm volatile("outw %w0, %w1" : : "a" (data), "Nd" (port));
}

uint16_t arch_inw(unsigned long port){
    uint16_t data;
    asm volatile("inw %w1, %w0" : "=a" (data) : "Nd" (port));
    return data;
}

void arch_outl(unsigned long port, uint32_t data){
    asm volatile("outl %0, %w1" : : "a" (data), "Nd" (port));
}

uint32_t arch_inl(unsigned long port){
    uint32_t data;
    asm volatile("inl %w1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

void arch_io_wait(){
    arch_outb(0x80, 0);
}