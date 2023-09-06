#include <module/module.h>
#include <drivers/ports.h>
#include <cpu/interrupts/idt.h>
#include <cpu/interrupts/interrupts.h>
#include <kstdio.h>
#include <sys/rblogs.h>

#define PIC1 0x20
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1+1)

#define PIC2 0xA0
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2+1)

#define PIC_EOI 0x20

#define ICW1_ICW4 0x01
#define ICW1_SINGLE 0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL 0x08
#define ICW1_INIT 0x10

#define ICW4_8086 0x01
#define ICW4_AUTO 0x02
#define ICW4_BUF_SLAVE 0x08
#define ICW4_BUF_MASTER	0x0C
#define ICW4_SFNM 0x10

void eoi(uint64_t irq){
    if(irq >= 8){
	    outb(PIC2_COMMAND, PIC_EOI);
    }
	outb(PIC1_COMMAND, PIC_EOI);
}

void init(){

    uint8_t m1, m2;

    m1 = inb(PIC1_DATA);
	m2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, 0x20);
	io_wait();
	outb(PIC2_DATA, 0x28);
	io_wait();
	outb(PIC1_DATA, 4);
	io_wait();
	outb(PIC2_DATA, 2);
	io_wait();
 
	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

    outb(PIC1_DATA, m1);
    outb(PIC2_DATA, m2);

    klog("PIC: Initialised PIC1 at offset 0x%x and PIC2 at offset 0x%x.\n", KLOG_OK, 0x20, 0x28);
	rb_log("PIC", KLOG_OK);

    outb(PIC1_DATA, 0);
    outb(PIC2_DATA, 0);
}

void fini(){
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
    return;
}

module_t metadata = {
    .name = "8259pic",
    .init = init,
    .fini = fini
};