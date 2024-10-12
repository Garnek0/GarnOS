/*  
*   File: panic.c
*
*   Author: Garnek
*   
*   Description: Kernel Panic Routine
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <garn/panic.h>
#include <garn/kstdio.h>
#include <garn/spinlock.h>
#include <sys/bootloader.h>
#include <arch/arch-internals.h>
#include <garn/arch.h>

spinlock_t panicLock;

//TODO: only halts bsp, make panic() halt the other processors too

void panic(const char* str, const char* component, ...){
    kernel_screen_output_enable();
    
    arch_disable_interrupts();

    va_list args;
    va_start(args, component);

    lock(panicLock, {
        kputchar('\n');
        klog("Kernel Panic!\n\n", KLOG_CRITICAL, component);
        kvprintf((char*)str, args);
    });

    va_end(args);

    arch_stop();

    __builtin_unreachable();
}

void panic_exception(const char* str, stack_frame_t* regs, ...){
    kernel_screen_output_enable();
    
    arch_disable_interrupts();

    va_list args;
    va_start(args, regs);

    kputchar('\n');
    klog("Kernel Panic!\n\n", KLOG_CRITICAL, "Exception Handler");
    kprintf("Exception: ");
    kvprintf((char*)str, args);
    kprintf("\n\n");

    lock(panicLock, {
        arch_dump_cpu_state(regs);
    });

    va_end(args);

    arch_stop();

    __builtin_unreachable();
}