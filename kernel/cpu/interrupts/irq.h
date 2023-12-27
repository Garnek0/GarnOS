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

typedef struct {
   void (*irq0)(stack_frame_t* frame);
   void (*irq1)(stack_frame_t* frame);
   void (*irq2)(stack_frame_t* frame);
   void (*irq3)(stack_frame_t* frame);
   void (*irq4)(stack_frame_t* frame);
   void (*irq5)(stack_frame_t* frame);
   void (*irq6)(stack_frame_t* frame);
   void (*irq7)(stack_frame_t* frame);
   void (*irq8)(stack_frame_t* frame);
   void (*irq9)(stack_frame_t* frame);
   void (*irq10)(stack_frame_t* frame);
   void (*irq11)(stack_frame_t* frame);
   void (*irq12)(stack_frame_t* frame);
   void (*irq13)(stack_frame_t* frame);
   void (*irq14)(stack_frame_t* frame);
   void (*irq15)(stack_frame_t* frame);
} irq_handler_t;
extern irq_handler_t irqHandler;

void irq_set_handler(uint8_t irq, void* handler);

#endif //IRQ_H