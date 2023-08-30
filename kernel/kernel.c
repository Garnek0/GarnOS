#include <types.h>
#include <limine.h>

#include <kstdio.h>

#include <cpu/gdt/gdt.h>
#include <cpu/interrupts/interrupts.h>

#include <mem/memmap/memmap.h>
#include <mem/mm/pmm.h>
#include <mem/mm/vmm.h>
#include <mem/mm/kheap.h>

#include <exec/elf.h>

#include <drivers/serial/serial.h>

#include <module/module.h>

#include <fs/vfs/vfs.h>
#include <fs/initrd/initrd.h>

#include <display/fb.h>

#include <term/term.h>

#include <acpi/tables/tables.h>

#include <sys/panic.h>
#include <sys/ksym.h>

#include <consoledemo/kcon.h>

static void halt(void) {
    for (;;) {
        asm ("hlt");
    }
}

// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {

    fb_init(); //initialise framebuffer

    term_init(); //initialise terminal emulator

    gdt_init(); //load the GDT

    serial_init(); //Initialise serial for debugging

    interrupts_init(); //enables interrupts

    pmm_init(); //initialise PMM

    vmm_init(); //Initialise VMM

    kheap_init(); //initialise Kernel Heap;

    acpi_tables_parse(); //Parse ACPI Tables

    ksym_init(); //initialise kernel symbol table

    module_init(); //initialise module manager

    init_kcon();

    halt();
}
