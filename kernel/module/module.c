/*  
*   File: module.c
*
*   Author: Garnek
*   
*   Description: Module Manager
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "module.h"
#include <exec/elf.h>
#include <mem/memutil/memutil.h>
#include <mem/mm/kheap.h>
#include <hw/ports.h>
#include <sys/fal/initrd/initrd.h>
#include <sys/fal/fal.h>
#include <acpi/tables/tables.h>
#include <cpu/smp/spinlock.h>
#include <ds/list.h>
#include <kstdio.h>

list_t* moduleList;

spinlock_t moduleListLock;

void module_init(){
    initrd_init(); //initialise initial ramdisk
    //init should always be 0:

    moduleList = list_create("moduleList");

    //Detect PS/2 Devices
    if(FADT!=NULL && (FADT->bootArchitectureFlags & (1 << 1))){
        elf_load_module("0:/ps2.mod");
        klog("DAL: Found Driver for PS2 Controller\n", KLOG_OK);
    }
}

bool module_list_search(char* name){
    loaded_mod_list_entry_t* entry;

    lock(moduleListLock, {
        foreach(item, moduleList){
            entry = (loaded_mod_list_entry_t*)item->value;
            if(!strcmp(entry->metadata->name, name)){
                return true;
            }
        }
    });

    return false;
}

loaded_mod_list_entry_t* module_list_get(char* name){
    loaded_mod_list_entry_t* entry;

    lock(moduleListLock, {
        foreach(item, moduleList){
            entry = (loaded_mod_list_entry_t*)item->value;
            if(!strcmp(entry->metadata->name, name)){
                return entry;
            }
        }
    });

    return NULL;
}

void module_list_add(loaded_mod_list_entry_t entry){
    loaded_mod_list_entry_t* newModListEntry = kmalloc(sizeof(loaded_mod_list_entry_t));
    memset((void*)newModListEntry, 0, sizeof(loaded_mod_list_entry_t));

    lock(moduleListLock, {
        newModListEntry->address = entry.address;
        newModListEntry->metadata = entry.metadata;
        newModListEntry->size = entry.size;
        newModListEntry->next = NULL;

        list_insert(moduleList, (void*)newModListEntry);
    });
}