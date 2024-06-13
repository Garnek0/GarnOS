/*  
*   File: pci.c
*
*   Author: Garnek
*   
*   Description: PCI Driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <garn/hw/pci.h>
#include <garn/hw/ports.h>
#include <garn/kstdio.h>
#include <exec/elf.h>
#include <garn/mm.h>
#include <garn/dal/dal.h>

uint16_t pci_config_read_word(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t busl  = (uint32_t)bus;
    uint32_t devl = (uint32_t)dev;
    uint32_t funcl = (uint32_t)func;
    uint16_t data = 0;
 
    address = (uint32_t)((busl << 16) | (devl << 11) | (funcl << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    outl(PCI_CONFIG_ADDRESS, address);

    data = (uint16_t)((inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return data;
}

uint32_t pci_config_read_address(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    uint32_t address = pci_config_read_word(bus, dev, func, offset);
    address |= (((uint32_t)pci_config_read_word(bus, dev, func, offset+2) << 16) & 0xFFFF0000);

    return address;
}