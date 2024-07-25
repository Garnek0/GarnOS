/*  
*   File: vfs.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef VFS_H
#define VFS_H

#define MAX_VFS 256

#include <garn/types.h>
#include <garn/dal/dal.h>
#include <garn/spinlock.h>
#include <garn/fal/vnode.h>

#define FILESYS_TYPE_UNDEFINED "undefined"
#define FILESYS_TYPE_INIT_USTAR "init-ustar"
#define FILESYS_TYPE_FAT12 "fat12"
#define FILESYS_TYPE_FAT16 "fat16"
#define FILESYS_TYPE_FAT32 "fat32"
#define FILESYS_TYPE_EXFAT "exfat"
#define FILESYS_TYPE_EXT "ext"
#define FILESYS_TYPE_EXT2 "ext2"
#define FILESYS_TYPE_EXT3 "ext3"
#define FILESYS_TYPE_EXT4 "ext4"
#define FILESYS_TYPE_ISO9660 "iso9660"

#define PATH_MAX 4096

struct _vfs; //Do this so that the compiler doesnt complain about incomplete types

typedef struct _vfs_operations {
    struct _file* (*open)(struct _vfs* self, char* path, int flags, int mode);
    ssize_t (*read)(struct _vfs* self, struct _file* file, size_t size, void* buf, size_t offset);
    ssize_t (*write)(struct _vfs* self, struct _file* file, size_t size, void* buf, size_t offset);
    int (*close)(struct _vfs* self, struct _file* file);
    int (*mkdir)(struct _vfs* self, char* path);
    int (*rmdir)(struct _vfs* self, char* path);
} vfs_operations_t;

typedef struct _vfs {
    char name[32];
    char type[32];
	size_t size;

    vfs_operations_t fsOperations;

    //should not be touched by drivers
    bool _valid;
    size_t mountNumber;

    struct _drive* drive;
    size_t partition; //partition index in the drive

    spinlock_t lock;

    void* context;
} vfs_t;

vfs_t* vfs_mount(vfs_t filesys);
void vfs_unmount(vfs_t* filesys);
vfs_t* vfs_get(size_t index);
vfs_t* vfs_get_all();

#endif //VFS_H
