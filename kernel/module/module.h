/*  
*   File: module.h
*
*   Author: Garnek
*   
*   Description: Module Metadata struct definition & other module related stuff
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef MODMGR_H
#define MODMGR_H

#include <types.h>

typedef struct {
	char* name;
	void (*init)(void);
	void (*fini)(void);
} module_t;

typedef struct _loaded_mod_list_entry {
	size_t size;
	void* address;
	module_t* metadata;
} loaded_mod_list_entry_t;

void module_init();
bool module_list_search(char* name);
loaded_mod_list_entry_t* module_list_get(char* name);
void module_list_add(loaded_mod_list_entry_t entry);

#endif //MODMGR_H