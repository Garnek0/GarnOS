/*  
*   File: smp.c
*
*   Author: Garnek
*   
*   Description: Symmetric Multiprocessing support
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "smp.h"
#include <sys/bootloader.h>
#include <kstdio.h>
#include <sys/rblogs.h>
#include <cpu/apic/apic.h>
#include <drivers/ioapic/ioapic.h>

#include <term/term.h>

#include <cpu/gdt/gdt.h>
#include <cpu/interrupts/interrupts.h>
#include <mem/mm/vmm.h>

bool isx2APIC;

void smp_ready_cpus(){
    //boot the other cpus
    gdt_init();
    interrupts_init();
    asm ("mov %0, %%cr3" : : "r" (PML4));
    apic_init(isx2APIC);
    term_enable();
    while(1) asm("hlt");
}

void smp_init(){
    if(bl_is_x2apic()){
        klog("Set x2APIC mode.\n", KLOG_OK);
        rb_log("x2APIC", KLOG_OK);
        isx2APIC = true;
    } else {
        klog("Could not set x2APIC mode!\n", KLOG_FAILED);
        rb_log("x2APIC", KLOG_FAILED);
        isx2APIC = false;
    }

    struct limine_smp_info* smpinfo;
    for(int i = 0; i < bl_get_cpu_count(); i++){
        smpinfo = bl_get_cpu_info(i);
        smpinfo->goto_address = smp_ready_cpus;
    }
    apic_init(isx2APIC);

    ioapic_init();

    klog("SMP and APICs Initialised Successfully. (%d CPUs)\n", KLOG_OK, bl_get_cpu_count());
    rb_log("SMP", KLOG_OK);
}