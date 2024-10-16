#include <sys/dal/dal-internals.h>
#include <garn/hw/pci.h>
#include <garn/kstdio.h>
#include <exec/elf.h>
#include <garn/mm.h>
#include <garn/dal/dal.h>

pci_location_t location;
pci_config_header_t hdr;
pci_config_device_t* pciDevice;
pci_config_pci_to_pci_t* pciToPci;
pci_config_pci_to_cardbus_t* pciToCardbus;

void pcidev_detect(){
    device_t* device;

    for(uint16_t i = 0; i < 256; i++){
        for(uint16_t j = 0; j < 32; j++){
            for(uint16_t k = 0; k < 8; k++){
                location.bus = i;
                location.dev = j;
                location.func = k;

                hdr.vendorID = pci_config_read_word(location, 0x0);
                hdr.deviceID = pci_config_read_word(location, 0x2);
                if(hdr.vendorID == 0xFFFF && hdr.deviceID == 0xFFFF) continue;

                hdr.command = pci_config_read_word(location, 0x4);
                hdr.status = pci_config_read_word(location, 0x6);
                hdr.revisionID = pci_config_read_word(location, 0x8) & 0xFF;
                hdr.progIF = (pci_config_read_word(location, 0x8) >> 8) & 0xFF;
                hdr.subclass = pci_config_read_word(location, 0xA) & 0xFF;
                hdr.class = (pci_config_read_word(location, 0xA) >> 8) & 0xFF;
                hdr.cacheLineSize = pci_config_read_word(location, 0xC) & 0xFF;
                hdr.latencyTimer = (pci_config_read_word(location, 0xC) >> 8) & 0xFF;
                hdr.headerType = pci_config_read_word(location, 0xE) & 0xFF;
                hdr.BIST = (pci_config_read_word(location, 0xE) >> 8) & 0xFF;

                device = kmalloc(sizeof(device_t));
                memset(device, 0, sizeof(device_t));
                device->bus = DEVICE_BUS_PCI;
				device_id_initialise(device);
				device_id_add(device, DEVICE_CREATE_ID_PCI(hdr.vendorID, hdr.deviceID, hdr.class, hdr.subclass, hdr.progIF));

                switch(hdr.headerType & 0x7F){
                    case PCI_HEADER_DEVICE:
                        pciDevice = kmalloc(sizeof(pci_config_device_t));
                        memset(pciDevice, 0, sizeof(pci_config_device_t));

                        pciDevice->hdr = hdr;
                        pciDevice->location = location;
                        pciDevice->BAR0 = pci_config_read_address(location, 0x10);
                        pciDevice->BAR1 = pci_config_read_address(location, 0x14);
                        pciDevice->BAR2 = pci_config_read_address(location, 0x18);
                        pciDevice->BAR3 = pci_config_read_address(location, 0x1C);
                        pciDevice->BAR4 = pci_config_read_address(location, 0x20);
                        pciDevice->BAR5 = pci_config_read_address(location, 0x24);
                        pciDevice->cardbusCISPtr = pci_config_read_address(location, 0x28);
                        pciDevice->subsystemVendorID = pci_config_read_word(location, 0x2C);
                        pciDevice->subsystemDeviceID = pci_config_read_word(location, 0x2E);
                        pciDevice->expansionROM = pci_config_read_address(location, 0x30);
                        pciDevice->capabilitiesPointer = pci_config_read_word(location, 0x34);
                        pciDevice->intLine = pci_config_read_word(location, 0x3C) & 0xFF;
                        pciDevice->intPIN = (pci_config_read_word(location, 0x3C) >> 8) & 0xFF;
                        pciDevice->minGrant = pci_config_read_word(location, 0x3E) & 0xFF;
                        pciDevice->maxLatency = (pci_config_read_word(location, 0x3E) >> 8) & 0xFF;

                        device->data = (void*)pciDevice;

                        switch(pciDevice->hdr.class){
                            case PCI_CLASS_UNCLASSIFIED:
                                device->type = DEVICE_TYPE_UNDEFINED;
                                device->name = "Unclassified PCI Device";
                                break;
                            case PCI_CLASS_STORAGE_CONTROLLER:
                                device->type = DEVICE_TYPE_STORAGE_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_SCSI_BUS:
                                        device->name = "SCSI Storage Controller";
                                        break;
                                    case PCI_SUBCLASS_IDE:
                                        device->name = "IDE Storage Controller";
                                        break;
                                    case PCI_SUBCLASS_FLOPPY:
                                        device->name = "Floppy Disk Controller";
                                        break;
                                    case PCI_SUBCLASS_IPI: 
                                        device->name = "IPI Bus Storage Controller";
                                        break;
                                    case PCI_SUBCLASS_RAID:
                                        device->name = "RAID Storage Controller";
                                        break;
                                    case PCI_SUBCLASS_ATA:
                                        device->name = "ATA Storage Controller";
                                        break;
                                    case PCI_SUBCLASS_SATA:
                                        device->name = "SATA Storage Controller";
                                        break;
                                    case PCI_SUBCLASS_SAS:
                                        device->name = "SAS Storage Controller";
                                        break;
                                    case PCI_SUBCLASS_NVM:
                                        device->name = "Non-Volatile Memory Controller";
                                        break;
                                    case PCI_SUBCLASS_UFS:
                                        device->name = "UFS Controller";
                                        break;
                                    default:
                                        device->name = "Mass Storage Controller";
                                        break;
                                }  
                                break;
                            case PCI_CLASS_NETWORK_CONTROLLER:
                                device->type = DEVICE_TYPE_NETWORK_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_ETHERNET:
                                        device->name = "Ethernet Network Controller";
                                        break;
                                    case PCI_SUBCLASS_TOKEN_RING:
                                        device->name = "Token Ring Network Controller";
                                        break;
                                    case PCI_SUBCLASS_FDDI:
                                        device->name = "FDDI Network Controller";
                                        break;
                                    case PCI_SUBCLASS_ATM:
                                        device->name = "ATM Network Controller";
                                        break;
                                    case PCI_SUBCLASS_ISDN:
                                        device->name = "ISDN Network Controller";
                                        break;
                                    case PCI_SUBCLASS_WORLDFLIP:
                                        device->name = "WorldFlip Network Controller";
                                        break;
                                    case PCI_SUBCLASS_PICMG_2_14:
                                        device->name = "PICMG Network Controller";
                                        break;
                                    case PCI_SUBCLASS_INFINIBAND:
                                        device->name = "Infiniband Network Controller";
                                        break;
                                    case PCI_SUBCLASS_FABRIC:
                                        device->name = "Fabric Network Controller";
                                        break;
                                    default:
                                        device->name = "Network Controller";
                                        break;
                                }
                                break;
                            case PCI_CLASS_DISPLAY_CONTROLLER:
                                device->type = DEVICE_TYPE_DISPLAY_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_VGA:
                                        device->name = "VGA Display Controller";
                                        break;
                                    case PCI_SUBCLASS_XGA:
                                        device->name = "XGA Display Controller";
                                        break;
                                    case PCI_SUBCLASS_3D:
                                        device->name = "3D Display Controller";
                                        break;
                                    default:
                                        device->name = "Display Controller";
                                }
                                break;
                            case PCI_CLASS_MULTIMEDIA_CONTROLLER:
                                device->type = DEVICE_TYPE_MULTIMEDIA_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_VIDEO:
                                        device->name = "Multimedia Video Controller";
                                        break;
                                    case PCI_SUBCLASS_AUDIO:
                                        device->name = "Multimedia Audio Controller";
                                        break;
                                    case PCI_SUBCLASS_TELEPHONY:
                                        device->name = "Computer Telephony Device";
                                        break;
                                    case PCI_SUBCLASS_HDA:
                                        device->name = "HDA Device";
                                        break;
									default:                                        
										device->name = "Multimedia Controller";
                                        break;
                                }
                                break;
                            case PCI_CLASS_MEMORY_CONTROLLER:
                                device->type = DEVICE_TYPE_MEMORY_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_RAM:
                                        device->name = "RAM Controller";
                                        break;
                                    case PCI_SUBCLASS_FLASH:
                                        device->name = "FLASH Controller";
                                        break;
                                    case PCI_SUBCLASS_CXL:
                                        device->name = "CXL";
                                        break;
                                    default:
                                        device->name = "Memory Controller";
                                        break;
                                }
                                break;
                            case PCI_CLASS_BRIDGE:
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
                                        device->name = "NuBus Bridge";
                                        break;
                                    case PCI_SUBCLASS_RACEWAY:
                                        device->name = "RACEWay Bridge";
                                        break;
                                    case PCI_SUBCLASS_INFINIBAND_TO_PCI:
                                        device->name = "Infiniband to PCI Host Bridge";
                                        break;
                                    default:
                                        device->name = "Bridge";
                                        break;
                                }
                                break;
                            case PCI_CLASS_COMMUNICATION_CONTROLLER:
                                device->type = DEVICE_TYPE_COMMUNICATION_DEVICE;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_MULTIPORT_SERIAL:
                                    case PCI_SUBCLASS_SERIAL:
                                        device->type = DEVICE_TYPE_SERIAL_DEVICE;
                                        device->name = "Serial Controller";
                                        break;
                                    case PCI_SUBCLASS_PARALLEL:
                                        device->type = DEVICE_TYPE_PARALLEL_DEVICE;
                                        device->name = "Parallel Controller";
                                        break;
                                    case PCI_SUBCLASS_MODEM:
                                        device->type = DEVICE_TYPE_MODEM;
                                        device->name = "Modem";
                                        break;
                                    case PCI_SUBCLASS_GPIB:
                                        device->type = DEVICE_TYPE_GPIB;
                                        device->name = "GPIB Controller";
                                        break;
                                    case PCI_SUBCLASS_SMART_CARD:
                                        device->name = "Smart Card Controller";
                                        break;
                                    default:
                                        device->name = "Communication Controller";
                                        break;
                                }
                                break;
                            case PCI_CLASS_BASE_SYSTEM_PERIPHERAL:
                                device->type = DEVICE_TYPE_SYSTEM_DEVICE;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_PIC:
                                        device->name = "Programmable Interrupt Controller";
                                       break;
                                    case PCI_SUBCLASS_DMA:
                                        device->name = "DMA Controller";
                                        break;
                                    case PCI_SUBCLASS_TIMER:
                                        device->name = "System Timer";
                                        break;
                                    case PCI_SUBCLASS_RTC:
                                        device->name = "Real-Time Clock";
                                        break;
                                    case PCI_SUBCLASS_HOT_PLUG:
                                        device->name = "PCI Hot-plug Controller";
                                        break;
                                    case PCI_SUBCLASS_SD:
                                        device->type = DEVICE_TYPE_SD_HOST_CONTROLLER;
                                        device->name = "SD Host Controller";
                                        break;
                                    case PCI_SUBCLASS_IOMMU:
                                        device->name = "IOMMU";
                                        break;
                                    case PCI_SUBCLASS_EVENT_COLLECTOR:
                                        device->name = "Event Collector";
                                        break;
                                    case PCI_SUBCLASS_TIMING_CARD:
                                        device->name = "Timing Card";
                                        break;
                                    default:
                                        device->name = "System Peripheral";
                                        break;
                                }
                                break;
                            case PCI_CLASS_INPUT_CONTROLLER:
                                device->type = DEVICE_TYPE_INPUT_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_KEYBOARD_CONTROLLER:
                                        device->name = "Keyboard Controller";
                                        break;
                                    case PCI_SUBCLASS_DIGITIZER:
                                        device->name = "Digitizer Pen";
                                        break;
                                    case PCI_SUBCLASS_MOUSE_CONTROLLER:
                                        device->name = "Mouse Controller";
                                        break;
                                    case PCI_SUBCLASS_SCANNER_CONTROLLER:
                                        device->name = "Scanner Controller";
                                        break;
                                    case PCI_SUBCLASS_GAMEPORT_CONTROLLER:
                                        device->name = "Gameport Controller";
                                        break;
                                    default:
                                        device->name = "Input Device Controller";
                                        break;
                                }
                                break;
                            case PCI_CLASS_DOCKING_STATION:
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
                            case PCI_CLASS_PROCESSOR:
                                device->type = DEVICE_TYPE_PROCESSOR;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_386:
                                        device->name = "PCI Attached 386 Processor";
                                        break;
                                    case PCI_SUBCLASS_486:
                                        device->name = "PCI Attached 486 Processor";
                                        break;
                                    case PCI_SUBCLASS_PENTIUM:
                                        device->name = "PCI Attached Pentium Processor";
                                        break;
                                    case PCI_SUBCLASS_PENTIUM_PRO:
                                        device->name = "PCI Attached Pentium Pro Processor";
                                        break;
                                    case PCI_SUBCLASS_ALPHA:
                                        device->name = "PCI Attached Alpha Processor";
                                        break;
                                    case PCI_SUBCLASS_PPC:
                                        device->name = "PCI Attached PowerPC Processor";
                                        break;
                                    case PCI_SUBCLASS_MIPS:
                                        device->name = "PCI Attached MIPS Processor";
                                        break;
                                    default:
                                        device->name = "Co-Processor";
                                        break;
                                }
                                break;
                            case PCI_CLASS_SERIAL_BUS_CONTROLLER:
                                device->type = DEVICE_TYPE_SERIAL_BUS;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_FIREWIRE:
                                        device->name = "FireWire Controller";
                                        break;
                                    case PCI_SUBCLASS_ACCESS_BUS:
                                        device->name = "ACCESS Bus Controller";
                                        break;
                                    case PCI_SUBCLASS_SSA:
                                        device->name = "SSA Controller";
                                        break;
                                    case PCI_SUBCLASS_USB:
                                        device->name = "USB Controller";
                                        break;
                                    case PCI_SUBCLASS_FIBRE:
                                        device->name = "Fibre Channel Controller";
                                        break;
                                    case PCI_SUBCLASS_SMBUS:
                                        device->name = "SMBus Controller";
                                        break;
                                    case PCI_SUBCLASS_SERIAL_INFINIBAND:
                                        device->name = "InfiniBand Controller";
                                        break;
                                    case PCI_SUBCLASS_IPMI:
                                        device->name = "IPMI Interface";
                                        break;
                                    case PCI_SUBCLASS_SERCOS:
                                        device->name = "SERCOS Interface";
                                        break;
                                    case PCI_SUBCLASS_CANBUS:
                                        device->name = "CANBUS Controller";
                                        break;
                                    default:
                                        device->name = "Serial Bus Controller";
                                        break;
                                }
                                break;
                            case PCI_CLASS_WIRELESS_CONTROLLER:
                                device->type = DEVICE_TYPE_NETWORK_CONTROLLER;
                                switch(pciDevice->hdr.subclass){
                                    case PCI_SUBCLASS_IRDA:
                                        device->name = "IRDA Controller";
                                        break;
                                    case PCI_SUBCLASS_IR:
                                        device->name = "Consumer IR Controller";
                                        break;
                                    case PCI_SUBCLASS_RF:
                                        device->name = "RF Controller";
                                        break;
                                    case PCI_SUBCLASS_BLUETOOTH:
                                        device->name = "Bluetooth Controller";
                                        break;
                                    case PCI_SUBCLASS_BROADBAND:
                                        device->name = "Broadband Controller";
                                        break;
                                    case PCI_SUBCLASS_ETHERNET_802_1A:
                                        device->name = "802.1a Controller";
                                        break;
                                    case PCI_SUBCLASS_ETHERNET_802_1B:
                                        device->name = "802.1b Controller";
                                        break;
                                    case PCI_SUBCLASS_CELLULAR_CONTROLLER_MODEM:
                                        device->name = "Cellular Controller Modem";
                                        break;
                                    case PCI_SUBCLASS_CELLULAR_CONTROLLER_MODEM_WITH_ETHERNET:
                                        device->name = "Cellular Controller Modem with Ethernet";
                                        break;
                                    default:
                                        device->name = "Wireless Controller";
                                        break;
                                }
                                break;
                            case PCI_CLASS_INTELLIGENT_CONTROLLER:
                                device->type = DEVICE_TYPE_SYSTEM_DEVICE;
                                device->name = "I2O Intelligent Controller";
                                break;
                            case PCI_CLASS_SATELLITE_COMMUNICATION_CONTROLLER:
                                device->type = DEVICE_TYPE_MULTIMEDIA_CONTROLLER;
                                device->name = "Sattelite Communication Device";
                                break;
                            case PCI_CLASS_ENCRYPTION_CONTROLLER:
                                device->type = DEVICE_TYPE_SYSTEM_DEVICE;
                                device->name = "Encryption Controller";
                                break;
                            case PCI_CLASS_SIGNAL_PROCESSING_CONTROLLER:
                                device->type = DEVICE_TYPE_SYSTEM_DEVICE;
                                device->name = "Signal Processing Controller";
                                break;
                            case PCI_CLASS_PROCESSING_ACCELERATOR:
                                device->type = DEVICE_TYPE_SYSTEM_DEVICE;
                                device->name = "Smart Data Accelerator Interface (SDXI)";
                                break;
                            case PCI_CLASS_NON_ESSENTIAL:
                                device->type = DEVICE_TYPE_UNDEFINED;
                                device->name = "Non-Essential Intrumentation";
                                break;
                            case PCI_CLASS_COPROCESSOR:
                                device->type = DEVICE_TYPE_PROCESSOR;
                                device->name = "Co-Processor";
                                break;
                            default:
                                device->type = DEVICE_TYPE_UNDEFINED;
                                device->name = "Unknown PCI Device";
                                break;
                        }

                        klog("Found device on Bus %d Slot %d Function %d: %s.\n", KLOG_INFO, "PCI", i, j, k, device->name);
                        device_add(device);
                        break;
                    case PCI_HEADER_PCI_TO_PCI:
                        pciToPci = kmalloc(sizeof(pci_config_pci_to_pci_t));
                        memset(pciToPci, 0, sizeof(pci_config_pci_to_pci_t));

                        pciToPci->hdr = hdr;
                        pciToPci->location = location;
                        pciToPci->BAR0 = pci_config_read_address(location, 0x10);
                        pciToPci->BAR1 = pci_config_read_address(location, 0x14);
                        pciToPci->primaryBus = pci_config_read_word(location, 0x18) & 0xFF;
                        pciToPci->secondaryBus = (pci_config_read_word(location, 0x18) >> 8) & 0xFF;
                        pciToPci->subordinateBus = pci_config_read_word(location, 0x1A) & 0xFF;
                        pciToPci->secondaryLatencyTimer = (pci_config_read_word(location, 0x1A) >> 8) & 0xFF;
                        pciToPci->IOBase = pci_config_read_word(location, 0x1C) & 0xFF;
                        pciToPci->IOLimit = (pci_config_read_word(location, 0x1C) >> 8) & 0xFF;
                        pciToPci->secondayStatus = pci_config_read_word(location, 0x1E);
                        pciToPci->memoryBase = pci_config_read_word(location, 0x20);
                        pciToPci->memoryLimit = pci_config_read_word(location, 0x22);
                        pciToPci->prefetchableMemoryBase = pci_config_read_word(location, 0x24);
                        pciToPci->prefetchableMemoryLimit = pci_config_read_word(location, 0x26);
                        pciToPci->prefetchableBaseUpper = pci_config_read_address(location, 0x28);
                        pciToPci->prefetchableLimitUpper = pci_config_read_address(location, 0x2C);
                        pciToPci->IOBaseUpper = pci_config_read_word(location, 0x30);
                        pciToPci->IOLimitUpper = pci_config_read_word(location, 0x32);
                        pciToPci->capabilitiesPointer = pci_config_read_word(location, 0x34) & 0xFF;
                        pciToPci->expansionROM = pci_config_read_address(location, 0x38);
                        pciToPci->intLine = pci_config_read_word(location, 0x3C) & 0xFF;
                        pciToPci->intPIN = (pci_config_read_word(location, 0x3C) >> 8) & 0xFF;
                        pciToPci->bridgeControl = pci_config_read_word(location, 0x3E);

                        device->data = (void*)pciToPci;
                        device->type = DEVICE_TYPE_SYSTEM_DEVICE;
                        device->name = "PCI-to-PCI Bridge";

                        klog("Found device on Bus %d Slot %d Function %d: %s.\n", KLOG_INFO, "PCI", i, j, k, device->name);
                        device_add(device);
                        break;
                    case PCI_HEADER_PCI_TO_CARDBUS:
                        pciToCardbus = kmalloc(sizeof(pci_config_pci_to_cardbus_t));
                        memset(pciToCardbus, 0, sizeof(pci_config_pci_to_cardbus_t));
                        //TODO: Do this
                        device->data = (void*)pciToCardbus;
                        device->type = DEVICE_TYPE_SYSTEM_DEVICE;
                        device->name = "Cardbus Bridge";
                        klog("Found device on Bus %d Slot %d Function %d: %s.\n", KLOG_INFO, "PCI", i, j, k, device->name);
                        device_add(device);
                        break;
                    default:
                        break;
                }        
            }
        }
    }
}
