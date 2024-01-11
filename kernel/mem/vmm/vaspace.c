/*  
*   File: vaspace.c
*
*   Author: Garnek
*   
*   Description: Management of process virtual address spaces
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "vmm.h"
#include <mem/pmm/pmm.h>
#include <mem/memutil/memutil.h>
#include <sys/bootloader.h>
#include <kstdio.h>

void vaspace_switch(page_table_t* pml4){
    uint64_t hhdm = bl_get_hhdm_offset();
    uint64_t kernelVirtBase = bl_get_kernel_virt_base();
    if(pml4 > kernelVirtBase) pml4 = (page_table_t*)((uint64_t)pml4 - kernelVirtBase);
    else if(pml4 > hhdm) pml4 = (page_table_t*)((uint64_t)pml4 - hhdm);
    asm volatile("mov %0, %%cr3" : : "r" (pml4));
}

void vaspace_new(process_t* process){
    process->pml4 = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
    memcpy(process->pml4, vmm_get_kernel_pml4(), sizeof(page_table_t));
    memset(process->pml4, 0, (sizeof(page_table_t)/2));
}

page_table_t* vaspace_new_raw(){
    page_table_t* pml4 = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
    memcpy(pml4, vmm_get_kernel_pml4(), sizeof(page_table_t));
    memset(pml4, 0, (sizeof(page_table_t)/2));
    return pml4;
}

void vaspace_destroy(process_t* process){
    page_table_t* pml4 = process->pml4;
    if(!pml4) return;
    for(int i = 0; i < 256; i++){
        if(pml4->entries[i].present){
            page_table_t* PDP = (page_table_t*)((pml4->entries[i].addr << 12) + bl_get_hhdm_offset());

            for(int j = 0; j < 512; j++){
                if(PDP->entries[j].present){
                    page_table_t* PD = (page_table_t*)((PDP->entries[j].addr << 12) + bl_get_hhdm_offset());

                    for(int k = 0; k < 512; k++){
                        if(PD->entries[k].present){
                            page_table_t* PT = (page_table_t*)((PD->entries[k].addr << 12) + bl_get_hhdm_offset());

                            for(int l = 0; l < 512; l++){
                                if(PT->entries[l].present){
                                    page_table_t* page = (page_table_t*)((PT->entries[k].addr << 12) + bl_get_hhdm_offset());\
                                    
                                    pmm_free((void*)((uint64_t)page - bl_get_hhdm_offset()), 1);
                                }
                            }
                            pmm_free((void*)((uint64_t)PT - bl_get_hhdm_offset()), 1);
                        }
                    }
                    pmm_free((void*)((uint64_t)PD - bl_get_hhdm_offset()), 1);
                }
            }

            pmm_free((void*)((uint64_t)PDP - bl_get_hhdm_offset()), 1);
        }
    }
    kmfree(process->pml4);
}

page_table_t* vaspace_clone(page_table_t* toClone){
    page_table_t* clone = vaspace_new_raw();
    for(int i = 0; i < 256; i++){
        if(toClone->entries[i].present){
            page_table_t* PDP = (page_table_t*)((toClone->entries[i].addr << 12) + bl_get_hhdm_offset());
            memcpy(&clone->entries[i], &toClone->entries[i], sizeof(page_table_entry_t));
            page_table_t* newPDP = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
            clone->entries[i].addr = (((uint64_t)newPDP - bl_get_hhdm_offset()) >> 12);

            for(int j = 0; j < 512; j++){
                if(PDP->entries[j].present){
                    page_table_t* PD = (page_table_t*)((PDP->entries[j].addr << 12) + bl_get_hhdm_offset());
                    memcpy(&newPDP->entries[j], &PDP->entries[j], sizeof(page_table_entry_t));
                    page_table_t* newPD = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
                    newPDP->entries[j].addr = (((uint64_t)newPD - bl_get_hhdm_offset()) >> 12);

                    for(int k = 0; k < 512; k++){
                        if(PD->entries[k].present){
                            page_table_t* PT = (page_table_t*)((PD->entries[k].addr << 12) + bl_get_hhdm_offset());
                            memcpy(&newPD->entries[k], &PD->entries[k], sizeof(page_table_entry_t));
                            page_table_t* newPT = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
                            newPD->entries[k].addr = (((uint64_t)newPT - bl_get_hhdm_offset()) >> 12);

                            for(int l = 0; l < 512; l++){
                                if(PT->entries[l].present){
                                    page_table_entry_t* page = &PT->entries[l];
                                    void* pagePhys = (void*)(page->addr << 12);
                                    void* newPagePhys = pmm_allocate(1);

                                    memcpy((void*)((uint64_t)newPagePhys + bl_get_hhdm_offset()), (void*)((uint64_t)pagePhys + bl_get_hhdm_offset()), PAGE_SIZE);
                                    memcpy(&newPT->entries[l], &PT->entries[l], sizeof(page_table_entry_t));
                                    newPT->entries[l].addr = ((uint64_t)newPagePhys >> 12);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return clone;
}

void vaspace_create_thread_user_stack(thread_t* thread){
    //FIXME:
    //FIXME:
    //FIXME: VERY IMPORTANT! This will blow up when creating more than 1 thread/process.
    thread->regs.rsp = (uint64_t)vaspace_create_area(thread->process->pml4, (VMM_USER_END - VMM_INIT_USER_STACK_SIZE),
                                        VMM_INIT_USER_STACK_SIZE, VMM_PRESENT | VMM_RW | VMM_USER) + ((VMM_INIT_USER_STACK_SIZE - 1) - 15);
}

void* vaspace_create_area(page_table_t* pml4, uint64_t virtAddr, size_t size, uint32_t flags){
    if(size%PAGE_SIZE != 0) size = ALIGN_UP(size, PAGE_SIZE);

    void* physAddr;

    for(size_t i = 0; i < size; i+=PAGE_SIZE){
        physAddr = pmm_allocate(1);
        vmm_map(pml4, (uint64_t)physAddr, (uint64_t)virtAddr+i, flags);
    }

    return (void*)virtAddr;
}