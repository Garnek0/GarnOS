/*  
*   File: mm.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MM_H
#define MM_H

#include <garn/types.h>
#include <garn/spinlock.h>
#include <garn/config.h>

#define PAGE_SIZE 4096
#define ALIGN_UP(x, alignment) (((uint64_t)x % alignment == 0) ? (typeof(x))(x) : (typeof(x))((uint64_t)x + (alignment - ((uint64_t)x % alignment))))

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
#define VMM_USER_END_32BIT 0x80000000 //End of 32bit userspace
#define VMM_USER_STACK_END 0x7ffffffffff0 //End of user stack area

#define VMM_INIT_USER_STACK_SIZE 0x1000000 // 1MiB
#define VMM_INIT_KERNEL_STACK_SIZE 0x4000 // 4KiB

#define VMM_PRESENT 1
#define VMM_RW (1 << 1)
#define VMM_USER (1 << 2)
#define VMM_WRITE_THROUGH (1 << 3)
#define VMM_CACHE_DISABLE (1 << 4)
#define VMM_ACCESSED (1 << 5)
#define VMM_DIRTY (1 << 6)
#define VMM_EXEC_DISABLE (1ull << 63)

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

typedef struct {
    uint64_t base;
    uint64_t length;
    uint8_t type;
} memmap_entry_t;

#ifdef CONFIG_ARCH_X86

#ifdef CONFIG_ARCH_64BIT

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
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
page_table_entry_t;

typedef struct _page_table {
    page_table_entry_t entries[512];
}
#ifndef DOXYGEN
__attribute__((packed))
__attribute__((aligned(0x1000)))
#endif
page_table_t;

#else

;

#endif //CONFIG_ARCH_64BIT

#elif CONFIG_ARCH_DUMMY

;

#endif

//kheap

void* kmalloc(size_t size);
void kmfree(void* ptr);
size_t kheap_get_size();

//memmap

void memmap_print();
int memmap_get_entry_count();
memmap_entry_t memmap_get_entry(int entry);
uint64_t memmap_get_highest_usable_address();
uint64_t memmap_get_highest_address();

//memutil

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
uint32_t strlen(const char *s);
char* strdup(const char* str1);
char *strncat(char *s1, const char *s2, size_t n);

//pmm

void* pmm_allocate(int npages);
void* pmm_allocate32(int npages);
void pmm_free(void* base, int npages);

size_t pmm_get_usable_pages_count();
size_t pmm_get_used_pages_count();
size_t pmm_get_free_pages_count();

//vmm

void vmm_map(page_table_t* pt, uint64_t physAddr, uint64_t virtAddr, uint32_t flags);
void vmm_map_range(page_table_t* pt, uint64_t physAddr, uint64_t virtAddr, size_t length, uint32_t flags);
void vmm_unmap(page_table_t* pt, uint64_t virtAddr);
void vmm_unmap_range(page_table_t* pt, uint64_t virtAddr, size_t length);
void vmm_set_flags(page_table_t* pt, uint64_t virtAddr, uint32_t flags);
void vmm_set_flags_range(page_table_t* pt, uint64_t virtAddr, size_t length, uint32_t flags);
bool vmm_is_page_free(page_table_t* pt, uint64_t virtAddr);
uint64_t vmm_virt_to_phys(page_table_t* pt, uint64_t virtAddr);
page_table_t* vmm_get_kernel_pt();

#endif //MM_H