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
#include "file.h"

#define FILESYS_TYPE_INIT_USTAR 0
#define FILESYS_TYPE_FAT12 1
#define FILESYS_TYPE_FAT16 2
#define FILESYS_TYPE_FAT32 3
#define FILESYS_TYPE_EXFAT 4
#define FILESYS_TYPE_EXT 5
#define FILESYS_TYPE_EXT2 6
#define FILESYS_TYPE_EXT3 7
#define FILESYS_TYPE_EXT4 8
#define FILESYS_TYPE_ISO9660 9

typedef struct _filesys {
    char name[32];
    uint8_t type;
    size_t size;

    struct _file* (*open)(struct _filesys* self, char* path, uint8_t access);
    uint8_t (*read)(struct _filesys* self, struct _file* file, size_t size, void* buf);
    uint8_t (*write)(struct _filesys* self, struct _file* file, size_t size, void* buf);
    uint8_t (*close)(struct _filesys* self, struct _file* file);
    uint8_t (*mkdir)(struct _filesys* self, char* path);
    uint8_t (*rmdir)(struct _filesys* self, char* path);

    //should not be touched by drivers
    bool _valid;
    size_t mountNumber;

    struct _drive* drive;
    size_t partition; //partition index in the drive

    void* context;
} filesys_t;

filesys_t* filesys_mount(filesys_t filesys);
void filesys_unmount(filesys_t* filesys);
filesys_t* filesys_get(size_t index);
filesys_t* filesys_get_all();
void filesys_set_name(filesys_t* fs, char* name);

#endif //FILESYS_H