#ifndef KSYM_TABLE_H
#define KSYM_TABLE_H

#include <types.h>
#include <exec/elfabi.h>

typedef struct _ksym_entry {
    Elf64_Addr address;
    char* name;
    struct _ksym_entry* next;
} ksym_entry_t;

void ksym_init();
void ksym_add(char* name, Elf64_Addr address);
Elf64_Addr ksym_find(char* name);

#endif //KSYM_TABLE_H