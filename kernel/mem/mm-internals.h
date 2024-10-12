#pragma once

#define KHEAP_FLAGS_FREE 1

#define KHEAP_INIT_PAGES 25000

#define MEMMAP_USABLE                 0
#define MEMMAP_RESERVED               1
#define MEMMAP_ACPI_RECLAIMABLE       2
#define MEMMAP_ACPI_NVS               3
#define MEMMAP_BAD_MEMORY             4
#define MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define MEMMAP_KERNEL_AND_MODULES     6
#define MEMMAP_FRAMEBUFFER            7

#define VMM_USER_END 0x800000000000 //End of user area
#define VMM_USER_STACK_END 0x7ffffffffff0 //End of user stack area

#define VMM_INIT_USER_STACK_SIZE 0x1000000 // 1MiB
#define VMM_INIT_KERNEL_STACK_SIZE 0x2000 // 8KiB

#define PROT_NONE  0x00
#define PROT_READ  0x01
#define PROT_WRITE 0x02
#define PROT_EXEC  0x04

#define MAP_FAILED ((void *)(-1))
#define MAP_FILE      0x00
#define MAP_PRIVATE   0x01
#define MAP_SHARED    0x02
#define MAP_FIXED     0x04
#define MAP_ANON      0x08
#define MAP_ANONYMOUS 0x08
#define MAP_NORESERVE 0x10
#define MAP_32BIT     0x20

#include <garn/types.h>
#include <garn/spinlock.h>
#include <garn/irq.h>
#include <garn/mm.h>

struct _thread;

typedef struct _kheap_block_header {
    size_t size;
    uint8_t flags; //bit 0 - block is free 
    struct _kheap_block_header* next;
    struct _kheap_block_header* prev;
} kheap_block_header_t;

//kheap

void kheap_init();

//pmm

void pmm_init();

//vmm

void vmm_init();

//vaspace

page_table_t* vaspace_new();
void vaspace_destroy(page_table_t* pt);
void vaspace_clear(page_table_t* pt);
page_table_t* vaspace_clone(page_table_t* toClone);
void vaspace_switch(page_table_t* pt);
void* vaspace_create_area(page_table_t* pt, uint64_t virtAddr, size_t size, uint32_t flags);
void vaspace_create_thread_user_stack(struct _thread* thread);
bool vmm_is_page_free(page_table_t* pt, uint64_t virtAddr);

void* sys_mmap(stack_frame_t* regs, void* addr, size_t length, int prot, int flags, int fd, uint64_t offset);
int sys_munmap(stack_frame_t* regs, void* addr, size_t length);
