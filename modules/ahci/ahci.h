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

#define	SATA_SIG_ATA 0x00000101
#define	SATA_SIG_ATAPI 0xEB140101
#define	SATA_SIG_SEMB 0xC33C0101
#define	SATA_SIG_PM	0x96690101

#define AHCI_ATA 0x1
#define AHCI_ATAPI 0x2
#define AHCI_SEMB 0x3
#define AHCI_PM 0x4

//CAP
#define AHCI_CAP_NCS (1 << 8)
#define AHCI_CAP_S64A (1 << 31)

//EXCAP
#define AHCI_EXCAP_BOH 1

//BOHC
#define AHCI_BOHC_BOS 1
#define AHCI_BOHC_OOS (1 << 1)
#define AHCI_BOHC_SOOE (1 << 2)
#define AHCI_BOHC_OOC (1 << 3)
#define AHCI_BOHC_BB (1 << 4)

//PxCMD
#define AHCI_PxCMD_ST 1
#define AHCI_PxCMD_SUD (1 << 1)
#define AHCI_PxCMD_POD (1 << 2)
#define AHCI_PxCMD_CLO (1 << 3)
#define AHCI_PxCMD_FRE (1 << 4)
#define AHCI_PxCMD_MPSS (1 << 13)
#define AHCI_PxCMD_FR (1 << 14)
#define AHCI_PxCMD_CR (1 << 15)
#define AHCI_PxCMD_CPS (1 << 16)
#define AHCI_PxCMD_PMA (1 << 17)
#define AHCI_PxCMD_HPCB (1 << 18)
#define AHCI_PxCMD_MPSP (1 << 19)
#define AHCI_PxCMD_CPD (1 << 20)
#define AHCI_PxCMD_ESP (1 << 21)
#define AHCI_PxCMD_FBSCB (1 << 22)
#define AHCI_PxCMD_APSTE (1 << 23)
#define AHCI_PxCMD_ATAPI (1 << 24)
#define AHCI_PxCMD_DLAE (1 << 25)
#define AHCI_PxCMD_ALPE (1 << 26)
#define AHCI_PxCMD_ASP (1 << 27)
#define AHCI_PxCMD_ASP (1 << 27)

//PxTFD
#define AHCI_PxTFD_BSY (1 << 7)
#define AHCI_PxTFD_DRQ (1 << 3)

//PxIS
#define AHCI_PxIS_TFES (1 << 30)

//ATA Commands
#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_READ_PIO_EXT 0x24
#define ATA_CMD_READ_DMA 0xC8
#define ATA_CMD_READ_DMA_EXT 0x25
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_WRITE_PIO_EXT 0x34
#define ATA_CMD_WRITE_DMA 0xCA
#define ATA_CMD_WRITE_DMA_EXT 0x35
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET 0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY 0xEC

//ATA IDENT
#define ATA_IDENT_CAPABILITIES 49
#define ATA_IDENT_MODEL 27

#define MAX_PRDS 248

typedef volatile struct {
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

typedef volatile struct {
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

typedef volatile struct {
	uint8_t fisType;
 
	uint8_t pmPort : 4;
	uint8_t reserved1 : 4;
 
	uint16_t reserved2;
 
	uint32_t data[1]; //size varies
}__attribute__((packed)) fis_data_t;

typedef volatile struct {
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

typedef volatile struct {
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
	uint8_t det : 4;
	uint8_t spd : 4;
	uint8_t ipm : 4;
	uint32_t reserved2 : 20;
	uint32_t sctl; // SATA control (SCR2:SControl)
	uint32_t serr; // SATA error (SCR1:SError)
	uint32_t sact; // SATA active (SCR3:SActive)
	uint32_t ci; // command issue
	uint32_t sntf; // SATA notification (SCR4:SNotification)
	uint32_t fbs; // FIS-based switch control
	uint32_t reserved3[11];	// Reserved
	uint32_t vendor[4];	// vendor specific
}__attribute((packed)) ahci_mem_port_t;

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
}__attribute((packed)) ahci_mem_t;

typedef volatile struct {
	uint8_t cfl : 5;
	uint8_t a : 1; // ATAPI
	uint8_t w : 1; // Write, 1: H2D, 0: D2H
	uint8_t p : 1; // Prefetchable
 
	uint8_t r : 1; // Reset
	uint8_t b : 1; // BIST
	uint8_t c : 1;
	uint8_t reserved1 : 1;
	uint8_t pmp : 4; // Port multiplier port
 
	uint16_t prdtLength;
 
	volatile uint32_t prdByteCount;
 
	uint32_t ctba;
	uint32_t ctbau;
 
	uint64_t reserved2[2];
}__attribute((packed)) ahci_command_header_t;

typedef volatile struct {
	uint32_t dba;
	uint32_t dbau;
	uint32_t reserved1;
 
	uint32_t dbc : 22;
	uint16_t reserved2 : 9;
	uint8_t i : 1;
}__attribute((packed)) ahci_prd_t;

typedef volatile struct {
	uint8_t  cfis[64]; // Command FIS
 
	uint8_t  acpiBuf[16]; // ATAPI command, 12 or 16 bytes
 
	uint8_t  reserved[48];
 
	ahci_prd_t prdtEntries[1]; // PRDT entries, 0 ~ 65535
}__attribute((packed)) ahci_cmd_table_t;

typedef volatile struct {
	fis_dma_setup_t	dmaSetupFIS;
	uint8_t padding1[4];
 
	fis_pio_setup_t	pioSetupFIS;
	uint8_t padding2[12];
 
	fis_reg_d2h_t registerD2HFIS;
	uint8_t padding3[4];
 
	uint8_t	setDeviceBitsFIS[8];
 
	uint8_t UnknownFIS[64];

}__attribute((packed)) ahci_recieved_fis_t;

typedef struct {
	ahci_mem_t* abar;
	int maxCommands;
} ahci_controller_t;

typedef struct {
	int type;
	size_t size;
	uint16_t idSpace[256];
	char model[41];

	ahci_controller_t* controller;
	int port;
} ahci_drive_t;

#endif //AHCI_MODULE_H
