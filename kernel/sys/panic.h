/*  
*   File: panic.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef PANIC_H
#define PANIC_H

#include <types.h>
#include <cpu/interrupts/interrupts.h>
#include <kstdio.h>

void panic(char* str, ...);
void panic_with_stack_frame(char* str, stack_frame_t* regs, ...);

#endif //PANIC_H