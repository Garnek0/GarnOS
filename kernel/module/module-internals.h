#pragma once

#include <garn/types.h>
#include <garn/module.h>

typedef struct _loaded_mod_list_entry {
	size_t size;
	void* address;
	module_t* metadata;
} loaded_mod_list_entry_t;

void module_init();
bool module_list_search(char* name);
loaded_mod_list_entry_t* module_list_get(char* name);
void module_list_add(loaded_mod_list_entry_t entry);
void module_shutdown();
