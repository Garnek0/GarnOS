/*  
*   File: ioapic.h
*
*   Author: Garnek
*   
*   Description: PIC and I/O APIC Commands Definition
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef IOAPIC_H
#define IOAPIC_H

#define IOREGSEL(x) (x)
#define IOWIN(x) (x+0x10)

#define IOAPICID 0x00
#define IOAPICVER 0x01
#define IOAPICARB 0x02
#define IOREDTBL(n) (0x10 + 2 * n)

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

#include <types.h>

typedef union {
    struct {
        uint8_t vector;
        uint8_t delvMode : 3;
        uint8_t destMode : 1;
        uint8_t delvStatus : 1;
        uint8_t pinPolarity : 1;
        uint8_t remoteIRR : 1;
        uint8_t triggerMode : 1;
        uint8_t mask : 1;
        uint64_t reserved : 39;
        uint8_t destination;
    } fields;
    uint64_t bits;
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
ioapic_redirection_entry_t;

void ioapic_init();

#endif //IOAPIC_H