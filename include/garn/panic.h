/*  
*   File: panic.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef PANIC_H
#define PANIC_H

#include <garn/types.h>
#include <garn/irq.h>
#include <garn/kstdio.h>

void panic(const char* str, const char* component, ...);
void panic_exception(const char* str, stack_frame_t* regs, ...);

#endif //PANIC_H