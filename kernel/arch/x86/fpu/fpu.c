/*
*	File: fpu.c	 
*
*	Author: Garnek 
*
*	Description: Support for x87, MMX and SSE
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fpu.h"
#include <arch/arch-internals.h>

void fpu_init(){
	// Get current CR0 and CR4
	uint64_t cr0, cr4;
	asm volatile("mov %%cr0, %%rax\n"
				 "mov %%rax, %0\n" 
				 "mov %%cr4, %%rax\n"
				 "mov %%rax, %1" : "=r"(cr0), "=r"(cr4) :: "%rax");

	// Clear EM and TS bits
	cr0 &= ~(1 << 2);
	cr0 &= ~(1 << 3);
	// Set NE bit
	cr0 |= (1 << 5);
	// Set MP bit
	cr0 |= (1 << 1);
	// Set OSFXSR and OSXMMEXCEPT bits (Enable SSE)
	cr4 |= (3 << 9);

	// Set CR0 and CR4
	asm volatile("mov %0, %%rax\n"
				 "mov %%rax, %%cr0\n"
				 "mov %1, %%rax\n"
				 "mov %%rax, %%cr4" :: "r"(cr0), "r"(cr4) : "%rax");
}

void arch_prepare_fpu(){
	asm volatile("fninit");

	uint16_t cw = 0x33F;
	asm volatile("fnclex; fldcw %0" :: "m"(cw));
	uint32_t mxcsr = 0x1F80;
	asm volatile("ldmxcsr %0" :: "m"(mxcsr));
}
