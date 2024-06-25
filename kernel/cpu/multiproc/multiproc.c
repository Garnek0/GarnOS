/*  
*   File: multiproc.c
*
*   Author: Garnek
*   
*   Description: Multiprocessing Code
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "multiproc-internals.h"
#include <sys/bootloader.h>
#include <garn/kstdio.h>
#include <garn/dal/dal.h>
#include <garn/dal/device-types.h>
#include <garn/term/term.h>
#include <arch/arch-internals.h>
#include <garn/irq.h>
#include <garn/mm.h>
#include <cpuid.h>
#include <garn/kerrno.h>

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

    //Load Kernel Address Space
    vaspace_switch(vmm_get_kernel_pml4());

    //Initialise this CPU
    arch_init_full(cpuinfo->processor_id);

    //CPU device
    multiproc_configure_cpu_device();

    //Report Initialisation Completion
    lock(InitCountLock, {
        klog("Processor %u Initialised.\n", KLOG_OK, "multiproc", cpuinfo->processor_id);
        CPUInitCount++;
    });

    //Stop (for now)
    while(1) arch_stop();
}

void multiproc_init(){
    klog("Initialising Processors...\n", KLOG_INFO, "multiproc");

    arch_init_late(0); //Initialise CPU 0 (late)

    multiproc_configure_cpu_device();

    klog("Bootstrap Processor Initialised (CPU 0).\n", KLOG_OK, "multiproc");

    CPUInitCount++;

    //Start up the other processors

    struct limine_smp_info* cpuinfo;
    for(size_t i = 0; i < bl_get_cpu_count(); i++){
        cpuinfo = bl_get_cpu_info(i);
        cpuinfo->goto_address = ready_cpus;
    }

    size_t CPUCount = bl_get_cpu_count();

    while(CPUInitCount != CPUCount) arch_no_op();

    klog("Processors Initialised Successfully. (%d CPUs)\n", KLOG_OK, "multiproc", CPUInitCount);
}