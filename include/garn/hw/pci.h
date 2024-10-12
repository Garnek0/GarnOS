#ifndef PCI_H
#define PCI_H

#include <garn/types.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_CLASS_UNCLASSIFIED 0x00
#define PCI_CLASS_STORAGE_CONTROLLER 0x01
#define PCI_CLASS_NETWORK_CONTROLLER 0x02
#define PCI_CLASS_DISPLAY_CONTROLLER 0x03
#define PCI_CLASS_MULTIMEDIA_CONTROLLER 0x04
#define PCI_CLASS_MEMORY_CONTROLLER 0x05
#define PCI_CLASS_BRIDGE 0x06
#define PCI_CLASS_COMMUNICATION_CONTROLLER 0x07
#define PCI_CLASS_BASE_SYSTEM_PERIPHERAL 0x08
#define PCI_CLASS_INPUT_CONTROLLER 0x09
#define PCI_CLASS_DOCKING_STATION 0x0A
#define PCI_CLASS_PROCESSOR 0x0B
#define PCI_CLASS_SERIAL_BUS_CONTROLLER 0x0C
#define PCI_CLASS_WIRELESS_CONTROLLER 0x0D
#define PCI_CLASS_INTELLIGENT_CONTROLLER 0x0E
#define PCI_CLASS_SATELLITE_COMMUNICATION_CONTROLLER 0x0F
#define PCI_CLASS_ENCRYPTION_CONTROLLER 0x10
#define PCI_CLASS_SIGNAL_PROCESSING_CONTROLLER 0x11
#define PCI_CLASS_PROCESSING_ACCELERATOR 0x12
#define PCI_CLASS_NON_ESSENTIAL 0x13
#define PCI_CLASS_COPROCESSOR 0x40
#define PCI_CLASS_UNASSIGNED 0xFF

//PCI_CLASS_STORAGE_CONTROLLER
#define PCI_SUBCLASS_SCSI_BUS 0x00
#define PCI_SUBCLASS_IDE 0x01
#define PCI_SUBCLASS_FLOPPY 0x02
#define PCI_SUBCLASS_IPI 0x03
#define PCI_SUBCLASS_RAID 0x04
#define PCI_SUBCLASS_ATA 0x05
#define PCI_SUBCLASS_SATA 0x06
#define PCI_SUBCLASS_SAS 0x07
#define PCI_SUBCLASS_NVM 0x08
#define PCI_SUBCLASS_UFS 0x09

//PCI_CLASS_NETWORK_CONTROLLER
#define PCI_SUBCLASS_ETHERNET 0x00
#define PCI_SUBCLASS_TOKEN_RING 0x01
#define PCI_SUBCLASS_FDDI 0x02
#define PCI_SUBCLASS_ATM 0x03
#define PCI_SUBCLASS_ISDN 0x04
#define PCI_SUBCLASS_WORLDFLIP 0x05
#define PCI_SUBCLASS_PICMG_2_14 0x06
#define PCI_SUBCLASS_INFINIBAND 0x07
#define PCI_SUBCLASS_FABRIC 0x08

//PCI_CLASS_DISPLAY_CONTROLLER
#define PCI_SUBCLASS_VGA 0x00
#define PCI_SUBCLASS_XGA 0x01
#define PCI_SUBCLASS_3D 0x02

//PCI_CLASS_MULTIMEDIA_CONTROLLER
#define PCI_SUBCLASS_VIDEO 0x00
#define PCI_SUBCLASS_AUDIO 0x01
#define PCI_SUBCLASS_TELEPHONY 0x02
#define PCI_SUBCLASS_HDA 0x03

//PCI_CLASS_MEMORY_CONTROLLER
#define PCI_SUBCLASS_RAM 0x00
#define PCI_SUBCLASS_FLASH 0x01
#define PCI_SUBCLASS_CXL 0x02

//PCI_CLASS_BRIDGE
#define PCI_SUBCLASS_HOST 0x00
#define PCI_SUBCLASS_ISA 0x01
#define PCI_SUBCLASS_EISA 0x02
#define PCI_SUBCLASS_MCA 0x03
#define PCI_SUBCLASS_PCI_TO_PCI_1 0x04
#define PCI_SUBCLASS_PCMCIA 0x05
#define PCI_SUBCLASS_NUBUS 0x06
#define PCI_SUBCLASS_CARDBUS 0x07
#define PCI_SUBCLASS_RACEWAY 0x08
#define PCI_SUBCLASS_PCI_TO_PCI_2 0x09
#define PCI_SUBCLASS_INFINIBAND_TO_PCI 0x0A

//PCI_CLASS_COMMUNICATION_CONTROLLER
#define PCI_SUBCLASS_SERIAL 0x00
#define PCI_SUBCLASS_PARALLEL 0x01
#define PCI_SUBCLASS_MULTIPORT_SERIAL 0x02
#define PCI_SUBCLASS_MODEM 0x03
#define PCI_SUBCLASS_GPIB 0x04
#define PCI_SUBCLASS_SMART_CARD 0x05

//PCI_CLASS_BASE_SYSTEM_PERIPHERAL
#define PCI_SUBCLASS_PIC 0x00
#define PCI_SUBCLASS_DMA 0x01
#define PCI_SUBCLASS_TIMER 0x02
#define PCI_SUBCLASS_RTC 0x03
#define PCI_SUBCLASS_HOT_PLUG 0x04
#define PCI_SUBCLASS_SD 0x05
#define PCI_SUBCLASS_IOMMU 0x06
#define PCI_SUBCLASS_EVENT_COLLECTOR 0x07
#define PCI_SUBCLASS_TIMING_CARD 0x99

