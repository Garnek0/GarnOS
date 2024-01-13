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
#include <cpu/smp/spinlock.h>
#include "file.h"

#define FILESYS_TYPE_UNDEFINED 0
#define FILESYS_TYPE_INIT_USTAR 1
#define FILESYS_TYPE_FAT12 2
#define FILESYS_TYPE_FAT16 3
#define FILESYS_TYPE_FAT32 4
#define FILESYS_TYPE_EXFAT 5
#define FILESYS_TYPE_EXT 6
#define FILESYS_TYPE_EXT2 7
#define FILESYS_TYPE_EXT3 8
#define FILESYS_TYPE_EXT4 9
#define FILESYS_TYPE_ISO9660 10

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
    uint8_t type;
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
void filesys_set_name(filesys_t* fs, char* name);

#endif //FILESYS_H