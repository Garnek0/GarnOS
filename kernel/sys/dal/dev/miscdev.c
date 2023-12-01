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

void miscdev_init(){
    //Detect PS/2 Devices
    if(FADT!=NULL && (FADT->bootArchitectureFlags & (1 << 1))){
        elf_load_module("0:/ps2.mod");
        //TODO: move this somewhere in the DAL
        klog("DAL: Found Driver for PS2 Controller\n", KLOG_OK);
    }
}