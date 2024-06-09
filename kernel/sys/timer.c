/*  
*   File: timer.c
*
*   Author: Garnek
*   
*   Description: Simple timer interface.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <garn/timer.h>
#include <garn/spinlock.h>

volatile uint64_t ticksSinceOSBoot = 0;

spinlock_t timerLock;

void timer_tick(stack_frame_t* regs){
    ticksSinceOSBoot++;
    //preempt every 10 ms
    if(ticksSinceOSBoot%10 == 0) sched_preempt(regs);
    return;
}

//TODO: Make it so that this can be used by multiple processors at once
void ksleep(uint64_t ms){
    lock(timerLock, {
        uint64_t finalTicks = ticksSinceOSBoot + ms;
        while(ticksSinceOSBoot < finalTicks){
            asm volatile("nop");
        }
    });
}

uint64_t timer_get_ticks(){
    return ticksSinceOSBoot;
}