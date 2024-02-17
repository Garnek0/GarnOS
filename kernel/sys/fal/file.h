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

#include <types.h>
#include <cpu/smp/spinlock.h>
#include "filesys.h"

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

typedef struct _fd {
    file_t* file;
    size_t offset;
    int flags;
} fd_t;

typedef struct _garn_dirent64 {
    uint64_t recordOffset;
    uint32_t recordLength;
    uint32_t type;
    char name[1];
} __attribute__((packed)) garn_dirent64_t;

//open file
file_t* file_open(char* path, int flags, int mode);

//close file
int file_close(file_t* file);

//read from file
ssize_t file_read(file_t* file, size_t size, void* buf, size_t offset);

//write to file
ssize_t file_write(file_t* file, size_t size, void* buf, size_t offset);

//new fd table
fd_t* file_alloc_fd_table(size_t size);

//reallocate existing fd table
fd_t* file_realloc_fd_table(fd_t* fd, size_t prevSize, size_t newSize);

int sys_open(stack_frame_t* regs, char* path, int flags, int mode);
ssize_t sys_read(stack_frame_t* regs, int fd, void* buf, size_t count);
ssize_t sys_write(stack_frame_t* regs, int fd, void* buf, size_t count);
int sys_close(stack_frame_t* regs, int fd);
char* file_get_absolute_path(char* root, char* relative);
uint64_t sys_getcwd(stack_frame_t* regs, const char* buf, size_t size);
int sys_chdir(stack_frame_t* regs, const char* path);
ssize_t sys_getdents64(stack_frame_t* regs, int fd, void* dirp, size_t count);

#endif //FILE_H