#include "rtc.h"
#include <drivers/ports.h>
#include <cpu/interrupts/interrupts.h>
#include <kstdio.h>
#include <sys/rblogs.h>

rtc_t rtc;

static bool hourMode = false;
static bool binary = false;

static uint8_t rtc_read_register(uint8_t reg){
    asm volatile("cli");
    outb(RTC_ADDRESS, reg | 0x80);
    return inb(RTC_DATA);
    asm volatile("sti");
}

static uint8_t rtc_write_register(uint8_t reg, uint8_t data){
    asm volatile("cli");
    outb(RTC_ADDRESS, reg | 0x80);
    outb(RTC_DATA, data);
    asm volatile("sti");
}

static uint8_t rtc_bcd_to_bin(uint8_t bcd){
    return ((bcd & 0xF0) >> 1) + ((bcd & 0xF0) >> 3) + (bcd & 0x0F);
}

void rtc_handler(stack_frame_t* regs){
    rtc.seconds = rtc_read_register(RTC_SECONDS);
    rtc.minutes = rtc_read_register(RTC_MINUTES);
    rtc.hours = rtc_read_register(RTC_HOURS);
    rtc.weekday = rtc_read_register(RTC_WEEKDAY);
    rtc.dayOfMonth = rtc_read_register(RTC_DAY_OF_MONTH);
    rtc.month = rtc_read_register(RTC_MONTH);
    rtc.year = rtc_read_register(RTC_YEAR);

    if(!binary){
        rtc.seconds = rtc_bcd_to_bin(rtc.seconds);
        rtc.minutes = rtc_bcd_to_bin(rtc.minutes);
        rtc.hours = rtc_bcd_to_bin(rtc.hours);
        rtc.weekday = rtc_bcd_to_bin(rtc.weekday);
        rtc.dayOfMonth = rtc_bcd_to_bin(rtc.dayOfMonth);
        rtc.month = rtc_bcd_to_bin(rtc.month);
        rtc.year = rtc_bcd_to_bin(rtc.year);
    }

    if(!hourMode && (rtc.hours & 0x80)) {
        rtc.hours = ((rtc.hours & 0x7F)+12) % 24;
    }

    rtc_read_register(RTC_STATUS_C);
}

void rtc_init(){
    uint8_t statb = rtc_read_register(RTC_STATUS_B);
    if(statb & RTC_BINARY) binary = true;
    if(statb & RTC_HOUR_FORMAT) hourMode = true;

    statb |= RTC_UPDATE_ENDED_INT;
    rtc_write_register(RTC_STATUS_B, statb);

    irqHandler.rtc_handler = rtc_handler;
    rtc_read_register(RTC_STATUS_C);

    klog("RTC Initialised Successfully.\n", KLOG_OK);
    rb_log("RTC", KLOG_OK);
}