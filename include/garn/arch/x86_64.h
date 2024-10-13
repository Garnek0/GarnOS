#pragma once

#include <garn/types.h>

#define X86_64_PUSH(type,val,stack) do { \
	stack -= sizeof(type); \
	while(stack & (sizeof(type)-1)) stack--; \
	*((type*)stack) = (type)(val); \
} while(0)

#define X86_64_PUSHSTR(s,stack) do { \
	ssize_t l = strlen(s); \
	do { \
		X86_64_PUSH(char,s[l],stack); \
		l--; \
	} while (l>=0); \
} while (0)

void arch_outb(uint32_t port, uint8_t data);
uint8_t arch_inb(uint32_t port);
void arch_outw(uint32_t port, uint16_t data);
uint16_t arch_inw(uint32_t port);
void arch_outl(uint32_t port, uint32_t data);
uint32_t arch_inl(uint32_t port);
void arch_io_wait();
