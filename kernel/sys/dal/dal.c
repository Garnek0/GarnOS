/*  
*   File: dal.c
*
*   Author: Garnek
*   
*   Description: Device Abstraction Layer. Provides Abstractions for
*                Things like Drives or Buses.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <sys/dal/dal-internals.h>
#include <garn/panic.h>
#include <hw/pit/pit.h>
#include <hw/rtc/rtc.h>

void dal_init(){
    device_init(); //Initialise device manager

    klog("DAL Initialised\n", KLOG_OK, "DAL");

    bcache_init(); //Initialise buffer cache

    multiproc_init(); //Initialize CPUs

    pit_init(); //Initialise PIT Timer

    rtc_init(); //Initialise realtime clock

    driver_init(); //Initialise device driver manager
    module_init(); //Initialise module manager

    pcidev_init(); //Detect pci devices
    miscdev_init(); //Detect misc devices

    if(!checksysfs_check()) panic("System FS Not found or Inaccessible!", "DAL");
}