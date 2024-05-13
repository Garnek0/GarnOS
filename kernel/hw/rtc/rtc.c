/*  
*   File: rtc.c
*
*   Author: Garnek
*   
*   Description: Realtime Clock Driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "rtc.h"
#include <hw/ports.h>
#include <cpu/interrupts/interrupts.h>
#include <cpu/interrupts/irq.h>
#include <kstdio.h>
#include <cpu/multiproc/spinlock.h>

rtc_t rtc;

static bool hourMode = false;
static bool binary = false;

spinlock_t rtcLock;

static uint8_t rtc_read_register(uint8_t reg){
    lock(rtcLock, {
        //disable NMIs. It takes just one
        //interrupt to put the CMOS in a dodgy state. (Interrupts should already be disabled
        //by the init function/irq handler)
        //Not a problem for VMs but a huge pain for real hardware
        //select the desired register
        outb(RTC_ADDRESS, reg | 0x80);
    });

    return inb(RTC_DATA);
}

static void rtc_write_register(uint8_t reg, uint8_t data){
    lock(rtcLock, {
        //disable NMIs.
        //select the desired register
        outb(RTC_ADDRESS, reg | 0x80);
        outb(RTC_DATA, data);
    });
}

//convert bcd to binary. Needed if bcd mode is enabled
static uint8_t rtc_bcd_to_bin(uint8_t bcd){
    return ((bcd & 0xF0) >> 1) + ((bcd & 0xF0) >> 3) + (bcd & 0x0F);
}

//actual rtc handler
void rtc_handler(stack_frame_t* regs){
        //get the time data from CMOS
        rtc.seconds = rtc_read_register(RTC_SECONDS);
        rtc.minutes = rtc_read_register(RTC_MINUTES);
        rtc.hours = rtc_read_register(RTC_HOURS);
        rtc.weekday = rtc_read_register(RTC_WEEKDAY);
        rtc.dayOfMonth = rtc_read_register(RTC_DAY_OF_MONTH);
        rtc.month = rtc_read_register(RTC_MONTH);
        rtc.year = rtc_read_register(RTC_YEAR);

    lock(rtcLock, {
        //if bcd mode, convert to binary
        if(!binary){
            rtc.seconds = rtc_bcd_to_bin(rtc.seconds);
            rtc.minutes = rtc_bcd_to_bin(rtc.minutes);
            rtc.hours = rtc_bcd_to_bin(rtc.hours);
            rtc.weekday = rtc_bcd_to_bin(rtc.weekday);
            rtc.dayOfMonth = rtc_bcd_to_bin(rtc.dayOfMonth);
            rtc.month = rtc_bcd_to_bin(rtc.month);
            rtc.year = rtc_bcd_to_bin(rtc.year);
        }

        //if 12 hour format, convert to 24 hour format
        if(!hourMode && (rtc.hours & 0x80)) {
            rtc.hours = ((rtc.hours & 0x7F)+12) % 24;
        }
    });

        //Status C must be read after each interrupt
        rtc_read_register(RTC_STATUS_C);
}

//initialise the rtc
void rtc_init(){

    asm volatile("cli");

    //get Status Register B and fill in binary and hourMode accordingly
    uint8_t statb = rtc_read_register(RTC_STATUS_B);
    if(statb & RTC_BINARY) binary = true;
    if(statb & RTC_HOUR_FORMAT) hourMode = true;

    //turn on update ended interrupts
    statb |= RTC_UPDATE_ENDED_INT;
    rtc_write_register(RTC_STATUS_B, statb);

    irq_add_handler(8, rtc_handler, 0);
    //This needs to be here on real hardware for the RTC to function.
    //I dont know why, but RTC interrupts won't work otherwise.
    rtc_read_register(RTC_STATUS_C);

    asm volatile("sti");

    klog("RTC Initialised Successfully.\n", KLOG_OK, "RTC");
}