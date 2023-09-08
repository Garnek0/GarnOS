/*  
*   File: kstdio.h
*
*   Author: Garnek
*   
*   Description: Kernel stdio
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef KSTDIO_H
#define KSTDIO_H

#include <types.h>
#include <stdarg.h>

#define KLOG_OK 0
#define KLOG_FAILED 1
#define KLOG_INFO 2
#define KLOG_WARNING 3
#define KLOG_FATAL 4

extern char stdin;

int kprintf(char* str, ...);
int kvprintf(char* str, va_list args);
char kputchar(char chr);
void klog(char* str, uint8_t loglevel, ...);
char* kreadline(char* prompt);

#endif //KSTDIO_H