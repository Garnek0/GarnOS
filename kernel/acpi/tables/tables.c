/*  
*   File: tables.c
*
*   Author: Garnek
*   
*   Description: Parsing of ACPI Tables
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "tables.h"
#include <limine.h>
#include <sys/panic.h>
#include <mem/memutil/memutil.h>
#include <sys/bootloader.h>
#include <kstdio.h>

//TODO: move to sys/bootloader.c
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

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


//returns true if acpi table checksum is valid.
static bool acpi_tables_validate_checksum(uint64_t ptr, size_t length){
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

//look through the XSDT/RSDT for the table with the signature (sig).
static uint64_t acpi_tables_find(const char* sig){
    //calculate number of entries using a little trick to determine the correct divisor
    //depending on wether the system is ACPI 1.0 compliant or ACPI 2.0+ compliant.
    int entries = (XSDT->header.length - sizeof(XSDT->header)) / (4 * ACPIVer);

    //look for the table (another trick is used here. tableArea is a uint32_t pointer,
    //which for ACPI 1.0's RSDT is not a problem, but since ACPI 2.0's XSDT uses 64-bit pointers,
    //we need to increment the index by two each time)
    for(int i = 0; i < entries; i++){
        acpi_sdt_hdr_t* h = (acpi_sdt_hdr_t*)((XSDT->tableArea)[i*ACPIVer] + bl_get_hhdm_offset());
        if(!strncmp(h->signature, sig, 4)) return (uint64_t)h;
    }
    return 0;
}

void acpi_tables_parse(){

    //Store HHDM offset

    uint64_t hhdm = bl_get_hhdm_offset();

    //Mandatory Tables

    RSDP = (acpi_rsdp_t*)rsdp_request.response->address;

    //there is definitely a better way to determine the size of the RSDP...
    if(!acpi_tables_validate_checksum((uint64_t)RSDP, (ACPI_RSDP_1_SZ + (RSDP->revision*(ACPI_RSDP_2_SZ/2))))){
        klog("ACPI: Could not Parse ACPI Tables.\n", KLOG_FAILED);
        panic("ACPI: Invalid RSDP or RSDP Pointer!");
    }

    if(RSDP->revision == 0){
        XSDT = (acpi_xsdt_t*)(RSDP->RSDTAddress + hhdm); //use the RSDT instead
        ACPIVer = 1;
    } else if(RSDP->revision == 2){
        XSDT = (acpi_xsdt_t*)(RSDP->XSDTAddress + hhdm);
        ACPIVer = 2;
    } else {
        klog("ACPI: Could not Parse ACPI Tables.\n", KLOG_FAILED);
        panic("ACPI: Invalid RSDP or RSDP Pointer!");
    }

    if(!acpi_tables_validate_checksum((uint64_t)XSDT, XSDT->header.length)){
        klog("ACPI: Could not Parse ACPI Tables.\n", KLOG_FAILED);
        panic("ACPI: Invalid RSDT/XSDT or RSDT/XSDT Pointer!");
    }

    //Start actually parsing tables
    klog("ACPI: Parsed ACPI Tables: ", KLOG_INFO);

    //FADT, probably the most important one
    FADT = (acpi_fadt_t*)acpi_tables_find("FACP");
    if(FADT == NULL || !acpi_tables_validate_checksum((uint64_t)FADT, FADT->header.length)){
        kprintf("\n");
        klog("ACPI: Could not Parse ACPI Tables.\n", KLOG_FAILED);
        panic("ACPI: Invalid FADT or FADT Not Found!");
    }
    kprintf("FADT ");

    //MADT, probably the second most inportant one
    MADT = (acpi_madt_t*)acpi_tables_find("APIC");
    if(MADT == NULL || !acpi_tables_validate_checksum((uint64_t)MADT, MADT->header.length)){
        kprintf("\n");
        klog("ACPI: Could not Parse ACPI Tables.\n", KLOG_FAILED);
        panic("ACPI: Invalid MADT or MADT Not Found!");
    }
    kprintf("MADT ");

    //DSDT, probably this won't be very useful for long while. Same goes for some other tables.
    if(FADT->DSDT != 0){
        DSDT = (acpi_dsdt_t*)(FADT->DSDT + hhdm);
    } else {
        DSDT = (acpi_dsdt_t*)(FADT->X_DSDT + hhdm);
    }
    if(DSDT == NULL || !acpi_tables_validate_checksum((uint64_t)DSDT, DSDT->header.length)){
        kprintf("\n");
        klog("ACPI: Could not Parse ACPI Tables.\n", KLOG_FAILED);
        panic("ACPI: Invalid DSDT or DSDT Not Found!");
    }
    kprintf("DSDT ");

    //Optional Tables

    //BGRT, contains the OEM logo displayed at boot
    BGRT = (acpi_bgrt_t*)acpi_tables_find("BGRT");
    if(BGRT != NULL && !acpi_tables_validate_checksum((uint64_t)BGRT, BGRT->header.length)){
        kprintf("\n");
        klog("ACPI: Could not Parse ACPI Tables.\n", KLOG_FAILED);
        panic("ACPI: Invalid BGRT Structure!");
    } else if (BGRT != NULL){
        kprintf("BGRT ");
    }

    BERT = (acpi_bert_t*)acpi_tables_find("BERT");
    if(BERT != NULL && !acpi_tables_validate_checksum((uint64_t)BERT, BERT->header.length)){
        kprintf("\n");
        klog("ACPI: Could not Parse ACPI Tables.\n", KLOG_FAILED);
        panic("ACPI: Invalid BERT Structure!");
    } else if (BERT != NULL){
        kprintf("BERT ");
    }

    //FACS is only optional in some cases.
    //It's here for the sake of simplicity
    FACS = (acpi_facs_t*)acpi_tables_find("FACS");
    if(FACS != NULL && !acpi_tables_validate_checksum((uint64_t)FACS, FACS->length)){
        kprintf("\n");
        klog("ACPI: Could not Parse ACPI Tables.\n", KLOG_FAILED);
        panic("ACPI: Invalid FACS Structure!");
    } else if (FACS != NULL){
        kprintf("FACS ");
    }

    //HPET, this is a timer.
    HPET = (acpi_hpet_t*)acpi_tables_find("HPET");
    if(HPET != NULL && !acpi_tables_validate_checksum((uint64_t)HPET, HPET->header.length)){
        kprintf("\n");
        klog("ACPI: Could not Parse ACPI Tables.\n", KLOG_FAILED);
        panic("ACPI: Invalid HPET Structure!");
    } else if (HPET != NULL){
        kprintf("HPET ");
    }

    //SBST, for devices that have batteries. contains information about things such as
    //when OSPM should notify the user that their battery is at a critical level
    SBST = (acpi_sbst_t*)acpi_tables_find("SBST");
    if(SBST != NULL && !acpi_tables_validate_checksum((uint64_t)SBST, SBST->header.length)){
        kprintf("\n");
        klog("ACPI: Could not Parse ACPI Tables.\n", KLOG_FAILED);
        panic("ACPI: Invalid SBST Structure!");
    } else if (SBST != NULL){
        kprintf("SBST ");
    }

    //MCFG, used for PCI(e) address spaces
    MCFG = (acpi_mcfg_t*)acpi_tables_find("MCFG");
    if(MCFG != NULL && !acpi_tables_validate_checksum((uint64_t)MCFG, MCFG->header.length)){
        kprintf("\n");
        klog("ACPI: Could not Parse ACPI Tables.\n", KLOG_FAILED);
        panic("ACPI: Invalid MCFG Structure!");
    } else if (MCFG != NULL){
        kprintf("MCFG ");
    }

    kprintf("\n");
    klog("ACPI: ACPI Tables Parsed Successfully.\n", KLOG_OK);
}