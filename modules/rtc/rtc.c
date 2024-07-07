/*
*   Module: rtc.sys
*
*   File: rtc.c
*
*   Module Author: Garnek
*
*   Module Description: RTC Driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "rtc.h"

#include <garn/arch.h>
#include <garn/irq.h>
#include <garn/irq.h>
#include <garn/kstdio.h>
#include <garn/spinlock.h>
#include <garn/dal/dal.h>
#include <garn/module.h>
#include <garn/time.h>
#include <garn/timer.h>

#include <uacpi/acpi.h>
#include <uacpi/tables.h>

rtc_t rtc;

static bool hourMode = false;
static bool binary = false;

struct acpi_fadt* FADT;

spinlock_t rtcLock;

static uint8_t rtc_read_register(uint8_t reg){
    lock(rtcLock, {
        //disable NMIs. It takes just one
        //interrupt to put the CMOS in a dodgy state. (Interrupts should already be disabled
        //by the init function/irq handler)
        //Not a problem for VMs but a huge pain for real hardware
        //select the desired register
        arch_outb(RTC_ADDRESS, reg | 0x80);
    });

    return arch_inb(RTC_DATA);
}

static void rtc_write_register(uint8_t reg, uint8_t data){
    lock(rtcLock, {
        //disable NMIs.
        //select the desired register
        arch_outb(RTC_ADDRESS, reg | 0x80);
        arch_outb(RTC_DATA, data);
    });
}

//convert bcd to binary. Needed if bcd mode is enabled
static uint8_t rtc_bcd_to_bin(uint8_t bcd){
    return ((bcd & 0xF0) >> 1) + ((bcd & 0xF0) >> 3) + (bcd & 0x0F);
}

//RTC handler
void rtc_handler(stack_frame_t* regs){


    //Status C must be read after each interrupt anyway
    if(!(rtc_read_register(RTC_STATUS_C) & ((1 << 4) | (1 << 7)))){
        return;
    }

    //get the time data from CMOS
    rtc.seconds = rtc_read_register(RTC_SECONDS);
    rtc.minutes = rtc_read_register(RTC_MINUTES);
    rtc.hours = rtc_read_register(RTC_HOURS);
    rtc.weekday = rtc_read_register(RTC_WEEKDAY);
    rtc.dayOfMonth = rtc_read_register(RTC_DAY_OF_MONTH);
    rtc.month = rtc_read_register(RTC_MONTH);
    rtc.year = rtc_read_register(RTC_YEAR);
    if(FADT->century) rtc.century = rtc_read_register(RTC_CENTURY);
    else rtc.century = 19;

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
            rtc.century = rtc_bcd_to_bin(rtc.century);
        }

        //if 12 hour format, convert to 24 hour format
        if(!hourMode && (rtc.hours & 0x80)) {
            rtc.hours = ((rtc.hours & 0x7F)+12) % 24;
        }
    });

    //Assemble systime
    systime_t time;
    time.seconds = rtc.seconds;
    time.minutes = rtc.minutes;
    time.hours = rtc.hours;
    time.dayOfMonth = rtc.dayOfMonth;
    time.month = rtc.month;
    time.year = rtc.century*100 + rtc.year;

    time_set(time);
}

void init(){
    return;
}

void fini(){
	irq_remove_handler(8, rtc_handler);
	klog("No longer receiving RTC interrupts\n", KLOG_OK, "RTC");
    return;
}

bool probe(device_t* device){
    if(!(device->type == DEVICE_TYPE_SYSTEM_DEVICE) || !(device->bus == DEVICE_BUS_NONE)) return false;

    //Check for any CMOS POST errors (errors may also mean the RTC is missing)
    if(!(rtc_read_register(RTC_STATUS_D) & (1 << 7))){
        uint8_t postError = rtc_read_register(RTC_POST_ERROR);
        if(postError & (1 << 2)){
            klog("Invalid RTC Time or RTC not present!\n", KLOG_FAILED, "RTC");
            return false;
        } else if(postError & (1 << 5)){
            klog("Invalid Configuration or RTC not present!\n", KLOG_FAILED, "RTC");
            return false;
        } else if(postError & (1 << 6)){
            klog("CMOS Checksum bad or RTC not present!\n", KLOG_FAILED, "RTC");
            return false;
        } else if(postError & (1 << 7)){
            klog("Clock lost power or RTC not present!\n", KLOG_FAILED, "RTC");
            return false;
        }
    }

    return true;
}

//initialise the rtc
bool attach(device_t* device){
    if(!probe(device)) return false;

    arch_disable_interrupts();

    //get Status Register B and fill in binary and hourMode accordingly
    uint8_t statb = rtc_read_register(RTC_STATUS_B);
    if(statb & RTC_BINARY) binary = true;
    if(statb & RTC_HOUR_FORMAT) hourMode = true;

    //turn on update ended interrupts, turn off everything else
    statb |= RTC_UPDATE_ENDED_INT;
    statb &= ~(RTC_ALARM_INT | RTC_PERIODIC_INT);
    rtc_write_register(RTC_STATUS_B, statb);

    uacpi_table FADTTable;
    uacpi_table_find_by_signature(ACPI_FADT_SIGNATURE, &FADTTable);
    FADT = (struct acpi_fadt*)FADTTable.virt_addr;

    irq_add_handler(8, rtc_handler, 0);
    //This needs to be here on real hardware for the RTC to function.
    //I dont know why, but RTC interrupts won't work otherwise.
    //(I think it may be because of previous unhandled ints occuring while the system is powered off?)
    rtc_read_register(RTC_STATUS_C);

    arch_enable_interrupts();;

    klog("RTC Initialised Successfully.\n", KLOG_OK, "RTC");

    return true;
}

bool remove(device_t* device){
	fini();
    return true;
}

module_t metadata = {
    .name = "rtc",
    .init = init,
    .fini = fini
};

device_driver_t driver_metadata = {
    .probe = probe,
    .attach = attach,
    .remove = remove
};

device_id_t driver_ids[] = {
    DEVICE_CREATE_ID_TIMER(DEVICE_ID_TIMER_RTC),
    0
};
