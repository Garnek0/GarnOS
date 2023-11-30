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
#include <hw/ports.h>
#include <cpu/smp/spinlock.h>
#include <kstdio.h>

spinlock_t PITLock;

pit_info PITInfo;

void pit_handler(stack_frame_t* regs){
    PITInfo.ticksSinceOSBoot++;
    return;
}

//TODO: Make it so that this can be used by multiple processors at once
void pit_sleep(size_t ms){
    lock(PITLock, {
        uint64_t finalTicks = PITInfo.ticksSinceOSBoot + ms;
        while(PITInfo.ticksSinceOSBoot < finalTicks){
            asm volatile("nop");
        }
    });
}

void pit_set_divisor(uint16_t div){
    lock(PITLock, {
        outb(PIT_CHANNEL_0, (div & 0xff));
        io_wait();
        outb(PIT_CHANNEL_0, ((div >> 8) & 0xff));
        io_wait();
    });
}

static void pit_set_frequency(uint32_t freq){
    pit_set_divisor(PIT_BASE_FREQUENCY / freq);
}

void pit_init(){

    outb(PIT_MODE_OR_COMMAND, 0b00110100);

    pit_set_frequency(1000); //1ms per tick

    irq_set_handler(0, pit_handler);

    klog("PIT: Timer Initialised.\n", KLOG_OK);
}