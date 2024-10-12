#include "elf.h"
#include "elfabi.h"
#include <garn/fal/vnode.h>
#include <garn/panic.h>
#include <garn/ksym.h>
#include <mem/mm-internals.h>
#include <module/module-internals.h>
#include <garn/spinlock.h>
#include <garn/kstdio.h>
#include <garn/kerrno.h>
#include <garn/kernel.h>
#include <garn/config.h>

spinlock_t moduleLoaderLock;

void* elf_find_symbol(void* elf, const char* symbol){
	Elf64_Ehdr* h = (Elf64_Ehdr*)elf;

	lock(moduleLoaderLock, {
		for(size_t i = 0; i < h->e_shnum; i++){
			Elf64_Shdr* sh = (Elf64_Shdr*)(elf + h->e_shoff + h->e_shentsize * i);
			sh->sh_addr = (Elf64_Addr)(elf + sh->sh_offset);
		}

		for(size_t i = 0; i < h->e_shnum; i++){
			Elf64_Shdr* sh = (Elf64_Shdr*)(elf + h->e_shoff + h->e_shentsize * i);
			if(sh->sh_type != SHT_SYMTAB) continue;

			Elf64_Shdr* strtab_hdr = (Elf64_Shdr*)(elf + h->e_shoff + h->e_shentsize * sh->sh_link);
			char* symNames = (char*)strtab_hdr->sh_addr;
			Elf64_Sym* symTable = (Elf64_Sym*)sh->sh_addr;

			for(size_t sym = 0; sym < sh->sh_size / sizeof(Elf64_Sym); sym++) {
				if(symTable[sym].st_shndx != SHN_UNDEF && symTable[sym].st_shndx < SHN_LORESERVE && symTable[sym].st_value < hhdmOffset) {
					Elf64_Shdr* sh_hdr = (Elf64_Shdr*)(elf + h->e_shoff + h->e_shentsize * symTable[sym].st_shndx);
					symTable[sym].st_value = symTable[sym].st_value + sh_hdr->sh_addr;
				}
				
				if(symTable[sym].st_shndx != SHN_UNDEF && symTable[sym].st_name && !strcmp(symNames + symTable[sym].st_name, symbol)){
					releaseLock(&moduleLoaderLock);
					return (void*)symTable[sym].st_value;
				}
			}
		}
	});
	return NULL;
}

bool elf_validate(Elf64_Ehdr* h, Elf64_Half etype){
	kerrno = 0;

	if (h->e_ident[0] != ELFMAG0 ||
	    h->e_ident[1] != ELFMAG1 ||
	    h->e_ident[2] != ELFMAG2 ||
	    h->e_ident[3] != ELFMAG3) {
		kerrno = ENOEXEC;
		return false;
	}

    if (h->e_ident[EI_CLASS] != ELFCLASS64) {
		kerrno = ENOEXEC;
		return false;
	}

    if (h->e_type != etype) {
		kerrno = ENOEXEC;
		return false;
	}

#ifdef __x86_64__
	if(h->e_machine != EM_X86_64){
		kerrno = ENOEXEC;
		return false;
	}
#elif ARCH_DUMMY
	;
#endif

	return true;
}

int elf_calculate_relocations(Elf64_Ehdr* h, void* elf_module){
	for(size_t i = 0; i < h->e_shnum; i++){
		Elf64_Shdr* sh = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * i);
		if(sh->sh_type != SHT_RELA) continue;

		Elf64_Rela* table = (Elf64_Rela*)sh->sh_addr;

		Elf64_Shdr* targetSection = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * sh->sh_info);

		Elf64_Shdr* symbolSection = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * sh->sh_link);
		Elf64_Sym* symbolTable = (Elf64_Sym*)symbolSection->sh_addr;

