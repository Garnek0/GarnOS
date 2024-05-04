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
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <hw/pci/pci.h>
#include <cpu/multiproc/spinlock.h>
#include <ds/list.h>
#include <acpi/tables/tables.h>
#include <kstdio.h>
#include <sys/dal/dal.h>

spinlock_t deviceManagerLock;

list_t* deviceList;

size_t deviceCount;

void device_init(){
    deviceList = list_create();
}

void device_add(device_t* device){
    list_insert(deviceList, (void*)device);
    deviceCount++;
    device_driver_attach(device);
}

int device_remove(device_t* device){
    if(device->node && device->node->loaded && device->node->driver->remove){
        if(!device->node->driver->remove(device)){
            klog("DAL: Could not remove device \'%s\'! Driver returned false.\n", KLOG_FAILED, device->name);
            return -1;
        }
    }
    if(list_remove(deviceList, device) != 0) return -1;
    deviceCount--;
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

bool device_attach_to_driver(driver_node_t* node){
    if(deviceCount == 0) return false;
    device_t* device;
    device_driver_t* driver;
    bool status;
    int i = 0;

    lock(deviceManagerLock, {
        foreach(item, deviceList){
            i = 0;
            status = false;
            device = (device_t*)item->value;
            if(!device) continue;

            for(;; i++){        
                if(node->ids[i] == 0) break;
                switch(DEVICE_ID_CLASS(node->ids[i])){
                    case DEVICE_ID_CLASS_PS2:
                    {
                        if(node->ids[i] == device->id) status = true;
                        break;
                    }
                    case DEVICE_ID_CLASS_PCI:
                    {
                        if((DEVICE_ID_PCI_VENDOR(node->ids[i]) == DEVICE_ID_PCI_VENDOR(device->id) || DEVICE_ID_PCI_VENDOR(node->ids[i]) == DEVICE_ID_PCI_VENDOR_ANY) &&
                        (DEVICE_ID_PCI_DEVICE(node->ids[i]) == DEVICE_ID_PCI_DEVICE(device->id) || DEVICE_ID_PCI_DEVICE(node->ids[i]) == DEVICE_ID_PCI_DEVICE_ANY) &&
                        DEVICE_ID_PCI_CLASS(node->ids[i]) == DEVICE_ID_PCI_CLASS(device->id) &&
                        DEVICE_ID_PCI_SUBCLASS(node->ids[i]) == DEVICE_ID_PCI_SUBCLASS(device->id) &&
                        (DEVICE_ID_PCI_PROGIF(node->ids[i]) == DEVICE_ID_PCI_PROGIF(device->id) || DEVICE_ID_PCI_PROGIF(node->ids[i]) == DEVICE_ID_PCI_PROGIF_ANY)) status = true;
                        break;
                    }
                    default:
                        break;

                }
                if(status){
                    if(!node->loaded) elf_load_driver(node);
                    break;
                }
            }
            if(!status) continue;
 
            driver = (device_driver_t*)node->driver;
            if(!driver || !driver->probe){
                continue;
            }

            status = driver->probe(device);
            if(status){
                device->node = node;
                klog("DAL: Found Driver for %s\n", KLOG_OK, device->name);
                status = driver->attach(device);
                if(status){
                    releaseLock(&deviceManagerLock);
                    return true;
                } else {
                    klog("DAL: Failed to attach device %s to %s\n", KLOG_FAILED, device->name, node->path);
                    device->node = NULL;
                }
            }
        }
    });

    return false;
}