/*  
*   File: module.c
*
*   Author: Garnek
*   
*   Description: Module Manager
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <module/module-internals.h>
#include <exec/elf.h>
#include <garn/mm.h>
#include <garn/arch.h>
#include <sys/fal/initrd/initrd.h>
#include <garn/fal/file.h>
#include <garn/spinlock.h>
#include <garn/ds/list.h>
#include <garn/kstdio.h>

list_t* moduleList;

spinlock_t moduleListLock;

void module_init(){
    initrd_init(); //initialise initial ramdisk
    //init should always be 0:

    if(device_driver_autoreg("0:/drv/initreg.txt") != 0){
        panic("initreg.txt not found on initrd!", "FAL");
    } else

    moduleList = list_create();
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

        list_insert(moduleList, (void*)newModListEntry);
    });
}

void module_shutdown(){
	loaded_mod_list_entry_t* entry;
	lock(moduleListLock, {
		foreach(item, moduleList){
			entry = (loaded_mod_list_entry_t*)item->value;
			entry->metadata->fini();
		}
	});
	return;
}
