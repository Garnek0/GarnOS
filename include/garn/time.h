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

typedef struct _timespec {
	uint32_t sec;
	uint32_t nsec;
} timespec_t;

typedef struct _timespec64 {
	uint64_t sec;
	uint32_t nsec;
} timespec64_t;

void time_set(timespec_t time);
void time_set64(timespec64_t time);

timespec_t time_get();
timespec64_t time_get64();

timespec_t time_conv_to_unix(systime_t time);
timespec64_t time_conv_to_unix64(systime_t time);

systime_t time_conv_to_systime(timespec_t time);
systime_t time_conv_to_systime64(timespec64_t time);

#endif //TIME_H
