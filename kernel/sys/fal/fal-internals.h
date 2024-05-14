/*  
*   File: fal-internals.h
*
*   Author: Garnek
*/
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FAL_INTERNALS_H
#define FAL_INTERNALS_H

#include <garn/types.h>
#include <garn/spinlock.h>
#include <garn/fal/filesys.h>
#include <garn/irq.h>

typedef struct _fd {
    file_t* file;
    size_t offset;
    int flags;
} fd_t;

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
ssize_t sys_getdents(stack_frame_t* regs, int fd, void* dirp, size_t count);

#endif //FAL_INTERNALS_H