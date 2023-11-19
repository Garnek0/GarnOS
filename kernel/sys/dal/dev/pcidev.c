/*  
*   File: pcidev.c
*
*   Author: Garnek
*   
*   Description: PCI Devices
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "pcidev.h"
#include <hw/pci/pci.h>
#include <hw/ports.h>
#include <kstdio.h>
#include <exec/elf.h>
#include <mem/mm/kheap.h>
#include <mem/memutil/memutil.h>
#include <sys/dal/dal.h>

pci_config_header_t hdr;
pci_config_device_t* pciDevice;
pci_config_pci_to_pci_t* pciToPci;
pci_config_pci_to_cardbus_t* pciToCardbus;

void pcidev_init(){
    device_t* device;

    for(uint16_t i = 0; i < 256; i++){
        for(uint16_t j = 0; j < 32; j++){
            for(uint16_t k = 0; k < 8; k++){
                hdr.vendorID = pci_config_read_word(i, j, k, 0x0);
                hdr.deviceID = pci_config_read_word(i, j, k, 0x2);
                if(hdr.vendorID == 0xFFFF && hdr.deviceID == 0xFFFF) continue;

                hdr.command = pci_config_read_word(i, j, k, 0x4);
                hdr.status = pci_config_read_word(i, j, k, 0x6);
                hdr.revisionID = pci_config_read_word(i, j, k, 0x8) & 0xFF;
                hdr.progIF = (pci_config_read_word(i, j, k, 0x8) >> 8) & 0xFF;
                hdr.subclass = pci_config_read_word(i, j, k, 0xA) & 0xFF;
                hdr.class = (pci_config_read_word(i, j, k, 0xA) >> 8) & 0xFF;
                hdr.cacheLineSize = pci_config_read_word(i, j, k, 0xC) & 0xFF;
                hdr.latencyTimer = (pci_config_read_word(i, j, k, 0xC) >> 8) & 0xFF;
                hdr.headerType = pci_config_read_word(i, j, k, 0xE) & 0xFF;
                hdr.BIST = (pci_config_read_word(i, j, k, 0xE) >> 8) & 0xFF;

                device = kmalloc(sizeof(device_t));
                memset(device, 0, sizeof(device_t));
                device->bus = DEVICE_BUS_PCI;
                device->type = DEVICE_TYPE_UNDEFINED;
                device->name = "Unknown PCI Bus Device";

                switch(hdr.headerType & 0x7F){
                    case PCI_HEADER_DEVICE:
                        pciDevice = kmalloc(sizeof(pci_config_device_t));
                        memset(pciDevice, 0, sizeof(pci_config_device_t));

                        pciDevice->hdr = hdr;
                        pciDevice->BAR0 = pci_config_read_address(i, j, k, 0x10);
                        pciDevice->BAR1 = pci_config_read_address(i, j, k, 0x14);
                        pciDevice->BAR2 = pci_config_read_address(i, j, k, 0x18);
                        pciDevice->BAR3 = pci_config_read_address(i, j, k, 0x1C);
                        pciDevice->BAR4 = pci_config_read_address(i, j, k, 0x20);
                        pciDevice->BAR5 = pci_config_read_address(i, j, k, 0x24);
                        pciDevice->cardbusCISPtr = pci_config_read_address(i, j, k, 0x28);
                        pciDevice->subsystemVendorID = pci_config_read_word(i, j, k, 0x2C);
                        pciDevice->subsystemDeviceID = pci_config_read_word(i, j, k, 0x2E);
                        pciDevice->expansionROM = pci_config_read_address(i, j, k, 0x30);
                        pciDevice->capabilitiesPointer = pci_config_read_word(i, j, k, 0x34);
                        pciDevice->intLine = pci_config_read_word(i, j, k, 0x3C) & 0xFF;
                        pciDevice->intPIN = (pci_config_read_word(i, j, k, 0x3C) >> 8) & 0xFF;
                        pciDevice->minGrant = pci_config_read_word(i, j, k, 0x3E) & 0xFF;
                        pciDevice->maxLatency = (pci_config_read_word(i, j, k, 0x3E) >> 8) & 0xFF;
                        device->data = (void*)pciDevice;

                        switch(pciDevice->hdr.class){
                            case PCI_CLASS_UNCLASSIFIED:
                            {
                                device->type = DEVICE_TYPE_UNDEFINED;
                                break;
                            }
                            case PCI_CLASS_STORAGE_CONTROLLER:
                            {
                                device->type = DEVICE_TYPE_STORAGE_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_SCSI_BUS:
                                        device->name = "SCSI Storage Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_IDE:
                                        device->name = "IDE Storage Controller";
                                        if(device_driver_attach(device)) break;
                                        elf_load_module("0:/ide.mod");
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_FLOPPY:
                                        device->name = "Floppy Disk Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_IPI:
                                        device->name = "IPI Storage Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_RAID:
                                        device->name = "RAID Storage Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_ATA:
                                        device->name = "ATA Storage Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_SATA:
                                        device->name = "SATA Storage Controller";
                                        if(device_driver_attach(device)) break;
                                        elf_load_module("0:/ahci.mod");
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_SAS:
                                        device->name = "SAS Storage Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_NVM:
                                        device->name = "NVM Storage Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_UFS:
                                        device->name = "UFS Storage Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    default:
                                        device->name = "Storage Controller";
                                        break;
                                }
                                break;
                            }
                            case PCI_CLASS_NETWORK_CONTROLLER:
                            {
                                device->type = DEVICE_TYPE_NETWORK_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_ETHERNET:
                                        device->name = "Ethernet Network Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_TOKEN_RING:
                                        device->name = "Token Ring Network Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_FDDI:
                                        device->name = "FDDI Network Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_ATM:
                                        device->name = "ATM Network Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_ISDN:
                                        device->name = "ISDN Network Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_WORLDFLIP:
                                        device->name = "WorldFlip Network Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_PICMG_2_14:
                                        device->name = "PICMG 2.14 Multi Computing Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_INFINIBAND:
                                        device->name = "Infiniband Network Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_FABRIC:
                                        device->name = "Fabric Network Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    default:
                                        device->name = "Network Controller";
                                        break;
                                }
                                break;
                            }
                            case PCI_CLASS_DISPLAY_CONTROLLER:
                            {
                                device->type = DEVICE_TYPE_DISPLAY_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_VGA:
                                        device->name = "VGA Compatible Display Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;
                                    
                                    case PCI_SUBCLASS_XGA:
                                        device->name = "XGA Compatible Display Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_3D:
                                        device->name = "Non-VGA 3D Display Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    default:
                                        device->name = "Display Controller";
                                        break;
                                }
                                break;
                            }
                            case PCI_CLASS_MULTIMEDIA_CONTROLLER:
                            {
                                device->type = DEVICE_TYPE_MULTIMEDIA_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_VIDEO:
                                        device->name = "Multimedia Video Device";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_AUDIO:
                                        device->name = "Multimedia Audio Device";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_TELEPHONY:
                                        device->name = "Multimedia Computer Telephony Device";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_HDA:
                                        device->name = "High Definition Audio Device";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    default:
                                        device->name = "Multimedia Controller";
                                        break;
                                }
                                break;
                            }
                            case PCI_CLASS_MEMORY_CONTROLLER:
                            {
                                device->type = DEVICE_TYPE_MEMORY_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_RAM:
                                        device->name = "RAM Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_FLASH:
                                        device->name = "Flash Memory Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    default:
                                        device->name = "Memory Controller";
                                        break;
                                }
                                break;
                            }
                            case PCI_CLASS_BRIDGE:
                            {
                                device->type = DEVICE_TYPE_SYSTEM_DEVICE;

                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_HOST:
                                        device->name = "Host Bridge";
                                        break;
                                    case PCI_SUBCLASS_ISA:
                                        device->name = "ISA Bridge";
                                        break;
                                    case PCI_SUBCLASS_EISA:
                                        device->name = "EISA Bridge";
                                        break;
                                    case PCI_SUBCLASS_MCA:
                                        device->name = "MCA Bridge";
                                        break;
                                    case PCI_SUBCLASS_PCMCIA:
                                        device->name = "PCMCIA Bridge";
                                        break;
                                    case PCI_SUBCLASS_NUBUS:
                                        device->name = "NUBUS Bridge";
                                        break;
                                    case PCI_SUBCLASS_RACEWAY:
                                        device->name = "RACEWAY Bridge";
                                        break;
                                    case PCI_SUBCLASS_INFINIBAND_TO_PCI:
                                        device->name = "InfiniBand-to-PCI Host Bridge";
                                        break;
                                    default:
                                        device->name = "Unknown Bridge";
                                        break;
                                }
                                break;
                            }
                            case PCI_CLASS_COMMUNICATION_CONTROLLER:
                            {
                                device->type = DEVICE_TYPE_COMMUNICATION_DEVICE;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_SERIAL:
                                        device->name = "Serial Communications Device";
                                        device->type = DEVICE_TYPE_SERIAL_DEVICE;
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;
                                    
                                    case PCI_SUBCLASS_PARALLEL:
                                        device->name = "Parallel Communications Device";
                                        device->type = DEVICE_TYPE_PARALLEL_DEVICE;
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_MULTIPORT_SERIAL:
                                        device->name = "Multiport Serial Communications Device";
                                        device->type = DEVICE_TYPE_SERIAL_DEVICE;
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_MODEM:
                                        device->name = "Modem";
                                        device->type = DEVICE_TYPE_MODEM;
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_GPIB:
                                        device->name = "General Purpose Interface Bus";
                                        device->type = DEVICE_TYPE_GPIB;
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_SMART_CARD:
                                        device->name = "Smart Card Device";
                                        device->type = DEVICE_TYPE_SMART_CARD;
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    default:
                                        device->name = "Simple Communications Device";
                                        break;
                                }
                                break;
                            }
                            case PCI_CLASS_BASE_SYSTEM_PERIPHERAL:
                            {
                                device->type = DEVICE_TYPE_SYSTEM_DEVICE;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_PIC:
                                        device->name = "Programmable Interrupt Controller";
                                        break;
                                    case PCI_SUBCLASS_DMA:
                                        device->name = "DMA Controller";
                                        break;
                                    case PCI_SUBCLASS_TIMER:
                                        device->name = "Timer";
                                        break;
                                    case PCI_SUBCLASS_RTC:
                                        device->name = "Realtime Clock";
                                        break;
                                    case PCI_SUBCLASS_HOT_PLUG:
                                        device->name = "PCI Hot-Plug Controller";
                                        break;
                                    case PCI_SUBCLASS_SD:
                                        device->name = "SD Host Controller";
                                        device->type = DEVICE_TYPE_SD_HOST_CONTROLLER;
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;
                                    case PCI_SUBCLASS_IOMMU:
                                        device->name = "IOMMU";
                                        break;
                                    case PCI_SUBCLASS_EVENT_COLLECTOR:
                                        device->name = "Root Complex Event Collector";
                                        break;
                                    default:
                                        device->name = "PCI Base Peripheral Device";
                                        break;
                                }
                                break;
                            }
                            case PCI_CLASS_INPUT_CONTROLLER:
                            {
                                device->type = DEVICE_TYPE_INPUT_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_KEYBOARD_CONTROLLER:
                                        device->name = "Keyboard Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_DIGITISER:
                                        device->name = "Digitiser";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_MOUSE_CONTROLLER:
                                        device->name = "Mouse Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_GAMEPORT_CONTROLLER:
                                        device->name = "Gameport Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;
                                    
                                    default:
                                        device->name = "Input Controller";
                                        break;
                                }
                                break;
                            }
                            case PCI_CLASS_DOCKING_STATION:
                            {
                                device->type = DEVICE_TYPE_DOCKING_STATION;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_GENERIC:
                                        device->name = "Generic Docking Station";
                                        break;
                                    default:
                                        device->name = "Docking Station";
                                        break;
                                }
                                break;
                            }
                            case PCI_CLASS_PROCESSOR:
                            {
                                device->type = DEVICE_TYPE_PROCESSOR;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_386:
                                        device->name = "PCI Mounted i386 Processor";
                                        break;
                                    case PCI_SUBCLASS_486:
                                        device->name = "PCI Mounted i486 Processor";
                                        break;
                                    case PCI_SUBCLASS_PENTIUM:
                                        device->name = "PCI Mounted Pentium Processor";
                                        break;
                                    case PCI_SUBCLASS_PENTIUM_PRO:
                                        device->name = "PCI Mounted Pentium Pro Processor";
                                        break;
                                    case PCI_SUBCLASS_PPC:
                                        device->name = "PCI Mounted PowerPC Processor";
                                        break;
                                    case PCI_SUBCLASS_ALPHA:
                                        device->name = "PCI Mounted Alpha Processor";
                                        break;
                                    case PCI_SUBCLASS_MIPS:
                                        device->name = "PCI Mounted MIPS Processor";
                                        break;
                                    case PCI_SUBCLASS_COPROCESSOR:
                                        device->name = "PCI Mounted Co-Processor";
                                        break;
                                    default:
                                        device->name = "PCI Mounted Processor/Co-Processor";
                                        break;
                                }
                                break;
                            }
                            case PCI_CLASS_SERIAL_BUS_CONTROLLER:
                            {
                                device->type = DEVICE_TYPE_SERIAL_BUS;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_FIREWIRE:
                                        device->name = "FireWire Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_ACCESS_BUS:
                                        device->name = "ACCESS Bus Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_SSA:
                                        device->name = "SSA Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_USB:
                                        device->name = "Universal Serial Bus Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_FIBRE:
                                        device->name = "Fibre Channel Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_SMBUS:
                                        device->name = "SMBus Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_SERIAL_INFINIBAND:
                                        device->name = "InfiniBand Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_IPMI:
                                        device->name = "IPMI Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_SERCOS:
                                        device->name = "SERCOS Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_CANBUS:
                                        device->name = "CANBUS Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    default:
                                        device->name = "Serial Bus Controller";
                                        break;
                                }
                                break;
                            }
                            case PCI_CLASS_WIRELESS_CONTROLLER:
                            {
                                device->type = DEVICE_TYPE_NETWORK_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_IRDA:
                                        device->name = "iRDA Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_IR:
                                        device->name = "IR Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_RF:
                                        device->name = "RF Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;
                                    
                                    case PCI_SUBCLASS_BLUETOOTH:
                                        device->name = "Bluetooth Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_BROADBAND:
                                        device->name = "Broadband Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_ETHERNET_802_11A:
                                        device->name = "Wireless 802.11a Ethernet Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_ETHERNET_802_11B:
                                        device->name = "Wireless 802.11b Ethernet Controller";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_CELLULAR_CONTROLLER_MODEM:
                                        device->name = "Cellular Controller/Modem";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;

                                    case PCI_SUBCLASS_CELLULAR_CONTROLLER_MODEM_WITH_ETHERNET:
                                        device->name = "Cellular Controller/Modem With Ethernet (802.11)";
                                        if(device_driver_attach(device)) break;
                                        //No driver module for this yet
                                        if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                        break;
                                    
                                    default:
                                        device->name = "Wireless Controller";
                                        break;
                                }
                                break;
                            }
                            case PCI_CLASS_INTELLIGENT_CONTROLLER:
                            {
                                device->name = "I20 Intelligent I/O Controller";
                                if(device_driver_attach(device)) break;
                                //No driver module for this yet
                                if(!device_driver_attach(device)) klog("Could not find driver for %s!\n", KLOG_WARNING, device->name);
                                break;
                            }
                            default:
                                break;
                        }
                        device_add(device);
                        break;
                    case PCI_HEADER_PCI_TO_PCI:
                        pciToPci = kmalloc(sizeof(pci_config_pci_to_pci_t));
                        memset(pciToPci, 0, sizeof(pci_config_pci_to_pci_t));

                        pciToPci->hdr = hdr;
                        pciToPci->BAR0 = pci_config_read_address(i, j, k, 0x10);
                        pciToPci->BAR1 = pci_config_read_address(i, j, k, 0x14);
                        pciToPci->primaryBus = pci_config_read_word(i, j, k, 0x18) & 0xFF;
                        pciToPci->secondaryBus = (pci_config_read_word(i, j, k, 0x18) >> 8) & 0xFF;
                        pciToPci->subordinateBus = pci_config_read_word(i, j, k, 0x1A) & 0xFF;
                        pciToPci->secondaryLatencyTimer = (pci_config_read_word(i, j, k, 0x1A) >> 8) & 0xFF;
                        pciToPci->IOBase = pci_config_read_word(i, j, k, 0x1C) & 0xFF;
                        pciToPci->IOLimit = (pci_config_read_word(i, j, k, 0x1C) >> 8) & 0xFF;
                        pciToPci->secondayStatus = pci_config_read_word(i, j, k, 0x1E);
                        pciToPci->memoryBase = pci_config_read_word(i, j, k, 0x20);
                        pciToPci->memoryLimit = pci_config_read_word(i, j, k, 0x22);
                        pciToPci->prefetchableMemoryBase = pci_config_read_word(i, j, k, 0x24);
                        pciToPci->prefetchableMemoryLimit = pci_config_read_word(i, j, k, 0x26);
                        pciToPci->prefetchableBaseUpper = pci_config_read_address(i, j, k, 0x28);
                        pciToPci->prefetchableLimitUpper = pci_config_read_address(i, j, k, 0x2C);
                        pciToPci->IOBaseUpper = pci_config_read_word(i, j, k, 0x30);
                        pciToPci->IOLimitUpper = pci_config_read_word(i, j, k, 0x32);
                        pciToPci->capabilitiesPointer = pci_config_read_word(i, j, k, 0x34) & 0xFF;
                        pciToPci->expansionROM = pci_config_read_address(i, j, k, 0x38);
                        pciToPci->intLine = pci_config_read_word(i, j, k, 0x3C) & 0xFF;
                        pciToPci->intPIN = (pci_config_read_word(i, j, k, 0x3C) >> 8) & 0xFF;
                        pciToPci->bridgeControl = pci_config_read_word(i, j, k, 0x3E);
                        device->data = (void*)pciToPci;
                        device->type = DEVICE_TYPE_SYSTEM_DEVICE;
                        device->name = "PCI-to-PCI Bridge";
                        device_add(device);
                        break;
                    case PCI_HEADER_PCI_TO_CARDBUS:
                        pciToCardbus = kmalloc(sizeof(pci_config_pci_to_cardbus_t));
                        memset(pciToCardbus, 0, sizeof(pci_config_pci_to_cardbus_t));
                        //TODO: Do this
                        device->data = (void*)pciToCardbus;
                        device->type = DEVICE_TYPE_SYSTEM_DEVICE;
                        device->name = "Cardbus Bridge";
                        device_add(device);
                        break;
                    default:
                        break;
                }

                
            }
        }
    }
}
