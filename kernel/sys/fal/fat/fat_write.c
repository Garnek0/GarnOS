/*  
*   File: fat_write.c
*
*   Author: Garnek
*   
*   Description: FAT Driver write routine
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fat.h"
#include <garn/mm.h>
#include <garn/kerrno.h>
#include <garn/kstdio.h>

ssize_t fat_write(vfs_t* self, vnode_t* file, size_t size, void* buf, size_t offset){
    return 0;
}
