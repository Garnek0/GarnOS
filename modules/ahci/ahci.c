/*  
*   Module: ahci.sys
*
*   File: ahci.c
*
*   Module Author: Garnek
*   
*   Module Description: AHCI Controller Driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "ahci.h"
#include <garn/kernel.h>
#include <garn/dal/dal.h>
#include <garn/hw/pci.h>
#include <garn/mm.h>
#include <garn/timer.h>
#include <garn/kerrno.h>
#include <garn/kstdio.h>
#include <garn/module.h>

/*TODO:
- [x] Finish attach() and add a metadata struct to be stored in the drive_t struct.
- [x] Add ahci_ata_read() and ahci_ata_write().
- [ ] Make this driver work on real hardware.
- [x] Add some polling function to replace those while() loops
- [ ] Deal with SEMB and PM Devices.
- [ ] Implement a more complete setup phase.
- [ ] Implement ATAPI read/write.
*/

static int ahci_find_cmd_slot(ahci_controller_t* controller, ahci_mem_port_t* port){
	uint32_t slots = (port->sact | port->ci);
	for(int i = 0; i < controller->maxCommands; i++){
		if((slots & 1) == 0) return i;
		slots >>= 1;
	}
	klog("Cannot find free command slot!\n", KLOG_FAILED, "AHCI");
	return -1;
}

static bool ahci_wait_set(volatile uint32_t* reg, uint32_t bit, uint32_t ms){
    for(size_t i = 0; i < ms + 1; i++){
        if(*reg & bit) return true;
        ksleep(1);
    }
    return false;
}

static bool ahci_wait_clear(volatile uint32_t* reg, uint32_t bit, uint32_t ms){
    for(size_t i = 0; i < ms + 1; i++){
        if(!(*reg & bit)) return true;
        ksleep(1);
    }
    return false;
}

static bool ahci_enable(ahci_mem_t* abar){
    //Some AHCI Controllers require AHCI_GHC_AE to be set multiple times
    if(!(abar->cap1 & AHCI_CAP_SAM)){
        if(!(abar->ghc & AHCI_GHC_AE)){
            for(int i = 0; i < 3; i++){
                abar->ghc |= AHCI_GHC_AE;
                if(abar->ghc & AHCI_GHC_AE) break;
                ksleep(10);
            }
        }
    }

    if(!(abar->ghc & AHCI_GHC_AE)) return false;
    else return true;
}

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

    while(port->ci != 0) asm volatile("nop");

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

    while(port->ci != 0) asm volatile("nop");

    memcpy(buf, (void*)((uint64_t)bufContinuous + hhdmOffset), count*512);

    pmm_free(bufContinuous, (count*512/PAGE_SIZE)+1);

    return 0;
}

int ahci_atapi_read(drive_t* drive, size_t startLBA, size_t blocks, void* buf){
    return 0;
}

int ahci_atapi_write(drive_t* drive, size_t startLBA, size_t blocks, void* buf){
    return 0;
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

    if(device->bus != DEVICE_BUS_PCI || pciConfig->hdr.class != PCI_CLASS_STORAGE_CONTROLLER || pciConfig->hdr.subclass != PCI_SUBCLASS_SATA ||
    pciConfig->hdr.progIF != 0x01){
        return false;
    }
    return true;
}