#define S (symbolTable[ELF64_R_SYM(table[rela].r_info)].st_value)
#define A (table[rela].r_addend)
#define P  ((uint64_t)target)

		for (size_t rela = 0; rela < sh->sh_size / sizeof(Elf64_Rela); rela++) {
			void* target = (void*)(table[rela].r_offset + targetSection->sh_addr);
			uint64_t Relx86_64_64 = S + A;
			uint32_t Relx86_64_32 = S + A;
			uint32_t Relx86_64_PC32 = S + A - P;
			switch (ELF64_R_TYPE(table[rela].r_info)) {
				case R_X86_64_64:
					memcpy(target, &Relx86_64_64, sizeof(uint64_t));
					break;
				case R_X86_64_32:
					memcpy(target, &Relx86_64_32, sizeof(uint32_t));
					break;
				case R_X86_64_PC32:
					memcpy(target, &Relx86_64_PC32, sizeof(uint32_t));
					break;
				default:
					kerrno = ENOEXEC;
					return -1;
					break;
			}
		}
	}

#undef S
#undef A
#undef P

	return 0;
}

//Common between elf_load_module() and elf_load_driver().
static int elf_module_load_common(Elf64_Ehdr* h, void* elf_module, const char* path, module_t** modData, device_driver_t** driverData){
	kerrno = 0;

	//allocate SHT_NOBITS sections and fill in sh_addr fields to have
	//the correct load addresses for each section
	for(size_t i = 0; i < h->e_shnum; i++){
		Elf64_Shdr* sh = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * i);
		if(sh->sh_type == SHT_NOBITS){
			if(!sh->sh_size || !(sh->sh_flags & SHF_ALLOC)) continue;
			sh->sh_addr = (Elf64_Addr)kmalloc(sh->sh_size);
			memset((void*)sh->sh_addr, 0, sh->sh_size);
		} else {
			sh->sh_addr = (Elf64_Addr)(elf_module + sh->sh_offset);
			if (sh->sh_addralign && (sh->sh_addr & (sh->sh_addralign - 1))) {
				klog("Section of module %s not correctly aligned! (sh_addr: 0x%x, sh_addralign: %u)\n", KLOG_WARNING, "ML", path, sh->sh_addr, sh->sh_addralign);
			}
		}
	}

	//Load symbols
	for(size_t i = 0; i < h->e_shnum; i++){
		Elf64_Shdr* sh = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * i);
		if(sh->sh_type != SHT_SYMTAB) continue;

		Elf64_Shdr* strtab_hdr = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * sh->sh_link);
		char* symNames = (char*)strtab_hdr->sh_addr;
		Elf64_Sym* symTable = (Elf64_Sym*)sh->sh_addr;

		for(size_t sym = 0; sym < sh->sh_size / sizeof(Elf64_Sym); sym++) {
			if(symTable[sym].st_shndx != SHN_UNDEF && symTable[sym].st_shndx < SHN_LORESERVE) {
				Elf64_Shdr* sh_hdr = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * symTable[sym].st_shndx);
				symTable[sym].st_value = symTable[sym].st_value + sh_hdr->sh_addr;
			} else if(symTable[sym].st_shndx == SHN_UNDEF) { //need to load kernel symbol here.
				//look for the kernel symbol in the kernel symbol list.
				symTable[sym].st_value = ksym_find(symNames + symTable[sym].st_name);
				if((int64_t)symTable[sym].st_value == -1){
					klog("Kernel Module/Driver '%s' contains an invalid symbol (%s)! Unloading!\n", KLOG_FAILED, "ML", path, symNames + symTable[sym].st_name);
					kerrno = ENOEXEC;
					return -1;
				}
			}

			//if the symbol is called "metadata", then save it. (We will need the metadata struct to
			//retrieve info about the module.)
			//if the symbol is called "driver_metadata", then this module is a driver and should
			//be treated accordingly
			if(symTable[sym].st_name && !strcmp(symNames + symTable[sym].st_name, "metadata")) {
				if(modData) *modData = (module_t*)symTable[sym].st_value;
			} else if(symTable[sym].st_name && !strcmp(symNames + symTable[sym].st_name, "driver_metadata")){
				if(driverData) *driverData = (device_driver_t*)symTable[sym].st_value;
			}
		}
	}

	//if no metadata struct was found, call the module invalid (and unload).
	if(modData && !*modData){
		klog("Module '%s' has invalid metadata struct! Unloading!\n", KLOG_FAILED, "ML", path);
		kerrno = ENOEXEC;
		return -1;
	}

	//...
	if(driverData && !*driverData){
		klog("Driver '%s' has invalid driverdata struct! Unloading!\n", KLOG_FAILED, "ML", path);
		kerrno = ENOEXEC;
		return -1;
	}

	//calculate relocations
	if(elf_calculate_relocations(h, elf_module) != 0){
		if(driverData) klog("Could not load Driver \'%s\': %s\n", KLOG_FAILED, "ML", path, kstrerror(kerrno));
		else klog("Could not load Kernel Module \'%s\': %s\n", KLOG_FAILED, "ML", path, kstrerror(kerrno));
		return -1;
	}

	return 0;
}

