/*  
*   File: arch.h
*
*   Author: Garnek
*   
*   Description: Architecture-specific stuff
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef ARCH_H
#define ARCH_H

#include <garn/irq.h>

void arch_disable_interrupts();
void arch_enable_interrupts();
void arch_end_interrupt();
int arch_get_irq_number(stack_frame_t* regs);

void arch_no_op();
void arch_stop();

void arch_outb(unsigned long port, uint8_t data);
uint8_t arch_inb(unsigned long port);
void arch_outw(unsigned long port, uint16_t data);
uint16_t arch_inw(unsigned long port);
void arch_outl(unsigned long port, uint32_t data);
uint32_t arch_inl(unsigned long port);
void arch_io_wait();

#endif //ARCH_H