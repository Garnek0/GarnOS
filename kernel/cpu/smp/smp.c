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
#include <sys/device.h>
#include <cpu/apic/apic.h>
#include <hw/ioapic/ioapic.h>

#include <term/term.h>

#include <cpu/gdt/gdt.h>
#include <cpu/interrupts/interrupts.h>
#include <mem/mm/vmm.h>
#include <mem/mm/kheap.h>
#include <mem/memutil/memutil.h>
#include <cpuid.h>

bool isx2APIC;

static void smp_configure_cpu_device(){
    device_t* cpuDevice = kmalloc(sizeof(device_t));
    cpuDevice->bus = DEVICE_BUS_NONE;
    cpuDevice->data = NULL;
    cpuDevice->driver = NULL;
    cpuDevice->type = DEVICE_TYPE_PROCESSOR;

    uint32_t regs[13];

    __get_cpuid(0x80000000, &regs[0], &regs[1], &regs[2], &regs[3]);

    if (regs[0] < 0x80000004) return;

    __get_cpuid(0x80000002, &regs[0], &regs[1], &regs[2], &regs[3]);
    __get_cpuid(0x80000003, &regs[4], &regs[5], &regs[6], &regs[7]);
    __get_cpuid(0x80000004, &regs[8], &regs[9], &regs[10], &regs[11]);

    regs[12] = 0;

    cpuDevice->name = kmalloc(sizeof(uint32_t) * 13);

    memcpy(cpuDevice->name, regs, sizeof(uint32_t) * 13);

    device_add(cpuDevice);
}

void smp_ready_cpus(){
    //boot the other cpus
    gdt_init();
    interrupts_init();
    asm ("mov %0, %%cr3" : : "r" (PML4));
    apic_init(isx2APIC);
    smp_configure_cpu_device();
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
    for(size_t i = 0; i < bl_get_cpu_count(); i++){
        smpinfo = bl_get_cpu_info(i);
        smpinfo->goto_address = smp_ready_cpus;
    }
    apic_init(isx2APIC);

    smp_configure_cpu_device();

    ioapic_init();

    klog("SMP and APICs Initialised Successfully. (%d CPUs)\n", KLOG_OK, bl_get_cpu_count());
    rb_log("SMP", KLOG_OK);
}