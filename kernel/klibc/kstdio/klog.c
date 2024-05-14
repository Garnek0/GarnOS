/*  
*   File: klog.c
*
*   Author: Garnek
*   
*   Description: Kernel logging
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <garn/kstdio.h>
#include <garn/term.h>
#include <garn/hw/serial.h>
#include <garn/timer.h>

spinlock_t klogLock;

void klog(char* fmt, uint8_t status, const char* component, ...){
    va_list args;
    va_start(args, component);
    uint16_t r, g, b;

    lock(klogLock, {
        uint32_t colour = term_get_colour(FOREGROUND);
        r = ((colour >> 16) & 0xff);
        g = ((colour >> 8) & 0xff);
        b = (colour & 0xff);

        //Print Timestamp

        size_t milliseconds = timer_get_ticks()%1000;
        size_t seconds = timer_get_ticks()/1000%1000000;

        size_t millisecondsCpy = milliseconds;
        size_t secondsCpy = seconds;

        int millisecondsDigitCount = 0;
        int secondsDigitCount = 0;

        while(millisecondsCpy != 0){
            millisecondsCpy/=10;
            millisecondsDigitCount++;
        }

        while(secondsCpy != 0){
            secondsCpy/=10;
            secondsDigitCount++;
        }

        if(millisecondsDigitCount == 0) millisecondsDigitCount = 1;
        if(secondsDigitCount == 0) secondsDigitCount = 1;

        kprintf("\e[38;2;180;180;180m");

        kputchar('[');

        for(int i = 0; i < 6-secondsDigitCount; i++){
            kputchar(' ');
        }

        kprintf("%u", seconds);
        kputchar('.');

        for(int i = 0; i < 3-millisecondsDigitCount; i++){
            kputchar('0');
        }

        kprintf("%u", milliseconds);

        kputchar(']');

        kprintf("\e[38;2;%d;%d;%dm", r, g, b);

        //Print Status

        switch(status){
            case KLOG_OK:
                kprintf("[   \e[38;2;0;255;0mOK\e[38;2;%d;%d;%dm   ] ", r, g, b, args);
                break;
            case KLOG_FAILED:
                kprintf("[ \e[38;2;255;0;0mFAILED\e[38;2;%d;%d;%dm ] ", r, g, b, args);
                break;
            case KLOG_INFO:
                kprintf("[  \e[38;2;0;255;255mINFO\e[38;2;%d;%d;%dm  ] ", r, g, b, args);
                break;
            case KLOG_WARNING:
                kprintf("[  \e[38;2;255;255;0mWARN\e[38;2;%d;%d;%dm  ] ", r, g, b, args);
                break;
            case KLOG_CRITICAL:
                kprintf("[\e[38;2;140;0;0mCRITICAL\e[38;2;%d;%d;%dm] ", r, g, b, args);
                break;
            default:
                break;
        }

        //Print component

        kprintf("\e[38;2;255;89;0m%s\e[38;2;%d;%d;%dm: ", component, r, g, b);

        //Print the actual message

        kvprintf(fmt, args);
        va_end(args);
    });
}