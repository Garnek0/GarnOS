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

void panic(const char* str, ...);
void panic_exception(const char* str, stack_frame_t* regs, ...);

#endif //PANIC_H