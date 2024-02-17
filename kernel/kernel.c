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
#include <cpu/smp/cpus.h>

#include <mem/memmap/memmap.h>
#include <mem/memutil/memutil.h>
#include <mem/pmm/pmm.h>
#include <mem/vmm/vmm.h>
#include <mem/kheap/kheap.h>

#include <exec/elf.h>

#include <hw/serial/serial.h>
#include <hw/rtc/rtc.h>
#include <hw/pit/pit.h>

#include <module/module.h>

#include <sys/fal/fal.h>
#include <sys/fal/initrd/initrd.h>

#include <display/fb.h>

#include <sys/term/term.h>

#include <acpi/tables/tables.h>

#include <sys/panic.h>
#include <sys/ksym.h>
#include <sys/compat.h>
#include <sys/timer.h>
#include <sys/power.h>
#include <sys/dal/dal.h>
#include <sys/input.h>

#include <process/sched/sched.h>

#include <consoledemo/kcon.h>

uint64_t kernelStack;

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

    compat_check(); //compatability checks;

    serial_init(); //Initialise serial for debugging

    gdt_init(0); //load the GDT

    interrupts_init(); //enables interrupts

    pmm_init(); //initialise PMM

    vmm_init(); //Initialise VMM

    kheap_init(); //initialise Kernel Heap

    tty_init(); //initialise tty stdout and stderr

    fb_read_init(); //initialise a read buffer

    power_init(); //initialise system power management

    acpi_tables_parse(); //Parse ACPI Tables

    ksym_init(); //initialise kernel symbol table

    rtc_init(); //initialise realtime clock //Move to DAL

    pit_init(); //initialise the PIT //Move to DAL

    input_init(); //initialise keyboard ringbuffer

    dal_init(); //initialise Device Abstraction Layer

    fb_clear(0x00000000);
    cursor_set(&tc.cursor, 0, 0);

    //init_kcon(); //initialise demo console

    sched_init(); //initialise thread scheduler

    halt(); //halt
}
