#include <mem/mm-internals.h>
#include <garn/kstdio.h>
#include <limine.h>
#include <garn/panic.h>
#include <garn/hw/serial.h>
#include <sys/bootloader.h>
#include <garn/kerrno.h>

static uint8_t* bitmap;
size_t bitmapSize;

size_t freePages;
size_t usedPages;
size_t usablePages;
spinlock_t lock;

static void pmm_bitmap_set(uint64_t page){
    if(page/8 > bitmapSize){
        panic("Bitmap bounds exceeded! Attempt to index physical page 0x%x (alloc)", "PMM", page);
        return;
    }
    usedPages++;
    freePages--;
    bitmap[page/8] |= (0b10000000 >> (page%8));
}

static void pmm_bitmap_clear(uint64_t page){
    if(page/8 > bitmapSize){
        panic("Bitmap bounds exceeded! Attempt to index physical page 0x%x (dealloc)", "PMM", page);
        return;
    }
    usedPages--;
    freePages++;
    bitmap[page/8] &= ~((0b10000000 >> (page%8)));
}

// static bool pmm_bitmap_test(int page){
//     return bitmap[page/8] & ((0b10000000 >> (page%8)));
// }

static uint64_t pmm_find_free(int npages){
    int foundPages = 0;
    uint64_t base = 0;
    bool inChunk = false;
    bool searchingHighPages = false;
    size_t i;
    if(bitmapSize > 131072){
        i = 131072;
        searchingHighPages = true;
    } else {
        i = 0;
    }
retry:
    for(; i < bitmapSize; i++){
        for(uint8_t j = 0; j < 8; j++){
            if(!(bitmap[i] & (0b10000000 >> j))){
                if(!inChunk){
                    inChunk = true;
                    base = (i*8)+j;
                }
                foundPages++;
                if(foundPages == npages){
                    return (base << 12);
                }
            } else {
                if(inChunk){
                    inChunk = false;
                    foundPages = 0;
                }
            }
        }
    }

    if(searchingHighPages){
        searchingHighPages = false;
        inChunk = false;
        i = 0;
        foundPages = 0;
        goto retry;
    }

    kerrno = ENOMEM;
    panic("Out of Memory!", "PMM");
    return 0;
}

static uint64_t pmm_find_free32(int npages){
    int foundPages = 0;
    uint64_t base = 0;
    bool inChunk = false;
    size_t limit;
    if(bitmapSize > 131072){
        limit = 131072;
    } else {
        limit = bitmapSize;
    }
    for(size_t i = 0; i < limit; i++){
        for(uint8_t j = 0; j < 8; j++){
            if(!(bitmap[i] & (0b10000000 >> j))){
                if(!inChunk){
                    inChunk = true;
                    base = (i*8)+j;
                }
                foundPages++;
                if(foundPages == npages){
                    return (base << 12);
                }
            } else {
                if(inChunk){
                    inChunk = false;
                    foundPages = 0;
                }
            }
        }
    }

    kerrno = ENOMEM;
    panic("Out of Memory! (32-bit Allocator)", "PMM");
    return 0;
}

void* pmm_allocate(int npages){
    uint64_t base;
    lock(lock, {
        base = pmm_find_free(npages);
        for(uint64_t i = (base >> 12); i < (base >> 12)+npages; i++){
            pmm_bitmap_set(i);
        }
    });
    return (void*)base;
}

void* pmm_allocate32(int npages){
    uint64_t base;
    lock(lock, {
        base = pmm_find_free32(npages);
        for(uint64_t i = (base >> 12); i < (base >> 12)+npages; i++){
            pmm_bitmap_set(i);
        }
    });
    return (void*)base;
}

void pmm_free(void* base, int npages){
    lock(lock, {
        for(uint64_t i = ((uint64_t)base >> 12); i < ((uint64_t)base >> 12)+npages; i++){
            memset((void*)((uint64_t)base+((npages-1)*PAGE_SIZE)+bl_get_hhdm_offset()), 0xff, PAGE_SIZE);
            pmm_bitmap_clear(i);
        }
    });
}


void pmm_init(){
    bitmapSize = ALIGN_UP(memmap_get_highest_usable_address(), PAGE_SIZE) / PAGE_SIZE / 8;
	usedPages = bitmapSize*8;

    memmap_entry_t current_entry;
    for(int i = 0; i < memmap_get_entry_count(); i++){
        current_entry = memmap_get_entry(i);

        if(current_entry.type != MEMMAP_USABLE) continue;
        if(current_entry.length < bitmapSize) continue;

        bitmap = (uint8_t*)(current_entry.base + bl_get_hhdm_offset());
        goto success;
    }

    klog("Could not Initialise Physical Memory Allocator", KLOG_FAILED, "PMM");
    panic("Not enough free memory to allocate for bitmap.", "PMM");

success:
    memset(bitmap, 0xff, bitmapSize);

    for(int i = 0; i < memmap_get_entry_count(); i++){
        current_entry = memmap_get_entry(i);

        if(current_entry.type != MEMMAP_USABLE) continue;

        for(uint64_t j = (current_entry.base >> 12); j < ((current_entry.base + current_entry.length) >> 12); j++){
            pmm_bitmap_clear(j);
            usablePages++;
        }
    }

    uint64_t bitmapNoHHDM = (uint64_t)bitmap - bl_get_hhdm_offset();

    for(uint64_t i = bitmapNoHHDM; i <= bitmapNoHHDM + bitmapSize; i+=PAGE_SIZE){
        pmm_bitmap_set(i/PAGE_SIZE);
        usablePages--;
    }

    klog("Initialised Physical Memory Allocator (Bitmap base: 0x%p)\n", KLOG_OK, "PMM", bitmap);
}

size_t pmm_get_usable_pages_count(){
    return usablePages;
}

size_t pmm_get_used_pages_count(){
    return usedPages;
}

size_t pmm_get_free_pages_count(){
    return freePages;
}
