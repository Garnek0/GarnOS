/*  
*   File: vmm.h
*
*   Author: Garnek
*   
*   Description: Page Table able Entry structure.
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef VMM_H
#define VMM_H

#define VMM_USER_END 0x800000000000 //End of user area
#define VMM_USER_STACK_END 0x7ffffffffff0 //End of user stack area

#define VMM_INIT_USER_STACK_SIZE 0x1000000 // 1MiB
#define VMM_INIT_KERNEL_STACK_SIZE 0x4000 // 4KiB

#include <types.h>
#include <cpu/smp/spinlock.h>
#include <process/thread/thread.h>
#include <process/process.h>

#define VMM_PRESENT 1
#define VMM_RW (1 << 1)
#define VMM_USER (1 << 2)
#define VMM_PWT (1 << 3)
#define VMM_PCD (1 << 4)
#define VMM_ACCESSED (1 << 5)
#define VMM_PS (1 << 7)
#define VMM_NX (1 << 63)

typedef struct {
    bool present : 1;
    bool readWrite : 1;
    bool userSupervisor : 1;
    bool writeThrough : 1;
    bool cacheDisable : 1;
    bool accessed : 1;
    bool avl0 : 1;
    bool pageSize : 1;
    uint8_t avl1 : 4;
    uint64_t addr : 51;
    bool nx : 1;
}__attribute__((packed)) page_table_entry_t;

typedef struct {
    page_table_entry_t entries[512];
}__attribute__((packed)) __attribute__((aligned(0x1000))) page_table_t;

//vmm

void vmm_init();
void vmm_map(page_table_t* pml4, uint64_t physAddr, uint64_t virtAddr, uint32_t flags);
void vmm_unmap(page_table_t* pml4, uint64_t virtAddr);
void vmm_set_flags(page_table_t* pml4, uint64_t virtAddr, uint32_t flags);

page_table_t* vmm_get_kernel_pml4();

//vaspace

void vaspace_new(struct _process* process);
void vaspace_switch(page_table_t* pml4);
void* vaspace_create_area(page_table_t* pml4, uint64_t virtAddr, size_t size, uint32_t flags);
void vaspace_create_thread_user_stack(struct _thread* thread);

#endif //VMM_H