//PCI_CLASS_INPUT_CONTROLLER
#define PCI_SUBCLASS_KEYBOARD_CONTROLLER 0x00
#define PCI_SUBCLASS_DIGITIZER 0x01
#define PCI_SUBCLASS_MOUSE_CONTROLLER 0x02
#define PCI_SUBCLASS_SCANNER_CONTROLLER 0x03
#define PCI_SUBCLASS_GAMEPORT_CONTROLLER 0x04

//PCI_CLASS_DOCKING_STATION
#define PCI_SUBCLASS_GENERIC 0x00

//PCI_CLASS_PROCESSOR
#define PCI_SUBCLASS_386 0x00
#define PCI_SUBCLASS_486 0x01
#define PCI_SUBCLASS_PENTIUM 0x02
#define PCI_SUBCLASS_PENTIUM_PRO 0x03
#define PCI_SUBCLASS_ALPHA 0x10
#define PCI_SUBCLASS_PPC 0x20
#define PCI_SUBCLASS_MIPS 0x30
#define PCI_SUBCLASS_COPROCESSOR 0x40

//PCI_CLASS_SERIAL_BUS_CONTROLLER
#define PCI_SUBCLASS_FIREWIRE 0x00
#define PCI_SUBCLASS_ACCESS_BUS 0x01
#define PCI_SUBCLASS_SSA 0x02
#define PCI_SUBCLASS_USB 0x03
#define PCI_SUBCLASS_FIBRE 0x04
#define PCI_SUBCLASS_SMBUS 0x05
#define PCI_SUBCLASS_SERIAL_INFINIBAND 0x06
#define PCI_SUBCLASS_IPMI 0x07
#define PCI_SUBCLASS_SERCOS 0x08
#define PCI_SUBCLASS_CANBUS 0x09

//PCI_CLASS_WIRELESS_CONTROLLER
#define PCI_SUBCLASS_IRDA 0x00
#define PCI_SUBCLASS_IR 0x01
#define PCI_SUBCLASS_RF 0x10
#define PCI_SUBCLASS_BLUETOOTH 0x11
#define PCI_SUBCLASS_BROADBAND 0x12
#define PCI_SUBCLASS_ETHERNET_802_1A 0x20
#define PCI_SUBCLASS_ETHERNET_802_1B 0x21
#define PCI_SUBCLASS_CELLULAR_CONTROLLER_MODEM 0x40
#define PCI_SUBCLASS_CELLULAR_CONTROLLER_MODEM_WITH_ETHERNET 0x41

#define PCI_HEADER_DEVICE 0x00
#define PCI_HEADER_PCI_TO_PCI 0x01
#define PCI_HEADER_PCI_TO_CARDBUS 0x02

typedef struct {
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
} pci_location_t;

typedef struct {
    uint16_t vendorID;
    uint16_t deviceID;
    uint16_t command;
    uint16_t status;
    uint8_t revisionID;
    uint8_t progIF;
    uint8_t subclass;
    uint8_t class;
    uint8_t cacheLineSize;
    uint8_t latencyTimer;
    uint8_t headerType;
    uint8_t BIST;
} pci_config_header_t;

typedef struct {
    pci_location_t location;
    pci_config_header_t hdr;
    uint32_t BAR0;
    uint32_t BAR1;
    uint32_t BAR2;
    uint32_t BAR3;
    uint32_t BAR4;
    uint32_t BAR5;
    uint32_t cardbusCISPtr;
    uint16_t subsystemVendorID;
    uint16_t subsystemDeviceID;
    uint32_t expansionROM;
    uint8_t capabilitiesPointer;
    uint8_t intLine;
    uint8_t intPIN;
    uint8_t minGrant;
    uint8_t maxLatency;
} pci_config_device_t;

typedef struct {
    pci_location_t location;
    pci_config_header_t hdr;
    uint32_t BAR0;
    uint32_t BAR1;
    uint8_t primaryBus;
    uint8_t secondaryBus;
    uint8_t subordinateBus;
    uint8_t secondaryLatencyTimer;
    uint8_t IOBase;
    uint8_t IOLimit;
    uint16_t secondayStatus;
    uint16_t memoryBase;
    uint16_t memoryLimit;
    uint16_t prefetchableMemoryBase;
    uint16_t prefetchableMemoryLimit;
    uint32_t prefetchableBaseUpper;
    uint32_t prefetchableLimitUpper;
    uint16_t IOBaseUpper;
    uint16_t IOLimitUpper;
    uint8_t capabilitiesPointer;
    uint32_t expansionROM;
    uint8_t intLine;
    uint8_t intPIN;
    uint16_t bridgeControl;
} pci_config_pci_to_pci_t;

typedef struct {
    pci_location_t location;
    pci_config_header_t hdr;
    //TODO: Do this
} pci_config_pci_to_cardbus_t;

uint16_t pci_config_read_word(pci_location_t location, uint8_t offset);
void pci_config_write_word(pci_location_t location, uint8_t offset, uint16_t data);
uint32_t pci_config_read_address(pci_location_t location, uint8_t offset);

#endif //PCI_H
