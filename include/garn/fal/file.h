/*  
*   File: file.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FILE_H
#define FILE_H

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      (1 << 1)
#define O_CREAT     (1 << 6)
#define O_EXCL      (1 << 7)
#define O_TRUNC     (1 << 9)
#define O_APPEND    (1 << 10)
#define O_NONBLOCK  (1 << 11)
#define O_DIRECTORY (1 << 16)

#define S_ISUID 0x0800
#define S_ISGID 0x0400
#define S_ISVTX 0x0200
#define S_IRWXU 0x01C0
#define S_IRUSR 0x0100
#define S_IWUSR 0x0080
#define S_IXUSR 0x0040
#define S_IRWXG 0x0038
#define S_IRGRP 0x0020
#define S_IWGRP 0x0010
#define S_IXGRP 0x0008
#define S_IRWXO 0x0007
#define S_IROTH 0x0004
#define S_IWOTH 0x0002
#define S_IXOTH 0x0001

#define DT_DIR 0
#define DT_REG 1

#include <garn/types.h>
#include <garn/spinlock.h>
#include <garn/fal/filesys.h>

typedef struct _file {
    char* filename;
    int mode;
    int flags;
    int refCount;
    size_t size;
    struct _filesys* fs;
    void* fsData;

    spinlock_t lock;

    struct _file* next;
    struct _file* prev;
} file_t;

typedef struct _dirent {
    long inode;
    long offset; 
    uint16_t reclen;
    uint8_t type;
    char name[1];
}
#ifndef DOXYGEN
__attribute__((packed))
#endif
dirent_t;

void file_list_add(file_t* file);
void file_list_remove(file_t* file);

//open file
file_t* file_open(char* path, int flags, int mode);

//close file
int file_close(file_t* file);

//read from file
ssize_t file_read(file_t* file, size_t size, void* buf, size_t offset);

//write to file
ssize_t file_write(file_t* file, size_t size, void* buf, size_t offset);

#endif //FILE_H