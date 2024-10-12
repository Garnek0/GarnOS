#include <arch/arch-internals.h>
#include <sys/bootloader.h>

void mm_indexer(uint64_t virtAddr, int* Pi, int* PTi, int* PDi, int* PDPi){
    virtAddr >>= 12;
    *Pi = virtAddr & 0x1ff;
    virtAddr >>= 9;
    *PTi = virtAddr & 0x1ff;
    virtAddr >>= 9;
    *PDi = virtAddr & 0x1ff;
    virtAddr >>= 9;
    *PDPi = virtAddr & 0x1ff;
}

page_table_t* arch_mm_init(){
    page_table_t* initPT = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
    memset((void*)initPT, 0, sizeof(page_table_t));

    return initPT;
}

void arch_mm_map(page_table_t* pt, uint64_t physAddr, uint64_t virtAddr, uint32_t flags){
    int Pi, PTi, PDi, PDPi;
    mm_indexer(virtAddr, &Pi, &PTi, &PDi, &PDPi);

    page_table_entry_t currentEntry;

    currentEntry = pt->entries[PDPi];
    page_table_t* PDP;
    if(!currentEntry.present){
        PDP = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
        memset((void*)PDP, 0, sizeof(page_table_t));
        currentEntry.addr = (uint64_t)((uint64_t)PDP - bl_get_hhdm_offset()) >> 12;
        currentEntry.present = true;
        currentEntry.readWrite = true;
        currentEntry.userSupervisor = (flags & VMM_USER);
        pt->entries[PDPi] = currentEntry;
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
    currentEntry.writeThrough = (flags & VMM_WRITE_THROUGH);
    currentEntry.cacheDisable = (flags & VMM_CACHE_DISABLE);
    currentEntry.accessed = (flags & VMM_ACCESSED);
    currentEntry.pageSize = false;
    currentEntry.nx = (flags & VMM_EXEC_DISABLE);
    PT->entries[Pi] = currentEntry;
}

void arch_mm_unmap(page_table_t* pt, uint64_t virtAddr){
    int Pi, PTi, PDi, PDPi;
    mm_indexer(virtAddr, &Pi, &PTi, &PDi, &PDPi);

    page_table_entry_t currentEntry;

    currentEntry = pt->entries[PDPi];
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
}

void arch_mm_set_flags(page_table_t* pt, uint64_t virtAddr, uint32_t flags){
    int Pi, PTi, PDi, PDPi;
	mm_indexer(virtAddr, &Pi, &PTi, &PDi, &PDPi);

    page_table_entry_t currentEntry;

    currentEntry = pt->entries[PDPi];
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
    currentEntry.writeThrough = (flags & VMM_WRITE_THROUGH);
    currentEntry.cacheDisable = (flags & VMM_CACHE_DISABLE);
    currentEntry.accessed = (flags & VMM_ACCESSED);
    currentEntry.pageSize = false;
    currentEntry.nx = (flags & VMM_EXEC_DISABLE);
    PT->entries[Pi] = currentEntry;

    asm("invlpg (%0)" :: "r"(virtAddr) : "memory");
}

bool arch_mm_is_page_free(page_table_t* pt, uint64_t virtAddr){
    int Pi, PTi, PDi, PDPi;
    page_table_entry_t currentEntry;

    mm_indexer(virtAddr, &Pi, &PTi, &PDi, &PDPi);

    currentEntry = pt->entries[PDPi];
    page_table_t* PDP;
    if(!currentEntry.present && currentEntry.addr == 0){
        return true;
    } else {
        PDP = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
    }

    currentEntry = PDP->entries[PDi];
    page_table_t* PD;
    if(!currentEntry.present && currentEntry.addr == 0){
        return true;
    } else {
        PD = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
    }

    currentEntry = PD->entries[PTi];
    page_table_t* PT;
    if(!currentEntry.present && currentEntry.addr == 0){
        return true;
    } else {
        PT = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
    }

    currentEntry = PT->entries[Pi];
    if(!currentEntry.present && currentEntry.addr == 0){
        return true;
    } else {
        return false;
    }
}

uint64_t arch_mm_virt_to_phys(page_table_t* pt, uint64_t virtAddr){
    int Pi, PTi, PDi, PDPi;
    page_table_entry_t currentEntry;

    mm_indexer((uint64_t)virtAddr, &Pi, &PTi, &PDi, &PDPi);

    currentEntry = pt->entries[PDPi];
    page_table_t* PDP;
    if(!currentEntry.present && currentEntry.addr == 0){
        return 0;
    } else {
        PDP = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
    }

    currentEntry = PDP->entries[PDi];
    page_table_t* PD;
    if(!currentEntry.present && currentEntry.addr == 0){
        return 0;
    } else {
        PD = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
    }

    currentEntry = PD->entries[PTi];
    page_table_t* PT;
    if(!currentEntry.present && currentEntry.addr == 0){
        return 0;
    } else {
        PT = (page_table_t*)(((uint64_t)currentEntry.addr << 12) + bl_get_hhdm_offset());
    }

    currentEntry = PT->entries[Pi];
    if(!currentEntry.present && currentEntry.addr == 0){
        return 0;
    } else {
        return (uint64_t)(currentEntry.addr << 12);
    }
}

void arch_vaspace_switch(page_table_t* pt){
    asm volatile("mov %0, %%cr3" : : "r" (pt));
}

page_table_t* arch_vaspace_new(){
    page_table_t* pt = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
    memcpy(pt, vmm_get_kernel_pt(), sizeof(page_table_t));
    memset(pt, 0, (sizeof(page_table_t)/2));
    return pt;
}

void arch_vaspace_destroy(page_table_t* pt){
    for(int i = 0; i < 256; i++){
        if(pt->entries[i].present){
            page_table_t* PDP = (page_table_t*)((pt->entries[i].addr << 12) + bl_get_hhdm_offset());

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
    pmm_free((void*)((uint64_t)pt - bl_get_hhdm_offset()), 1);
}

void arch_vaspace_clear(page_table_t* pt){
    for(int i = 0; i < 256; i++){
        if(pt->entries[i].present){
            page_table_t* PDP = (page_table_t*)((pt->entries[i].addr << 12) + bl_get_hhdm_offset());

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

            pt->entries[i].present = false;
            pt->entries[i].addr = 0;
            pmm_free((void*)((uint64_t)PDP - bl_get_hhdm_offset()), 1);
        }
    }
}

page_table_t* arch_vaspace_clone(page_table_t* toClone){
    page_table_t* clone = arch_vaspace_new();
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
