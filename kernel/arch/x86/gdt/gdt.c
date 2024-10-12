#include "gdt.h"
#include <cpu/smp/smp.h>
#include <arch/arch-internals.h>

gdt_t gdt[MAX_CPUS];
tss_t tss[MAX_CPUS];
gdtr_t gdtr[MAX_CPUS];

extern void gdt_load(gdtr_t* gdtr);

static void gdt_set_entry(gdt_entry_t* entry, uint8_t access, uint8_t flags){

    //base and limit are ignored in long mode because paging is used
    //instead of segmentation

    entry->access = access;
    entry->base0 = 0;
    entry->base1 = 0;
    entry->base2 = 0;
    entry->flags = flags;
    entry->limit0 = 0;
    entry->limit1 = 0;
}

static void gdt_set_system_entry(gdt_system_entry_t* entry, uint8_t access, uint8_t flags, uint64_t base, uint32_t limit){
    entry->access = access;
    entry->base0 = (base & 0xFFFF);
    entry->base1 = (base >> 16) & 0xFF;
    entry->base2 = (base >> 24) & 0xFF;
    entry->base3 = (base >> 32) & 0xFFFFFFFF;
    entry->flags = flags;
    entry->limit0 = (limit & 0xFFFF);
    entry->limit1 = (limit >> 16) & 0xF;
}

void tss_set_rsp(size_t cpu, uint64_t rsp){
    tss[cpu].rsp[0] = rsp;
}

extern void tss_flush();

void gdt_init(size_t cpu){
    //nullseg
    gdt_set_entry(&gdt[cpu].null, 0, 0);
    //kernel codeseg
    gdt_set_entry(&gdt[cpu].kernelCode, 0x9A, 0xA);
    //kernel dataseg
    gdt_set_entry(&gdt[cpu].kernelData, 0x92, 0xC);
    //user codeseg
    gdt_set_entry(&gdt[cpu].userCode, 0xFA, 0xA);
    //user dataseg
    gdt_set_entry(&gdt[cpu].userData, 0xF2, 0xC);
    //tss
    gdt_set_system_entry(&gdt[cpu].tss, 0xE9, 0x0, (uint64_t)&tss[cpu], sizeof(tss_t));

    gdtr[cpu].offset = (uint64_t)&gdt[cpu];
    gdtr[cpu].size = sizeof(gdt_t) - 1;

    gdt_load(&gdtr[cpu]);

    tss_flush();
}

void arch_set_kernel_stack(size_t cpu, uintptr_t stack){
    tss_set_rsp(cpu, stack);
}
