/*  
*   File: elf.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef ELF_H
#define ELF_H

#include <types.h>

int elf_load_module(char* modulePath);
void* elf_find_symbol(void* elf, const char* symbol);

#endif //ELF_H