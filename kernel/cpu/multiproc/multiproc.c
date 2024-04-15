/*  
*   File: smp.c
*
*   Author: Garnek
*   
*   Description: Symmetric Multiprocessing support
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "multiproc.h"
#include <sys/bootloader.h>
#include <kstdio.h>
#include <sys/dal/dal.h>
#include <cpu/apic/apic.h>
#include <hw/ioapic/ioapic.h>

#include <sys/term/term.h>

#include <cpu/gdt/gdt.h>
#include <cpu/interrupts/interrupts.h>
#include <mem/vmm/vmm.h>
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <cpuid.h>
#include <kerrno.h>

bool isx2APIC;

//Create a CPU device
static void multiproc_configure_cpu_device(){
    device_t* cpuDevice = kmalloc(sizeof(device_t));
    cpuDevice->bus = DEVICE_BUS_NONE;
    cpuDevice->data = NULL;
    cpuDevice->node = NULL;
    cpuDevice->type = DEVICE_TYPE_PROCESSOR;
    cpuDevice->id = 0;

    //Using CPUID to get the CPU brand name

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

void _ready_cpus(struct limine_smp_info* cpuinfo){
    //boot the other cpus

    //GDT and TSS
    gdt_init(cpuinfo->processor_id);

    //IDT and Interrupts
    interrupts_init();

    //Load Kernel Address Space
    vaspace_switch(vmm_get_kernel_pml4());

    //APIC
    apic_init(isx2APIC);

    //CPU device
    multiproc_configure_cpu_device();

    //Halt (for now)
    while(1) asm("hlt");
}

bool multiproc_check_x2apic(){
    //Check if x2APIC is supported
    if(bl_is_x2apic()) return true;
    else {
        kerrno = ENODEV;
        return false;
    }
}

void multiproc_init(){
    kerrno = 0;

    if(multiproc_check_x2apic()){
        klog("multiproc: Set x2APIC mode.\n", KLOG_OK);
        isx2APIC = true;
    } else {
        klog("multiproc: Could not set x2APIC mode!\n", KLOG_FAILED);
        isx2APIC = false;
    }

    //Start up the other processors

    struct limine_smp_info* cpuinfo;
    for(size_t i = 0; i < bl_get_cpu_count(); i++){
        cpuinfo = bl_get_cpu_info(i);
        cpuinfo->goto_address = _ready_cpus;
    }
    apic_init(isx2APIC);

    multiproc_configure_cpu_device();

    //Initialise I/O APICS

    ioapic_init();

    klog("multiproc: Processors and APICs Initialised Successfully. (%d CPUs)\n", KLOG_OK, bl_get_cpu_count());
}