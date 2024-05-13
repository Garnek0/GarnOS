/*  
*   File: irq.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef IRQ_H
#define IRQ_H

#include <types.h>
#include <cpu/interrupts/interrupts.h>
#include <ds/list.h>

#define IRQ_SHARED 1

typedef struct {
    void* handler;
} irq_t;

void irq_init();
int irq_add_handler(uint8_t irq, void* handler, uint32_t flags);
void irq_remove_handler(uint8_t irq, void* handler);

#endif //IRQ_H