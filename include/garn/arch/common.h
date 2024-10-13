#pragma once

#include <garn/irq.h>

void arch_disable_interrupts();
void arch_enable_interrupts();
void arch_end_interrupt();
int arch_get_irq_number(stack_frame_t* regs);

void arch_get_cpu_model_name(char* str);

void arch_no_op();
void arch_stop();
void arch_pause();

