/*  
*   File: serial.c
*
*   Author: Garnek
*   
*   Description: Serial Console Driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "serial.h"
#include <hw/ports.h>
#include <mem/memutil/memutil.h>
#include <cpu/smp/spinlock.h>

#include <kstdio.h>

static bool serialPresent = false;

spinlock_t serialLock;

//initialise the serial console
int serial_init(){

    serialPresent = true;

    //initialise the UART
    outb(COM_INT, 0x00);
    outb(COM_LINE_CONTROL, 0x80);
    outb(COM_DIVISOR_LSB, 0x0C);
    outb(COM_DIVISOR_MSB, 0x00);
    outb(COM_LINE_CONTROL, 0x03);
    outb(COM_FIFO_CTRL, 0xC7);
    outb(COM_MODEM_CTRL, 0x0B);
    outb(COM_MODEM_CTRL, 0x1E);

    //Theres a 1 in 256 chance an unavailable/disconnected
    //Serial will return the correct value for a single test.
    //To minimise that chance, run 3 tests instead.

    //test the serial port
    outb(COM_DATA, 0xAE);
    if(inb(COM_DATA) != 0xAE) {
        serialPresent = false;
        klog("Serial Console Not Initialised. Serial not present or disconnected?\n", KLOG_FAILED);
        return 1;
    }

    outb(COM_DATA, 0x56);
    if(inb(COM_DATA) != 0x56) {
        serialPresent = false;
        klog("Serial Console Not Initialised. Serial not present or disconnected?\n", KLOG_FAILED);
        return 1;
    }

    outb(COM_DATA, 0xA3);
    if(inb(COM_DATA) != 0xA3) {
        serialPresent = false;
        klog("Serial Console Not Initialised. Serial not present or disconnected?\n", KLOG_FAILED);
        return 1;
    }
 
    outb(COM_MODEM_CTRL, 0x0F);

    serial_log("Serial Initialised.\n\r");
    klog("Serial Console Initialised.\n", KLOG_OK);

    return 0;
}

void serial_write(uint8_t data){
    if(!serialPresent) return;

    lock(serialLock, {
        //poll bit 5 of the line status register
        while((inb(COM_LINE_STATUS) & 0x20) == 0);
        outb(COM_DATA, data);
    });
}

void serial_log(const char* str){
    if(!serialPresent) return;

    for(uint32_t i = 0; i < strlen(str); i++){
        serial_write(str[i]);
    }
}