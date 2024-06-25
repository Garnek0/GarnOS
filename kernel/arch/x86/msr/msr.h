/*  
*   File: msr.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MSR_H
#define MSR_H

#include <garn/types.h>

uint64_t rdmsr(uint64_t msr);
void wrmsr(uint64_t msr, uint64_t data);

#endif //MSR_H