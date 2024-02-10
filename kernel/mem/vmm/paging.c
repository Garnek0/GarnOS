/*  
*   File: vmm.c
*
*   Author: Garnek
*   
*   Description: Virtual Memory Manager
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "vmm.h"
#include <mem/pmm/pmm.h>
#include <mem/memutil/memutil.h>
#include <mem/memmap/memmap.h>
#include <sys/bootloader.h>
#include <limine.h>
#include <kstdio.h>

page_table_t* PML4;
page_table_t* kernelPML4;

spinlock_t VMMlock; //vmm lock

void vmm_indexer(uint64_t virtAddr, int* Pi, int* PTi, int* PDi, int* PDPi){
    virtAddr >>= 12;
    *Pi = virtAddr & 0x1ff;
    virtAddr >>= 9;
    *PTi = virtAddr & 0x1ff;
    virtAddr >>= 9;
    *PDi = virtAddr & 0x1ff;
    virtAddr >>= 9;
    *PDPi = virtAddr & 0x1ff;
}

void vmm_init(){
    kernelPML4 = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
    memset((void*)kernelPML4, 0, PAGE_SIZE);
    
    //map kernel

    memmap_entry_t entry;
    for(size_t i = 0; i < memmap_get_entry_count(); i++){
        entry = memmap_get_entry(i);
        if(entry.type != MEMMAP_KERNEL_AND_MODULES) continue;

        for(uint64_t i = 0; i <= ALIGN_UP(entry.length, PAGE_SIZE); i+=PAGE_SIZE){
            vmm_map(kernelPML4, i+bl_get_kernel_phys_base(), i+bl_get_kernel_virt_base(), 0x3);
        }
    }

    //map first 4 GiB
    for(uint64_t i = 0x1000; i <= 0xffffffff; i+=PAGE_SIZE){
        vmm_map(kernelPML4, i, i, 0x3);
        vmm_map(kernelPML4, i, i+bl_get_hhdm_offset(), 0x3);
    }

    //map any additional memmap entry
    memmap_entry_t currentEntry;
    for(int i = 0; i < memmap_get_entry_count(); i++){
        currentEntry = memmap_get_entry(i);

        if(currentEntry.base+currentEntry.length <= 0xffffffff) continue;

        for(uint64_t j = 0; j < ALIGN_UP(currentEntry.length, PAGE_SIZE); j+=PAGE_SIZE){
            vmm_map(kernelPML4, j+currentEntry.base, j+currentEntry.base, 0x3);
            vmm_map(kernelPML4, j+currentEntry.base, j+currentEntry.base+bl_get_hhdm_offset(), 0x3);
        }
    }
    vaspace_switch(kernelPML4);
}

inline page_table_t* vmm_get_kernel_pml4(){
    return kernelPML4;
}

void vmm_map(page_table_t* pml4, uint64_t physAddr, uint64_t virtAddr, uint32_t flags){
    int Pi, PTi, PDi, PDPi;
    vmm_indexer(virtAddr, &Pi, &PTi, &PDi, &PDPi);

    page_table_entry_t currentEntry;

    lock(VMMlock, {
        currentEntry = pml4->entries[PDPi];
        page_table_t* PDP;
        if(!currentEntry.present){
            PDP = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
            memset((void*)PDP, 0, sizeof(page_table_t));
            currentEntry.addr = (uint64_t)((uint64_t)PDP - bl_get_hhdm_offset()) >> 12;
            currentEntry.present = true;
            currentEntry.readWrite = true;
            currentEntry.userSupervisor = (flags & VMM_USER);
            pml4->entries[PDPi] = currentEntry;
        } else {
            PDP = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
        }

        currentEntry = PDP->entries[PDi];
        page_table_t* PD;
        if(!currentEntry.present){
            PD = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
            memset((void*)PD, 0, sizeof(page_table_t));
            currentEntry.addr = (uint64_t)((uint64_t)PD - bl_get_hhdm_offset()) >> 12;
            currentEntry.present = true;
            currentEntry.readWrite = true;
            currentEntry.userSupervisor = (flags & VMM_USER);
            PDP->entries[PDi] = currentEntry;
        } else {
            PD = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
        }

        currentEntry = PD->entries[PTi];
        page_table_t* PT;
        if(!currentEntry.present){
            PT = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
            memset((void*)PT, 0, sizeof(page_table_t));
            currentEntry.addr = (uint64_t)((uint64_t)PT - bl_get_hhdm_offset()) >> 12;
            currentEntry.present = true;
            currentEntry.readWrite = true;
            currentEntry.userSupervisor = (flags & VMM_USER);
            PD->entries[PTi] = currentEntry;
        } else {
            PT = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
        }

        currentEntry = PT->entries[Pi];
        currentEntry.addr = (uint64_t)physAddr >> 12;
        currentEntry.present = (flags & VMM_PRESENT);
        currentEntry.readWrite = (flags & VMM_RW);
        currentEntry.userSupervisor = (flags & VMM_USER);
        currentEntry.writeThrough = (flags & VMM_PWT);
        currentEntry.cacheDisable = (flags & VMM_PCD);
        currentEntry.accessed = (flags & VMM_ACCESSED);
        currentEntry.pageSize = (flags & VMM_PS);
        currentEntry.nx = (flags & VMM_NX);
        PT->entries[Pi] = currentEntry;
    });
}

void vmm_unmap(page_table_t* pml4, uint64_t virtAddr){
    int Pi, PTi, PDi, PDPi;
    vmm_indexer(virtAddr, &Pi, &PTi, &PDi, &PDPi);

    page_table_entry_t currentEntry;

    lock(VMMlock, {
        currentEntry = pml4->entries[PDPi];
        page_table_t* PDP;
        PDP = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());

        currentEntry = PDP->entries[PDi];
        page_table_t* PD;
        PD = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());


        currentEntry = PD->entries[PTi];
        page_table_t* PT;
        PT = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());

        currentEntry = PT->entries[Pi];
        currentEntry.present = 0;
        currentEntry.addr = 0;
        PT->entries[Pi] = currentEntry;

        asm("invlpg (%0)" :: "r"(virtAddr) : "memory");
    });
}

void vmm_set_flags(page_table_t* pml4, uint64_t virtAddr, uint32_t flags){
	int Pi, PTi, PDi, PDPi;
	vmm_indexer(virtAddr, &Pi, &PTi, &PDi, &PDPi);

    page_table_entry_t currentEntry;

    lock(VMMlock, {
        currentEntry = pml4->entries[PDPi];
        page_table_t* PDP;
        PDP = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());

        currentEntry = PDP->entries[PDi];
        page_table_t* PD;
        PD = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());

        currentEntry = PD->entries[PTi];
        page_table_t* PT;
        PT = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());

        currentEntry = PT->entries[Pi];
        currentEntry.present = (flags & VMM_PRESENT);
        currentEntry.readWrite = (flags & VMM_RW);
        currentEntry.userSupervisor = (flags & VMM_USER);
        currentEntry.writeThrough = (flags & VMM_PWT);
        currentEntry.cacheDisable = (flags & VMM_PCD);
        currentEntry.accessed = (flags & VMM_ACCESSED);
        currentEntry.pageSize = (flags & VMM_PS);
        currentEntry.nx = (flags & VMM_NX);
        PT->entries[Pi] = currentEntry;

        asm("invlpg (%0)" :: "r"(virtAddr) : "memory");
    });
}