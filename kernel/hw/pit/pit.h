/*  
*   File: pit.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef PIT_H
#define PIT_H

#define PIT_CHANNEL_0 0x40
#define PIT_CHANNEL_1 0x41
#define PIT_CHANNEL_2 0x42
#define PIT_MODE_OR_COMMAND 0x43

#define PIT_BASE_FREQUENCY 1193182

#include <types.h>

typedef struct {
    volatile uint64_t ticksSinceOSBoot;
} pit_info;
extern pit_info PITInfo;

void pit_init();
void pit_sleep(size_t ms);
void pit_set_divisor(uint16_t div);

#endif //PIT_H