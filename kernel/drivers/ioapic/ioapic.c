#include "ioapic.h"
#include <kstdio.h>
#include <drivers/ports.h>
#include <acpi/tables/tables.h>
#include <sys/rblogs.h>
#include <cpu/smp/smp.h>

uint8_t ioapicCount;
uint8_t ioapicIDs[256];
void* ioapicAddresses[256];
uint32_t ioapicGSIs[256];

spinlock_t ioapicLock;

void ioapic_write_reg(uint64_t ioapicAddress, uint8_t reg, uint32_t data){
    *(volatile uint32_t*)(IOREGSEL(ioapicAddress)) = reg;
    *(volatile uint32_t*)(IOWIN(ioapicAddress)) = data;
}

uint32_t ioapic_read_reg(uint64_t ioapicAddress, uint32_t reg){
    *(volatile uint32_t*)(IOREGSEL(ioapicAddress)) = reg;
    return *(volatile uint32_t*)(IOWIN(ioapicAddress));
}

void ioapic_redirect(ioapic_redirection_entry_t redirection, uint32_t entry){
    uint64_t data = redirection.bits;
    size_t ioapic;

    lock(ioapicLock, {
        for(ioapic = 0; (ioapicGSIs[ioapic] + (ioapic_read_reg(ioapicAddresses[ioapic], IOAPICVER) >> 16)) < entry; ioapic++);

        ioapic_write_reg(ioapicAddresses[ioapic], IOREDTBL(entry), (data & 0x00000000FFFFFFFF));
        ioapic_write_reg(ioapicAddresses[ioapic], (IOREDTBL(entry)+1), ((data & 0xFFFFFFFF00000000) >> 32));
    });
}

void ioapic_init(){
    outb(0x20, 0x20); //the pic needs this
    outb(0xA0, 0x20);

    //Remap PIC and disable it

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

    //Disable PIC
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);

    acpi_madt_record_hdr_t* hdr = &MADT->MADTRecords;
    acpi_madt_record_ioapic_t* ioapicRec;

    for(size_t i = 0; i < MADT->header.length - sizeof(acpi_madt_t); i++){
        i += hdr->recordLength;
        hdr += hdr->recordLength;

        if(hdr->entryType != ACPI_MADT_IOAPIC) continue;

        ioapicRec = (acpi_madt_record_ioapic_t*)hdr;
        ioapicIDs[ioapicCount] = ioapicRec->ioapicID;
        ioapicGSIs[ioapicCount] = ioapicRec->gsiBase;
        ioapicAddresses[ioapicCount] = ioapicRec->ioapicAddress;
        ioapicCount++;
    }

    ioapic_redirection_entry_t red;
    red.fields.delvMode = 0;
    red.fields.destMode = 0;
    red.fields.pinPolarity = 0;
    red.fields.triggerMode = 0;
    red.fields.mask = 0;
    red.fields.destination = 0;

    red.fields.vector = 32;
    ioapic_redirect(red, 0);
    red.fields.vector = 33;
    ioapic_redirect(red, 1);
    red.fields.vector = 34;
    ioapic_redirect(red, 2);
    red.fields.vector = 35;
    ioapic_redirect(red, 3);
    red.fields.vector = 36;
    ioapic_redirect(red, 4);
    red.fields.vector = 37;
    ioapic_redirect(red, 5);
    red.fields.vector = 38;
    ioapic_redirect(red, 6);
    red.fields.vector = 39;
    ioapic_redirect(red, 7);
    red.fields.vector = 40;
    ioapic_redirect(red, 8);
    red.fields.vector = 41;
    ioapic_redirect(red, 9);
    red.fields.vector = 42;
    ioapic_redirect(red, 10);
    red.fields.vector = 43;
    ioapic_redirect(red, 11);
    red.fields.vector = 44;
    ioapic_redirect(red, 12);
    red.fields.vector = 45;
    ioapic_redirect(red, 13);
    red.fields.vector = 46;
    ioapic_redirect(red, 14);
    red.fields.vector = 47;
    ioapic_redirect(red, 15);

    klog("I/O APIC Initialised Successfully.\n", KLOG_OK);
    rb_log("I/O APIC", KLOG_OK);
}