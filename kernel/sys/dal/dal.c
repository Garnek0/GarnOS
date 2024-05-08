/*  
*   File: dal.c
*
*   Author: Garnek
*   
*   Description: Device Abstraction Layer. Provides Abstractions for
*                Things like Drives or Buses.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "dal.h"
#include <sys/panic.h>

void dal_init(){
    device_init(); //initialise device manager

    klog("DAL Initialised\n", KLOG_OK, "DAL");

    bcache_init(); //Initialise buffer cache

    multiproc_init(); //initialize CPUs

    driver_init(); //initialise device driver manager
    module_init(); //initialise module manager

    pcidev_init(); //detect pci devices
    miscdev_init(); //detect misc devices

    if(!checksysfs_check()) panic("System FS Not found or Inaccessible!", "DAL");
}