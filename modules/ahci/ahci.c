#include "ahci.h"

int ahci_find_cmd_slot(ahci_controller_t* controller, ahci_mem_port_t* port){
	uint32_t slots = (port->sact | port->ci);
	for(int i = 0; i < controller->maxCommands; i++){
		if((slots & 1) == 0) return i;
		slots >>= 1;
	}
	klog("Cannot find free command slot!\n", KLOG_FAILED, "AHCI");
	return -1;
}

bool ahci_enable(ahci_mem_t* abar){
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

void init(){
    return;
}

void fini(){
	//TODO: Make devices spin down or something
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

    //Enable bus mastering, memory access and interrupts in the PCI command register

    pciConfig->hdr.command |= ((1 << 1) & (1 << 2));
    pciConfig->hdr.command &= ~(1 << 10);

    pci_config_write_word(pciConfig->location, 0x4, pciConfig->hdr.command);

	//Set device name
	device->name = "AHCI Controller";

    //Get ABAR and disable caching

    ahci_mem_t* abar = (ahci_mem_t*)((pciConfig->BAR5 & 0xFFFFFFF0) + hhdmOffset);
    vmm_set_flags(vmm_get_kernel_pt(), (uint64_t)abar, VMM_PRESENT | VMM_RW | VMM_CACHE_DISABLE);

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
    drive_t* drive;

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
                vmm_set_flags(vmm_get_kernel_pt(), (uint64_t)abar->ports[i].clb + hhdmOffset, VMM_PRESENT | VMM_RW | VMM_CACHE_DISABLE);
            } else {
                uint64_t addr = (uint64_t)pmm_allocate(1);
                abar->ports[i].clb = (addr & 0x00000000FFFFFFFF);
                abar->ports[i].clbu = ((addr & 0xFFFFFFFF00000000) >> 32);
                memset((void*)((uint64_t)addr + hhdmOffset), 0, PAGE_SIZE);
                vmm_set_flags(vmm_get_kernel_pt(), (uint64_t)addr + hhdmOffset, VMM_PRESENT | VMM_RW | VMM_CACHE_DISABLE);
            }

            ahci_command_header_t* commandList = (ahci_command_header_t*)(((uint64_t)abar->ports[i].clb | ((uint64_t)abar->ports[i].clbu << 32)) + hhdmOffset);

            //Allocate command tables
            for(int j = 0; j < ahciController->maxCommands; j++){
                if(!(abar->cap1 & AHCI_CAP_S64A)){
                    commandList[j].ctba = (uint32_t)((uint64_t)pmm_allocate32(1));
                    commandList[j].ctbau = 0;
                    memset((void*)((uint64_t)commandList[j].ctba + hhdmOffset), 0, PAGE_SIZE);
                    vmm_set_flags(vmm_get_kernel_pt(), (uint64_t)commandList[j].ctba + hhdmOffset, VMM_PRESENT | VMM_RW | VMM_CACHE_DISABLE);
                } else {
                    uint64_t addr = (uint64_t)pmm_allocate(1);
                    commandList[j].ctba = (addr & 0x00000000FFFFFFFF);
                    commandList[j].ctbau = ((addr & 0xFFFFFFFF00000000) >> 32);
                    memset((void*)((uint64_t)addr + hhdmOffset), 0, PAGE_SIZE);
                    vmm_set_flags(vmm_get_kernel_pt(), (uint64_t)addr + hhdmOffset, VMM_PRESENT | VMM_RW | VMM_CACHE_DISABLE);
                }
            }

            //Allocate Received FIS
            if(!(abar->cap1 & AHCI_CAP_S64A)){
                abar->ports[i].fb = (uint32_t)((uint64_t)pmm_allocate32(1));
                abar->ports[i].fbu = 0;
                memset((void*)((uint64_t)abar->ports[i].fb + hhdmOffset) , 0, PAGE_SIZE);
                vmm_set_flags(vmm_get_kernel_pt(), (uint64_t)abar->ports[i].fb + hhdmOffset, VMM_PRESENT | VMM_RW | VMM_CACHE_DISABLE);
            } else {
                uint64_t addr = (uint64_t)pmm_allocate(1);
                abar->ports[i].fb = (addr & 0x00000000FFFFFFFF);
                abar->ports[i].fbu = ((addr & 0xFFFFFFFF00000000) >> 32);
                memset((void*)((uint64_t)addr + hhdmOffset), 0, PAGE_SIZE);
                vmm_set_flags(vmm_get_kernel_pt(), (uint64_t)addr + hhdmOffset, VMM_PRESENT | VMM_RW | VMM_CACHE_DISABLE);
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

					drive = kmalloc(sizeof(drive_t));
					memset(drive, 0, sizeof(drive_t));

                    drive->context = (void*)ahciDrive;
                    drive->blockSize = 512;
                    drive->interface = DRIVE_IF_AHCI;
                    drive->name = ahciDrive->model;
                    drive->size = ahciDrive->size;
                    drive->type = abar->ports[i].sig == SATA_SIG_ATA ? DRIVE_TYPE_DISK : DRIVE_TYPE_OPTICAL;
                    //ATAPI access not implemented
                    drive->read = abar->ports[i].sig == SATA_SIG_ATA ? ahci_ata_read : NULL;
                    drive->write = abar->ports[i].sig == SATA_SIG_ATA ? ahci_ata_write : NULL;

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
    DEVICE_ID_LIST_END
};
