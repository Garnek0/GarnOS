#include "gdt.h"

#include <kstdio.h>
#include <sys/rblogs.h>

#define GDT_ENTRIES 5

gdt_entry_t gdt[GDT_ENTRIES];
gdtr_t gdtr;

extern gdt_load(gdtr_t* gdtr);

static void gdt_set_entry(uint16_t entry, uint8_t access, uint8_t flags){
    gdt[entry].access = access;
    gdt[entry].base0 = 0;
    gdt[entry].base1 = 0;
    gdt[entry].base2 = 0;
    gdt[entry].flags = flags;
    gdt[entry].limit0 = 0;
    gdt[entry].limit1 = 0;
}

void gdt_init(){
    gdt_set_entry(0, 0, 0);
    gdt_set_entry(1, 0x9A, 0xA);
    gdt_set_entry(2, 0x92, 0xC);
    gdt_set_entry(3, 0xFA, 0xA);
    gdt_set_entry(4, 0xF2, 0xC);

    gdtr.offset = gdt;
    gdtr.size = sizeof(gdt_entry_t)*GDT_ENTRIES - 1;

    gdt_load(&gdtr);

    klog("GDT Loaded Successfully.\n", KLOG_OK);
    rb_log("GDT", KLOG_OK);
}