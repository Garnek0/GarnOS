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

//Note to future self: DONT FORGET TO REMOVE THE BREAKPOINT IN kernel.c!!!

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

    achi_mem_t* abar = pciConfig->BAR5 + bl_get_hhdm_offset();
    vmm_set_flags(vmm_get_kernel_pml4(), (uint64_t)abar, VMM_PRESENT | VMM_RW | VMM_PCD);

    if(!(abar->ghc & (1 << 31))) return false;

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