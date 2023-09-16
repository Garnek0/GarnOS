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
#include <sys/rblogs.h>
#include <module/module.h>
#include <sys/bootloader.h>
#include <mem/mm/vmm.h>
#include <acpi/tables/tables.h>
#include <cpu/smp/smp.h>

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
}

void apic_init(bool isx2APIC){

    //TODO VERY IMPORTANT: set up mtrr memory regions

    LAPICAddress = (uint64_t*)(MADT->LAPICAddress + bl_get_hhdm_offset());

    uint32_t lapicID = apic_read_register(APIC_ID);
    if(!isx2APIC) lapicID = (lapicID >> 24);

    //This is basically turning on the APIC
    apic_write_register(APIC_SPURIOUS_INT_VECT, apic_read_register(APIC_SPURIOUS_INT_VECT) | 0x100);
}