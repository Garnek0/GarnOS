#include "apic.h"
#include <garn/kstdio.h>
#include <sys/bootloader.h>
#include <garn/mm.h>
#include <cpu/smp/smp.h>
#include <garn/irq.h>
#include <garn/arch.h>
#include <garn/kernel.h>
#include <uacpi/acpi.h>
#include <uacpi/tables.h>

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
    arch_outb(0x20, 0x20);
    arch_outb(0xA0, 0x20);
}

void apic_init(){
    uacpi_table MADTTable;
    uacpi_table_find_by_signature(ACPI_MADT_SIGNATURE, &MADTTable);
    struct acpi_madt* MADT = (struct acpi_madt*)MADTTable.virt_addr;

    //We only need to get the address once    
    if(!LAPICAddress){
        LAPICAddress = (void*)((uint64_t)MADT->local_interrupt_controller_address);

        //Get LAPIC Address override (if it exists)
        struct acpi_madt_lapic_address_override* lapicAddrOverride;
        struct acpi_entry_hdr* hdr = MADT->entries;

        for(size_t i = 0; i < MADT->hdr.length - sizeof(struct acpi_madt);){
            if(hdr->type != ACPI_MADT_ENTRY_TYPE_LAPIC_ADDRESS_OVERRIDE){
                i += hdr->length;
                hdr = (struct acpi_entry_hdr*)((uint64_t)hdr + hdr->length);
                continue;
            }

            lapicAddrOverride = (struct acpi_madt_lapic_address_override*)hdr;

            klog("Using 64-bit Address.\n", KLOG_INFO, "APIC");

            LAPICAddress = (void*)lapicAddrOverride->address;

            i += hdr->length;
            hdr = (struct acpi_entry_hdr*)((uint64_t)hdr + hdr->length);
        }

        vmm_map(vmm_get_kernel_pt(), (uint64_t)LAPICAddress, (uint64_t)((uint64_t)LAPICAddress + bl_get_hhdm_offset()), (VMM_PRESENT | VMM_RW | VMM_CACHE_DISABLE));
        LAPICAddress = (void*)((uint64_t)LAPICAddress + bl_get_hhdm_offset());
    }

    //This only works for xAPIC
    uint32_t lapicID = apic_read_register(APIC_ID) >> 24;

    //Get the LAPIC NMI Input

    //First, find the current APIC in the MADT
    struct acpi_entry_hdr* hdr = MADT->entries;
    struct acpi_madt_lapic* lapicRec;

    for(size_t i = 0; i < MADT->hdr.length - sizeof(struct acpi_madt);){
        if(hdr->type != ACPI_MADT_ENTRY_TYPE_LAPIC){
            i += hdr->length;
            hdr = (struct acpi_entry_hdr*)((uint64_t)hdr + hdr->length);
            continue;
        }

        lapicRec = (struct acpi_madt_lapic*)hdr;


        if(lapicRec->id == lapicID){
            //Match!
            //Now look for the corresponding APIC NMI Structure
            struct acpi_entry_hdr* hdr2 = MADT->entries;
            struct acpi_madt_lapic_nmi* lapicNMIRec;

            for(size_t j = 0; j < MADT->hdr.length - sizeof(struct acpi_madt);){
                if(hdr2->type != ACPI_MADT_ENTRY_TYPE_LAPIC_NMI){
                    j += hdr2->length;
                    hdr2 = (struct acpi_entry_hdr*)((uint64_t)hdr2 + hdr2->length);
                    continue;
                }

                lapicNMIRec = (struct acpi_madt_lapic*)hdr2;


                if(lapicNMIRec->uid == lapicRec->uid || lapicNMIRec->uid == 0xFF){
                    //We found the right entry

                    klog("Found Local APIC NMI Structure for APIC %u.\n", KLOG_INFO, "APIC", lapicRec->id);

                    uint32_t data = 2; //NMI Vector
                    data |= (0b100 << 8); //NMI Delivery Mode

                    uint8_t polarity = lapicNMIRec->flags & 0b11;
                    if(polarity == 0b01) data &= ~(1 << 13);
                    else if(polarity == 0b11) data |= (1 << 13);

                    uint8_t triggerMode = (lapicNMIRec->flags >> 2) & 0b11;
                    if(triggerMode == 0b01) data &= ~(1 << 15);
                    else if(triggerMode == 0b11) data |= (1 << 15);

                    if(lapicNMIRec->lint == 0){
                        apic_write_register(APIC_LVT_LINT0, data);
                    } else if(lapicNMIRec->lint == 1){
                        apic_write_register(APIC_LVT_LINT1, data);
                    }
                }

                j += hdr2->length;
                hdr2 = (struct acpi_entry_hdr*)((uint64_t)hdr2 + hdr2->length);
            }
        }

        i += hdr->length;
        hdr = (struct acpi_entry_hdr*)((uint64_t)hdr + hdr->length);
    }

    //This basically turns on the APIC
    apic_write_register(APIC_SPURIOUS_INT_VECT, 255 | 0x100);

    //APIC Error
    irq_add_handler(223, apic_error_handler, IRQ_SHARED);
    apic_write_register(APIC_LVT_ERROR, 254);

    apic_write_register(APIC_TPR, 0);
}
