/*  
*   Module: ide.sys
*
*   File: ide-ata-io.c
*
*   Module Author: Garnek
*   
*   Module Description: IDE ATA I/O Functions
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "ide.h"

int ide_ata_read(drive_t* drive, size_t startLBA, size_t blocks, void* buf){
    uint8_t lba_mode, dma, cmd;
    uint8_t lba_io[6];
    ide_drive_t* ideDrive = (ide_drive_t*)drive->context;
    ide_channel_t* channel = ideDrive->channel;
    uint16_t cyl;
    uint8_t head, sect, err;

    if(startLBA >= 0x100000){
        // LBA48:
        lba_mode = 2;
        lba_io[0] = (startLBA & 0x0000000000FF) >> 0;
        lba_io[1] = (startLBA & 0x00000000FF00) >> 8;
        lba_io[2] = (startLBA & 0x000000FF0000) >> 16;
        lba_io[3] = (startLBA & 0x0000FF000000) >> 24;
        lba_io[4] = (startLBA & 0x00FF00000000) >> 32;
        lba_io[5] = (startLBA & 0xFF0000000000) >> 40;
    } else if(ideDrive->idSpace.capabilities & (1 << 9)){ 
        // LBA28:
        lba_mode = 1;
        lba_io[0] = (startLBA & 0x00000FF) >> 0;
        lba_io[1] = (startLBA & 0x000FF00) >> 8;
        lba_io[2] = (startLBA & 0x0FF0000) >> 16;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head = (startLBA & 0xF000000) >> 24;
    } else {
        // CHS:
        lba_mode = 0;
        sect = (startLBA % 63) + 1;
        cyl = (startLBA + 1  - sect) / (16 * 63);
        lba_io[0] = sect;
        lba_io[1] = (cyl >> 0) & 0xFF;
        lba_io[2] = (cyl >> 8) & 0xFF;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head = (startLBA + 1  - sect) % (16 * 63) / (63);
    }

    dma = 0;

    while (ide_read_reg(channel, ATA_REG_STATUS) & ATA_SR_BSY);

    if(lba_mode == 0) ide_write_reg(channel, ATA_REG_HDDEVSEL, 0xA0 | (ideDrive->masterSlave << 4) | head);
    else ide_write_reg(channel, ATA_REG_HDDEVSEL, 0xE0 | (ideDrive->masterSlave << 4) | head);

    if (lba_mode == 2) {
        ide_write_reg(channel, ATA_REG_SECCOUNT1, ((blocks & 0xFF00) >> 8));
        ide_write_reg(channel, ATA_REG_LBA3, lba_io[3]);
        ide_write_reg(channel, ATA_REG_LBA4, lba_io[4]);
        ide_write_reg(channel, ATA_REG_LBA5, lba_io[5]);
    }
    ide_write_reg(channel, ATA_REG_SECCOUNT0, (blocks & 0x00FF));
    ide_write_reg(channel, ATA_REG_LBA0, lba_io[0]);
    ide_write_reg(channel, ATA_REG_LBA1, lba_io[1]);
    ide_write_reg(channel, ATA_REG_LBA2, lba_io[2]);

    if (lba_mode == 0 && dma == 0) cmd = ATA_CMD_READ_PIO;
    if (lba_mode == 1 && dma == 0) cmd = ATA_CMD_READ_PIO;   
    if (lba_mode == 2 && dma == 0) cmd = ATA_CMD_READ_PIO_EXT;   
    if (lba_mode == 0 && dma == 1) cmd = ATA_CMD_READ_DMA;
    if (lba_mode == 1 && dma == 1) cmd = ATA_CMD_READ_DMA;
    if (lba_mode == 2 && dma == 1) cmd = ATA_CMD_READ_DMA_EXT;
    ide_write_reg(channel, ATA_REG_COMMAND, cmd);

    if(dma){
        //TODO: DMA
    } else {
        uint16_t* tmp = buf;
        for(size_t i = 0; i < blocks; i++) {
            if((err = ide_poll(channel, ATA_REG_STATUS, ATA_SR_BSY, true))) return -1;
            for(int j = 0; j < 256; j++){
                tmp[j + 256*i] = arch_inw(channel->iobase + ATA_REG_DATA);
            }
        }
    }

    return 0;
}

int ide_ata_write(drive_t* drive, size_t startLBA, size_t blocks, void* buf){
    uint8_t lba_mode, dma, cmd;
    uint8_t lba_io[6];
    ide_drive_t* ideDrive = (ide_drive_t*)drive->context;
    ide_channel_t* channel = ideDrive->channel;
    uint16_t cyl;
    uint8_t head, sect;

    if(startLBA >= 0x100000){
        // LBA48:
        lba_mode = 2;
        lba_io[0] = (startLBA & 0x0000000000FF) >> 0;
        lba_io[1] = (startLBA & 0x00000000FF00) >> 8;
        lba_io[2] = (startLBA & 0x000000FF0000) >> 16;
        lba_io[3] = (startLBA & 0x0000FF000000) >> 24;
        lba_io[4] = (startLBA & 0x00FF00000000) >> 32;
        lba_io[5] = (startLBA & 0xFF0000000000) >> 40;
    } else if(ideDrive->idSpace.capabilities & (1 << 9)){ 
        // LBA28:
        lba_mode = 1;
        lba_io[0] = (startLBA & 0x00000FF) >> 0;
        lba_io[1] = (startLBA & 0x000FF00) >> 8;
        lba_io[2] = (startLBA & 0x0FF0000) >> 16;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head = (startLBA & 0xF000000) >> 24;
    } else {
        // CHS:
        lba_mode = 0;
        sect = (startLBA % 63) + 1;
        cyl = (startLBA + 1  - sect) / (16 * 63);
        lba_io[0] = sect;
        lba_io[1] = (cyl >> 0) & 0xFF;
        lba_io[2] = (cyl >> 8) & 0xFF;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head = (startLBA + 1  - sect) % (16 * 63) / (63);
    }

    dma = 0;

    while (ide_read_reg(channel, ATA_REG_STATUS) & ATA_SR_BSY);

    if(lba_mode == 0) ide_write_reg(channel, ATA_REG_HDDEVSEL, 0xA0 | (ideDrive->masterSlave << 4) | head);
    else ide_write_reg(channel, ATA_REG_HDDEVSEL, 0xE0 | (ideDrive->masterSlave << 4) | head);

    if (lba_mode == 2) {
        ide_write_reg(channel, ATA_REG_SECCOUNT1, ((blocks & 0xFF00) >> 8));
        ide_write_reg(channel, ATA_REG_LBA3, lba_io[3]);
        ide_write_reg(channel, ATA_REG_LBA4, lba_io[4]);
        ide_write_reg(channel, ATA_REG_LBA5, lba_io[5]);
    }
    ide_write_reg(channel, ATA_REG_SECCOUNT0, (blocks & 0x00FF));
    ide_write_reg(channel, ATA_REG_LBA0, lba_io[0]);
    ide_write_reg(channel, ATA_REG_LBA1, lba_io[1]);
    ide_write_reg(channel, ATA_REG_LBA2, lba_io[2]);

    if (lba_mode == 0 && dma == 0) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 1 && dma == 0) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 2 && dma == 0) cmd = ATA_CMD_WRITE_PIO_EXT;
    if (lba_mode == 0 && dma == 1) cmd = ATA_CMD_WRITE_DMA;
    if (lba_mode == 1 && dma == 1) cmd = ATA_CMD_WRITE_DMA;
    if (lba_mode == 2 && dma == 1) cmd = ATA_CMD_WRITE_DMA_EXT;
    ide_write_reg(channel, ATA_REG_COMMAND, cmd);  

    if(dma){
        //TODO: DMA
    } else {
        uint16_t* tmp = buf;
        for (size_t i = 0; i < blocks; i++) {
            ide_poll(channel, ATA_REG_STATUS, ATA_SR_BSY, false);
            for(int j = 0; j < 256; j++){
                arch_outw(channel->iobase + ATA_REG_DATA, tmp[j + 256*i]);
            }
        }
        ide_write_reg(channel, ATA_REG_COMMAND, (char []){ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]);
        ide_poll(channel, ATA_REG_STATUS, ATA_SR_BSY, false);
    }

    return 0;
}