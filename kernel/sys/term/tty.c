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
#include <garn/term/term.h>
#include <garn/fal/vnode.h>
#include <garn/fal/vfs.h>
#include <garn/kerrno.h>

vnode_t* tty;

ssize_t tty_write(vfs_t* self, vnode_t* file, size_t size, void* buf, size_t offset){
    for(size_t i = 0; i < size; i++){
        term_putchar(((char*)buf)[i]);
    }
    return size;
}

ssize_t tty_read(vfs_t* self, vnode_t* file, size_t size, void* buf, size_t offset){
    return -EINVAL;
}

int tty_close(vfs_t* self, vnode_t* file){
    return 0;
}

void tty_init(){
    vfs_t* ttyfs = kmalloc(sizeof(vfs_t));
    memset(ttyfs, 0, sizeof(vfs_t));

    ttyfs->size = 0;

    ttyfs->fsOperations.write = tty_write;
    ttyfs->fsOperations.read = tty_read;
    ttyfs->fsOperations.close = tty_close;

    tty = kmalloc(sizeof(vnode_t));
    memset(tty, 0, sizeof(vnode_t));
    tty->size = 0;
    tty->filename = "tty";
    tty->fs = ttyfs;
    tty->flags = O_RDWR;
    tty->refCount = 1;

    vnode_list_add(tty);
}
