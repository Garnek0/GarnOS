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
#include <sys/dal/dal.h>
#include <hw/pci/pci.h>
#include <sys/bootloader.h>
#include <mem/vmm/vmm.h> 
#include <mem/pmm/pmm.h>
#include <mem/memutil/memutil.h>
#include <mem/kheap/kheap.h>
#include <kerrno.h>

//Note to future self: DONT FORGET TO REMOVE THE BREAKPOINT IN kernel.c!!!

/*TODO:
- [ ] Finish attach() and add a metadata struct to be stored in the drive_t struct.
- [ ] Add ahci_ata_read() and ahci_ata_write()
- [ ] Deal with SEMB and PM Devices.
- [ ] Implement a more complete setup phase.
- [ ] Implement ATAPI read/write.
*/

int ahci_find_cmd_slot(ahci_controller_t* controller, ahci_mem_port_t* port){
	uint32_t slots = (port->sact | port->ci);
	for(int i = 0; i < controller->maxCommands; i++){
		if((slots & 1) == 0) return i;
		slots >>= 1;
	}
	klog("AHCI: Cannot find free command slot!\n", KLOG_FAILED);
	return -1;
}

int ahci_ata_read(drive_t* drive, size_t startLBA, size_t blocks, void* buf){
    //kprintf("[AHCI] Read Req: LBA 0x%x, Seccount %d\n", startLBA, blocks);
    //prdtLength would never go over MAX_PRDS with this restriction
    if(blocks > 0xFFFF) return -EINVAL;

    size_t count = blocks;

    ahci_drive_t* ahciDrive = (ahci_drive_t*)drive->context;
    ahci_controller_t* ahciController = ahciDrive->controller;
    ahci_mem_t* abar = ahciController->abar;
    ahci_mem_port_t* port = &abar->ports[ahciDrive->port];


    port->cmd |= AHCI_PxCMD_FRE;
    port->cmd |= AHCI_PxCMD_ST;
    port->serr = 0xFFFFFFFF;
    port->is = 0xFFFFFFFF; 

    int freeCMDSlot = ahci_find_cmd_slot(ahciController, port);
    if(freeCMDSlot < 0) return -1;

    ahci_command_header_t* commandList = (ahci_command_header_t*)(((uint64_t)port->clb | (uint64_t)port->clbu << 32) + bl_get_hhdm_offset());

    ahci_command_header_t* cmdHeader = &commandList[freeCMDSlot];
    ahci_cmd_table_t* cmdTable = (ahci_cmd_table_t*)(((uint64_t)cmdHeader->ctba | (uint64_t)cmdHeader->ctbau << 32) + bl_get_hhdm_offset());

    cmdHeader->cfl = sizeof(fis_reg_h2d_t)/sizeof(uint32_t);
    cmdHeader->c = 1;
    cmdHeader->p = 1;
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
            cmdTable->prdtEntries[i].dba = (uint32_t)bufContinuous + bufContOffset;
            cmdTable->prdtEntries[i].dbau = 0;
        } else {
            cmdTable->prdtEntries[i].dba = ((uint64_t)(bufContinuous + bufContOffset) & 0x00000000FFFFFFFF);
            cmdTable->prdtEntries[i].dbau = (((uint64_t)(bufContinuous + bufContOffset) & 0xFFFFFFFF00000000) >> 32);
        }
        cmdTable->prdtEntries[i].dbc = 0x3FFFFF;
        cmdTable->prdtEntries[i].i = 0;
        bufContOffset += 0x400000;
        blocks -= 0x2000;
    }

    if(!(abar->cap1 & AHCI_CAP_S64A)){
        cmdTable->prdtEntries[i].dba = (uint32_t)bufContinuous + bufContOffset;
        cmdTable->prdtEntries[i].dbau = 0;
    } else {
        cmdTable->prdtEntries[i].dba = ((uint64_t)(bufContinuous + bufContOffset) & 0x00000000FFFFFFFF);
        cmdTable->prdtEntries[i].dbau = (((uint64_t)(bufContinuous + bufContOffset) & 0xFFFFFFFF00000000) >> 32);
    }
    cmdTable->prdtEntries[i].dbc = blocks*512-1;
	cmdTable->prdtEntries[i].i = 0;

    fis_reg_h2d_t* cmdFIS = (fis_reg_h2d_t*)(&cmdTable->cfis);
    memset((void*)cmdFIS, 0, 64);
    cmdFIS->fisType = FIS_TYPE_REG_H2D;
    cmdFIS->c = 1;
    
    if(startLBA >= 0x10000000){
        // LBA48:
        cmdFIS->lba0 = (startLBA & 0x0000000000FF) >> 0;
        cmdFIS->lba1 = (startLBA & 0x00000000FF00) >> 8;
        cmdFIS->lba2 = (startLBA & 0x000000FF0000) >> 16;
        cmdFIS->lba3 = (startLBA & 0x0000FF000000) >> 24;
        cmdFIS->lba4 = (startLBA & 0x00FF00000000) >> 32;
        cmdFIS->lba5 = (startLBA & 0xFF0000000000) >> 40;
        cmdFIS->device = 1 << 6;
        cmdFIS->command = ATA_CMD_READ_DMA_EXT;
    } else if(ahciDrive->idSpace[ATA_IDENT_CAPABILITIES] & (1 << 9)){ 
        // LBA28:
        cmdFIS->lba0 = (startLBA & 0x00000FF) >> 0;
        cmdFIS->lba1 = (startLBA & 0x000FF00) >> 8;
        cmdFIS->lba2 = (startLBA & 0x0FF0000) >> 16;
        cmdFIS->lba3 = 0;
        cmdFIS->lba4 = 0;
        cmdFIS->lba5 = 0;
        cmdFIS->device = 1 << 6;
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

    //Wait for the port to finish whatever it's doing
    while(port->tfd & (AHCI_PxTFD_BSY | AHCI_PxTFD_DRQ));

    port->ci = 1 << freeCMDSlot;
    
    //TODO: add polling function
    while (1){
        if((port->ci & (1 << freeCMDSlot)) == 0) break;
        //Task file error
        if (port->is & AHCI_PxIS_TFES){
            klog("AHCI: Controller or Device Faulty or Unsupported! Could not Read Device\n", KLOG_FAILED);
            return -ENODEV;
        }
    }
    if (port->is & AHCI_PxIS_TFES){
        klog("AHCI: Controller or Device Faulty or Unsupported! Could not Read Device\n", KLOG_FAILED);
        return -ENODEV;
    }

    while(port->ci != 0) asm volatile("nop");

    memcpy(buf, bufContinuous, count*512);

    pmm_free(bufContinuous, (count*512/PAGE_SIZE)+1);

    return 0;
}

