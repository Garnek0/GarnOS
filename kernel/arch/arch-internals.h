/*  
*   File: arch-internals.h
*
*   Author: Garnek
*   
*   Description: Internal architecture-specific stuff
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef ARCH_INTERNALS_H
#define ARCH_INTERNALS_H

#include <garn/irq.h>
#include <garn/mm.h>

//These functions are expected to be called multiple times (once by each core)
void arch_init_full(int cpu);
void arch_init_early(int cpu);
void arch_init_late(int cpu);

void arch_prepare_fpu();

void arch_syscall_init();
int arch_syscall_number(stack_frame_t* regs);
void arch_syscall_return(stack_frame_t* regs, long value);
long arch_syscall_arg0(stack_frame_t* regs);
long arch_syscall_arg1(stack_frame_t* regs);
long arch_syscall_arg2(stack_frame_t* regs);
long arch_syscall_arg3(stack_frame_t* regs);
long arch_syscall_arg4(stack_frame_t* regs);
long arch_syscall_arg5(stack_frame_t* regs);

void arch_dump_cpu_state(stack_frame_t* regs);

void arch_set_kernel_stack(size_t cpu, uintptr_t stack);
void arch_usermode_enter(uintptr_t entry, uintptr_t stack);

void arch_store_context(stack_frame_t* regs);
void arch_restore_context(stack_frame_t* regs);
void arch_set_tsp(uintptr_t pointer, stack_frame_t* regs);

void arch_compat_checks();

page_table_t* arch_mm_init();
void arch_mm_map(page_table_t* pt, uint64_t physAddr, uint64_t virtAddr, uint32_t flags);
void arch_mm_unmap(page_table_t* pt, uint64_t virtAddr);
void arch_mm_set_flags(page_table_t* pt, uint64_t virtAddr, uint32_t flags);
bool arch_mm_is_page_free(page_table_t* pt, uint64_t virtAddr);
uint64_t arch_mm_virt_to_phys(page_table_t* pt, uint64_t virtAddr);

void arch_vaspace_switch(page_table_t* pt);
page_table_t* arch_vaspace_new();
void arch_vaspace_destroy(page_table_t* pt);
void arch_vaspace_clear(page_table_t* pt);
page_table_t* arch_vaspace_clone(page_table_t* toClone);

#endif //ARCH_INTERNALS_H
