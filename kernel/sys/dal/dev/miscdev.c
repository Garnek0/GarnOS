/*  
*   File: miscdev.c
*
*   Author: Garnek
*   
*   Description: Misc devices such as a PS/2 controller
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "miscdev.h"
#include <sys/dal/dal.h>
#include <acpi/tables/tables.h>
#include <mem/kheap/kheap.h>

void miscdev_init(){
    //Detect PS/2 Devices
    if(FADT!=NULL && (FADT->bootArchitectureFlags & (1 << 1))){
        device_t* ps2controller = kmalloc(sizeof(device_t));
        ps2controller->bus = DEVICE_BUS_NONE;
        ps2controller->data = NULL;
        ps2controller->name = "i8042 PS/2 Controller";
        ps2controller->node = NULL;
        ps2controller->type = DEVICE_TYPE_INPUT_CONTROLLER;
        ps2controller->id = DEVICE_CREATE_ID_PS2;
        device_add(ps2controller);
    }
}