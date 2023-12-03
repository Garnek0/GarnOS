/*  
*   File: smp.c
*
*   Author: Garnek
*   
*   Description: Symmetric Multiprocessing support
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "cpus.h"
#include <sys/bootloader.h>
#include <kstdio.h>
#include <sys/dal/dal.h>
#include <cpu/apic/apic.h>
#include <hw/ioapic/ioapic.h>

#include <term/term.h>

#include <cpu/gdt/gdt.h>
#include <cpu/interrupts/interrupts.h>
#include <mem/mm/vmm.h>
#include <mem/mm/kheap.h>
#include <mem/memutil/memutil.h>
#include <cpuid.h>
#include <kerrno.h>

bool isx2APIC;

static void smp_configure_cpu_device(){
    device_t* cpuDevice = kmalloc(sizeof(device_t));
    cpuDevice->bus = DEVICE_BUS_NONE;
    cpuDevice->data = NULL;
    cpuDevice->node = NULL;
    cpuDevice->type = DEVICE_TYPE_PROCESSOR;
    cpuDevice->id = 0;

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

void _ready_cpus(){
    //boot the other cpus
    gdt_init();
    interrupts_init();
    asm ("mov %0, %%cr3" : : "r" (PML4));
    apic_init(isx2APIC);
    smp_configure_cpu_device();
    while(1) asm("hlt");
}

bool cpus_check_x2apic(){
    if(bl_is_x2apic()) return true;
    else {
        kerrno = ENODEV;
        return false;
    }
}

void cpus_init(){
    kerrno = 0;

    if(cpus_check_x2apic()){
        klog("SMP: Set x2APIC mode.\n", KLOG_OK);
        isx2APIC = true;
    } else {
        klog("SMP: Could not set x2APIC mode!\n", KLOG_FAILED);
        isx2APIC = false;
    }

    struct limine_smp_info* smpinfo;
    for(size_t i = 0; i < bl_get_cpu_count(); i++){
        smpinfo = bl_get_cpu_info(i);
        smpinfo->goto_address = _ready_cpus;
    }
    apic_init(isx2APIC);

    smp_configure_cpu_device();

    ioapic_init();

    klog("SMP: SMP and APICs Initialised Successfully. (%d CPUs)\n", KLOG_OK, bl_get_cpu_count());
}