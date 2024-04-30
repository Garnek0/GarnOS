/*  
*   File: filesys.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FILESYS_H
#define FILESYS_H

#define MAX_FILESYSTEMS 256

#include <types.h>
#include <sys/dal/dal.h>
#include <cpu/multiproc/spinlock.h>
#include "file.h"

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

typedef struct _filesys_operations {
    struct _file* (*open)(struct _filesys* self, char* path, int flags, int mode);
    ssize_t (*read)(struct _filesys* self, struct _file* file, size_t size, void* buf, size_t offset);
    ssize_t (*write)(struct _filesys* self, struct _file* file, size_t size, void* buf, size_t offset);
    int (*close)(struct _filesys* self, struct _file* file);
    int (*mkdir)(struct _filesys* self, char* path);
    int (*rmdir)(struct _filesys* self, char* path);
} filesys_operations_t;

typedef struct _filesys {
    char name[32];
    char type[32];
    size_t size;

    filesys_operations_t fsOperations;

    //should not be touched by drivers
    bool _valid;
    size_t mountNumber;

    struct _drive* drive;
    size_t partition; //partition index in the drive

    spinlock_t lock;

    void* context;
} filesys_t;

filesys_t* filesys_mount(filesys_t filesys);
void filesys_unmount(filesys_t* filesys);
filesys_t* filesys_get(size_t index);
filesys_t* filesys_get_all();

#endif //FILESYS_H