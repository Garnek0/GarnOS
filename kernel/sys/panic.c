/*  
*   File: panic.c
*
*   Author: Garnek
*   
*   Description: Kernel Panic Routine
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "panic.h"
#include <kstdio.h>

void panic(char* str, ...){

    va_list args;
    va_start(args, str);

    klog("Kernel Panic! ", KLOG_FATAL);
    kvprintf(str, args);

    va_end(args);
    
    asm("cli");
    while(true){
        asm("hlt");
    }
}