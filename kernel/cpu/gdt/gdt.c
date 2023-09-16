/*  
*   File: gdt.c
*
*   Author: Garnek
*   
*   Description: Global Descriptor Table
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "gdt.h"

//nullseg, kernel codeseg, kernel dataseg, user codeseg, user dataseg
#define GDT_ENTRIES 5

gdt_entry_t gdt[GDT_ENTRIES];
gdtr_t gdtr;

//This is implemented in assembly
extern gdt_load(gdtr_t* gdtr);

//set a GDT entry
static void gdt_set_entry(uint16_t entry, uint8_t access, uint8_t flags){

    //base and limit are ignored in long mode because paging is used
    //instead of segmentation

    gdt[entry].access = access;
    gdt[entry].base0 = 0;
    gdt[entry].base1 = 0;
    gdt[entry].base2 = 0;
    gdt[entry].flags = flags;
    gdt[entry].limit0 = 0;
    gdt[entry].limit1 = 0;
}

//initialise gdt
void gdt_init(){
    //nullseg
    gdt_set_entry(0, 0, 0);
    //kernel codeseg
    gdt_set_entry(1, 0x9A, 0xA);
    //kernel dataseg
    gdt_set_entry(2, 0x92, 0xC);
    //user codeseg
    gdt_set_entry(3, 0xFA, 0xA);
    //user dataseg
    gdt_set_entry(4, 0xF2, 0xC);

    gdtr.offset = gdt;
    gdtr.size = sizeof(gdt_entry_t)*GDT_ENTRIES - 1;

    gdt_load(&gdtr);
}