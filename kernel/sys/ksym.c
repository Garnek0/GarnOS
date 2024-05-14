/*  
*   File: ksym.c
*
*   Author: Garnek
*   
*   Description: Kernel Symbol table list. Without it, modules cant call kernel functions
*                And acces kernel valiables and constants
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "ksym-internals.h"

#include <garn/ksym.h>
#include <garn/kstdio.h>
#include <sys/bootloader.h>
#include <garn/mm.h>
#include <garn/spinlock.h>
#include <garn/ds/list.h>

list_t* ksymList;

spinlock_t ksymLock;

void ksym_init(){
    ksymList = list_create();
    Elf64_Ehdr* h = (Elf64_Ehdr*)bl_get_kernel_file_address();

    for(size_t i = 0; i < h->e_shnum; i++){
		Elf64_Shdr* sh = (Elf64_Shdr*)(bl_get_kernel_file_address() + h->e_shoff + h->e_shentsize * i);
        sh->sh_addr = (Elf64_Addr)(bl_get_kernel_file_address() + sh->sh_offset);
	}

    for(size_t i = 0; i < h->e_shnum; i++){
		Elf64_Shdr* sh = (Elf64_Shdr*)(bl_get_kernel_file_address() + h->e_shoff + h->e_shentsize * i);
		if(sh->sh_type != SHT_SYMTAB) continue;

		Elf64_Shdr* strtab_hdr = (Elf64_Shdr*)(bl_get_kernel_file_address() + h->e_shoff + h->e_shentsize * sh->sh_link);
		char* symNames = (char*)strtab_hdr->sh_addr;
		Elf64_Sym* symTable = (Elf64_Sym*)sh->sh_addr;

		for(size_t sym = 0; sym < sh->sh_size / sizeof(Elf64_Sym); sym++) {
			if(symTable[sym].st_shndx != SHN_UNDEF) {
                ksym_add(symNames + symTable[sym].st_name, symTable[sym].st_value);
			}
		}
	}
}

void ksym_add(char* name, Elf64_Addr address){
    lock(ksymLock, {
        ksym_entry_t* newKsym = (ksym_entry_t*)kmalloc(sizeof(ksym_entry_t));
        memset(newKsym, 0, sizeof(ksym_entry_t));

        newKsym->address = (address - 0xFFFFFFFF80000000 + bl_get_kernel_virt_base());
        newKsym->name = name;

        list_insert(ksymList, (void*)newKsym);
    });
}

Elf64_Addr ksym_find(char* name){
    lock(ksymLock, {
        ksym_entry_t* ksym;
        foreach(i, ksymList){
            ksym = (ksym_entry_t*)i->value;
            if(!strcmp(ksym->name, name)){
                releaseLock(&ksymLock);
                return ksym->address;
            }
        }
    });
    return (Elf64_Addr)-1;
}