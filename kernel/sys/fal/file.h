/*  
*   File: file.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FILE_H
#define FILE_H

#include <types.h>
#include "filesys.h"

#define FILE_ACCESS_R 0
#define FILE_ACCESS_W 1
#define FILE_ACCESS_RW 2

#define FILE_SEEK_SET 1
#define FILE_SEEK_CUR 2 
#define FILE_SEEK_END 3

typedef struct _file {
    char* filename;
    void* address;
    uint8_t mode;
    size_t size;
    size_t seek;
    struct _filesys* fs;
} file_t;

typedef struct _fd {
    file_t* file;
    uint8_t permissions;
} fd_t;

//open file
file_t* kfopen(char* path, uint8_t mode);

//modify current seek position
int kfseek(file_t* file, int seekPos, int whence);

//close file
int kfclose(file_t* file);

//read from file
int kfread(file_t* file, size_t size, void* buf);

//write to file
int kfwrite(file_t* file, size_t size, void* buf);

#endif //FILE_H