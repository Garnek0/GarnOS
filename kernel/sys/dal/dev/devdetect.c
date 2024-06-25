/*  
*   File: miscdev.c
*
*   Author: Garnek
*   
*   Description: Misc devices such as a PS/2 controller
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <sys/dal/dal-internals.h>
#include <garn/dal/dal.h>
#include <uacpi/acpi.h>
#include <uacpi/tables.h>
#include <garn/mm.h>
#include <garn/arch.h>
#include <garn/config.h>

void devdetect(){
    uacpi_table FADTTable;
    uacpi_table_find_by_signature(ACPI_FADT_SIGNATURE, &FADTTable);
    struct acpi_fadt* FADT = (struct acpi_fadt*)FADTTable.virt_addr;

#ifdef CONFIG_INCLUDE_PIT_DRIVER

    device_t* pitDev = kmalloc(sizeof(device_t));
    pitDev->bus = DEVICE_BUS_NONE;
    pitDev->data = NULL;
    pitDev->name = "Programmable Interval Timer";
    pitDev->node = NULL;
    pitDev->type = DEVICE_TYPE_SYSTEM_DEVICE;
    pitDev->id = DEVICE_CREATE_ID_TIMER(DEVICE_ID_TIMER_PIT);
    device_add(pitDev);

#endif //CONFIG_INCLUDE_PIT_DRIVER

#ifdef CONFIG_INCLUDE_i8042_DRIVER

    //Detect i8042 PS/2 Controller

    if(FADT!=NULL && (FADT->iapc_boot_arch & (1 << 1))){
        goto i8042_found;
    } else {
        arch_inb(0x60);

        uint8_t res;
        arch_outb(0x64, 0xAA);
        size_t timeout = 100000;

        while(timeout--) if(arch_inb(0x64) & 1) break;
        if(!timeout) goto i8042_not_found;
        res = arch_inb(0x60);

        if(res == 0x55) goto i8042_found;
    }

i8042_found:
    device_t* ps2controller = kmalloc(sizeof(device_t));
    ps2controller->bus = DEVICE_BUS_NONE;
    ps2controller->data = NULL;
    ps2controller->name = "i8042 PS/2 Controller";
    ps2controller->node = NULL;
    ps2controller->type = DEVICE_TYPE_INPUT_CONTROLLER;
    ps2controller->id = DEVICE_CREATE_ID_PS2;
    device_add(ps2controller);

i8042_not_found:
    ;

#endif //CONFIG_INCLUDE_i8042_DRIVER

#ifdef CONFIG_INCLUDE_RTC_DRIVER

    device_t* rtcDev = kmalloc(sizeof(device_t));
    rtcDev->bus = DEVICE_BUS_NONE;
    rtcDev->data = NULL;
    rtcDev->name = "Real-Time Clock";
    rtcDev->node = NULL;
    rtcDev->type = DEVICE_TYPE_SYSTEM_DEVICE;
    rtcDev->id = DEVICE_CREATE_ID_TIMER(DEVICE_ID_TIMER_RTC);
    device_add(rtcDev);

#endif //CONFIG_INCLUDE_RTC_DRIVER

#ifdef CONFIG_INCLUDE_PCI_DRIVER

    pcidev_detect();

#endif //CONFIG_INCLUDE_PCI_DRIVER

}