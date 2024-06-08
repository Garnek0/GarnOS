/*  
*   File: kernel.c
*
*   Author: Garnek
*   
*   Description: Main kernel file. its mostly just init routines.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <limine.h>

#include <garn/kernel.h>
#include <garn/irq.h>
#include <garn/mm.h>
#include <garn/fb.h>
#include <garn/hw/serial.h>
#include <garn/acpi/acpi-tables.h>
#include <garn/dal/dal.h>
#include <garn/term/term.h>
#include <garn/input.h>
#include <garn/fal/file.h>
#include <garn/fal/filesys.h>

#include <mem/mm-internals.h>

#include <sys/ksym-internals.h>
#include <sys/compat.h>
#include <garn/timer.h>
#include <garn/power.h>
#include <sys/fal/initrd/initrd.h>
#include <sys/term/term-internals.h>
#include <sys/power-internals.h>
#include <sys/input-internals.h>
#include <sys/ksym-internals.h>
#include <sys/bootloader.h>

#include <cpu/gdt/gdt.h>
#include <cpu/multiproc/multiproc-internals.h>

#include <exec/elf.h>

#include <hw/rtc/rtc.h>
#include <hw/pit/pit.h>

#include <module/module-internals.h>

#include <process/sched/sched.h>

#include <consoledemo/kcon.h>

uint64_t kernelStack;
uint64_t hhdmOffset;

static void halt(void) {
    for (;;) {
        asm ("hlt");
    }
}

// If you want to rename _start(), make sure to change the linker script accordingly.
void _start(){
    hhdmOffset = bl_get_hhdm_offset();

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

    acpi_tables_init(); //Parse ACPI Tables

    ksym_init(); //initialise kernel symbol table

    input_init(); //initialise keyboard ringbuffer

    dal_init(); //initialise Device Abstraction Layer

    kernel_screen_output_disable();
    
    term_clear();

    init_kcon(); //start demo console

    sched_init(); //initialise thread scheduler
    
    //Something went very wrong if this is reached
    panic("Scheduler is broken!", "kernel");
    
    halt(); //halt
}