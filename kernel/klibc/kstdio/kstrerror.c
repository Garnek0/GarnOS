/*  
*   File: kstrerror.c
*
*   Author: Garnek
*   
*   Description: Error Printing
*/
// SPDX-License-Identifier: BSD-2-Clause

#include <kstdio.h>
#include <kerrno.h>

const char* kerrStr[] = {
    "Not administrator",
    "No such file or directory",
    "No such process",
    "Interrupted system call",
    "I/O error",
    "No such device or address",
    "Arg list too long",
    "Exec format error",
    "Bad file number",
    "No children",
    "No more processes",
    "Out of Mmemory",
    "Permission denied",
    "Bad address",
    "Block device required",
    "Mount device busy",
    "File exists",
    "Cross-device link",
    "No such device",
    "Not a directory",
    "Is a directory",
    "Invalid argument",
    "Too many open files in system",
    "Too many open files",
    "Not a typewriter",
    "Text file busy",
    "File too large",
    "No space left on device",
    "Illegal seek",
    "Read only file system",
    "Too many links",
    "Broken pipe",
    "Math arg out of domain of func"
    //TODO: Complete this list
};

void kperror(const char* str){
    int err = kerrno;
    kprintf("%s: %s", str, kerrStr[err-1]);
}

void kerrlog(const char* str, uint8_t loglevel){
    int err = kerrno;
    klog("%s: %s", str, kerrStr[err-1]);
}

char* kstrerror(int err){
    return kerrStr[err-1];
}