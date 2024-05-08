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
#include <process/sched/sched.h>
#include <kerrno.h>
#include <kstdio.h>

void vaspace_switch(page_table_t* pml4){
    uint64_t hhdm = bl_get_hhdm_offset();
    uint64_t kernelVirtBase = bl_get_kernel_virt_base();
    if((uint64_t)pml4 > kernelVirtBase) pml4 = (page_table_t*)((uint64_t)pml4 - kernelVirtBase);
    else if((uint64_t)pml4 > hhdm) pml4 = (page_table_t*)((uint64_t)pml4 - hhdm);
    asm volatile("mov %0, %%cr3" : : "r" (pml4));
}

page_table_t* vaspace_new(){
    page_table_t* pml4 = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
    memcpy(pml4, vmm_get_kernel_pml4(), sizeof(page_table_t));
    memset(pml4, 0, (sizeof(page_table_t)/2));
    return pml4;
}

void vaspace_destroy(page_table_t* pml4){
    if(!pml4) return;
    for(int i = 0; i < 256; i++){
        if(pml4->entries[i].present){
            page_table_t* PDP = (page_table_t*)((pml4->entries[i].addr << 12) + bl_get_hhdm_offset());

            for(int j = 0; j < 512; j++){
                if(PDP && PDP->entries[j].present){
                    page_table_t* PD = (page_table_t*)((PDP->entries[j].addr << 12) + bl_get_hhdm_offset());

                    for(int k = 0; k < 512; k++){
                        if(PD && PD->entries[k].present){
                            page_table_t* PT = (page_table_t*)((PD->entries[k].addr << 12) + bl_get_hhdm_offset());

                            for(int l = 0; l < 512; l++){
                                if(PT && PT->entries[l].present){
                                    page_table_t* page = (page_table_t*)((PT->entries[l].addr << 12) + bl_get_hhdm_offset());
                                    
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
    pmm_free((void*)((uint64_t)pml4 - bl_get_hhdm_offset()), 1);
}

void vaspace_clear(page_table_t* pml4){
    if(!pml4) return;
    for(int i = 0; i < 256; i++){
        if(pml4->entries[i].present){
            page_table_t* PDP = (page_table_t*)((pml4->entries[i].addr << 12) + bl_get_hhdm_offset());

            for(int j = 0; j < 512; j++){
                if(PDP && PDP->entries[j].present){
                    page_table_t* PD = (page_table_t*)((PDP->entries[j].addr << 12) + bl_get_hhdm_offset());

                    for(int k = 0; k < 512; k++){
                        if(PD && PD->entries[k].present){
                            page_table_t* PT = (page_table_t*)((PD->entries[k].addr << 12) + bl_get_hhdm_offset());

                            for(int l = 0; l < 512; l++){
                                if(PT && PT->entries[l].present){
                                    page_table_t* page = (page_table_t*)((PT->entries[l].addr << 12) + bl_get_hhdm_offset());
                                    
                                    PT->entries[l].present = false;
                                    PT->entries[l].addr = 0;
                                    pmm_free((void*)((uint64_t)page - bl_get_hhdm_offset()), 1);
                                }
                            }
                            PD->entries[k].present = false;
                            PD->entries[k].addr = 0;
                            pmm_free((void*)((uint64_t)PT - bl_get_hhdm_offset()), 1);
                        }
                    }
                    PDP->entries[j].present = false;
                    PDP->entries[j].addr = 0;
                    pmm_free((void*)((uint64_t)PD - bl_get_hhdm_offset()), 1);
                }
            }

            pml4->entries[i].present = false;
            pml4->entries[i].addr = 0;
            pmm_free((void*)((uint64_t)PDP - bl_get_hhdm_offset()), 1);
        }
    }
}

page_table_t* vaspace_clone(page_table_t* toClone){
    page_table_t* clone = vaspace_new();
    for(int i = 0; i < 256; i++){
        if(toClone->entries[i].present){
            page_table_t* PDP = (page_table_t*)((toClone->entries[i].addr << 12) + bl_get_hhdm_offset());
            memcpy(&clone->entries[i], &toClone->entries[i], sizeof(page_table_entry_t));
            page_table_t* newPDP = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
            memset(newPDP, 0, sizeof(page_table_t));
            clone->entries[i].addr = (((uint64_t)newPDP - bl_get_hhdm_offset()) >> 12);

            for(int j = 0; j < 512; j++){
                if(PDP->entries[j].present){
                    page_table_t* PD = (page_table_t*)((PDP->entries[j].addr << 12) + bl_get_hhdm_offset());
                    memcpy(&newPDP->entries[j], &PDP->entries[j], sizeof(page_table_entry_t));
                    page_table_t* newPD = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
                    memset(newPD, 0, sizeof(page_table_t));
                    newPDP->entries[j].addr = (((uint64_t)newPD - bl_get_hhdm_offset()) >> 12);

                    for(int k = 0; k < 512; k++){
                        if(PD->entries[k].present){
                            page_table_t* PT = (page_table_t*)((PD->entries[k].addr << 12) + bl_get_hhdm_offset());
                            memcpy(&newPD->entries[k], &PD->entries[k], sizeof(page_table_entry_t));
                            page_table_t* newPT = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
                            memset(newPT, 0, sizeof(page_table_t));
                            newPD->entries[k].addr = (((uint64_t)newPT - bl_get_hhdm_offset()) >> 12);

                            for(int l = 0; l < 512; l++){
                                if(PT->entries[l].present){
                                    page_table_entry_t* page = &PT->entries[l];
                                    void* pagePhys = (void*)((uint64_t)(page->addr << 12));
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

void* sys_mmap(stack_frame_t* regs, void* addr, size_t length, int prot, int flags, int fd, uint64_t offset){
    if(length == 0) return (void*)-EINVAL;
    if((uint64_t)addr > VMM_USER_END) return (void*)-ENOMEM;

    //We dont support shared mappings yet
    if(flags & MAP_SHARED) return (void*)-EINVAL;
    if(!(flags & MAP_PRIVATE)) return (void*)-EINVAL;

    //We also dont support file mappings yet
    if(!(flags & MAP_ANONYMOUS)) return (void*)-EINVAL;

    process_t* currentProcess = sched_get_current_process();

    if(addr != NULL && (uint64_t)addr%PAGE_SIZE != 0) ALIGN_UP(addr, PAGE_SIZE);
    if(addr == NULL) addr = (void*)0x1000;

    if(length%PAGE_SIZE != 0) ALIGN_UP(length, PAGE_SIZE);
    length /= PAGE_SIZE;

    int Pi, PTi, PDi, PDPi;
    page_table_entry_t currentEntry;
    size_t npages = 0;

    uint64_t userEnd = VMM_USER_END;
    if(flags & MAP_32BIT) userEnd = VMM_USER_END_32BIT;

    for(uint64_t i = (uint64_t)addr; i < userEnd; i+=PAGE_SIZE){
        vmm_indexer(i, &Pi, &PTi, &PDi, &PDPi);

        currentEntry = currentProcess->pml4->entries[PDPi];
        page_table_t* PDP;
        if(!currentEntry.present && currentEntry.addr == 0){
            goto found_addr;
        } else {
            PDP = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
        }

        currentEntry = PDP->entries[PDi];
        page_table_t* PD;
        if(!currentEntry.present && currentEntry.addr == 0){
            goto found_addr;
        } else {
            PD = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
        }

        currentEntry = PD->entries[PTi];
        page_table_t* PT;
        if(!currentEntry.present && currentEntry.addr == 0){
            goto found_addr;
        } else {
            PT = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
        }

        currentEntry = PT->entries[Pi];
        if(!currentEntry.present && currentEntry.addr == 0){
            goto found_addr;
        } else {
            npages = 0;
            continue;
        }

        continue;

found_addr:
        npages++;
        if(npages == 1) addr = (void*)i;
        if(npages == length){
            uint64_t phys;
            uint64_t vmmflags = 0;

            vmmflags = (VMM_USER | VMM_PRESENT | VMM_NX);

            //if(prot & PROT_READ);
            if(prot & PROT_WRITE) vmmflags |= VMM_RW;
            if(prot & PROT_EXEC) vmmflags &= ~(VMM_NX);
            
            for(size_t j = 0; j < length; j++){
                phys = (uint64_t)pmm_allocate(1);
                vmm_map(currentProcess->pml4, phys, (uint64_t)addr+(j*PAGE_SIZE), vmmflags);
            }
            return addr;
        } else continue;
    }

    return (void*)-ENOMEM;
}

int sys_munmap(stack_frame_t* regs, void* addr, size_t length){
    if(length == 0) return -EINVAL;
    if((uint64_t)addr > VMM_USER_END || (uint64_t)addr%PAGE_SIZE != 0) return -EINVAL;

    if(length%PAGE_SIZE != 0) ALIGN_UP(length, PAGE_SIZE);
    length /= PAGE_SIZE;

    process_t* currentProcess = sched_get_current_process();

    int Pi, PTi, PDi, PDPi;
    page_table_entry_t currentEntry;
    for(size_t i = 0; i < length; i++){
        vmm_indexer((uint64_t)addr, &Pi, &PTi, &PDi, &PDPi);

        currentEntry = currentProcess->pml4->entries[PDPi];
        page_table_t* PDP;
        if(!currentEntry.present && currentEntry.addr == 0){
            return -EINVAL;
        } else {
            PDP = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
        }

        currentEntry = PDP->entries[PDi];
        page_table_t* PD;
        if(!currentEntry.present && currentEntry.addr == 0){
            return -EINVAL;
        } else {
            PD = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
        }

        currentEntry = PD->entries[PTi];
        page_table_t* PT;
        if(!currentEntry.present && currentEntry.addr == 0){
            return -EINVAL;
        } else {
            PT = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
        }

        currentEntry = PT->entries[Pi];
        if(!currentEntry.present && currentEntry.addr == 0){
            return -EINVAL;
        } else {
            pmm_free((void*)((uint64_t)(currentEntry.addr << 12)), 1);
            vmm_unmap(currentProcess->pml4, (uint64_t)addr);
        }

        addr += PAGE_SIZE;
    }

    return -ENOMEM;
}