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
    list_insert(driverList, (void*)node);
    driverCount++;
}

void device_driver_register(const char* path){

    if(driverCount != 0){
        driver_node_t* node;
        foreach(item, driverList){
            node = (driver_node_t*)item->value;
            if(!strcmp(node->path, path)) return -1; //driver already registered
        }
    }

    file_t* file = kfopen(path, FILE_ACCESS_R);
    if(!file){
        klog("DAL: Failed to register driver \'%s\': %s!\n", KLOG_FAILED, path, kstrerror(kerrno));
        return -1;
    }
    device_driver_t* driver = elf_find_symbol(file->address, "driver_metadata");
    if(!driver){
        klog("DAL: Failed to register driver \'%s\': Could not find \"driver_metadata\" structure!\n", KLOG_FAILED, path);
        goto fail;
    }
    device_id_t* driverIDs = elf_find_symbol(file->address, "driver_ids");
    if(!driverIDs){
        klog("DAL: Failed to register driver \'%s\': Could not find driver ID list!\n", KLOG_FAILED, path);
        goto fail;
    }

    size_t idListSize = 0;
    while(driverIDs[idListSize] != 0) idListSize++;
    if(idListSize == 0){
        klog("DAL: Failed to register driver \'%s\': No IDs in ID List!\n", KLOG_FAILED, path);
        goto fail;
    }
    idListSize++;

    device_id_t* driverIDsStore = kmalloc(sizeof(device_id_t) * idListSize);

    memcpy(driverIDsStore, driverIDs, sizeof(device_id_t) * idListSize);

    driver_node_t* driverNode = kmalloc(sizeof(driver_node_t));
    driverNode->driver = NULL;
    driverNode->ids = driverIDsStore;
    driverNode->loaded = false;
    driverNode->path = kmalloc(strlen(path)+1);
    memcpy(driverNode->path, path, strlen(path)+1);

    device_driver_add(driverNode);

    klog("Registered Driver \'%s\'.\n", KLOG_OK, path);
    kfclose(file);

    device_attach_to_driver(driverNode);

    return 0;

fail:
    kfclose(file);
    return -1;
}

//true - device attached, false - device not attached
bool device_driver_attach(device_t* device){
    if(driverCount == 0) return false;
    device_driver_t* driver;
    driver_node_t* node;
    bool status;
    int i = 0;

    lock(driverManagerLock, {
        foreach(item, driverList){
            i = 0;
            status = false;
            node = (driver_node_t*)item->value;
            if(!node || !node->ids) continue;

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
                        DEVICE_ID_PCI_SUBCLASS(node->ids[i]) == DEVICE_ID_PCI_SUBCLASS(device->id)) status = true;
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

device_driver_t device_driver_get_driver(size_t i){
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