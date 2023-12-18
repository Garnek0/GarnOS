/*  
*   File: vmm.c
*
*   Author: Garnek
*   
*   Description: Virtual Memory Manager
*/
// SPDX-License-Identifier: BSD-2-Clause

//TODO: Just like the kheap, this thing is a mess too. I should probably clean this up a little.

#include "vmm.h"
#include <mem/mm/pmm.h>
#include <mem/memutil/memutil.h>
#include <mem/memmap/memmap.h>
#include <sys/bootloader.h>
#include <limine.h>
#include <kstdio.h>

page_table_t* PML4;
page_table_t* kernelPML4;

spinlock_t lock; //vmm lock

static void vmm_indexer(uint64_t virtAddr, int* Pi, int* PTi, int* PDi, int* PDPi){
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
    PML4 = pmm_allocate(1);
    memset((void*)PML4, 0, PAGE_SIZE);
    
    //map kernel

    memmap_entry_t entry;
    for(size_t i = 0; i < memmap_get_entry_count(); i++){
        entry = memmap_get_entry(i);
        if(entry.type != MEMMAP_KERNEL_AND_MODULES) continue;

        for(uint64_t i = 0; i <= ALIGN_UP(entry.length, PAGE_SIZE); i+=PAGE_SIZE){
            vmm_map(i+bl_get_kernel_phys_base(), i+bl_get_kernel_virt_base(), 0x3);
        }
    }

	klog("VMM: Mapped 0x%x->0x%x to 0x%x->0x%x\n", KLOG_INFO, bl_get_kernel_phys_base(), ALIGN_UP((bl_get_kernel_phys_base()+bl_get_kernel_file_size()), PAGE_SIZE), bl_get_kernel_virt_base(), ALIGN_UP((bl_get_kernel_virt_base()+bl_get_kernel_file_size()), PAGE_SIZE));

    //map first 4 GiB
    for(uint64_t i = 0x1000; i <= 0xffffffff; i+=PAGE_SIZE){
        vmm_map(i, i, 0x3);
        vmm_map(i, i+bl_get_hhdm_offset(), 0x3);
    }
	klog("VMM: Mapped 0x%x->0x%x to 0x%x->0x%x\n", KLOG_INFO, 0x1000, 0xffffffff, 0x1000, 0xffffffff);
	klog("VMM: Mapped 0x%x->0x%x to 0x%x->0x%x\n", KLOG_INFO, 0x1000, 0xffffffff, 0x1000+bl_get_hhdm_offset(), 0xffffffff+bl_get_hhdm_offset());

    //map any additional memmap entry
    memmap_entry_t currentEntry;
    for(int i = 0; i < memmap_get_entry_count(); i++){
        currentEntry = memmap_get_entry(i);

        if(currentEntry.base+currentEntry.length <= 0xffffffff) continue;

        for(uint64_t j = 0; j < ALIGN_UP(currentEntry.length, PAGE_SIZE); j+=PAGE_SIZE){
            vmm_map(j+currentEntry.base, j+currentEntry.base, 0x3);
            vmm_map(j+currentEntry.base, j+currentEntry.base+bl_get_hhdm_offset(), 0x3);
        }
		klog("VMM: Mapped 0x%x->0x%x to 0x%x->0x%x\n", KLOG_INFO, currentEntry.base, currentEntry.base+currentEntry.length, currentEntry.base, currentEntry.base+currentEntry.length);
		klog("VMM: Mapped 0x%x->0x%x to 0x%x->0x%x\n", KLOG_INFO, currentEntry.base, currentEntry.base+currentEntry.length, currentEntry.base+bl_get_hhdm_offset(), currentEntry.base+currentEntry.length+bl_get_hhdm_offset());
    }

    kernelPML4 = PML4;

    vmm_switch_address_space(kernelPML4);
	klog("VMM: Initialised Virtual Memory Manager (CR3: 0x%x).\n", KLOG_OK, kernelPML4);
}

