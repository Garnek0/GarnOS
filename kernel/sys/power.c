#include "garn/arch/common.h"
#include "power-internals.h"

#include <garn/power.h>
#include <garn/kstdio.h>
#include <module/module-internals.h>
#include <garn/kstdio.h>
#include <sys/term/term-internals.h>

power_t power;

int power_shutdown(){
    if(power.shutdown == NULL){
        klog("Shutdown Not Implemented!\n", KLOG_FAILED, "Power");
        return -1;
    }

	term_clear();
	kernel_screen_output_enable();
	module_shutdown();

	klog("Reached shutdown\n", KLOG_OK, "Power");

    return power.shutdown();
}

int power_reboot(){
    if(power.reboot == NULL){
        klog("Reboot Not Implemented!\n", KLOG_FAILED, "Power");
        return -1;
    }

	term_clear();
	kernel_screen_output_enable();
	module_shutdown();

	klog("Reached reboot\n", KLOG_OK, "Power");

    return power.reboot();
}

int power_suspend(){
    if(power.suspend == NULL){
        klog("Suspend Not Implemented!\n", KLOG_FAILED, "Power");
        return -1;
    }

	klog("Reached suspend\n", KLOG_OK, "Power");

    return power.suspend();
}

inline void power_set_shutdown(void* func){
    power.shutdown = func;
}

inline void power_set_reboot(void* func){
    power.reboot = func;
}

inline void power_set_suspend(void* func){
    power.suspend = func;
}

static int _power_shutdown_default(){
    kprintf("You may now power off your computer.");
    arch_disable_interrupts();
    for(;;){
        arch_stop();
    }
    __builtin_unreachable();
}

static int _power_reboot_default(){
    kprintf("You may now *manually* restart your computer ;)");
    arch_disable_interrupts();
    for(;;){
        arch_stop();
    }
    __builtin_unreachable();
}

void power_init(){
    power.shutdown = _power_shutdown_default;
    power.reboot = _power_reboot_default;

    klog("Power Initialised.\n", KLOG_OK, "Power");
}
