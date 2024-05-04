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

void irq_add_handler(uint8_t irq, void* handler);
void irq_remove_handler(uint8_t irq, void* handler);

#endif //IRQ_H