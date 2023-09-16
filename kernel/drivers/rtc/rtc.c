/*  
*   File: rtc.c
*
*   Author: Garnek
*   
*   Description: Realtime Clock Driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "rtc.h"
#include <drivers/ports.h>
#include <cpu/interrupts/interrupts.h>
#include <kstdio.h>
#include <sys/rblogs.h>
#include <cpu/smp/spinlock.h>

rtc_t rtc;

static bool hourMode = false;
static bool binary = false;

spinlock_t rtcLock;

static uint8_t rtc_read_register(uint8_t reg){
    lock(rtcLock, {
        //disable interrupts and NMIs. It takes just one
        //interrupt to put the CMOS in a dodgy state.
        //Not a problem for VMs but a huge pain for real hardware
        asm volatile("cli");
        //select the desired register
        outb(RTC_ADDRESS, reg | 0x80);
    });

    return inb(RTC_DATA);
    asm volatile("sti");
}

static void rtc_write_register(uint8_t reg, uint8_t data){
    lock(rtcLock, {
        //disable interrupts and NMIs.
        asm volatile("cli");
        //select the desired register
        outb(RTC_ADDRESS, reg | 0x80);
        outb(RTC_DATA, data);
        asm volatile("sti");
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
    //get Status Register B and fill in binary and hourMode accordingly
    uint8_t statb = rtc_read_register(RTC_STATUS_B);
    if(statb & RTC_BINARY) binary = true;
    if(statb & RTC_HOUR_FORMAT) hourMode = true;

    //turn on update ended interrupts
    statb |= RTC_UPDATE_ENDED_INT;
    rtc_write_register(RTC_STATUS_B, statb);

    irqHandler.rtc_handler = rtc_handler;
    //This needs to be here on real hardware for the RTC to function.
    //I dont know why, but RTC interrupts won't work otherwise.
    rtc_read_register(RTC_STATUS_C);

    klog("RTC Initialised Successfully.\n", KLOG_OK);
    rb_log("RTC", KLOG_OK);
}