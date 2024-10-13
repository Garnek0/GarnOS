#pragma once

#include <garn/types.h>
#include <garn/dal/dal.h>
#include <garn/hw/pci.h>
#include <garn/kstdio.h>
#include <garn/fal/vnode.h>
#include <garn/arch/common.h>
#include <garn/arch/x86_64.h>
#include <garn/mm.h>
#include <garn/timer.h>
#include <garn/irq.h>
#include <garn/module.h>

#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define ATAPI_CMD_READ       0xA8
#define ATAPI_CMD_EJECT      0x1B

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01
 
#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

#define ATA_PRIMARY      0x00
#define ATA_SECONDARY    0x01
 
#define ATA_READ      0x00
#define ATA_WRITE     0x01

#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D
#define ATA_REG_BMIDE_CMD  0x0E
#define ATA_REG_BMIDE_STAT 0x10
#define ATA_REG_BMIDE_ADDR 0x12

#define IDE_CONTROLLER_PCI_NATIVE_PRIMARY 1
#define IDE_CONTROLLER_MODE_TOGGLEABLE_PRIMARY (1 << 1)
#define IDE_CONTROLLER_PCI_NATIVE_SECONDARY (1 << 2)
#define IDE_CONTROLLER_MODE_TOGGLEABLE_SECONDARY (1 << 3)
#define IDE_CONTROLLER_BUS_MASTERING (1 << 7)

typedef struct {
    uint16_t iobase;
    uint16_t control;
    uint16_t busMastering;
    uint8_t noInt;
} ide_channel_t;

typedef volatile struct {
	uint16_t flags;
	uint16_t unused0[9];
	uint8_t serial[20];
	uint16_t unused1[3];
	uint8_t firmware[8];
	uint8_t model[40];
	uint16_t sectsPerInt;
	uint16_t unused2;
	uint32_t capabilities;
	uint16_t unused3[2];
	uint16_t validExtData;
	uint16_t unused4[5];
	uint16_t sizeOfRWMult;
	uint32_t sectorsLBA28;
	uint16_t unused5[20];
    uint16_t commandSets[6];
    uint16_t unused6[12];
	uint64_t sectorsLBA48;
	uint16_t unused7[152];
}__attribute__((packed)) ide_ata_identify_t;

typedef struct {
    uint8_t ideChannel;
    uint8_t masterSlave;
    uint8_t type; //ATA or ATAPI
    ide_ata_identify_t idSpace;
    size_t size;
    char model[41];

    ide_channel_t* channel;
    drive_t* drive;
} ide_drive_t;

typedef struct {
    ide_drive_t* primaryMaster;
    ide_drive_t* primarySlave;
    ide_drive_t* secondaryMaster;
    ide_drive_t* secondarySlave;
} ide_controller_t;

typedef struct {
    uint32_t addr;
    uint16_t byteCount;
    uint16_t reserved : 15;
    uint8_t eot : 1;
}__attribute__((packed)) ide_prd_t;

void ide_write_reg(ide_channel_t* channel, unsigned char reg, unsigned char data);
uint8_t ide_read_reg(ide_channel_t* channel, unsigned char reg);

uint8_t ide_error(uint8_t error);

uint8_t ide_poll(ide_channel_t* channel, uint8_t reg, uint8_t bit, bool checkErrors);

int ide_ata_read(drive_t* drive, size_t startLBA, size_t blocks, void* buf);
int ide_ata_write(drive_t* drive, size_t startLBA, size_t blocks, void* buf);

int ide_atapi_read(drive_t* drive, size_t startLBA, size_t blocks, void* buf);
int ide_atapi_write(drive_t* drive, size_t startLBA, size_t blocks, void* buf);
