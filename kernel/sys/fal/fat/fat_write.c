/*  
*   File: fat_write.c
*
*   Author: Garnek
*   
*   Description: FAT Driver write routine
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fat.h"
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <kerrno.h>
#include <kstdio.h>

ssize_t fat_write(filesys_t* self, file_t* file, size_t size, void* buf, size_t offset){
    return 0;
}