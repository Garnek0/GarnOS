#include <mem/mm-internals.h>
#include <sys/bootloader.h>
#include <limine.h>
#include <garn/kstdio.h>
#include <arch/arch-internals.h>

page_table_t* kernelPT;

spinlock_t VMMlock; //vmm lock

void vmm_init(){
    kernelPT = arch_mm_init();
    
    //map kernel

    memmap_entry_t entry;
    for(int i = 0; i < memmap_get_entry_count(); i++){
        entry = memmap_get_entry(i);
        if(entry.type != MEMMAP_KERNEL_AND_MODULES) continue;

        for(uint64_t i = 0; i <= ALIGN_UP(entry.length, PAGE_SIZE); i+=PAGE_SIZE){
            vmm_map(kernelPT, i+bl_get_kernel_phys_base(), i+bl_get_kernel_virt_base(), (VMM_PRESENT | VMM_RW));
        }
    }

    //map first 4 GiB
    for(uint64_t i = 0x1000; i <= 0xffffffff; i+=PAGE_SIZE){
        vmm_map(kernelPT, i, i, 0x3);
        vmm_map(kernelPT, i, i+bl_get_hhdm_offset(), (VMM_PRESENT | VMM_RW));
    }

    //map any additional memmap entry
    memmap_entry_t currentEntry;
    for(int i = 0; i < memmap_get_entry_count(); i++){
        currentEntry = memmap_get_entry(i);

        if(currentEntry.base+currentEntry.length <= 0xffffffff) continue;

        for(uint64_t j = 0; j < ALIGN_UP(currentEntry.length, PAGE_SIZE); j+=PAGE_SIZE){
            vmm_map(kernelPT, j+currentEntry.base, j+currentEntry.base, (VMM_PRESENT | VMM_RW));
            vmm_map(kernelPT, j+currentEntry.base, j+currentEntry.base+bl_get_hhdm_offset(), (VMM_PRESENT | VMM_RW));
        }
    }
    vaspace_switch(kernelPT);
}

page_table_t* vmm_get_kernel_pt(){
    return kernelPT;
}

void vmm_map(page_table_t* pt, uint64_t physAddr, uint64_t virtAddr, uint32_t flags){
    lock(VMMlock, {
        arch_mm_map(pt, physAddr, virtAddr, flags);
    });
}

void vmm_map_range(page_table_t* pt, uint64_t physAddr, uint64_t virtAddr, size_t length, uint32_t flags){
    for(size_t i = 0; i < length; i+=PAGE_SIZE){
        vmm_map(pt, (uint64_t)physAddr, (uint64_t)virtAddr, flags);
        physAddr += PAGE_SIZE;
        virtAddr += PAGE_SIZE;
    }
}

void vmm_unmap(page_table_t* pt, uint64_t virtAddr){
    lock(VMMlock, {
        arch_mm_unmap(pt, virtAddr);
    });
}

void vmm_unmap_range(page_table_t* pt, uint64_t virtAddr, size_t length){
    for(size_t i = 0; i < length; i+=PAGE_SIZE){
        vmm_unmap(pt, (uint64_t)virtAddr);
        virtAddr += PAGE_SIZE;
    }
}

void vmm_set_flags(page_table_t* pt, uint64_t virtAddr, uint32_t flags){
	lock(VMMlock, {
        arch_mm_set_flags(pt, virtAddr, flags);
    });
}

void vmm_set_flags_range(page_table_t* pt, uint64_t virtAddr, size_t length, uint32_t flags){
    for(size_t i = 0; i < length; i+=PAGE_SIZE){
        vmm_set_flags(pt, virtAddr, flags);
        virtAddr += PAGE_SIZE;
    }
}

bool vmm_is_page_free(page_table_t* pt, uint64_t virtAddr){
    bool result;
    lock(VMMlock, {
        result = arch_mm_is_page_free(pt, virtAddr);
    });
    return result;
}

uint64_t vmm_virt_to_phys(page_table_t* pt, uint64_t virtAddr){
    uint64_t result;
    lock(VMMlock, {
        result = arch_mm_virt_to_phys(pt, virtAddr);
    });
    return result;
}
