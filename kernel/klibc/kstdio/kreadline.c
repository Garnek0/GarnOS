/*  
*   File: kreadline.c
*
*   Author: Garnek
*   
*   Description: Kernel readline. Reads characters from stdin.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <kstdio.h>
#include <term/term.h>
#include <mem/mm/kheap.h>

spinlock_t kreadlineLock;

char* kreadline(char* prompt){
    if(prompt) kprintf(prompt);
    char* line = kmalloc(256);
    size_t i = 0;
    size_t backspaces = 0;

    char chr;

    stdin = 0;

    lock(kreadlineLock, {
        while(stdin!='\n'){
            stdin = 0;
            while(!stdin) asm volatile("nop");
            if(stdin == '\b' && backspaces){
                kputchar(stdin);
                backspaces--;
                i--;
                continue;
            } else if (stdin == '\b' && !backspaces){
                continue;
            }
            chr = kputchar(stdin);
            if(tc.escape || chr == 0) continue;
            line[i] = stdin;
            i++;
            backspaces++;
            if(i > 255){
                kprintf("\n");
                klog("kreadline Buffer Overflow!\n", KLOG_WARNING);
                line[0] = 0;
                break;
            }
        }
        line[i-1] = '\0';
    
    });
    return line;
}