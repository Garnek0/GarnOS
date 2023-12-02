/*  
*   File: driver.c
*
*   Author: Garnek
*   
*   Description: Device Driver Manager
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "driver.h"
#include <mem/mm/kheap.h>
#include <mem/memutil/memutil.h>
#include <hw/pci/pci.h>
#include <cpu/smp/spinlock.h>
#include <ds/list.h>
#include <kstdio.h>
#include <kerrno.h>
#include <sys/fal/fal.h>
#include <exec/elf.h>

spinlock_t driverManagerLock;

list_t* driverList;

size_t driverCount;

void driver_init(){
    driverList = list_create("DALdriverList");   
}

void device_driver_add(driver_node_t* node){
    lock(driverManagerLock, {
        list_insert(driverList, (void*)node);
        driverCount++;
    });
}

//true - device attached, false - device not attached
bool device_driver_attach(device_t* device){
    if(driverCount == 0) return false;
    device_driver_t* driver;
    driver_node_t* node;
    bool status = false;

    lock(driverManagerLock, {
        foreach(item, driverList){
            node = (driver_node_t*)item->value;
            driver = (device_driver_t*)node->driver;
            if(!driver || !driver->probe){
                continue;
            }
            status = driver->probe(device);
            if(status){
                device->node = node;
                status = driver->attach(device);
                if(status){
                    klog("DAL: Found Driver for %s\n", KLOG_OK, device->name);
                    releaseLock(&driverManagerLock);
                    return true;
                } else device->node = NULL;
            }
        }
    });

    return false;
}

size_t device_driver_get_driver_count(){
    return driverCount;
}

device_driver_t device_get_driver(size_t i){
    device_driver_t* driver;
    size_t count = 0;

    if(i >= driverCount) i = driverCount;

    lock(driverManagerLock, {
        foreach(item, driverList){
            driver = (device_t*)item->value;
            if(count == i) break;
            count++;
        }
    });

    return *driver;
}