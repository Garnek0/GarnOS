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
#include <garn/config.h>

#define PUSH(type,val,stack) do { \
	stack -= sizeof(type); \
	while(stack & (sizeof(type)-1)) stack--; \
	*((type*)stack) = (type)(val); \
} while(0)

#define PUSHSTR(s,stack) do { \
	ssize_t l = strlen(s); \
	do { \
		PUSH(char,s[l],stack); \
		l--; \
	} while (l>=0); \
} while (0)

void arch_disable_interrupts();
void arch_enable_interrupts();
void arch_end_interrupt();
int arch_get_irq_number(stack_frame_t* regs);

void arch_get_cpu_model_name(char* str);

void arch_no_op();
void arch_stop();

void arch_outb(uint32_t port, uint8_t data);
uint8_t arch_inb(uint32_t port);
void arch_outw(uint32_t port, uint16_t data);
uint16_t arch_inw(uint32_t port);
void arch_outl(uint32_t port, uint32_t data);
uint32_t arch_inl(uint32_t port);
void arch_io_wait();

#endif //ARCH_H
