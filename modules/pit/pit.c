/*  
*   Module: pit.sys
*
*   File: pit.c
*
*   Module Author: Garnek
*   
*   Module Description: PIT Driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "pit.h"

#include <garn/arch.h>
#include <garn/kstdio.h>
#include <garn/mm.h>
#include <garn/dal/dal.h>
#include <garn/module.h>
#include <garn/irq.h>

static void pit_set_divisor(uint16_t div){
    arch_outb(PIT_CHANNEL_0, (div & 0xff));
    arch_io_wait();
    arch_outb(PIT_CHANNEL_0, ((div >> 8) & 0xff));
    arch_io_wait();
}

void pit_set_frequency(uint32_t freq){
    uint32_t div = PIT_BASE_FREQUENCY / freq;

    pit_set_divisor(div);
    klog("Set Frequency to %uHz\n", KLOG_INFO, "PIT", PIT_BASE_FREQUENCY / div);
}

void pit_handler(stack_frame_t* regs){
    timer_tick(regs);
}

void init(){
    return;
}

void fini(){
    return;
}

bool probe(device_t* device){
    if(device->type == DEVICE_TYPE_SYSTEM_DEVICE && device->bus == DEVICE_BUS_NONE) return true;
    return false;
}

bool attach(device_t* device){
    if(!probe(device)) return false;

    arch_outb(PIT_MODE_OR_COMMAND, 0b00110100);

    pit_set_frequency(1000); //1ms per tick

    irq_add_handler(0, pit_handler, 0);

    klog("Timer Initialised.\n", KLOG_OK, "PIT");

    return true;
}

bool remove(device_t* device){
    return true;
}

module_t metadata = {
    .name = "pit",
    .init = init,
    .fini = fini
};

device_driver_t driver_metadata = {
    .probe = probe,
    .attach = attach,
    .remove = remove
};

device_id_t driver_ids[] = {
    DEVICE_CREATE_ID_TIMER(DEVICE_ID_TIMER_PIT),
    0
};