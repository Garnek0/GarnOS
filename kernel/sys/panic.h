/*  
*   File: panic.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef PANIC_H
#define PANIC_H

#include <types.h>
#include <kstdio.h>

void panic(char* str, ...);

#endif //PANIC_H