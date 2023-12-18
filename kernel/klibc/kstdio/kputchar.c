/*  
*   File: kputchar.c
*
*   Author: Garnek
*   
*   Description: Kernel putchar. Wrapper to term_putchar.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <kstdio.h>
#include <term/term.h>

//this is just a wrapper, it doesnt need a lock as
//the wrapped function (term_putchar) already has one

char kputchar(char chr){
    return term_putchar(chr);
}