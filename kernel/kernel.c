/*  
*   File: kernel.c
*
*   Author: Garnek
*   
*   Description: Main kernel file. its mostly just init routines.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <types.h>
#include <limine.h>

#include <kstdio.h>

#include <cpu/gdt/gdt.h>
#include <cpu/interrupts/interrupts.h>
#include <cpu/smp/smp.h>

#include <mem/memmap/memmap.h>
#include <mem/mm/pmm.h>
#include <mem/mm/vmm.h>
#include <mem/mm/kheap.h>

#include <exec/elf.h>

#include <hw/serial/serial.h>
#include <hw/rtc/rtc.h>
#include <hw/pit/pit.h>

#include <module/module.h>

#include <fs/vfs/vfs.h>
#include <fs/initrd/initrd.h>

#include <display/fb.h>

#include <term/term.h>

#include <acpi/tables/tables.h>

#include <sys/panic.h>
#include <sys/ksym.h>
#include <sys/rblogs.h>
#include <sys/compat.h>
#include <sys/timer.h>

#include <consoledemo/kcon.h>

static void halt(void) {
    for (;;) {
        asm ("hlt");
    }
}

// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {

    rb_init(); //initialise ringbuffer for logs

    fb_init(); //initialise framebuffer

    term_init(); //initialise terminal emulator

    compat_check(); //compatability checks;

    serial_init(); //Initialise serial for debugging

    gdt_init(); //load the GDT

    interrupts_init(); //enables interrupts
    
    pit_init(); //initialise the PIT

    pmm_init(); //initialise PMM

    vmm_init(); //Initialise VMM

    kheap_init(); //initialise Kernel Heap

    rtc_init(); //initialise realtime clock

    fb_read_init(); //initialise a read buffer

    acpi_tables_parse(); //Parse ACPI Tables

    ksym_init(); //initialise kernel symbol table

    smp_init(); //initialise SMP

    module_init(); //initialise module manager

    init_kcon(); //initialise demo console

    halt(); //halt
}
