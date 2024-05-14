/*  
*   File: ioapic.c
*
*   Author: Garnek
*   
*   Description: Driver for the I/O APIC
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "ioapic.h"
#include <garn/kstdio.h>
#include <garn/hw/ports.h>
#include <garn/acpi/acpi-tables.h>
#include <cpu/multiproc/multiproc-internals.h>
#include <garn/mm.h>
#include <sys/bootloader.h>
#include <cpu/apic/apic.h>

uint8_t ioapicCount;
uint8_t ioapicIDs[256];
void* ioapicAddresses[256];
uint32_t ioapicGSIs[256];

spinlock_t ioapicLock;

bool fallback = false;

void ioapic_write_reg(uint64_t ioapicAddress, uint8_t reg, uint32_t data){
    if(fallback) return;
    lock(ioapicLock, {
        *(volatile uint32_t*)(IOREGSEL(ioapicAddress)) = reg;
        *(volatile uint32_t*)(IOWIN(ioapicAddress)) = data;
    });
}

uint32_t ioapic_read_reg(uint64_t ioapicAddress, uint32_t reg){
    if(fallback) return 0;
    lock(ioapicLock, {
        *(volatile uint32_t*)(IOREGSEL(ioapicAddress)) = reg;
    });
    return *(volatile uint32_t*)(IOWIN(ioapicAddress));
}

void ioapic_redirect(ioapic_redirection_entry_t redirection, uint32_t entry){
    if(fallback) return;

    uint64_t data = redirection.bits;
    size_t ioapic;

    for(ioapic = 0; (ioapicGSIs[ioapic] + (ioapic_read_reg((uint64_t)ioapicAddresses[ioapic], IOAPICVER) >> 16)) < entry; ioapic++);

    klog("Redirecting GSI %u to vector %u (IRQ %u) (I/O APIC %u).\n", KLOG_INFO, "I/O APIC", entry, redirection.fields.vector, redirection.fields.vector-32, ioapic);

    ioapic_write_reg((uint64_t)ioapicAddresses[ioapic], IOREDTBL(entry), (data & 0x00000000FFFFFFFF));
    ioapic_write_reg((uint64_t)ioapicAddresses[ioapic], (IOREDTBL(entry)+1), ((data & 0xFFFFFFFF00000000) >> 32));
}

ioapic_redirection_entry_t ioapic_get_redirection(uint32_t entry){
    ioapic_redirection_entry_t red;
    memset(&red, 0, sizeof(ioapic_redirection_entry_t));

    if(fallback) return red;
    size_t ioapic;

    for(ioapic = 0; (ioapicGSIs[ioapic] + (ioapic_read_reg((uint64_t)ioapicAddresses[ioapic], IOAPICVER) >> 16)) < entry; ioapic++);

    red.bits = ((uint64_t)ioapic_read_reg((uint64_t)ioapicAddresses[ioapic], IOREDTBL(entry)) | 
               (uint64_t)((uint64_t)ioapic_read_reg((uint64_t)ioapicAddresses[ioapic], IOREDTBL(entry)+1) << 32));

    return red;
}

void ioapic_init(){
    asm volatile("cli");

    outb(0x20, 0x20); //PIC EOI
    outb(0xA0, 0x20);

    //Remap PIC and disable it

    uint8_t m1, m2;

    m1 = inb(PIC1_DATA);
	m2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, 0x20);
	io_wait();
	outb(PIC2_DATA, 0x28);
	io_wait();
	outb(PIC1_DATA, 4);
	io_wait();
	outb(PIC2_DATA, 2);
	io_wait();
 
	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

    outb(PIC1_DATA, m1);
    io_wait();
    outb(PIC2_DATA, m2);
    io_wait();

    //Disable PIC
    outb(PIC1_DATA, 0xff);
    io_wait();
    outb(PIC2_DATA, 0xff);
    io_wait();

    acpi_madt_record_hdr_t* hdr = MADT->records;
    acpi_madt_record_ioapic_t* ioapicRec;

    ioapic_redirection_entry_t red;
    red.fields.delvMode = 0;
    red.fields.destMode = 0;
    red.fields.pinPolarity = 0;
    red.fields.triggerMode = 0;
    red.fields.mask = 0;
    red.fields.destination = 0;

    for(size_t i = 0; i < MADT->header.length - sizeof(acpi_madt_t);){
        if(hdr->entryType != ACPI_MADT_IOAPIC){
            i += hdr->recordLength;
            hdr = (acpi_madt_record_hdr_t*)((uint64_t)hdr + hdr->recordLength);
            continue;
        }

        ioapicRec = (acpi_madt_record_ioapic_t*)hdr;

        ioapicIDs[ioapicCount] = ioapicRec->ioapicID;
        ioapicGSIs[ioapicCount] = ioapicRec->gsiBase;
        ioapicAddresses[ioapicCount] = (void*)((uint64_t)ioapicRec->ioapicAddress + bl_get_hhdm_offset());
        vmm_map(vmm_get_kernel_pml4(), ioapicRec->ioapicAddress, ioapicRec->ioapicAddress + bl_get_hhdm_offset(), (VMM_PRESENT | VMM_RW | VMM_PCD));
        ioapicCount++;

        klog("New I/O APIC. (ID: %u, GSI Base: %u, Pin count: %u)\n", KLOG_INFO, "I/O APIC", ioapicRec->ioapicID, ioapicRec->gsiBase, ((ioapic_read_reg((uint64_t)ioapicAddresses[ioapicCount-1], IOAPICVER) >> 16) & 0xFF)+1);

        for(int j = 0; j < ((ioapic_read_reg((uint64_t)ioapicAddresses[ioapicCount-1], IOAPICVER) >> 16) & 0xFF)+1; j++){
            red.fields.vector = 32 + ioapicRec->gsiBase + j;
            ioapic_redirect(red, ioapicRec->gsiBase + j);
        }

        i += hdr->recordLength;
        hdr = (acpi_madt_record_hdr_t*)((uint64_t)hdr + hdr->recordLength);
    }

    if(ioapicCount == 0){
        if(!(MADT->flags & 1)){
            panic("I/O APICs not found and PICs not installed.", "I/O APIC");
        }

        //use the PIC as a fallback interrupt controller
        outb(PIC1_DATA, 0x00);
        io_wait();
        outb(PIC2_DATA, 0x00);
        io_wait();

        klog("I/O APICs Not Found! Using PIC Instead.\n", KLOG_FAILED, "I/O APIC");

        fallback = true;

        asm volatile("sti");

        return;
    }

    //Get Interrupt source overrides

    hdr = MADT->records;
    acpi_madt_record_source_override_t* sourceOverrideRec;

    for(size_t i = 0; i < MADT->header.length - sizeof(acpi_madt_t);){
        if(hdr->entryType != ACPI_MADT_INTERRUPT_SOURCE_OVERRIDE){
            i += hdr->recordLength;
            hdr = (acpi_madt_record_hdr_t*)((uint64_t)hdr + hdr->recordLength);
            continue;
        }

        sourceOverrideRec = (acpi_madt_record_source_override_t*)hdr;

        klog("Found I/O APIC Interrupt Override Source. (IRQ %u -> GSI %u)\n", KLOG_INFO, "I/O APIC", sourceOverrideRec->IRQSource, sourceOverrideRec->GSI);

        red.bits = 0;

        red.fields.vector = 32 + sourceOverrideRec->IRQSource;

        uint8_t polarity = sourceOverrideRec->flags & 0b11;
        if(polarity == 0b01) red.fields.pinPolarity = 0;
        else if(polarity == 0b11) red.fields.pinPolarity = 1;

        uint8_t triggerMode = (sourceOverrideRec->flags >> 2) & 0b11;
        if(triggerMode == 0b01) red.fields.triggerMode = 0;
        else if(triggerMode == 0b11) red.fields.triggerMode = 1;

        ioapic_redirect(red, sourceOverrideRec->GSI);
        i += hdr->recordLength;
        hdr = (acpi_madt_record_hdr_t*)((uint64_t)hdr + hdr->recordLength);
    }

    // Get NMI sources

    hdr = MADT->records;
    acpi_madt_record_nmi_source_t* nmiRec;

    for(size_t i = 0; i < MADT->header.length - sizeof(acpi_madt_t);){
        if(hdr->entryType != ACPI_MADT_NMI_SOURCE){
            i += hdr->recordLength;
            hdr = (acpi_madt_record_hdr_t*)((uint64_t)hdr + hdr->recordLength);
            continue;
        }

        nmiRec = (acpi_madt_record_nmi_source_t*)hdr;

        klog("Found I/O APIC NMI Source. (GSI: %u)\n", KLOG_INFO, "I/O APIC", nmiRec->GSI);

        red = ioapic_get_redirection(nmiRec->GSI);

        uint8_t polarity = nmiRec->flags & 0b11;
        if(polarity == 0b01) red.fields.pinPolarity = 0;
        else if(polarity == 0b11) red.fields.pinPolarity = 1;

        uint8_t triggerMode = (nmiRec->flags >> 2) & 0b11;
        if(triggerMode == 0b01) red.fields.triggerMode = 0;
        else if(triggerMode == 0b11) red.fields.triggerMode = 1;

        red.fields.delvMode = 0b100;

        ioapic_redirect(red, nmiRec->GSI);
        i += hdr->recordLength;
        hdr = (acpi_madt_record_hdr_t*)((uint64_t)hdr + hdr->recordLength);
    }

    asm volatile("sti");

    klog("I/O APICs Initialised Successfully.\n", KLOG_OK, "I/O APIC");
}