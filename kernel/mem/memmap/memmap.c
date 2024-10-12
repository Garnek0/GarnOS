/*  
*   File: memmap.c
*
*   Author: Garnek
*   
*   Description: Memory Map fetching from the bootloader and additional functions
*                to calculate things like the highest usable physical memory address
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <mem/mm-internals.h>
#include <limine.h>
#include <garn/kstdio.h>

static char* memmap_type_strings[] = {
    "USABLE",
    "RESERVED",
    "ACPI_RECLAIMABLE",
    "ACPI_NVS",
    "BAD_MEMORY",
    "BOOTLOADER_RECLAIMABLE",
    "KERNEL_AND_MODULES",
    "FRAMEBUFFER"
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

void memmap_print(){
    struct limine_memmap_response* memmap = memmap_request.response;
    klog("Printing memmap...\n", KLOG_INFO, "memmap");
    for(uint64_t i = 0; i < memmap->entry_count; i++){
        klog("[0x%x->0x%x] %uB %s\n", KLOG_INFO, "memmap", memmap->entries[i]->base, memmap->entries[i]->base + memmap->entries[i]->length, memmap->entries[i]->length, memmap_type_strings[memmap->entries[i]->type]);
    }
}

int memmap_get_entry_count(){
    return memmap_request.response->entry_count;
}

uint64_t memmap_get_highest_usable_address(){
    uint64_t highest = 0;
    struct limine_memmap_response* memmap = memmap_request.response;
    for(uint64_t i = 0; i < memmap->entry_count; i++){
        if(memmap->entries[i]->type == LIMINE_MEMMAP_USABLE){
            highest = memmap->entries[i]->base+memmap->entries[i]->length;
        }
    }
    return highest;
}

uint64_t memmap_get_highest_address(){
    uint64_t highest = 0;
    struct limine_memmap_response* memmap = memmap_request.response;
    highest = memmap->entries[memmap->entry_count-1]->base + memmap->entries[memmap->entry_count-1]->length;
    return highest;
}


memmap_entry_t memmap_get_entry(int entry){
    memmap_entry_t temp_entry;
    temp_entry.base = memmap_request.response->entries[entry]->base;
    temp_entry.length = memmap_request.response->entries[entry]->length;
    temp_entry.type = memmap_request.response->entries[entry]->type;
    return temp_entry;
}
