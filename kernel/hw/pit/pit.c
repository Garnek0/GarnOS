/*  
*   File: pit.c
*
*   Author: Garnek
*   
*   Description: Programmable Interval Timer
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "pit.h"
#include <cpu/interrupts/interrupts.h>
#include <cpu/interrupts/irq.h>
#include <hw/ports.h>
#include <cpu/multiproc/spinlock.h>
#include <process/sched/sched.h>
#include <kstdio.h>

spinlock_t PITLock;

volatile uint64_t pitTicksSinceOSBoot;

void pit_handler(stack_frame_t* regs){
    pitTicksSinceOSBoot++;
    //preempt every 10 ms
    if(pitTicksSinceOSBoot%10 == 0) sched_preempt(regs);
    return;
}

//TODO: Make it so that this can be used by multiple processors at once
void pit_sleep(size_t ms){
    lock(PITLock, {
        uint64_t finalTicks = pitTicksSinceOSBoot + ms;
        while(pitTicksSinceOSBoot < finalTicks){
            asm volatile("nop");
        }
    });
}

static void pit_set_divisor(uint16_t div){
    lock(PITLock, {
        outb(PIT_CHANNEL_0, (div & 0xff));
        io_wait();
        outb(PIT_CHANNEL_0, ((div >> 8) & 0xff));
        io_wait();
    });
}

void pit_set_frequency(uint32_t freq){
    uint32_t div = PIT_BASE_FREQUENCY / freq;

    pit_set_divisor(div);
    klog("Set Frequency to %uHz\n", KLOG_INFO, "PIT", PIT_BASE_FREQUENCY / div);
}

uint64_t pit_get_ticks(){
    return pitTicksSinceOSBoot;
}

void pit_init(){

    outb(PIT_MODE_OR_COMMAND, 0b00110100);

    pit_set_frequency(1000); //1ms per tick

    irq_add_handler(0, pit_handler, 0);

    klog("Timer Initialised.\n", KLOG_OK, "PIT");
}