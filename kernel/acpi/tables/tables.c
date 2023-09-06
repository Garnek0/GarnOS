#include "tables.h"
#include <limine.h>
#include <sys/panic.h>
#include <mem/memutil/memutil.h>
#include <kstdio.h>
#include <sys/rblogs.h>

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

    for(int i = 0; i < length; i++){
        checksum += ((uint8_t*)ptr)[i];
    }
    checksum &= 0xFF;

    if(checksum != 0) return false;
    else return true;
}

static uint64_t acpi_tables_find(const char* sig){
    int entries = (XSDT->header.length - sizeof(XSDT->header)) / (4 * ACPIVer);
    for(int i = 0; i < entries; i++){
        acpi_sdt_hdr_t* h = (acpi_sdt_hdr_t*)XSDT->tableArea[i*ACPIVer];
        if(!strncmp(h->signature, sig, 4)) return (uint64_t)h;
    }
    return NULL;
}

void acpi_tables_parse(){

    //Mandatory Tables

    RSDP = (acpi_rsdp_t*)rsdp_request.response->address;

    if(!acpi_tables_validate_checksum((uint64_t)RSDP, (ACPI_RSDP_1_SZ + (RSDP->revision*(ACPI_RSDP_2_SZ/2))))){
        klog("Could not Parse ACPI Tables.\n", KLOG_FAILED);
        rb_log("ACPITables", KLOG_FAILED);
        panic("Invalid RSDP or RSDP Pointer!");
    }

    if(RSDP->revision == 0){
        XSDT = (acpi_xsdt_t*)RSDP->RSDTAddress; //use the RSDT instead
        ACPIVer = 1;
    } else if(RSDP->revision == 2){
        XSDT = (acpi_xsdt_t*)RSDP->XSDTAddress;
        ACPIVer = 2;
    } else {
        klog("Could not Parse ACPI Tables.\n", KLOG_FAILED);
        rb_log("ACPITables", KLOG_FAILED);
        panic("Invalid RSDP or RSDP Pointer!");
    }

    if(!acpi_tables_validate_checksum((uint64_t)XSDT, XSDT->header.length)){
        klog("Could not Parse ACPI Tables.\n", KLOG_FAILED);
        rb_log("ACPITables", KLOG_FAILED);
        panic("Invalid RSDT/XSDT or RSDT/XSDT Pointer!");
    }

    klog("Parsed ACPI Tables: ", KLOG_INFO);

    FADT = (acpi_fadt_t*)acpi_tables_find("FACP");
    if(FADT == NULL || !acpi_tables_validate_checksum((uint64_t)FADT, FADT->header.length)){
        kprintf("\n");
        klog("Could not Parse ACPI Tables.\n", KLOG_FAILED);
        rb_log("ACPITables", KLOG_FAILED);
        panic("Invalid FADT or FADT Not Found!");
    }
    kprintf("FADT ");

    MADT = (acpi_madt_t*)acpi_tables_find("APIC");
    if(MADT == NULL || !acpi_tables_validate_checksum((uint64_t)MADT, MADT->header.length)){
        kprintf("\n");
        klog("Could not Parse ACPI Tables.\n", KLOG_FAILED);
        rb_log("ACPITables", KLOG_FAILED);
        panic("Invalid MADT or MADT Not Found!");
    }
    kprintf("MADT ");

    if(FADT->DSDT != 0){
        DSDT = FADT->DSDT;
    } else {
        DSDT = FADT->X_DSDT;
    }
    if(DSDT == NULL || !acpi_tables_validate_checksum((uint64_t)DSDT, DSDT->header.length)){
        kprintf("\n");
        klog("Could not Parse ACPI Tables.\n", KLOG_FAILED);
        rb_log("ACPITables", KLOG_FAILED);
        panic("Invalid DSDT or DSDT Not Found!");
    }
    kprintf("DSDT ");

    //Optional Tables

    BGRT = (acpi_bgrt_t*)acpi_tables_find("BGRT");
    if(BGRT != NULL && !acpi_tables_validate_checksum((uint64_t)BGRT, BGRT->header.length)){
        kprintf("\n");
        klog("Could not Parse ACPI Tables.\n", KLOG_FAILED);
        rb_log("ACPITables", KLOG_FAILED);
        panic("Invalid BGRT Structure!");
    } else if (BGRT != NULL){
        kprintf("BGRT ");
    }

    BERT = (acpi_bert_t*)acpi_tables_find("BERT");
    if(BERT != NULL && !acpi_tables_validate_checksum((uint64_t)BERT, BERT->header.length)){
        kprintf("\n");
        klog("Could not Parse ACPI Tables.\n", KLOG_FAILED);
        rb_log("ACPITables", KLOG_FAILED);
        panic("Invalid BERT Structure!");
    } else if (BERT != NULL){
        kprintf("BERT ");
    }

    //FACS is only optional in some cases.
    //It's here for the sake of simplicity
    FACS = (acpi_facs_t*)acpi_tables_find("FACS");
    if(FACS != NULL && !acpi_tables_validate_checksum((uint64_t)FACS, FACS->length)){
        kprintf("\n");
        klog("Could not Parse ACPI Tables.\n", KLOG_FAILED);
        rb_log("ACPITables", KLOG_FAILED);
        panic("Invalid FACS Structure!");
    } else if (FACS != NULL){
        kprintf("FACS ");
    }

    HPET = (acpi_hpet_t*)acpi_tables_find("HPET");
    if(HPET != NULL && !acpi_tables_validate_checksum((uint64_t)HPET, HPET->header.length)){
        kprintf("\n");
        klog("Could not Parse ACPI Tables.\n", KLOG_FAILED);
        rb_log("ACPITables", KLOG_FAILED);
        panic("Invalid HPET Structure!");
    } else if (HPET != NULL){
        kprintf("HPET ");
    }

    SBST = (acpi_sbst_t*)acpi_tables_find("SBST");
    if(SBST != NULL && !acpi_tables_validate_checksum((uint64_t)SBST, SBST->header.length)){
        kprintf("\n");
        klog("Could not Parse ACPI Tables.\n", KLOG_FAILED);
        rb_log("ACPITables", KLOG_FAILED);
        panic("Invalid SBST Structure!");
    } else if (SBST != NULL){
        kprintf("SBST ");
    }

    MCFG = (acpi_mcfg_t*)acpi_tables_find("MCFG");
    if(MCFG != NULL && !acpi_tables_validate_checksum((uint64_t)MCFG, MCFG->header.length)){
        kprintf("\n");
        klog("Could not Parse ACPI Tables.\n", KLOG_FAILED);
        rb_log("ACPITables", KLOG_FAILED);
        panic("Invalid MCFG Structure!");
    } else if (MCFG != NULL){
        kprintf("MCFG ");
    }

    kprintf("\n");
    klog("ACPI Tables Parsed Successfully.\n", KLOG_OK);
    rb_log("ACPITables", KLOG_OK);
}