bool attach(device_t* device){
    if(!probe(device)) return false;

    pci_config_device_t* pciConfig;
    pciConfig = (pci_config_device_t*)device->data;

    //Get ABAR and disable caching

    ahci_mem_t* abar = (ahci_mem_t*)((pciConfig->BAR5 & 0xFFFFFFF0) + hhdmOffset);
    vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)abar, VMM_PRESENT | VMM_RW | VMM_PCD);

    ahci_controller_t* ahciController = kmalloc(sizeof(ahci_controller_t));
    ahciController->abar = abar;
    ahciController->maxCommands = ((abar->cap1 >> 8) & 0x1F) + 1;

    //Get HBA Ownership
    if(abar->cap2 & AHCI_EXCAP_BOH){
        if(!(abar->bohc & AHCI_BOHC_OOS)){
            abar->bohc |= AHCI_BOHC_OOS;
            ksleep(25); //Give the BIOS time to set AHCI_BOHC_BB
            if(!ahci_wait_clear(&abar->bohc, AHCI_BOHC_BB, 3000) ||
               !ahci_wait_clear(&abar->bohc, AHCI_BOHC_BOS, 3000)){
                klog("Controller Faulty or Unsupported! Could not Complete BIOS/OS Handoff!\n", KLOG_FAILED, "AHCI");
                return false;
            }
        }   
    }

    //Disable Interrupts
    abar->ghc &= ~AHCI_GHC_IE;

    //Enable AHCI
    if(!ahci_enable(abar)){
        klog("Controller Faulty or Unsupported! Could not Enable AHCI Mode!\n", KLOG_FAILED, "AHCI");
        return false;
    }

    //Clear Pending interrupts
    abar->is |= 0xFFFFFFFF;

    //Start port initialisation and device detection

    ahci_drive_t* ahciDrive;
    drive_t drive;
    memset((void*)&drive, 0, sizeof(drive_t));

    for(int i = 0; i < 32; i++){
        if(abar->pi & (1 << i)){
            //Stop command processing
            abar->ports[i].cmd &= ~AHCI_PxCMD_ST;
            abar->ports[i].cmd &= ~AHCI_PxCMD_FRE;

            //TODO: Maybe add some code to handle these kinds of situations...
            if(!ahci_wait_clear(&abar->ports[i].cmd, AHCI_PxCMD_CR, 600) ||
               !ahci_wait_clear(&abar->ports[i].cmd, AHCI_PxCMD_FR, 600)){
                klog("Port %d hung!\n", KLOG_WARNING, "AHCI", i);
                continue;
            }

            //Clear errors and Pending IRQs
            abar->ports[i].serr = 0xFFFFFFFF;
            abar->ports[i].is = 0xFFFFFFFF;

            abar->ports[i].ie = 0; //No int support. Maybe support ints in the future?

            //Allocate Command list
            if(!(abar->cap1 & AHCI_CAP_S64A)){
                abar->ports[i].clb = (uint32_t)((uint64_t)pmm_allocate32(1));
                abar->ports[i].clbu = 0;
                memset((void*)((uint64_t)abar->ports[i].clb + hhdmOffset) , 0, PAGE_SIZE);
                vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)abar->ports[i].clb + hhdmOffset, VMM_PRESENT | VMM_RW | VMM_PCD);
            } else {
                uint64_t addr = (uint64_t)pmm_allocate(1);
                abar->ports[i].clb = (addr & 0x00000000FFFFFFFF);
                abar->ports[i].clbu = ((addr & 0xFFFFFFFF00000000) >> 32);
                memset((void*)((uint64_t)addr + hhdmOffset), 0, PAGE_SIZE);
                vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)addr + hhdmOffset, VMM_PRESENT | VMM_RW | VMM_PCD);
            }

            ahci_command_header_t* commandList = (ahci_command_header_t*)(((uint64_t)abar->ports[i].clb | ((uint64_t)abar->ports[i].clbu << 32)) + hhdmOffset);

            //Allocate command tables
            for(int j = 0; j < ahciController->maxCommands; j++){
                if(!(abar->cap1 & AHCI_CAP_S64A)){
                    commandList[j].ctba = (uint32_t)((uint64_t)pmm_allocate32(1));
                    commandList[j].ctbau = 0;
                    memset((void*)((uint64_t)commandList[j].ctba + hhdmOffset), 0, PAGE_SIZE);
                    vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)commandList[j].ctba + hhdmOffset, VMM_PRESENT | VMM_RW | VMM_PCD);
                } else {
                    uint64_t addr = (uint64_t)pmm_allocate(1);
                    commandList[j].ctba = (addr & 0x00000000FFFFFFFF);
                    commandList[j].ctbau = ((addr & 0xFFFFFFFF00000000) >> 32);
                    memset((void*)((uint64_t)addr + hhdmOffset), 0, PAGE_SIZE);
                    vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)addr + hhdmOffset, VMM_PRESENT | VMM_RW | VMM_PCD);
                }
            }

            //Allocate Received FIS
            if(!(abar->cap1 & AHCI_CAP_S64A)){
                abar->ports[i].fb = (uint32_t)((uint64_t)pmm_allocate32(1));
                abar->ports[i].fbu = 0;
                memset((void*)((uint64_t)abar->ports[i].fb + hhdmOffset) , 0, PAGE_SIZE);
                vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)abar->ports[i].fb + hhdmOffset, VMM_PRESENT | VMM_RW | VMM_PCD);
            } else {
                uint64_t addr = (uint64_t)pmm_allocate(1);
                abar->ports[i].fb = (addr & 0x00000000FFFFFFFF);
                abar->ports[i].fbu = ((addr & 0xFFFFFFFF00000000) >> 32);
                memset((void*)((uint64_t)addr + hhdmOffset), 0, PAGE_SIZE);
                vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)addr + hhdmOffset, VMM_PRESENT | VMM_RW | VMM_PCD);
            }

            //Check device presence and type
            if(abar->ports[i].det && abar->ports[i].ipm){
                //Start command processing
                abar->ports[i].cmd |= AHCI_PxCMD_FRE;
                abar->ports[i].cmd |= AHCI_PxCMD_ST;

                if(!ahci_wait_set(&abar->ports[i].cmd, AHCI_PxCMD_CR, 600) ||
                   !ahci_wait_set(&abar->ports[i].cmd, AHCI_PxCMD_FR, 600)){
                    klog("Port %d hung!\n", KLOG_WARNING, "AHCI", i);
                    continue;
                }

                if(abar->ports[i].sig == SATA_SIG_ATA || abar->ports[i].sig == SATA_SIG_ATAPI){
                    if(abar->ports[i].sig == SATA_SIG_ATA){
                        klog("Found ATA device on port %u.\n", KLOG_INFO, "AHCI", i);
                    } else {
                        klog("Found ATAPI device on port %u.\n", KLOG_INFO, "AHCI", i);
                    }

                    ahciDrive = kmalloc(sizeof(ahci_drive_t));
                    memset((void*)ahciDrive, 0, sizeof(ahci_drive_t));
                    ahciDrive->controller = ahciController;
                    ahciDrive->port = i;
                    ahciDrive->type = abar->ports[i].sig == SATA_SIG_ATA ? AHCI_ATA : AHCI_ATAPI;

                    //Prepare for an identify Command

                    int freeCMDSlot = ahci_find_cmd_slot(ahciController, &abar->ports[i]);
                    
                    ahci_command_header_t* cmdHeader = &commandList[freeCMDSlot];
                    ahci_cmd_table_t* cmdTable = (ahci_cmd_table_t*)(((uint64_t)cmdHeader->ctba | (uint64_t)cmdHeader->ctbau << 32) + hhdmOffset);

                    cmdHeader->cfl = sizeof(fis_reg_h2d_t)/sizeof(uint32_t);
                    cmdHeader->c = 1;
                    cmdHeader->w = 0;
                    cmdHeader->prdtLength = 1;

                    uint16_t* buf;

                    if(!(abar->cap1 & AHCI_CAP_S64A)){
                        buf = pmm_allocate32(1);
                        cmdTable->prdtEntries[0].dba = (uint32_t)((uint64_t)buf);
                        cmdTable->prdtEntries[0].dbau = 0;
                    } else {
                        buf = pmm_allocate(1);
                        cmdTable->prdtEntries[0].dba = ((uint64_t)buf & 0x00000000FFFFFFFF);
                        cmdTable->prdtEntries[0].dbau = (((uint64_t)buf & 0xFFFFFFFF00000000) >> 32);
                    }

                    cmdTable->prdtEntries[0].dbc = 256*sizeof(uint16_t)-1;
                    cmdTable->prdtEntries[0].i = 0;

                    fis_reg_h2d_t* cmdFIS = (fis_reg_h2d_t*)cmdTable->cfis;
                    memset((void*)cmdFIS, 0, sizeof(fis_reg_h2d_t));
                    cmdFIS->fisType = FIS_TYPE_REG_H2D;
                    cmdFIS->c = 1;
                    cmdFIS->command = abar->ports[i].sig == SATA_SIG_ATA ? ATA_CMD_IDENTIFY : ATA_CMD_IDENTIFY_PACKET;

                    //Wait for the device to finish whatever it may be doing
                    if(!ahci_wait_clear(&abar->ports[i].tfd, AHCI_PxTFD_BSY, 1000) ||
                       !ahci_wait_clear(&abar->ports[i].tfd, AHCI_PxTFD_DRQ, 1000)){
                        klog("Device on port %d hung!\n", KLOG_WARNING, "AHCI", i);
                        pmm_free(buf, 1);
                        continue;
                    }

                    abar->ports[i].ci = 1 << freeCMDSlot;
                    
                    if(!ahci_wait_clear(&abar->ports[i].tfd, AHCI_PxTFD_BSY, 1000)){
                        klog("Device on port %d hung!\n", KLOG_WARNING, "AHCI", i);
                        pmm_free(buf, 1);
                        continue;
                    }

                    bool status = true;
                    while (1){
                        if((abar->ports[i].ci & (1 << freeCMDSlot)) == 0) break;
                        //Task file error
                        if (abar->ports[i].is & AHCI_PxIS_TFES){
                            klog("Controller or Device Faulty or Unsupported! Could not Identify Device\n", KLOG_WARNING, "AHCI");
                            pmm_free(buf, 1);
                            status = false;
                            break;
                        }
                    }
                    if(!status) continue;

                    memcpy((void*)&ahciDrive->idSpace, (void*)buf, 512);

                    char* tmp = (char*)&ahciDrive->idSpace.model;

                    for(uint8_t k = 0; k < 40; k+=2){
                        ahciDrive->model[k] = tmp[k + 1];
                        ahciDrive->model[k + 1] = tmp[k];
                    }
                    ahciDrive->model[40] = 0;

                    for(uint8_t k = 39; k > 0; k--){
                        if(ahciDrive->model[k] == 0x20) ahciDrive->model[k] = 0;
                        else break;
                    }

                    if(ahciDrive->idSpace.commandSets[1] & (1 << 10)) ahciDrive->size = ahciDrive->idSpace.sectorsLBA48;
                    else ahciDrive->size = ahciDrive->idSpace.sectorsLBA28;

                    ahciDrive->size *= 512;

                    drive.context = (void*)ahciDrive;
                    drive.blockSize = 512;
                    drive.interface = DRIVE_IF_AHCI;
                    drive.name = ahciDrive->model;
                    drive.size = ahciDrive->size;
                    drive.type = abar->ports[i].sig == SATA_SIG_ATA ? DRIVE_TYPE_DISK : DRIVE_TYPE_OPTICAL;
                    //ATAPI access not implemented
                    drive.read = abar->ports[i].sig == SATA_SIG_ATA ? ahci_ata_read : NULL;
                    drive.write = abar->ports[i].sig == SATA_SIG_ATA ? ahci_ata_write : NULL;

                    pmm_free(buf, 1);

                    drive_add(drive);
                } else {
                    klog("Found PM or EM device (both of which are currently unsupported).\n", KLOG_INFO, "AHCI");
                }
            }
            
        }
    }
    klog("Controller Initialised. (HBA Implements AHCI %d.%d.%d)\n", KLOG_OK, "AHCI", abar->vsMajor, abar->vsMinor, abar->vsPatch);
    return true;
}

bool remove(device_t* dev){
    return false;
}

module_t metadata = {
    .name = "ahci",
    .init = init,
    .fini = fini
};

device_driver_t driver_metadata = {
    .probe = probe,
    .attach = attach,
    .remove = remove
};

device_id_t driver_ids[] = {
    DEVICE_CREATE_ID_PCI(DEVICE_ID_PCI_VENDOR_ANY, DEVICE_ID_PCI_DEVICE_ANY,PCI_CLASS_STORAGE_CONTROLLER, PCI_SUBCLASS_SATA, 0x01),
    0
};