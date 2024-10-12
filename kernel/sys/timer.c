#include <garn/timer.h>
#include <garn/spinlock.h>

#include "process/sched/sched.h"

volatile uint64_t ticksSinceOSBoot = 0;

void timer_tick(stack_frame_t* regs){
    ticksSinceOSBoot++;
    //preempt every 10 ms
    if(ticksSinceOSBoot%10 == 0) sched_preempt(regs);
    return;
}

void ksleep(uint64_t ms){
    uint64_t finalTicks = ticksSinceOSBoot + ms;
    while(ticksSinceOSBoot < finalTicks){
        arch_no_op();
    }
}

uint64_t timer_get_ticks(){
    return ticksSinceOSBoot;
}
