/*  
*   File: tables.c
*
*   Author: Garnek
*   
*   Description: Code for finding ACPI tables and setting their respective pointers in the kernel.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <acpi/acpi-internals.h>
#include <garn/acpi/acpi-tables.h>
#include <limine.h>
#include <garn/panic.h>
#include <garn/mm.h>
#include <sys/bootloader.h>
#include <garn/kstdio.h>

uint8_t ACPIVer;
acpi_rsdp_t* RSDP;
acpi_xsdt_t* XSDT;
acpi_fadt_t* FADT;
acpi_madt_t* MADT;
acpi_dsdt_t* DSDT;
acpi_bgrt_t* BGRT;
acpi_bert_t* BERT;
acpi_facs_t* FACS;
acpi_hpet_t* HPET;
acpi_sbst_t* SBST;
acpi_mcfg_t* MCFG;

void acpi_tables_panic(const char* str){
    klog("Could initialise ACPI Table Pointers.\n", KLOG_FAILED, "ACPI");
    panic(str, "ACPI");
}

static bool acpi_tables_validate_checksum(void* ptr, size_t length){
    int checksum = 0;

    //To validate a table's checksum, you must add each byte in the table.
    //(including the checksum byte). The last byte of the result should be 0.
    //if it is != 0, then the table is invalid and it should be assumed to have
    //bogus values. In GarnOS, this causes a kernel panic even if an optional table is invalid
    //because an invalid checksum on any table means something is terribly wrong with the ACPI.
    for(size_t i = 0; i < length; i++){
        checksum += ((uint8_t*)ptr)[i];
    }
    checksum &= 0xFF;

    if(checksum != 0) return false;
    else return true;
}

static uint64_t acpi_tables_find(const char* sig){
    //calculate number of entries using a little trick to determine the correct divisor
    //depending on wether the system is ACPI 1.0 compliant or ACPI 2.0+ compliant.
    int entries = (XSDT->header.length - sizeof(XSDT->header)) / (4 * ACPIVer);

    //look for the table (tableArea is a uint32_t pointer, which for ACPI 1.0's RSDT is not a problem,
    //but since ACPI 2.0's XSDT uses 64-bit pointers, we need to increment the index by two each time)
    for(int i = 0; i < entries; i++){
        acpi_sdt_hdr_t* h = (acpi_sdt_hdr_t*)((XSDT->tableArea)[i*ACPIVer] + bl_get_hhdm_offset());
        if(!strncmp(h->signature, sig, 4)) return (uint64_t)h;
    }
    return 0;
}

char OEMIDBuf[6];
static char* acpi_tables_get_oemid(char* str){
    memcpy((void*)OEMIDBuf, (void*)str, 6);
    OEMIDBuf[5] = 0;
    return OEMIDBuf;
}

void acpi_tables_init(){

    //Store HHDM offset

    uint64_t hhdm = bl_get_hhdm_offset();

    //Start getting tables
    klog("Finding ACPI Tables...\n", KLOG_INFO, "ACPI");

    //Mandatory Tables

    RSDP = (acpi_rsdp_t*)bl_get_rsdp_address();

    //Validate RSDP Checksum
    if(!acpi_tables_validate_checksum((uint64_t)RSDP, RSDP->revision == 2 ? ACPI_RSDP_1_SZ+ACPI_RSDP_2_SZ : ACPI_RSDP_1_SZ)){
        acpi_tables_panic("ACPI: Invalid RSDP or RSDP Pointer!");
    }
    klog("Found RSDP at 0x%x %s\n", KLOG_OK, "ACPI", (uint64_t)RSDP - bl_get_hhdm_offset(), acpi_tables_get_oemid(RSDP->OEMID));

    if(RSDP->revision == 0){
        XSDT = (acpi_xsdt_t*)(RSDP->RSDTAddress + hhdm); //use the RSDT instead
        ACPIVer = 1;
    } else if(RSDP->revision == 2){
        XSDT = (acpi_xsdt_t*)(RSDP->XSDTAddress + hhdm);
        ACPIVer = 2;
    } else {
        acpi_tables_panic("ACPI: Invalid RSDP or RSDP Pointer!");
    }

    if(!acpi_tables_validate_checksum((uint64_t)XSDT, XSDT->header.length)){
        acpi_tables_panic("ACPI: Invalid RSDT/XSDT or RSDT/XSDT Pointer!");
    }

    if(ACPIVer == 1) klog("Found RSDT at 0x%x %s\n", KLOG_OK, "ACPI", (uint64_t)XSDT - bl_get_hhdm_offset(), acpi_tables_get_oemid(XSDT->header.OEMID));
    else klog("Found XSDT at 0x%x %s\n", KLOG_OK, "ACPI", (uint64_t)XSDT - bl_get_hhdm_offset(), acpi_tables_get_oemid(XSDT->header.OEMID));
    

    //FADT, contains information about ACPI fixed registers.
    FADT = (acpi_fadt_t*)acpi_tables_find("FACP");
    if(FADT == NULL || !acpi_tables_validate_checksum((uint64_t)FADT, FADT->header.length)){
        kprintf("\n");
        acpi_tables_panic("ACPI: Invalid FADT or FADT Not Found!");
    }
    klog("Found FADT at 0x%x %s\n", KLOG_OK, "ACPI", (uint64_t)FADT - bl_get_hhdm_offset(), acpi_tables_get_oemid(FADT->header.OEMID));

    //MADT, contains info about APICs, I/O APICs, which interrupts should be set as
    //non-maskable etc.
    MADT = (acpi_madt_t*)acpi_tables_find("APIC");
    if(MADT == NULL || !acpi_tables_validate_checksum((uint64_t)MADT, MADT->header.length)){
        kprintf("\n");
        acpi_tables_panic("ACPI: Invalid MADT or MADT Not Found!");
    }
    klog("Found MADT at 0x%x %s\n", KLOG_OK, "ACPI", (uint64_t)MADT - bl_get_hhdm_offset(), acpi_tables_get_oemid(MADT->header.OEMID));

    //DSDT, used for various power functions.
    if(FADT->DSDT != 0){
        DSDT = (acpi_dsdt_t*)(FADT->DSDT + hhdm);
    } else {
        DSDT = (acpi_dsdt_t*)(FADT->X_DSDT + hhdm);
    }
    if(DSDT == NULL || !acpi_tables_validate_checksum((uint64_t)DSDT, DSDT->header.length)){
        kprintf("\n");
        acpi_tables_panic("ACPI: Invalid DSDT or DSDT Not Found!");
    }
    klog("Found DSDT at 0x%x %s\n", KLOG_OK, "ACPI", (uint64_t)DSDT - bl_get_hhdm_offset(), acpi_tables_get_oemid(DSDT->header.OEMID));

    //I dont really know what the FACS does. I think it has something to do
    //with waking the system up from sleep.
    //FACS is optional in some cases.
    if(FADT->X_FirmwareControl){
        FACS = (acpi_facs_t*)(FADT->X_FirmwareControl + bl_get_hhdm_offset());
    } else if(FADT->firmwareCtrl){
        FACS = (acpi_facs_t*)((uint64_t)FADT->firmwareCtrl + bl_get_hhdm_offset());
    } else {
        FACS = NULL;
    }

    if(FACS == NULL && !(FADT->flags & HARDWARE_REDUCED_ACPI)){
        kprintf("\n");
        acpi_tables_panic("ACPI: Invalid FACS or FACS Not Found! (FACS not optional)");
    } else if (FACS != NULL){
        if(!(FADT->flags & HARDWARE_REDUCED_ACPI)){
            klog("Found FACS at 0x%x\n", KLOG_OK, "ACPI", (uint64_t)FACS - bl_get_hhdm_offset());
        } else {
            klog("Found FACS at 0x%x\n", KLOG_INFO, "ACPI", (uint64_t)FACS - bl_get_hhdm_offset());
        }
        
    }

    //Optional Tables

    //BGRT, contains the OEM logo displayed at boot
    BGRT = (acpi_bgrt_t*)acpi_tables_find("BGRT");
    if(BGRT != NULL && !acpi_tables_validate_checksum((uint64_t)BGRT, BGRT->header.length)){
        kprintf("\n");
        acpi_tables_panic("ACPI: Invalid BGRT Structure!");
    } else if (BGRT != NULL){
        klog("Found BGRT at 0x%x %s\n", KLOG_INFO, "ACPI", (uint64_t)BGRT - bl_get_hhdm_offset(), acpi_tables_get_oemid(BGRT->header.OEMID));
    }

    BERT = (acpi_bert_t*)acpi_tables_find("BERT");
    if(BERT != NULL && !acpi_tables_validate_checksum((uint64_t)BERT, BERT->header.length)){
        kprintf("\n");
        acpi_tables_panic("ACPI: Invalid BERT Structure!");
    } else if (BERT != NULL){
        klog("Found BERT at 0x%x %s\n", KLOG_INFO, "ACPI", (uint64_t)BERT - bl_get_hhdm_offset(), acpi_tables_get_oemid(BERT->header.OEMID));
    }

    //HPET, this is a timer.
    HPET = (acpi_hpet_t*)acpi_tables_find("HPET");
    if(HPET != NULL && !acpi_tables_validate_checksum((uint64_t)HPET, HPET->header.length)){
        kprintf("\n");
        acpi_tables_panic("ACPI: Invalid HPET Structure!");
    } else if (HPET != NULL){
        klog("Found HPET at 0x%x %s\n", KLOG_INFO, "ACPI", (uint64_t)HPET - bl_get_hhdm_offset(), acpi_tables_get_oemid(HPET->header.OEMID));
    }

    //SBST, for devices that have smart batteries. Contains the battery energy
    //levels at which, for example, the OS should warn the user or
    //perform an emergency shutdown.
    SBST = (acpi_sbst_t*)acpi_tables_find("SBST");
    if(SBST != NULL && !acpi_tables_validate_checksum((uint64_t)SBST, SBST->header.length)){
        kprintf("\n");
        acpi_tables_panic("ACPI: Invalid SBST Structure!");
    } else if (SBST != NULL){
        klog("Found SBST at 0x%x %s\n", KLOG_INFO, "ACPI", (uint64_t)SBST - bl_get_hhdm_offset(), acpi_tables_get_oemid(SBST->header.OEMID));
    }

    //MCFG, used for PCI(e) address spaces
    MCFG = (acpi_mcfg_t*)acpi_tables_find("MCFG");
    if(MCFG != NULL && !acpi_tables_validate_checksum((uint64_t)MCFG, MCFG->header.length)){
        kprintf("\n");
        acpi_tables_panic("ACPI: Invalid MCFG Structure!");
    } else if (MCFG != NULL){
        klog("Found MCFG at 0x%x %s\n", KLOG_INFO, "ACPI", (uint64_t)MCFG - bl_get_hhdm_offset(), acpi_tables_get_oemid(MCFG->header.OEMID));
    }

    klog("All supported and available tables found.\n", KLOG_OK, "ACPI");
}