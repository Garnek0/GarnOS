/*  
*   File: apic.c
*
*   Author: Garnek
*   
*   Description: APIC Enabling and Support
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "apic.h"
#include <kstdio.h>
#include <module/module.h>
#include <sys/bootloader.h>
#include <mem/vmm/vmm.h>
#include <acpi/tables/tables.h>
#include <cpu/smp/cpus.h>
#include <hw/ports.h>

static uint64_t* LAPICAddress;

spinlock_t apicLock;

static uint32_t apic_read_register(uint16_t reg){
    return *(volatile uint32_t*)((uint64_t)LAPICAddress+reg);
}

static void apic_write_register(uint16_t reg, uint32_t data){
    lock(apicLock, {
        *(volatile uint32_t*)((uint64_t)LAPICAddress+reg) = data;
    });
}

void apic_eoi(){
    apic_write_register(APIC_EOI, 0);

    //ensures PIC support in case it is used instead
    //of the I/O APIC
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

void apic_init(bool isx2APIC){
    vmm_map(vmm_get_kernel_pml4(), MADT->LAPICAddress, MADT->LAPICAddress + bl_get_hhdm_offset(), 0x13);
    LAPICAddress = (uint64_t*)(MADT->LAPICAddress + bl_get_hhdm_offset());

    uint32_t lapicID = apic_read_register(APIC_ID);
    if(!isx2APIC) lapicID = (lapicID >> 24);

    //This is basically turning on the APIC
    apic_write_register(APIC_SPURIOUS_INT_VECT, apic_read_register(APIC_SPURIOUS_INT_VECT) | 0x100);
}