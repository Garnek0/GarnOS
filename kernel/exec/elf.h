/*  
*   File: elf.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef ELF_H
#define ELF_H

#include <types.h>
#include "elfabi.h"
#include <sys/dal/dal.h>

int elf_load_module(char* modulePath);
void* elf_find_symbol(void* elf, const char* symbol);
bool elf_validate(void* elf, Elf64_Half etype);
int elf_load_driver(driver_node_t* node);

#endif //ELF_H