/*  
*   Module: ahci.sys
*
*   File: ahci-ata-io.c
*
*   Module Author: Garnek
*   
*   Module Description: AHCI ATA I/O Control
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "ahci.h"

int ahci_ata_read(drive_t* drive, size_t startLBA, size_t blocks, void* buf){
    //prdtLength would never go over MAX_PRDS with this restriction
    if(blocks > 0xFFFF) return -EINVAL;

    size_t count = blocks;

    ahci_drive_t* ahciDrive = (ahci_drive_t*)drive->context;
    ahci_controller_t* ahciController = ahciDrive->controller;
    ahci_mem_t* abar = ahciController->abar;
    ahci_mem_port_t* port = &abar->ports[ahciDrive->port];

    port->serr = 0xFFFFFFFF;
    port->is = 0xFFFFFFFF; 

    int freeCMDSlot = ahci_find_cmd_slot(ahciController, port);
    if(freeCMDSlot < 0) return -1;

    ahci_command_header_t* commandList = (ahci_command_header_t*)(((uint64_t)port->clb | (uint64_t)port->clbu << 32) + hhdmOffset);

    ahci_command_header_t* cmdHeader = &commandList[freeCMDSlot];
    ahci_cmd_table_t* cmdTable = (ahci_cmd_table_t*)(((uint64_t)cmdHeader->ctba | (uint64_t)cmdHeader->ctbau << 32) + hhdmOffset);

    cmdHeader->cfl = sizeof(fis_reg_h2d_t)/sizeof(uint32_t);
    cmdHeader->c = 1;
    cmdHeader->p = 0;
    cmdHeader->w = 0;
    cmdHeader->prdtLength = (uint16_t)((blocks-1)/0x400000)+1;

    memset((void*)cmdTable, 0, sizeof(ahci_cmd_table_t)+(MAX_PRDS-1)*sizeof(ahci_prd_t));

    //Maybe this is not the best way to do DMA...
    void* bufContinuous;

    if(!(abar->cap1 & AHCI_CAP_S64A)){
        bufContinuous = pmm_allocate32((blocks*512/PAGE_SIZE)+1);
    } else {
        bufContinuous = pmm_allocate((blocks*512/PAGE_SIZE)+1);
    }

    int i = 0;
    int bufContOffset = 0;
    for(; i < cmdHeader->prdtLength-1; i++){
        if(!(abar->cap1 & AHCI_CAP_S64A)){
            cmdTable->prdtEntries[i].dba = (uint32_t)((uint64_t)bufContinuous + bufContOffset);
            cmdTable->prdtEntries[i].dbau = 0;
        } else {
            cmdTable->prdtEntries[i].dba = (uint32_t)((uint64_t)(bufContinuous + bufContOffset) & 0x00000000FFFFFFFF);
            cmdTable->prdtEntries[i].dbau = (uint32_t)(((uint64_t)(bufContinuous + bufContOffset) & 0xFFFFFFFF00000000) >> 32);
        }
        cmdTable->prdtEntries[i].dbc = 0x3FFFFF;
        cmdTable->prdtEntries[i].i = 0;
        bufContOffset += 0x400000;
        blocks -= 0x2000;
    }

    if(!(abar->cap1 & AHCI_CAP_S64A)){
        cmdTable->prdtEntries[i].dba = (uint32_t)((uint64_t)bufContinuous + bufContOffset);
        cmdTable->prdtEntries[i].dbau = 0;
    } else {
        cmdTable->prdtEntries[i].dba = (uint32_t)((uint64_t)(bufContinuous + bufContOffset) & 0x00000000FFFFFFFF);
        cmdTable->prdtEntries[i].dbau = (uint32_t)(((uint64_t)(bufContinuous + bufContOffset) & 0xFFFFFFFF00000000) >> 32);
    }
    cmdTable->prdtEntries[i].dbc = blocks*512-1;
	cmdTable->prdtEntries[i].i = 0;

    fis_reg_h2d_t* cmdFIS = (fis_reg_h2d_t*)(&cmdTable->cfis);
    memset((void*)cmdFIS, 0, 64);
    cmdFIS->fisType = FIS_TYPE_REG_H2D;
    cmdFIS->c = 1;
    if(startLBA >= 0x100000){
        // LBA48:
        cmdFIS->lba0 = (startLBA & 0x0000000000FF) >> 0;
        cmdFIS->lba1 = (startLBA & 0x00000000FF00) >> 8;
        cmdFIS->lba2 = (startLBA & 0x000000FF0000) >> 16;
        cmdFIS->device = 1 << 6;
        cmdFIS->lba3 = (startLBA & 0x0000FF000000) >> 24;
        cmdFIS->lba4 = (startLBA & 0x00FF00000000) >> 32;
        cmdFIS->lba5 = (startLBA & 0xFF0000000000) >> 40;
        cmdFIS->command = ATA_CMD_READ_DMA_EXT;
    } else if(ahciDrive->idSpace.capabilities & (1 << 9)){ 
        // LBA28:
        cmdFIS->lba0 = (startLBA & 0x00000FF) >> 0;
        cmdFIS->lba1 = (startLBA & 0x000FF00) >> 8;
        cmdFIS->lba2 = (startLBA & 0x0FF0000) >> 16;
        cmdFIS->device = 1 << 6;
        cmdFIS->lba3 = 0;
        cmdFIS->lba4 = 0;
        cmdFIS->lba5 = 0;
        cmdFIS->command = ATA_CMD_READ_DMA;
    } else {
        // CHS:
        int sect = (startLBA % 63) + 1;
        int cyl = (startLBA + 1  - sect) / (16 * 63);
        cmdFIS->lba0 = sect;
        cmdFIS->lba1 = (cyl >> 0) & 0xFF;
        cmdFIS->lba2 = (cyl >> 8) & 0xFF;
        cmdFIS->lba3 = 0;
        cmdFIS->lba4 = 0;
        cmdFIS->lba5 = 0;
        cmdFIS->command = ATA_CMD_READ_DMA;
    }

    cmdFIS->countLow = count & 0xFF;
	cmdFIS->countHigh = (count >> 8) & 0xFF;

    //Wait for the port to finish whatever it may be doing
    if(!ahci_wait_clear(&abar->ports[i].tfd, AHCI_PxTFD_BSY, 1000) ||
        !ahci_wait_clear(&abar->ports[i].tfd, AHCI_PxTFD_DRQ, 1000)){
        klog("Device on port %d hung!\n", KLOG_WARNING, "AHCI", ahciDrive->port);
        return -ENODEV;
    }

    port->ci = 1 << freeCMDSlot;
    
    //TODO: add polling function
    while (1){
        if((port->ci & (1 << freeCMDSlot)) == 0) break;
        //Task file error
        if (port->is & AHCI_PxIS_TFES){
            klog("Controller or Device Faulty or Unsupported! Could not Read Device\n", KLOG_FAILED, "AHCI");
            return -ENODEV;
        }
    }
    if (port->is & AHCI_PxIS_TFES){
        klog("Controller or Device Faulty or Unsupported! Could not Read Device\n", KLOG_FAILED, "AHCI");
        return -ENODEV;
    }

    while(port->ci != 0) arch_no_op();

    memcpy(buf, (void*)((uint64_t)bufContinuous + hhdmOffset), count*512);

    pmm_free(bufContinuous, (count*512/PAGE_SIZE)+1);

    return 0;
}

int ahci_ata_write(drive_t* drive, size_t startLBA, size_t blocks, void* buf){
    //prdtLength would never go over MAX_PRDS with this restriction
    if(blocks > 0xFFFF) return -EINVAL;

    size_t count = blocks;

    ahci_drive_t* ahciDrive = (ahci_drive_t*)drive->context;
    ahci_controller_t* ahciController = ahciDrive->controller;
    ahci_mem_t* abar = ahciController->abar;
    ahci_mem_port_t* port = &abar->ports[ahciDrive->port];

    port->serr = 0xFFFFFFFF;
    port->is = 0xFFFFFFFF; 

    int freeCMDSlot = ahci_find_cmd_slot(ahciController, port);
    if(freeCMDSlot < 0) return -1;

    ahci_command_header_t* commandList = (ahci_command_header_t*)(((uint64_t)port->clb | (uint64_t)port->clbu << 32) + hhdmOffset);

    ahci_command_header_t* cmdHeader = &commandList[freeCMDSlot];
    ahci_cmd_table_t* cmdTable = (ahci_cmd_table_t*)(((uint64_t)cmdHeader->ctba | (uint64_t)cmdHeader->ctbau << 32) + hhdmOffset);

    cmdHeader->cfl = sizeof(fis_reg_h2d_t)/sizeof(uint32_t);
    cmdHeader->c = 1;
    cmdHeader->p = 0;
    cmdHeader->w = 1;
    cmdHeader->prdtLength = (uint16_t)((blocks-1)/0x400000)+1;

    memset((void*)cmdTable, 0, sizeof(ahci_cmd_table_t)+(MAX_PRDS-1)*sizeof(ahci_prd_t));

    //Maybe this is not the best way to do DMA...
    void* bufContinuous;

    if(!(abar->cap1 & AHCI_CAP_S64A)){
        bufContinuous = pmm_allocate32((blocks*512/PAGE_SIZE)+1);
    } else {
        bufContinuous = pmm_allocate((blocks*512/PAGE_SIZE)+1);
    }

    int i = 0;
    int bufContOffset = 0;
    for(; i < cmdHeader->prdtLength-1; i++){
        if(!(abar->cap1 & AHCI_CAP_S64A)){
            cmdTable->prdtEntries[i].dba = (uint32_t)((uint64_t)bufContinuous + bufContOffset);
            cmdTable->prdtEntries[i].dbau = 0;
        } else {
            cmdTable->prdtEntries[i].dba = (uint32_t)((uint64_t)(bufContinuous + bufContOffset) & 0x00000000FFFFFFFF);
            cmdTable->prdtEntries[i].dbau = (uint32_t)(((uint64_t)(bufContinuous + bufContOffset) & 0xFFFFFFFF00000000) >> 32);
        }
        cmdTable->prdtEntries[i].dbc = 0x3FFFFF;
        cmdTable->prdtEntries[i].i = 0;
        bufContOffset += 0x400000;
        blocks -= 0x2000;
    }

    if(!(abar->cap1 & AHCI_CAP_S64A)){
        cmdTable->prdtEntries[i].dba = (uint32_t)((uint64_t)bufContinuous + bufContOffset);
        cmdTable->prdtEntries[i].dbau = 0;
    } else {
        cmdTable->prdtEntries[i].dba = (uint32_t)((uint64_t)(bufContinuous + bufContOffset) & 0x00000000FFFFFFFF);
        cmdTable->prdtEntries[i].dbau = (uint32_t)(((uint64_t)(bufContinuous + bufContOffset) & 0xFFFFFFFF00000000) >> 32);
    }
    cmdTable->prdtEntries[i].dbc = blocks*512-1;
	cmdTable->prdtEntries[i].i = 0;

    fis_reg_h2d_t* cmdFIS = (fis_reg_h2d_t*)(&cmdTable->cfis);
    memset((void*)cmdFIS, 0, 64);
    cmdFIS->fisType = FIS_TYPE_REG_H2D;
    cmdFIS->c = 1;
    
    if(startLBA >= 0x100000){
        // LBA48:
        cmdFIS->lba0 = (startLBA & 0x0000000000FF) >> 0;
        cmdFIS->lba1 = (startLBA & 0x00000000FF00) >> 8;
        cmdFIS->lba2 = (startLBA & 0x000000FF0000) >> 16;
        cmdFIS->device = 1 << 6;
        cmdFIS->lba3 = (startLBA & 0x0000FF000000) >> 24;
        cmdFIS->lba4 = (startLBA & 0x00FF00000000) >> 32;
        cmdFIS->lba5 = (startLBA & 0xFF0000000000) >> 40;
        cmdFIS->command = ATA_CMD_WRITE_DMA_EXT;
    } else if(ahciDrive->idSpace.capabilities & (1 << 9)){ 
        // LBA28:
        cmdFIS->lba0 = (startLBA & 0x00000FF) >> 0;
        cmdFIS->lba1 = (startLBA & 0x000FF00) >> 8;
        cmdFIS->lba2 = (startLBA & 0x0FF0000) >> 16;
        cmdFIS->device = 1 << 6;
        cmdFIS->lba3 = 0;
        cmdFIS->lba4 = 0;
        cmdFIS->lba5 = 0;
        cmdFIS->command = ATA_CMD_WRITE_DMA;
    } else {
        // CHS:
        int sect = (startLBA % 63) + 1;
        int cyl = (startLBA + 1  - sect) / (16 * 63);
        cmdFIS->lba0 = sect;
        cmdFIS->lba1 = (cyl >> 0) & 0xFF;
        cmdFIS->lba2 = (cyl >> 8) & 0xFF;
        cmdFIS->lba3 = 0;
        cmdFIS->lba4 = 0;
        cmdFIS->lba5 = 0;
        cmdFIS->command = ATA_CMD_WRITE_DMA;
    }

    cmdFIS->countLow = count & 0xFF;
	cmdFIS->countHigh = (count >> 8) & 0xFF;

    //Wait for the port to finish whatever it may be doing
    if(!ahci_wait_clear(&abar->ports[i].tfd, AHCI_PxTFD_BSY, 1000) ||
        !ahci_wait_clear(&abar->ports[i].tfd, AHCI_PxTFD_DRQ, 1000)){
        klog("Device on port %d hung!\n", KLOG_WARNING, "AHCI", ahciDrive->port);
        return -ENODEV;
    }

    port->ci = 1 << freeCMDSlot;
    
    while (1){
        if((port->ci & (1 << freeCMDSlot)) == 0) break;
        //Task file error
        if (port->is & AHCI_PxIS_TFES){
            klog("Controller or Device Faulty or Unsupported! Could not Write to Device\n", KLOG_FAILED, "AHCI");
            return -ENODEV;
        }
    }

    if (port->is & AHCI_PxIS_TFES){
        klog("Controller or Device Faulty or Unsupported! Could not Write to Device\n", KLOG_FAILED, "AHCI");
        return -ENODEV;
    }

    while(port->ci != 0) arch_no_op();

    memcpy(buf, (void*)((uint64_t)bufContinuous + hhdmOffset), count*512);

    pmm_free(bufContinuous, (count*512/PAGE_SIZE)+1);

    return 0;
}