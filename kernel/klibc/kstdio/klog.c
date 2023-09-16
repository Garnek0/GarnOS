/*  
*   File: klog.c
*
*   Author: Garnek
*   
*   Description: Kernel logging
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "kstdio.h"
#include <term/term.h>
#include <drivers/serial/serial.h>

spinlock_t klogLock;

void klog(char* fmt, uint8_t status, ...){
    va_list args;
    va_start(args, fmt);
    uint16_t r, g, b;

    lock(klogLock, {
        //TODO: Add string formatting for this:
        serial_log(fmt);
        serial_log("\r");

        uint32_t colour = term_get_colour(FOREGROUND);
        r = ((colour >> 16) & 0xff);
        g = ((colour >> 8) & 0xff);
        b = (colour & 0xff);

        switch(status){
            case KLOG_OK:
                kprintf("[ \e[38;2;0;255;0mOK\e[38;2;%d;%d;%dm ] ", r, g, b, args);
                break;
            case KLOG_FAILED:
                kprintf("[ \e[38;2;255;0;0mFAILED\e[38;2;%d;%d;%dm ] ", r, g, b, args);
                break;
            case KLOG_INFO:
                kprintf("[ \e[38;2;0;255;255mINFO\e[38;2;%d;%d;%dm ] ", r, g, b, args);
                break;
            case KLOG_WARNING:
                kprintf("[ \e[38;2;255;255;0mWARNING\e[38;2;%d;%d;%dm ] ", r, g, b, args);
                break;
            case KLOG_FATAL:
                kprintf("\n[ \e[38;2;140;0;0mFATAL\e[38;2;%d;%d;%dm ] ", r, g, b, args);
                break;
            default:
                break;
        }

        kvprintf(fmt, args);
        va_end(args);
    });
}