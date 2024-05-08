/*  
*   File: fat_mkdir.c
*
*   Author: Garnek
*   
*   Description: FAT Driver create directory routine
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fat.h"
#include <mem/kheap/kheap.h>
#include <mem/memutil/memutil.h>
#include <kerrno.h>
#include <kstdio.h>

int fat_mkdir(filesys_t* self, char* path){
    return 0;
}