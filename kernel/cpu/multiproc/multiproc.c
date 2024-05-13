/*  
*   File: smp.c
*
*   Author: Garnek
*   
*   Description: Multiprocessing Code
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

    char name[49];
    memcpy((void*)name, (void*)regs, 48);
    name[48] = 0;

    //Remove the spaces
    for(int i = 47; i > 0; i--){
        if(name[i] == ' ' || name[i] == 0) name[i] = 0;
        else break;
    }

    cpuDevice->name = kmalloc(sizeof(uint32_t) * 13);

    memcpy(cpuDevice->name, name, 49);

    device_add(cpuDevice);
}

volatile size_t CPUInitCount;
spinlock_t InitCountLock;

void ready_cpus(struct limine_smp_info* cpuinfo){
    //boot the other cpus

    //GDT and TSS
    gdt_init(cpuinfo->processor_id);

    //IDT and Interrupts
    interrupts_init();

    //Load Kernel Address Space
    vaspace_switch(vmm_get_kernel_pml4());

    //APIC
    apic_init();

    //CPU device
    multiproc_configure_cpu_device();

    //Report Initialisation Completion
    lock(InitCountLock, {
        klog("Processor and APIC %u Initialised.\n", KLOG_OK, "multiproc", cpuinfo->processor_id);
        CPUInitCount++;
    });

    //Halt (for now)
    while(1) asm("hlt");
}

void multiproc_init(){
    klog("Initialising Processors and APICs...\n", KLOG_INFO, "multiproc");

    apic_init();

    multiproc_configure_cpu_device();

    klog("Bootstrap Processor APIC Initialised (APIC 0).\n", KLOG_OK, "multiproc");

    CPUInitCount++;

    //Start up the other processors

    struct limine_smp_info* cpuinfo;
    for(size_t i = 0; i < bl_get_cpu_count(); i++){
        cpuinfo = bl_get_cpu_info(i);
        cpuinfo->goto_address = ready_cpus;
    }

    size_t CPUCount = bl_get_cpu_count();

    while(CPUInitCount != CPUCount) asm volatile("nop");

    klog("Processors and APICs Initialised Successfully. (%d CPUs)\n", KLOG_OK, "multiproc", CPUInitCount);

    //Initialise I/O APICS

    ioapic_init();
}