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

//These functions are expected to be called multiple times (once by each core)
void arch_init_full(int cpu);
void arch_init_early(int cpu);
void arch_init_late(int cpu);

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

#endif //ARCH_INTERNALS_H