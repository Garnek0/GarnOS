#pragma once

#include <garn/types.h>
#include <garn/spinlock.h>
#include <garn/config.h>

#define PAGE_SIZE 4096
#define ALIGN_UP(x, alignment) (((uint64_t)x % alignment == 0) ? (typeof(x))(x) : (typeof(x))((uint64_t)x + (alignment - ((uint64_t)x % alignment))))

#define VMM_PRESENT 1
#define VMM_RW (1 << 1)
#define VMM_USER (1 << 2)
#define VMM_WRITE_THROUGH (1 << 3)
#define VMM_CACHE_DISABLE (1 << 4)
#define VMM_ACCESSED (1 << 5)
#define VMM_DIRTY (1 << 6)
#define VMM_EXEC_DISABLE (1ull << 63)

typedef struct {
    uint64_t base;
    uint64_t length;
    uint8_t type;
} memmap_entry_t;

#ifdef __x86_64__

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

#elif ARCH_DUMMY

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
