/*  
*   File: dal.h
*
*   Author: Garnek
*   
*   Description: Device Abstraction Layer. Provides Abstractions for
*                Things like Drives or Buses.
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef DAL_H
#define DAL_H

#include <types.h>
#include <module/module.h>
#include <kstdio.h>

#include "dev/device.h"
#include "dev/driver.h"
#include "stor/drive.h"
#include "stor/part/part.h"
#include "stor/bcache.h"
#include "checksysfs.h"
#include <cpu/smp/cpus.h>

#include <sys/dal/dev/pcidev.h>
#include <sys/dal/dev/miscdev.h>
#include <sys/panic.h>

static inline void dal_init(){
    device_init(); //initialise device manager

    klog("DAL Initialised\n", KLOG_OK);

    bcache_init(); //Initialise buffer cache

    cpus_init(); //initialize CPUs

    driver_init(); //initialise device driver manager
    module_init(); //initialise module manager

    pcidev_init(); //detect pci devices
    miscdev_init(); //detect misc devices

    if(!checksysfs_check()) panic("System FS Not found or Inaccessible!");
}

#endif //DAL_H