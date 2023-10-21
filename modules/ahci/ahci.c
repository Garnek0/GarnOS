/*  
*   Module: ahci.mod
*
*   File: ahci.c
*
*   Module Author: Garnek
*   
*   Mdoule Description: AHCI Driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <module/module.h>
#include <sys/device.h>
#include <hw/pci/pci.h>
#include <kstdio.h>

void init(){
    return;
}

void fini(){
    return;
}

bool probe(device_t* device){
    pci_config_device_t* pciConfig;
    pciConfig = (pci_config_device_t*)device->data;

    if(device->bus != DEVICE_BUS_PCI || pciConfig->hdr.class != PCI_CLASS_STORAGE_CONTROLLER || pciConfig->hdr.subclass != PCI_SUBCLASS_SATA){
        return false;
    }
    return true;
}

bool attach(device_t* device){
    if(!probe(device)) return false;
    return true;
}

module_t metadata = {
    .name = "ahci",
    .init = init,
    .fini = fini
};

device_driver_t driver_metadata = {
    .probe = probe,
    .attach = attach
};