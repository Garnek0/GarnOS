#include <limine.h>

#include <garn/kernel.h>
#include <garn/irq.h>
#include <garn/mm.h>
#include <garn/fb.h>
#include <garn/hw/serial.h>
#include <garn/dal/dal.h>
#include <garn/term/term.h>
#include <garn/input.h>
#include <garn/fal/vnode.h>
#include <garn/fal/vfs.h>
#include <garn/config.h>
#include <garn/arch.h>

#include "mem/mm-internals.h"
#include "sys/ksym-internals.h"
#include "display/fb-internals.h"
#include "hw/serial/serial-internals.h"
#include "acpi/acpi-internals.h"
#include "sys/term/term-internals.h"
#include "sys/power-internals.h"
#include "sys/ksym-internals.h"
#include "sys/bootloader.h"
#include "sys/dal/dal-internals.h"
#include "sys/syscall_internals.h"
#include "arch/arch-internals.h"

#include <garn/timer.h>
#include <garn/power.h>
#include <cpu/smp/smp.h>
#include <exec/elf.h>
#include <process/sched/sched.h>
#include <consoledemo/kcon.h>

uint64_t kernelStack;
uint64_t hhdmOffset;

static void halt(void) {
    arch_stop();
}

// If you want to rename _start(), make sure to change the linker script accordingly.
void _start(){
    hhdmOffset = bl_get_hhdm_offset();

    fb_init(); //initialise framebuffer

    term_init(); //initialise terminal emulator

    arch_compat_checks(); //compatability checks

    serial_init(); //Initialise serial for debugging

    arch_init_early(0); //initialise CPU 0 (early)

    pmm_init(); //initialise PMM

    vmm_init(); //Initialise VMM

    kheap_init(); //initialise Kernel Heap

    fb_read_init(); //initialise a read buffer

    power_init(); //initialise system power management

    acpi_init(); //Initialise ACPI

    ksym_init(); //initialise kernel symbol table

    input_init(); //initialise keyboard ringbuffer

    dal_init(); //initialise Device Abstraction Layer

#ifdef CONFIG_KCON
    init_kcon(); //start demo console
#endif //CONFIG_KCON

    //We should not proceed if the system FS hasn't been found
    if(!checksysfs_check()) panic("System FS Not found or Inaccessible!", "DAL");

    syscall_init();

    kernel_screen_output_disable();
    
    term_clear();

    sched_init(); //initialise thread scheduler
    
    //Something went very wrong if this is reached
    panic("Scheduler is broken!", "kernel");
    
    halt(); //halt
}
