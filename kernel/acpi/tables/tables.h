/*  
*   File: tables.h
*
*   Author: Garnek
*   
*   Description: Contains typedefs of table structs
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef ACPI_TABLES_H
#define ACPI_TABLES_H

#include <types.h>

//RSDP

#define ACPI_RSDP_1_SZ 20
#define ACPI_RSDP_2_SZ 16

typedef struct {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t RSDTAddress;
    uint32_t length;
    uint64_t XSDTAddress;
    uint8_t exChecksum;
    uint8_t reserved[3];
}__attribute__((packed)) acpi_rsdp_t;
extern acpi_rsdp_t* RSDP;

//SDT Header Structure

typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t creatorID;
    uint32_t creatorRevision;
}__attribute__((packed)) acpi_sdt_hdr_t;

//XSDT

typedef struct {
    acpi_sdt_hdr_t header;
    uint32_t tableArea[];
}__attribute__((packed)) acpi_xsdt_t;
extern acpi_xsdt_t* XSDT;

//GAS Structure

typedef struct {
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
}__attribute__((packed)) acpi_gas_t;

//FADT

#define HARDWARE_REDUCED_ACPI (1 << 20)

typedef struct {
    acpi_sdt_hdr_t header;
    uint32_t firmwareCtrl;
    uint32_t DSDT;
 
    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t  reserved0;
 
    uint8_t preferredPowerManagementProfile;
    uint16_t SCIInterrupt;
    uint32_t SMICommandPort;
    uint8_t ACPIEnable;
    uint8_t ACPIDisable;
    uint8_t S4BIOS_REQ;
    uint8_t PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t PM1EventLength;
    uint8_t PM1ControlLength;
    uint8_t PM2ControlLength;
    uint8_t PMTimerLength;
    uint8_t GPE0Length;
    uint8_t GPE1Length;
    uint8_t GPE1Base;
    uint8_t CStateControl;
    uint16_t worstC2Latency;
    uint16_t worstC3Latency;
    uint16_t flushSize;
    uint16_t flushStride;
    uint8_t dutyOffset;
    uint8_t dutyWidth;
    uint8_t dayAlarm;
    uint8_t monthAlarm;
    uint8_t century;
 
    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t bootArchitectureFlags;
 
    uint8_t reserved1;
    uint32_t flags;
 
    acpi_gas_t resetReg;
 
    uint8_t resetValue;
    uint8_t reserved2[3];
 
    // 64bit pointers - Available on ACPI 2.0+
    uint64_t X_FirmwareControl;
    uint64_t X_DSDT;
 
    acpi_gas_t X_PM1aEventBlock;
    acpi_gas_t X_PM1bEventBlock;
    acpi_gas_t X_PM1aControlBlock;
    acpi_gas_t X_PM1bControlBlock;
    acpi_gas_t X_PM2ControlBlock;
    acpi_gas_t X_PMTimerBlock;
    acpi_gas_t X_GPE0Block;
    acpi_gas_t X_GPE1Block;
}__attribute__((packed)) acpi_fadt_t;
extern acpi_fadt_t* FADT;

//MADT

#define ACPI_MADT_APIC 0x00
#define ACPI_MADT_IOAPIC 0x01
#define ACPI_MADT_INTERRUPT_SOURCE_OVERRIDE 0x02

typedef struct {
    uint8_t entryType;
    uint8_t recordLength;
}__attribute__((packed)) acpi_madt_record_hdr_t;

typedef struct {
    acpi_sdt_hdr_t header;
    uint32_t LAPICAddress;
    uint32_t flags;
    acpi_madt_record_hdr_t MADTRecords;
}__attribute__((packed)) acpi_madt_t;
extern acpi_madt_t* MADT;

typedef struct {
    acpi_madt_record_hdr_t hdr;
    uint8_t acpiID;
    uint8_t apicID;
    uint32_t flags;
}__attribute__((packed)) acpi_madt_record_lapic_t;

typedef struct {
    acpi_madt_record_hdr_t hdr;
    uint8_t ioapicID;
    uint8_t reserved;
    uint32_t ioapicAddress;
    uint32_t gsiBase;
}__attribute__((packed)) acpi_madt_record_ioapic_t;

typedef struct {
    acpi_madt_record_hdr_t hdr;
    uint8_t busSource;
    uint8_t IRQSource;
    uint32_t GSI;
    uint16_t flags;
}__attribute__((packed)) acpi_madt_record_source_override_t;

//BGRT

typedef struct {
    acpi_sdt_hdr_t header;
    uint16_t versionID;
    uint8_t status;
    uint8_t imageType;
    uint64_t imageAddress;
    uint32_t imageYOffset;
    uint32_t imageXOffset;
}__attribute__((packed)) acpi_bgrt_t;
extern acpi_bgrt_t* BGRT;

//BERT

typedef struct {
    acpi_sdt_hdr_t header;
    uint32_t bootErrorRegionLength;
    uint64_t bootErrorRegionAddr;
}__attribute__((packed)) acpi_bert_t;
extern acpi_bert_t* BERT;

//DSDT

typedef struct {
    acpi_sdt_hdr_t header;
    uint8_t definitionBlock[];
}__attribute__((packed)) acpi_dsdt_t;
extern acpi_dsdt_t* DSDT;

//FACS

typedef struct {
    char signature[4];
    uint32_t length;
    uint32_t hardwareSignature;
    uint32_t firmwareWakingVector;
    uint32_t globalLock;
    uint32_t flags;
    uint64_t X_FirmwareWakingVector;
    uint8_t version;
    uint8_t reserved0[3];
    uint32_t OSPMFlags;
    uint64_t reserved1[3];
}__attribute__((packed)) acpi_facs_t;
extern acpi_facs_t* FACS;

//HPET

typedef struct {
    acpi_sdt_hdr_t header;
    uint8_t hardwareRevisionID;
    uint8_t comparatorCount : 5;
    uint8_t counterSize : 1;
    uint8_t reserved : 1;
    uint8_t legacyReplacement : 1;
    uint16_t PCIVendorID;
    acpi_gas_t address;
    uint8_t HPETNumber;
    uint16_t minTick;
    uint8_t pageProtection;
}__attribute__((packed)) acpi_hpet_t;
extern acpi_hpet_t* HPET;

//SBST

typedef struct {
    acpi_sdt_hdr_t header;
    uint32_t warningEnergyLevel;
    uint32_t lowEnergyLevel;
    uint32_t criticalEnergyLevel;
}__attribute__((packed)) acpi_sbst_t;
extern acpi_sbst_t* SBST;

//MCFG

typedef struct {
    uint64_t addr;
    uint16_t PCISegGroup;
    uint8_t startPCIBus;
    uint8_t endPCIBus;
    uint32_t reserved;
}__attribute__((packed)) acpi_mcfg_config_space_t;

typedef struct {
    acpi_sdt_hdr_t header;
    uint64_t reserved;
    acpi_mcfg_config_space_t configSpaces[];
}__attribute__((packed)) acpi_mcfg_t;
extern acpi_mcfg_t* MCFG;

void acpi_tables_parse();

#endif //ACPI_TABLES_H