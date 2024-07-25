/*  
*   File: driver.c
*
*   Author: Garnek
*   
*   Description: Device Driver Manager
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <sys/dal/dal-internals.h>
#include <garn/mm.h>
#include <garn/hw/pci.h>
#include <garn/spinlock.h>
#include <garn/ds/list.h>
#include <garn/kstdio.h>
#include <garn/kerrno.h>
#include <garn/fal/vnode.h>
#include <exec/elf.h>

spinlock_t driverManagerLock;

list_t* driverList;

size_t driverCount;

void driver_init(){
    driverList = list_create();   
}

void device_driver_add(driver_node_t* node){
    list_insert(driverList, (void*)node);
    driverCount++;
}

int device_driver_register(const char* path){

    if(driverCount != 0){
        driver_node_t* node;
        foreach(item, driverList){
            node = (driver_node_t*)item->value;
            if(!strcmp(node->path, path)) return -1; //driver already registered
        }
    }

    vnode_t* file = vnode_open((char*)path, O_RDONLY, 0);

    if(!file){
        klog("Failed to register driver \'%s\': %s!\n", KLOG_FAILED, "DAL", path, kstrerror(kerrno));
        return -1;
    }

	//TODO: This may be slow with large drivers. Maybe add some way to load only part of the executable?

    void* elf = kmalloc(file->size);
    vnode_read(file, file->size, elf, 0);

	if(!elf_validate((Elf64_Ehdr*)elf, ET_REL)){
		klog("Failed to register driver \'%s\': Executable validation failed!\n", KLOG_FAILED, "DAL", path);
		goto fail;
	}

    device_driver_t* driver = elf_find_symbol(elf, "driver_metadata");
    if(!driver){
        klog("Failed to register driver \'%s\': Could not find \"driver_metadata\" structure!\n", KLOG_FAILED, "DAL", path);
        goto fail;
    }
    device_id_t* driverIDs = elf_find_symbol(elf, "driver_ids");
    if(!driverIDs){
        klog("Failed to register driver \'%s\': Could not find driver ID list!\n", KLOG_FAILED, "DAL", path);
        goto fail;
    }

	//calculate relocations (to get the strings)
	if(elf_calculate_relocations((Elf64_Ehdr*)elf, elf) != 0){
		klog("Failed to register driver \'%s\': %s\n", KLOG_FAILED, "DAL", path, kstrerror(kerrno));
		goto fail;
	}

    size_t idListSize = 0;
    while(driverIDs[idListSize].class != DEVICE_ID_CLASS_NONE) idListSize++;
    if(idListSize == 0){
        klog("Failed to register driver \'%s\': No IDs in ID List!\n", KLOG_FAILED, "DAL", path);
        goto fail;
    }
    idListSize++;

    device_id_t* driverIDsStore = kmalloc(sizeof(device_id_t) * idListSize);

    memcpy(driverIDsStore, driverIDs, sizeof(device_id_t) * idListSize);

	char* newStr;
	for(int i = 0; i < idListSize; i++){
		if(driverIDsStore[i].string[0] != NULL){
			newStr = kmalloc(strlen(driverIDsStore[i].string[0])+1);
			memcpy(newStr, driverIDsStore[i].string[0], strlen(driverIDsStore[i].string[0])+1);
			driverIDsStore[i].string[0] = newStr;
		}

		if(driverIDsStore[i].string[1] != NULL){
			newStr = kmalloc(strlen(driverIDsStore[i].string[1])+1);
			memcpy(newStr, driverIDsStore[i].string[1], strlen(driverIDsStore[i].string[1])+1);
			driverIDsStore[i].string[1] = newStr;
		}
	}

    driver_node_t* driverNode = kmalloc(sizeof(driver_node_t));
    driverNode->driver = NULL;
    driverNode->ids = driverIDsStore;
    driverNode->loaded = false;
    driverNode->path = kmalloc(strlen(path)+1);
    memcpy(driverNode->path, path, strlen(path)+1);

    device_driver_add(driverNode);

    klog("Registered Driver \'%s\'.\n", KLOG_OK, "DAL", path);
    kmfree(elf);
    vnode_close(file);

    device_attach_to_driver(driverNode);

    return 0;

fail:
    kmfree(elf);
    vnode_close(file);
    return -1;
}

int device_driver_unregister_node(driver_node_t* node){
    if(node->loaded){
        klog("Failed to unregister driver \'%s\'! Already Loaded.\n", KLOG_FAILED, "DAL", node->path);
        return -1;
    }
    if(list_remove(driverList, (void*)node) != 0) return -1;
    klog("Unregistered Driver \'%s\'.\n", KLOG_OK, "DAL", node->path);
    kmfree(node);
    return 0;
}

int device_driver_unregister(const char* path){
    driver_node_t* driverNode;
    foreach(node, driverList){
        driverNode = (driver_node_t*)node->value;
        if(!strcmp(driverNode->path, path)){
            return device_driver_unregister_node(driverNode);
        }
    }
    return -1;
}

//true - device attached, false - device not attached
bool device_driver_attach(device_t* device){
    if(driverCount == 0) return false;

    device_driver_t* driver;
    driver_node_t* node;
    bool status;
    int i = 0;

    foreach(item, driverList){
        i = 0;
        status = false;
        node = (driver_node_t*)item->value;
        if(!node || !node->ids) continue;

		for(;; i++){
			if(node->ids[i].class == DEVICE_ID_CLASS_NONE) break;
			foreach(id, device->idList){
				status = device_match_ids(node->ids[i], *((device_id_t*)id->value));
				if(status){
					klog("Found Possible Driver for %s\n", KLOG_OK, "DAL", device->name);
					if(!node->loaded) elf_load_driver(node);
    	            break;
				}
			}
			if(status) break;
        }


        driver = (device_driver_t*)node->driver;
        if(!driver || !driver->probe){
            continue;
        }

        status = driver->probe(device);
        if(status){
            device->node = node;
            klog("Found Driver for %s\n", KLOG_OK, "DAL", device->name);
            status = driver->attach(device);
            if(status){
                releaseLock(&driverManagerLock);
                return true;
            } else {
                klog("Failed to attach device %s to %s\n", KLOG_FAILED, "DAL", device->name, node->path);
                device->node = NULL;
            }
        }
    }

    return false;
}

int device_driver_autoreg(const char* path){
    vnode_t* file = vnode_open((char*)path, O_RDONLY, 0);
    
    if(!file){
        klog("Failed to load autoreg \'%s\'!\n", KLOG_FAILED, "DAL", path);
        return -1;
    }

    char str[128]; //should be enough for a single filename
    char* regPath;

    size_t ptr = 0;
    size_t tempPtr = 0;

    for(size_t i = 0; i < file->size; i++){
        vnode_read(file, 1, &str[ptr], i);
        if(str[ptr] == '\n'){
            if(ptr <= 0) continue;
            str[ptr] = 0;
            regPath = kmalloc(256);

            for(uint32_t j = 0; j < strlen(path); j++){
                if(path[j] == '/'){
                    tempPtr = j+1;
                    break;
                }
            }

            memcpy(regPath, path, tempPtr);
            memcpy(regPath+tempPtr, str, strlen(str)+1);

            device_driver_register(regPath);

            kmfree(regPath);
            ptr = 0;
            continue;
        }
        ptr++;
        if(ptr >= 128) ptr = 0;
    }
    vnode_close(file);
    return 0;
}

size_t device_driver_get_driver_count(){
    return driverCount;
}

device_driver_t device_driver_get_driver(size_t i){
    device_driver_t* driver = NULL;
    size_t count = 0;

    if(i >= driverCount) i = driverCount;

    lock(driverManagerLock, {
        foreach(item, driverList){
            driver = (device_driver_t*)item->value;
            if(count == i) break;
            count++;
        }
    });

    return *driver;
}
