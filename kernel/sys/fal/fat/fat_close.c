/*  
*   File: fat_close.c
*
*   Author: Garnek
*   
*   Description: FAT Driver close routine
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fat.h"
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <kerrno.h>
#include <kstdio.h>

int fat_close(filesys_t* self, file_t* file){
    if(file->fsData) kmfree(file->fsData);
    kmfree(file);
}