int ahci_ata_write(drive_t* drive, size_t startLBA, size_t blocks, void* buf){
    ;
}

int ahci_atapi_read(drive_t* drive, size_t startLBA, size_t blocks, void* buf){
    ;
}

int ahci_atapi_write(drive_t* drive, size_t startLBA, size_t blocks, void* buf){
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

    ahci_mem_t* abar = (pciConfig->BAR5 & 0xFFFFF000) + bl_get_hhdm_offset();
    vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)abar, VMM_PRESENT | VMM_RW | VMM_PCD);

    if(!(abar->ghc & (1 << 31))) return false;

    ahci_controller_t* ahciController = kmalloc(sizeof(ahci_controller_t));
    ahciController->abar = abar;
    ahciController->maxCommands = ((abar->cap1 >> 8) & 0x1F) + 1;

    //Get HBA Ownership
    if(abar->cap2 & AHCI_EXCAP_BOH){
        if(!(abar->bohc & AHCI_BOHC_OOS)){
            abar->bohc |= AHCI_BOHC_OOS;
            while((abar->bohc & AHCI_BOHC_BOS) || (abar->bohc & AHCI_BOHC_BB)) asm volatile("nop");
        }   
    }

    ahci_drive_t* ahciDrive;
    drive_t drive;

    for(int i = 0; i < 32; i++){
        if(abar->pi & (1 << i)){
            //Check device presence and type
            if(abar->ports[i].det && abar->ports[i].ipm){
                if(abar->ports[i].sig == SATA_SIG_ATA || abar->ports[i].sig == SATA_SIG_ATAPI){
                    //stop command processing
                    abar->ports[i].cmd &= ~AHCI_PxCMD_ST;
                    abar->ports[i].cmd &= ~AHCI_PxCMD_FRE;

                    while((abar->ports[i].cmd & AHCI_PxCMD_CR) || (abar->ports[i].cmd & AHCI_PxCMD_FR)) { asm volatile("nop"); }

                    //Set Maximum speed
                    if(abar->ports[i].spd == 0) abar->ports[i].spd = 0x3;

                    //Allocate Command list
                    if(!(abar->cap1 & AHCI_CAP_S64A)){
                        abar->ports[i].clb = pmm_allocate32(1);
                        abar->ports[i].clbu = 0;
                        memset((void*)((uint64_t)abar->ports[i].clb + bl_get_hhdm_offset()) , 0, PAGE_SIZE);
                        vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)abar->ports[i].clb + bl_get_hhdm_offset(), VMM_PRESENT | VMM_RW | VMM_PCD);
                    } else {
                        uint64_t addr = pmm_allocate(1);
                        abar->ports[i].clb = (addr & 0x00000000FFFFFFFF);
                        abar->ports[i].clbu = ((addr & 0xFFFFFFFF00000000) >> 32);
                        memset((void*)((uint64_t)addr + bl_get_hhdm_offset()), 0, PAGE_SIZE);
                        vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)addr + bl_get_hhdm_offset(), VMM_PRESENT | VMM_RW | VMM_PCD);
                    }

                    ahci_command_header_t* commandList = (ahci_command_header_t*)((uint64_t)abar->ports[i].clb | ((uint64_t)abar->ports[i].clbu << 32) + bl_get_hhdm_offset());

                    //Allocate command tables
                    for(int j = 0; j < ahciController->maxCommands; j++){
                        if(!(abar->cap1 & AHCI_CAP_S64A)){
                            commandList[j].ctba = pmm_allocate32(1);
                            commandList[j].ctbau = 0;
                            memset((void*)((uint64_t)commandList[j].ctba + bl_get_hhdm_offset()), 0, PAGE_SIZE);
                            vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)commandList[j].ctba + bl_get_hhdm_offset(), VMM_PRESENT | VMM_RW | VMM_PCD);
                        } else {
                            uint64_t addr = pmm_allocate(1);
                            commandList[j].ctba = (addr & 0x00000000FFFFFFFF);
                            commandList[j].ctbau = ((addr & 0xFFFFFFFF00000000) >> 32);
                            memset((void*)((uint64_t)addr + bl_get_hhdm_offset()), 0, PAGE_SIZE);
                            vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)addr + bl_get_hhdm_offset(), VMM_PRESENT | VMM_RW | VMM_PCD);
                        }
                    }

                    //Allocate Recieved FIS
                    if(!(abar->cap1 & AHCI_CAP_S64A)){
                        abar->ports[i].fb = pmm_allocate32(1);
                        abar->ports[i].fbu = 0;
                        memset((void*)((uint64_t)abar->ports[i].fb + bl_get_hhdm_offset()) , 0, PAGE_SIZE);
                        vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)abar->ports[i].fb + bl_get_hhdm_offset(), VMM_PRESENT | VMM_RW | VMM_PCD);
                    } else {
                        uint64_t addr = pmm_allocate(1);
                        abar->ports[i].fb = (addr & 0x00000000FFFFFFFF);
                        abar->ports[i].fbu = ((addr & 0xFFFFFFFF00000000) >> 32);
                        memset((void*)((uint64_t)addr + bl_get_hhdm_offset()), 0, PAGE_SIZE);
                        vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)addr + bl_get_hhdm_offset(), VMM_PRESENT | VMM_RW | VMM_PCD);
                    }

                    while(abar->ports[i].cmd & AHCI_PxCMD_CR);

                    abar->ports[i].cmd |= AHCI_PxCMD_FRE;
                    abar->ports[i].cmd |= AHCI_PxCMD_ST;
                    abar->ports[i].serr = 0xFFFFFFFF;
                    abar->ports[i].is = 0xFFFFFFFF;
                    abar->is = 0xFFFFFFFF;

                    ahciDrive = kmalloc(sizeof(ahci_drive_t));
                    ahciDrive->controller = ahciController;
                    ahciDrive->port = i;
                    ahciDrive->type = abar->ports[i].sig == SATA_SIG_ATA ? AHCI_ATA : AHCI_ATAPI;

                    //Prepare for an identify Command

                    int freeCMDSlot = ahci_find_cmd_slot(ahciController, &abar->ports[i]);
                    
                    ahci_command_header_t* cmdHeader = &commandList[freeCMDSlot];
                    ahci_cmd_table_t* cmdTable = (ahci_cmd_table_t*)(((uint64_t)cmdHeader->ctba | (uint64_t)cmdHeader->ctbau << 32) + bl_get_hhdm_offset());

                    cmdHeader->cfl = sizeof(fis_reg_h2d_t)/sizeof(uint32_t);
                    cmdHeader->c = 0;
                    cmdHeader->w = 0;
                    cmdHeader->prdtLength = 1;

                    uint16_t* buf;

                    if(!(abar->cap1 & AHCI_CAP_S64A)){
                        buf = pmm_allocate32(1);
                        cmdTable->prdtEntries[0].dba = (uint32_t)buf;
                        cmdTable->prdtEntries[0].dbau = 0;
                    } else {
                        buf = pmm_allocate(1);
                        cmdTable->prdtEntries[0].dba = ((uint64_t)buf & 0x00000000FFFFFFFF);
                        cmdTable->prdtEntries[0].dbau = (((uint64_t)buf & 0xFFFFFFFF00000000) >> 32);
                    }

                    cmdTable->prdtEntries[0].dbc = 256*sizeof(uint16_t)-1;
                    cmdTable->prdtEntries[0].i = 1;

                    fis_reg_h2d_t* cmdFIS = (fis_reg_h2d_t*)cmdTable->cfis;
                    cmdFIS->fisType = FIS_TYPE_REG_H2D;
                    cmdFIS->c = 1;
                    cmdFIS->command = abar->ports[i].sig == SATA_SIG_ATA ? ATA_CMD_IDENTIFY : ATA_CMD_IDENTIFY_PACKET;

                    cmdFIS->lba0 = cmdFIS->lba1 = cmdFIS->lba2 = cmdFIS->lba3 = cmdFIS->lba4 = cmdFIS->lba5 = cmdFIS->countHigh = cmdFIS->countLow = 0;

                    //Wait for the port to finish whatever it's doing
                    while(abar->ports[i].tfd & (AHCI_PxTFD_BSY | AHCI_PxTFD_DRQ));

                    abar->ports[i].ci = 1 << freeCMDSlot;
                    
                    //TODO: add some kind of polling function
                    while (1){
                        if((abar->ports[i].ci & (1 << freeCMDSlot)) == 0) break;
                        //Task file error
                        if (abar->ports[i].is & AHCI_PxIS_TFES){
                            klog("AHCI: Controller or Device Faulty or Unsupported! Could not Identify Device\n", KLOG_FAILED);
                            return false;
                        }
                    }

                    char* tmp = &buf[ATA_IDENT_MODEL];

                    for(uint8_t k = 0; k < 40; k+=2){
                        ahciDrive->model[k] = tmp[k + 1];
                        ahciDrive->model[k + 1] = tmp[k];
                    }
                    ahciDrive->model[40] = 0;

                    for(uint8_t k = 39; k > 0; k--){
                        if(ahciDrive->model[k] == 0x20) ahciDrive->model[k] = 0;
                        else break;
                    }

                    drive.context = (void*)ahciDrive;
                    drive.blockSize = 512;
                    drive.interface = DRIVE_IF_AHCI;
                    drive.name = ahciDrive->model;
                    drive.size = 0; //!
                    drive.type = abar->ports[i].sig == SATA_SIG_ATA ? DRIVE_TYPE_DISK : DRIVE_TYPE_OPTICAL;
                    //ATAPI access not implemented
                    drive.read = abar->ports[i].sig == SATA_SIG_ATA ? ahci_ata_read : NULL;
                    drive.write = abar->ports[i].sig == SATA_SIG_ATA ? ahci_ata_write : NULL;

                    kmfree(buf);

                    drive_add(drive);
                } else {
                    //. . .
                }
            }
            
        }
    }

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