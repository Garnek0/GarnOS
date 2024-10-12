#include <garn/hw/pci.h>
#include <garn/arch.h>
#include <garn/kstdio.h>
#include <exec/elf.h>
#include <garn/mm.h>
#include <garn/dal/dal.h>
#include <garn/config.h>

#ifdef CONFIG_INCLUDE_PCI_DRIVER

uint16_t pci_config_read_word(pci_location_t location, uint8_t offset){
    uint32_t address;
    uint32_t bus  = (uint32_t)location.bus;
    uint32_t dev = (uint32_t)location.dev;
    uint32_t func = (uint32_t)location.func;
    uint16_t data = 0;
 
    address = (uint32_t)((bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    arch_outl(PCI_CONFIG_ADDRESS, address);

    data = (uint16_t)((arch_inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return data;
}

void pci_config_write_word(pci_location_t location, uint8_t offset, uint16_t data){
    uint32_t address;
    uint32_t bus  = (uint32_t)location.bus;
    uint32_t dev = (uint32_t)location.dev;
    uint32_t func = (uint32_t)location.func;
 
    address = (uint32_t)((bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    arch_outl(PCI_CONFIG_ADDRESS, address);

    arch_outl(PCI_CONFIG_DATA, data >> ((offset & 2) * 8));
}

uint32_t pci_config_read_address(pci_location_t location, uint8_t offset){
    uint32_t address = pci_config_read_word(location, offset);
    address |= (((uint32_t)pci_config_read_word(location, offset+2) << 16) & 0xFFFF0000);

    return address;
}

#endif //CONFIG_INCLUDE_PCI_DRIVER
