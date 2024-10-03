/*  
*   File: initrd.c
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef INITRD_H
#define INITRD_H

#include <garn/types.h>
#include <garn/fal/vnode.h>
#include <garn/fal/vfs.h>

#define INITRD_FILENAME "initrd.grd"

typedef struct {
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mTime[12];
    char checksum[8];
    char typeFlag[1];
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
initrd_tar_header_t;

typedef struct {
    size_t startOffset;
} initrd_file_fs_data_t;

void initrd_init();

ssize_t initrd_read(vnode_t* self, size_t size, void* buf, size_t offset);
int initrd_inactive(vnode_t* self);
vnode_t* initrd_lookup(vnode_t* self, const char* name);
statfs_t initrd_statfs(vfs_t* self);

#endif //INITRD_H
