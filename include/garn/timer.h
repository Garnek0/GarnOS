#pragma once

#include <garn/types.h>
#include <garn/irq.h>

void timer_tick(stack_frame_t* regs);
void ksleep(uint64_t ms);
uint64_t timer_get_ticks();
