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
#include <cpu/multiproc/spinlock.h>

#define KLOG_OK 0
#define KLOG_FAILED 1
#define KLOG_INFO 2
#define KLOG_WARNING 3
#define KLOG_CRITICAL 4

extern bool kernelScreenOut;

int kprintf(char* str, ...);
int kvprintf(char* str, va_list args);
char kputchar(char chr);
void klog(char* fmt, uint8_t status, const char* component, ...);
char* kreadline(char* prompt);
const char* kstrerror(int err);

inline void kernel_screen_output_disable();
inline void kernel_screen_output_enable();

#endif //KSTDIO_H