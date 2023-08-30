#ifndef MEMMAP_H
#define MEMMAP_H

#include <types.h>

#define MEMMAP_USABLE                 0
#define MEMMAP_RESERVED               1
#define MEMMAP_ACPI_RECLAIMABLE       2
#define MEMMAP_ACPI_NVS               3
#define MEMMAP_BAD_MEMORY             4
#define MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define MEMMAP_KERNEL_AND_MODULES     6
#define MEMMAP_FRAMEBUFFER            7

typedef struct {
    uint64_t base;
    uint64_t length;
    uint8_t type;
} memmap_entry_t;

void memmap_print();
int memmap_get_entry_count();
memmap_entry_t memmap_get_entry(int entry);
uint64_t memmap_get_highest_usable_address();
uint64_t memmap_get_highest_address();

#endif //MEMMAP_H