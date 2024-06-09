/*  
*   File: timer.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef TIMER_H
#define TIMER_H

#include <garn/types.h>
#include <garn/irq.h>

void timer_tick(stack_frame_t* regs);
void ksleep(uint64_t ms);
uint64_t timer_get_ticks();

#endif //TIMER_H