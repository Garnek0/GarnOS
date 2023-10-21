/*  
*   File: filesys.c
*
*   Author: Garnek
*   
*   Description: Filesystem Abstraction
*/
// SPDX-License-Identifier: BSD-2-Clause
#include "filesys.h"

filesys_t filesystems[MAX_FILESYSTEMS];
size_t nextAvailFSIndex;

static size_t _filesys_get_next_avail_index(){
    for(size_t i = 0; i < MAX_FILESYSTEMS; i++){
        if(!filesystems[i]._avail) return i;
    }

    panic("Too many filesystems mounted! Limit of %d Filesystems exceeded!", MAX_FILESYSTEMS);
}

filesys_t* filesys_mount(filesys_t filesys){
    filesys._avail = true;

    filesys_t* fsAddr;

    filesystems[nextAvailFSIndex] = filesys;
    filesystems[nextAvailFSIndex].mountNumber = nextAvailFSIndex;
    fsAddr = &filesystems[nextAvailFSIndex];

    nextAvailFSIndex = _filesys_get_next_avail_index();

    return fsAddr;
}

void filesys_unmount(filesys_t* filesys){
    filesys->_avail = false;
    nextAvailFSIndex = _filesys_get_next_avail_index();
}

filesys_t* filesys_get(size_t index){
    return &filesystems[index];
}

filesys_t* filesys_get_all(){
    return filesystems;
}