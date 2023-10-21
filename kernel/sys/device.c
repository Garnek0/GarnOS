/*  
*   File: device.c
*
*   Author: Garnek
*   
*   Description: Device Manager
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "device.h"
#include <mem/mm/kheap.h>
#include <mem/memutil/memutil.h>
#include <hw/pci/pci.h>
#include <cpu/smp/spinlock.h>
#include <ds/list.h>
#include <kstdio.h>

spinlock_t deviceManagerLock;

list_t* deviceList;
list_t* driverList;

size_t deviceCount, driverCount;

void device_init(){
    deviceList = list_create("deviceList");
    driverList = list_create("driverList");    

}

void device_add(device_t* device){
    lock(deviceManagerLock, {
        list_insert(deviceList, (void*)device);
        deviceCount++;
    });
}

void device_driver_add(device_driver_t* driver){
    lock(deviceManagerLock, {
        list_insert(driverList, (void*)driver);
        driverCount++;
    });
}

//true - device attached, false - device not attached
bool device_driver_attach(device_t* device){
    if(driverCount == 0) return false;
    device_driver_t* driver;
    bool status = false;

    lock(deviceManagerLock, {
        foreach(item, driverList){
            driver = (device_driver_t*)item->value;
            if(!driver || !driver->probe){ 
                releaseLock(&deviceManagerLock);
                return false; 
            }
            status = driver->probe(device);
            if(status){
                status = driver->attach(device);
                if(status){
                    device->driver = driver;
                    releaseLock(&deviceManagerLock);
                        return true;
                }
                releaseLock(&deviceManagerLock);
                return false;
            }
        }
    });

    return false;
}

size_t device_get_device_count(){
    return deviceCount;
}

size_t device_get_driver_count(){
    return driverCount;
}

device_t device_get_device(size_t i){
    device_t* device;
    size_t count = 0;

    if(i >= deviceCount) i = deviceCount;

    foreach(item, deviceList){
        device = (device_t*)item->value;
        if(count == i) break;
        count++;
    }

    return *device;
}

device_driver_t device_get_driver(size_t i){
    device_driver_t* driver;
    size_t count = 0;

    if(i >= driverCount) i = driverCount;

    foreach(item, driverList){
        driver = (device_t*)item->value;
        if(count == i) break;
        count++;
    }

    return *driver;
}

device_t* new_device(){
    device_t* device = kmalloc(sizeof(device_t));
    memset(device, 0, sizeof(device_t));
    device_add(device);
    return device;
}