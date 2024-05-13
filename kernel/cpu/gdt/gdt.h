/*  
*   File: gdt.h
*
*   Author: Garnek
*   
*   Description: typedefs for the GDT and GDTR structures
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef GDT_H
#define GDT_H

#include <types.h>

typedef struct {
    uint16_t size;
    uint64_t offset;
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
gdtr_t;

typedef struct {
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access;
    uint8_t limit1 : 4;
    uint8_t flags : 4;
    uint8_t base2;
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
gdt_entry_t;

typedef struct {
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t access;
    uint8_t limit1 : 4;
    uint8_t flags : 4;
    uint8_t base2;
    uint32_t base3;
    uint32_t reserved;
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
gdt_system_entry_t;

typedef struct {
    gdt_entry_t null;
    gdt_entry_t kernelCode;
    gdt_entry_t kernelData;
    gdt_entry_t userCode;
    gdt_entry_t userData;
    gdt_system_entry_t tss;
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
gdt_t;

typedef struct {
    uint32_t reserved0;
    uint64_t rsp[3];
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomapBase;
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
tss_t;

void gdt_init(size_t cpu);
void tss_set_rsp(size_t cpu, uint64_t rsp);

#endif //GDT_H