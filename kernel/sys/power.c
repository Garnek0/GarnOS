/*  
*   File: power.c
*
*   Author: Garnek
*   
*   Description: System Power Management (for now, just basic 'shutdown')
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "power.h"
#include <kstdio.h>

power_t power;

int power_shutdown(){
    if(power.shutdown == NULL){
        klog("Shutdown Not Implemented!\n", KLOG_FAILED);
        return -1;
    }
    return power.shutdown();
}

int power_restart(){
    if(power.restart == NULL){
        klog("Restart Not Implemented!\n", KLOG_FAILED);
        return -1;
    }
    return power.restart();
}

int power_suspend(){
    if(power.suspend == NULL){
        klog("Suspend Not Implemented!\n", KLOG_FAILED);
        return -1;
    }
    return power.suspend();
}

inline void power_set_shutdown(void* func){
    power.shutdown = func;
}

inline void power_set_restart(void* func){
    power.restart = func;
}

inline void power_set_suspend(void* func){
    power.suspend = func;
}

static int _power_shutdown_default(){
    kprintf("You may now power off your computer.");
    asm volatile("cli");
    for(;;){
        asm volatile("hlt");
    }
}

static int _power_restart_default(){
    kprintf("You may now *manually* restart your computer ;)");
    asm volatile("cli");
    for(;;){
        asm volatile("hlt");
    }
}

void power_init(){
    power.shutdown = _power_shutdown_default;
    power.restart = _power_restart_default;

    klog("Power: Power Initialised.\n", KLOG_OK);
}