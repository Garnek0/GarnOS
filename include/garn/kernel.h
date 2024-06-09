/*  
*   File: kernel.h
*
*   Author: Garnek
*   
*   Description: Kernel header file.
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef KERNEL_H
#define KERNEL_H

#include <garn/types.h>
#include <garn/kstdio.h>
#include <garn/spinlock.h>
#include <garn/kerrno.h>
#include <garn/panic.h>

#define KERNEL_VER "v0.12.1-0"

extern uint64_t hhdmOffset;

#endif //KERNEL_H
