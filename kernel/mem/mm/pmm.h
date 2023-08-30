#ifndef PMM_H
#define PMM_H

#include <types.h>

#define PAGE_SIZE 4096

extern int freePages;
extern int usedPages;
extern int usablePages;

void pmm_init();
void* pmm_allocate(int npages);
void pmm_free(void* base, int npages);

#endif //PMM_H