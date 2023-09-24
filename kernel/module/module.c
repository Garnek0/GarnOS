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
#include <fs/initrd/initrd.h>
#include <acpi/tables/tables.h>
#include <cpu/smp/spinlock.h>

loaded_mod_list_entry_t* modListEntryLast;
loaded_mod_list_entry_t* modListEntryFirst;

spinlock_t moduleListLock;

void module_init(){
    initrd_init(); //initialise initial ramdisk

    modListEntryLast = modListEntryFirst = kmalloc(sizeof(loaded_mod_list_entry_t));
    memset(modListEntryFirst, 0, sizeof(loaded_mod_list_entry_t));

    //init should always be 0:

    //predefined modules

    //TODO: first make sure the machine supports ide
    elf_load_module("0:/ide.mod");

    //TODO: first make sure the machine supports ahci
    elf_load_module("0:/ahci.mod");

    if(FADT!=NULL && (FADT->bootArchitectureFlags & (1 << 1))){
        elf_load_module("0:/ps2.mod");
        elf_load_module("0:/ps2kb.mod");
    }
}

bool module_list_search(char* name){
    loaded_mod_list_entry_t* modListEntry = modListEntryFirst;

    lock(moduleListLock, {
        while(modListEntry && modListEntry->metadata){
            if(!strcmp(name, modListEntry->metadata->name)){
                releaseLock(&moduleListLock);
                return true;
            }
            modListEntry = modListEntry->next;
        }
    });

    return false;
}

loaded_mod_list_entry_t* module_list_get(char* name){
    loaded_mod_list_entry_t* modListEntry = modListEntryFirst;

    lock(moduleListLock, {
        while(modListEntry && modListEntry->metadata){
            if(!strcmp(name, modListEntry->metadata->name)){
                releaseLock(&moduleListLock);
                return modListEntry;
            }
            modListEntry = modListEntry->next;
        }
    });

    return NULL;
}

void module_list_add(loaded_mod_list_entry_t entry){
    lock(moduleListLock, {
        loaded_mod_list_entry_t* newModListEntry = kmalloc(sizeof(loaded_mod_list_entry_t));
        memset((void*)newModListEntry, 0, sizeof(loaded_mod_list_entry_t));

        modListEntryLast->address = entry.address;
        modListEntryLast->metadata = entry.metadata;
        modListEntryLast->size = entry.size;
        modListEntryLast->next = NULL;

        modListEntryLast->next = newModListEntry;
        modListEntryLast = newModListEntry;
    });
}