void vmm_switch_address_space(page_table_t* pml4){
    PML4 = pml4;
    uint64_t hhdm = bl_get_hhdm_offset();
    if(PML4 > hhdm) PML4 = (page_table_t*)((uint64_t)PML4 - hhdm);
    asm volatile("mov %0, %%cr3" : : "r" (PML4));
}

page_table_t* vmm_get_current_address_space(){
    return PML4;
}

void vmm_map(uint64_t physAddr, uint64_t virtAddr, uint32_t flags){
    int Pi, PTi, PDi, PDPi;
    vmm_indexer(virtAddr, &Pi, &PTi, &PDi, &PDPi);

    page_table_entry_t currentEntry;

    lock(lock, {
        currentEntry = PML4->entries[PDPi];
        page_table_t* PDP;
        if(!currentEntry.present){
            PDP = pmm_allocate(1);
            memset((void*)PDP, 0, PAGE_SIZE);
            currentEntry.addr = (uint64_t)PDP >> 12;
            currentEntry.present = true;
            currentEntry.readWrite = true;
            PML4->entries[PDPi] = currentEntry;
        } else {
            PDP = (page_table_t*)((uint64_t)currentEntry.addr << 12);
        }

        currentEntry = PDP->entries[PDi];
        page_table_t* PD;
        if(!currentEntry.present){
            PD = pmm_allocate(1);
            memset((void*)PD, 0, PAGE_SIZE);
            currentEntry.addr = (uint64_t)PD >> 12;
            currentEntry.present = true;
            currentEntry.readWrite = true;
            PDP->entries[PDi] = currentEntry;
        } else {
            PD = (page_table_t*)((uint64_t)currentEntry.addr << 12);
        }

        currentEntry = PD->entries[PTi];
        page_table_t* PT;
        if(!currentEntry.present){
            PT = pmm_allocate(1);
            memset((void*)PT, 0, PAGE_SIZE);
            currentEntry.addr = (uint64_t)PT >> 12;
            currentEntry.present = true;
            currentEntry.readWrite = true;
            PD->entries[PTi] = currentEntry;
        } else {
            PT = (page_table_t*)((uint64_t)currentEntry.addr << 12);
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

void vmm_unmap(uint64_t virtAddr){
    int Pi, PTi, PDi, PDPi;
    vmm_indexer(virtAddr, &Pi, &PTi, &PDi, &PDPi);

    page_table_entry_t currentEntry;

    lock(lock, {
        currentEntry = PML4->entries[PDPi];
        page_table_t* PDP;
        PDP = (page_table_t*)((uint64_t)currentEntry.addr << 12);

        currentEntry = PDP->entries[PDi];
        page_table_t* PD;
        PD = (page_table_t*)((uint64_t)currentEntry.addr << 12);


        currentEntry = PD->entries[PTi];
        page_table_t* PT;
        PT = (page_table_t*)((uint64_t)currentEntry.addr << 12);

        currentEntry = PT->entries[Pi];
        currentEntry.present = 0;
        PT->entries[Pi] = currentEntry;

        asm("invlpg (%0)" :: "r"(virtAddr) : "memory");
    });
}

void vmm_set_flags(uint64_t virtAddr, uint32_t flags){
	int Pi, PTi, PDi, PDPi;
	vmm_indexer(virtAddr, &Pi, &PTi, &PDi, &PDPi);

    page_table_entry_t currentEntry;

    lock(lock, {
        currentEntry = PML4->entries[PDPi];
        page_table_t* PDP;
        PDP = (page_table_t*)((uint64_t)currentEntry.addr << 12);

        currentEntry = PDP->entries[PDi];
        page_table_t* PD;
        PD = (page_table_t*)((uint64_t)currentEntry.addr << 12);

        currentEntry = PD->entries[PTi];
        page_table_t* PT;
        PT = (page_table_t*)((uint64_t)currentEntry.addr << 12);

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