int elf_load_module(char* modulePath){
	kerrno = 0;
	int err;

    Elf64_Ehdr* h = kmalloc(sizeof(Elf64_Ehdr));
    vnode_t* file = vnode_open(modulePath, O_RDONLY, 0);
	err = kerrno;

	if(!file){
		klog("Could not load Module \'%s\': %s\n", KLOG_FAILED, "ML", modulePath, kstrerror(err));
		kmfree(h);
		return -1;
	}

    vnode_read(file, sizeof(Elf64_Ehdr), h, 0);

	//Validate module
	if(!elf_validate(h, ET_REL)){
		klog("Could not load Module \'%s\': %s\n", KLOG_FAILED, "ML", modulePath, kstrerror(kerrno));
		kmfree(h);
		return -1;
	}

	//FIXME: slapping the whole thing in memory without doing any alignment checks seems
	//to cause some issues later on in the loading phase.
	void* elf_module = kmalloc(file->size);
	vnode_read(file, file->size, (void*)elf_module, 0);

	module_t* modData = NULL;

	if(elf_module_load_common(h, elf_module, modulePath, &modData, NULL) != 0){
		goto unload;
	}

	module_t* modDataStore = kmalloc(sizeof(module_t));
	modDataStore->name = kmalloc(strlen(modData->name)+1);
	memcpy(modDataStore, modData, sizeof(module_t));
	memcpy(modDataStore->name, modData->name, strlen(modData->name)+1);

	//loading done, now check if the module is already loaded.
	//If it is, then unload its copy.
	loaded_mod_list_entry_t entry;
	entry.address = (void*)elf_module;
	entry.metadata = modData;
	entry.size = file->size;

	lock(moduleLoaderLock, {
		if(module_list_search(entry.metadata->name)){
			for(size_t i = 0; i < h->e_shnum; i++){
				Elf64_Shdr* sh = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * i);
				if(sh->sh_type == SHT_NOBITS){
					kmfree((void*)sh->sh_addr);
				}
			}
			kerrno = EEXIST;
			klog("Could not load Kernel Module \'%s\': %s\n", KLOG_FAILED, "ML", modulePath, kstrerror(kerrno));
			releaseLock(&moduleLoaderLock);
			goto unload;
		}
	});

	module_list_add(entry);

	//cleanup

	vnode_close(file);

	klog("Loaded Module \'%s\'\n", KLOG_OK, "ML", modulePath);

	modData->init();

	return 0;

unload:
	kmfree((void*)elf_module);
	vnode_close(file);
	return -1;
}

