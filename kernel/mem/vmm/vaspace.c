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

void vaspace_switch(page_table_t* pml4){
    uint64_t hhdm = bl_get_hhdm_offset();
    if(pml4 > hhdm) pml4 = (page_table_t*)((uint64_t)pml4 - hhdm);
    asm volatile("mov %0, %%cr3" : : "r" (pml4));
}

void vaspace_new(process_t* process){
    process->pml4 = (page_table_t*)((uint64_t)pmm_allocate(1) + bl_get_hhdm_offset());
    memcpy(process->pml4, vmm_get_kernel_pml4(), sizeof(page_table_t));
    memset(process->pml4, 0, (sizeof(page_table_t)/2));
}

void vaspace_create_thread_stack(thread_t* thread){
    //FIXME:
    //FIXME:
    //FIXME: VERY IMPORTANT! This will blow up when creating more than 1 thread/process.
    thread->regs.rsp = vaspace_create_area(thread->process->pml4, (VMM_USER_END - VMM_INIT_PROCESS_STACK_SIZE),
                                        VMM_INIT_PROCESS_STACK_SIZE, (VMM_PRESENT | VMM_RW | VMM_USER)) + VMM_INIT_PROCESS_STACK_SIZE - 1;
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