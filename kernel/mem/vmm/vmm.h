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
#define VMM_USER_END_32BIT 0x80000000 //End of 32bit userspace
#define VMM_USER_STACK_END 0x7ffffffffff0 //End of user stack area

#define VMM_INIT_USER_STACK_SIZE 0x1000000 // 1MiB
#define VMM_INIT_KERNEL_STACK_SIZE 0x4000 // 4KiB

#define VMM_PRESENT 1
#define VMM_RW (1 << 1)
#define VMM_USER (1 << 2)
#define VMM_PWT (1 << 3)
#define VMM_PCD (1 << 4)
#define VMM_ACCESSED (1 << 5)
#define VMM_PS (1 << 7)
#define VMM_NX (1 << 63)

#define PROT_READ 1
#define PROT_WRITE (1 << 1)
#define PROT_EXEC (1 << 2)
#define PROT_NONE 0

#define MAP_SHARED 1
#define MAP_PRIVATE (1 << 1)
#define MAP_32BIT (1 << 6)
#define MAP_ANONYMOUS (1 << 5)
#define MAP_ANON MAP_ANONYMOUS

#include <types.h>
#include <cpu/smp/spinlock.h>
#include <process/thread/thread.h>
#include <process/process.h>

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

typedef struct _page_table {
    page_table_entry_t entries[512];
}__attribute__((packed)) __attribute__((aligned(0x1000))) page_table_t;

//vmm

void vmm_init();
void vmm_indexer(uint64_t virtAddr, int* Pi, int* PTi, int* PDi, int* PDPi);
void vmm_map(page_table_t* pml4, uint64_t physAddr, uint64_t virtAddr, uint32_t flags);
void vmm_unmap(page_table_t* pml4, uint64_t virtAddr);
void vmm_set_flags(page_table_t* pml4, uint64_t virtAddr, uint32_t flags);

page_table_t* vmm_get_kernel_pml4();

//vaspace

page_table_t* vaspace_new();
void vaspace_destroy(page_table_t* pml4);
void vaspace_clear(page_table_t* pml4);
page_table_t* vaspace_clone(page_table_t* toClone);
void vaspace_switch(page_table_t* pml4);
void* vaspace_create_area(page_table_t* pml4, uint64_t virtAddr, size_t size, uint32_t flags);
void vaspace_create_thread_user_stack(struct _thread* thread);

void* sys_mmap(stack_frame_t* regs, void* addr, size_t length, int prot, int flags, int fd, uint64_t offset);
int sys_munmap(stack_frame_t* regs, void* addr, size_t length);

#endif //VMM_H