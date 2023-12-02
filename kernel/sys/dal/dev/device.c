/*  
*   File: device.c
*
*   Author: Garnek
*   
*   Description: Device Manager
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "device.h"
#include "pcidev.h"
#include <mem/mm/kheap.h>
#include <mem/memutil/memutil.h>
#include <hw/pci/pci.h>
#include <cpu/smp/spinlock.h>
#include <ds/list.h>
#include <acpi/tables/tables.h>
#include <kstdio.h>

spinlock_t deviceManagerLock;

list_t* deviceList;

size_t deviceCount;

void device_init(){
    deviceList = list_create("DALdeviceList");
}

void device_add(device_t* device){
    lock(deviceManagerLock, {
        list_insert(deviceList, (void*)device);
        deviceCount++;
    });
}

size_t device_get_device_count(){
    return deviceCount;
}

device_t device_get_device(size_t i){
    device_t* device;
    size_t count = 0;

    if(i >= deviceCount) i = deviceCount;

    lock(deviceManagerLock, {
        foreach(item, deviceList){
            device = (device_t*)item->value;
            if(count == i) break;
            count++;
        }
    });

    return *device;
}

//TODO: Remove this
device_t* new_device(){
    device_t* device = kmalloc(sizeof(device_t));
    memset(device, 0, sizeof(device_t));
    device_add(device);
    return device;
}