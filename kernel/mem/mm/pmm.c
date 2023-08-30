#include "pmm.h"
#include <kstdio.h>
#include <limine.h>
#include <sys/panic.h>
#include <mem/memmap/memmap.h>
#include <mem/memutil/memutil.h>
#include <drivers/serial/serial.h>

uint8_t* bitmap;
int bitmapSize;

int freePages = 0;
int usedPages = 0;
int usablePages = 0;

static void pmm_bitmap_set(int page){
    bitmap[page/8] |= (0b10000000 >> (page%8));
}

static void pmm_bitmap_clear(int page){
    bitmap[page/8] &= ~((0b10000000 >> (page%8)));
}

static bool pmm_bitmap_test(int page){
    return bitmap[page/8] & ((0b10000000 >> (page%8)));
}

static uint64_t pmm_find_free(int npages){
    int foundPages = 0;
    uint64_t base = 0;
    bool inChunk = false;
    for(int i = 0; i < bitmapSize; i++){
        for(int j = 0; j < 8; j++){
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
    panic("Out of Memory!");
    serial_log("Kernel Panic from PMM: Out of Memory!\n");
    return NULL;
}

void* pmm_allocate(int npages){
    uint64_t base = pmm_find_free(npages);
    for(uint64_t i = (base >> 12); i < (base >> 12)+npages; i++){
        pmm_bitmap_set(i);
        usedPages++;
        freePages--;
    }
    return base;
}

void pmm_free(void* base, int npages){
    for(uint64_t i = ((uint64_t)base >> 12); i < ((uint64_t)base >> 12)+npages; i++){
        pmm_bitmap_clear(i);
        usedPages--;
        freePages++;
    }
}


void pmm_init(){
    bitmapSize = ALIGN_UP(memmap_get_highest_usable_address(), PAGE_SIZE * 8) / PAGE_SIZE / 8;

    memmap_entry_t current_entry;
    for(int i = 0; i < memmap_get_entry_count(); i++){
        current_entry = memmap_get_entry(i);

        if(current_entry.type != MEMMAP_USABLE) continue;
        if(current_entry.length < bitmapSize) continue;

        bitmap = (uint8_t*)current_entry.base;
        goto success;
    }

    klog("Could not Initialise Physical Memory Allocator", KLOG_FAILED);
    panic("Not enough free memory to allocate for bitmap.");
    serial_log("Kernel Panic from PMM: Not enough free memory to allocate for bitmap.\n");

success:
    memset(bitmap, 0xff, bitmapSize);

    for(int i = 0; i < memmap_get_entry_count(); i++){
        current_entry = memmap_get_entry(i);

        if(current_entry.type != MEMMAP_USABLE) continue;

        for(uint64_t j = (current_entry.base >> 12); j < ((current_entry.base + current_entry.length) >> 12); j++){
            pmm_bitmap_clear(j);
            usablePages++;
            freePages++;
        }
    }

    for(uint64_t i = ((uint64_t)bitmap >> 12); i < (((uint64_t)bitmap + bitmapSize) >> 12)+1; i++){
        pmm_bitmap_set(i);
        freePages--;
        usedPages++;
    }

    klog("Initialised Physical Memory Allocator (Bitmap base: 0x%p)\n", KLOG_OK, bitmap);
}