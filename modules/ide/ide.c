/*  
*   Module: ide.mod
*
*   File: ide.c
*
*   Module Author: Garnek
*   
*   Mdoule Description: IDE Driver
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
    .name = "ide",
    .init = init,
    .fini = fini
};