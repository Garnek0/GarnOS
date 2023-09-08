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

char kputchar(char chr){
    return term_putchar(chr);
}