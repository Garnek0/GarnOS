/*  
*   File: time.h
*
*   Author: Garnek
*
*   Description: System time
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef TIME_H
#define TIME_H

#include <garn/types.h>

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t dayOfMonth;
    uint8_t month;
    uint64_t year;
} systime_t;

void time_set(systime_t time);
systime_t time_get();

#endif //TIME_H