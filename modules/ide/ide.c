/*  
*   Module: ata.mod
*
*   File: ata.c
*
*   Module Author: Garnek
*   
*   Mdoule Description: ATA Controller Driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "ide.h"
#include <module/module.h>
#include <sys/dal/dal.h>
#include <hw/pci/pci.h>
#include <kstdio.h>
#include <sys/fal/fal.h>
#include <mem/mm/kheap.h>
#include <hw/ports.h>
#include <mem/memutil/memutil.h>
#include <sys/timer.h>

typedef struct {
    uint16_t iobase;
    uint16_t control;
    uint16_t busMastering;
    uint8_t noInt;
} ide_channel_t;
typedef struct {
    uint8_t ideChannel;
    uint8_t masterSlave;
    uint8_t type; //ATA or ATAPI
    uint16_t idSpace[256];
    size_t size;
    char model[41];

    ide_channel_t* channel;
} ide_drive_t;

uint8_t ide_read(ide_channel_t* channel, unsigned char reg){
    uint8_t result;
    if(reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channel->noInt);
    if(reg < 0x08)
        result = inb(channel->iobase + reg - 0x00);
    else if(reg < 0x0C)
        result = inb(channel->iobase  + reg - 0x06);
    else if(reg < 0x0E)
        result = inb(channel->control  + reg - 0x0A);
    else if(reg < 0x16)
        result = inb(channel->busMastering + reg - 0x0E);
    if(reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, channel->noInt);
    return result;
}

void ide_write(ide_channel_t* channel, unsigned char reg, unsigned char data){
    if(reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, 0x80 | channel->noInt);
    if(reg < 0x08)
        outb(channel->iobase  + reg - 0x00, data);
    else if(reg < 0x0C)
        outb(channel->iobase  + reg - 0x06, data);
    else if(reg < 0x0E)
        outb(channel->control  + reg - 0x0A, data);
    else if(reg < 0x16)
        outb(channel->busMastering + reg - 0x0E, data);
    if(reg > 0x07 && reg < 0x0C)
        ide_write(channel, ATA_REG_CONTROL, channel->noInt);
}



uint8_t ide_error(uint8_t error){
    if(error == 0) return 0;

    kprintf("IDE: ");

    if(error & ATA_ER_AMNF) kprintf("Address Mark Not Found!\n");
    if(error & ATA_ER_TK0NF) kprintf("Track 0 Not Found!\n");
    if(error & ATA_ER_ABRT) kprintf("Command Aborted!\n");
    if(error & ATA_ER_MCR) kprintf("Media Change Request!\n");
    if(error & ATA_ER_IDNF) kprintf("ID Mark Not Found!\n");
    if(error & ATA_ER_MC) kprintf("Media Changed!\n");
    if(error & ATA_ER_UNC) kprintf("Uncorrectable Data!\n");
    if(error & ATA_ER_BBK) kprintf("Bad Block!\n");

    return error;
    
}

uint8_t ide_poll(ide_channel_t* channel, uint8_t reg, uint8_t bit, bool checkErrors){
    ksleep(1);

    while(ide_read(channel, reg) & bit);

    uint8_t err = ide_error(ide_read(channel, ATA_REG_ERROR));

    if(checkErrors){
        if(ide_read(channel, ATA_REG_STATUS) & ATA_SR_ERR) err;
    }

    return err;
}

void ide_ata_read(drive_t* drive, size_t startLBA, size_t size, void* buf){
    size = ALIGN_UP(size, 512);

    uint8_t lba_mode, dma, cmd;
    uint8_t lba_io[6];
    ide_drive_t* ideDrive = (ide_drive_t*)drive->context;
    ide_channel_t* channel = ideDrive->channel;
    uint16_t cyl;
    uint8_t head, sect, err;

    size_t seccount = size/512;

    if(startLBA >= 0x10000000){
        // LBA48:
        lba_mode = 2;
        lba_io[0] = (startLBA & 0x0000000000FF) >> 0;
        lba_io[1] = (startLBA & 0x00000000FF00) >> 8;
        lba_io[2] = (startLBA & 0x000000FF0000) >> 16;
        lba_io[3] = (startLBA & 0x0000FF000000) >> 24;
        lba_io[4] = (startLBA & 0x00FF00000000) >> 32;
        lba_io[5] = (startLBA & 0xFF0000000000) >> 40;
    } else if(ideDrive->idSpace[ATA_IDENT_CAPABILITIES] & (1 << 9)){ 
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

    //TODO: Add DMA Support
    dma = 0;

    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY);

    if(lba_mode == 0) ide_write(channel, ATA_REG_HDDEVSEL, 0xA0 | (ideDrive->masterSlave << 4) | head);
    else ide_write(channel, ATA_REG_HDDEVSEL, 0xE0 | (ideDrive->masterSlave << 4) | head);

    if (lba_mode == 2) {
        ide_write(channel, ATA_REG_SECCOUNT1, ((seccount & 0xFF00) >> 8));
        ide_write(channel, ATA_REG_LBA3, lba_io[3]);
        ide_write(channel, ATA_REG_LBA4, lba_io[4]);
        ide_write(channel, ATA_REG_LBA5, lba_io[5]);
    }
    ide_write(channel, ATA_REG_SECCOUNT0, (seccount & 0x00FF));
    ide_write(channel, ATA_REG_LBA0, lba_io[0]);
    ide_write(channel, ATA_REG_LBA1, lba_io[1]);
    ide_write(channel, ATA_REG_LBA2, lba_io[2]);

    if (lba_mode == 0 && dma == 0) cmd = ATA_CMD_READ_PIO;
    if (lba_mode == 1 && dma == 0) cmd = ATA_CMD_READ_PIO;   
    if (lba_mode == 2 && dma == 0) cmd = ATA_CMD_READ_PIO_EXT;   
    if (lba_mode == 0 && dma == 1) cmd = ATA_CMD_READ_DMA;
    if (lba_mode == 1 && dma == 1) cmd = ATA_CMD_READ_DMA;
    if (lba_mode == 2 && dma == 1) cmd = ATA_CMD_READ_DMA_EXT;
    ide_write(channel, ATA_REG_COMMAND, cmd);

    if(dma){
        //TODO: DMA
    } else {
        uint16_t* tmp = buf;
        for(int i = 0; i < seccount; i++) {
            if(err = ide_poll(channel, ATA_REG_STATUS, ATA_SR_BSY, true)) return;
            for(int j = 0; j < 256; j++){
                tmp[j] = inw(channel->iobase + ATA_REG_DATA);
            }
        }
    }
}

void ide_ata_write(drive_t* drive, size_t startLBA, size_t size, void* buf){
    size = ALIGN_UP(size, 512);

    uint8_t lba_mode, dma, cmd;
    uint8_t lba_io[6];
    ide_drive_t* ideDrive = (ide_drive_t*)drive->context;
    ide_channel_t* channel = ideDrive->channel;
    uint16_t cyl;
    uint8_t head, sect, err;

    size_t seccount = size/512;

    if(startLBA >= 0x10000000){
        // LBA48:
        lba_mode = 2;
        lba_io[0] = (startLBA & 0x0000000000FF) >> 0;
        lba_io[1] = (startLBA & 0x00000000FF00) >> 8;
        lba_io[2] = (startLBA & 0x000000FF0000) >> 16;
        lba_io[3] = (startLBA & 0x0000FF000000) >> 24;
        lba_io[4] = (startLBA & 0x00FF00000000) >> 32;
        lba_io[5] = (startLBA & 0xFF0000000000) >> 40;
    } else if(ideDrive->idSpace[ATA_IDENT_CAPABILITIES] & (1 << 9)){ 
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

    //TODO: Add DMA Support
    dma = 0;

    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY);

    if(lba_mode == 0) ide_write(channel, ATA_REG_HDDEVSEL, 0xA0 | (ideDrive->masterSlave << 4) | head);
    else ide_write(channel, ATA_REG_HDDEVSEL, 0xE0 | (ideDrive->masterSlave << 4) | head);

    if (lba_mode == 2) {
        ide_write(channel, ATA_REG_SECCOUNT1, ((seccount & 0xFF00) >> 8));
        ide_write(channel, ATA_REG_LBA3, lba_io[3]);
        ide_write(channel, ATA_REG_LBA4, lba_io[4]);
        ide_write(channel, ATA_REG_LBA5, lba_io[5]);
    }
    ide_write(channel, ATA_REG_SECCOUNT0, (seccount & 0x00FF));
    ide_write(channel, ATA_REG_LBA0, lba_io[0]);
    ide_write(channel, ATA_REG_LBA1, lba_io[1]);
    ide_write(channel, ATA_REG_LBA2, lba_io[2]);

    if (lba_mode == 0 && dma == 0) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 1 && dma == 0) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 2 && dma == 0) cmd = ATA_CMD_WRITE_PIO_EXT;
    if (lba_mode == 0 && dma == 1) cmd = ATA_CMD_WRITE_DMA;
    if (lba_mode == 1 && dma == 1) cmd = ATA_CMD_WRITE_DMA;
    if (lba_mode == 2 && dma == 1) cmd = ATA_CMD_WRITE_DMA_EXT;
    ide_write(channel, ATA_REG_COMMAND, cmd);  

    if(dma){
        //TODO: DMA
    } else {
        uint16_t* tmp = buf;
        for (int i = 0; i < seccount; i++) {
            ide_poll(channel, ATA_REG_STATUS, ATA_SR_BSY, false);
            for(int j = 0; j < 256; j++){
                outw(channel->iobase + ATA_REG_DATA, tmp[j]);
            }
        }
        ide_write(channel, ATA_REG_COMMAND, (char []){ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]);
        ide_poll(channel, ATA_REG_STATUS, ATA_SR_BSY, false);
    }
}

//TODO: ATAPI

void ide_atapi_read(drive_t* drive, size_t startLBA, size_t size, void* buf){
    ;
}

void ide_atapi_write(drive_t* drive, size_t startLBA, size_t size, void* buf){
    ;
}

void init(){
    return;
}

void fini(){
    return;
}

bool probe(device_t* device){
    pci_config_device_t* pciConfig;
    pciConfig = (pci_config_device_t*)device->data;

    if(device->bus != DEVICE_BUS_PCI || pciConfig->hdr.class != PCI_CLASS_STORAGE_CONTROLLER || pciConfig->hdr.subclass != PCI_SUBCLASS_IDE){
        return false;
    }
    return true;
}

drive_t drive;

bool attach(device_t* device){
    if(!probe(device)) return false;

    pci_config_device_t* pciConfig;
    pciConfig = (pci_config_device_t*)device->data;

    ide_channel_t* channelPrimary = kmalloc(sizeof(ide_channel_t));
    memset(channelPrimary, 0, sizeof(ide_channel_t));
    ide_channel_t* channelSecondary = kmalloc(sizeof(ide_channel_t));
    memset(channelSecondary, 0, sizeof(ide_channel_t));

    ide_drive_t* primaryMaster = kmalloc(sizeof(ide_drive_t));
    memset(primaryMaster, 0, sizeof(ide_drive_t));
    ide_drive_t* primarySlave = kmalloc(sizeof(ide_drive_t));
    memset(primarySlave, 0, sizeof(ide_drive_t));
    ide_drive_t* secondaryMaster = kmalloc(sizeof(ide_drive_t));
    memset(secondaryMaster, 0, sizeof(ide_drive_t));
    ide_drive_t* secondarySlave = kmalloc(sizeof(ide_drive_t));
    memset(secondarySlave, 0, sizeof(ide_drive_t));

    primaryMaster->ideChannel = ATA_PRIMARY;
    primaryMaster->masterSlave = ATA_MASTER;
    primarySlave->ideChannel = ATA_PRIMARY;
    primarySlave->masterSlave = ATA_SLAVE;

    secondaryMaster->ideChannel = ATA_SECONDARY;
    secondaryMaster->masterSlave = ATA_MASTER;
    secondarySlave->ideChannel = ATA_SECONDARY;
    secondarySlave->masterSlave = ATA_SLAVE;

    if(pciConfig->hdr.progIF & IDE_CONTROLLER_PCI_NATIVE_PRIMARY){
        channelPrimary->iobase = pciConfig->BAR0;
        channelPrimary->control = pciConfig->BAR1;
        if(pciConfig->hdr.progIF & IDE_CONTROLLER_BUS_MASTERING){
            channelPrimary->busMastering = pciConfig->BAR4;
        } else {
            channelPrimary->busMastering = 0;
        }
    } else {
        channelPrimary->iobase = 0x1F0;
        channelPrimary->control = 0x3F6;
    }

    if(pciConfig->hdr.progIF & IDE_CONTROLLER_PCI_NATIVE_SECONDARY){
        channelSecondary->iobase = pciConfig->BAR2;
        channelSecondary->control = pciConfig->BAR3;
        if(pciConfig->hdr.progIF & IDE_CONTROLLER_BUS_MASTERING){
            channelSecondary->busMastering = pciConfig->BAR4 + 8;
        } else {
            channelSecondary->busMastering = 0;
        }
    } else {
        channelSecondary->iobase = 0x170;
        channelSecondary->control = 0x376;
    }

    primaryMaster->channel = channelPrimary;
    primarySlave->channel = channelPrimary;
    secondaryMaster->channel = channelSecondary;
    secondarySlave->channel = channelSecondary;

    ide_channel_t* currentChannel;
    ide_drive_t* currentDrive;

    ide_write(channelPrimary, ATA_REG_CONTROL, 2);
    ide_write(channelSecondary, ATA_REG_CONTROL, 2);

    bool error = false;
    char* tmp;

    for(uint8_t i = 0; i < 2; i++){
        if(i == 0){
            currentChannel = channelPrimary;
        } else {
            currentChannel = channelSecondary;
        }

        for(uint8_t j = 0; j < 2; j++){
            if(i == 0){
                if(j == 0){
                    currentDrive = primaryMaster;
                } else {
                    currentDrive = primarySlave;
                }
            } else {
                if(j == 0){
                    currentDrive = secondaryMaster;
                } else {
                    currentDrive = secondarySlave;
                }
            }

            // select drive
            ide_write(currentChannel, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
            ksleep(10);

            ide_write(currentChannel, ATA_REG_SECCOUNT0, 0);
            ide_write(currentChannel, ATA_REG_LBA0, 0);
            ide_write(currentChannel, ATA_REG_LBA1, 0);
            ide_write(currentChannel, ATA_REG_LBA2, 0);

            //identify drive
            ide_write(currentChannel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            ksleep(10);

            if(ide_read(currentChannel, ATA_REG_STATUS) == 0 || ide_read(currentChannel, ATA_REG_STATUS) == 0x7F || ide_read(currentChannel, ATA_REG_STATUS) == 0xFF) continue; //no drive

            currentDrive->type = IDE_ATA;

            if(ide_read(currentChannel, ATA_REG_LBA1) != 0 || ide_read(currentChannel, ATA_REG_LBA2) != 0){
                currentDrive->type = IDE_ATAPI;

                //identify ATAPI drive
                ide_write(currentChannel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                ksleep(10);
            }

            error = false;

            while(!ide_read(currentChannel, ATA_REG_STATUS) & ATA_SR_DRQ){
                if(ide_read(currentChannel, ATA_REG_STATUS) & ATA_SR_ERR){
                    ide_error(ide_read(currentChannel, ATA_REG_ERROR));
                    error = true;
                    break;
                }
            }

            if(error) continue;

            for(uint16_t k = 0; k < 256; k++){
                currentDrive->idSpace[k] = inw(currentChannel->iobase);
            }
            
            if(currentDrive->idSpace[ATA_IDENT_COMMANDSETS+1] & (1 << 10)) currentDrive->size = *((uint64_t*)(currentDrive->idSpace + ATA_IDENT_MAX_LBA_EXT));
            else currentDrive->size = *((uint32_t*)(currentDrive->idSpace + ATA_IDENT_MAX_LBA));

            currentDrive->size *= 512;

            tmp = &currentDrive->idSpace[ATA_IDENT_MODEL];

            for(uint8_t k = 0; k < 40; k+=2){
                currentDrive->model[k] = tmp[k + 1];
                currentDrive->model[k + 1] = tmp[k];
            }
            currentDrive->model[40] = 0;

            for(uint8_t k = 39; k > 0; k--){
                if(currentDrive->model[k] == 0x20) currentDrive->model[k] = 0;
                else break;
            }

            drive.context = currentDrive;
            drive.interface = DRIVE_IF_IDE;
            drive.name = currentDrive->model;
            drive.size = currentDrive->size;
            drive.blockSize = 512;

            if(currentDrive->type == IDE_ATA){
                drive.read = ide_ata_read;
                drive.write = ide_ata_write;
                drive.type = DRIVE_TYPE_DISK;
            } else {
                drive.read = ide_atapi_read;
                drive.write = ide_atapi_write;
                drive.type = DRIVE_TYPE_OPTICAL;
            }
            
            drive_add(drive);
        }
    }

    klog("IDE: Initialised Controller! (BAR0: 0x%x, BAR1: 0x%x, BAR2: 0x%x, BAR3: 0x%x, BAR4: 0x%x)\n", KLOG_OK, channelPrimary->iobase, channelPrimary->control, channelSecondary->iobase, channelSecondary->control, pciConfig->BAR4);

    return true;
}

module_t metadata = {
    .name = "ide",
    .init = init,
    .fini = fini
};

device_driver_t driver_metadata = {
    .probe = probe,
    .attach = attach
};
