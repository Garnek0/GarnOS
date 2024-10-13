#include <garn/hw/serial.h>
#include "serial-internals.h"
#include <garn/arch/common.h>
#include <garn/arch/x86_64.h>
#include <garn/mm.h>
#include <garn/spinlock.h>
#include <garn/kstdio.h>

static bool serialPresent;
static bool enableSerialLogs = false;

spinlock_t serialLock;

//initialise the serial console
int serial_init(){
    serialPresent = true;

    //initialise the UART
    arch_outb(COM_INT, 0x00);
    arch_outb(COM_LINE_CONTROL, 0x80);
    arch_outb(COM_DIVISOR_LSB, 0x0C);
    arch_outb(COM_DIVISOR_MSB, 0x00);
    arch_outb(COM_LINE_CONTROL, 0x03);
    arch_outb(COM_FIFO_CTRL, 0xC7);
    arch_outb(COM_MODEM_CTRL, 0x0B);
    arch_outb(COM_MODEM_CTRL, 0x1E);

    //Theres a 1 in 256 chance an unavailable/disconnected
    //Serial will return the correct value for a single test.
    //To minimise that chance, run 3 tests instead.

    //test the serial port
    arch_outb(COM_DATA, 0xAE);
    if(arch_inb(COM_DATA) != 0xAE) {
        serialPresent = false;
        klog("Serial Console Not Initialised. Serial not present or disconnected?\n", KLOG_FAILED, "Serial");
        return 1;
    }

    arch_outb(COM_DATA, 0x56);
    if(arch_inb(COM_DATA) != 0x56) {
        serialPresent = false;
        klog("Serial Console Not Initialised. Serial not present or disconnected?\n", KLOG_FAILED, "Serial");
        return 1;
    }

    arch_outb(COM_DATA, 0xA3);
    if(arch_inb(COM_DATA) != 0xA3) {
        serialPresent = false;
        klog("Serial Console Not Initialised. Serial not present or disconnected?\n", KLOG_FAILED, "Serial");
        return 1;
    }
 
	arch_outb(COM_MODEM_CTRL, 0x0F);

	serial_enable_logs();

	serial_log("Serial initialised for logging.\n\r");
	klog("Serial Console Initialised.\n", KLOG_OK, "Serial");


	return 0;
}

void serial_write(uint8_t data){
    if(!serialPresent || !enableSerialLogs) return;

    lock(serialLock, {
        //poll bit 5 of the line status register
        while((arch_inb(COM_LINE_STATUS) & 0x20) == 0);
        arch_outb(COM_DATA, data);
    });
}

void serial_log(const char* str){
    if(!serialPresent || !enableSerialLogs) return;

    for(uint32_t i = 0; i < strlen(str); i++){
        serial_write(str[i]);
    }
}

void serial_enable_logs(){
    enableSerialLogs = true;
}

void serial_disable_logs(){
	enableSerialLogs = false;
}
