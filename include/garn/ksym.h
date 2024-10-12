#ifndef KSYM_TABLE_H
#define KSYM_TABLE_H

#include <garn/types.h>
#include <exec/elfabi.h>

typedef struct {
    Elf64_Addr address;
    char* name;
} ksym_entry_t;

void ksym_add(char* name, Elf64_Addr address);
Elf64_Addr ksym_find(char* name);

#endif //KSYM_TABLE_H
