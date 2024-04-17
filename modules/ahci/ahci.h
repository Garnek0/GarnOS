/*  
*   File: ahci.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef AHCI_MODULE_H
#define AHCI_MODULE_H

#include <types.h>

#define FIS_TYPE_REG_H2D 0x27
#define FIS_TYPE_REG_D2H 0x34
#define FIS_TYPE_DMA_ACT 0x39
#define FIS_TYPE_DMA_SETUP 0x41
#define FIS_TYPE_DATA 0x46
#define FIS_TYPE_BIST 0x58
#define FIS_TYPE_PIO_SETUP 0x5F
#define FIS_TYPE_DEV_BITS 0xA1

typedef struct {
	uint8_t fisType;
	uint8_t pmPort : 4;
	uint8_t reserved1 : 3;
	uint8_t c : 1;

	uint8_t command;
	uint8_t featureLow;

	uint8_t lba0;
	uint8_t lba1;
	uint8_t lba2;
	uint8_t device;
	uint8_t lba3;
	uint8_t lba4;
	uint8_t lba5;
	uint8_t featureHigh;
 
	uint8_t countLow;
	uint8_t countHigh;
	uint8_t icc;
	uint8_t control;
 
	uint32_t reserved2;
}__attribute__((packed)) fis_reg_h2d_t;

typedef struct {
	uint8_t fisType;
	uint8_t pmPort : 4;
	uint8_t reserved1 : 2;
	uint8_t i : 1;
    uint8_t reserved2 : 1;

	uint8_t status;
	uint8_t error;

	uint8_t lba0;
	uint8_t lba1;
	uint8_t lba2;
	uint8_t device;
	uint8_t lba3;
	uint8_t lba4;
	uint8_t lba5;
	uint8_t reserved3;
 
	uint8_t countLow;
	uint8_t countHigh;
	uint16_t reserved4;
 
	uint32_t reserved5;
}__attribute__((packed)) fis_reg_d2h_t;

typedef struct {
	uint8_t fisType;
 
	uint8_t pmPort : 4;
	uint8_t reserved1 : 4;
 
	uint16_t reserved2;
 
	uint32_t data[1]; //size varies
}__attribute__((packed)) fis_data_t;

typedef struct {
	uint8_t fisType;
	uint8_t pmPort : 4;
	uint8_t reserved1 : 1;
	uint8_t d : 1;
	uint8_t i : 1;
	uint8_t reserved2 : 1;

	uint8_t status;
	uint8_t error;

	uint8_t lba0;
	uint8_t lba1;
	uint8_t lba2;
	uint8_t device;
	uint8_t lba3;
	uint8_t lba4;
	uint8_t lba5;
	uint8_t reserved3;
 
	uint8_t countLow;
	uint8_t countHigh;
	uint8_t reserved4;
	uint8_t newStatus;
 
    uint16_t transferCount;
	uint16_t reserved5;
}__attribute__((packed)) fis_pio_setup_t;

typedef struct {
	uint8_t fisType;
	uint8_t pmPort : 4;
	uint8_t reserved1 : 1;
	uint8_t d : 1;
	uint8_t i : 1;
	uint8_t a : 1;

	uint16_t reserved2;

	uint64_t dmaBufID;
    uint32_t reserved3;
    uint32_t dmaBufOffset;
    uint32_t transferCount;
    uint32_t reserved4;
}__attribute__((packed)) fis_dma_setup_t;

typedef volatile struct {
    uint32_t clb; // command list base address, 1K-byte aligned
	uint32_t clbu; // command list base address upper 32 bits
	uint32_t fb; // FIS base address, 256-byte aligned
	uint32_t fbu; // FIS base address upper 32 bits
	uint32_t is; // interrupt status
	uint32_t ie; // interrupt enable
	uint32_t cmd; // command and status
	uint32_t reserved1; // Reserved
	uint32_t tfd; // task file data
	uint32_t sig; // signature
	uint32_t ssts; // SATA status (SCR0:SStatus)
	uint32_t sctl; // SATA control (SCR2:SControl)
	uint32_t serr; // SATA error (SCR1:SError)
	uint32_t sact; // SATA active (SCR3:SActive)
	uint32_t ci; // command issue
	uint32_t sntf; // SATA notification (SCR4:SNotification)
	uint32_t fbs; // FIS-based switch control
	uint32_t reserved2[11];	// Reserved
	uint32_t vendor[4];	// vendor specific
} ahci_mem_port_t;

typedef volatile struct {
    uint32_t cap1; // Host capabilities
	uint32_t ghc; // Global host control
	uint32_t is; // Interrupt status
	uint32_t pi; // Port implemented
	uint32_t vs; // Version
	uint32_t cccControl; // Command completion coalescing control
	uint32_t cccPorts; // Command completion coalescing ports
	uint32_t emLocation; // Enclosure management location
	uint32_t emControl; // Enclosure management control
	uint32_t cap2; // Host capabilities extended
	uint32_t bohc; // BIOS/OS handoff control and status

    uint8_t reserved[0xA0-0x2C];

    uint8_t vendorSpecific[0x100-0xA0];

    ahci_mem_port_t ports[32];
} achi_mem_t;

#endif //AHCI_MODULE_H