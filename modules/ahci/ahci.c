/*  
*   Module: ahci.mod
*
*   File: ahci.c
*
*   Module Author: Garnek
*   
*   Mdoule Description: AHCI Driver
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <module/module.h>
#include <kstdio.h>

void init(){
    return;
}

void fini(){
    return;
}

module_t metadata = {
    .name = "ahci",
    .init = init,
    .fini = fini
};