/*  
*   File: rtc.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef RTC_H
#define RTC_H

#include <types.h>

#define RTC_ADDRESS 0x70
#define RTC_DATA 0x71

#define RTC_SECONDS 0x00
#define RTC_MINUTES 0x02
#define RTC_HOURS 0x04
#define RTC_WEEKDAY 0x06
#define RTC_DAY_OF_MONTH 0x07
#define RTC_MONTH 0x08
#define RTC_YEAR 0x09

#define RTC_STATUS_A 0x0A
#define RTC_STATUS_B 0x0B
#define RTC_STATUS_C 0x0C

#define RTC_HOUR_FORMAT (1 << 1)
#define RTC_BINARY (1 << 2)
#define RTC_UPDATE_ENDED_INT (1 << 4)

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t weekday;
    uint8_t dayOfMonth;
    uint8_t month;
    uint8_t year;
} rtc_t;
extern rtc_t rtc;

void rtc_init();

#endif //RTC_H