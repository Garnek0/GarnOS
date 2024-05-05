/*  
*   File: apic.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef APIC_H
#define APIC_H

#define APIC_ID 0x20
#define APIC_VER 0x30

#define APIC_TPR 0x80
#define APIC_APR 0x90
#define APIC_PPR 0xA0
#define APIC_EOI 0xB0
#define APIC_RRD 0xC0

#define APIC_LOGICAL_DEST 0xD0
#define APIC_DEST_FORMAT 0xE0

#define APIC_SPURIOUS_INT_VECT 0xF0

#define APIC_ISR0 0x100
#define APIC_ISR1 0x110
#define APIC_ISR2 0x120
#define APIC_ISR3 0x130
#define APIC_ISR4 0x140
#define APIC_ISR5 0x150
#define APIC_ISR6 0x160
#define APIC_ISR7 0x170

#define APIC_TMR0 0x180
#define APIC_TMR1 0x190
#define APIC_TMR2 0x1A0
#define APIC_TMR3 0x1B0
#define APIC_TMR4 0x1C0
#define APIC_TMR5 0x1D0
#define APIC_TMR6 0x1E0
#define APIC_TMR7 0x1F0

#define APIC_IRR0 0x200
#define APIC_IRR1 0x210
#define APIC_IRR2 0x220
#define APIC_IRR3 0x230
#define APIC_IRR4 0x240
#define APIC_IRR5 0x250
#define APIC_IRR6 0x260
#define APIC_IRR7 0x270

#define APIC_ERROR_STATUS 0x280

#define APIC_LVT_CMCI 0x2F0

#define APIC_ICR0 0x300
#define APIC_ICR1 0x310

#define APIC_LVT_TIMER 0x320
#define APIC_LVT_THERMAL 0x330
#define APIC_LVT_PERFORMANCE 0x340
#define APIC_LVT_LINT0 0x350
#define APIC_LVT_LINT1 0x360
#define APIC_LVT_ERROR 0x370

#define APIC_TIMER_INITIAL_COUNT 0x380
#define APIC_TIMER_CURRENT_COUNT 0x390
#define APIC_TIMER_DIVIDE_CONFIG 0x3E0

#define APIC_ESR_CHECKSUM_SEND 1
#define APIC_ESR_CHECKSUM_RECEIVE (1 << 1)
#define APIC_ESR_ACC_ERR_SEND (1 << 2)
#define APIC_ESR_ACC_ERR_RECEIVE (1 << 3)
#define APIC_ESR_REDIRECTABLE_IPI (1 << 4)
#define APIC_ESR_ILLEGAL_VECTOR_SEND (1 << 5)
#define APIC_ESR_ILLEGAL_VECTOR_RECEIVE (1 << 6)
#define APIC_ESR_ILLEGAL_VREG_ADDR (1 << 7)

#include <types.h>

void apic_init();
void apic_eoi();

#endif //APIC_H