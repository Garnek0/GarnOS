/*  
*   File: elf.c
*
*   Author: Garnek
*   
*   Description: utils for loading and managing ELF
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "elfabi.h"
#include "elf.h"
#include <fs/vfs/vfs.h>
#include <sys/panic.h>
#include <sys/ksym.h>
#include <mem/mm/kheap.h>
#include <mem/memutil/memutil.h>
#include <module/module.h>

void elf_load_module(char* modulePath){
    Elf64_Ehdr* h;
    vfs_file_t* file = vfs_open(modulePath, VFS_FILE_ACCESS_R);

    h = (Elf64_Ehdr*)file->address;

    if (h->e_ident[0] != ELFMAG0 ||
	    h->e_ident[1] != ELFMAG1 ||
	    h->e_ident[2] != ELFMAG2 ||
	    h->e_ident[3] != ELFMAG3) {
		panic("Corrupt Kernel Module \'%s\'! Invalid Header.", modulePath);
	}

    if (h->e_ident[EI_CLASS] != ELFCLASS64) {
		panic("Invalid Kernel Module \'%s\'! ELF Class not 64-bit.", modulePath);
	}

    if (h->e_type != ET_REL) {
		panic("Invalid Kernel Module \'%s\'! Module not a relocatable object.", modulePath);
	}

	uint8_t* elf_module = (uint8_t*)kmalloc(file->size);
	vfs_read(file, file->size, (void*)elf_module);

	for(size_t i = 0; i < h->e_shnum; i++){
		Elf64_Shdr* sh = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * i);
		if(sh->sh_type == SHT_NOBITS){
			sh->sh_addr = (Elf64_Addr)kmalloc(sh->sh_size);
			memset((void*)sh->sh_addr, 0, sh->sh_size);
		} else {
			sh->sh_addr = (Elf64_Addr)(elf_module + sh->sh_offset);
			if (sh->sh_addralign && (sh->sh_addr & (sh->sh_addralign -1))) {
				klog("Module %s not correctly aligned!\n", KLOG_WARNING, modulePath);
			}
		}
	}

	module_t* modData = NULL;

	for(size_t i = 0; i < h->e_shnum; i++){
		Elf64_Shdr* sh = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * i);
		if(sh->sh_type != SHT_SYMTAB) continue;

		Elf64_Shdr* strtab_hdr = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * sh->sh_link);
		char* symNames = (char*)strtab_hdr->sh_addr;
		Elf64_Sym* symTable = (Elf64_Sym*)sh->sh_addr;

		for(size_t sym = 0; sym < sh->sh_size / sizeof(Elf64_Sym); sym++) {

			if(symTable[sym].st_shndx > 0 && symTable[sym].st_shndx < SHN_LORESERVE) {
				Elf64_Shdr* sh_hdr = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * symTable[sym].st_shndx);
				symTable[sym].st_value = symTable[sym].st_value + sh_hdr->sh_addr;
			} else if(symTable[sym].st_shndx == SHN_UNDEF) {
				symTable[sym].st_value = ksym_find(symNames + symTable[sym].st_name);
			}

			if(symTable[sym].st_name && !strcmp(symNames + symTable[sym].st_name, "metadata")) {
				modData = (void*)symTable[sym].st_value;
			}
		}
	}

	for(size_t i = 0; i < h->e_shnum; i++){
		Elf64_Shdr* sh = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * i);
		if(sh->sh_type != SHT_RELA) continue;

		Elf64_Rela* table = (Elf64_Rela*)sh->sh_addr;

		Elf64_Shdr* targetSection = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * sh->sh_info);

		Elf64_Shdr* symbolSection = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * sh->sh_link);
		Elf64_Sym* symbolTable = (Elf64_Sym*)symbolSection->sh_addr;

#define S (symbolTable[ELF64_R_SYM(table[rela].r_info)].st_value)
#define A (table[rela].r_addend)
#define T32 (*(uint32_t*)target)
#define T64 (*(uint64_t*)target)
#define P  (target)

		for (size_t rela = 0; rela < sh->sh_size / sizeof(Elf64_Rela); rela++) {
			size_t target = table[rela].r_offset + targetSection->sh_addr;
			switch (ELF64_R_TYPE(table[rela].r_info)) {
				case R_X86_64_64:
					T64 = S + A;
					break;
				case R_X86_64_32:
					T32 = S + A;
					break;
				case R_X86_64_PC32:
					T32 = S + A - P;
					break;
				default:
					panic("Invalid Kernel Module \'%s\'! Module has Unsupported Relocation %d", modulePath, ELF64_R_TYPE(table[rela].r_info));
					break;
			}
		}
	}

#undef S
#undef A
#undef T32
#undef T64
#undef P

	if(!modData){
		panic("Module %s has invalid/missing metadata structure!", modulePath);
	}

	loaded_mod_list_entry_t entry;
	entry.address = (void*)elf_module;
	entry.metadata = modData;
	entry.size = file->size;

	if(module_list_search(entry.metadata->name)){
		for(size_t i = 0; i < h->e_shnum; i++){
			Elf64_Shdr* sh = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * i);
			if(sh->sh_type == SHT_NOBITS){
				kmfree(sh->sh_addr);
			}
		}
		kmfree(elf_module);
		vfs_close(file);
		klog("Attempt to load already loaded module \'%s\'!\n", KLOG_WARNING, modulePath);
		return;
	}

	module_list_add(entry);

	vfs_close(file);

	klog("Module: Loaded Module \'%s\'\n", KLOG_OK, modulePath);

	modData->init();

}

