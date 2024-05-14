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
#include <garn/acpi/acpi-tables.h>
#include <garn/mm.h>
#include <garn/hw/ports.h>

void miscdev_init(){
    //Detect PS/2 Controller

    if(FADT!=NULL && (FADT->bootArchitectureFlags & (1 << 1))){
        goto ps2_found;
    } else {
        inb(0x60);

        uint8_t res;
        outb(0x64, 0xAA);
        size_t timeout = 100000;

        while(timeout--) if(inb(0x64) & 1) break;
        if(!timeout) goto ps2_not_found;
        res = inb(0x60);

        if(res == 0x55) goto ps2_found;
    }

ps2_found:
    device_t* ps2controller = kmalloc(sizeof(device_t));
    ps2controller->bus = DEVICE_BUS_NONE;
    ps2controller->data = NULL;
    ps2controller->name = "i8042 PS/2 Controller";
    ps2controller->node = NULL;
    ps2controller->type = DEVICE_TYPE_INPUT_CONTROLLER;
    ps2controller->id = DEVICE_CREATE_ID_PS2;
    device_add(ps2controller);

ps2_not_found:
    ;
}