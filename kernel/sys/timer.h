/*  
*   File: timer.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef TIMER_H
#define TIMER_H

#include <types.h>

void ksleep(uint64_t ms);
inline uint64_t timer_get_ticks();

#endif //TIMER_H