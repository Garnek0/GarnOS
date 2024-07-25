/*  
*   File: fat_close.c
*
*   Author: Garnek
*   
*   Description: FAT Driver close routine
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "fat.h"
#include <garn/mm.h>
#include <garn/kerrno.h>
#include <garn/kstdio.h>

int fat_close(vfs_t* self, vnode_t* file){
    if(file->fsData) kmfree(file->fsData);
    kmfree(file);
    return 0;
}
