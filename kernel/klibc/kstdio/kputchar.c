/*  
*   File: kputchar.c
*
*   Author: Garnek
*   
*   Description: Kernel putchar. Wrapper to term_putchar.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <kstdio.h>
#include <sys/term/term.h>

//this is just a wrapper, it doesnt need a lock as
//the wrapped function (term_putchar) already has one

bool kernelScreenOut = false;

char kputchar(char chr){
    if(kernelScreenOut){
        return term_putchar(chr);
    } else {
        return term_putchar_dbg(chr);
    }
}