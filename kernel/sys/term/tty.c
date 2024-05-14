/*  
*   File: tty.c
*
*   Author: Garnek
*   
*   Description: tty for process stdout and stderr
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "tty.h"
#include <garn/mm.h>
#include <garn/term.h>
#include <garn/fal/file.h>
#include <garn/fal/filesys.h>
#include <garn/kerrno.h>

file_t* tty;

ssize_t tty_write(filesys_t* self, file_t* file, size_t size, void* buf, size_t offset){
    for(size_t i = 0; i < size; i++){
        term_putchar(((char*)buf)[i]);
    }
    return size;
}

ssize_t tty_read(filesys_t* self, file_t* file, size_t size, void* buf, size_t offset){
    return -EINVAL;
}

int tty_close(filesys_t* self, file_t* file){
    return 0;
}

void tty_init(){
    filesys_t* ttyfs = kmalloc(sizeof(filesys_t));
    memset(ttyfs, 0, sizeof(filesys_t));
    ttyfs->_valid = true;
    ttyfs->size = 0;

    ttyfs->fsOperations.write = tty_write;
    ttyfs->fsOperations.read = tty_read;
    ttyfs->fsOperations.close = tty_close;

    tty = kmalloc(sizeof(file_t));
    memset(tty, 0, sizeof(file_t));
    tty->size = 0;
    tty->filename = "tty";
    tty->fs = ttyfs;
    tty->flags = O_RDWR;
    tty->refCount = 1;

    file_list_add(tty);
}