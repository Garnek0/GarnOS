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
#include <sys/input.h>
#include <kerrno.h>

spinlock_t kreadlineLock;

char* kreadline(char* prompt){
    if(prompt) kprintf(prompt);
    char* line = kmalloc(256);
    size_t i = 0;
    size_t backspaces = 0;

    char chr;

    keyBuffer = 0;

    lock(kreadlineLock, {
        kerrno = 0;
        while(keyBuffer!='\n'){
            keyBuffer = 0;
            while(!keyBuffer) asm volatile("nop");
            if(keyBuffer == '\b' && backspaces){
                kputchar(keyBuffer);
                backspaces--;
                i--;
                continue;
            } else if (keyBuffer == '\b' && !backspaces){
                continue;
            }
            chr = kputchar(keyBuffer);
            if(tc.escape || chr == 0) continue;
            line[i] = keyBuffer;
            i++;
            backspaces++;
            if(i > 255){
                kprintf("\n");
                klog("kreadline Buffer Overflow!\n", KLOG_WARNING);
                kerrno = EOVERFLOW;
                line[0] = 0;
                break;
            }
        }
        line[i-1] = '\0';
    
    });
    return line;
}