int elf_load_driver(driver_node_t* node){
	kerrno = 0;
	int err;

    Elf64_Ehdr* h = kmalloc(sizeof(Elf64_Ehdr));
    vnode_t* file = vnode_open(node->path, O_RDONLY, 0);

	err = kerrno;

	if(!file){
		klog("Could not load Driver \'%s\': %s\n", KLOG_FAILED, "ML", node->path, kstrerror(err));
		kmfree(h);
		return -1;
	}

	vnode_read(file, sizeof(Elf64_Ehdr), h, 0);

	//Validate module
	if(!elf_validate(h, ET_REL)){
		klog("Could not load Driver \'%s\': %s\n", KLOG_FAILED, "ML", node->path, kstrerror(kerrno));
		kmfree(h);
		return -1;
	}

	void* elf_module = kmalloc(file->size);
	vnode_read(file, file->size, (void*)elf_module, 0);

	module_t* modData = NULL;
	device_driver_t* driverData = NULL;

	if(elf_module_load_common(h, elf_module, node->path, &modData, &driverData) != 0){
		goto unload;
	}

	module_t* modDataStore = kmalloc(sizeof(module_t));
	modDataStore->name = kmalloc(strlen(modData->name)+1);
	memcpy(modDataStore, modData, sizeof(module_t));
	memcpy(modDataStore->name, modData->name, strlen(modData->name)+1);

	//loading done, now check if the module is already loaded.
	//If it is, then unload its copy.
	loaded_mod_list_entry_t entry;
	entry.address = (void*)elf_module;
	entry.metadata = modDataStore;
	entry.size = file->size;

	lock(moduleLoaderLock, {
		if(module_list_search(entry.metadata->name)){
			for(size_t i = 0; i < h->e_shnum; i++){
				Elf64_Shdr* sh = (Elf64_Shdr*)(elf_module + h->e_shoff + h->e_shentsize * i);
				if(sh->sh_type == SHT_NOBITS){
					kmfree((void*)sh->sh_addr);
				}
			}
			kerrno = EEXIST;
			klog("Could not load Driver \'%s\': %s\n", KLOG_FAILED, "ML", node->path, kstrerror(kerrno));
			releaseLock(&moduleLoaderLock);
			goto unload;
		}
	});

	module_list_add(entry);
	if(driverData){
		device_driver_t* driver = kmalloc(sizeof(device_driver_t));
		memcpy(driver, driverData, sizeof(device_driver_t));
		node->driver = driver;
		node->loaded = true;
	}

	//cleanup

	vnode_close(file);

	klog("Loaded Driver \'%s\'\n", KLOG_OK, "ML", node->path);

	modData->init();

	return 0;

unload:
	kmfree((void*)elf_module);
	vnode_close(file);
	return -1;
}

int elf_exec_load(process_t* process, char* path){
	kerrno = 0;

	vnode_t* file = vnode_open(path, O_RDWR, 0);

	if(!file){
		kerrno = ENOENT;
		return -1;
	}

	Elf64_Ehdr* h = kmalloc(sizeof(Elf64_Ehdr));

	vnode_read(file, sizeof(Elf64_Ehdr), h, 0);

	//TODO: handle ET_DYN
	if(!elf_validate(h, ET_EXEC)){
		klog("Could not load executable %s! Executable is corrupt!\n", KLOG_FAILED, "ELF", path);
		return -1;
	}
	
	void* elfExec = kmalloc(file->size);
	vnode_read(file, file->size, elfExec, 0);

	Elf64_Phdr* phdr;

	for(int i = 0; i < h->e_phnum; i++) {
		phdr = (Elf64_Phdr*)((uint64_t)elfExec + h->e_phoff + h->e_phentsize * i);
		if(phdr->p_type == PT_LOAD) {
			vaspace_create_area(process->pt, phdr->p_vaddr, phdr->p_memsz, VMM_PRESENT | VMM_USER | VMM_RW);

			//TODO: this is not very efficient since it requires flushing the tlb. Make a 
			//vaspace_memcpy() or sth that can memcpy into another address space
			vaspace_switch(process->pt);

			memcpy((void*)phdr->p_vaddr, (void*)((uint64_t)elfExec + phdr->p_offset), phdr->p_filesz);

			for(size_t i = phdr->p_filesz; i < phdr->p_memsz; i++) {
				*(uint8_t*)(phdr->p_vaddr + i) = 0;
			}

			vaspace_switch(vmm_get_kernel_pt());
		}
	}

	process->mainThread->regs.rip = (uint64_t)h->e_entry;

	kmfree(elfExec);
	kmfree(h);

	klog("Program '%s' Loaded.\n", KLOG_OK, "exec", path);

	return 0;
}
