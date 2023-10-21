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
#include "drive.h"
#include "file.h"

typedef struct _filesys {
    bool _avail;
    char* name;
    size_t mountNumber;
    size_t totalSize;
    size_t usedSize;

    struct _file* (*open)(struct _filesys* self, char* path, uint8_t access);
    uint8_t (*read)(struct _filesys* self, struct _file* file, size_t size, void* buf);
    uint8_t (*write)(struct _filesys* self, struct _file* file, size_t size, void* buf);
    uint8_t (*close)(struct _filesys* self, struct _file* file);
    uint8_t (*mkdir)(struct _filesys* self, char* path);
    uint8_t (*rmdir)(struct _filesys* self, char* path);

    drive_t* drive;

    void* context;
} filesys_t;

filesys_t* filesys_mount(filesys_t filesys);
void filesys_unmount(filesys_t* filesys);
filesys_t* filesys_get(size_t index);
filesys_t* filesys_get_all();

#endif //FILESYS_H