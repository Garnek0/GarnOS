#pragma once

#include <garn/types.h>
#include "elfabi.h"
#include <garn/dal/dal.h>
#include <process/process.h>

int elf_load_module(char* modulePath);
void* elf_find_symbol(void* elf, const char* symbol);
bool elf_validate(Elf64_Ehdr* h, Elf64_Half etype);
int elf_load_driver(driver_node_t* node);
int elf_exec_load(process_t* process, char* path);
int elf_calculate_relocations(Elf64_Ehdr* h, void* elf_module);
