/*  
*   File: apic.c
*
*   Author: Garnek
*   
*   Description: APIC Enabling and Support
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "apic.h"
#include <garn/kstdio.h>
#include <sys/bootloader.h>
#include <garn/mm.h>
#include <garn/acpi/acpi-tables.h>
#include <cpu/multiproc/multiproc-internals.h>
#include <garn/irq.h>
#include <garn/hw/ports.h>
#include <garn/acpi/acpi-tables.h>

static const char* apicErrorStrings[] = {
    "Send Checksum Error",
    "Receive Checksum Error",
    "Send Accept Error",
    "Receive Accept Error",
    "Redirectable IPI",
    "Send Illegal Vector",
    "Received Illegal Vector",
    "Illegal Register Address"
};

static void* LAPICAddress;

spinlock_t apicLock;

static uint32_t apic_read_register(uint16_t reg){
    return *(volatile uint32_t*)((uint64_t)LAPICAddress+reg);
}

static void apic_write_register(uint16_t reg, uint32_t data){
    lock(apicLock, {
        *(volatile uint32_t*)((uint64_t)LAPICAddress+reg) = data;
    });
}

void apic_error_handler(stack_frame_t* regs){
    uint32_t esr = apic_read_register(APIC_ERROR_STATUS);

    for(int i = 0; i < 8; i++){
        if(esr & (1 << i)){
            panic("A Local APIC Signaled %s!", "APIC", apicErrorStrings[i]);
        }
    }
}

void apic_eoi(){
    apic_write_register(APIC_EOI, 0);

    //ensures PIC support in case it is used instead
    //of the I/O APIC
    outb(0x20, 0x20);
    outb(0xA0, 0x20);
}

void apic_init(){
    //We only need to get the address once
    if(!LAPICAddress){
        LAPICAddress = (void*)((uint64_t)MADT->LAPICAddress);

        //Get LAPIC Address override (if it exists)

        acpi_madt_record_hdr_t* hdr = MADT->records;
        acpi_madt_record_lapic_addr_override_t* lapicAddrOverride;

        for(size_t i = 0; i < MADT->header.length - sizeof(acpi_madt_t);){
            if(hdr->entryType != ACPI_MADT_LAPIC_ADDR_OVERRIDE){
                i += hdr->recordLength;
                hdr = (acpi_madt_record_hdr_t*)((uint64_t)hdr + hdr->recordLength);
                continue;
            }

            lapicAddrOverride = (acpi_madt_record_lapic_addr_override_t*)hdr;

            klog("Using 64-bit Address.\n", KLOG_INFO, "APIC");

            LAPICAddress = (void*)lapicAddrOverride->addr;

            i += hdr->recordLength;
            hdr = (acpi_madt_record_hdr_t*)((uint64_t)hdr + hdr->recordLength);
        }

        vmm_map(vmm_get_kernel_pml4(), (uint64_t)LAPICAddress, (uint64_t)(LAPICAddress + bl_get_hhdm_offset()), (VMM_PRESENT | VMM_RW | VMM_PCD));
        LAPICAddress = (void*)((uint64_t)LAPICAddress + bl_get_hhdm_offset());
    }

    //This only works for xAPIC
    uint32_t lapicID = apic_read_register(APIC_ID) >> 24;

    //Get the LAPIC NMI Input

    //First, find the current APIC in the MADT
    acpi_madt_record_hdr_t* hdr = MADT->records;
    acpi_madt_record_lapic_t* lapicRec;

    for(size_t i = 0; i < MADT->header.length - sizeof(acpi_madt_t);){
        if(hdr->entryType != ACPI_MADT_APIC){
            i += hdr->recordLength;
            hdr = (acpi_madt_record_hdr_t*)((uint64_t)hdr + hdr->recordLength);
            continue;
        }

        lapicRec = (acpi_madt_record_lapic_t*)hdr;


        if(lapicRec->apicID == lapicID){
            //Match!
            //Now look for the corresponding APIC NMI Structure
            acpi_madt_record_hdr_t* hdr2 = MADT->records;
            acpi_madt_record_lapic_nmi_t* lapicNMIRec;

            for(size_t j = 0; j < MADT->header.length - sizeof(acpi_madt_t);){
                if(hdr2->entryType != ACPI_MADT_LAPIC_NMI){
                    j += hdr2->recordLength;
                    hdr2 = (acpi_madt_record_hdr_t*)((uint64_t)hdr2 + hdr2->recordLength);
                    continue;
                }

                lapicNMIRec = (acpi_madt_record_lapic_nmi_t*)hdr2;


                if(lapicNMIRec->acpiID == lapicRec->acpiID || lapicNMIRec->acpiID == 0xFF){
                    //We found the right entry

                    klog("Found Local APIC NMI Structure for APIC %u.\n", KLOG_INFO, "APIC", lapicRec->apicID);

                    uint32_t data = 2; //NMI Vector
                    data |= (0b100 << 8); //NMI Delivery Mode

                    uint8_t polarity = lapicNMIRec->flags & 0b11;
                    if(polarity == 0b01) data &= ~(1 << 13);
                    else if(polarity == 0b11) data |= (1 << 13);

                    uint8_t triggerMode = (lapicNMIRec->flags >> 2) & 0b11;
                    if(triggerMode == 0b01) data &= ~(1 << 15);
                    else if(triggerMode == 0b11) data |= (1 << 15);

                    if(lapicNMIRec->pinNumber == 0){
                        apic_write_register(APIC_LVT_LINT0, data);
                    } else if(lapicNMIRec->pinNumber == 1){
                        apic_write_register(APIC_LVT_LINT1, data);
                    }
                }

                j += hdr2->recordLength;
                hdr2 = (acpi_madt_record_hdr_t*)((uint64_t)hdr2 + hdr2->recordLength);
            }
        }

        i += hdr->recordLength;
        hdr = (acpi_madt_record_hdr_t*)((uint64_t)hdr + hdr->recordLength);
    }

    //This basically turns on the APIC
    apic_write_register(APIC_SPURIOUS_INT_VECT, 255 | 0x100);

    //APIC Error
    irq_add_handler(223, apic_error_handler, 0);
    apic_write_register(APIC_LVT_ERROR, 254);

    apic_write_register(APIC_TPR, 0);
}